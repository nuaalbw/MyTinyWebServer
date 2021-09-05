/*************************************************************************
	> File Name: Locker.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: 2021年07月22日 星期四 15时54分39秒
 ************************************************************************/

#ifndef _LOCKER_H__
#define _LOCKER_H__

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/* 封装信号量的类 */
class Sem
{
public:
	/* 创建并初始化一个信号量 */
	Sem();
	/* 创建并初始化一个信号量，设置初值为num */
	Sem(int num);
	/* 销毁信号量 */
	~Sem();
	/* 等待一个信号量 */
	bool wait();
	/* 增加信号量 */
	bool post();

private:
	sem_t m_sem;
};

/* 封装互斥锁的类 */
class Locker
{
public:
	/* 创建并初始化互斥锁 */	
	Locker();
	/* 销毁互斥锁 */
	~Locker();
	/* 获取互斥锁 */
	bool lock();
	/* 释放互斥锁 */
	bool unlock();
	/* 获取当前互斥锁对象 */
	pthread_mutex_t* get();

private:
	pthread_mutex_t m_mutex;
};

/* 封装条件变量的类 */
class Cond
{
public:
	/* 创建并初始化条件变量 */
	Cond();
	/* 销毁条件变量 */
	~Cond();

	/* 等待条件变量 */
	bool wait(pthread_mutex_t* mutex);
	bool timeWait(pthread_mutex_t* mutex, struct timespec t);

	/* 唤醒等待条件变量的队列 */
	bool signal();
	bool broadcast();

private:
	// pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
};

#endif
