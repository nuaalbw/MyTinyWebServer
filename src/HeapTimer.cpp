/*************************************************************************
	> File Name: HeapTimer.cpp
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Thu 04 Nov 2021 04:19:21 PM CST
 ************************************************************************/

#include <iostream>
#include "HeapTimer.h"
using namespace std;


HeapTimer::HeapTimer(int delay)
{
	expire = time(nullptr) + delay;
}

TimeHeap::TimeHeap(): capacity(10), curSize(0) 
{
	array = new HeapTimer*[capacity];		// 创建堆数组
	if (array == nullptr) {
		throw exception();
	}
	for (int i = 0; i < capacity; ++i) {
		array[i] = nullptr;
	}
}

TimeHeap::TimeHeap(int cap): capacity(cap), curSize(0)
{
	array = new HeapTimer*[capacity];		// 创建堆数组
	if (array == nullptr) {
		throw exception();
	}
	for (int i = 0; i < capacity; ++i) {
		array[i] = nullptr;
	}
}

TimeHeap::TimeHeap(HeapTimer** initArray, int size, int capacity)
: curSize(size), capacity(capacity)
{
	if (capacity < size) {
		throw exception();
	}
	array = new HeapTimer*[capacity];		// 创建堆数组
	if (array == nullptr) {
		throw exception();
	}
	for (int i = 0; i < capacity; ++i) {
		array[i] = nullptr;
	}
	if (size != 0) {
		/* 初始化堆数组 */
		for (int i = 0; i < size; ++i) {
			array[i] = initArray[i];
		}
		/* 从最后一个非叶子节点开始，建立一个小顶堆 */
		for (int i = (curSize - 1) / 2; i >= 0; --i) {
			percolateDown(i);
		}
	}
}

TimeHeap::~TimeHeap()
{
	for (int i = 0; i < curSize; ++i) {
		if (array[i] != nullptr) {
			delete array[i];
			array[i] = nullptr;
		}
		delete [] array;
	}
}

void TimeHeap::addTimer(HeapTimer* timer)
{
	if (timer == nullptr) {
		return;
	}
	if (curSize >= capacity) {	// 	如果当前堆数组容量不够，则扩大一倍
		resize();
	}
	/* 推到堆的末尾，开始上移操作 */
	array[curSize] = timer;
	percolateUp(curSize);
	curSize++;
}

void TimeHeap::delTimer(HeapTimer* timer)
{
	if (timer == nullptr) {
		return;
	}
	int loc = timer->loc;
	/* 与堆尾定时器交换位置 */
	swapTimer(loc, curSize - 1);
	curSize--;
	percolateDown(loc);
	delete timer;
}

void TimeHeap::adjustTimer(HeapTimer* timer)
{
	percolateDown(timer->loc);
}

HeapTimer* TimeHeap::top() const
{
	if (empty()) {
		return nullptr;
	}
	return array[0];
}

void TimeHeap::popTimer()
{
	if (empty()) {
		return;
	}
	if (array[0]) {
		delete array[0];
		/* 将原来的堆顶元素替换为堆数组中最后一个元素 */
		array[0] = array[--curSize];
		/* 对新的堆顶元素执行下虑操作,维持堆的特性 */
		percolateDown(0);
	}
}

void TimeHeap::tick()
{
	HeapTimer* tmp = array[0];
	/* 循环处理堆中到期的定时器 */
	time_t cur = time(nullptr);
	while (!empty()) {
		if (tmp == nullptr) {
			break;
		}
		/* 如果连堆顶的计时器都没到期，则剩下的也没到期，退出循环 */
		if (tmp->expire > cur) {
			break;
		}
		/* 否则将循环执行堆顶定时器的任务 */
		if (array[0]->cbFunc) {
			array[0]->cbFunc(array[0]->userData);
		}
		/* 执行完任务后，弹出堆顶元素 */
		popTimer();
		tmp = array[0];
	}
}

bool TimeHeap::empty() const
{
	return curSize == 0;
}

void TimeHeap::percolateDown(int hole)
{
	int left = 2 * hole + 1, right = 2 * hole + 2;
	int minLoc = hole;
	if (left < curSize && array[left]->expire < array[minLoc]->expire) {
		minLoc = left;
	}
	if (right < curSize && array[right]->expire < array[minLoc]->expire) {
		minLoc = right;
	}
	if (minLoc != hole) {
		swapTimer(minLoc, hole);
		percolateDown(minLoc);
	}
}

void TimeHeap::percolateUp(int hole)
{
	HeapTimer* timer = array[hole];

	int parent = 0;
	for (; hole > 0; hole = parent) {
		parent = (hole - 1) / 2;
		if (array[parent]->expire <= timer->expire) {
			break;
		}
		array[hole] = array[parent];
		array[hole]->loc = hole;
	}
	array[hole] = timer;
	array[hole]->loc = hole;
}

void TimeHeap::resize()
{
	HeapTimer** temp = new HeapTimer*[2 * capacity];
	if (temp == nullptr) {
		throw exception();
	}
	for (int i = 0; i < 2 * capacity; ++i) {
		temp[i] = nullptr;
	}
	capacity = 2 * capacity;
	for (int i = 0; i < curSize; ++i) {
		temp[i] = array[i];
	}
	delete [] array;
	array = temp;
}

void TimeHeap::swapTimer(int a, int b)
{
	HeapTimer* tmp = array[a];
	array[a] = array[b];
	array[a]->loc = a;		// 更新定时器所处位置 

	array[b] = tmp;
	array[b]->loc = b;		// 更新定时器所处位置
}

void TimeHeap::printSelf()
{
	printTree(0);
}

void TimeHeap::printTree(int hole)
{
	static int level = -1; //记录是第几层次
	int i;

	if (hole >= curSize) {
		return;
	}

	level++;
	printTree(2 * hole + 2);
	level--;

	level++;
	for (i = 0; i < level; i++) {
		printf("\t");
	}
	printf("%2d\n", array[hole]->expire);
	printTree(2 * hole + 1);
	level--;
}