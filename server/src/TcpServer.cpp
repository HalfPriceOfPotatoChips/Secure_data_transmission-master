﻿#include "TcpServer.h"
#include <iostream>

TcpServer::TcpServer()
{
}


TcpServer::~TcpServer()
{
}

int TcpServer::getfg()
{
	return m_lfd;
}

int TcpServer::setListen(unsigned short port)
{
	int 	ret = 0;
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 监听 socket 设置为非阻塞
	m_lfd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (m_lfd < 0)
	{
		ret = errno;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func socket() err");
		LOG_ERR("file:%s  line:%d  func socket() err %d", __FILE__, __LINE__, ret);
		return ret;
	}

	int on = 1;
	ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (ret < 0)
	{
		ret = errno;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func setsockopt() err");
		LOG_ERR("file:%s  line:%d  func setsockopt() err %d", __FILE__, __LINE__, ret);
		return ret;
	}


	ret = bind(m_lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (ret < 0)
	{
		ret = errno;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func bind() err");
		LOG_ERR("file:%s  line:%d  func bind() err %d", __FILE__, __LINE__, ret);
		return ret;
	}

	ret = listen(m_lfd, SOMAXCONN);
	if (ret < 0)
	{
		ret = errno;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func listen() err");
		LOG_ERR("file:%s  line:%d  func listen() err %d", __FILE__, __LINE__, ret);
		return ret;
	}

	return ret;
}

int TcpServer::acceptConn(TcpSocket** socket)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	while(true){
		ret = accept(m_lfd, (sockaddr *)&m_addrCli, &addrlen);
		if(ret == -1){
			if(errno == EINTR) continue;        //被中断 再次连接
			else if (errno == EAGAIN){
				return EAGAIN;
			}
			else{
				// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "accept() err");
				LOG_ERR("file:%s  line:%d  func accept() err %d", __FILE__, __LINE__, ret);
				std::cout << "客户端连接失败" << std::endl;
			}
		}else{
			*socket = new TcpSocket(ret);
			break;
		} 
	}
	return 0;
}

void TcpServer::closefd()
{
	close(m_lfd);
}
