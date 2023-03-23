#include "SecKeyShm.h"

using namespace std;

SecKeyShm::SecKeyShm(int key) : ShareMemory(key)
{
}

SecKeyShm::SecKeyShm(int key, int maxNode) 
	: ShareMemory(key, maxNode * sizeof(NodeSecKeyInfo))
	, m_maxNode(maxNode)
{
}

SecKeyShm::SecKeyShm(const char * pathName) : ShareMemory(pathName)
{
}

SecKeyShm::SecKeyShm(const char * pathName, int maxNode) 
	: ShareMemory(pathName, maxNode * sizeof(NodeSecKeyInfo))
	, m_maxNode(maxNode)
{
}

SecKeyShm::~SecKeyShm()
{
}

int SecKeyShm::shmWrite(NodeSecKeyInfo * pNodeInfo)
{
	int ret = -1;
	// 关联共享内存
	NodeSecKeyInfo* pAddr = static_cast<NodeSecKeyInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return ret;
	}

	// 判断传入的网点密钥是否已经存在
	NodeSecKeyInfo	*pNode = NULL;
	for (int i = 0; i < m_maxNode; i++)
	{
		// pNode依次指向每个节点的首地址
		pNode = pAddr + i;
		if (strcmp(pNode->clientID, pNodeInfo->clientID) == 0 &&
			strcmp(pNode->serverID, pNodeInfo->serverID) == 0)
		{
			// 如果找到了该网点秘钥已经存在, 使用新秘钥覆盖旧的值
			memcpy(pNode, pNodeInfo, sizeof(NodeSecKeyInfo));
			unmapShm();
			return 0;
		}
	}

	// 若没有找到对应的信息, 找一个空节点将秘钥信息写入
	int i = 0;
	NodeSecKeyInfo  tmpNodeInfo; //空结点
	memset(&tmpNodeInfo, 0, sizeof(NodeSecKeyInfo));
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		if (memcmp(&tmpNodeInfo, pNode, sizeof(NodeSecKeyInfo)) == 0)
		{
			ret = 0;
			memcpy(pNode, pNodeInfo, sizeof(NodeSecKeyInfo));
			break;
		}
	}
	if (i == m_maxNode)
	{
		ret = -1;
	}

	unmapShm();
	return ret;
}

int SecKeyShm::shmRead(const char * clientID, const char * serverID, NodeSecKeyInfo * pNodeInfo)
{
	int ret = 0;
	// 关联共享内存
	NodeSecKeyInfo *pAddr = NULL;
	pAddr = static_cast<NodeSecKeyInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return -1;
	}

	//遍历网点信息
	int i = 0;
	NodeSecKeyInfo	*pNode = NULL;
	// 通过clientID和serverID查找节点
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		if (strcmp(pNode->clientID, clientID) == 0 &&
			strcmp(pNode->serverID, serverID) == 0)
		{
			// 找到的节点信息, 拷贝到传出参数
			memcpy(pNodeInfo, pNode, sizeof(NodeSecKeyInfo));
			break;
		}
	}
	if (i == m_maxNode)
	{
		ret = -1;
	}

	unmapShm();
	return ret;
}

int SecKeyShm::updateStatue(int keyid, int state)
{
	int ret = -1;
	// 关联共享内存
	NodeSecKeyInfo* pAddr = static_cast<NodeSecKeyInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return ret;
	}
	
	NodeSecKeyInfo	*pNode = NULL;
	int i = 0;
	for (; i < m_maxNode; i++)
	{
		// pNode依次指向每个节点的首地址
		pNode = pAddr + i;
		if (pNode->seckeyID == keyid)
		{
			pNode->status = state;
			// unmapShm();
			// return 0;
			break;
		}
	}
	unmapShm();
	if (i == m_maxNode){
		return -1;
	}
	return 1;
}

