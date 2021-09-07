/*************************************************************************
	> File Name: TimerList.cpp
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Tue 07 Sep 2021 04:03:16 PM CST
 ************************************************************************/

#include <iostream>
#include "TimerList.h"
using namespace std;

SortTimerList::SortTimerList()
{
	m_head = nullptr;
	m_tail = nullptr;
}

SortTimerList::~SortTimerList()
{
	UtilTimer* tmp = m_head;
	while (tmp) {
		m_head = tmp->next;
		delete tmp;
		tmp = m_head;
	}
}

void SortTimerList::addTimer(UtilTimer* timer)
{
	if (timer == nullptr) {
		return;
	}
	/* 若当前链表为空，则头尾指针均指向该节点 */
	if (m_head == nullptr) {
		m_head = m_tail = timer;
		return;
	}
	/* 若当前定时器超时时间小于链表头节点，则插入到链表头部 */
	if (timer->expire < m_head->expire) {
		timer->next = m_head;
		m_head->prev = timer;
		m_head = timer;
		return;
	}
	/* 其他情况则调用辅助插入函数 */
	addTimer(timer, m_head);
}

void SortTimerList::adjustTimer(UtilTimer* timer)
{
	if (timer == nullptr) {
		return;
	}
	UtilTimer* tmp = timer->next;
	if (tmp == nullptr || (timer->expire < tmp->expire)) {
		return;
	}
	/* 将节点从链表中摘下，再重新插入之 */
	if (timer == m_head) {
		m_head = m_head->next;
		m_head->prev = nullptr;
		timer->next = nullptr;
		addTimer(timer, m_head);
	}
	else {
		timer->prev->next = timer->next;
		timer->next->prev = timer->prev;
		addTimer(timer, timer->next);
	}
}

void SortTimerList::deleteTimer(UtilTimer* timer)
{
	if (timer == nullptr) {
		return;
	}
	/* 若当前链表中仅有这一个计时器 */
	if ((timer == m_head) && (timer == m_tail)) {
		delete timer;
		m_head = nullptr;
		m_tail = nullptr;
		return;
	}
	/* 若删除的是头节点 */
	if (timer == m_head) {
		m_head = m_head->next;
		m_head->prev = nullptr;
		delete timer;
		return;
	}
	/* 若删除的是尾节点 */
	if (timer == m_tail) {
		m_tail = m_tail->prev;
		m_tail->next = nullptr;
		delete timer;
		return;
	}
	/* 若删除的是中间节点 */
	timer->prev->next = timer->next;
	timer->next->prev = timer->prev;
	delete timer;
}

void SortTimerList::tick()
{
	if (m_head == nullptr) {
		return;
	}
	/* 获取当前时间，与定时器链表中的各节点进行对比 */
	time_t cur = time(nullptr);
	UtilTimer* tmp = m_head;
	while (tmp) {
		/* 若当前节点的超时时间还没到，则退出 */
		if (cur < tmp->expire) {
			break;
		}
		/* 否则调用当前节点的任务回调函数 */
		tmp->cbFunc(tmp->userData);
		m_head = tmp->next;
		/* 将已到达超时时间的定时器从链表中摘除 */
		if (m_head) {
			m_head->prev = nullptr;
		}
		delete tmp;
		tmp = m_head;
	}
}

void SortTimerList::addTimer(UtilTimer* timer, UtilTimer* listHead)
{
	UtilTimer* prev = listHead;
	UtilTimer* tmp =  prev->next;
	while (tmp) {
		if (timer->expire < tmp->expire) {
			prev->next = timer;
			timer->next = tmp;
			tmp->prev = timer;
			timer->prev = prev;
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
	/* 插入到链表尾部的情况 */
	if (tmp == nullptr) {
		prev->next = timer;
		timer->prev = prev;
		timer->next = nullptr;
		m_tail = timer;
	}
}