#include "RsaCrypto.h"

RsaCrypto::RsaCrypto(){}

RsaCrypto::~RsaCrypto(){
    RSA_free(publicKey);
	RSA_free(privateKey);
}

void RsaCrypto::generateRsakey(const string& path, int bits){
    RSA* rsa = RSA_new();
    BIGNUM* e = BN_new();
    BN_set_word(e, 12345);
    RSA_generate_key_ex(rsa, bits, e, NULL);
#if 0
    FILE* fp = fopen("public.pem", "w");
    PEM_write_RSAPublicKey(fp, rsa);
    fclose(fp);

    fp = fopen("private.pem", "w");
    PEM_write_RSAPrivateKey(fp, rsa, NULL, NULL, 0, NULL, NULL);
    fclose(fp);
#else
    BIO* pubIO = BIO_new_file((path+"/public.pem").data(), "w");
    PEM_write_bio_RSAPublicKey(pubIO, rsa);
    BIO_flush(pubIO);
    BIO_free(pubIO);

    BIO* priBio = BIO_new_file((path+"/private.pem").data(), "w");
    PEM_write_bio_RSAPrivateKey(priBio, rsa, NULL, NULL, 0, NULL, NULL);
    BIO_flush(pubIO);
    BIO_free(priBio);
#endif
    publicKey = RSAPublicKey_dup(rsa);
    privateKey = RSAPrivateKey_dup(rsa);

    BN_free(e);
	RSA_free(rsa);
}

bool RsaCrypto::loadRsaPublicKey(const char * publicKeyPath){
    BIO* bio = BIO_new_file(publicKeyPath, "r");
    publicKey = RSA_new();
    if (PEM_read_bio_RSAPublicKey(bio, &publicKey, NULL, NULL) == NULL){
        ERR_print_errors_fp(stdout);
		return false;
    }
    BIO_free(bio);
    return true;
}

bool RsaCrypto::loadRsaPrivateKey(const char * privateKeyPath){
    BIO* bio = BIO_new_file(privateKeyPath, "r");
    privateKey = RSA_new();
    if (PEM_read_bio_RSAPublicKey(bio, &privateKey, NULL, NULL) == NULL){
        ERR_print_errors_fp(stdout);
		return false;    
    }
    BIO_free(bio);
    return true;
}

string RsaCrypto::rsaPublicKeyEncrypt(const string &text){
    cout << "加密数据长度: " << text.size() << endl;
	// 计算公钥长度
	int keyLen = RSA_size(publicKey);
	cout << "pubKey len: " << keyLen << endl;
    char *encode = new char[RSA_size(publicKey) + 1];
    int ret = RSA_public_encrypt(text.size(), (const unsigned char*)text.data(), 
                        (unsigned char *)encode, publicKey, RSA_PKCS1_PADDING);
    string retStr;
	if (ret >= 0)
	{
		// 加密成功
		cout << "ret: " << ret << ", keyLen: " << keyLen << endl;
		retStr = toBase64(encode, ret);
	}
	else
	{
		ERR_print_errors_fp(stdout);
	}
	// 释放资源
	delete[]encode;
	return retStr;
}

string RsaCrypto::rsaPriKeyDecrypt(string encData)
{
	// text指向的内存需要释放
	char* text = fromBase64(encData);
	// 计算私钥长度
	//cout << "解密数据长度: " << text.size() << endl;
	int keyLen = RSA_size(privateKey);
	// 使用私钥解密
	char* decode = new char[keyLen + 1];
	// 数据加密完成之后, 密文长度 == 秘钥长度
	int ret = RSA_private_decrypt(keyLen, (const unsigned char*)text,
		(unsigned char*)decode, privateKey, RSA_PKCS1_PADDING);
	string retStr = string();
	if (ret >= 0)
	{
		retStr = string(decode, ret);
	}
	else
	{
		cout << "私钥解密失败..." << endl;
		ERR_print_errors_fp(stdout);
	}
	delete[]decode;
	delete[]text;
	return retStr;
}

string RsaCrypto::rsaSign(const string& data, SignLevel level)
{
	unsigned int len;
    int keylen = RSA_size(privateKey);
    cout << "私钥 len: " << keylen << endl;
    unsigned char* signBuf = new unsigned char[10240];
    memset(signBuf, 0, 10240);
	int ret = RSA_sign(level, (const unsigned char*)data.data(), data.size(), signBuf,
		&len, privateKey);
	if (ret == -1)
	{
		ERR_print_errors_fp(stdout);
	}
	cout << "sign len: " << len << ", ret: " << ret << endl;
	string retStr = toBase64((const char *)signBuf, len);
	delete[]signBuf;
	return retStr;
}

bool RsaCrypto::rsaVertify(const string &data, const string &signData, SignLevel level){
    int keyLen = RSA_size(publicKey);
	char* sign = fromBase64(signData);
    int ret = RSA_verify(NID_sha1, (const unsigned char*)data.data(), data.size(),
                        (const unsigned char*)sign, keyLen, publicKey);
	delete[]sign;
    // return ret==1 ? true : false;
    if (ret == -1)
	{
		ERR_print_errors_fp(stdout);
	}
	if (ret != 1)
	{
		return false;
	}
	return true;
}

string RsaCrypto::toBase64(const char* str, int len){
    BIO* mem = BIO_new(BIO_s_mem());
    BIO* bs64 = BIO_new(BIO_f_base64());
    // mem添加到bs64中
    bs64 = BIO_push(bs64, mem);
    // 写数据
    BIO_write(bs64, str, len);
    BIO_flush(bs64);
    // 得到内存对象指针
    BUF_MEM *memPtr;
    BIO_get_mem_ptr(bs64, &memPtr);
    string retStr = string(memPtr->data, memPtr->length);
    BIO_free_all(bs64);
    return retStr;
}

char* RsaCrypto::fromBase64(string str){
    BIO *bs64 = NULL;
    BIO *mem = NULL;
    char *buffer = (char *)malloc(str.size());
    bs64 = BIO_new(BIO_f_base64());
    mem = BIO_new_mem_buf(str.data(), str.size());
    mem = BIO_push(bs64, mem);
    BIO_read(mem, buffer, str.size());
    BIO_free_all(mem);

    return buffer;
}