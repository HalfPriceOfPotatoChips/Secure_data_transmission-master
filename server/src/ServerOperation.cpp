#include "ServerOperation.h"

using namespace std;

ServerOperation::ServerOperation(string jsonPath){
    ifstream ifs(jsonPath);
    Reader r;
    Value root;
    r.parse(ifs, root);
	ifs.close();

    m_info.sPort = root["port"].asInt();
    m_info.serverID = root["ID"].asString();
    // m_info.keyLen = root["keyLen"].asInt();
	m_info.dbHost = root["dbHose"].asString();
	m_info.dbUse = root["dbUse"].asString();
	m_info.dbPasswd = root["dbPasswd"].asString();
	m_info.dbName = root["dbName"].asString();
	m_info.shmPath = root["shmPath"].asString();
	m_info.maxnode = root["shmMaxNode"].asInt();
	m_info.maxClient = root["maxClient"].asInt();
	m_info.rsaPath = root["rsaPath"].asString();

	// 检查并生成非对称加密密钥
	loadPPKey(m_info.rsaPath);

	threadPool = new thread_pool(1, 20, 10000, false);
	pthread_mutex_init(&mutex, nullptr);

	hmacPool = HmacPool::GetInstance();

	// m_SQL.connectDB(m_info.dbHost, m_info.dbUse, m_info.dbPasswd, m_info.dbName);
	conPool = connection_pool::GetInstance();
	conPool->init(m_info.dbHost, m_info.dbUse, m_info.dbPasswd, m_info.dbName);

	// cout << "m_info.shmPath.data() = " << m_info.shmPath.data() <<"  m_info.maxnode = "
	// 	 << m_info.maxnode << "m_info.maxClient = " << m_info.maxClient << endl;
	m_shm = new SecKeyShm(m_info.shmPath.data(), m_info.maxnode);

	ep_fd = epoll_create(m_info.maxClient);
	if (ep_fd == -1) LOG_ERR("file:%s  line:%d  func epoll_create() err %d", __FILE__, __LINE__, ep_fd);

	m_server.setListen(m_info.sPort);
	epoll_event event;
	event.data.fd = m_server.getfg();
	event.events = EPOLLIN;
	epoll_ctl(ep_fd, EPOLL_CTL_ADD, m_server.getfg(), &event);
}

ServerOperation::~ServerOperation(){
	serverClose();
	google::protobuf::ShutdownProtobufLibrary();
	std::cout << "服务器关闭，结束\n";
	close(m_server.getfg());
}

void ServerOperation::loadPPKey(const string& path){
	
	// load 下 RSA_size 段错误？？？
	// if (access((path+"/public.pem").c_str(), F_OK) == -1 || 
	// 	access((path+"/private.pem").c_str(), F_OK) == -1) {
	// 		rsa = RsaCrypto();
	// 		rsa.generateRsakey(path);
	// 	}
	// else{
	// 	rsa.loadRsaPrivateKey((path+"/private.pem").c_str());
	// 	rsa.loadRsaPublicKey((path+"/public.pem").c_str());
	// } 
	rsa.generateRsakey(path);
}

void ServerOperation::startwork(){

    // m_server.setListen(m_info.sPort);
	int ret;
	epoll_event events[1024];

    while (1){
        // TcpSocket* socket = m_server.acceptConn();
        // if (socket == NULL){
		// 	cout << "accept 超时或失败" << endl;
		// 	continue;
		// }
		// cout << "客户端成功连接服务器..." << endl;
        // pthread_t pid;
        // pthread_create(&pid, NULL, workhard, this);
        // pthread_detach(pid);
        // m_listSocket.insert(make_pair(pid, socket));

		int ready = epoll_wait(ep_fd, events, m_info.maxClient, -1);
		if (ready == -1 && errno != EINTR)
			LOG_ERR("file:%s  line:%d  func epoll_wait() err %d", __FILE__, __LINE__, ready);
		for (int i = 0; i < ready; ++i){
			int fd = events[i].data.fd;
			if (fd == m_server.getfg()){
				//监听 fd用来 处理连接
				// sockaddr_in client_addr;
				// socklen_t client_len = sizeof(client_addr);
				TcpSocket * socket = nullptr;
				while (true){
					ret = m_server.acceptConn(&socket);
					// cout << "ret: " << ret << endl;
					//连接成功
					if (!ret){
						cout << "客户端连接成功！" << endl;
						initConnection(socket->getfd(), socket);
					}else break;
					// else cout << "客户端连接失败" << endl;
				}
			}else{
				if(events[i].events & EPOLLRDHUP){
					cout << "客户端关闭连接" << endl;
					//对方关闭连接
					close(fd);
					delete m_sockets[fd];
					m_sockets.erase(fd);
					// epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, nullptr);
				}else if (events[i].events & EPOLLIN){
					cout << "接受到任务" << endl;
					task t;
					t.func = workThread;
					// TcpSocket *socket = new TcpSocket(m_sockets[cfd]);
					// t.arg = (void *)socket;
					// t.arg = m_sockets[cfd];
					t.arg = this;
					t.fd = fd;
					threadPool->add_task(t);
				}
			}
		}
    }
}

void ServerOperation::initConnection(int fd, TcpSocket* socket){
	//设置非阻塞
	// int flag = fcntl(cfd, F_GETFL);
	// flag |= O_NONBLOCK;
	// fcntl(cfd, F_SETFL, flag);
	socket->noBlockIO();
	//加入监听红黑树中
	socket->addEpollTree(ep_fd);

	m_sockets.insert(make_pair(fd, socket));
}

void ServerOperation::getRandString(int len, string &randBuf, bool curtime)
{
	int flag = -1;
	// 设置随机种子
	srand(time(NULL));
	// 随机字符串: A-Z, a-z, 0-9, 特殊字符(!@#$%^&*()_+=)
	char chars[] = "!@#$%^&*()_+=";
	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;
		switch (flag)
		{
		case 0:
			// randBuf[i] = 'Z' - rand() % 26;
            randBuf.append(1, 'Z' - rand() % 26);
			break;
		case 1:
			// randBuf[i] = 'z' - rand() % 26;
            randBuf.append(1, 'z' - rand() % 26);
			break;
		case 3:
			// randBuf[i] = rand() % 10 + '0';
            randBuf.append(1, rand() % 10 + '0');
			break;
		case 2:
			// randBuf[i] = chars[rand() % strlen(chars)];
            randBuf.append(1, chars[rand() % strlen(chars)]);
			break;
		default:
			break;
		}
	}
	// randBuf[len - 1] = '\0';
	if (curtime){
		time_t timep;
		time(&timep);
		char tmp[4];
		memset(tmp, 0, sizeof(tmp));
		// strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
		strftime(tmp, sizeof(tmp), "%M%S", localtime(&timep));
		randBuf += string(tmp);
	}
}

int ServerOperation::clientRandomNum(Key::RequestMsg* reqMsg, TcpSocket* socket){
	// 访问数据库拿公私钥，生成随机数，拼接data，签名，保存随机数
	RespondInfo resInfo;
	//memset(&resInfo, 0x00, sizeof(RespondInfo));
	resInfo.type = Key::RANDOMNUM;
	strcpy(resInfo.clientId, reqMsg->clientid().data());
	strcpy(resInfo.serverId, reqMsg->serverid().data());
	resInfo.rv = Key::SUCCESS;
	// 1. 生成随机数并保存
	string serverRandomNum;
	getRandString(4, serverRandomNum, true);
	cout << "客户端随机串: " << reqMsg->data() << "\n";
	cout << "服务器随机串: " << serverRandomNum << "\n";
	pthread_mutex_lock(&mutex);
	randomNum[socket->getfd()] = make_pair(reqMsg->data(), serverRandomNum);
	pthread_mutex_unlock(&mutex);
	// 2. 拼接data
	ifstream ifs(m_info.rsaPath+"/public.pem");
    stringstream str;
    str << ifs.rdbuf();
	string publicStr = str.str();
	int randomnlen = serverRandomNum.size();
	int pkeylen = publicStr.size();
	char lenBuf[4] = {0};
	memcpy(lenBuf, &randomnlen, 4);

	cout << "随机串长度: "<< randomnlen << "\n";
	for (int i = 0; i < 4; ++i){
		printf("%d ", lenBuf[i]);
	}
	printf("\n");

	resInfo.data += string(lenBuf, 4)+serverRandomNum;
	memcpy(lenBuf, &pkeylen, 4);

	cout << "公钥长度: "<< pkeylen << "\n";
	for (int i = 0; i < 4; ++i){
		printf("%X ", lenBuf[i]);
	}
	printf("\n");
	
	resInfo.data += string(lenBuf, 4)+publicStr;

	// resInfo.data = "hello";
	// cout << "拼接后的data: " << resInfo.data << "\n";
	for (int i = 0; i < resInfo.data.size(); ++i){
        printf("%d ", resInfo.data[i]);
    }
    printf("\n");
	cout << "大小: " << resInfo.data.size() << "\n";
	// 3. 签名
	// resInfo.sign = rsa.rsaSign(resInfo.data);
	// 4. 序列化并发送
	CodecFactory* fac = new RespondFactory(&resInfo);
	Codec* c = fac->createCodec();
	string encMsg = c->msgEncode();
	// cout << "编码后的字节数: " << encMsg.size() << endl;
	// for (int i = 0; i < encMsg.size(); ++i){
    //     printf("%d ", encMsg[i]);
    // }
	socket->sendMsg(encMsg.data(), encMsg.size());
	delete fac;

	return 0;
}

int ServerOperation::clientPremaster(Key::RequestMsg* reqMsg, TcpSocket* socket){
	/* 私钥解密得premaster，拼接得两个共享密钥，premaster和共享密钥进行消息认证，
	   保存密钥到共享内存和数据库，发送keyid */
	RespondInfo resInfo;
	resInfo.type = Key::PREKEY;
	strcpy(resInfo.clientId, reqMsg->clientid().data());
	strcpy(resInfo.serverId, reqMsg->serverid().data());
	// 1. 私钥解密得premaster
	string premaster = rsa.rsaPriKeyDecrypt(reqMsg->data());
	// 2. 拼接得两个共享密钥
	pthread_mutex_lock(&mutex);
	pair<string, string> keys = randomNum[socket->getfd()];
	randomNum.erase(socket->getfd());
	pthread_mutex_unlock(&mutex);
	string endeKey = keys.first + keys.second.substr(0, 4) + premaster;
	string authKey = keys.first.substr(0, 4) + keys.second + premaster;
	cout << "endeKey: " << endeKey << endl;
	cout << "authKey: " << authKey << endl;
	// 3. 消息认证	先sha再认证
	Hash hash(HashType::T_SHA256);
	hash.addData(reqMsg->data());
	string hashData = hash.result();
	cout << hashData << endl;

	Hmac* hmac = nullptr;
	HmacRAII(&hmac, hmacPool);
	hmac->init(authKey);
	string serverAuthCode = hmac->auth(hashData);
	cout << "服务器生成的消息认证码: " << serverAuthCode << endl;
	cout << "客户端生成的消息认证码: " << reqMsg->authcode() << endl;

	// 服务器生成的消息认证码和客户端的进行对比
	do{
		if (serverAuthCode.compare(reqMsg->authcode()))
		{
			cout << "消息认证码不匹配..." << endl;
			resInfo.rv = Key::AUTHERR;
			break;
		}else{
			cout << "消息认证码匹配成功!!!" << endl;
		}
		// 4. 保存密钥到共享内存和数据库
		NodeSecKeyInfo node;
		strcpy(node.clientID, reqMsg->clientid().data());
		strcpy(node.serverID, reqMsg->serverid().data());
		strcpy(node.authKey, authKey.data());
		strcpy(node.endeKey, endeKey.data());
		MySQLOP* mysql = NULL;
		connectionRAII(&mysql, conPool);
		
		// // 通过加锁解决 多线程getKeyID()得到相同keyid
		// pthread_mutex_lock(&mutex);
		// node.seckeyID = mysql->getKeyID();
		// bool ret = mysql->writeSecKey(&node);
		// if (ret){
		// 	mysql->updataKeyID(node.seckeyID + 1);
		// }
		// pthread_mutex_unlock(&mutex);

		// 设置数据库keyid字段为auto_increment，每次insert都能得到不同keyid
		int keyID = -1;
		bool ret = mysql->writeSecKey(&node, keyID);
		if (!ret){
			resInfo.rv = Key::SERVERERR;
			break;
		}		
		node.seckeyID = keyID;
		resInfo.data = to_string(node.seckeyID);
		node.status = 1;

		if (m_shm->shmWrite(&node) == -1){
			resInfo.rv = Key::SERVERERR;
			break;
		}

		resInfo.rv = Key::SUCCESS;
	}while(0);
	// 5. 序列化并发送
	CodecFactory* fac = new RespondFactory(&resInfo);
	Codec* c = fac->createCodec();
	string encMsg = c->msgEncode();
	socket->sendMsg(encMsg.data(), encMsg.size());
	delete fac;


	return 0;
}

// int ServerOperation::keyRevoke(Key::RequestMsg* reqMsg){}

int ServerOperation::keyCheck(Key::RequestMsg* reqMsg, TcpSocket* socket){
	/* 根据keyid查询数据库得key，解密得明文，消息认证*/
	RespondInfo resInfo;
	resInfo.type = Key::KEYCHECK;
	strcpy(resInfo.clientId, reqMsg->clientid().data());
	strcpy(resInfo.serverId, reqMsg->serverid().data());
	// 1. 查询数据库
	MySQLOP* mysql = NULL;
	connectionRAII(&mysql, conPool);
	tuple<string, bool, string, string> out;
	do{
		int ret;
		ret = mysql->querySecKey(out, reqMsg->keyid());
		if (!ret || !get<1>(out)){
			// 查无此数据 或 密钥失效
			cout << "无此密钥" << endl;
			resInfo.rv = Key::AUTHERR;
			break;
		}
		string authKey = get<2>(out);
		string endeKey = get<3>(out);
		// 2. 解密
		// AesCrypto aes(endeKey);
		// string clientdata = aes.aesCBCDecrypt(reqMsg->data());
		string clientdata = rsa.rsaPriKeyDecrypt(reqMsg->data());
		cout << "解密明文: " << clientdata << endl;
		// 3. 消息认证
		Hash hash(HashType::T_SHA256);
		hash.addData(reqMsg->data());
		string hashData = hash.result();
		cout << hashData << endl;

		Hmac* hmac = nullptr;
		HmacRAII(&hmac, hmacPool);
		hmac->init(authKey, T_SHA256);
		string serverAuthCode = hmac->auth(hashData);
		cout << "服务器生成的消息认证码: " << serverAuthCode << endl;
		cout << "客户端生成的消息认证码: " << reqMsg->authcode() << endl;
		if (serverAuthCode.compare(reqMsg->authcode()))
		{
			cout << "消息认证码不匹配..." << endl;
			resInfo.rv = Key::AUTHERR;
			break;
		}else{
			cout << "消息认证码匹配成功!!!" << endl;
			// resInfo.rv = 0;
		}

		// 4. key 比对
		if(clientdata.compare(authKey+endeKey))
		{
			cout << "密钥匹配失败" << endl;
			resInfo.rv = Key::AUTHERR;
			break;
		}else cout << "密钥匹配成功" << endl;
		
		// 5. key保存共享内存
		NodeSecKeyInfo node;
		strcpy(node.clientID, reqMsg->clientid().data());
		strcpy(node.serverID, reqMsg->serverid().data());
		strcpy(node.authKey, authKey.data());
		strcpy(node.endeKey, endeKey.data());
		node.seckeyID = reqMsg->keyid();
		node.status = 1;
		if (m_shm->shmWrite(&node) == -1){
			resInfo.rv = Key::SERVERERR;
			break;
		}

		resInfo.rv = Key::SUCCESS;
	}while(0);
	// 6. 序列化并发送
	CodecFactory* fac = new RespondFactory(&resInfo);
	Codec* c = fac->createCodec();
	string encMsg = c->msgEncode();
	socket->sendMsg(encMsg.data(), encMsg.size());
	delete fac;

	return 0;
}

int ServerOperation::keyRevoke(Key::RequestMsg* reqMsg, TcpSocket* socket){
	/* 校验密钥，修改 database 和 共享内存 key state*/
	RespondInfo resInfo;
	resInfo.type = Key::KEYREVOKE;
	strcpy(resInfo.clientId, reqMsg->clientid().data());
	strcpy(resInfo.serverId, reqMsg->serverid().data());
	// 1. 查询数据库
	MySQLOP* mysql = NULL;
	connectionRAII(&mysql, conPool);
	tuple<string, bool, string, string> out;
	do{
		int ret;
		ret = mysql->querySecKey(out, reqMsg->keyid());
		if (!ret || !get<1>(out)){
			// 查无此数据 或 密钥失效
			cout << "无此密钥" << endl;
			resInfo.rv = Key::AUTHERR;
			break;
		}
		string authKey = get<2>(out);
		string endeKey = get<3>(out);
		// 2. 解密
		// AesCrypto aes(endeKey);
		// string clientdata = aes.aesCBCDecrypt(reqMsg->data());
		string clientdata = rsa.rsaPriKeyDecrypt(reqMsg->data());

		// cout << "解密明文: " << clientdata << endl;
		// 3. 消息认证
		Hash hash(HashType::T_SHA256);
		hash.addData(reqMsg->data());
		string hashData = hash.result();
		cout << hashData << endl;

		Hmac* hmac = nullptr;
		HmacRAII(&hmac, hmacPool);
		hmac->init(authKey, T_SHA256);
		string serverAuthCode = hmac->auth(hashData);
		cout << "服务器生成的消息认证码: " << serverAuthCode << endl;
		cout << "客户端生成的消息认证码: " << reqMsg->authcode() << endl;
		if (serverAuthCode.compare(reqMsg->authcode()))
		{
			cout << "消息认证码不匹配..." << endl;
			resInfo.rv = Key::AUTHERR;
			break;
		}else{
			cout << "消息认证码匹配成功!!!" << endl;
			// resInfo.rv = 0;
		}

		// 4. key 比对
		if(clientdata.compare(authKey+endeKey))
		{
			cout << "密钥匹配失败" << endl;
			resInfo.rv = Key::AUTHERR;
			break;
		}else cout << "密钥匹配成功" << endl;
		
		// 5. 更新共享内存节点state
		ret = m_shm->updateStatue(reqMsg->keyid(), 0);
		if (ret == -1){
			cout << "共享内存无该keyid对应节点" << endl;
		}else if (ret != 1){
			cout << "共享内存节点state更新失败" << endl;
			resInfo.rv = Key::SERVERERR;
			break;
		}

		// 6. 更新数据库state
		ret = mysql->updataKeyState(reqMsg->keyid(), 0);
		if (!ret) {
			cout << "database state update fail" << endl;
			resInfo.rv = Key::SERVERERR;
			break;
		}

		resInfo.rv = Key::SUCCESS;
	}while(0);
	// 6. 序列化并发送
	CodecFactory* fac = new RespondFactory(&resInfo);
	Codec* c = fac->createCodec();
	string encMsg = c->msgEncode();
	socket->sendMsg(encMsg.data(), encMsg.size());
	delete fac;

	return 0;
}

void* ServerOperation::workThread(void* arg, int fd){
	// TcpSocket* socket = (TcpSocket*)arg;

	cout << "cur thread is " << std::this_thread::get_id() << endl;
	ServerOperation* op = (ServerOperation*)arg;
	int ret;
	TcpSocket* socket = op->m_sockets[fd];
	std::cout << " 处理任务\n";
	int recvLen = -1;
	char* recvBuf = NULL;
	vector<string> messages;
	// 1. 读出全部请求
	do{
		ret = socket->recvMsg(&recvBuf, &recvLen);
		cout << "recvLen: " << recvLen <<" ret: " << ret << endl;
		if (ret != -2) {
			messages.emplace_back(recvBuf, recvLen);
			delete recvBuf;
			recvBuf = nullptr;
		}
	}while (ret != -2);
	// cout << "recvBuf: " << recvBuf << endl;
	// cout << "recvLen: " << recvLen << endl;
	cout << "message.size = " << messages.size() << endl;

	// 2. 序列化之后的数据解码
	for (int i = 0; i < messages.size(); ++i){
		CodecFactory* factory = new RequestFactory(messages[i]);
		Codec* codec = factory->createCodec();
		Key::RequestMsg * reqMsg = (Key::RequestMsg*)codec->msgDecode();

		// 3. 判断Type
		cout << "reqMsg type: " << reqMsg->type() << endl;
		switch (reqMsg->type())
		{
		case(Key::RANDOMNUM):
			ret = op->clientRandomNum(reqMsg, socket);
			break;
		case(Key::PREKEY):
			ret = op->clientPremaster(reqMsg, socket);
			break;
		case(Key::KEYCHECK):
			ret = op->keyCheck(reqMsg, socket);
			break;
		case(Key::KEYREVOKE):
			ret = op->keyRevoke(reqMsg, socket);
			break;
		default:
			break;
		}
		if (ret == -1){
			cout << "请求处理出现错误" << endl;
		}
		// 释放资源
		delete factory;
	}
	// 重新注册读事件
	op->fdMode(fd, EPOLLIN);
	return nullptr;
}

// int ServerOperation::getepfd(){
// 	return ep_fd;
// }

void ServerOperation::fdMode(int fd, int ev){
	epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLRDHUP;
	event.events |= EPOLLONESHOT;
	epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &event);
}

void ServerOperation::serverClose(){
	// 线程池最先析构，子线程生命周期小于连接池哈希池等资源，安全退出
	// threadPool->close();
	delete threadPool;
	// connectionPool->DestroyPool();
	pthread_mutex_destroy(&mutex);
}