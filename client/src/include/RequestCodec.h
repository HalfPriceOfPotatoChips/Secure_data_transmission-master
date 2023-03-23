#ifndef REQUEST_CODEC_H
#define REQUEST_CODEC_H

#include "Codec.h"
#include "transit_data.pb.h"

struct RequestInfo
{
	//1 密钥协商  	//2 密钥校验; 	// 3 密钥注销
    Key::TYPE		Type;				// 报文类型
	char	clientId[12] = { 0 };	// 客户端编号
	char	authCode[65] = { 0 };	// 认证码
	char	serverId[12] = { 0 };	// 服务器端编号 
	string	sign;
	string  data;
	int		keyId;
};

class RequestCodec : public Codec
{
public:
	// enum CmdType{NewOrUpdate=1, Check, Revoke, View};
	RequestCodec();
	RequestCodec(string encstr);
	RequestCodec(RequestInfo* info);
	~RequestCodec();

	void initMessage(string encstr);
	void initMessage(RequestInfo* info);

	// 重写父类函数
	string msgEncode();
	void* msgDecode();

private:
	Key::RequestMsg m_msg;
	string m_encstr;
};

#endif