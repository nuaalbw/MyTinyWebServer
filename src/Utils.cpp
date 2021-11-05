/*************************************************************************
	> File Name: Utils.cpp
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Tue 07 Sep 2021 05:17:24 PM CST
 ************************************************************************/

#include <iostream>
#include "Utils.h"
#include "HttpConn.h"
using namespace std;

/* 类的静态成员需在类外初始化 */
int* Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

Utils::Utils()
{
}

Utils::~Utils()
{
}

void Utils::init(int timeslot)
{
	m_timeslot = timeslot;
}

int Utils::setNonblocking(int fd)
{
	int oldOption = fcntl(fd, F_GETFL);
	int newOption = oldOption | O_NONBLOCK;
	fcntl(fd, F_SETFL, newOption);
	return oldOption;
}

void Utils::addfd(int epollfd, int fd, bool oneShot, TriggerMode mode)
{
	epoll_event event;
	event.data.fd = fd;
	/* 边缘触发模式 */
	if (mode == EPOLL_ET) {
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	}
	/* 水平触发模式 */
	else {
		event.events = EPOLLIN | EPOLLRDHUP;
	}
	if (oneShot) {
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setNonblocking(fd);
}

void Utils::sigHandler(int sig)
{
	/* 为保证函数的可重入性，保留原来的errno */
	int saveErrno = errno;
	int msg = sig;
	/* 统一事件源，通过管道向主线程通知信号事件 */
	send(u_pipefd[1], (char*)&msg, 1, 0);
	errno = saveErrno;
}

void Utils::addSig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart) {
        sa.sa_flags |= SA_RESTART;
	}
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void Utils::timerHandler()
{
	m_timeHeap.tick();
	alarm(m_timeslot);	
}

void Utils::showError(int connfd, const char* info)
{
	send(connfd, info, strlen(info), 0);
	close(connfd);
}

void cbFunc(ClientData* userData)
{
	epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, userData->sockfd, nullptr);
	assert(userData);
	close(userData->sockfd);
	HttpConn::m_userCount--;
}
