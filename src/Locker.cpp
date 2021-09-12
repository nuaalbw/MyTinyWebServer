/*************************************************************************
	> File Name: Locker.cpp
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: 2021年07月22日 星期四 15时59分15秒
 ************************************************************************/

#include "Locker.h"
using namespace std;

Sem::Sem()
{
	if (sem_init(&m_sem, 0, 0) != 0) {
		/* 构造函数没有返回值, 可以通过抛出异常来报告错误 */
		throw exception();
	}
}

Sem::Sem(int num)
{
	if (sem_init(&m_sem, 0, num) != 0) {
		throw exception();
	}
}

Sem::~Sem()
{
	sem_destroy(&m_sem);
}

bool Sem::wait()
{
	return sem_wait(&m_sem) == 0;
}

bool Sem::post()
{
	return sem_post(&m_sem) == 0;
}

Locker::Locker()
{
	if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
		throw exception();
	}
}

Locker::~Locker()
{
	pthread_mutex_destroy(&m_mutex);
}

bool Locker::lock()
{
	return pthread_mutex_lock(&m_mutex) == 0;
}

bool Locker::unlock()
{
	return pthread_mutex_unlock(&m_mutex) == 0;
}

pthread_mutex_t* Locker::get()
{
	return &m_mutex;
}

Cond::Cond()
{
	// if (pthread_mutex_init(&m_mutex, NULL) != 0) {
	// 	throw exception();
	// }
	if (pthread_cond_init(&m_cond, nullptr) != 0) {
		// pthread_mutex_destroy(&m_mutex);
		throw exception();
	}
}

Cond::~Cond()
{
	// pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_cond);
}

bool Cond::wait(pthread_mutex_t* mutex)
{
	int ret = 0;
	// pthread_mutex_lock(&m_mutex);
	ret = pthread_cond_wait(&m_cond, mutex);
	// pthread_mutex_unlock(&m_mutex);
	return ret == 0;
}

bool Cond::timeWait(pthread_mutex_t* mutex, struct timespec t)
{
	int ret = 0;
	ret = pthread_cond_timedwait(&m_cond, mutex, &t);
	return ret == 0;
}

bool Cond::signal()
{
	return pthread_cond_signal(&m_cond) == 0;
}

bool Cond::broadcast()
{
	return pthread_cond_broadcast(&m_cond) == 0;
}