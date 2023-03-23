#pragma once
#include "lockqueue.h"
#include <string>
#include <memory>

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(INDO, c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(ERROR, c); \
    } while(0) \

// 定义日志级别
enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};

struct LogData
{
    LogLevel lv_;
    std::string s_;
    LogData(LogLevel lv, std::string& s) : lv_(lv), s_(s)
    {}
};


// Mprpc框架提供的日志系统
class Logger
{
public:
    // 获取日志的单例
    static Logger& GetInstance();
    // // 设置日志级别 
    // void SetLogLevel(LogLevel level);
    // 写日志
    void Log(LogLevel level, std::string msg);
private:
    // int m_loglevel; // 记录日志级别
    LockQueue<std::shared_ptr<LogData>>  m_lckQue; // 日志缓冲队列

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
};