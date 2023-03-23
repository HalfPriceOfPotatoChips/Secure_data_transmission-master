#include "RespondFactory.h"


RespondFactory::RespondFactory(string encstr){
    isEncode = false;
    m_encstr = encstr;
}

RespondFactory::RespondFactory(RespondInfo* msg){
    isEncode = true;
    m_respond = msg;
}

RespondFactory::~RespondFactory(){}

Codec* RespondFactory::createCodec(){
#if 0
    return isEncode ? new RespondCodec(m_respond) : new RespondCodec(m_encstr);
#endif
    // m_ptr = isEncode ? make_shared<RespondCodec>(new RespondCodec(m_respond)) : make_shared<RespondCodec>(new RespondCodec(m_encstr));
    m_ptr = isEncode ? make_shared<RespondCodec>(m_respond) : make_shared<RespondCodec>(m_encstr);
    
    
    // Codec* p = new RespondCodec(m_respond);
    // // m_ptr = make_shared<Codec>(p);
	// m_ptr.reset(p);

    return m_ptr.get();
}