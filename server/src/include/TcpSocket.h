﻿#pragma once
// #include "ItcastLog.h"
#include "logger.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

/* 用于通信的套接字类 */
// 超时的时间
static const int TIMEOUT = 1000;
class TcpSocket
{
public:
	enum ErrorType {ParamError = 3001, TimeoutError, PeerCloseError, MallocError};
	TcpSocket();
	// 使用一个可以用于通信的套接字实例化套接字对象
	TcpSocket(int connfd);
	TcpSocket(const TcpSocket* socket);
	~TcpSocket();

	// 连接服务器
	int connectToHost(char* ip, unsigned short port, int timeout = TIMEOUT);
	// 发送数据
	int sendMsg(const char* sendData, int dataLen, int timeout = TIMEOUT);
	// 接收数据
	int recvMsg(char** recvData, int *recvLen, int timeout = TIMEOUT);
	// 断开连接
	void disConnect();
	// 释放内存
	void freeMemory(char** buf);
	// 设置I/O为非阻塞模式
	int blockIO();
	// 设置I/O为阻塞模式
	int noBlockIO();
	// 添加到epoll
	void addEpollTree(int ep_fd, bool oneshot = true);
	// 访问fd
	int getfd();

private:
	// // 设置I/O为非阻塞模式
	// int blockIO(int fd);
	// // 设置I/O为阻塞模式
	// int noBlockIO(int fd);


	// 读超时检测函数，不含读操作
	int readTimeout(unsigned int wait_seconds);
	// 写超时检测函数, 不包含写操作
	int writeTimeout(unsigned int wait_seconds);
	// 带连接超时的connect函数
	int connectTimeout(struct sockaddr_in *addr, unsigned int wait_seconds);
	// 每次从缓冲区中读取n个字符
	int readn(void *buf, int count);
	// 每次往缓冲区写入n个字符
	int writen(const void *buf, int count);

private:
	int m_socket;		// 用于通信的套接字
	// ItcastLog m_log;	// log对象
};

