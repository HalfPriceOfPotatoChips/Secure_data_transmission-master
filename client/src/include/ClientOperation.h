#pragma once
#include "TcpSocket.h"
#include "SecKeyShm.h"
#include "RsaCrypto.h"
#include <jsoncpp/json/json.h> 

using namespace Json;

class ClientInfo
{
public:
	char clinetID[12];			// 客户端ID
	char serverID[12];			// 服务器ID
	char serverIP[32];			// 服务器IP
	unsigned short sPort;	    // 服务器端口
	int maxNode;				// 共享内存节点个数
	int shmKey;					// 共享内存的Key
};

class ClientOperation
{
public:
	ClientOperation(string jsonPath);
	~ClientOperation();

	// 秘钥协商
	int secKeyAgree();
	// 秘钥校验
	int secKeyCheck();
	// 秘钥注销
	int secKeyRevoke();
	// // 秘钥查看
	// int secKeyView()

private:
	void getRandString(int len, string &randBuf, bool curtime);

private:
	ClientInfo m_info;
	TcpSocket m_socket;
	SecKeyShm* m_shm;
    RsaCrypto rsa;
	// std::unique_ptr<AesCrypto> aes;
};

