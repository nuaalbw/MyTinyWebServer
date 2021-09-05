/*************************************************************************
	> File Name: unp.h
	> Author: lbw
	> Mail: 296273803@qq.com 
	> Created Time: 2021年07月03日 星期六 17时44分37秒
 ************************************************************************/

#ifndef _UNP_H__
#define _UNP_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>

typedef void Sigfunc(int);

#define SERV_PORT	"9999"
#define LISTENQ		20	
#define MAXLINE		1024

#ifdef __cplusplus
extern "C" {
#endif

ssize_t readline(int fd, void* vptr, size_t maxlen);

ssize_t readlinebuf(void** vptrptr);

ssize_t readn(int fd, void* vptr, size_t n);

ssize_t writen(int fd, const void* vptr, size_t n);

void sys_error(const char* str);

Sigfunc* Signal(int signo, Sigfunc* func);

#ifdef __cplusplus
}
#endif

#endif
