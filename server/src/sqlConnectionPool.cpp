//
// Created by chen on 2022/7/5.
//

#include "sqlConnectionPool.h"

connection_pool::connection_pool()
{
	m_CurConn = 0;
	m_FreeConn = 0;
}

connection_pool *connection_pool::GetInstance()
{
	static connection_pool connPool;
	return &connPool;
}

//构造初始化
void connection_pool::init(std::string url, std::string User, std::string PassWord, std::string DBName, int MaxConn)
{
	m_url = url;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;
	std::cout << "数据库连接池初始化\n";
	for (int i = 0; i < MaxConn; i++)
	{
		MySQLOP *con = NULL;
		con = new MySQLOP();
		con->connectDB(m_url, m_User, m_PassWord, m_DatabaseName);
		std::cout << i + 1 << "数据库连接以创建\n";
		connList.push_back(con);
		++m_FreeConn;
	}
	pthread_mutex_init(&lock, nullptr);
	pthread_cond_init(&hasConn, nullptr);

	m_MaxConn = m_FreeConn;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MySQLOP *connection_pool::GetConnection()
{
	MySQLOP *con = NULL;

	pthread_mutex_lock(&lock);
	while(connList.size() == 0){
		pthread_cond_wait(&hasConn, &lock);
	}

	con = connList.front();
	connList.pop_front();

	--m_FreeConn;
	++m_CurConn;

	pthread_mutex_unlock(&lock);
	return con;
}

//释放当前使用的连接
bool connection_pool::ReleaseConnection(MySQLOP *con)
{
	if (NULL == con)
		return false;

	pthread_mutex_lock(&lock);

	connList.push_back(con);
	++m_FreeConn;
	--m_CurConn;

	pthread_cond_signal(&hasConn);
	pthread_mutex_unlock(&lock);

	return true;
}

//销毁数据库连接池
void connection_pool::DestroyPool()
{

	pthread_mutex_lock(&lock);
	if (connList.size() > 0)
	{
		std::list<MySQLOP *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MySQLOP *con = *it;
			con->disconnect();
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}

	pthread_mutex_unlock(&lock);
}

//当前空闲的连接数
int connection_pool::GetFreeConn()
{
	return this->m_FreeConn;
}

connection_pool::~connection_pool()
{
	DestroyPool();
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&hasConn);
}

connectionRAII::connectionRAII(MySQLOP **SQL, connection_pool *connPool){
	*SQL = connPool->GetConnection();

	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}