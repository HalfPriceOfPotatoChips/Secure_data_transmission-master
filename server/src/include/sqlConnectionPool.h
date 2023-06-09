#ifndef SQLCONNECTIONPOOL_H
#define SQLCONNECTIONPOOL_H


#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include "MySQLOP.h"

const int MAX_CONN_SIZE = 100;

class connection_pool
{
public:
	MySQLOP *GetConnection();				 	//获取数据库连接
	bool ReleaseConnection(MySQLOP *conn); 		//释放连接
	int GetFreeConn();							//获取空闲连接数量
	void DestroyPool();							//销毁所有连接

	//单例模式
	static connection_pool *GetInstance();

	void init(std::string url, std::string User, std::string PassWord, 
			  std::string DataBaseName, int MaxConn = MAX_CONN_SIZE);

private:
	connection_pool();
	~connection_pool();

	int m_MaxConn;  //最大连接数
	int m_CurConn;  //当前已使用的连接数
	int m_FreeConn; //当前空闲的连接数
	pthread_mutex_t lock;
	std::list<MySQLOP *> connList; //连接池
	pthread_cond_t hasConn;

public:
	std::string m_url;			 //主机地址
	// int m_Port;		 //数据库端口号
	std::string m_User;		 //登陆数据库用户名
	std::string m_PassWord;	 //登陆数据库密码
	std::string m_DatabaseName; //使用数据库名
};

class connectionRAII{

public:
	connectionRAII(MySQLOP **con, connection_pool *connPool);
	~connectionRAII();

private:
	MySQLOP *conRAII;
	connection_pool *poolRAII;
};


#endif 
