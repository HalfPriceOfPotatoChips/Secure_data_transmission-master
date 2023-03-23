#ifndef SERVER_OP_H
#define SERVER_OP_H


#include "TcpSocket.h"
#include "TcpServer.h"
#include <map>
#include <string>
#include "SecKeyShm.h"
#include "AesCrypto.h"
#include <jsoncpp/json/json.h> 
#include "thread_pool.h"
#include "sqlConnectionPool.h"
#include "HmacPool.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "RsaCrypto.h"
#include "openssl/sha.h"
#include "Hash.h"
#include <fstream>
#include <sstream>
#include <tuple>
#include "logger.h"


// using namespace std;
using namespace Json;

struct ServerInfo
{
    // char serverID[12]; // 服务器端编号
    string serverID;
    // char dbUse[24];    // 数据库用户名
    string dbUse;
    // char dbPasswd[24]; // 数据库密码
    string dbPasswd;
    // char dbName[24];    // 数据库sid
    string dbName;
    // int  keyLen;       // 公钥长度
    // char dbHost[24];
    string dbHost;

    unsigned short sPort; // 服务器绑定的端口
    int maxnode;          // 共享内存最大网点树 客户端默认1个
    int shmkey;           // 共享内存keyid 创建共享内存时使用
    string shmPath;
    int maxClient;
    string rsaPath;       // 非对称加密密钥；
};

class ServerOperation
{
public:

    ServerOperation(string jsonPath);
    ~ServerOperation();

    void startwork();

    int clientRandomNum(Key::RequestMsg* reqMsg, TcpSocket* socket);

    int clientPremaster(Key::RequestMsg* reqMsg, TcpSocket* socket);

    // int keyRevoke(Key::RequestMsg* reqMsg);

    int keyCheck(Key::RequestMsg* reqMsg, TcpSocket* socket);

    int keyRevoke(Key::RequestMsg* reqMsg, TcpSocket* socket);

    void serverClose();

    static void* workThread(void* arg, int cfd);

    // int getepfd();

private:
    void loadPPKey(const string& path);
	void getRandString(int len, string& randBuf, bool curtime = false);
    void initConnection(int cfd, TcpSocket* socket);
    void fdMode(int fd, int ev);

private:
    ServerInfo m_info;
    TcpServer m_server;
    int ep_fd;
    RsaCrypto rsa;
    SecKeyShm* m_shm;
    // MySQLOP m_SQL;
    // ItcastLog& m_log;
    thread_pool* threadPool;
    HmacPool* hmacPool;
    connection_pool* conPool;
    pthread_mutex_t mutex; 
    std::unordered_map<int, TcpSocket*> m_sockets;
    std::unordered_map<int, pair<string, string>> randomNum;
};

#endif