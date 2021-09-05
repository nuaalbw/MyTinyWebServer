/*************************************************************************
	> File Name: main.cpp
	> Author: lbw
	> Mail: 296273803@qq.com 
	> Created Time: 2021年07月27日 星期二 15时58分09秒
 ************************************************************************/

#include "Locker.h"
#include "Threadpool.h"
#include "HttpConn.h"
using namespace std;

#define MAX_FD	65536
#define MAX_EVENT_NUMBER	10000

/* extern关键字可以置于变量或者函数前，以表示变量或者函数的定义在别的文件中 */
extern void addfd(int epollfd, int fd, bool oneShot);
extern void removefd(int epollfd, int fd);

static void addSignal(int sig, void(*handler)(int), bool restart = true)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if (restart) {
		sa.sa_flags |= SA_RESTART;
	}
	sigfillset(&sa.sa_mask);
	int ret = sigaction(sig, &sa, nullptr);
	if (ret == -1) 
		sys_error("addSignal: sigaction error");
}

static void showError(int connfd, const char* info)
{
	printf("%s", info);
	send(connfd, info, strlen(info), 0);
	close(connfd);
}

int main(int argc, char* argv[])
{
	/* 忽略SIGPIPE信号
	 * 当向一个已经关闭的socket进行两次write操作时，将会触发SIGPIPE信号 */
	addSignal(SIGPIPE, SIG_IGN);

	/* 创建线程池 */
	Threadpool<HttpConn>* pool = nullptr;
	try {
		pool = new Threadpool<HttpConn>;
	}
	catch (...){
		return 1;
	}

	/* 预先为每个可能的客户连接分配一个HttpConn对象 */
	HttpConn* users = new HttpConn[MAX_FD];
	if (users == nullptr) {
		fprintf(stderr, "HttpConn init error\n");
		exit(1);
	}
	int userCount = 0;

	/* 创建socket，得到监听文件描述符listenfd  */
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
		sys_error("socket error");

	/* 通过设置SO_LINGER选项，保证服务器数据能够发送到对端 */
	struct linger tmp = { 1, 0 };
	int ret = setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
	if (ret == -1)
		sys_error("setsockopt error");

	/* 为listenfd绑定地址结构 */
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(SERV_PORT));
	address.sin_addr.s_addr = INADDR_ANY;
	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	if (ret < 0)
		sys_error("bind error");

	ret = listen(listenfd, LISTENQ);
	if (ret < 0)
		sys_error("listen error");

	/* 创建epoll内核事件表 */
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	if (epollfd < 0)
		sys_error("epoll_create error");
	addfd(epollfd, listenfd, false);
	HttpConn::m_epollfd = epollfd;
	
	while (true) {
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if (number < 0 && errno != EINTR) {
			perror("epoll_wait error");
			break;
		}
		
		for (int i = 0; i < number; ++i) {
			int sockfd = events[i].data.fd;
			/* 监听文件描述符上发生读事件，说明有新连接到来 */
			if (sockfd == listenfd) {
				struct sockaddr_in clientAddr;
				socklen_t addrLen = sizeof(clientAddr);
				int connfd = accept(listenfd, (struct sockaddr*)&clientAddr, &addrLen);
				if (connfd < 0 && errno != EWOULDBLOCK) {
					perror("accept error");
					continue;
				}
				if (HttpConn::m_userCount >= MAX_FD) {
					showError(connfd, "Internal server busy");
					continue;
				}
				/* 初始化客户连接 */
				users[connfd].init(connfd, clientAddr);
			}
			/* 如果有异常，直接关闭客户连接 */
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				users[sockfd].closeConn();
			}
			/* 若连接描述符上发生读事件，说明客户有数据发来 */
			else if (events[i].events & EPOLLIN) {
				/* 根据读的结果，决定是将任务添加到线程池，还是关闭连接 */	
				if (users[sockfd].read()) {
					pool->append(users + sockfd);
				}
				else {
					users[sockfd].closeConn();
				}
			}
			/* 若连接文件描述符上发生可写事件 */
			else if (events[i].events & EPOLLOUT) {
				/* 根据写的结果，决定是否关闭连接 */
				if (!users[sockfd].write()) {
					users[sockfd].closeConn();
				}
			}
		}

	}
	close(epollfd);
	close(listenfd);
	delete [] users;
	delete pool;

	return 0;
}




