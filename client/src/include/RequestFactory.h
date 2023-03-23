#pragma once
#include "CodecFactory.h"
#include "RequestCodec.h"
#include <memory>

class RequestFactory :
	public CodecFactory
{
public:
	RequestFactory(string encstr);
	RequestFactory(RequestInfo* msg);
	~RequestFactory();

	Codec* createCodec();

private:
	bool isEncode = false;
	RequestInfo * m_request;
	string m_encstr;
	std::shared_ptr<Codec> m_ptr;
};

