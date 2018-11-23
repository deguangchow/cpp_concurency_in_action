///    Copyright (C)2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    console log system.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/23
#pragma once
#ifndef CONSOLE_LOG_H
#define CONSOLE_LOG_H

//#include <tchar.h>
#include <io.h>
/*
����������غ����������Ҫ������ÿ���������������� TICK(); �꣬�磺
void load() {
    TICK();
    ...
    Do something else...
}

�����Ҫ�ڿ���̨�������־����ʹ��
ERR��format, ...��������־
INFO(format, ...)һ����Ϣ��ʾ
WARN(format, ...)�澯��Ϣ��ʾ
DEBUG(format, ...)������Ϣ��ʾ
BAR(format, ...)������Ϣ��ʾ
*/

class Tick {
    static int  FuncDeep;
    static char Prefix[1024];
    const char* fn;
    long t;

public:
    explicit Tick(const char* funcname);
    ~Tick();

    static void error(char* format, ...);
    static void warn(char* format, ...);
    static void debug(char* format, ...);
    static void info(char* format, ...);
    static void logfile(char* format, ...);
    static void progress(char* format, ...);

    //Transfer UNIX time to Beijing-Time string.
    // e.g. 2018-08-07 15:24:45
    static char* unixTime2Str() {
        time_t tick;
        struct tm tm;
        static char strTime[32] = { 0 };
#if _WIN32
        tick = time(NULL);
        localtime_s(&tm, &tick);
        strftime(strTime, sizeof(strTime), "%H:%M:%S", &tm);
        //strftime(strTime, sizeof(strTime), "%Y-%m-%d %H:%M:%S", &tm);
#endif
        return strTime;
    }
};

#define FS_JOIN(X, Y)       FS_DO_JOIN(X, Y)
#define FS_DO_JOIN(X, Y)    FS_DO_JOIN2(X, Y)
#define FS_DO_JOIN2(X, Y)   X##Y

#define LOG_FILE(_F_, ...)  Tick::logfile(_F_, __VA_ARGS__)

#ifdef _DEBUG
#define TICK()Tick FS_DO_JOIN(tick_, __COUNTER__)(__FUNCTION__)
#define ERR(_F_, ...)Tick::error(_F_, __VA_ARGS__)
#define WARN(_F_, ...)Tick::warn(_F_, __VA_ARGS__)
#define DEBUG(_F_, ...)Tick::debug(_F_, __VA_ARGS__)
#define INFO(_F_, ...)Tick::info(_F_, __VA_ARGS__)
#define BAR(_F_, ...)Tick::progress(_F_, __VA_ARGS__)
#define ST(_ID)
#else
#define TICK()
#define ERR(_F_, ...)
#define WARN(_F_, ...)
#define DEBUG(_F_, ...)
#define INFO(_F_, ...)
#define BAR(_F_, ...)
#define ST(_ID)Tick FS_DO_JOIN(tick_, __COUNTER__)(_ID)
#endif

#endif  //CONSOLE_LOG_H