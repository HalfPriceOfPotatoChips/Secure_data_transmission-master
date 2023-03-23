#include "HmacPool.h"

const int HmacMaxSize = 1024;

void Hmac::init(const string& key, HashType type){ 
    key_ = key;
    type_ = type;    
}

string Hmac::auth(const string& data){
    string authCode;
    switch (type_)
    {
    case HashType::T_MD5:
        md5Auth(authCode, data);
        break;
    case HashType::T_SHA1:
        sha1Auth(authCode, data);
        break;
    case HashType::T_SHA224:
        sha224Auth(authCode, data);
        break;
    case HashType::T_SHA256:
        sha256Auth(authCode, data);
        break;
    case HashType::T_SHA512:
        sha512Auth(authCode, data);
        break;
    default:
        sha256Auth(authCode, data);
        break;
    }
    return authCode;
}

bool Hmac::md5Auth(string& buf, const string& data){
    unsigned char md[MD5_DIGEST_LENGTH];
    if (!HMAC(EVP_md5(), &key_, key_.size(), (const unsigned char*)data.data(), 
        data.size(), md, NULL)){
        ERR_print_errors_fp(stdout);
        return false;
    }
    char authCode[1024] = {0};
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        sprintf(&authCode[i * 2], "%02x", md[i]);
    }
    buf = string(authCode);
    return true;
}

bool Hmac::sha1Auth(string& buf, const string& data){
    unsigned char md[SHA_DIGEST_LENGTH];
    if (!HMAC(EVP_sha1(), &key_, key_.size(), (const unsigned char*)data.data(), 
        data.size(), md, NULL)){
        ERR_print_errors_fp(stdout);
        return false;
    }
    char authCode[1024] = {0};
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        sprintf(&authCode[i * 2], "%02x", md[i]);
    }
    buf = string(authCode);
    return true;
}

bool Hmac::sha224Auth(string& buf, const string& data){
    unsigned char md[SHA224_DIGEST_LENGTH];
    if (!HMAC(EVP_sha224(), &key_, key_.size(), (const unsigned char*)data.data(), 
        data.size(), md, NULL)){
        ERR_print_errors_fp(stdout);
        return false;
    }
    char authCode[1024] = {0};
    for (int i = 0; i < SHA224_DIGEST_LENGTH; ++i)
    {
        sprintf(&authCode[i * 2], "%02x", md[i]);
    }
    buf = string(authCode);
    return true;
}

bool Hmac::sha256Auth(string& buf, const string& data){
    // cout << "before hmac: " << data << endl;
    unsigned char md[SHA256_DIGEST_LENGTH];
    cout << "authkey_: " << key_ << endl;
    cout << "key,size: " << key_.size() << endl;

    cout << "data: " << data.data() << endl;
    cout << "data.size: " << data.size() <<endl;
    // unsigned int len = -1;
    if (!HMAC(EVP_sha256(), key_.data(), key_.size(), (const unsigned char*)data.data(), 
        data.size(), md, NULL)){
        ERR_print_errors_fp(stdout);
        return false;
    }
    // printf("after sha: ");
    // for (int i = 0; i < sizeof(md); ++i){
    //     printf("%d ", md[i]);
    // }
    // printf("\n");

    char authCode[512] = {0};
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        sprintf(&authCode[i * 2], "%02x", md[i]);
    }

    // printf("after code: ");
    // for (int i = 0; i < sizeof(authCode); ++i){
    //     printf("%d ", authCode[i]);
    // }
    // printf("\n");

    buf = string(authCode);

    cout << "buf: " << buf << endl;
    return true;
}

bool Hmac::sha512Auth(string& buf, const string& data){
    unsigned char md[SHA512_DIGEST_LENGTH];
    if (!HMAC(EVP_sha512(), &key_, key_.size(), (const unsigned char*)data.data(), 
        data.size(), md, NULL)){
        ERR_print_errors_fp(stdout);
        return false;
    }
    char authCode[1024] = {0};
    for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i)
    {
        sprintf(&authCode[i * 2], "%02x", md[i]);
    }
    buf = string(authCode);
    return true;
}

HmacPool* HmacPool::GetInstance(){
    // 单例构造
    static HmacPool hmacPool(100);
    return &hmacPool;
}

HmacPool::HmacPool(int HmacSize)
: m_CurHmac(0)
, m_FreeHmac(0)
, m_MaxHmac(HmacMaxSize)
{
    init(HmacSize);
}

HmacPool::~HmacPool(){
    DestroyPool();
    pthread_mutex_destroy(&(this->lock));
    pthread_cond_destroy(&(this->hasHmac));
}

void HmacPool::init(int HmacSize){
    std::cout << "Hmac池初始化\n";
    m_FreeHmac = HmacSize;
    for (int i = 0; i < HmacSize; ++i){
        Hmac* hmac = new Hmac;
        HmacList.push_back(hmac);
        // ++m_FreeHmac;
    }
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_init(&hasHmac, nullptr);
}

Hmac* HmacPool::GetHmac(){
    Hmac *hmac = NULL;

	pthread_mutex_lock(&lock);
	while(HmacList.size() == 0){
		pthread_cond_wait(&hasHmac, &lock);
	}

	hmac = HmacList.front();
	HmacList.pop_front();

	--m_FreeHmac;
	++m_CurHmac;

	pthread_mutex_unlock(&lock);
	return hmac;
}

bool HmacPool::ReleaseHmac(Hmac* hmac){
    if (NULL == hmac)
		return false;

	pthread_mutex_lock(&lock);

	HmacList.push_back(hmac);
	++m_FreeHmac;
	--m_CurHmac;

	pthread_cond_signal(&hasHmac);
	pthread_mutex_unlock(&lock);

	return true;
}

void HmacPool::DestroyPool()
{
    std::cout << "HmacPool 退出" << std::endl;
	pthread_mutex_lock(&lock);
	if (HmacList.size() > 0)
	{
		std::list<Hmac *>::iterator it;
		for (it = HmacList.begin(); it != HmacList.end(); ++it)
		{
			delete *it;
		}
		m_CurHmac = 0;
		m_FreeHmac = 0;
		HmacList.clear();
	}

	pthread_mutex_unlock(&lock);
}

HmacRAII::HmacRAII(Hmac **hmac, HmacPool* hmacPool){
    *hmac = hmacPool->GetHmac();
    hmacRAII = *hmac;
    poolRAII = hmacPool;
}

HmacRAII::~HmacRAII(){
    poolRAII->ReleaseHmac(hmacRAII);
}