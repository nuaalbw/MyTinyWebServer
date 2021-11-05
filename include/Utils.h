/*************************************************************************
	> File Name: Utils.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Tue 07 Sep 2021 05:13:00 PM CST
 ************************************************************************/

#ifndef _UTILS_H__
#define _UTILS_H__

#include <iostream>
#include "HeapTimer.h"
#include "Web.h"
using namespace std;

class Utils
{
public:
	Utils();
	~Utils();

	void init(int timeslot);
	/* 对文件描述符设置非阻塞 */
	int setNonblocking(int fd);
	/* 向epoll内核事件表中注册事件，ET模式，选择开启EPOLLONESHOT */
	void addfd(int epollfd, int fd, bool oneShot, TriggerMode mode);
	/* 信号处理的回调函数 */
	static void sigHandler(int sig);
	/* 注册信号处理函数 */
	void addSig(int sig, void(handler)(int), bool restart = true);
	/* 定时处理任务，重新定时以不断触发SIGALARM信号 */
	void timerHandler();
	/* 向客户连接发送错误信息 */
	void showError(int connfd, const char* info);

public:
	static int* u_pipefd;
	TimeHeap m_timeHeap;
	static int u_epollfd;
	int m_timeslot;
};

void cbFunc(ClientData* userData);

#endif