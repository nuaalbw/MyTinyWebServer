/*************************************************************************
	> File Name: BlockQueue.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Sun 05 Sep 2021 11:30:22 AM CST
 ************************************************************************/

#ifndef _BLOCK_QUEUE_H__
#define _BLOCK_QUEUE_H__

#include <iostream>
#include <sys/time.h>
#include "Locker.h"
using namespace std;

/*
* 利用循环数组实现的阻塞队列，m_back = (m_back + 1) % m_maxSize;  
* 线程安全，每个操作前都要先加互斥锁，操作完后，再解锁
*/
template<typename T>
class BlockQueue
{
public:
	BlockQueue(int maxSize = 1000);
	~BlockQueue();
	
	/* 清空队列 */
	void clear();
	/* 判断队列是否满了 */
	bool isFull();
	/* 判断队列是否为空 */
	bool isEmpty();
	/* 返回队首元素 */
	bool front(T& value);
	/* 返回队尾元素 */
	bool back(T& value);
	/* 获取当前队列大小 */
	int size();
	/* 获取队列的容量 */
	int maxSize();
	/* 向队列中添加元素 */
	bool push(const T& item);
	/* 弹出队首元素，若队列为空，则等待条件变量 */
	bool pop(T& item);
	/* 增加了超时处理 */
	bool pop(T& item, int ms_timeout);

private:
	Locker m_mutex;
	Cond m_cond;
	T* m_array;
	int m_size;
	int m_maxSize;
	int m_front;
	int m_back;
};

template<typename T>
BlockQueue<T>::BlockQueue(int maxSize)
{
	if (maxSize <= 0) {
		throw exception();
	}

	m_maxSize = maxSize;
	m_array = new T[maxSize];
	m_size = 0;
	m_front = -1;
	m_back = -1;
}

template<typename T>
BlockQueue<T>::~BlockQueue()
{
	m_mutex.lock();
	if (m_array != nullptr) {
		delete [] m_array;
		m_array = nullptr;
	}
	m_mutex.unlock();
}

template<typename T>
void BlockQueue<T>::clear()
{
	m_mutex.lock();
	m_size = 0;
	m_front = -1;
	m_back = -1;
	m_mutex.unlock();
}

template<typename T>
bool BlockQueue<T>::isFull()
{
	m_mutex.lock();
	if (m_size >= m_maxSize) {
		m_mutex.unlock();
		return true;
	}
	m_mutex.unlock();
	return false;
}

template<typename T>
bool BlockQueue<T>::isEmpty()
{
	m_mutex.lock();
	if (m_size == 0) {
		m_mutex.unlock();
		return true;
	}
	m_mutex.unlock();
	return false;
}

template<typename T>
bool BlockQueue<T>::front(T& value)
{
	m_mutex.lock();
	if (m_size == 0) {
		m_mutex.unlock();
		return false;
	}
	value = m_array[m_front];
	m_mutex.unlock();
	return true;
}

template<typename T>
bool BlockQueue<T>::back(T& value)
{
	m_mutex.lock();
	if (m_size == 0) {
		m_mutex.unlock();
		return false;
	}
	value = m_array[m_back];
	m_mutex.unlock();
	return true;
}

template<typename T>
int BlockQueue<T>::size()
{
	int tmp = 0;
	m_mutex.lock();
	tmp = m_size;
	m_mutex.unlock();
	return tmp;
}

template<typename T>
int BlockQueue<T>::maxSize()
{
	int tmp = 0;
	m_mutex.lock();
	tmp = m_maxSize;
	m_mutex.unlock();
	return tmp;
}

/* 
往队列添加元素，需要将所有使用队列的线程先唤醒
当有元素push进队列,相当于生产者生产了一个元素
若当前没有线程等待条件变量,则唤醒无意义
*/
template<typename T>
bool BlockQueue<T>::push(const T& item)
{
	m_mutex.lock();
	if (m_size >= m_maxSize) {
		m_cond.broadcast();
		m_mutex.unlock();
		return false;
	}

	m_back = (m_back + 1) % m_maxSize;
	m_array[m_back] = item;
	m_size++;
	m_cond.broadcast();
	m_mutex.unlock();
	return true;
}

template<typename T>
bool BlockQueue<T>::pop(T& item)
{
	/* 弹出队首元素时，若队列为空，将会等待条件变量 */
	m_mutex.lock();
	while (m_size <= 0) {
		if (!m_cond.wait(m_mutex.get())) {
			m_mutex.unlock();
			return false;
		}
	}

	m_front = (m_front + 1) % m_maxSize;
	item = m_array[m_front];
	m_size--;
	m_mutex.unlock();
	return true;
}

template<typename T>
bool BlockQueue<T>::pop(T& item, int ms_timeout)
{
	struct timespec t = { 0, 0 };
	struct timeval now = { 0, 0 };
	gettimeofday(&now, nullptr);

	m_mutex.lock();
	if (m_size <= 0) {
		t.tv_sec = now.tv_sec + ms_timeout / 1000;
		t.tv_nsec = (ms_timeout % 1000) * 1000;
		if (!m_cond.timeWait(m_mutex.get(), t)) {
			m_mutex.unlock();
			return false;
		}
	}

	/* 若超时时间到时队列仍为空，则解锁后返回 */
	if (m_size <= 0) {
		m_mutex.unlock();
		return false;
	}
	/* 否则弹出队首元素 */
	m_front = (m_front + 1) % m_maxSize;
	item = m_array[m_front];
	m_size--;
	m_mutex.unlock();
	return true;
}

#endif