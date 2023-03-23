#include "RequestFactory.h"


RequestFactory::RequestFactory(string encstr){
    isEncode = false;
    m_encstr = encstr;
}

RequestFactory::RequestFactory(RequestInfo* msg){
    isEncode = true;
    m_request = msg;
}

RequestFactory::~RequestFactory(){

}

Codec* RequestFactory::createCodec(){
#if 0
    return isEncode ? new RequestCodec(m_request) : new RequestCodec(m_encstr);
#endif
    m_ptr = isEncode ? make_shared<RequestCodec>(m_request) : make_shared<RequestCodec>(m_encstr);
    return m_ptr.get();
}