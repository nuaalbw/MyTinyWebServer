/*************************************************************************
	> File Name: Log.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Sun 05 Sep 2021 07:22:33 PM CST
 ************************************************************************/

#ifndef _LOG_H__
#define _LOG_H__

#include <iostream>
#include <cstdio>
#include <cstdarg>
#include "BlockQueue.h"
using namespace std;

/* 单例模式实现的日志类 */
class Log
{
public:
	/* 日志打印级别 */
	enum Level { DEBUG = 1, INFO, WARN, ERROR };
	/* 获取该类的全局唯一实例 */
	static Log* getInstance();
	/* 工作线程的回调函数 */
	static void* flushLogThread(void* arg);
	/* 可选择的参数：日志文件、日志缓冲区大小、最大行数以及最长日志条队列 */
	bool init(const char* fileName, int closeLog, int logBufSize = 8192, int splitLines = 5000000, int maxQueueSize = 0);
	/* 以指定格式打印日志 */
	void writeLog(Level level, const char* format, ...);
	/* 强制刷新到磁盘 */
	void flush(void);

private:
	/* 私有化构造和析构函数 */
	Log();
	virtual ~Log();
	/* 防止对象的拷贝 */
	Log(const Log& log) = delete;
	Log& operator=(const Log& log) = delete;
	/* 异步写日志 */
	void* asyncWriteLog();

private:
	/* 日志所在文件夹路径名 */
	char m_dirName[128];
	/* 日志文件名 */
	char m_logName[128];
	/* 日志最大行数 */
	int m_spiltLines;
	/* 日志缓冲区大小 */
	int m_logBufSize;
	/* 日志行数记录 */
	long long m_count;
	/* 日志按天进行打印，用来记录当前是哪一天 */
	int m_today;
	/* 指向日志文件的文件指针 */
	FILE* m_fp;
	/* 日志缓冲区首地址 */
	char* m_buf;
	/* 线程安全的阻塞队列 */
	BlockQueue<string>* m_logQueue;
	/* 用来标志是否同步*/	
	bool m_isAsync;
	/* 互斥锁 */
	Locker m_mutex;
	/* 用来标志是否关闭日志 */
	int m_closeLog;
};

#define LOG_DEBUG(format, ...) 	if (m_closeLog == 0) { Log::getInstance()->writeLog(Log::DEBUG, format, ##__VA_ARGS__);	Log::getInstance()->flush(); }
#define LOG_INFO(format, ...) 	if (m_closeLog == 0) { Log::getInstance()->writeLog(Log::INFO, format, ##__VA_ARGS__); 	Log::getInstance()->flush(); }
#define LOG_WARN(format, ...) 	if (m_closeLog == 0) { Log::getInstance()->writeLog(Log::WARN, format, ##__VA_ARGS__); 	Log::getInstance()->flush(); }
#define LOG_ERROR(format, ...) 	if (m_closeLog == 0) { Log::getInstance()->writeLog(Log::ERROR, format, ##__VA_ARGS__); Log::getInstance()->flush(); }

#endif