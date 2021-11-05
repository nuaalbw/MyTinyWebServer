/*************************************************************************
	> File Name: HeapTimer.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Thu 04 Nov 2021 03:38:08 PM CST
 ************************************************************************/

#pragma once
#include <iostream>
#include <unordered_map>
#include <netinet/in.h>
#include <time.h>
using namespace std;

#define BUFFER_SIZE	64

class HeapTimer;	// 前向声明
/* 绑定socket和定时器 */
struct ClientData
{
	sockaddr_in address;
	int sockfd;
	char buf[BUFFER_SIZE];
	HeapTimer* timer;
};

/* 定时器类 */
class HeapTimer
{
public:
	HeapTimer(int delay);

public:
	time_t expire;					/* 定时器生效的绝对时间 */
	void (*cbFunc)(ClientData*);	/* 定时器的回调函数 */
	ClientData* userData;			/* 用户数据 */
	int loc;						/* 记录定时器所处位置 */
};

/* 时间堆类 */
class TimeHeap
{
public:
	TimeHeap();
	/* 初始化一个大小为cap的空堆 */
	TimeHeap(int cap);
	/* 用一个已有数组来初始化堆 */
	TimeHeap(HeapTimer** initArray, int size, int capacity);
	/* 销毁时间堆 */
	~TimeHeap();
	/* 添加目标定时器 */
	void addTimer(HeapTimer* timer);
	/* 删除目标定时器 */
	void delTimer(HeapTimer* timer);
	/* 调整目标定时器的位置 */
	void adjustTimer(HeapTimer* timer);
	/* 获得堆顶部的定时器 */
	HeapTimer* top() const;
	/* 删除堆顶部的定时器 */
	void popTimer();
	/* 心搏函数 */
	void tick();
	/* 判断是否为空 */
	bool empty() const;
	/* 测试使用 */
	void printSelf();

private:
	/* 最小堆的下虑操作，它确保堆数组中第hole个节点作为根的子树具有最小堆的性质 */
	void percolateDown(int hole);
	/* 上虑操作 */
	void percolateUp(int hole);
	/* 将堆数组容量扩大一倍 */
	void resize();
	/* 交换堆中两个元素的位置 */
	void swapTimer(int a, int b);
	/* 测试使用 */
	void printTree(int hole);

private:
	HeapTimer** array;						// 堆数组
	int capacity;							// 堆数组的容量
	int curSize;							// 堆数组当前包含元素的个数
};