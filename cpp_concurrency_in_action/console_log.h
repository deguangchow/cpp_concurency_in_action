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
辅助调试相关函数，如果需要，请在每个函数的入口入加入 TICK(); 宏，如：
void load() {
    TICK();
    ...
    Do something else...
}

如果需要在控制台上输出日志，请使用
ERR（format, ...）错误日志
INFO(format, ...)一般信息提示
WARN(format, ...)告警信息提示
DEBUG(format, ...)调试信息提示
BAR(format, ...)进度信息提示
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
    static void print(char* format, ...);
    static void logfile(char* format, ...);
    static void progress(char* format, ...);

    template<typename T>
    static void TV(T input) {
        for (auto &pos = input.cbegin(); pos != input.cend(); ++pos) {
            std::cout << *pos << " ";
        }
        std::cout << std::endl;
    }

    template<typename T>
    static void TV2(T input) {
        for (auto &pos = input.cbegin(); pos != input.cend(); ++pos) {
            std::cout << pos->first << "=" << pos->second << std::endl;
        }
    }

    //Transfer UNIX time to Beijing-Time string.
    // e.g. 2018-08-07 15:24:45
    static char* unixTime2Str() {
        time_t tick;
        struct tm tm;
        static char strTime[32] = { 0 };
#if _WIN32
        tick = time(NULL);
        localtime_s(&tm, &tick);
        strftime(strTime, sizeof(strTime), "%M:%S", &tm);
        //strftime(strTime, sizeof(strTime), "%H:%M:%S", &tm);
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
#define PRINT(_F_, ...)Tick::print(_F_, __VA_ARGS__)
#define BAR(_F_, ...)Tick::progress(_F_, __VA_ARGS__)
#define ST(_ID)
#define TV_LIST_INT(X)Tick::TV<std::list<int>>(X);
#define TV_MAP_INT(X)Tick::TV2<std::map<int, int>>(X);
#else
#define TICK()
#define ERR(_F_, ...)
#define WARN(_F_, ...)
#define DEBUG(_F_, ...)
#define INFO(_F_, ...)Tick::info(_F_, __VA_ARGS__)
#define PRINT(_F_, ...)
#define BAR(_F_, ...)
#define ST(_ID)Tick FS_DO_JOIN(tick_, __COUNTER__)(_ID)
#define TV_LIST_INT(X)
#define TV_MAP_INT(X)
#endif

#endif  //CONSOLE_LOG_H
