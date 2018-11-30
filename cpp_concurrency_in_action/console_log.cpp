///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    console log system.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/23
#include "stdafx.h"
#include "console_log.h"

#ifdef _WIN32
#include <windows.h>
#endif

#if 1

#define WhiteOnBlack()      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define RedOnBlack()        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_RED)
#define GreenOnBlack()      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_GREEN)
#define YellowOnBlack()     SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_RED | FOREGROUND_GREEN)
#define BlueOnBlack()       SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_BLUE)
#define MagentaOnBlack()    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_RED | FOREGROUND_BLUE)
#define CyanOnBlack()       SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BlackOnGray()       SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
BACKGROUND_INTENSITY |BACKGROUND_INTENSITY)
#define BlackOnWhite()      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
BACKGROUND_INTENSITY |FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define RedOnWhite()        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
BACKGROUND_INTENSITY |FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |FOREGROUND_RED)
#define DefaultOnBlack()    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
FOREGROUND_INTENSITY |FOREGROUND_INTENSITY)
#else

#define WhiteOnBlack()
#define RedOnBlack()
#define GreenOnBlack()
#define YellowOnBlack()
#define BlueOnBlack()
#define MagentaOnBlack()
#define CyanOnBlack()
#define BlackOnGray()
#define BlackOnWhite()
#define RedOnWhite()
#define DefaultOnBlack()
#endif

int Tick::FuncDeep = 0;
char Tick::Prefix[1024] = { 0 };
static std::mutex mutex_console_log;    //mutex for multi-threading programming console log system.

Tick::Tick(const char* funcname) :fn(funcname), t(0) {
#if _WIN32
    t = GetTickCount();
#endif
#ifdef _DEBUG
    Prefix[FuncDeep] = ' '; ++FuncDeep;
    std::lock_guard<std::mutex> lock_console_log(mutex_console_log);
    MagentaOnBlack();
    printf("[CALL  %s]%s[>>]%s\n", unixTime2Str(), Prefix, funcname);
    DefaultOnBlack();
#endif //_DEBUG
}

Tick::~Tick() {
#ifdef _DEBUG
#if _WIN32
    long t2 = GetTickCount();
#endif
    {//Pay attention for the namespace of lock_console_log because the call of function 'WARN' using the same mutex!!!
        std::lock_guard<std::mutex> lock_console_log(mutex_console_log);
        WhiteOnBlack();
        printf("[EXIT  %s]%s[<<]%s[%ld]\n", unixTime2Str(), Prefix, fn, t2 - t);
        DefaultOnBlack();
    }
    Prefix[--FuncDeep] = '\0';

#if _WIN32
    //printf("%s tick %ld[ms]\n", fn, t2 - t);
    if (t2 - t > 500) {
        WARN(" !!!Function[%s] block too long, please attention.\n", fn);
    }
#endif
#endif //_DEBUG
}

void Tick::info(char* format, ...) {
    va_list vl;
    va_start(vl, format);
    std::lock_guard<std::mutex> lock_console_log(mutex_console_log);
    GreenOnBlack();
    printf("[INFO  %s]%*s", unixTime2Str(), FuncDeep, "");
    vprintf(format, vl);
    DefaultOnBlack();
    va_end(vl);
    printf("\r\n");
}

void Tick::debug(char* format, ...) {
    va_list vl;
    va_start(vl, format);
    std::lock_guard<std::mutex> lock_console_log(mutex_console_log);
    BlueOnBlack();
    printf("[DEBUG %s]%*s", unixTime2Str(), FuncDeep, "");
    vprintf(format, vl);
    DefaultOnBlack();
    va_end(vl);
    printf("\r\n");
}

void Tick::warn(char* format, ...) {
    va_list vl;
    va_start(vl, format);
    std::lock_guard<std::mutex> lock_console_log(mutex_console_log);
    YellowOnBlack();
    printf("[WARN  %s]%*s", unixTime2Str(), FuncDeep, "");
    vprintf(format, vl);
    DefaultOnBlack();
    va_end(vl);
    printf("\r\n");
}

void Tick::error(char* format, ...) {
    va_list vl;
    va_start(vl, format);
    std::lock_guard<std::mutex> lock_console_log(mutex_console_log);
    RedOnBlack();
    printf("\n[ERROR %s]%*s", unixTime2Str(), FuncDeep, "");
    vprintf(format, vl);
    DefaultOnBlack();
    va_end(vl);
    printf("\r\n");
}

#include <tchar.h>

void Tick::logfile(char* format, ...) {
    va_list args;
    char string[100000] = { 0 };
    char stringOut[100000] = { 0 };
    va_start(args, format);

    vsprintf_s(string, format, args);
    va_end(args);

#ifdef _WIN32
    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);

    sprintf_s(stringOut, "[time]%4d-%02d-%02d %d:%02d:%02d \n[file]%s\n[line]%d\n[content]\n%s\r\n",
        sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
        __FILE__, __LINE__, string);

    TCHAR szFilePath[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(NULL, szFilePath, MAX_PATH);
    (_tcsrchr(szFilePath, _T('\\')))[1] = 0; //delete filename, only get filepath

    LPTSTR lpStr = lstrcat(szFilePath, _T("console_system.log"));
    if (!lpStr) {
        return;
    }

    FILE* fd = nullptr;
    _tfopen_s(&fd, lpStr, _T("ab"));
    if (fd == NULL) {
        return;
    }

    int size = _filelength(_fileno(fd));

    if (size > 1e7) {
        fclose(fd);
        _tfopen_s(&fd, lpStr, _T("wb"));

        if (fd == NULL) {
            return;
        }
    }
    fwrite(stringOut, 1, strlen(stringOut), fd);
    fclose(fd);

#endif
}

void Tick::progress(char* format, ...) {
    va_list vl;
    va_start(vl, format);
    long t = 0;
#ifdef _WIN32
    t = GetTickCount();
    CONSOLE_SCREEN_BUFFER_INFO screen_info;
    ::GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen_info);
    screen_info.dwCursorPosition.X = 0;
    ::SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), screen_info.dwCursorPosition);
#endif
    printf("[BAR   %10ld]%*s", t, FuncDeep, "");
    vprintf(format, vl);

    va_end(vl);
}


#if _WIN32

#include <windows.h>
#include <tchar.h>

class ConsoleOutput {
public:
    ConsoleOutput() {
        AllocConsole();
        SetConsoleTitle(GetTitle());
        FILE *fp = nullptr;
        errno_t err = freopen_s(&fp, "CONOUT$", "w", stdout);
        DisableClose();

#ifdef _DEBUG
        //printf("%f\n", 1.0 / 3.0);
        //printf("%.16f\n", 1.0 / 3.0);
#endif
    }
    ~ConsoleOutput() {
        FreeConsole();
    }

    TCHAR* GetTitle() {
        // get the title of our console and return it
        static TCHAR szWindowTitle[256] = _T("");
        GetConsoleTitle(szWindowTitle, sizeof(szWindowTitle));

        return szWindowTitle;
    }

    HWND GetHWND() {
        return FindWindow(_T("ConsoleWindowClass"), GetTitle());
    }

    void DisableClose() {
        HWND hWnd = GetHWND();
        if (hWnd != NULL) {
            HMENU hMenu = GetSystemMenu(hWnd, 0);
            if (hMenu != NULL) {
                DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
                DrawMenuBar(hWnd);
            }

            //set debug windows top.
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
    }
};
#ifdef _DEBUG
static ConsoleOutput openConsole;
#endif // _DEBUG
#endif
