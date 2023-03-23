#include "RespondCodec.h"

RespondCodec::RespondCodec(){}

RespondCodec::RespondCodec(string encstr){
    initMessage(encstr);
}

RespondCodec::RespondCodec(RespondInfo *msg){
    initMessage(msg);
}

RespondCodec::~RespondCodec(){
    std::cout << "加解码析构" << std::endl;
}

void RespondCodec::initMessage(string encstr){
    m_encstr = encstr;
}
void RespondCodec::initMessage(RespondInfo* info){
    m_msg.set_type(info->type);
    m_msg.set_rv(info->rv);
    m_msg.set_clientid(info->clientId);
    m_msg.set_serverid(info->serverId);
    // m_msg.set_key(info->key);
    // m_msg.set_seckeyid(info->seckeyid);
    m_msg.set_sign(info->sign);
    m_msg.set_authcode(info->authCode);
    m_msg.set_data(info->data);
}

string RespondCodec::msgEncode(){
    return m_msg.SerializeAsString();
}
void* RespondCodec::msgDecode(){
    // cout << "m_encstr size: " << m_encstr.size() << endl;
    // for (int i = 0; i < m_encstr.size(); ++i){
    //     printf("%d ", m_encstr[i]);
    // }
    // printf("\n");


    if (m_msg.ParseFromString(m_encstr)){
        // cout << m_msg.clientid() << "\n";
        // cout << m_msg.serverid() << "\n";
        // cout << m_msg.data() << "\n";
        return &m_msg;
    }
    else {
        cout << "服务器数据解码失败" << endl;
        return nullptr;
    }
}