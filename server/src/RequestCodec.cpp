#include "RequestCodec.h"

RequestCodec::RequestCodec(){

}

RequestCodec::~RequestCodec(){

}


RequestCodec::RequestCodec(string encstr){
    initMessage(encstr);
}

RequestCodec::RequestCodec(RequestInfo* info){
    initMessage(info);
}

void RequestCodec::initMessage(string encstr){
    m_encstr = encstr;
}

void RequestCodec::initMessage(RequestInfo* info){
    m_msg.set_type(info->Type);
    m_msg.set_clientid(info->clientId);
    m_msg.set_authcode(info->authCode);
    m_msg.set_serverid(info->serverId);
    m_msg.set_data(info->data);
    m_msg.set_sign(info->sign);
    m_msg.set_keyid(info->keyId);
}

string RequestCodec::msgEncode(){
    return m_msg.SerializeAsString();
}

void* RequestCodec::msgDecode(){
    // cout << m_encstr << endl;

    // Key::RequestMsg msg;
    // msg.ParseFromString(m_encstr);
    // cout << msg.clientid() << "\n";
    // cout << msg.serverid() << "\n";
    // cout << msg.data() << "\n";
    // cout << msg.authcode() << "\n";

    if(m_msg.ParseFromString(m_encstr))
        return &m_msg;
    else{
        cout << "客户端数据解码失败" << endl;
        return nullptr;
    }
}