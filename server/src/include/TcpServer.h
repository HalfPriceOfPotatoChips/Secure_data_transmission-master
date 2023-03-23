#pragma once
#include <netinet/in.h>
#include <arpa/inet.h>
// #include "ItcastLog.h"
#include "logger.h"
#include "TcpSocket.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// 超时的时间
// static const int TIMEOUT = 10000;

class TcpServer
{
public:
	TcpServer();
	~TcpServer();

	int getfg();
	// 服务器设置监听
	int setListen(unsigned short port);
	// 等待并接受客户端连接请求, 默认连接超时时间为10000s
	int acceptConn(TcpSocket** socket);
	// // 设置I/O为非阻塞模式
	// int blockIO();
	// // 设置I/O为阻塞模式
	// int noBlockIO();
	void closefd();

// private:
// 	int acceptTimeout(int wait_seconds);

private:
	int m_lfd;	// 用于监听的文件描述符
	struct sockaddr_in m_addrCli;
	// ItcastLog m_log;
};

