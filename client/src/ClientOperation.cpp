#include "ClientOperation.h"
#include "RequestCodec.h"
#include <string.h>
#include <time.h>
#include <fstream>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "CodecFactory.h"
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "Hash.h"
#include "HmacPool.h"


using namespace std;

ClientOperation::ClientOperation(string jsonPath)
{
	ifstream ifs(jsonPath);
    Reader r;
    Value root;
    r.parse(ifs, root);
	ifs.close();

    m_info.sPort = root["port"].asInt();
	strcpy(m_info.serverID, root["serverID"].asString().data());
    // m_info.serverID = root["serverID"].asString();
	strcpy(m_info.clinetID, root["clientID"].asString().data());
	strcpy(m_info.serverIP, root["IP"].asString().data());
	m_info.maxNode = root["shmMaxNode"].asInt();
	m_info.shmKey = root["shmKey"].asInt();

	// cout << "m_info.shmKey = " << m_info.shmKey <<" m_info.maxNode = " << m_info.maxNode << endl;
	//创建共享内存
	m_shm = new SecKeyShm(m_info.shmKey, m_info.maxNode);

	// 连接服务端
	m_socket.connectToHost(m_info.serverIP, m_info.sPort);
	m_socket.blockIO();
    cout << "连接服务器成功" << endl;
}

ClientOperation::~ClientOperation()
{

}


int ClientOperation::secKeyAgree()
{
	int ret;
	//准备请求数据 
	RequestInfo req;
	// memset(&req, 0x00, sizeof(RequestInfo));
	req.Type = Key::RANDOMNUM;
	strcpy(req.clientId, m_info.clinetID);
	strcpy(req.serverId, m_info.serverID);

    // 生成随机数
    string clientRandomNum;
	getRandString(4, clientRandomNum, true);
    req.data = clientRandomNum;

	// // 连接服务端
	// m_socket.connectToHost(m_info.serverIP, m_info.sPort);
	// m_socket.blockIO();
    // cout << "连接服务器成功" << endl;

    // 序列化并发送
	CodecFactory* fac = new RequestFactory(&req);
	Codec* Codec = fac->createCodec();
	string encMsg = Codec->msgEncode();
	m_socket.sendMsg(encMsg.data(), encMsg.size());
	delete fac;
	fac = nullptr;

	//等待接收服务端的应答
	char *inData = nullptr;
    int dataLen = 0;
	ret = m_socket.recvMsg(&inData, &dataLen);
	if (ret != 0) cout << "读取socket失败" << endl;
	else cout << "读取socket成功, 字节数: " << dataLen << "\n";

	//解码
	fac = new RespondFactory(string(inData, dataLen));
	Codec = fac->createCodec();
	Key::RespondMsg* resMsg = (Key::RespondMsg*)Codec->msgDecode();

	// 工厂与code资源绑定一起，工厂析构 资源也会析构
	// delete fac;
	// fac = nullptr;

	if (resMsg->type() == Key::RANDOMNUM) cout << "收到服务器响应数据" << endl;

	//判断服务端是否成功
	auto rv = resMsg->rv();
	if (rv == Key::SUCCESS)
	{
		cout << "成功收到服务器随机数" << endl;
	}
	else 
	{
		cout << "服务器随机数获取失败！错误码：" << rv << endl;
		return -1;
	}
	cout << "------------------------" << "\n";
	cout << "clientid: " << resMsg->clientid() << endl;
	cout << "serverid: " << resMsg->serverid() << endl;

    // 解析数据 randomnlen + serverRandomNum + pkeylen + pkey
    char resdata[2048] = {0};

	// strcpy 拷贝过程遇 \0 会停止拷贝
    // strcpy(resdata, resMsg->data().data());
	memcpy(resdata, resMsg->data().data(), resMsg->data().size());
	cout << "接受数据: " << "\n";
	cout << "resMsg->data size: " << resMsg->data().size() << endl;

    int randomnlen = *((int*)resdata);
	// cout << "randomnlen = " << randomnlen << endl;

    string serverRandomNum(resdata+4, randomnlen);
    int pkeylen = *((int*)(resdata+4+randomnlen));
    string pkey(resdata+8+randomnlen, pkeylen);

	cout << "randomnlen = " << randomnlen << endl;
	cout << "serverRandomNum: " << serverRandomNum << endl;
	cout << "pkeylen = " << pkeylen << endl;
	
    // 公钥保存
    fstream fout;
    fout.open("./public.pem", ios_base::in | ios_base::out | ios_base::trunc );
    fout << pkey << endl;
    fout.close();

    // // 校验签名
    // rsa.loadRsaPublicKey("./public.pem");
    // if (!rsa.rsaVertify(resMsg->data(), resMsg->sign())){
    //     cout << "校验签名失败" << endl;
    //     return -1;
    // }

// 发送 premaster
    // memset(&req, 0x00, sizeof(RequestInfo));
	req.Type = Key::PREKEY;
	strcpy(req.clientId, m_info.clinetID);
	strcpy(req.serverId, m_info.serverID);

    // 生成premaster
    string premaster;
	getRandString(4, premaster, true);

    // 随机串 + premaster 拼接 -》 对称密码
    string endeKey = clientRandomNum + serverRandomNum.substr(0, 4) + premaster;
	string authKey = clientRandomNum.substr(0, 4) + serverRandomNum + premaster;
	cout << "endeKey: " << endeKey << endl;
	cout << "authKey: " << authKey << endl;

    // 加密 premaster
	rsa.loadRsaPublicKey("./public.pem");
    req.data = rsa.rsaPublicKeyEncrypt(premaster);
	cout << "加密后premaster: " << req.data << endl;

	// 消息认证
	Hmac hmac;
	hmac.init(authKey);
	Hash hash(HashType::T_SHA256);
	hash.addData(req.data);
	string hashData = hash.result();
	cout << hashData << endl;


	string clientAuthCode = hmac.auth(hashData);
	cout << "客户端认证码：" << clientAuthCode << endl;
	strcpy(req.authCode, clientAuthCode.data());
	cout << "请求认证码：" << req.authCode << endl;


	// 序列化并发送
	delete fac;
	fac = new RequestFactory(&req);
	Codec = fac->createCodec();
	encMsg = Codec->msgEncode();
	cout << encMsg << endl;
	m_socket.sendMsg(encMsg.data(), encMsg.size());
	// delete fac;
	fac = nullptr;


	//给秘钥结构体赋值
	NodeSecKeyInfo node;
	// memset(&node, 0x00, sizeof(NodeSecKeyInfo));
	node.status = 1;
	strcpy(node.clientID, m_info.clinetID);
	strcpy(node.serverID, m_info.serverID);
	strcpy(node.authKey, authKey.data());
	strcpy(node.endeKey, endeKey.data());

	// 接受 keyID
	m_socket.recvMsg(&inData, &dataLen);

	//解码
	delete fac;
	fac = new RespondFactory(inData);
	Codec = fac->createCodec();
	resMsg = (Key::RespondMsg*)Codec->msgDecode();
	// delete fac;
	fac = nullptr;


	rv = resMsg->rv();
	if (rv != Key::SUCCESS)
	{
		cout << "客户端获取keyID失败! 错误码: " << rv << endl;
		return -1;
	}
	else
	{
		cout << "获取keyID成功" << endl;
	}
	node.seckeyID = atoi(resMsg->data().data());

	//将秘钥信息写入共享内存
	m_shm->shmWrite(&node);
	cout << "共享内存写入成功" << endl;

	// //关闭网络连接
	// m_socket.disConnect();

	// 释放资源
	delete fac;

	return 0;
}

int ClientOperation::secKeyCheck()
{
	int ret;
	//准备请求数据 
	RequestInfo req;
	// memset(&req, 0x00, sizeof(RequestInfo));
	req.Type = Key::KEYCHECK;
	

	// // 连接服务端
	// m_socket.connectToHost(m_info.serverIP, m_info.sPort);
	// m_socket.blockIO();
    // cout << "连接服务器成功" << endl;

	// 共享内存读取密钥消息
	NodeSecKeyInfo node;
	m_shm->shmRead(m_info.clinetID, m_info.serverID, &node);
	if (node.status != 1){
		cout << "密钥已失效，请重新协商..." << endl;
		return -1;
	}

	// 组织请求
	strcpy(req.clientId, node.clientID);
	strcpy(req.serverId, node.serverID);
	req.keyId = node.seckeyID;
	req.data = node.authKey;
	req.data += node.endeKey;
	req.data = rsa.rsaPublicKeyEncrypt(req.data);

	Hmac hmac;
	hmac.init(node.authKey);
	Hash hash(HashType::T_SHA256);
	hash.addData(req.data);
	string hashData = hash.result();
	cout << hashData << endl;

	string clientAuthCode = hmac.auth(hashData);
	// cout << "客户端认证码：" << clientAuthCode << endl;
	strcpy(req.authCode, clientAuthCode.data());
	cout << "请求认证码：" << req.authCode << endl;

	// 序列化并发送
	CodecFactory* fac = new RequestFactory(&req);
	Codec* Codec = fac->createCodec();
	string encMsg = Codec->msgEncode();
	m_socket.sendMsg(encMsg.data(), encMsg.size());
	delete fac;
	fac = nullptr;
	Codec = nullptr;

	// 等待接收服务端的应答
	char *inData = nullptr;
    int dataLen = 0;
	ret = m_socket.recvMsg(&inData, &dataLen);
	if (ret != 0) cout << "读取socket失败" << endl;
	else cout << "读取socket成功, 字节数: " << dataLen << "\n";

	// 解码
	fac = new RespondFactory(string(inData, dataLen));
	Codec = fac->createCodec();
	Key::RespondMsg* resMsg = (Key::RespondMsg*)Codec->msgDecode();

	if (resMsg->type() == Key::KEYCHECK) cout << "收到服务器响应数据" << endl;

	// 判断服务端是否正常处理请求
	auto rv = resMsg->rv();
	if (rv == Key::SUCCESS)
	{
		cout << "校验成功!!!" << endl;
	}
	else 
	{
		cout << "校验失败...错误码: " << rv << endl;
		return -1;
	}

	return 0;
}

int ClientOperation::secKeyRevoke(){
	int ret;
	//准备请求数据 
	RequestInfo req;
	// memset(&req, 0x00, sizeof(RequestInfo));
	req.Type = Key::KEYREVOKE;

	// 共享内存读取密钥消息
	NodeSecKeyInfo node;
	m_shm->shmRead(m_info.clinetID, m_info.serverID, &node);
	if (node.status != 1){
		cout << "密钥已注销..." << endl;
		return -1;
	}

	// 组织请求
	strcpy(req.clientId, node.clientID);
	strcpy(req.serverId, node.serverID);
	req.keyId = node.seckeyID;
	req.data = node.authKey;
	req.data += node.endeKey;
	req.data = rsa.rsaPublicKeyEncrypt(req.data);
	

	Hmac hmac;
	hmac.init(node.authKey);
	Hash hash(HashType::T_SHA256);
	hash.addData(req.data);
	string hashData = hash.result();
	cout << hashData << endl;

	string clientAuthCode = hmac.auth(hashData);
	// cout << "客户端认证码：" << clientAuthCode << endl;
	strcpy(req.authCode, clientAuthCode.data());
	cout << "请求认证码：" << req.authCode << endl;

	// 序列化并发送
	CodecFactory* fac = new RequestFactory(&req);
	Codec* Codec = fac->createCodec();
	string encMsg = Codec->msgEncode();
	m_socket.sendMsg(encMsg.data(), encMsg.size());
	delete fac;
	fac = nullptr;
	Codec = nullptr;

	// 等待接收服务端的应答
	char *inData = nullptr;
    int dataLen = 0;
	ret = m_socket.recvMsg(&inData, &dataLen);
	if (ret != 0) cout << "读取socket失败" << endl;
	else cout << "读取socket成功, 字节数: " << dataLen << "\n";

	// 解码
	fac = new RespondFactory(string(inData, dataLen));
	Codec = fac->createCodec();
	Key::RespondMsg* resMsg = (Key::RespondMsg*)Codec->msgDecode();

	if (resMsg->type() == Key::KEYREVOKE) cout << "收到服务器响应数据" << endl;
	else {
		cout << "收到服务器错误匹配数据包, 请重新校验" << endl;
		return -1;
	}

	// 判断服务端是否正常处理请求
	auto rv = resMsg->rv();
	if (rv == Key::SUCCESS)
	{
		cout << "注销成功!!!" << endl;
	}
	else 
	{
		cout << "注销失败...错误码: " << rv << endl;
		return -1;
	}

	return 0;
}

// char randBuf[64]; , 参数 64, randBuf
void ClientOperation::getRandString(int len, string &randBuf, bool curtime)
{
	int flag = -1;
	// 设置随机种子
	srand(time(NULL));
	// 随机字符串: A-Z, a-z, 0-9, 特殊字符(!@#$%^&*()_+=)
	char chars[] = "!@#$%^&*()_+=";
	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;
		switch (flag)
		{
		case 0:
			// randBuf[i] = 'Z' - rand() % 26;
            randBuf.append(1, 'Z' - rand() % 26);
			break;
		case 1:
			// randBuf[i] = 'z' - rand() % 26;
            randBuf.append(1, 'z' - rand() % 26);
			break;
		case 3:
			// randBuf[i] = rand() % 10 + '0';
            randBuf.append(1, rand() % 10 + '0');
			break;
		case 2:
			// randBuf[i] = chars[rand() % strlen(chars)];
            randBuf.append(1, chars[rand() % strlen(chars)]);
			break;
		default:
			break;
		}
	}
	// randBuf[len - 1] = '\0';
	if (curtime){
		time_t timep;
		time(&timep);
		char tmp[4];
		memset(tmp, 0, sizeof(tmp));
		// strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
		strftime(tmp, sizeof(tmp), "%M%S", localtime(&timep));
		randBuf += string(tmp);
	}
}