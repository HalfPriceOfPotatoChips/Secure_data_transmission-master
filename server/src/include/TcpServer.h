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


class TcpServer
{
public:
	TcpServer();
	~TcpServer();

	int getfg();
	// 服务器设置监听
	int setListen(unsigned short port);
	// 等待并接受客户端连接请求
	int acceptConn(TcpSocket** socket);

	void closefd();


private:
	int m_lfd;	// 用于监听的文件描述符
	struct sockaddr_in m_addrCli;
	// ItcastLog m_log;
};

