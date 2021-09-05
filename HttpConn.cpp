/*************************************************************************
	> File Name: HttpConn.cpp
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: 2021年07月24日 星期六 20时54分27秒
 ************************************************************************/

#include "HttpConn.h"
using namespace std;

/* 定义HTTP响应的一些状态信息 */
const char* OK_200_TITLE = "OK";
const char* ERROR_400_TITLE = "Bad Request";
const char* ERROR_400_FORM = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* ERROR_403_TITLE = "Forbidden";
const char* ERROR_403_FORM = "You do not have permission to get file from this server.\n";
const char* ERROR_404_TITLE = "Not Found";
const char* ERROR_404_FORM = "The requested file was not found on this server.\n";
const char* ERROR_500_TITLE = "Internal Error";
const char* ERROR_500_FORM = "There was an unusual problem serving the requested file.\n";
/* 网站的根目录 */
const char* DOC_ROOT = "/home/lbw/music";

/* 将文件描述符设置为非阻塞 */
static int setNonblocking(int fd)
{
	int oldOption = fcntl(fd, F_GETFL);
	int newOption = oldOption | O_NONBLOCK;
	fcntl(fd, F_SETFL, newOption);
	return oldOption;
}

/* 向epoll内核事件表中注册文件描述符事件 */
void addfd(int epollfd, int fd, bool oneShot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if (oneShot) {
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setNonblocking(fd);
}

/* 向epoll内核事件表中删除文件描述符所注册的事件 */
void removefd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
	close(fd);
}

static void modfd(int epollfd, int fd, int ev)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

/* 类的静态成员变量必须在类外初始化 */
int HttpConn::m_userCount = 0;
int HttpConn::m_epollfd = -1;

void HttpConn::closeConn(bool realClose)
{
	if (realClose && m_sockfd != -1) {
		removefd(m_epollfd, m_sockfd);
		m_sockfd = -1;
		m_userCount--;	/* 关闭一个连接时, 将客户总量减1 */
	}
}

void HttpConn::init(int sockfd, const sockaddr_in& addr)
{
	m_sockfd = sockfd;
	m_address = addr;
	/* 如下两行是为了避免TIME_WAIT状态，仅用于调试，实际使用时应去掉 */
	int reuse = 1;
	setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	addfd(m_epollfd, sockfd, true);
	m_userCount++;
	init();
}

void HttpConn::init()
{
	m_checkState = CHECK_STATE_REQUESTLINE;
	m_linger = false;
	m_method = GET;
	m_url = nullptr;
	m_version = nullptr;
	m_contentLength = 0;
	m_host = nullptr;
	m_startLine = 0;
	m_checkedIdx = 0;
	m_readIdx = 0;
	m_writeIdx = 0;
	m_bytesToSend = 0;
	m_bytesHaveSend = 0;
	bzero(m_readBuf, READ_BUFFER_SIZE);
	bzero(m_writeBuf, WRITE_BUFFER_SIZE);
	bzero(m_realFile, FILENAME_LEN);
}

/* 从状态机 */
HttpConn::LINE_STATUS HttpConn::parseLine()
{
	char temp;
	for (; m_checkedIdx < m_readIdx; ++m_checkedIdx) {
		temp = m_readBuf[m_checkedIdx];
		if (temp == '\r') {
			if (m_checkedIdx + 1 == m_readIdx) {
				return LINE_OPEN;
			}
			else if (m_readBuf[m_checkedIdx + 1] == '\n') {
				m_readBuf[m_checkedIdx++] = '\0';
				m_readBuf[m_checkedIdx++] = '\0';
				return LINE_OK;
			}
		}
		else if (temp == '\n') {
			if (m_checkedIdx > 1 && m_readBuf[m_checkedIdx - 1] == '\r') {
				m_readBuf[m_checkedIdx - 1] = '\0';
				m_readBuf[m_checkedIdx++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
	}
	return LINE_OPEN;
}

/* 循环读取客户数据, 直到无数据可读或者对方关闭连接 */
bool HttpConn::read()
{
	if (m_readIdx >= READ_BUFFER_SIZE) {
		return false;
	}
	int bytesRead = 0;
	while (true) {
		bytesRead = recv(m_sockfd, m_readBuf + m_readIdx, READ_BUFFER_SIZE - 
						m_readIdx, 0);
		if (bytesRead == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}
			return false;
		}
		else if (bytesRead == 0) {
			return false;
		}
		m_readIdx += bytesRead;
	}
	return true;
}

/* 解析HTTP请求行，获得请求方法、目标URL、以及HTTP版本号 */
HttpConn::HTTP_CODE HttpConn::parseRequestLine(char* text)
{
	m_url = strpbrk(text, " \t");
	if (m_url == nullptr) {
		return BAD_REQUEST;
	}
	*m_url++ = '\0';
	
	char* method = text;
	if (strcasecmp(method, "GET") == 0) {
		m_method = GET;
	}
	else {
		return BAD_REQUEST;
	}

	/* 跳过中间的空格和制表符 */
	m_url += strspn(m_url, " \t");
	m_version = strpbrk(m_url, " \t");
	if (m_version == nullptr) {
		return BAD_REQUEST;
	}
	*m_version++ = '\0';
	m_version += strspn(m_version, " \t");
	if (strcasecmp(m_version, "HTTP/1.1") != 0) {
		return BAD_REQUEST;
	}
	if (strncasecmp(m_url, "http://", 7) == 0) {
		m_url += 7;
		m_url = strchr(m_url, '/');
	}
	if (!m_url || m_url[0] != '/') {
		return BAD_REQUEST;
	}

	m_checkState = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

/* 解析HTTP请求的一个头部信息 */
HttpConn::HTTP_CODE HttpConn::parseHeaders(char* text)
{
	/* 遇到空行，表示头部字段解析完毕 */
	if (text[0] == '\0') {
		/* 如果HTTP请求还有请求体，则状态机转移到CHECK_STATE_CONTENT */	
		if (m_contentLength != 0) {
			m_checkState = CHECK_STATE_CONTENT;
			return NO_REQUEST;
		}
		/* 否则说明我们已经得到了一个完整的HTTP请求 */
		return GET_REQUEST;
	}
	/* 处理Connection头部字段 */
	else if (strncasecmp(text, "Connection:", 11) == 0) {
		text += 11;
		text += strspn(text, " \t");
		if (strcasecmp(text, "keep-alive") == 0) {
			m_linger = true;
		}
	}
	/* 处理Content-Length头部字段 */
	else if (strncasecmp(text, "Content-Length:", 15) == 0) {
		text += 15;
		text += strspn(text, " \t");
		m_contentLength = atoi(text);
	}
	/* 处理HOST头部字段 */
	else if (strncasecmp(text, "Host:", 5) == 0) {
		text += 5;
		text += strspn(text, " \t");
		m_host = text;
	}
	else {
		// printf("oop! unknown header %s\n", text);
	}
	return NO_REQUEST;
}

/* 我们没有真正解析HTTP请求的消息体，只是判断它是否被完整地读入了 */
HttpConn::HTTP_CODE HttpConn::parseContent(char* text)
{
	if (m_readIdx >= (m_contentLength + m_checkedIdx)) {
		text[m_contentLength] = '\0';
		return GET_REQUEST;
	}
	return NO_REQUEST;
}

/* 主状态机 */
HttpConn::HTTP_CODE HttpConn::processRead()
{
	LINE_STATUS lineStatus = LINE_OK;
	HTTP_CODE ret = NO_REQUEST;
	char* text;

	while ((m_checkState == CHECK_STATE_CONTENT && lineStatus == LINE_OK) 
		 || (lineStatus = parseLine()) == LINE_OK) {
		text = getLine();
		m_startLine = m_checkedIdx;
		printf("got 1 http line: %s\n", text);

		switch (m_checkState) {
			case CHECK_STATE_REQUESTLINE:
			{
				ret = parseRequestLine(text);
				if (ret == BAD_REQUEST) {
					return BAD_REQUEST;
				}
				break;
			}
			case CHECK_STATE_HEADER:
			{
				ret = parseHeaders(text);
				if (ret == BAD_REQUEST) {
					return BAD_REQUEST;
				}
				else if (ret == GET_REQUEST) {
					return doRequest();
				}
				break;
			}
			case CHECK_STATE_CONTENT:
			{
				ret = parseContent(text);
				if (ret == GET_REQUEST) {
					return doRequest();
				}
				lineStatus = LINE_OPEN;
				break;
			}
			default:
			{
				return INTERNAL_ERROR;
			}
		}
	}
	return NO_REQUEST;
}

/* 当得到一个完整、正确的HTTP请求时，我们就分析目标文件的属性。如果目标文件存在
 * 且对所有用户可读，则使用mmap将其映射到内存地址m_fileAddress处，并返回成功 */
HttpConn::HTTP_CODE HttpConn::doRequest()
{
	strcpy(m_realFile, DOC_ROOT);
	int len = strlen(DOC_ROOT);
	strncpy(m_realFile + len, m_url, FILENAME_LEN - len - 1);
	if (stat(m_realFile, &m_fileStat) < 0) {
		return NO_RESOURCE;
	}

	if (!(m_fileStat.st_mode & S_IROTH)) {
		return FORBIDDEN_REQUEST;
	}

	if (S_ISDIR(m_fileStat.st_mode)) {
		return BAD_REQUEST;
	}

	int fd = open(m_realFile, O_RDONLY);
	m_fileAddress = (char*)mmap(0, m_fileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	return FILE_REQUEST;
}

/* 对内存映射区执行munmap操作 */
void HttpConn::unmap()
{
	if (m_fileAddress) {
		munmap(m_fileAddress, m_fileStat.st_size);
		m_fileAddress = nullptr;
	}
}

/* 写HTTP响应 */
bool HttpConn::write()
{
	int temp = 0;
	if (m_bytesToSend == 0) {
		modfd(m_epollfd, m_sockfd, EPOLLIN);
		init();
		return true;
	}
	
	while (true) {
		temp = writev(m_sockfd, m_iv, m_ivCount);
		if (temp <= -1) {
			/* 如果TCP写缓冲没有空间，则等待下一轮EPOLLOUT事件 */
			if (errno == EAGAIN) {
				modfd(m_epollfd, m_sockfd, EPOLLOUT);
				return true;
			}
			unmap();
			return false;
		}

		m_bytesToSend -= temp;
		m_bytesHaveSend += temp;
		/* 解决大文件的传输问题 */
		if (m_bytesHaveSend >= m_iv[0].iov_len) {
			m_iv[0].iov_len = 0;
			m_iv[1].iov_base = m_fileAddress + (m_bytesHaveSend - m_writeIdx);
			m_iv[1].iov_len = m_bytesToSend;
		}
		else {
			m_iv[0].iov_base = m_writeBuf + m_bytesHaveSend;
			m_iv[0].iov_len = m_iv[0].iov_len - m_bytesHaveSend;
		}

		/* 发送HTTP响应成功，根据HTTP请求中的Connection字段决定是否立即关闭连接 */
		if (m_bytesToSend <= 0) {
			unmap();
			modfd(m_epollfd, m_sockfd, EPOLLIN);
			if (m_linger) {
				init();
				return true;
			}
			else {
				return false;
			}
		}
	}
}

/* 往写缓冲区中写入待发送的数据 */
bool HttpConn::addResponse(const char* format, ...)
{
	if (m_writeIdx >= WRITE_BUFFER_SIZE) {
		return false;
	}
	va_list argList;
	va_start(argList, format);
	int len = vsnprintf(m_writeBuf + m_writeIdx, WRITE_BUFFER_SIZE - 1 - m_writeIdx,
				        format, argList);
	if (len >= WRITE_BUFFER_SIZE - 1 - m_writeIdx) {
		return false;
	}
	m_writeIdx += len;
	va_end(argList);
	return true;
}

bool HttpConn::addStatusLine(int status, const char* title)
{
	return addResponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HttpConn::addHeaders(int contentLength)
{
	addContentLength(contentLength);
	addLinger();
	addBlankLine();
	return true;
}

bool HttpConn::addContentLength(int contentLength)
{
	return addResponse("Content-Length: %d\r\n", contentLength);
}

bool HttpConn::addLinger()
{
	return addResponse("Connection: %s\r\n", (m_linger == true) ? "keep-alive" : "close");
}

bool HttpConn::addBlankLine()
{
	return addResponse("%s", "\r\n");
}

bool HttpConn::addContent(const char* content)
{
	return addResponse("%s", content);
}

/* 根据服务器处理HTTP请求的结果，决定返回给客户端的内容 */
bool HttpConn::processWrite(HTTP_CODE ret)
{
	switch (ret) {
		case INTERNAL_ERROR:
		{
			addStatusLine(500, ERROR_500_TITLE);
			addHeaders(strlen(ERROR_500_FORM));
			if (!addContent(ERROR_500_FORM)) {
				return false;
			}
			break;
		}
		case BAD_REQUEST:
		{
			addStatusLine(400, ERROR_400_TITLE);
			addHeaders(strlen(ERROR_400_FORM));
			if (!addContent(ERROR_400_FORM)) {
				return false;
			}
			break;
		}
		case NO_RESOURCE:
		{
			addStatusLine(404, ERROR_404_TITLE);
			addHeaders(strlen(ERROR_404_FORM));
			if (!addContent(ERROR_404_FORM)) {
				return false;
			}
			break;
		}
		case FORBIDDEN_REQUEST:
		{
			addStatusLine(403, ERROR_403_TITLE);
			addHeaders(strlen(ERROR_403_FORM));
			if (!addContent(ERROR_403_FORM)) {
				return false;
			}
			break;
		}
		case FILE_REQUEST:
		{
			addStatusLine(200, OK_200_TITLE);
			if (m_fileStat.st_size != 0) {
				addHeaders(m_fileStat.st_size);
				m_iv[0].iov_base = m_writeBuf;
				m_iv[0].iov_len = m_writeIdx;
				m_iv[1].iov_base = m_fileAddress;
				m_iv[1].iov_len = m_fileStat.st_size;
				m_ivCount = 2;
				m_bytesToSend = m_writeIdx + m_fileStat.st_size;
				return true;
			}
			else {
				const char* OK_STRING = "<html><body></body></html>";
				addHeaders(strlen(OK_STRING));
				if (!addContent(OK_STRING)) {
					return false;
				}
			}
		}
		default:
		{
			return false;
		}
	}
	m_iv[0].iov_base = m_writeBuf;
	m_iv[0].iov_len = m_writeIdx;
	m_ivCount = 1;
	m_bytesToSend = m_writeIdx;
	return true;
}

/* 由线程池中的工作线程调用，这是处理HTTP请求的入口函数 */
void HttpConn::process()
{
	HTTP_CODE readRet = processRead();
	if (readRet == NO_REQUEST) {
		modfd(m_epollfd, m_sockfd, EPOLLIN);
		return;
	}
	bool writeRet = processWrite(readRet);
	if (!writeRet) {
		closeConn();
	}
	modfd(m_epollfd, m_sockfd, EPOLLOUT);
}
