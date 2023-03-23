#include "TcpSocket.h"


TcpSocket::TcpSocket()
{
}


TcpSocket::TcpSocket(int connfd)
{
	m_socket = connfd;
}

TcpSocket::TcpSocket(const TcpSocket* socket){
	m_socket = socket->m_socket;
	// m_log = socket->m_log;
}

TcpSocket::~TcpSocket()
{
	disConnect();
}

int TcpSocket::connectToHost(char * ip, unsigned short port, int timeout)
{
	int ret = 0;
	if (ip == NULL || port <= 0 || port > 65535 || timeout < 0)
	{
		ret = ParamError;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func sckClient_connect() err, check  (ip==NULL || connfd==NULL || port<=0 || port>65537 || connecttime < 0)");
		LOG_ERR("file:%s  line:%d  sckClient_connect() err %d", __FILE__, __LINE__, ret);
		return ret;
	}

	m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket < 0)
	{
		ret = errno;
		printf("func socket() err:  %d\n", ret);
		return ret;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip);

	ret = connectTimeout((struct sockaddr_in*) (&servaddr), (unsigned int)timeout);
	if (ret < 0)
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = TimeoutError;
			return ret;
		}
		else
		{
			//printf("func connect_timeout() err:  %d\n", ret);
			// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func connect_timeout() err");
			LOG_ERR("file:%s  line:%d  connect_timeout() err %d", __FILE__, __LINE__, ret);
		}
	}

	return ret;
}

int TcpSocket::sendMsg(const char * sendData, int dataLen, int timeout)
{
	int 	ret = 0;

	if (sendData == NULL || dataLen <= 0)
	{
		ret = ParamError;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func sckClient_send() err, check (data == NULL || datalen <= 0)");
		LOG_ERR("file:%s  line:%d  func sckClient_send() err, check (data == NULL || datalen <= 0) %d", __FILE__, __LINE__, ret);
		return ret;
	}

	ret = writeTimeout(timeout);
	if (ret == 0)
	{
		int writed = 0;
		unsigned char *netdata = (unsigned char *)malloc(dataLen + 4);
		if (netdata == NULL)
		{
			ret = MallocError;
			printf("func sckClient_send() mlloc Err:%d\n ", ret);
			return ret;
		}
		int netlen = htonl(dataLen);
		memcpy(netdata, &netlen, 4);
		memcpy(netdata + 4, sendData, dataLen);

		writed = writen(netdata, dataLen + 4);
		if (writed < (dataLen + 4))
		{
			if (netdata != NULL)
			{
				free(netdata);
				netdata = NULL;
			}
			return writed;
		}

		if (netdata != NULL)  //wangbaoming 20150630 modify bug
		{
			free(netdata);
			netdata = NULL;
		}
	}

	if (ret < 0)
	{
		//失败返回-1，超时返回-1并且errno = ETIMEDOUT
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = TimeoutError;
			printf("func sckClient_send() mlloc Err:%d\n ", ret);
			return ret;
		}
		return ret;
	}

	return ret;
}

int TcpSocket::recvMsg(char ** recvData, int *recvLen, int timeout)
{
	int		ret = 0;

	if (recvData == NULL || recvLen == NULL)
	{
		ret = ParamError;
		printf("func sckClient_rev() timeout , err:%d \n", TimeoutError);
		return ret;
	}

	/*使用epoll检测读事件，故不需要select检测是否可读*/
	// ret = readTimeout(timeout); //bugs modify bombing
	// if (ret != 0)
	// {
	// 	if (ret == -1 || errno == ETIMEDOUT)
	// 	{
	// 		ret = TimeoutError;
	// 		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func read_timeout() timeout");
	// 		return ret;
	// 	}
	// 	else
	// 	{
	// 		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func read_timeout() err");
	// 		return ret;
	// 	}
	// }

	int netdatalen = 0;
	ret = readn(&netdatalen, 4); //读包头 4个字节
	if (ret == 0)
	{
		std::cout << "全部请求以读出" << std::endl;
		*recvLen = 0;
		*recvData = nullptr;
		return 0;
	}
	else if (ret == -1)
	{
		//printf("func readn() err:%d \n", ret);
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func readn() err");
		LOG_ERR("file:%s  line:%d  func readn() err %d", __FILE__, __LINE__, ret);
		return ret;
	}
	else if (ret < 4)
	{
		ret = PeerCloseError;
		//printf("func readn() err peer closed:%d \n", ret);
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func readn() err, peer closed");
		LOG_ERR("file:%s  line:%d  func readn() err, peer closed %d", __FILE__, __LINE__, ret);
		return ret;
	}

	int n;
	n = ntohl(netdatalen);
	char* tmpBuf = (char *)malloc(n + 1);
	if (tmpBuf == NULL)
	{
		ret = MallocError;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "malloc() err");
		LOG_ERR("file:%s  line:%d  malloc() err %d", __FILE__, __LINE__, ret);
		return ret;
	}


	ret = readn(tmpBuf, n); //根据长度读数据
	if (ret == -1)
	{
		//printf("func readn() err:%d \n", ret);
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "readn() err");
		LOG_ERR("file:%s  line:%d  readn() err %d", __FILE__, __LINE__, ret);
		return ret;
	}
	else if (ret < n)
	{
		ret = PeerCloseError;
		//printf("func readn() err peer closed:%d \n", ret);
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func readn() err,  peer closed");
		LOG_ERR("file:%s  line:%d  func readn() err,  peer closed %d", __FILE__, __LINE__, ret);
		return ret;
	}

	*recvData = tmpBuf;
	*recvLen = n;
	tmpBuf[n] = '\0'; //多分配一个字节内容，兼容可见字符串 字符串的真实长度仍然为n

	return 0;
}

void TcpSocket::disConnect()
{
	if (m_socket >= 0)
	{
		close(m_socket);
	}
}

void TcpSocket::freeMemory(char ** buf)
{
	if (*buf != NULL)
	{
		free(*buf);
		*buf = NULL;
	}
}

/////////////////////////////////////////////////
//////             子函数                   //////
/////////////////////////////////////////////////
/*
* noBlockIO - 设置I/O为非阻塞模式
* @fd: 文件描符符
*/
int TcpSocket::noBlockIO()
{
	int ret = 0;
	int flags = fcntl(m_socket, F_GETFL);
	if (flags == -1)
	{
		ret = flags;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func activate_nonblock() err");
		LOG_ERR("file:%s  line:%d  func activate_nonblock() err %d", __FILE__, __LINE__, ret);
		return ret;
	}

	flags |= O_NONBLOCK;
	ret = fcntl(m_socket, F_SETFL, flags);
	if (ret == -1)
	{
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func activate_nonblock() err");
		LOG_ERR("file:%s  line:%d  func activate_nonblock() err %d", __FILE__, __LINE__, ret);
		return ret;
	}
	return ret;
}

void TcpSocket::addEpollTree(int ep_fd){
	epoll_event event;
	event.data.fd = m_socket;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	event.events |= EPOLLONESHOT;
	epoll_ctl(ep_fd, EPOLL_CTL_ADD, m_socket, &event);
}

int TcpSocket::getfd(){
	return m_socket;
}

/*
* blockIO - 设置I/O为阻塞模式
* @fd: 文件描符符
*/
int TcpSocket::blockIO()
{
	int ret = 0;
	int flags = fcntl(m_socket, F_GETFL);
	if (flags == -1)
	{
		ret = flags;
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func deactivate_nonblock() err");
		LOG_ERR("file:%s  line:%d  func deactivate_nonblock() err %d", __FILE__, __LINE__, ret);
		return ret;
	}

	flags &= ~O_NONBLOCK;
	ret = fcntl(m_socket, F_SETFL, flags);
	if (ret == -1)
	{
		// m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func deactivate_nonblock() err");
		LOG_ERR("file:%s  line:%d  func deactivate_nonblock() err %d", __FILE__, __LINE__, ret);
		return ret;
	}
	return ret;
}

/*
* readTimeout - 读超时检测函数，不含读操作
* @wait_seconds: 等待超时秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::readTimeout(unsigned int wait_seconds)
{
	int ret = 0;
	if (wait_seconds > 0)
	{
		fd_set read_fdset;
		struct timeval timeout;

		FD_ZERO(&read_fdset);
		FD_SET(m_socket, &read_fdset);

		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		//select返回值三态
		//1 若timeout时间到（超时），没有检测到读事件 ret返回=0
		//2 若ret返回<0 &&  errno == EINTR 说明select的过程中被别的信号中断（可中断睡眠原理）
		//2-1 若返回-1，select出错
		//3 若ret返回值>0 表示有read事件发生，返回事件发生的个数

		do
		{
			ret = select(m_socket + 1, &read_fdset, NULL, NULL, &timeout);

		} while (ret < 0 && errno == EINTR);

		if (ret == 0)
		{
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if (ret == 1)
			ret = 0;
	}

	return ret;
}

/*
* writeTimeout - 写超时检测函数，不含写操作
* @wait_seconds: 等待超时秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::writeTimeout(unsigned int wait_seconds)
{
	int ret = 0;
	if (wait_seconds > 0)
	{
		fd_set write_fdset;
		struct timeval timeout;

		FD_ZERO(&write_fdset);
		FD_SET(m_socket, &write_fdset);

		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(m_socket + 1, NULL, &write_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);

		if (ret == 0)
		{
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if (ret == 1)
			ret = 0;
	}

	return ret;
}

/*
* connectTimeout - connect
* @addr: 要连接的对方地址
* @wait_seconds: 等待超时秒数，如果为0表示正常模式
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::connectTimeout(sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
		blockIO();

	ret = connect(m_socket, (struct sockaddr*)addr, addrlen);
	if (ret < 0 && errno == EINPROGRESS)
	{
		//printf("11111111111111111111\n");
		fd_set connect_fdset;
		struct timeval timeout;
		FD_ZERO(&connect_fdset);
		FD_SET(m_socket, &connect_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			// 一但连接建立，则套接字就可写  所以connect_fdset放在了写集合中
			ret = select(m_socket + 1, NULL, &connect_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);
		if (ret == 0) //连接时，fd不可写        连接超时
		{
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if (ret < 0)
			return -1;
		else if (ret == 1)
		{
			//printf("22222222222222222\n");
			/* ret返回为1（表示套接字可写），可能有两种情况，一种是连接建立成功，一种是套接字产生错误，*/
			/* 此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取。 */
			int err;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &err, &socklen);
			if (sockoptret == -1)
			{
				return -1;
			}
			if (err == 0)
			{
				//printf("3333333333333\n");
				ret = 0;
			}
			else
			{
				//printf("4444444444444444:%d\n", err);
				errno = err;
				ret = -1;
			}
		}
	}
	if (wait_seconds > 0)
	{
		noBlockIO();
	}
	return ret;
}

/*
* readn - 读取固定字节数
* @fd: 文件描述符
* @buf: 接收缓冲区
* @count: 要读取的字节数
* 成功返回count，失败返回-1，读到EOF返回<count
*/
int TcpSocket::readn(void *buf, int count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char*)buf;

	while (nleft > 0)
	{
		if ((nread = read(m_socket, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nread == 0)
			return count - nleft;

		bufp += nread;
		nleft -= nread;
	}

	return count;
}

/*
* writen - 发送固定字节数
* @buf: 发送缓冲区
* @count: 要读取的字节数
* 成功返回count，失败返回-1
*/
int TcpSocket::writen(const void *buf, int count)
{
	size_t nleft = count;
	ssize_t nwritten;
	char *bufp = (char*)buf;

	while (nleft > 0)
	{
		if ((nwritten = write(m_socket, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nwritten == 0)
			continue;

		bufp += nwritten;
		nleft -= nwritten;
	}

	return count;
}
