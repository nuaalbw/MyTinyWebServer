/*************************************************************************
	> File Name: main.cpp
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Fri 10 Sep 2021 03:13:02 PM CST
 ************************************************************************/

#include <iostream>
#include "Config.h"
using namespace std;

int main(int argc, char* argv[])
{
	/* 需要修改的数据库信息 */
	string dbUser = "root";
	string dbPassword = "Lbw_13999231708";
	string dbName = "tiny_web_server";

	/* 命令行解析 */
	Config config;
	config.parseArg(argc, argv);

	/* 服务器初始化 */
	WebServer server;
	/* 进入守护进程 */
	server.initDaemon();

	server.init(config.port, dbUser, dbPassword, dbName, config.logWrite, config.optLinger, config.triggerMode, 
				config.sqlNum, config.threadNum, config.closeLog, config.model);

	/* 日志 */
	server.logWriteInit();

	/* 数据库连接池 */
	server.connectionPoolInit();

	/* 线程池 */
	server.threadPoolInit();

	/* 触发模式 */
	server.trigMode();

	/* 设置监听 */
	server.eventListen();

	/* 运行 */
	server.eventLoop();

	return 0;
}