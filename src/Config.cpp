/*************************************************************************
	> File Name: Config.cpp
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Fri 10 Sep 2021 02:53:00 PM CST
 ************************************************************************/

#include <iostream>
#include "Config.h"
using namespace std;

Config::Config()
{
	/* 端口号，默认8888 */
	port = 8888;
	/* 日志写入方式，默认同步 */
	logWrite = 0;
	/* 触发组合模式，默认listenfd ET + connfd ET */
	triggerMode = 3;
	/* listenfd 触发模式，默认为LT */
	lfdMode = EPOLL_LT;
	/* connfd 触发模式，默认为LT */
	cfdMode = EPOLL_LT;
	/* 优雅关闭连接，默认不使用 */
	optLinger = 0;
	/* 数据库连接池数量，默认为8 */
	sqlNum = 8;
	/* 线程池内的线程数量，默认为8 */
	threadNum = 8;
	/* 关闭日志，默认关闭 */
	closeLog = 1;
	/* 服务器并发模型，默认为proactor */
	model = PROACTOR;
}

void Config::parseArg(int argc, char* argv[])
{
	int opt;
	const char* str = "p:l:m:o:s:t:c:a:";
	while ((opt = getopt(argc, argv, str)) != -1) {
		switch (opt)
		{
		case 'p':
			port = atoi(optarg);
			break;
		case 'l':
			logWrite = atoi(optarg);
			break;
		case 'm':
			triggerMode = atoi(optarg);
			break;
		case 'o':
			optLinger = atoi(optarg);
			break;
		case 's':
			sqlNum = atoi(optarg);
			break;
		case 't':
			threadNum = atoi(optarg);
			break;
		case 'c':
			closeLog = atoi(optarg);
			break;
		case 'a':
		{
			int tmp = atoi(optarg);	
			model = tmp == 0 ? PROACTOR : REACTOR;
			break;
		}
		default:
			break;
		}
	}
}