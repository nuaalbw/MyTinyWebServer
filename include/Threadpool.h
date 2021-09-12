/*************************************************************************
	> File Name: Threadpool.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: 2021年07月24日 星期六 17时01分05秒
 ************************************************************************/

#ifndef _THREADPOOL_H__
#define _THREADPOOL_H__

#include <list>
#include <exception>
#include <pthread.h>
#include "Web.h"
#include "Locker.h"
#include "ConnectionPool.h"

/* 线程池类，将它定义为模板类是为了代码复用, T是任务类 */
template<typename T>
class Threadpool
{
public:
	Threadpool(ActorModel model, ConnectionPool* connPool, int threadNumber = 8, int maxRequests = 10000);
	~Threadpool();
	/* 向请求队列中添加任务 */
	bool append(T* request, int state);
	/* proactor模式下添加任务 */
	bool append_p(T* requrst);

private:
	/* 工作线程运行的函数, 它不断从工作队列中取出任务并执行之 */
	static void* worker(void* arg);
	void run();

private:
	/* 线程池中的线程数 */
	int m_threadNumber;
	/* 请求队列中允许的最大请求数 */
	int m_maxRequests;
	/* 描述线程池的数组, 其大小为m_threadNumber */
	pthread_t* m_threads;
	/* 请求队列 */
	std::list<T*> m_workQueue;
	/* 保护请求队列的互斥锁 */
	Locker m_queueLocker;
	/* 是否有任务需要处理 */
	Sem m_queueStat;
	/* 是否结束线程 */
	bool m_stop;
	/* 数据库连接池 */
	ConnectionPool* m_connPool;
	/* 模型切换 */
	ActorModel m_model;
};

template<typename T>
Threadpool<T>::Threadpool(ActorModel model, ConnectionPool* connPool, int threadNumber, int maxRequests)
{
	/* 参数检查 */
	if (threadNumber <= 0 || maxRequests <= 0) {
		throw std::exception();
	}
	/* 成员变量初始化 */
	m_threadNumber = threadNumber;
	m_maxRequests = maxRequests;
	m_stop = false;
	m_threads = new pthread_t[m_threadNumber];
	m_model = model;
	m_connPool = connPool;
	if (m_threads == nullptr) {
		throw std::exception();
	}

	/* 创建threadNumber个线程, 并设置线程分离 */
	for (int i = 0; i < threadNumber; ++i) {
		printf("create the %dth thread\n", i);
		if (pthread_create(m_threads + i, nullptr, worker, this) != 0) {
			delete [] m_threads;
			throw std::exception();
		}
		if (pthread_detach(m_threads[i]) != 0) {
			delete [] m_threads;
			throw std::exception();
		}
	}
}

template<typename T>
Threadpool<T>::~Threadpool()
{
	delete [] m_threads;
	m_stop = true;
}

template<typename T>
bool Threadpool<T>::append(T* request, int state)
{
	m_queueLocker.lock();
	if (m_workQueue.size() >= m_maxRequests) {
		m_queueLocker.unlock();
		return false;
	}
	request->m_state = state;
	m_workQueue.push_back(request);
	m_queueLocker.unlock();
	m_queueStat.post();
	return true;
}

template<typename T>
bool Threadpool<T>::append_p(T* request)
{
	/* 操作工作队列时一定要加锁, 因为它被所有线程共享 */
	m_queueLocker.lock();
	if (m_workQueue.size() >= m_maxRequests) {
		m_queueLocker.unlock();
		return false;
	}
	m_workQueue.push_back(request);
	m_queueLocker.unlock();
	m_queueStat.post();
	return true;
}

template<typename T>
void* Threadpool<T>::worker(void* arg)
{
	Threadpool* pool = (Threadpool*)arg;
	pool->run();
	return pool;
}

template<typename T>
void Threadpool<T>::run()
{
	while (!m_stop) {
		m_queueStat.wait();
		m_queueLocker.lock();
		if (m_workQueue.empty()) {
			m_queueLocker.unlock();
			continue;
		}
		T* request = m_workQueue.front();
		m_workQueue.pop_front();
		m_queueLocker.unlock();
		if (!request) {
			continue;
		}
		if (m_model == REACTOR) {
			if (request->m_state == 0) {
				if (request->readn()) {
					request->m_improv = 1;
					ConnectionRAII mysqlConn(&request->m_mysql, m_connPool);
					request->process();
				}
				else {
					request->m_improv = 1;
					request->m_timerFlag = 1;
				}
			}
			else {
				if (request->writen()) {
					request->m_improv = 1;
				}
				else {
					request->m_improv = 1;
					request->m_timerFlag = 1;
				}
			}
		}
		else {
			ConnectionRAII mysqlConn(&request->m_mysql, m_connPool);
			request->process();
		}
	}
}

#endif
