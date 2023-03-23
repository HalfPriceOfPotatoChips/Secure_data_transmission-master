#ifndef RSA_CRYPTO_H
#define RSA_CRYPTO_H

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string>
#include <string.h>
#include <iostream>
using namespace std;

enum SignLevel
{
	Level1 = NID_md5,
	Level2 = NID_sha1,
	Level3 = NID_sha224,
	Level4 = NID_sha256,
	Level5 = NID_sha384,
	Level6 = NID_sha512
};

class RsaCrypto
{
public:
    RsaCrypto();
    ~RsaCrypto();
    void generateRsakey(const string& path, int bits = 1024);
    bool loadRsaPublicKey(const char * publicKeyPath);
    bool loadRsaPrivateKey(const char * privateKeyPath);
    string rsaPublicKeyEncrypt(const string &text);
    string rsaPriKeyDecrypt(string encData);
    string rsaSign(const string& data, SignLevel level = Level4);
    bool rsaVertify(const string &data, const string &signData, SignLevel level = Level4);

private:
    string toBase64(const char* str, int len);
    char* fromBase64(string str);

private:
    RSA* publicKey;
    RSA* privateKey;
};

#endif