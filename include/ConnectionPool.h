/*************************************************************************
	> File Name: ConnectionPool.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: Tue 07 Sep 2021 06:57:05 PM CST
 ************************************************************************/

#ifndef _CONNECTION_POOL_H__
#define _CONNECTION_POOL_H__

#include <iostream>
#include <cstdio>
#include <mysql/mysql.h>
#include <list>
#include "Locker.h"
#include "Log.h"
using namespace std;

/* 单例模式实现的数据库连接池类 */
class ConnectionPool
{
public:
	/* 从连接池中获取一个数据库连接 */
	MYSQL* getConnection();
	/* 将一个数据库连接放回到连接池中 */
	bool releaseConnection(MYSQL* conn);
	/* 获取当前空闲连接数量 */
	int getFreeConnNumber();
	/* 销毁所有连接 */
	void destroyPool();
	/* 单例模式下获取全局唯一实例 */
	static ConnectionPool* getInstance();
	/* 初始化数据库连接池 */
	void initPool(string url, string user, string password, string dbName, int port, int maxConn, int closeLog);

public:
	/* 数据库所在主机地址*/
	string m_url;
	/* 数据库端口号 */
	string m_port;
	/* 数据库登录用户名 */
	string m_user;
	/* 登录数据库密码 */
	string m_password;
	/* 所用数据库的名称 */
	string m_dbName;
	/* 日志开关 */
	int m_closeLog;

private:
	/* 私有化构造和析构函数 */
	ConnectionPool();
	~ConnectionPool();
	/* 禁止对象的拷贝 */
	ConnectionPool(const ConnectionPool& pool) = delete;
	ConnectionPool& operator=(const ConnectionPool& pool) = delete;

private:
	/* 最大连接数 */
	int m_maxConn;
	/* 当前已经使用的连接数 */
	int m_curConn;
	/* 当前空闲的连接数 */
	int m_freeConn;
	/* 互斥锁 */
	Locker m_lock;
	/* 数据库连接池 */
	list<MYSQL*> m_connList;
	/* 信号量 */
	Sem m_reserve;
};

/* 利用RAII机制来自动管理内存 */
class ConnectionRAII
{
public:
	/* 从连接池中取出一个MYSQL连接 */
	ConnectionRAII(MYSQL** conn, ConnectionPool* connPool);
	/* 自动释放资源 */
	~ConnectionRAII();

private:
	MYSQL* m_connRAII;
	ConnectionPool* m_poolRAII;
};

#endif