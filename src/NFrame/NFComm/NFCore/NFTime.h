// -------------------------------------------------------------------------
//    @FileName         :    NFTime.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFTime.h
 * @brief 时间处理类和日期结构定义
 * 
 * 此文件定义了时间处理的核心类和结构，提供了高精度的时间操作、
 * 时间转换、格式化等功能。支持Unix时间戳、本地时间、毫秒精度等。
 */

#pragma once

#include <string>

#include "NFPlatform.h"

#include <time.h>
#include <stdint.h>

/**
 * @brief 日期结构体
 * 
 * 与标准库的struct tm结构体相同，用于表示分解后的日期时间信息。
 * 包含年、月、日、时、分、秒以及星期、年中天数等信息。
 * 
 * 字段说明：
 * - sec: 秒（0-59）
 * - min: 分钟（0-59）
 * - hour: 小时（0-23）
 * - mday: 月中的日期（1-31）
 * - mon: 月份（0-11，0表示1月）
 * - year: 年份（从1900年开始的年数）
 * - wday: 星期（0-6，0表示星期天）
 * - yday: 年中的天数（0-365）
 * - isdst: 夏令时标志
 */
struct _NFExport NFDate
{
	/** @brief 秒（0-59） */
	int sec;
	/** @brief 分钟（0-59） */
	int min;
	/** @brief 小时（0-23） */
	int hour;
	/** @brief 月中的日期（1-31） */
	int mday;
	/** @brief 月份（0-11，0表示1月） */
	int mon;
	/** @brief 年份（从1900年开始的年数） */
	int year;
	/** @brief 星期（0-6，0表示星期天） */
	int wday;
	/** @brief 年中的天数（0-365） */
	int yday;
	/** @brief 夏令时标志（正数表示夏令时，0表示标准时间，负数表示信息不可用） */
	int isdst;
	/** @brief 填充字段，保持结构体对齐 */
	long int __padding[4];
};

/**
 * @brief 时间处理类
 * 
 * NFTime提供了完整的时间处理功能，包括时间创建、转换、格式化等。
 * 支持高精度时间（纳秒级别）和多种时间表示格式。
 * 
 * 主要功能：
 * - 时间创建：支持当前时间、指定日期、Unix时间戳等多种方式
 * - 时间转换：Unix时间戳、毫秒时间戳、纳秒时间戳之间的转换
 * - 时间格式化：支持自定义格式的时间字符串输出
 * - 本地时间：自动处理时区转换
 * - 系统时间：获取系统启动后的时间（Tick）
 * 
 * 使用方法：
 * @code
 * // 获取当前时间
 * NFTime now = NFTime::Now();
 * 
 * // 创建指定日期的时间
 * NFTime date(2023, 12, 25, 10, 30, 0);
 * 
 * // 获取Unix时间戳
 * time_t timestamp = now.UnixSec();
 * 
 * // 格式化输出
 * char timeStr[64];
 * now.LocalDateFormat("%Y-%m-%d %H:%M:%S", timeStr, sizeof(timeStr));
 * @endcode
 */
class _NFExport NFTime
{
protected:
	/** @brief Unix时间戳（秒） */
	time_t sec_;
	/** @brief 纳秒部分（0-999999999） */
	uint64_t nsec_;
	/** @brief 实际秒数（保留字段） */
	uint64_t realSec_;
	
	/**
	 * @brief 受保护的默认构造函数
	 * 初始化所有时间字段为0
	 */
	NFTime() : sec_(0), nsec_(0), realSec_(0) {}
public:
	/**
	 * @brief 创建表示当前时间的NFTime对象
	 * 
	 * 获取系统当前时间并创建NFTime对象，包含秒和纳秒精度。
	 * 
	 * @return NFTime 表示当前时间的NFTime对象
	 */
	static NFTime Now();

	/**
	 * @brief 将指定日期时间转换为Unix时间戳
	 * 
	 * 将年月日时分秒转换为对应的Unix时间戳（秒）。
	 * 
	 * @param year 年份（公历年份，如2023）
	 * @param month 月份（1-12）
	 * @param day 日期（1-31）
	 * @param hour 小时（0-23）
	 * @param min 分钟（0-59）
	 * @param sec 秒（0-59）
	 * @return uint64_t Unix时间戳（秒）
	 */
	static uint64_t GetSecTime(int year, int month, int day, int hour, int min, int sec);

	/**
	 * @brief 通过指定日期时间创建NFTime对象
	 * 
	 * 使用公历日期时间创建NFTime对象，年份为实际年份（非1900年开始）。
	 * 
	 * @param year 年份（公历年份，如2023）
	 * @param month 月份（1-12）
	 * @param day 日期（1-31）
	 * @param hour 小时（0-23）
	 * @param min 分钟（0-59）
	 * @param sec 秒（0-59）
	 */
	NFTime(int year, int month, int day, int hour, int min, int sec);

	/**
	 * @brief 通过毫秒时间戳创建NFTime对象
	 * 
	 * 使用毫秒时间戳创建NFTime对象，通常与UnixMSec()或Tick()返回值配合使用。
	 * 
	 * @param ms 毫秒时间戳
	 */
	explicit NFTime(uint64_t ms)
	{
		sec_ = ms / 1000;
		nsec_ = (ms % 1000) * 1000000;
		realSec_ = 0;
	}

	/**
	 * @brief 通过Unix时间戳和纳秒创建NFTime对象
	 * 
	 * 使用Unix时间戳（秒）和纳秒部分创建高精度的NFTime对象。
	 * 
	 * @param sec Unix时间戳（秒）
	 * @param nsec 纳秒部分（可以为0）
	 */
	NFTime(time_t sec, uint64_t nsec)
	{
		sec_ = sec;
		nsec_ = nsec;
		realSec_ = 0;
	}

	/**
	 * @brief 获取系统启动后经过的毫秒数
	 * 
	 * 返回系统启动以来经过的毫秒数，通常用于性能测量和时间间隔计算。
	 * 这个时间不受系统时间调整的影响。
	 * 
	 * @return uint64_t 系统启动后的毫秒数
	 */
	static uint64_t Tick();

	/**
	 * @brief 获取Unix时间戳（秒）
	 * 
	 * 返回从1970年1月1日0时0分0秒（UTC）开始的秒数。
	 * 
	 * @return time_t Unix时间戳（秒）
	 */
	time_t UnixSec();

	/**
	 * @brief 获取以周一0点为基准的Unix时间戳
	 * 
	 * 返回本周星期一0点0分0秒对应的Unix时间戳，
	 * 常用于周期性逻辑的时间计算。
	 * 
	 * @return time_t 周一0点的Unix时间戳
	 */
	time_t UnixSecOnMondayZero();

	/**
	 * @brief 获取Unix毫秒时间戳
	 * 
	 * 返回从1970年1月1日0时0分0秒（UTC）开始的毫秒数。
	 * 
	 * @return uint64_t Unix毫秒时间戳
	 */
	uint64_t UnixMSec();

	/**
	 * @brief 获取Unix纳秒时间戳
	 * 
	 * 返回从1970年1月1日0时0分0秒（UTC）开始的纳秒数，
	 * 提供最高精度的时间表示。
	 * 
	 * @return uint64_t Unix纳秒时间戳
	 */
	uint64_t UnixNano();

	/**
	 * @brief 将时间转换为本地日期结构
	 * 
	 * 将NFTime对象转换为本地时区的NFDate结构，
	 * 便于访问年、月、日等各个时间组件。
	 * 
	 * @param d [out] 输出的NFDate结构指针
	 */
	void LocalDate(struct NFDate *d);
	
	/**
	 * @brief 检查时间是否为零值
	 * 
	 * 判断NFTime对象是否表示零时间（即未初始化或为空的时间）。
	 * 
	 * @return bool 如果是零时间返回true，否则返回false
	 */
	bool IsZero();

	/**
	 * @brief 线程睡眠指定毫秒数
	 * 
	 * 使当前线程睡眠指定的毫秒数，这是一个静态工具方法。
	 * 
	 * @param ms 睡眠时间（毫秒）
	 */
	static void Sleep(uint32_t ms);

	/**
	 * @brief 格式化本地时间输出
	 * 
	 * 将NFTime对象按照指定格式转换为本地时间字符串。
	 * 
	 * 格式化示例：
	 * - "%Y-%m-%d[%H.%M.%S]" -> "2023-12-25[10.30.00]"
	 * - "%H:%M:%S" -> "10:30:00"
	 * 
	 * @param fmt 格式字符串（遵循strftime格式）
	 * @param timestr [out] 输出的时间字符串缓冲区
	 * @param len 缓冲区长度
	 * @return bool 格式化成功返回true，否则返回false
	 */
	bool LocalDateFormat(const char *fmt, char *timestr, size_t len);

	/**
	 * @brief 获取本地时间字符串（时:分:秒格式）
	 * 
	 * 将NFTime对象转换为"时:分:秒"格式的本地时间字符串。
	 * 
	 * @param timestr [out] 输出的时间字符串缓冲区
	 * @param len 缓冲区长度
	 * @return bool 格式化成功返回true，否则返回false
	 */
	bool LocalTime(char *timestr, size_t len)
	{
		return LocalDateFormat("%H:%M:%S", timestr, len);
	}

	/**
	 * @brief 获取本地时间字符串（时:分:秒格式）
	 * 
	 * 将NFTime对象转换为"时:分:秒"格式的本地时间字符串。
	 * 
	 * @return std::string 时间字符串
	 */
    std::string LocalTime()
    {
        char str[32];
        if (LocalTime(str, sizeof(str)))
        {
            return std::string(str);
        }
        return std::string();
    }

	/**
	 * @brief 获取本地日期时间字符串（缓冲区版本）
	 * 
	 * 将NFTime对象格式化为"年-月-日[时:分:秒]"格式的本地时间字符串。
	 * 
	 * @param timestr [out] 输出的时间字符串缓冲区
	 * @param len 缓冲区长度
	 * @return bool 格式化成功返回true，否则返回false
	 * 
	 * @note 输出格式：2023-12-25[10:30:00]
	 */
	bool LocalDateTime(char *timestr, size_t len)
	{
		return LocalDateFormat("%Y-%m-%d[%H:%M:%S]", timestr, len);
	}

	/**
	 * @brief 获取本地日期时间字符串（字符串版本）
	 * 
	 * 将NFTime对象格式化为"年-月-日[时:分:秒]"格式的本地时间字符串。
	 * 
	 * @return std::string 格式化后的时间字符串，失败时返回空字符串
	 * 
	 * @note 输出格式：2023-12-25[10:30:00]
	 */
    std::string LocalDateTime()
    {
        char str[32];
        if (LocalDateTime(str, sizeof(str)))
        {
            return std::string(str);
        }
        return std::string();
    }

	/**
	 * @brief 获取标准格式时间字符串（缓冲区版本）
	 * 
	 * 将NFTime对象格式化为"年-月-日 时:分:秒"格式的标准时间字符串。
	 * 
	 * @param timestr [out] 输出的时间字符串缓冲区
	 * @param len 缓冲区长度
	 * @return bool 格式化成功返回true，否则返回false
	 * 
	 * @note 输出格式：2023-12-25 10:30:00
	 */
	bool GetFormatTime(char *timestr, size_t len)
	{
		return LocalDateFormat("%Y-%m-%d %H:%M:%S", timestr, len);
	}

	/**
	 * @brief 获取格式化日期字符串（缓冲区版本）
	 * 
	 * 将NFTime对象格式化为"年-月-日"格式的日期字符串。
	 * 
	 * @param timestr [out] 输出的日期字符串缓冲区
	 * @param len 缓冲区长度
	 * @return bool 格式化成功返回true，否则返回false
	 * 
	 * @note 输出格式：2023-12-25
	 */
    bool GetFormatDate(char *timestr, size_t len)
    {
        return LocalDateFormat("%Y-%m-%d", timestr, len);
    }

	/**
	 * @brief 获取格式化日期字符串（字符串版本）
	 * 
	 * 将NFTime对象格式化为"年-月-日"格式的日期字符串。
	 * 
	 * @return std::string 格式化后的日期字符串，失败时返回空字符串
	 * 
	 * @note 输出格式：2023-12-25
	 */
    std::string GetFormatDate()
    {
        char str[32];
        if (GetFormatDate(str, sizeof(str)))
        {
            return std::string(str);
        }
        return std::string();
    }

	/**
	 * @brief 获取标准格式时间字符串（字符串版本）
	 * 
	 * 将NFTime对象格式化为"年-月-日 时:分:秒"格式的标准时间字符串。
	 * 
	 * @return std::string 格式化后的时间字符串，失败时返回空字符串
	 * 
	 * @note 输出格式：2023-12-25 10:30:00
	 */
    std::string GetFormatTime()
    {
        char str[32];
        if (GetFormatTime(str, sizeof(str)))
        {
            return std::string(str);
        }
        return std::string();
    }

	/**
	 * @brief 获取秒数部分
	 * 
	 * 返回NFTime对象中的Unix时间戳秒数部分。
	 * 
	 * @return uint64_t Unix时间戳（秒）
	 */
	uint64_t sec()
	{
		return sec_;
	}

	/**
	 * @brief 获取纳秒部分
	 * 
	 * 返回NFTime对象中的纳秒部分（不包括秒数）。
	 * 
	 * @return uint64_t 纳秒部分（0-999999999）
	 */
	uint64_t nsec()
	{
		return nsec_;
	}

	/**
	 * @brief 获取当前本地日期
	 * 
	 * 获取当前本地时间对应的日期结构体，包含年月日时分秒等信息。
	 * 
	 * @return struct NFDate 当前本地日期结构
	 */
	static struct NFDate GetLocalDate();
	
	/**
	 * @brief 获取当前本地日期的Unix时间戳
	 * 
	 * 获取当前本地日期（当天0点）对应的Unix时间戳。
	 * 
	 * @return uint64_t 当前本地日期0点的Unix时间戳（秒）
	 */
    static uint64_t  GetLocalDateSec();
    
	/**
	 * @brief 将Unix时间戳转换为本地日期
	 * 
	 * 将指定的Unix时间戳转换为本地时区的日期结构体。
	 * 
	 * @param unixSec Unix时间戳（秒）
	 * @return struct NFDate 对应的本地日期结构
	 */
	static struct NFDate GetLocalDate(uint64_t unixSec);

	/**
	 * @brief 计算两个时间戳的本地日期差值
	 * 
	 * 计算两个Unix时间戳在本地时区下相隔的天数。
	 * 
	 * @param left 第一个时间戳
	 * @param right 第二个时间戳
	 * @return int32_t 日期差值（天数），left较新时为正数
	 */
	static int32_t GetLocalDayDifference(uint64_t left, uint64_t right);
	
	/**
	 * @brief 获取每天更新的时间戳
	 * 
	 * 根据指定的时间戳和小时，计算对应的每日更新时间点。
	 * 
	 * @param timeSec 基准时间戳（秒）
	 * @param nHour 更新小时（0-23）
	 * @return uint64_t 每日更新时间的Unix时间戳（秒）
	 */
	static uint64_t GetDayUpdateTime(uint64_t timeSec, int32_t nHour);
	
	/**
	 * @brief 获取每周更新的时间戳
	 * 
	 * 根据指定的时间戳和小时，计算对应的每周更新时间点（周一）。
	 * 
	 * @param timeSec 基准时间戳（秒）
	 * @param nHour 更新小时（0-23）
	 * @return uint64_t 每周更新时间的Unix时间戳（秒）
	 */
	static uint64_t GetWeekUpdateTime(uint64_t timeSec, int32_t nHour);

	/**
	 * @brief 获取下周剩余时间
	 * 
	 * 计算从当前时间到下周开始还剩余的秒数。
	 * 
	 * @return uint64_t 到下周开始的剩余秒数
	 */
	static uint64_t GetNextWeekRemainingTime();

	/**
	 * @brief 获取下月剩余时间
	 * 
	 * 计算从当前时间到下月开始还剩余的秒数。
	 * 
	 * @return uint64_t 到下月开始的剩余秒数
	 */
	static uint64_t GetNextMonthRemainingTime();

	/**
	 * @brief 获取指定时间戳的当天零点时间
	 * 
	 * 计算指定时间戳所在日期的零点（00:00:00）时间戳。
	 * 
	 * @param timeSec 指定的时间戳（秒）
	 * @return uint64_t 对应日期零点的Unix时间戳（秒）
	 */
	static uint64_t GetZeroTime(uint64_t timeSec);

	/**
	 * @brief 获取指定时间戳的当月第一天零点时间
	 * 
	 * 计算指定时间戳所在月份第一天的零点（00:00:00）时间戳。
	 * 
	 * @param timeSec 指定的时间戳（秒）
	 * @return uint64_t 对应月份第一天零点的Unix时间戳（秒）
	 */
    static uint64_t GetMonthZeroTime(uint64_t timeSec);

	/**
	 * @brief 获取当前服务器所在的时区偏移
	 * 
	 * 获取当前服务器相对于UTC的时区偏移秒数。
	 * 
	 * @return int32_t 时区偏移秒数（东时区为正，西时区为负）
	 */
    static int32_t GetCurTimeZone();
};

/**
 * @name 时间宏定义
 * @brief 提供快速访问系统时间的宏定义
 * @{
 */

/**
 * @brief 获取系统时间（秒）
 * 
 * 通过NFServerTime单例获取当前系统时间的Unix时间戳。
 * 这是服务器全局统一的时间，确保各模块时间同步。
 * 
 * @return time_t 系统时间的Unix时间戳（秒）
 * 
 * @note 使用NFServerTime单例，性能较高
 * @note 适用于需要统一时间基准的场景
 */
#define NF_TIMENOW() (NFServerTime::Instance()->UnixSec())

/**
 * @brief 获取系统时间（毫秒）
 * 
 * 通过NFServerTime单例获取当前系统时间的毫秒时间戳。
 * 提供毫秒级精度的时间获取。
 * 
 * @return uint64_t 系统时间的毫秒时间戳
 * 
 * @note 使用NFServerTime单例，性能较高
 * @note 适用于需要高精度时间的场景
 */
#define NF_TIMENOW_MS() (NFServerTime::Instance()->UnixMSec())

/**
 * @brief 获取调整后的当前时间（秒）
 * 
 * 通过NFTime::Now()获取实时的系统时间，不依赖单例缓存。
 * 可能包含时间校正和调整。
 * 
 * @return time_t 调整后的当前时间Unix时间戳（秒）
 * 
 * @note 每次都获取实时时间，精度更高但性能稍低
 * @note 适用于对时间精度要求极高的场景
 */
#define NF_ADJUST_TIMENOW() NFTime::Now().UnixSec()

/**
 * @brief 获取调整后的当前时间（毫秒）
 * 
 * 通过NFTime::Now()获取实时的系统毫秒时间戳，不依赖单例缓存。
 * 提供最高精度的时间获取。
 * 
 * @return uint64_t 调整后的当前时间毫秒时间戳
 * 
 * @note 每次都获取实时时间，精度最高但性能较低
 * @note 适用于时间测量和高精度计时场景
 */
#define NF_ADJUST_TIMENOW_MS() NFTime::Now().UnixMSec()

/** @} */
