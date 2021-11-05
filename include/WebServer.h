/*************************************************************************
	> File Name: WebServer.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Thu 09 Sep 2021 07:50:21 PM CST
 ************************************************************************/

#ifndef _WEB_SERVER_H__
#define _WEB_SERVER_H__

#include <iostream>
#include "Web.h"
#include "Threadpool.h"
#include "HttpConn.h"
#include "Utils.h"
using namespace std;

/* 最大文件描述符数量 */
static constexpr int MAX_FD = 10000;
/* 最大事件数 */
static constexpr int MAX_EVENT_NUMBER = 10000;
/* 最小超时单位 */
static constexpr int TIMESLOT = 5;

class WebServer
{
public:
	WebServer();
	~WebServer();

	/* 初始化web服务器 */
	void init(int port, string dbUser, string dbPwd, string dbName, 
			  int logWrite, int optLinger, int triggerMode, int sqlNum, 
			  int threadNum, int closeLog, ActorModel model);
	/* 初始化线程池 */
	void threadPoolInit();
	/* 初始化数据库连接池 */
	void connectionPoolInit();
	/* 初始化日志文件 */
	void logWriteInit();
	/* 配置触发模式 */
	void trigMode();
	/* 设置监听 */
	void eventListen();
	/* 服务器主循环 */
	void eventLoop();
	/* 初始化定时器 */
	void initTimer(int connfd, sockaddr_in clientAddress);
	/* 调整定时器 */
	void adjustTimer(HeapTimer* timer);
	/* 处理定时器事件 */
	void dealTimer(HeapTimer* timer, int sockfd);
	/* 处理客户端数据 */
	bool dealClientData();
	/* 处理信号事件 */
	bool dealWithSignal(bool& timeout, bool& stopServer);
	/* 处理读事件 */
	void dealWithRead(int sockfd);
	/* 处理写事件 */
	void dealWithWrite(int sockfd);
	/* 设置守护进程 */
	int initDaemon();

public:
	int m_port;
	char* m_root;
	int m_logWrite;
	int m_closeLog;
	ActorModel m_actormodel;

	int m_pipefd[2];
	int m_epollfd;
	HttpConn* m_users;

	/* 数据库相关 */
	ConnectionPool* m_connPool;
	string m_dbUser;
	string m_dbPassword;
	string m_dbName;
	int m_sqlNum;

	/* 线程池相关 */
	Threadpool<HttpConn>* m_pool;
	int m_threadNum;

	/* epoll相关 */
	epoll_event events[MAX_EVENT_NUMBER];
	int m_listenfd;
	int m_optLinger;
	int m_triggerMode;
	TriggerMode m_lfdMode;
	TriggerMode m_cfdMode;

	/* 定时器相关 */
	ClientData* m_usersTimer;
	Utils m_utils;
};

#endif
