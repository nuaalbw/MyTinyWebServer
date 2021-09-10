/*************************************************************************
	> File Name: Config.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Fri 10 Sep 2021 02:47:04 PM CST
 ************************************************************************/

#ifndef _CONFIG_H__
#define _CONFIG_H__

#include <iostream>
#include "WebServer.h"
using namespace std;

class Config
{
public:
	Config();

	void parseArg(int argc, char* argv[]);
	/* 端口号 */
	int port;
	/* 日志写入方式 */
	int logWrite;
	/* 触发组合模式 */
	int triggerMode;
	/* lfd触发模式 */
	TriggerMode lfdMode;
	/* cfd触发模式 */
	TriggerMode cfdMode;
	/* 优雅关闭连接 */
	int optLinger;
	/* 数据库连接池数量 */
	int sqlNum;
	/* 线程池内的线程数量 */
	int threadNum;
	/* 是否关闭日志 */
	int closeLog;
	/* 并发模型选择 */
	ActorModel model;
};

#endif