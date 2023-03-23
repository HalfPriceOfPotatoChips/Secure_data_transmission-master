#ifndef RESPOND_CODEC_H
#define RESPOND_CODEC_H

#include "Codec.h"
#include "transit_data.pb.h"

struct  RespondInfo
{
	Key::TYPE type;
	Key::RV	rv;		// 返回值
	char	clientId[12] = {0};	// 客户端编号
	char	serverId[12] = {0};	// 服务器编号
	// string	key;		// 公钥
	// int	seckeyid;	// 对称公钥编号    keysn
	string  sign;
	string  authCode;
	string  data;
};

class RespondCodec : public Codec
{
public:
	RespondCodec();
	RespondCodec(string encstr);
	RespondCodec(RespondInfo *msg);
	~RespondCodec();

	void initMessage(string encstr);
	void initMessage(RespondInfo* info);

	// 函数重写
	string msgEncode();
	void* msgDecode();


private:
	Key::RespondMsg m_msg;
	string m_encstr;
};

#endif