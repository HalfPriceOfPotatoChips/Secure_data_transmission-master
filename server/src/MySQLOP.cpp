#include "MySQLOP.h"

const int ON = 1, OFF = 0;

MySQLOP::MySQLOP(){}
MySQLOP::~MySQLOP(){}

bool MySQLOP::connectDB(string host,string user,string pwd,string dbname){
    mysql = mysql_init(NULL);
    if(!mysql)
    {
        cout << "connectDB Error: "<< mysql_error(mysql);
        LOG_ERR("file:%s  line:%d  mysql_init() err  %s", __FILE__, __LINE__, mysql_error(mysql));
        return false;
    }
    if(!mysql_real_connect(mysql,host.c_str(),user.c_str(),pwd.c_str(),dbname.c_str(),0,NULL,0))
    {
        cout << "connect fial: "<< mysql_error(mysql);
        // m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, errno, "func mysql_real_connect() err");
        LOG_ERR("file:%s  line:%d  mysql_real_connect() err  %s", __FILE__, __LINE__, mysql_error(mysql));
        return false;
    }
    return true;
}

int MySQLOP::getKeyID(){
    string sql = "select ikeysn from KEYSN for update;";
    if (mysql_query(mysql, sql.data())){
        cout << "getKeyID fail: " << mysql_error(mysql);
        return -1;
    }else{
        MYSQL_RES * result = mysql_store_result(mysql);
        int keyID = -1;
        MYSQL_ROW row = NULL;
        if (row = mysql_fetch_row(result)){
            keyID = atoi(row[0]);
        }
        mysql_free_result(result);

        return keyID;
    }
}

bool MySQLOP::updataKeyID(int keyID){
    string sql = "update KEYSN set ikeysn = " + to_string(keyID);
    mysql_autocommit(mysql, OFF);
    if (mysql_query(mysql, sql.data())){
        cout << "updataKeyID fail: " << mysql_error(mysql);
        mysql_rollback(mysql);
        mysql_autocommit(mysql, ON);
        return false;
    }
    mysql_commit(mysql);
    mysql_autocommit(mysql, ON);
    return true;
}

bool MySQLOP::updataKeyState(int keyID, int state){
    char sql[512] = {0};
    sprintf(sql, "update AASECKEYINFO set state=%d where keyid=%d;", state, keyID);
    cout << "update sql: " << sql << endl;
    mysql_autocommit(mysql, OFF);
    if (mysql_query(mysql, sql)) {
        cout << "updataKeyState fail: " << mysql_error(mysql);
        mysql_rollback(mysql);
        mysql_autocommit(mysql, ON);
        return false;
    }
    mysql_commit(mysql);
    mysql_autocommit(mysql, ON);
    return true;
}


bool MySQLOP::writeSecKey(NodeSecKeyInfo *pNode, int& keyID){
    char sql[1024] = { 0 };
    sprintf(sql, "insert into AASECKEYINFO(clientid, serverid, createtime, \
                    state, authseckey, endeseckey) values ('%s', '%s', \
                    str_to_date('%s', '%%Y-%%m-%%d %%H:%%i:%%s') , %d, '%s', '%s');", 
		pNode->clientID, pNode->serverID, 
		getCurTime().data(), 1, pNode->authKey, pNode->endeKey);
	cout << "insert sql: " << sql << endl;
    mysql_autocommit(mysql, OFF);
    if (mysql_query(mysql, sql)){
        cout << "writeSecKey fail: " << mysql_error(mysql);
        mysql_rollback(mysql);
        mysql_autocommit(mysql, ON);
        return false;
    }
    mysql_commit(mysql);
    mysql_autocommit(mysql, ON);
    
    // 返回keyid
    sprintf(sql, "SELECT LAST_INSERT_ID();");
    if (mysql_query(mysql, sql)){
        cout << "query keyid fail: " << mysql_error(mysql);
        return false;
    }
    
    MYSQL_RES * result = mysql_store_result(mysql);
    MYSQL_ROW row = NULL;
    if (row = mysql_fetch_row(result)){
        keyID = atoi(row[0]);
    }
    cout << "keyID: " << keyID << endl;
    
    mysql_free_result(result);
    return true;
}

bool MySQLOP::querySecKey(tuple<string, bool, string, string>& out, const int& keyID){
    char sql[1024] = { 0 };
    sprintf(sql, "select clientid, state, authseckey, endeseckey \
            from AASECKEYINFO where keyid='%d';", keyID);
    cout << "select sql: " << sql << endl;
    if (mysql_query(mysql, sql)){
        // 查询无符合条件的数据时，从此返回
        cout << "querySecKey fail: " << mysql_error(mysql);
        return false;
    }else{
        MYSQL_RES * result = mysql_store_result(mysql);
        MYSQL_ROW row = NULL;
        if (row = mysql_fetch_row(result)){
            get<0>(out) = row[0];
            get<1>(out) = row[1];
            get<2>(out) = row[2];
            get<3>(out) = row[3];
        }
        mysql_free_result(result);

        return true;
    }
}


void MySQLOP::disconnect(){
    mysql_close(mysql);
}


string MySQLOP::getCurTime(){
    time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));

	return tmp;
}