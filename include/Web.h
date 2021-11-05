/*************************************************************************
	> File Name: Web.h
	> Author: lbw
	> Mail: nuaalbw@163.com 
	> Created Time: 2021年07月03日 星期六 17时44分37秒
 ************************************************************************/

#ifndef _WEB_H__
#define _WEB_H__

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
#include <sys/param.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <dirent.h>

enum TriggerMode { EPOLL_LT = 1, EPOLL_ET };
enum ActorModel { REACTOR = 1, PROACTOR };

#endif
