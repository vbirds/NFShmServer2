// -------------------------------------------------------------------------
//    @FileName         :    NFPlatform.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// ------------------------------------------------------------------------

/**
 * @file NFPlatform.h
 * @brief 平台相关的头文件和宏定义
 * 
 * 此文件包含了跨平台的宏定义、类型定义、断言函数以及平台特定的函数包装。
 * 主要用于屏蔽不同平台之间的差异，提供统一的接口。
 */

#pragma once

#include "NFMacros.h"
#include "NFPlatformMacros.h"
#include "common/spdlog/fmt/fmt.h"
#include "common/spdlog/fmt/bundled/printf.h"
#include <stdint.h>
#include <chrono>
#include <string>
#include <memory>
#include <stdio.h>
#include <string>
#include <thread>

///////////////////////////////////////////////////////////////
#include <string>
#include <algorithm>
#include <cmath>
#include <time.h>
#include <sstream>
#include <stdio.h>

#if NF_PLATFORM == NF_PLATFORM_LINUX
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

/** @brief EPOCH文件时间常量 (Linux) */
#define EPOCHFILETIME 11644473600000000ULL
#else
#include <windows.h>
#include <time.h>
#include <process.h>
/** @brief EPOCH文件时间常量 (Windows) */
#define EPOCHFILETIME 11644473600000000Ui64
#endif


using namespace std;


#include <assert.h>

/**
 * @brief 通用断言宏，当条件不满足时调用NFCommAssertFail
 * @param condition 需要检查的条件
 */
#define NF_COMM_ASSERT(condition)                    if (!(condition)) NFCommAssertFail(__FILE__, __LINE__); else { }

/**
 * @brief 带消息的断言宏，当条件不满足时调用NFCommAssertFail并显示消息
 * @param condition 需要检查的条件
 * @param msg 要显示的错误消息
 */
#define NF_COMM_ASSERT_MSG(condition, msg)            if (!(condition)) NFCommAssertFail(__FILE__, __LINE__, msg); else { }

/**
 * @brief 断言失败处理函数
 * 当断言失败时，此函数会输出文件名、行号和可选的错误消息
 * @param file 断言失败的文件名
 * @param line 断言失败的行号
 * @param message 可选的错误消息，默认为空字符串
 */
inline void NFCommAssertFail(const char *const file, const unsigned int line, const std::string &message = std::string())
{
    fprintf(stderr, "FAIL in %s (%d)", file, line);
    if (!message.empty())
    {
        fprintf(stderr, ": %s", message.c_str());
    }

    fprintf(stderr, "\n");
#ifdef NF_DEBUG_MODE
    assert(false);
#else
#endif
}

#if NF_PLATFORM == NF_PLATFORM_WIN
/** @brief Windows平台的字长定义 */
#define __WORDSIZE 64
/** @brief Windows平台的安全sprintf宏 */
#define NFSPRINTF sprintf_s
/** @brief Windows平台的字符串比较宏 */
#define NFSTRICMP stricmp
/** @brief Windows平台获取进程ID的宏 */
#define NFGetPID() getpid()
/** @brief Windows平台的线程ID类型定义 */
typedef unsigned int NF_THREAD_ID;

/**
 * @brief 获取当前线程ID (Windows平台)
 * @return 当前线程的ID
 */
inline NF_THREAD_ID ThreadId()
{
    return GetCurrentThreadId();
}

/**
 * @brief 线程安全的localtime函数 (Windows平台)
 * @param timep 指向time_t的指针
 * @param result 用于存储结果的tm结构体指针
 * @return 返回result指针
 */
inline struct tm* localtime_r(const time_t* timep, struct tm* result)
{
    localtime_s(result, timep);
    return result;
}

/** @brief Windows平台的字符串比较宏定义 */
#define strcasecmp   _stricmp
/** @brief Windows平台的字符串长度比较宏定义 */
#define strncasecmp  _strnicmp
#else
/** @brief Linux平台的sprintf宏 */
#define NFSPRINTF snprintf
/** @brief Linux平台的字符串比较宏 */
#define NFSTRICMP strcasecmp

/** @brief Linux平台获取进程ID的宏 */
#define NFGetPID() getpid()
/** @brief Linux平台的线程ID类型定义 */
typedef unsigned long int NF_THREAD_ID;

/**
 * @brief 获取当前线程ID (Linux平台)
 * @return 当前线程的ID
 */
inline NF_THREAD_ID ThreadId()
{
    return syscall(SYS_gettid);
}

#endif

#define NFSLEEP(us) std::this_thread::sleep_for(std::chrono::microseconds(us));

//use actor mode--begin
#define NF_ACTOR_THREAD_COUNT 3
#define NF_DEFAULT_MYSQL_DB_NAME "proto_ff"

#ifndef NF_USE_ACTOR
#define NF_USE_ACTOR
#endif

#ifdef NF_USE_ACTOR

#ifdef NF_DEBUG_MODE
#define THERON_DEBUG 1
#else
#define THERON_DEBUG 0
#endif

#ifndef THERON_CPP11
#define THERON_CPP11 1
#endif

#ifndef NF_ENABLE_SSL
#define NF_ENABLE_SSL 1
#endif

#ifndef USE_NET_EVPP
#define USE_NET_EVPP
#endif

#endif
//use actor mode--end

#define GET_CLASS_NAME(className) (#className)

#define NF_SHARE_PTR std::shared_ptr

#define NF_NEW new

#define  NF_SAFE_DELETE(pData) { try { delete pData; } catch (...) { NF_COMM_ASSERT_MSG(false, "NFSafeDelete error"); } pData=NULL; }

inline int64_t NFGetTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline int64_t NFGetSecondTime()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline int64_t NFGetMicroSecondTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline int64_t NFGetNanoSeccondTime()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

//
#ifndef NF_CASE_STRING_BIGIN
#define NF_CASE_STRING_BIGIN(state) switch(state){
#define NF_CASE_STRING(state) case state:return #state;break;
#define NF_CASE_STRING_END()  default:return "Unknown";break;}
#endif

#ifndef NF_FUNCTION_LINE
#define NF_FUNCTION_LINE __FUNCTION__, __LINE__
#endif

#ifndef NF_FILE_FUNCTION_LINE
#define NF_FILE_FUNCTION_LINE __FILE__, __FUNCTION__, __LINE__
#endif
#define NF_FORMAT(my_fmt, ...)             fmt::format(my_fmt, ##__VA_ARGS__)
#define NF_SPRINTF(my_fmt, ...)             fmt::sprintf(my_fmt, ##__VA_ARGS__)
#define NF_FORMAT_EXPR(str, my_fmt, ...)   try { str = fmt::format(my_fmt, ##__VA_ARGS__); } catch (fmt::format_error& error) { NFLogError(NF_LOG_DEFAULT, 0, "fmt:{} err:{}", my_fmt, error.what()); }

template<typename... ARGS>
inline std::string NFFormatFunc(const char* my_fmt, const ARGS& ... args)
{
    try {
        std::string str = fmt::format(my_fmt, args...);
        return str;
    }
    catch (fmt::format_error& error)
    {
        return my_fmt + std::string(" err:") + error.what();
    }
}

template<typename... ARGS>
inline std::string NFSprintfFunc(const char* my_fmt, const ARGS& ... args)
{
    try {
        std::string str = fmt::sprintf(my_fmt, args...);
        return str;
    }
    catch (fmt::format_error& error)
    {
        return my_fmt + std::string(" err:") + error.what();
    }
}

template<typename... ARGS>
inline std::string NFFormatFunc(const std::string& my_fmt, const ARGS& ... args)
{
    try {
        std::string str = fmt::format(my_fmt, args...);
        return str;
    }
    catch (fmt::format_error& error)
    {
        return my_fmt + std::string(" err:") + error.what();
    }
}

template<typename... ARGS>
inline std::string NFSprintfFunc(const std::string& my_fmt, const ARGS& ... args)
{
    try {
        std::string str = fmt::sprintf(my_fmt, args...);
        return str;
    }
    catch (fmt::format_error& error)
    {
        return my_fmt + std::string(" err:") + error.what();
    }
}

#define NF_FORMAT_FUNC(my_fmt, ...)             NFFormatFunc(my_fmt, ##__VA_ARGS__)
#define NF_SPRINTF_FUNC(my_fmt, ...)             NFFormatFunc(my_fmt, ##__VA_ARGS__)

#define MMO_LOWORD(l)           ((uint16_t)(l))
#define MMO_HIWORD(l)           ((uint16_t)(((uint32_t)(l) >> 16) & 0xFFFF))
#define MMO_LOBYTE(w)           ((uint8_t)(w))
#define MMO_HIBYTE(w)           ((uint8_t)(((uint16_t)(w) >> 8) & 0xFF))
#define MMO_LOWLONG(ll)         ((uint32_t)(ll))
#define MMO_HILONG(ll)           (uint32_t)(((uint64_t)(ll) >> 32) & 0xFFFFFFFF)
#define MMO_MAKEWORD(a, b)      ((uint16_t)(((uint8_t)(a))  | ((uint16_t)((uint8_t)(b))) << 8))
#define MMO_MAKELONG(a, b)      ((uint32_t)(((uint16_t)(a)) | ((uint32_t)((uint16_t)(b))) << 16))
#define MMO_MAKELONGLONG(a, b)  ((uint64_t)(((uint32_t)(a)) | ((uint64_t)((uint32_t)(b))) << 32))

#define MAKE_UINT32(low, high)    ((uint32_t)(((uint16_t)((uint32_t)(low) & 0xffff)) | ((uint32_t)((uint16_t)((uint32_t)(high) & 0xffff))) << 16))
#define HIGH_UINT16(l) ((uint16_t)((uint32_t)(l) >> 16))
#define LOW_UINT16(l) ((uint16_t)((uint32_t)(l) & 0xffff))

#define MAKE_UINT64(low, high)    ((uint64_t)(((uint32_t)((uint64_t)(low) & 0xffffffff)) | ((uint64_t)((uint32_t)((uint64_t)(high) & 0xffffffff))) << 32))
#define HIGH_UINT32(l) ((uint32_t)((uint64_t)(l) >> 32))
#define LOW_UINT32(l) ((uint32_t)((uint64_t)(l) & 0xffffffff))

#define BIT_ENABLED(AWORD, BITS) (((AWORD) & (BITS)) == (BITS))
#define BIT_DISABLED(AWORD, BITS) (((AWORD) & (BITS)) == 0)
#define SET_BITS(AWORD, BITS) ((AWORD) |= (BITS))
#define CLR_BITS(AWORD, BITS) ((AWORD) &= ~(BITS))

template<typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];


#ifndef _MSC_VER

template<typename T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];

#endif


#ifndef _MSC_VER
#define ARRAYSIZE(array) (sizeof(ArraySizeHelper(array)))
#endif

#define ARRAYSIZE_UNSAFE(a) \
    ((sizeof(a) / sizeof(*(a))) / \
     static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#ifndef NF_DEBUG_MODE
#define TEST_SERVER
#endif

#ifndef NF_MACRO_FUNCTION
#if NF_PLATFORM == NF_PLATFORM_WIN
//#define NF_MACRO_FUNCTION __FUNCSIG__
#define NF_MACRO_FUNCTION __FUNCTION__
#else
//#define NF_MACRO_FUNCTION __PRETTY_FUNCTION__
#define NF_MACRO_FUNCTION __FUNCTION__
#endif
#endif

//时间轴检查频率 ms
#define TIMER_AXIS_CHECK_FREQUENCE 32
//时间轴刻度
#define TIME_GRID 64
//时间轴长度
#define TIME_AXIS_LENGTH            120000        // 毫秒为单位的
#define TIME_AXIS_SECLENGTH            108000        // 秒为单位的支持到30个小时
#define INVALID_TIMER                0xffffffff  // 无效定时器
#define INFINITY_CALL                0xffffffff    // 调用无限次

#define FIND_MODULE(classBaseName, className)  \
assert((TIsDerived<classBaseName, NFIModule>::Result));

template<typename DerivedType, typename BaseType>
class TIsDerived {
public:
    static int AnyFunction(BaseType *base) {
        return 1;
    }

    static char AnyFunction(void *t2) {
        return 0;
    }

    enum {
        Result = sizeof(int) == sizeof(AnyFunction((DerivedType *) NULL)),
    };
};