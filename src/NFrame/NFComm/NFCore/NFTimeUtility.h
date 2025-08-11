// -------------------------------------------------------------------------
//    @FileName         :    NFMagicTimeUtil.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMagicTimeUtil.h
//
// -------------------------------------------------------------------------

/**
 * @file NFTimeUtility.h
 * @brief 时间实用工具类
 * 
 * 此文件提供了一套完整的时间处理工具函数，包括时区转换、日期计算、
 * 时间格式化、周期性检查等功能。主要用于游戏服务器中的时间逻辑处理。
 */

#pragma once

#include "NFStringUtility.h"
#include "NFTimeUtil.h"

/**
 * @brief 时间实用工具类
 * 
 * NFTimeUtility提供了丰富的时间处理功能，专门为游戏服务器设计。
 * 主要功能包括：
 * 
 * - 时区处理：GMT和本地时间的转换
 * - 日期计算：日、周、月的计算和比较
 * - 时间格式化：多种格式的时间字符串生成
 * - 周期检查：日更新、周更新、月更新检查
 * - 时间解析：字符串时间戳转换为Unix时间
 * 
 * 适用场景：
 * - 游戏日常更新逻辑
 * - 活动时间判断
 * - 日志时间格式化
 * - 玩家数据重置周期
 * 
 * 使用方法：
 * @code
 * // 获取当前本地时间
 * uint32_t localTime = NFTimeUtility::GetLocalTime();
 * 
 * // 检查是否需要日更新
 * bool needUpdate = NFTimeUtility::CheckDayUpdate(currentTime, lastUpdateTime);
 * 
 * // 格式化时间戳
 * std::string timeStr = NFTimeUtility::GetLocalTimeStampPrint();
 * 
 * // 判断是否是同一周
 * bool sameWeek = NFTimeUtility::CheckSameWeek(time1, time2);
 * @endcode
 */
class NFTimeUtility
{
public:
    /**
     * @brief 获取GMT时间偏移秒数
     * 
     * 获取当前时区相对于GMT（格林威治标准时间）的时差秒数。
     * 正数表示东时区，负数表示西时区。
     * 
     * @return uint32_t GMT时间偏移秒数
     */
    static uint32_t GetGMTSec();
    
    /**
     * @brief 获取本地时间（Unix时间戳）
     * 
     * 获取当前本地时间的Unix时间戳（从1970年1月1日0时0分0秒开始的秒数）。
     * 
     * @return uint32_t 本地时间的Unix时间戳
     */
    static uint32_t GetLocalTime();
    
    /**
     * @brief 根据当前时间计算本地天数
     * 
     * 根据当前本地时间计算从1970年1月1日开始的天数。
     * - 1970年1月1日 返回0
     * - 1970年1月2日 返回1
     * - 1970年1月3日 返回2
     * 依此类推
     * 
     * @return uint32_t 本地天数
     */
    static uint32_t GetLocalDay();
    
    /**
     * @brief 根据指定Unix时间戳计算本地天数
     * 
     * @param unixSec Unix时间戳（秒）
     * @return uint32_t 本地天数
     */
    static uint32_t GetLocalDay(uint64_t unixSec);
    
    /**
     * @brief 根据当前时间计算本地周数
     * 
     * 计算从1970年1月1日（星期四）开始的周数，以星期一为一周的开始。
     * - 1970年1月1日 星期四 返回0
     * - 1970年1月4日 星期日 返回0
     * - 1970年1月5日 星期一 返回1
     * 
     * @return uint32_t 本地周数
     */
    static uint32_t GetLocalWeek();
    
    /**
     * @brief 根据指定Unix时间戳计算本地周数
     * 
     * @param unixSec Unix时间戳（秒）
     * @return uint32_t 本地周数
     */
    static uint32_t GetLocalWeek(uint64_t unixSec);
    
    /**
     * @brief 根据指定Unix时间戳计算本地月份
     * 
     * 计算指定时间对应的月份数（从1970年1月开始计算）。
     * 
     * @param unixSec Unix时间戳（秒）
     * @return uint32_t 本地月份数
     */
    static uint32_t GetLocalMonth(uint64_t unixSec);
    
    /**
     * @brief 根据当前本地时间计算星期几
     * 
     * 计算当前本地时间是一周中的哪一天。
     * 
     * @return uint32_t 星期几（1-7，1表示星期一，7表示星期天）
     */
    static uint32_t GetLocalWeekDay();
    
    /**
     * @brief 根据指定Unix时间戳计算星期几
     * 
     * @param unixSec Unix时间戳（秒）
     * @return uint32_t 星期几（1-7，1表示星期一，7表示星期天）
     */
    static uint32_t GetLocalWeekDay(uint64_t unixSec);
    
    /**
     * @brief 根据当前本地时间计算月中的日期
     * 
     * 计算当前本地时间是本月的第几天。
     * 
     * @return uint32_t 月中的日期（1-31）
     */
    static uint32_t GetLocalMonthDay();
    
    /**
     * @brief 判断是否为闰年
     * 
     * 根据闰年规则判断指定年份是否为闰年：
     * - 能被4整除但不能被100整除的年份是闰年
     * - 能被400整除的年份是闰年
     * 
     * @param year 年份
     * @return bool 如果是闰年返回true，否则返回false
     */
    static bool IsLeapYear(uint32_t year) { return ((0 == year % 4 && 0 != year % 100) || 0 == year % 400); }
    
    /**
     * @brief 获取指定年月的天数
     * 
     * 根据年份和月份计算该月有多少天，会考虑闰年的情况。
     * 
     * @param year 年份
     * @param month 月份（1-12）
     * @return uint32_t 该月的天数（28-31）
     */
    static uint32_t GetDaysOfMonth(int year, int month);
    
    /**
     * @brief 获取当前月份的天数
     * 
     * 根据当前本地时间计算当前月份有多少天。
     * 
     * @return uint32_t 当前月份的天数（28-31）
     */
    static uint32_t GetDaysOfMonth();
    
    /**
     * @brief 格式化本地时间
     * 
     * 将当前本地时间按照指定格式转换为字符串。
     * 
     * @param fmt 格式字符串（如"%Y%m%d%H%M%S"）
     * @param timeStr [out] 输出的时间字符串缓冲区
     * @param len 缓冲区长度
     * @return bool 格式化成功返回true，否则返回false
     */
    static bool LocalDateFormat(const char* fmt, char* timeStr, size_t len);
    
    /**
     * @brief 获取本地时间戳字符串
     * 
     * 返回当前本地时间的时间戳字符串，格式为：20150911111926
     * 
     * @return std::string 时间戳字符串
     */
    static std::string GetLocalTimeStamp();
    
    /**
     * @brief 将Unix毫秒时间戳转换为时间戳字符串
     * 
     * @param unixMSec Unix毫秒时间戳
     * @return std::string 时间戳字符串（格式：20150911111926）
     */
    static std::string GetTimeStamp(uint64_t unixMSec);
    
    /**
     * @brief 获取可打印的本地时间戳字符串
     * 
     * 返回当前本地时间的可读时间戳字符串，格式为：2015_09_11 11:19:26
     * 
     * @return std::string 可读的时间戳字符串
     */
    static std::string GetLocalTimeStampPrint();
    
    /**
     * @brief 将Unix毫秒时间戳转换为可打印的时间戳字符串
     * 
     * @param unixMSec Unix毫秒时间戳
     * @return std::string 可读的时间戳字符串（格式：2015_09_11 11:19:26）
     */
    static std::string GetTimeStampPrint(uint64_t unixMSec);
    
    /**
     * @brief 将本地时间戳字符串转换为Unix秒时间戳
     * 
     * 将格式为20150911111926的时间戳字符串转换为Unix秒时间戳。
     * 
     * @param timestamp 时间戳字符串（格式：20150911111926）
     * @return uint64_t Unix秒时间戳
     */
    static uint64_t LocalTimeStampToUnixSec(const char* timestamp);
    
    /**
     * @brief 按指定格式将时间戳字符串转换为Unix秒时间戳
     * 
     * @param format 格式字符串（如"%d-%d-%d %d:%d:%d"）
     * @param timestamp 时间戳字符串
     * @return uint64_t Unix秒时间戳
     */
    static uint64_t LocalTimeStampToUnixSec(const char* format, const char* timestamp);
    
    /**
     * @brief 检查是否换天
     * 
     * 检查两个时间戳之间是否发生了换天，同时可选检查是否换周。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 上次检查的Unix秒时间戳
     * @param isWeekChange [out] 可选参数，返回是否同时换周
     * @return bool 如果换天返回true，否则返回false
     */
    static bool CheckDayChange(uint64_t curUnixSec, uint64_t lastUnixSec, bool* isWeekChange);
    
    /**
     * @brief 检查是否是同一周
     * 
     * 判断两个时间戳是否在同一周内（以星期一为一周的开始）。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 对比的Unix秒时间戳
     * @return bool 如果是同一周返回true，否则返回false
     */
    static bool CheckSameWeek(uint64_t curUnixSec, uint64_t lastUnixSec);
    
    /**
     * @brief 检查是否是同一天
     * 
     * 判断两个时间戳是否在同一天内。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 对比的Unix秒时间戳
     * @return bool 如果是同一天返回true，否则返回false
     */
    static bool CheckSameDay(uint64_t curUnixSec, uint64_t lastUnixSec);
    
    /**
     * @brief 检查是否是同一个月
     * 
     * 判断两个时间戳是否在同一个月内。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 对比的Unix秒时间戳
     * @return bool 如果是同一个月返回true，否则返回false
     */
    static bool CheckSameMonth(uint64_t curUnixSec, uint64_t lastUnixSec);
    
    /**
     * @brief 检查是否需要每日更新（0点更新）
     * 
     * 检查两个时间戳之间是否跨越了0点，用于判断是否需要执行日更新逻辑。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 上次更新的Unix秒时间戳
     * @return bool 如果需要日更新返回true，否则返回false
     */
    static bool CheckDayUpdate(uint64_t curUnixSec, uint64_t lastUnixSec);
    
    /**
     * @brief 检查是否需要每周更新（周一0点更新）
     * 
     * 检查两个时间戳之间是否跨越了周一0点，用于判断是否需要执行周更新逻辑。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 上次更新的Unix秒时间戳
     * @return bool 如果需要周更新返回true，否则返回false
     */
    static bool CheckWeekUpdate(uint64_t curUnixSec, uint64_t lastUnixSec);
    
    /**
     * @brief 获取下次日更新的时间
     * 
     * 根据当前时间计算下次日更新（次日0点）的Unix时间戳。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @return uint64_t 下次日更新的Unix时间戳
     */
    static uint64_t GetDayUpdateTime(uint64_t curUnixSec);
    
    /**
     * @brief 获取下次周更新的时间
     * 
     * 根据当前时间计算下次周更新（下周一0点）的Unix时间戳。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @return uint64_t 下次周更新的Unix时间戳
     */
    static uint64_t GetWeekUpdateTime(uint64_t curUnixSec);
    
    /**
     * @brief 检查是否需要每周更新（指定小时更新）
     * 
     * 检查两个时间戳之间是否跨越了指定小时的周更新时间。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 上次更新的Unix秒时间戳
     * @param nHour 更新的小时（0-23）
     * @return bool 如果需要周更新返回true，否则返回false
     */
    static bool CheckWeekUpdate(uint64_t curUnixSec, uint64_t lastUnixSec, int32_t nHour);
    
    /**
     * @brief 检查是否需要每日更新（指定小时更新）
     * 
     * 检查两个时间戳之间是否跨越了指定小时的日更新时间。
     * 
     * @param curUnixSec 当前Unix秒时间戳
     * @param lastUnixSec 上次更新的Unix秒时间戳
     * @param nHour 更新的小时（0-23）
     * @return bool 如果需要日更新返回true，否则返回false
     */
    static bool CheckDayUpdate(uint64_t curUnixSec, uint64_t lastUnixSec, int32_t nHour);
    
    /**
     * @brief 判断当前时间是否在指定时间段范围内
     * 
     * 判断当前时间是否落在指定的开始日期和结束日期之间。
     * 
     * @param startDate 开始日期
     * @param endData 结束日期
     * @return bool 如果在时间段内返回true，否则返回false
     */
    static bool BetweenDate(uint32_t startDate, uint32_t endData);
public:
    /**
     * @brief 获取当前日期信息
     * 
     * 根据指定的Unix时间戳解析出年、月、日信息。
     * 
     * @param iCurTime Unix时间戳
     * @param iYear [out] 年份
     * @param iMonth [out] 月份
     * @param iMonthDay [out] 月中的日期
     * @return int 解析结果，0表示成功
     */
    static int GetCurDate(unsigned int iCurTime, int &iYear, int &iMonth, int &iMonthDay);
    
    /**
     * @brief 从Unix时间戳解析出时间信息
     * 
     * 将Unix时间戳分解为星期、小时、分钟、秒等信息。
     * 
     * @param iCurTime Unix时间戳
     * @param week [out] 星期几（1-7，1表示星期一）
     * @param hour [out] 小时（0-23）
     * @param minute [out] 分钟（0-59）
     * @param second [out] 秒（0-59）
     * @return int 解析结果，0表示成功
     */
    static int GetCurTime( unsigned int iCurTime, int &week, int &hour, int &minute, int &second );
    
    /**
     * @brief 判断两个时间是否为同一天（标准零点分界）
     * 
     * 使用标准的零点（00:00:00）作为日期分界线判断两个时间是否为同一天。
     * 支持时间回退的情况。
     * 
     * @param tCur 当前时间
     * @param tBefore 对比的时间
     * @return bool 如果是同一天返回true，否则返回false
     * 
     * @note 使用标准时间，以零点为分界，支持回调
     */
    static bool IsSameDayStd(time_t tCur, time_t tBefore);
    
    /**
     * @brief 判断两个时间是否为同一周（标准零点分界）
     * 
     * 使用标准的零点（00:00:00）作为日期分界线判断两个时间是否为同一周。
     * 以星期一为一周的开始，支持时间回退的情况。
     * 
     * @param tNow 当前时间
     * @param tBeforeTime 对比的时间
     * @return bool 如果是同一周返回true，否则返回false
     * 
     * @note 使用标准时间，以零点为分界，支持回调
     */
    static bool IsSameWeekStd(time_t tNow,time_t tBeforeTime);
    
    /**
     * @brief 判断两个时间是否为同一天（游戏重置时间分界）
     * 
     * 使用游戏中定义的每日重置时间点作为日期分界线判断两个时间是否为同一天。
     * 与IsSameDayStd不同，此方法使用游戏特定的重置时间。
     * 
     * @param tCur 当前时间
     * @param tBefore 对比的时间
     * @return bool 如果是同一天返回true，否则返回false
     * 
     * @note 以每天GAME_RESET_HOUR_EVERYDAY点为准
     */
    static bool IsSameDay(time_t tCur, time_t tBefore);
    
    /**
     * @brief 判断两个时间是否为同一小时
     * 
     * 判断两个时间戳是否在同一个小时内。
     * 
     * @param tCur 当前时间
     * @param tBefore 对比的时间
     * @return bool 如果是同一小时返回true，否则返回false
     */
    static bool IsSameHour( time_t tCur, time_t tBefore );
    
    /**
     * @brief 获取今天零点的绝对时间秒值
     * 
     * 根据指定时间计算当天00:00:00的Unix时间戳。
     * 
     * @param tTimeNow 指定的时间
     * @return time_t 今天零点的Unix时间戳
     */
    static time_t GetTodayStartTime( time_t tTimeNow );

    //static time_t MakeTimeBegin(time_t tNow,char chType, TIMEDETAIL * pstConfigTime);
    //static time_t MakeTimeEnd(time_t tNow,char chType, TIMEDETAIL * pstConfigTime);

    /**
     * @brief 获取时间是星期几（1-7格式）
     * 
     * 获取指定时间是星期几，返回值范围为1-7。
     * 注意：1表示星期一，7表示星期天。
     * 
     * @param tTime 指定的时间
     * @return int 星期几（1-7，1=星期一，7=星期天）
     */
    static int GetWeekDay127(time_t tTime);
    
    /**
     * @brief 获取月中的日期
     * 
     * 获取指定时间是当月的第几天。
     * 
     * @param tTime 指定的时间
     * @return int 月中的日期（1-31）
     */
    static int GetMonthDay(time_t tTime);
    
    /**
     * @brief 获取本周开始时间（从星期一凌晨开始）
     * 
     * 计算指定时间所在星期的开始时间，以星期一凌晨为准。
     * 可以指定具体的小时作为开始时间。
     * 
     * @param tTime 指定的时间
     * @param iHour 开始的小时（默认为0，即凌晨0点）
     * @return time_t 本周开始的时间戳
     */
    static time_t GetThisWeekStartTime1(time_t tTime,int iHour = 0);


    /**
     * @brief 获取今天开始时间（带偏移）
     * 
     * 计算当天开始时间，可以指定秒数偏移量。
     * 
     * @param tTimeNow 当前时间
     * @param iOffset 偏移的秒数
     * @return time_t 今天开始时间（加上偏移后）
     */
    static time_t GetTodayStartTime2( time_t tTimeNow,int iOffset);
    
    /**
     * @brief 获取本周开始时间（带偏移）
     * 
     * 计算本周开始时间，可以指定秒数偏移量。
     * 
     * @param tTime 指定时间
     * @param iOffset 偏移的秒数
     * @return time_t 本周开始时间（加上偏移后）
     */
    static time_t GetThisWeekStartTime2(time_t tTime, int iOffset);
    
    /**
     * @brief 获取星期几（带偏移）
     * 
     * 获取指定时间是星期几，可以指定秒数偏移量。
     * 
     * @param tTime 指定时间
     * @param iOffset 偏移的秒数
     * @return time_t 考虑偏移后的星期信息
     */
    static time_t GetWeekDay2(time_t tTime, int iOffset);

    /**
     * @brief 获取两个时间之间相隔的周数
     * 
     * 计算两个时间戳之间相隔多少个完整的周。
     * 
     * @param tTime 起始时间
     * @param tNow 结束时间
     * @return int 相隔的周数
     */
    static int GetOffsetWeeks( time_t tTime, time_t tNow );

    /**
     * @brief 获取本周结束时间（到星期天指定小时结束）
     * 
     * 计算指定时间所在星期的结束时间，以星期天的指定小时为准。
     * 
     * @param tTime 指定时间
     * @param iHour 结束的小时（默认为0）
     * @return time_t 本周结束的时间戳
     */
    static time_t GetThisWeekEndTime(time_t tTime,int iHour = 0);

    /**
     * @brief 判断是否是同一周（1-7星期格式）
     * 
     * 判断两个时间是否在同一周内，以星期一凌晨开始计算。
     * 可以指定具体的开始小时。
     * 
     * @param tNow 当前时间
     * @param tBeforeTime 对比时间
     * @param iHour 一周开始的小时（默认为0）
     * @return bool 如果是同一周返回true，否则返回false
     */
    static bool IsSameWeek127(time_t tNow,time_t tBeforeTime,int iHour = 0);

    /**
     * @brief 获取周循环链索引
     * 
     * 按周循环链功能计算当前时间对应的链索引。
     * 用于实现基于周的循环功能。
     * 
     * @param tNow 当前时间
     * @param iChainLength 链的长度
     * @return int 循环链中的索引
     */
    static int GetWeekCycleChainIndex(time_t tNow,int iChainLength);

    /**
     * @brief 获取本月开始和结束时间
     * 
     * 计算指定时间所在月份的开始和结束时间。
     * 可以指定每天的起始小时。
     * 
     * @param tTime 指定时间
     * @param tBegin [out] 本月开始时间
     * @param tEnd [out] 本月结束时间
     * @param iHour 每天的起始小时（默认为0）
     */
    static void GetThisMonthStartEndTime(time_t tTime,time_t &tBegin,time_t &tEnd,int iHour = 0);

    /**
     * @brief 判断是否为同一时间段
     * 
     * 判断两个时间是否在同一时间段内，可以是同一天、同一周或同一个月。
     * 要求逻辑上tCur时候在tBefore之后，比如tCur是当前时间，tBefore是之前存的一个时间。
     * 
     * @param tCur 当前时间
     * @param tBefore 之前的时间
     * @param chType 时间段类型（'d'=天，'w'=周，'m'=月）
     * @return bool 如果是同一时间段返回true，否则返回false
     */
    static bool IsSameTimePeroid(time_t tCur, time_t tBefore,char chType);

    /**
     * @brief 判断现在是否在指定时间的后1~n天
     * 
     * 检查当前时间是否已经是指定时间的后1天及以上。
     * 天的临界点时间按指定的小时计算。
     * 
     * @param tNow 当前时间
     * @param tTime 参考时间
     * @param iHour 日期分界的小时（默认为0，即凌晨0点）
     * @return bool 如果已经过了指定天数返回true，否则返回false
     */
    static bool IsAfterDayByUTCTime(time_t tNow,time_t tTime,int iHour = 0);

    /**
     * @brief 计算过去了多少天
     * 
     * 计算从指定时间到当前时间过去了多少天。
     * 
     * @param tNow 当前时间
     * @param tTime 起始时间
     * @param iHour 日期分界的小时（默认为0）
     * @return int 过去的天数
     */
    static int GetDayElapse(time_t tNow,time_t tTime,int iHour = 0);

    /**
     * @brief 计算绝对天数
     * 
     * 以指定小时为分界计算天数。
     * 
     * @param tNow 当前时间
     * @param iHour 日期分界的小时（默认为0）
     * @return uint32_t 绝对天数
     */
    static uint32_t GetAbsDay(time_t tNow, int iHour=0);

    /**
     * @brief 获取以指定小时为基准的今天开始时间
     * 
     * 计算以指定小时为起始点的今天开始时间。
     * 与GetTodayStartTime(time_t)不同，此方法允许自定义起始小时。
     * 
     * @param tTimeNow 当前时间
     * @param iHour 起始小时（0-23）
     * @return time_t 今天在指定小时的开始时间
     */
    static time_t GetTodayStartTime( time_t tTimeNow,int iHour);
    
    /**
     * @brief 获取以指定小时和分钟为基准的今天开始时间
     * 
     * 计算以指定小时和分钟为起始点的今天开始时间。
     * 提供更精确的时间控制。
     * 
     * @param tTimeNow 当前时间
     * @param iHour 起始小时（0-23）
     * @param iMin 起始分钟（0-59）
     * @return time_t 今天在指定小时和分钟的开始时间
     */
    static time_t GetTodayStartTimeByMin( time_t tTimeNow, int iHour , int iMin);
    
    /**
     * @brief TDR时间转换成秒数
     * 
     * 将TDR（Time Data Record）格式的时间转换为秒数。
     * 计算公式：H*3600 + M*60 + S
     * 
     * @param tBreakTime TDR格式的时间值
     * @return time_t 转换后的秒数
     * 
     * @note TDR是特定的时间格式，用于某些游戏或应用中的时间表示
     */
    static time_t GetShiTuoFengYinBreakTime( time_t tBreakTime );

    /**
     * @brief 根据星期索引获取星期字符串
     * 
     * 将从0开始的星期索引转换为对应的星期字符串。
     * 
     * @param bWeekDayIndex 星期索引（0-6，0表示星期日）
     * @return const char* 星期字符串表示
     * 
     * @note 索引从0开始，0=星期日，1=星期一，依此类推
     * @note 返回的字符串可能是中文或英文，取决于具体实现
     */
    static const char *GetWeekDayStringByStartZero( uint8_t bWeekDayIndex );

    /**
     * @brief 检查指定时间间隔是否已过（秒级别）
     * 
     * 判断从上次时间到现在是否已经过去了指定的秒数。
     * 
     * @param tNow 当前时间
     * @param tLast 上次记录的时间
     * @param iGap 时间间隔（秒）
     * @return bool 如果时间间隔已过返回true，否则返回false
     * 
     * @note 这是一个内联函数，性能较高
     * @note 常用于定时器和周期性任务的时间检查
     * 
     * 使用示例：
     * @code
     * static time_t lastUpdate = 0;
     * time_t now = time(nullptr);
     * if (NFTimeUtility::IsTimePassedS(now, lastUpdate, 60)) {
     *     // 距离上次更新已经过去60秒
     *     lastUpdate = now;
     *     // 执行定时任务
     * }
     * @endcode
     */
    static bool IsTimePassedS(time_t tNow, time_t tLast, int iGap)
    {
        return (tNow - tLast) >= iGap;
    }

    /**
     * @brief 检查指定时间间隔是否已过（毫秒级别）
     * 
     * 判断从上次时间到现在是否已经过去了指定的毫秒数。
     * 使用高精度的timeval结构进行计算。
     * 
     * @param tvNow 当前时间（timeval结构）
     * @param tvLast 上次记录的时间（timeval结构）
     * @param iGapMS 时间间隔（毫秒）
     * @return bool 如果时间间隔已过返回true，否则返回false
     * 
     * @note 这是一个内联函数，提供毫秒级精度
     * @note 用于需要高精度计时的场景
     * 
     * 使用示例：
     * @code
     * static struct timeval lastFrame = {0, 0};
     * struct timeval now;
     * gettimeofday(&now, nullptr);
     * if (NFTimeUtility::IsTimePassedMS(now, lastFrame, 16)) {
     *     // 距离上次更新已经过去16毫秒（约60FPS）
     *     lastFrame = now;
     *     // 执行帧更新
     * }
     * @endcode
     */
    static bool IsTimePassedMS(struct timeval tvNow, struct timeval tvLast, int iGapMS)
    {
        int64_t i64RealGapMS = (tvNow.tv_sec - tvLast.tv_sec)*1000 + (tvNow.tv_usec - tvLast.tv_usec)/1000;
        return i64RealGapMS >= iGapMS;
    }

    /**
     * @brief 检查两个时间是否在同一个月
     * 
     * 判断两个时间戳是否属于同一个月份，使用标准的月份边界。
     * 
     * @param tCur 当前时间
     * @param tBefore 对比的时间
     * @return bool 如果在同一个月返回true，否则返回false
     */
    static bool IsSameMonth(time_t tCur, time_t tBefore);

    /**
     * @brief 根据起始小时获取星期索引
     * 
     * 根据指定的起始小时计算当前时间的星期索引。
     * 允许自定义一天的开始时间，适用于游戏中非标准日期切换的需求。
     * 
     * @param dwTimeNow 当前时间戳
     * @param bStartHour 一天的起始小时（0-23）
     * @return int 星期索引（0-6，0表示星期日）
     */
    static int GetWeekDayIndexWithStartHour( uint32_t dwTimeNow, uint8_t bStartHour );

    /**
     * @brief 检查星期控制标志是否匹配当前时间
     * 
     * 根据星期控制标志检查当前时间是否在允许的星期范围内。
     * 用于实现基于星期的功能开关或活动控制。
     * 
     * @param bWeekDayCtrlFlag 星期控制标志（位标志，bit0=星期日，bit1=星期一...bit6=星期六）
     * @param dwTimeNow 当前时间戳
     * @return bool 如果当前星期匹配控制标志返回true，否则返回false
     * 
     * @note bWeekDayCtrlFlag是位标志，每一位代表一个星期
     * @note 例如：0x7F表示所有星期都开放，0x3E表示周一到周五开放
     */
    static bool IsOKWithWeekDayCtrl( uint8_t bWeekDayCtrlFlag, uint32_t dwTimeNow );

    /**
     * @brief 将日期时间组件转换为时间戳
     * 
     * 将年、月、日、时、分、秒各个组件转换为Unix时间戳。
     * 
     * @param iYear 年份（如2023）
     * @param iMon 月份（1-12）
     * @param iMDay 月中的日期（1-31）
     * @param iHour 小时（0-23）
     * @param iMin 分钟（0-59）
     * @param iSec 秒（0-59）
     * @return time_t Unix时间戳
     * 
     * @note 输入的日期时间组件必须有效，否则结果未定义
     */
    static time_t ToTimestamp(int iYear, int iMon, int iMDay/*day of the month */, int iHour, int iMin, int iSec );

    /**
     * @brief 获取指定年月的天数
     * 
     * 计算指定年份和月份有多少天，自动考虑闰年情况。
     * 
     * @param year 年份
     * @param month 月份（1-12）
     * @return short 该月的天数（28-31）
     * 
     * @note 会正确处理闰年的2月份（29天 vs 28天）
     */
    static short GetCurMonthDay(int year, int month);
    
    /**
     * @brief 获取下个月的指定日期
     * 
     * 计算相对于当前年月日的下个月对应日期。
     * 如果下个月没有对应的日期（如1月31日的下个月），会进行适当调整。
     * 
     * @param year 当前年份
     * @param month 当前月份（1-12）
     * @param day 当前日期（1-31）
     * @param nMonth [out] 输出的目标月份
     * @return int64_t 下个月对应日期的时间戳表示
     * 
     * @note 当前月份的日期在下个月不存在时（如1月31日），会调整到下个月的最后一天
     */
    static int64_t GetNextMonthDay(int year, int month, int day, int &nMonth);

    /**
     * @brief 获取距离下一个整点小时的秒数
     * 
     * 计算当前时间距离下一个整点小时还有多少秒。
     * 
     * @param tCur 当前时间
     * @return int 距离下一个整点的秒数（0-3599）
     * 
     * 使用示例：
     * @code
     * time_t now = time(nullptr);
     * int seconds = NFTimeUtility::GetOffsetToNextHour(now);
     * // 如果现在是14:30:45，则返回1755秒（到15:00:00的秒数）
     * @endcode
     */
    static int GetOffsetToNextHour(time_t tCur);
    
    /**
     * @brief 获取距离下一个半小时整点的秒数
     * 
     * 计算当前时间距离下一个半小时整点还有多少秒。
     * 半小时整点指xx:00:00和xx:30:00。
     * 
     * @param tCur 当前时间
     * @return int 距离下一个半小时整点的秒数（0-1799）
     */
    static int GetOffsetToNextHalfHour(time_t tCur);
    
    /**
     * @brief 获取距离下一天零点的秒数
     * 
     * 计算当前时间距离次日00:00:00还有多少秒。
     * 
     * @param tCur 当前时间
     * @return int 距离下一天零点的秒数（0-86399）
     */
    static int GetOffsetToNextDay(time_t tCur);

    /**
     * @brief 获取时间戳对应的年份
     * 
     * 从Unix时间戳中提取实际的年份值（公历年份）。
     * 
     * @param tCur 时间戳
     * @return int 年份值（如2023）
     * 
     * @note 返回的是实际的年份数字，不是相对于1900的偏移
     */
    static int GetYearByTimestamp(time_t tCur);
    
    /**
     * @brief 计算两个时间相隔的天数
     * 
     * 按照零点分界计算两个时间戳相隔的完整天数。
     * 结果可以为负数，表示时间顺序。
     * 
     * @param tOne 第一个时间戳
     * @param tTwo 第二个时间戳
     * @return int 相隔的天数，tTwo比tOne晚则为正数，否则为负数
     * 
     * @note 计算基于自然日，以午夜00:00:00为分界
     */
    static int GetDaysDelta(time_t tOne, time_t tTwo);
    
    /**
     * @brief 获取本周指定星期的开始时间
     * 
     * 计算本周内指定星期几的00:00:00时间戳。
     * 如果指定的星期几就是今天或者还未到来，返回对应日期的开始时间。
     * 
     * @param tNow 当前时间
     * @param iWeekDay 星期几（1-7，1表示星期一，7表示星期天）
     * @return time_t 指定星期的开始时间戳（该天的00:00:00）
     * 
     * @note 如果指定的星期几已经过去，返回的是本周对应星期的时间
     */
    static time_t GetThisWeekDayTime(time_t tNow, int iWeekDay);

    /**
     * @brief 根据周数和星期几计算时间戳
     * 
     * 根据指定的周数和星期几计算对应的时间戳。
     * 周数从某个基准点开始计算（通常是1970年1月1日所在的周）。
     * 
     * @param ullWeek 周数（从基准点开始的周数）
     * @param iDay 星期几（1-7，1表示星期一）
     * @return uint64_t 计算得到的时间戳
     * 
     * @note 基准点通常是Unix纪元时间（1970年1月1日）所在的周
     * @note 返回的是指定星期几的00:00:00时间戳
     */
    static uint64_t GetTimeByWeekDay(uint64_t ullWeek, int iDay);
};

