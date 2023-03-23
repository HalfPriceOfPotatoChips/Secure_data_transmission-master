#ifndef MYSQL_OP_H
#define MYSQL_OP_H

#include <string>
#include <iostream>
#include <mysql/mysql.h>
#include "SecKeyShm.h"
#include <tuple>
// #include "ItcastLog.h"
#include "logger.h"
using namespace std;

class MySQLOP
{
public:
    MySQLOP();
    ~MySQLOP();
    bool connectDB(string host,string user,string pwd,string dbname);
    int getKeyID();
    bool updataKeyID(int keyID);
    bool updataKeyState(int keyID, int state);
    bool writeSecKey(NodeSecKeyInfo *pNode, int& keyID);
    bool querySecKey(tuple<string, bool, string, string>& out, const int& keyID);
    void disconnect();

private:
    string getCurTime();

private:
    MYSQL * mysql;
    // ItcastLog m_log;
};

#endif
