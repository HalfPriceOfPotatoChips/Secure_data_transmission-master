#ifndef RESPOND_FACTORY_H
#define RESPOND_DACTORY_H

#include "CodecFactory.h"
#include "RespondCodec.h"
#include <memory>

class RespondFactory :
	public CodecFactory
{
public:
	RespondFactory(string encstr);
	RespondFactory(RespondInfo* msg);
	~RespondFactory();

	Codec* createCodec();

private:
	bool isEncode = false;
	RespondInfo * m_respond;
	string m_encstr;
	// std::shared_ptr<RespondCodec> m_ptr;
	std::shared_ptr<Codec> m_ptr;

};

#endif