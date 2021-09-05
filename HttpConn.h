/*************************************************************************
	> File Name: HttpConn.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: 2021年07月24日 星期六 18时57分29秒
 ************************************************************************/

#ifndef _HTTP_CONN_H__
#define _HTTP_CONN_H__

#include "unp.h"
#include "Locker.h"

class HttpConn
{
public:
	/* 文件名的最大长度 */
	static const int FILENAME_LEN = 200;
	/* 读缓冲区的大小 */
	static const int READ_BUFFER_SIZE = 2048;
	/* 写缓冲区的大小 */
	static const int WRITE_BUFFER_SIZE = 1024;
	/* HTTP请求方法, 但我们只支持GET */
	enum METHOD { GET = 0, POST, HEAD, PUT, DELETE, 
				  TRACE, OPTIONS, CONNECT, PATCH };
	/* 解析客户请求时，主状态机所处的状态 */
	enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, 
					   CHECK_STATE_HEADER, 
					   CHECK_STATE_CONTENT };
	/* 服务器处理HTTP请求的可能结果 */
	enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, 
					 NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, 
					 INTERNAL_ERROR, CLOSED_CONNECTION };
	/* 行的读取状态 */
	enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

public:
	HttpConn(){};
	~HttpConn(){};

public:
	/* 初始化新接受的连接 */
	void init(int sockfd, const sockaddr_in& addr);
	/* 关闭连接 */
	void closeConn(bool realClose = true);
	/* 处理客户请求 */
	void process();
	/* 非阻塞读操作 */
	bool read();
	/* 非阻塞写操作 */
	bool write();

private:
	/* 初始化连接 */
	void init();
	/* 解析HTTP请求 */
	HTTP_CODE processRead();
	/* 填充HTTP应答 */
	bool processWrite(HTTP_CODE ret);

	/* 下面这一组函数被processRead调用以分析HTTP请求 */
	HTTP_CODE parseRequestLine(char* text);
	HTTP_CODE parseHeaders(char* text);
	HTTP_CODE parseContent(char* text);
	HTTP_CODE doRequest();
	char* getLine() { return m_readBuf + m_startLine; }
	LINE_STATUS parseLine();

	/* 下面这一组函数被processWrite调用以填充HTTP应答 */
	void unmap();
	bool addResponse(const char* format, ...);
	bool addContent(const char* content);
	bool addStatusLine(int status, const char* title);
	bool addHeaders(int contentLength);
	bool addContentLength(int contentLength);
	bool addLinger();
	bool addBlankLine();

public:
	/* 所有socket上的事件都被注册到同一个epoll内核事件表中 */
	static int m_epollfd;
	/* 统计用户数量 */
	static int m_userCount;

private:
	/* 连接套接字 */
	int m_sockfd;
	/* 对方的socket地址 */
	sockaddr_in m_address;
	/* 读缓冲区 */
	char m_readBuf[READ_BUFFER_SIZE];
	/* 标识读缓冲区中已经读入的客户数据的最后一个字节的下一个位置 */
	int m_readIdx;
	/* 当前正在分析的字符在读缓冲区中的位置 */
	int m_checkedIdx;
	/* 当前正在解析的行的起始位置 */
	int m_startLine;
	/* 写缓冲区 */
	char m_writeBuf[WRITE_BUFFER_SIZE];
	/* 写缓冲区中待发送的字节数 */
	int m_writeIdx;

	/* 主状态机当前所处的位置 */ 
	CHECK_STATE m_checkState;
	/* 请求方法 */
	METHOD m_method;

	/* 客户请求的目标文件的完整路径, 相当于网站根目录 + m_url */
	char m_realFile[FILENAME_LEN];
	/* 客户请求的目标文件的文件名 */
	char* m_url;
	/* HTTP协议版本号, 我们仅支持HTTP/1.1 */
	char* m_version;
	/* 主机名 */
	char* m_host;
	/* HTTP请求的消息体的长度 */
	int m_contentLength;
	/* HTTP请求是否要求保持连接 */
	bool m_linger;

	/* 客户请求的目标文件被mmap到内存中的起始位置 */
	char* m_fileAddress;
	/* 目标文件的状态(文件是否存在, 是否为目录, 是否可读等) */
	struct stat m_fileStat;
	/* 采用writev来执行写操作 */
	struct iovec m_iv[2];
	/* 被写内存块的数量 */
	int m_ivCount;
	/* 将要从缓冲区中发送的字节数 */
	size_t m_bytesToSend;
	/* 已经从缓冲区中发送的字节数 */
	size_t m_bytesHaveSend;
};

#endif
