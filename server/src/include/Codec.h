#pragma once
#include <iostream>
using namespace std;

class Codec
{
public:
	Codec();
	virtual ~Codec();
	
	// 数据编码
	virtual string msgEncode();
	// 数据解码
	virtual void* msgDecode();
};

