#ifndef _ITCAST_LOG_H_
#define _ITCAST_LOG_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <unistd.h>

#include <string>
using namespace std;
#include <cstdarg>
/************************************************************************/
/* 
const char *file：文件名称
int line：文件行号
int level：错误级别
		0 -- 没有日志
		1 -- debug级别
		2 -- info级别
		3 -- warning级别
		4 -- err级别
int status：错误码
const char *fmt：可变参数
*/
/************************************************************************/
// 日志类
class ItcastLog
{
public:
    enum LogLevel{NOLOG, DEBUG, INFO, WARNING, ERROR};
    void Log(const char *file, int line, int level, int status, const char *fmt, ...);
    static ItcastLog& GetInstance();

    ItcastLog(const ItcastLog&) = delete;
    ItcastLog& operator=(const ItcastLog&) = delete;
private:
    ItcastLog() = default;
    ~ItcastLog() = default;

    int ITCAST_Error_GetCurTime(char* strTime);
    int ITCAST_Error_OpenFile(int* pf);
    void ITCAST_Error_Core(const char *file, int line, int level, int status, const char *fmt, va_list args);
};

#endif
