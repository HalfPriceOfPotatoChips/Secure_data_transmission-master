#ifndef AESCRYPTO_H
#define AESCRYPTO_H

#include <string>
#include <openssl/aes.h>
#include <iostream>
using namespace std;

class AesCrypto
{
public:
	// 可使用 16byte, 24byte, 32byte 的秘钥
	AesCrypto(string key);
	~AesCrypto();
	// 加密
	string aesCBCEncrypt(string text);
	// 解密
	string aesCBCDecrypt(string encStr);

private:
	string aesCrypto(string data, int crypto);
	void generateIvec(unsigned char* ivec);
    string toBase64(const char* str, int len);
    char* fromBase64(string str);

private:
	AES_KEY m_encKey;
	AES_KEY m_decKey;
	string m_key;	// 秘钥
};

#endif
