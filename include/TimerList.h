/*************************************************************************
	> File Name: TimerList.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Tue 07 Sep 2021 03:19:41 PM CST
 ************************************************************************/

#ifndef _TIMER_LIST__
#define _TIMER_LIST__

#include <iostream>
#include "Web.h"
#include "Log.h"
using namespace std;

/* 前向声明 */
class UtilTimer;
/* 用户数据结构*/
struct ClientData
{
	/* 客户端地址 */
	sockaddr_in address;
	/* socket文件描述符 */
	int sockfd;
	/* 定时器 */
	UtilTimer* timer;
};

class UtilTimer
{
public:
	UtilTimer(): prev(nullptr), next(nullptr) {}

public:
	/* 任务的超时时间，使用绝对时间 */
	time_t expire;
	/* 任务的回调函数 */
	void (*cbFunc)(ClientData*);
	/* 回调函数处理的用户数据 */
	ClientData* userData;
	/* 指向前一个定时器 */
	UtilTimer* prev;
	/* 指向后一个定时器 */
	UtilTimer* next;
};

/* 升序排序的双向定时器链表 */
class SortTimerList
{
public:
	SortTimerList();
	~SortTimerList();

	/* 将目标定时器添加到链表中 */
	void addTimer(UtilTimer* timer);
	/* 调整对应定时器在链表中的位置 */
	void adjustTimer(UtilTimer* timer);
	/* 将目标定时器从链表中删除 */
	void deleteTimer(UtilTimer* timer);
	/* 处理链表上到期的任务 */
	void tick();

private:
	/* 一个重载的辅助函数 */
	void addTimer(UtilTimer* timer, UtilTimer* listHead);

private:
	UtilTimer* m_head;
	UtilTimer* m_tail;
};

#endif