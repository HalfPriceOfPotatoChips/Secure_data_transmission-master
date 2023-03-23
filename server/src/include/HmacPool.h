#pragma once
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <string>
#include "Hash.h"
#include <list>
using namespace std;

class Hmac
{
public:
    Hmac() = default;
    ~Hmac() = default;
    void init(const string& key, HashType type = HashType::T_SHA256);
    string auth(const string& data);
private:
    bool md5Auth(string& buf, const string& data);
    bool sha1Auth(string& buf, const string& data);
    bool sha224Auth(string& buf, const string& data);
    bool sha256Auth(string& buf, const string& data);
    bool sha512Auth(string& buf, const string& data);
private:
    string key_;
    HashType type_;
};

class HmacPool
{
public:
    Hmac* GetHmac();
    bool ReleaseHmac(Hmac* hmac);

    static HmacPool* GetInstance();

    void init(int MaxHmac);

    void DestroyPool();	

    HmacPool(const HmacPool&) = delete;
    HmacPool& operator=(HmacPool&) = delete;

private:
    HmacPool(int MaxHmac);
    HmacPool(HmacPool&& hmacpool) = default;
    ~HmacPool();

private:
    int m_MaxHmac;
    int m_CurHmac;
    int m_FreeHmac;
    pthread_mutex_t lock;
	std::list<Hmac *> HmacList; 
	pthread_cond_t hasHmac;
};


class HmacRAII
{
public:
    HmacRAII(Hmac **hmac, HmacPool* hmacPool);
    ~HmacRAII();
private:
    Hmac* hmacRAII;
    HmacPool* poolRAII;
};