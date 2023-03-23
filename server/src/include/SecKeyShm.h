#pragma once
#include "ShareMemory.h"
#include <string.h>
#include <iostream>

class NodeSecKeyInfo
{
public:
	NodeSecKeyInfo() : status(0), seckeyID(0)
	{
		bzero(clientID, sizeof(clientID));
		bzero(serverID, sizeof(serverID));
		bzero(endeKey, sizeof(endeKey));
		bzero(authKey, sizeof(authKey));
	}
	int status;
	int seckeyID;
	char clientID[12];
	char serverID[12];
	char endeKey[128];
	char authKey[128];
};

class SecKeyShm : public ShareMemory
{
public:
	SecKeyShm(int key);
	SecKeyShm(int key, int maxNode);
	SecKeyShm(const char* pathName);
	SecKeyShm(const char* pathName, int maxNode);
	~SecKeyShm();

	int shmWrite(NodeSecKeyInfo* pNodeInfo);
	int shmRead(const char* clientID, const char* serverID, NodeSecKeyInfo* pNodeInfo);
	int updateStatue(int keyid, int state);

private:
	int m_maxNode;
};

