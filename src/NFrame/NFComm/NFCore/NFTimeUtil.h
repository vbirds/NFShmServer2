// -------------------------------------------------------------------------
//    @FileName         :    NFTimeUtil.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTimeUtil.h
//
// -------------------------------------------------------------------------

/**
 * @file NFTimeUtil.h
 * @brief 时间工具类
 * 
 * 此文件提供了时间处理的基础工具类和函数，包括时间转换、格式化、
 * 比较操作等。特别针对游戏开发中的时间处理需求进行了优化，
 * 提供了本地时间和UTC时间的转换支持。
 */

#pragma once

#include "NFPlatform.h"

#include <stdlib.h>
#include <stdio.h>
#include "NFSocketLibFunction.h"


/** @brief 时间值类型定义，基于标准timeval结构 */
typedef struct timeval TTimeVal;

/**
 * @brief 时间相关常量枚举
 * 
 * 定义了游戏开发中常用的时间常量，包括时区偏移、
 * 时间间隔和重置时间点等。
 */
enum
{
    /** @brief 本地时间时区偏移小时数（东八区） */
    LOCAL_TIME_CORRECTION_HOUR = 8,
    /** @brief 本地时间时区偏移秒数（东八区偏移UTC+8） */
    LOCAL_TIME_CORRECTION = LOCAL_TIME_CORRECTION_HOUR * 3600,
    /** @brief 一天的总秒数（24小时） */
    SECONDS_ADAY = 24 * 3600,
    /** @brief 一周的总秒数（7天） */
    SECONDS_AWEEK = 24 * 3600 * 7,
    /** @brief 一小时的总秒数 */
    SECONDS_AHOUR = 3600,
    /** @brief 半小时的总秒数 */
    SECONDS_HALF_AHOUR = 1800,
    /** @brief 一周的总小时数 */
    HOURS_AWEEK = 24 * 7,
    /** @brief 一个月的总秒数（按30天计算） */
    SECONDS_AMONTH = 24 * 3600 * 30,
    /** @brief 游戏每日重置的小时数（凌晨0点） */
    GAME_RESET_HOUR_EVERYDAY = 0,
    /** @brief 游戏每日重置的分钟数 */
    GAME_RESET_MIN_EVERYDAY = 0,
    /** @brief 游戏每日重置的秒数 */
    GAME_RESET_SEC_EVERYDAY = 0,
};

/**
 * @brief TTimeVal小于比较操作符
 * 
 * 比较两个时间值的大小，考虑微秒精度。
 * 
 * @param lhs 左侧时间值
 * @param rhs 右侧时间值
 * @return bool lhs小于rhs时返回true
 */
inline bool operator<(const TTimeVal& lhs, const TTimeVal& rhs)
{
    return ((lhs.tv_sec - rhs.tv_sec) * 1000000 + (lhs.tv_usec - rhs.tv_usec)) < 0;
}

/**
 * @brief TTimeVal小于等于比较操作符
 * 
 * @param lhs 左侧时间值
 * @param rhs 右侧时间值
 * @return bool lhs小于等于rhs时返回true
 */
inline bool operator<=(const TTimeVal& lhs, const TTimeVal& rhs)
{
    return !(rhs < lhs);
}

/**
 * @brief TTimeVal相等比较操作符
 * 
 * @param lhs 左侧时间值
 * @param rhs 右侧时间值
 * @return bool lhs等于rhs时返回true
 */
inline bool operator==(const TTimeVal& lhs, const TTimeVal& rhs)
{
    return (!(lhs < rhs) && !(rhs < lhs));
}

/**
 * @brief TTimeVal减法操作符
 * 
 * 计算两个时间值的差值，自动处理微秒借位。
 * 
 * @param lhs 被减数时间值
 * @param rhs 减数时间值
 * @return TTimeVal 时间差值
 */
inline TTimeVal operator-(const TTimeVal& lhs, const TTimeVal& rhs)
{
    TTimeVal tvGap;
    tvGap.tv_sec = lhs.tv_sec - rhs.tv_sec;
    tvGap.tv_usec = lhs.tv_usec - rhs.tv_usec;

    if (tvGap.tv_usec < 0)
    {
        tvGap.tv_usec += 1000000;
        tvGap.tv_sec -= 1;
    }

    return tvGap;
}

/**
 * @brief 时间工具类
 * 
 * NFTimeUtil提供了丰富的时间处理功能，专门为游戏开发设计。
 * 包括时间格式转换、字符串格式化、游戏时间计算等功能。
 * 
 * 主要功能：
 * - 时间格式转换：time_t、tm结构、字符串之间的转换
 * - 多语言支持：支持中文简体日期时间格式
 * - 游戏时间：按游戏规则的时间计算和重置
 * - 高精度时间：支持微秒级时间处理
 * - 时区处理：本地时间和UTC时间转换
 * 
 * 适用场景：
 * - 游戏服务器时间管理
 * - 日志时间戳格式化
 * - 定时任务调度
 * - 游戏事件时间计算
 * - 跨时区时间同步
 * 
 * 使用方法：
 * @code
 * // 获取当前时间字符串
 * time_t now = time(nullptr);
 * char* timeStr = NFTimeUtil::SecondToStr(now);
 * 
 * // 中文格式时间
 * const char* cnTime = NFTimeUtil::DataTimeToStrSimCN(now);
 * 
 * // 时间格式转换
 * char buffer[64];
 * int bufSize = sizeof(buffer);
 * NFTimeUtil::DateTimeToStr_R(&now, buffer, &bufSize);
 * @endcode
 * 
 * @note 所有方法都是静态方法，不需要创建实例
 * @note 线程安全性取决于具体方法的实现
 * @note 部分方法使用静态缓冲区，不适合多线程并发使用
 */
class NFTimeUtil
{
public:
    /**
     * @brief 将时间转换为日期时间字符串（线程安全版本）
     * 
     * 将time_t时间值转换为可读的日期时间字符串格式。
     * 支持仅日期模式和完整日期时间模式。
     * 
     * @param mytime 指向time_t类型的指针，表示需要转换的时间
     * @param s 用于存储转换后日期时间字符串的字符数组
     * @param pio 指向整型变量的指针，输入缓冲区大小，输出实际使用长度
     * @param bOnlyDay 可选参数，若为true，则仅转换为日期格式，默认为false
     * @return char* 返回指向存储转换后日期时间字符串的字符数组的指针
     * 
     * @note 线程安全，使用调用者提供的缓冲区
     * @note 缓冲区应至少分配32字节以确保足够空间
     * @note 当bOnlyDay为true时，仅输出年月日信息
     */
    static char* DateTimeToStr_R(time_t* mytime, char* s, int* pio, bool bOnlyDay = false);

    /**
     * @brief 将时间转换为简化中文日期时间字符串（线程安全版本）
     * 
     * 将时间转换为中文简体格式的日期时间字符串，
     * 便于中文用户界面显示。
     * 
     * @param mytime 指向time_t类型的指针，表示需要转换的时间
     * @param s 用于存储转换后日期时间字符串的字符数组
     * @param pio 指向整型变量的指针，输入缓冲区大小，输出实际使用长度
     * @return const char* 返回指向存储转换后简化中文日期时间字符串的字符数组的指针
     * 
     * @note 输出格式类似"2023年12月25日 14:30:25"
     * @note 线程安全，使用调用者提供的缓冲区
     */
    static const char* DateTimeToStrSimCN_R(time_t* mytime, char* s, int* pio);

    /**
     * @brief 将秒数转换为日期时间字符串
     * 
     * 将time_t秒数直接转换为标准的日期时间字符串格式。
     * 
     * @param mytime 表示需要转换的秒数（Unix时间戳）
     * @return char* 返回指向存储转换后日期时间字符串的字符数组的指针
     * 
     * @warning 此方法使用静态缓冲区，不是线程安全的
     * @note 返回的指针指向内部静态缓冲区，下次调用会覆盖
     * @note 输出格式为标准的"YYYY-MM-DD HH:MM:SS"格式
     */
    static char* SecondToStr(time_t mytime);

    /**
     * @brief 将时间转换为简化中文日期时间字符串
     * 
     * 将时间值转换为中文简体显示格式，适合用户界面展示。
     * 
     * @param tmytime 表示需要转换的时间（Unix时间戳）
     * @return const char* 返回指向存储转换后简化中文日期时间字符串的字符数组的指针
     * 
     * @warning 此方法使用静态缓冲区，不是线程安全的
     * @note 返回的指针指向内部静态缓冲区
     * @note 输出格式为中文格式，包含年月日时分秒
     */
    static const char* DataTimeToStrSimCN(time_t tmytime);

    /**
     * @brief 将微秒精度的时间转换为字符串
     * 
     * 将TTimeVal结构体表示的微秒精度时间转换为字符串格式。
     * 
     * @param tvTime 表示需要转换的微秒精度时间
     * @param pszOut 可选参数，用于存储转换后时间字符串的字符数组，默认为NULL
     * @param iOutLen 可选参数，表示pszOut的长度，默认为128
     * @return char* 返回指向存储转换后时间字符串的字符数组的指针
     * 
     * @note 线程安全，使用调用者提供的缓冲区
     * @note 缓冲区应至少分配128字节以确保足够空间
     */
    static char* USecondTimeToStr(const TTimeVal& tvTime, char* pszOut = NULL, int iOutLen = 128);

    /**
     * @brief 将时间转换为日期时间字符串
     * 
     * 将time_t时间值转换为标准的日期时间字符串格式。
     * 
     * @param mytime 指向time_t类型的指针，表示需要转换的时间
     * @return char* 返回指向存储转换后日期时间字符串的字符数组的指针
     * 
     * @warning 此方法使用静态缓冲区，不是线程安全的
     * @note 返回的指针指向内部静态缓冲区，下次调用会覆盖
     * @note 输出格式为标准的"YYYY-MM-DD HH:MM:SS"格式
     */
    static char* DateTimeToStr(time_t* mytime);

    /**
     * @brief 将时间转换为日期时间字符串
     * 
     * 将time_t时间值转换为可读的日期时间字符串格式，
     * 支持仅日期模式和完整日期时间模式。
     * 
     * @param mytime 表示需要转换的时间
     * @param bOnlyDay 可选参数，若为true，则仅转换为日期格式，默认为false
     * @return char* 返回指向存储转换后日期时间字符串的字符数组的指针
     * 
     * @warning 此方法使用静态缓冲区，不是线程安全的
     * @note 返回的指针指向内部静态缓冲区，下次调用会覆盖
     * @note 当bOnlyDay为true时，仅输出年月日信息
     */
    static char* DateTimeToStr(time_t mytime, bool bOnlyDay = false);

    /**
     * @brief 将整型时间转换为日期时间字符串
     * 
     * 将int类型的时间值转换为标准的日期时间字符串格式。
     * 
     * @param imytime 表示需要转换的整型时间
     * @return char* 返回指向存储转换后日期时间字符串的字符数组的指针
     * 
     * @warning 此方法使用静态缓冲区，不是线程安全的
     * @note 返回的指针指向内部静态缓冲区，下次调用会覆盖
     * @note 输出格式为标准的"YYYY-MM-DD HH:MM:SS"格式
     */
    static char* DateTimeToStr(int imytime);

    // 将uint32_t类型的时间转换为字符串表示
    // 参数dwMytime: 代表时间的uint32_t类型变量
    // 返回值: 转换后的字符串指针
    static char* DateTimeToStrDw(uint32_t dwMytime);

    // 将time_t类型的时间转换为字符串表示
    // 参数mytime: 指向time_t类型的时间变量
    // 参数piIn: 用于输出的字符数组指针
    // 返回值: 转换后的字符串指针
    static char* DateTimeToStr(time_t* mytime, char* piIn);

    // 将当前时间转换为字符串表示
    // 参数tNow: 表示当前时间的time_t类型变量
    // 返回值: 转换后的字符串指针
    static char* CurTimeToStr(time_t tNow);

    // xxxx 20160325 修改时间由外部传入，因为时间可能不是time（NULL）取到的当前时间
    // 将当前时间转换为字符串表示，带缓冲区长度参数
    // 参数tNow: 表示当前时间的time_t类型变量
    // 参数pszDateTime: 用于输出的字符数组指针
    // 参数piInOutLen: 指向缓冲区长度的整型指针
    // 返回值: 转换后的字符串指针
    static char* CurTimeToStr_R(time_t tNow, char* pszDateTime, int* piInOutLen);


    /*
    //note: rhs 应该是间隔,而不是实际的时间,否则出错
    //因为返回值是32位。只能表示24天
    inline unsigned int ToMs(const TTimeVal &rhs)
    {
        unsigned int m_tmp =0;
        m_tmp = rhs.tv_sec * 1000;
        m_tmp = m_tmp + rhs.tv_usec / 1000;
        return m_tmp;
    }
    */
    static inline void AddMsWithPointer(TTimeVal* plhs, int iMs)
    {
        plhs->tv_sec += (iMs / 1000);
        plhs->tv_usec += ((iMs % 1000) * 1000);
        plhs->tv_sec += (plhs->tv_usec / 1000000);
        plhs->tv_usec = plhs->tv_usec % 1000000;
    }

    // Adds milliseconds to a TTimeVal structure
    static inline void AddMs(TTimeVal& lhs, int iMs)
    {
        AddMsWithPointer(&lhs, iMs);
    }

    // Adds microseconds to a TTimeVal structure, adjusting seconds as necessary
    static inline void AddUsWithPointer(TTimeVal* plhs, int iUs)
    {
        plhs->tv_usec += iUs;
        plhs->tv_sec += (plhs->tv_usec / 1000000);
        plhs->tv_usec = plhs->tv_usec % 1000000;
    }

    // Adds microseconds to a TTimeVal structure
    static inline void AddUs(TTimeVal& lhs, int iUs)
    {
        AddUsWithPointer(&lhs, iUs);
    }

    // Converts a string in the format "YYYY-MM-DD HH:MM:SS" to a time_t value
    static int StrToTime(const char* psztime, time_t* ptime);

    // Converts a string in the format "YYYY-MM-DD HH:MM:SS" to a time_t value without additional parameters
    static time_t StrToTimePure(const char* psztime);

    // Retrieves the system time in a specified buffer
    static int SysTime(char* pszBuff, int iBuff);

    // Converts __DATE__ and __TIME__ macros to a time_t value
    static time_t __DATE__TIME_toTime(const char* sz__DATE__, const char* sz__TIME__);

    // Retrieves the current time in microseconds
    static uint64_t GetCurrTimeUs();

    // Calculates the time difference in milliseconds between two timeval structures
    static inline unsigned int TimeMsPass(struct timeval* pstTv1, struct timeval* pstTv2)
    {
        int iSec;
        iSec = pstTv1->tv_sec - pstTv2->tv_sec;

        if (iSec < 0 || iSec > 100000)
        {
            iSec = 100000;
        }

        return iSec * 1000 + (pstTv1->tv_usec - pstTv2->tv_usec) / 1000;
    }

    // Calculates the time difference in microseconds between two timeval structures
    static inline int64_t TimeUsPass(struct timeval* pstTv1, struct timeval* pstTv2)
    {
        int iSec;
        iSec = pstTv1->tv_sec - pstTv2->tv_sec;

        if (iSec < 0 || iSec > 100000)
        {
            iSec = 100000;
        }

        return iSec * 1000000 + (pstTv1->tv_usec - pstTv2->tv_usec);
    }

    // Retrieves the current time of day in milliseconds
    static uint64_t GetTimeOfDayMS();

    /**
     * @brief 将时间转换为字符串表示
     * 
     * 将TTimeVal结构体表示的时间转换为可读的字符串格式。
     * 
     * @param pstCurr 指向时间结构体的指针，用于获取时间信息
     * @param pszString 用于存储时间字符串的字符数组指针，默认为NULL
     * @param iMaxLen 字符数组的最大长度，默认为64
     * @return char* 返回指向存储时间字符串的字符数组指针
     * 
     * @note 线程安全，使用调用者提供的缓冲区
     * @note 缓冲区应至少分配64字节以确保足够空间
     */
    static char* TimeToStr(const TTimeVal* pstCurr, char* pszString = NULL, int iMaxLen = 64);

    /**
     * @brief 获取当前时间的字符串表示
     * 
     * 将TTimeVal结构体表示的当前时间转换为可读的字符串格式。
     * 
     * @param pstCurr 指向时间结构体的指针，用于获取当前时间信息
     * @return char* 返回指向存储当前时间字符串的字符数组指针
     * 
     * @note 线程安全，使用调用者提供的缓冲区
     * @note 缓冲区应至少分配64字节以确保足够空间
     */
    static char* CurrTimeStr(const TTimeVal* pstCurr);

    /**
     * @brief 生成一个表示时间的短整型，其中前7位表示年份（从2000年开始），中间4位表示月份，最后5位表示日期
     * 
     * 将time_t类型的时间值转换为短整型，用于快速存储日期信息。
     * 
     * @param tTime 表示时间的时间_t类型变量
     * @return unsigned short 返回表示时间的短整型
     * 
     * @note 返回值包含年份（从2000年开始）、月份和日期
     */
    static unsigned short MakeShortTime(time_t tTime);

    /**
     * @brief 判断两个时间是否为游戏重置时间的同一天
     * 
     * 判断两个时间是否在游戏每日重置时间点（00:00:00）的同一天。
     * 
     * @param tCur 第一个时间的时间_t类型变量
     * @param tBefore 第二个时间的时间_t类型变量
     * @return bool 如果两个时间是游戏重置时间的同一天，则返回true；否则返回false
     */
    static bool IsSameDayByGameResetTime(time_t tCur, time_t tBefore);

    /**
     * @brief 获取给定时间的绝对周数
     * 
     * 计算从1970年1月1日到给定时间的总周数。
     * 
     * @param tTime 表示时间的时间_t类型变量
     * @return uint32_t 返回给定时间的绝对周数
     */
    static uint32_t GetAbsWeek(time_t tTime);

    /**
     * @brief 获取给定时间的绝对天数
     * 
     * 计算从1970年1月1日到给定时间的总天数。
     * 
     * @param tTime 表示时间的时间_t类型变量
     * @return uint32_t 返回给定时间的绝对天数
     */
    static uint32_t GetAbsDay(time_t tTime);

    /**
     * @brief 获取给定时间所在周的开始时间（以绝对时间表示）
     * 
     * 获取给定时间所在周的开始时间（以Unix时间戳表示），
     * 即该周的周一00:00:00。
     * 
     * @param tTime 表示时间的时间_t类型变量
     * @return uint32_t 返回给定时间所在周的开始时间（以绝对时间表示）
     */
    static uint32_t GetThisWeekStartTime(time_t tTime);

    static uint32_t GetThisWeekEndTime(time_t tTime);

    /**
     * @brief 判断两个时间是否为同一个月份（考虑时区偏移）
     * 
     * 判断两个时间是否在同一月份，并考虑时区偏移。
     * 
     * @param tTimeA 第一个时间的时间_t类型变量
     * @param tTimeB 第二个时间的时间_t类型变量
     * @param iOffsetHour 时区偏移小时数
     * @return bool 如果两个时间是同一个月份，则返回true；否则返回false
     */
    static bool IsSameMonthWithOffsetHour(time_t tTimeA, time_t tTimeB, int iOffsetHour);

    /**
     * @brief 判断两个时间是否为同一个月份
     * 
     * 判断两个时间是否在同一月份，不考虑时区偏移。
     * 
     * @param tTimeA 第一个时间的时间_t类型变量
     * @param tTimeB 第二个时间的时间_t类型变量
     * @return bool 如果两个时间是同一个月份，则返回true；否则返回false
     */
    static bool IsSameMonth(time_t tTimeA, time_t tTimeB);


    //根据周几和偏移时间值,拼接当前的绝对时间. bWeekDay 里 0 表示周一.
    //xxxx ,2013-03-21
    static uint32_t GetAbsTimeByWeekDayAndTime(time_t tNow, uint8_t bWeekDay, uint32_t dwWeekTime);

    // 获取自然年
    static uint16_t GetTimeYear(time_t tTime);

    // 获取星期几
    static uint16_t GetWeekDay(time_t tTime);

    // 获取当前星期几
    static uint16_t GetWeekDay();
    static uint16_t GetWeekDay127(time_t tTime);

    // 获取月份中的第几天
    static uint16_t GetMonthDay(time_t tTime);

    // 获取当前月份中的第几天
    static uint16_t GetMonthDay();

    // 获取月份
    static uint16_t GetMonth(time_t tTime);

    // 获取小时
    static uint16_t GetHour(time_t tTime);

    // 获取一天中的绝对秒数
    static uint32_t GetDayAbsSec(time_t tTime);

    // 将格式化的日期时间字符串转换为UTC时间
    // 格式：YYYY-MM-DD-HH-MM-SS
    static time_t time_str_to_utc(char* szInput);

    // 获取今天开始的时间，考虑到时区差异
    static time_t GetTodayStartTime(time_t tTimeNow, int iHour = 0);

    // 将日期时间字符串转换为本地时间
    // 新版本
    static int DataStrToLocalTimeNew(const char* pStr, time_t* pTime);

    // 将日期字符串转换为本地时间
    static int DateStrToLocalTime(const char* pStr, time_t* pTime);

    // 获取时间的秒部分
    static int GetTimeSec(const char* pStr, time_t* pTime);

    // 扩展版本，获取时间的秒部分
    static int GetTimeSecEx(const char* pStr, time_t* pTime);

    // dwUnixSec所在日的开始时刻
    static uint32_t GetDayStartUnixSec(uint32_t dwUnixSec);
    static uint32_t GetMonthStartUnixSec(uint32_t dwUnixSec);
    static uint32_t GetYearStartUnixSec(uint32_t dwUnixSec);
    // dwUnixSec在本日的秒数 00:00:00=0 00:00:01=1
    static uint32_t GetCurDaySec(uint32_t dwUnixSec);

    // 按4点算跨天, 获取到1970-01-01的天数
    //                     - 1970-01-02 03:59:00 return 0
    // 1970-01-02 04:00:00 - 1970-01-03 03:59:00 return 1
    // ...
    static uint32_t GetCurDay04(uint32_t dwUnixSec);
    // 从今年1月1日到目前的天数，范围0-365
    static uint16_t GetYearDay(time_t tTime);
};
