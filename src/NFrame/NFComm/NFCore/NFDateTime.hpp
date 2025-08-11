// -------------------------------------------------------------------------
//    @FileName         :    NFDateTime.hpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFDateTime
//
// -------------------------------------------------------------------------

/**
 * @file NFDateTime.hpp
 * @brief 日期时间工具类集合
 * 
 * 此文件提供了完整的日期时间处理功能，包括时间跨度计算、日期时间操作、
 * 时区转换等。主要包含NFTimeSpan和NFDateTime两个核心类。
 */

#ifndef NF_DATETIME_HPP
#define NF_DATETIME_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <time.h>
#include <string>
#include <map>
#include <vector>
#include "NFException.hpp"
#include "NFPlatform.h"

//for timespan
/** @brief 秒到毫秒的转换因子 */
#define FACTOR_SEC_TO_MILLI                     (1000)
/** @brief 分钟到毫秒的转换因子 */
#define FACTOR_MIN_TO_MILLI                (60 * 1000)
/** @brief 分钟到秒的转换因子 */
#define FACTOR_MIN_TO_SEC                         (60)
/** @brief 小时到毫秒的转换因子 */
#define FACTOR_HOUR_TO_MILLI          (60 * 60 * 1000)
/** @brief 小时到分钟的转换因子 */
#define FACTOR_HOUR_TO_MIN                        (60)
/** @brief 天到毫秒的转换因子 */
#define FACTOR_DAY_TO_MILLI      (24 * 60 * 60 * 1000)
/** @brief 天到小时的转换因子 */
#define FACTOR_DAY_TO_HOUR                        (24)

///////////////////////////////
//for datetime
/** @brief 时间基准年份（1900年） */
#define SINCE_YEAR               1900

// Summertime
/** @brief 夏令时开始月份 */
#define SUMMERTIME_BEGIN_MONTH      3
/** @brief 夏令时结束月份 */
#define SUMMERTIME_END_MONTH       10

// Min and Max values
/** @brief 最小月份值 */
#define MIN_MONTH                   1
/** @brief 最大月份值 */
#define MAX_MONTH                  12

/** @brief 最小日期值 */
#define MIN_DAY                     1
/** @brief 最大日期值 */
#define MAX_DAY                    30

/** @brief 最小星期值 */
#define MIN_WEEKDAY                 0
/** @brief 最大星期值 */
#define MAX_WEEKDAY                 7

/** @brief 最小小时值 */
#define MIN_HOUR                    0
/** @brief 最大小时值 */
#define MAX_HOUR                   24

/** @brief 最小分钟值 */
#define MIN_MINUTE                  0
/** @brief 最大分钟值 */
#define MAX_MINUTE                 60

/** @brief 最小秒数值 */
#define MIN_SECOND                  0
/** @brief 最大秒数值 */
#define MAX_SECOND                 60

/** @brief 最小毫秒值 */
#define MIN_MILLISECOND             0
/** @brief 最大毫秒值 */
#define MAX_MILLISECOND          1000

/**
 * @brief 时间跨度类
 * 
 * NFTimeSpan类用于表示一个时间间隔，可以精确到毫秒级别。
 * 支持天、小时、分钟、秒、毫秒的组合表示，提供各种时间单位之间的转换。
 * 
 * 主要功能：
 * - 支持多种时间单位的构造
 * - 提供静态工厂方法创建时间跨度
 * - 支持获取各个时间分量
 * - 提供总时间的毫秒、秒、分钟表示
 * 
 * 使用方法：
 * @code
 * NFTimeSpan span1(1, 2, 30, 45, 500); // 1天2小时30分45秒500毫秒
 * NFTimeSpan span2 = NFTimeSpan::FromHours(3); // 3小时
 * long long totalMs = span1.GetTotalMilliseconds();
 * @endcode
 */
class NFTimeSpan
{
public:
	/**
	 * @brief 构造函数（仅秒）
	 * @param seconds 秒数
	 */
	NFTimeSpan(int seconds)
	{
		Init(0, 0, 0, seconds, 0);
	}

	/**
	 * @brief 构造函数（小时、分钟、秒）
	 * @param hours 小时数
	 * @param minutes 分钟数
	 * @param seconds 秒数
	 */
	NFTimeSpan(int hours, int minutes, int seconds)
	{
		Init(0, hours, minutes, seconds, 0);
	}

	/**
	 * @brief 构造函数（天、小时、分钟、秒）
	 * @param days 天数
	 * @param hours 小时数
	 * @param minutes 分钟数
	 * @param seconds 秒数
	 */
	NFTimeSpan(int days, int hours, int minutes, int seconds)
	{
		Init(days, hours, minutes, seconds, 0);
	}

	/**
	 * @brief 构造函数（完整参数）
	 * @param days 天数
	 * @param hours 小时数
	 * @param minutes 分钟数
	 * @param seconds 秒数
	 * @param milliseconds 毫秒数
	 */
	NFTimeSpan(int days, int hours, int minutes, int seconds, int milliseconds)
	{
		Init(days, hours, minutes, seconds, milliseconds);
	}

	/**
	 * @brief 虚析构函数
	 */
	virtual ~NFTimeSpan()
	{
	}

	/**
	 * @brief 从毫秒创建时间跨度
	 * @param milliseconds 毫秒数
	 * @return NFTimeSpan 时间跨度对象
	 */
	static NFTimeSpan FromMilliseconds(int milliseconds)
	{
		return NFTimeSpan(0, 0, 0, 0, milliseconds);
	}

	/**
	 * @brief 从秒创建时间跨度
	 * @param seconds 秒数
	 * @return NFTimeSpan 时间跨度对象
	 */
	static NFTimeSpan FromSeconds(int seconds)
	{
		return NFTimeSpan(0, 0, 0, seconds, 0);
	}

	/**
	 * @brief 从分钟创建时间跨度
	 * @param minutes 分钟数
	 * @return NFTimeSpan 时间跨度对象
	 */
	static NFTimeSpan FromMinutes(int minutes)
	{
		return NFTimeSpan(0, 0, minutes, 0, 0);
	}

	/**
	 * @brief 从小时创建时间跨度
	 * @param hours 小时数
	 * @return NFTimeSpan 时间跨度对象
	 */
	static NFTimeSpan FromHours(int hours)
	{
		return NFTimeSpan(0, hours, 0, 0, 0);
	}

	/**
	 * @brief 从天数创建时间跨度
	 * @param days 天数
	 * @return NFTimeSpan 时间跨度对象
	 */
	static NFTimeSpan FromDays(int days)
	{
		return NFTimeSpan(days, 0, 0, 0, 0);
	}

	/**
	 * @brief 获取毫秒分量
	 * @return int 毫秒数（0-999）
	 */
	int GetMilliseconds() const
	{
		return mMilliseconds;
	}

	/**
	 * @brief 获取秒分量
	 * @return int 秒数（0-59）
	 */
	int GetSeconds() const
	{
		return mSeconds;
	}

	/**
	 * @brief 获取分钟分量
	 * @return int 分钟数（0-59）
	 */
	int GetMinutes() const
	{
		return mMinutes;
	}

	/**
	 * @brief 获取小时分量
	 * @return int 小时数（0-23）
	 */
	int GetHours() const
	{
		return mHours;
	}

	/**
	 * @brief 获取天数分量
	 * @return int 天数
	 */
	int GetDays() const
	{
		return mDays;
	}

	/**
	 * @brief 获取总毫秒数
	 * @return long long 时间跨度的总毫秒数
	 */
	long long GetTotalMilliseconds() const
	{
		return mMilliseconds + static_cast<long long>(mSeconds) * FACTOR_SEC_TO_MILLI + static_cast<long long>(mMinutes) * FACTOR_MIN_TO_MILLI + static_cast<long long>(mHours) * FACTOR_HOUR_TO_MILLI + (long long)mDays * FACTOR_DAY_TO_MILLI;
	}

	/**
	 * @brief 获取总秒数
	 * @return long long 时间跨度的总秒数
	 * @note 更新：GetTotalXXXXs系列函数不再返回double类型
	 */
	long long GetTotalSeconds() const
	{
		return GetTotalMilliseconds() / FACTOR_SEC_TO_MILLI;
	}

	/**
	 * @brief 获取总分钟数
	 * @return long long 时间跨度的总分钟数
	 */
	long long GetTotalMinutes() const
	{
		return GetTotalSeconds() / FACTOR_MIN_TO_SEC;
	}

	/**
	 * @brief 获取总小时数
	 * @return long long 时间跨度的总小时数
	 */
	long long GetTotalHours() const
	{
		return GetTotalMinutes() / FACTOR_HOUR_TO_MIN;
	}

	/**
	 * @brief 获取总天数
	 * @return long long 时间跨度的总天数
	 */
	long long GetTotalDays() const
	{
		return GetTotalHours() / FACTOR_DAY_TO_HOUR;
	}

	bool operator<(const NFTimeSpan& ts) const
	{
		return GetTotalMilliseconds() < ts.GetTotalMilliseconds();
	}

	bool operator>(const NFTimeSpan& ts) const
	{
		return GetTotalMilliseconds() > ts.GetTotalMilliseconds();
	}

	bool operator<=(const NFTimeSpan& ts) const
	{
		return GetTotalMilliseconds() <= ts.GetTotalMilliseconds();
	}

	bool operator>=(const NFTimeSpan& ts) const
	{
		return GetTotalMilliseconds() >= ts.GetTotalMilliseconds();
	}

	bool operator==(const NFTimeSpan& ts) const
	{
		return GetTotalMilliseconds() == ts.GetTotalMilliseconds();
	}

	void Init(int days, int hours, int minutes, int seconds, int milliseconds)
	{
		this->mDays = days;
		this->mHours = hours;
		this->mMinutes = minutes;
		this->mSeconds = seconds;
		this->mMilliseconds = milliseconds;
	}

protected:

	int mMilliseconds;
	int mSeconds;
	int mMinutes;
	int mHours;
	int mDays;
};

//////////////////////////////////////////////

class NFDateTime
{
public:

	enum Day
	{
		Monday,
		Thuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday,
		Sunday
	};

	enum Month
	{
		January,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};

	NFDateTime(int day, int month, int year) : mSummertime(false), mLeapyear(false)
	{
		Init(day, month, year, 0, 0, 0, 0);
	}

	NFDateTime(int day, int month, int year, int hour, int minute, int second) : mSummertime(false), mLeapyear(false)
	{
		Init(day, month, year, hour, minute, second, 0);
	}

	NFDateTime(int day, int month, int year, int hour, int minute, int second, int millisecond) : mSummertime(false), mLeapyear(false)
	{
		Init(day, month, year, hour, minute, second, millisecond);
	}

	NFDateTime(time_t timestamp, int millisecond = 0) : mSummertime(false), mLeapyear(false)
	{
		SetWithTimestamp(timestamp);
		AddMilliseconds(millisecond);
	}

	~NFDateTime()
	{
	}

	static NFDateTime Now()
	{
		NFDateTime dt;
		dt.SetNow();
		return dt;
	}

	void SetNow()
	{
		SetWithTimestamp(time(nullptr));
	}

	int GetYear() const
	{
		return mYear;
	}

	int GetMonth() const
	{
		return mMonth;
	}

	int GetDay() const
	{
		return mDay;
	}

	int GetHour() const
	{
		return mHour;
	}

	int GetMinute() const
	{
		return mMinute;
	}

	int GetSecond() const
	{
		return mSecond;
	}

	int GetMillisecond() const
	{
		return mMillisecond;
	}

	void SetWithTimestamp(time_t timestamp)
	{
		struct tm time = *localtime(&timestamp);
		Init(time.tm_mday, time.tm_mon + 1, time.tm_year + SINCE_YEAR, time.tm_hour, time.tm_min, time.tm_sec, 0);
	}

	time_t GetTimestamp() const
	{
		time_t rawtime(0);
		struct tm* time = localtime(&rawtime);
		time->tm_year = mYear - SINCE_YEAR;
		time->tm_mon = mMonth - 1;
		time->tm_mday = mDay;
		time->tm_hour = mHour;
		time->tm_min = mMinute;
		time->tm_sec = mSecond;
		return mktime(time);
	}

	struct tm GetTMStruct() const
	{
		time_t rawtime(0);
		struct tm* time = localtime(&rawtime);
		time->tm_year = mYear - SINCE_YEAR;
		time->tm_mon = mMonth - 1;
		time->tm_mday = mDay;
		time->tm_hour = mHour;
		time->tm_min = mMinute;
		time->tm_sec = mSecond;
		mktime(time);
		return *time;
	}

	void Add(const NFTimeSpan* ts)
	{
		AddMilliseconds(ts->GetTotalMilliseconds());
	}

	void AddYears(int years)
	{
		mYear += years;
	}

	void AddMonths(int months)
	{
		AddYears(months / MAX_MONTH);
		mMonth += months % MAX_MONTH;
	}

	void AddDays(int days)
	{
		AddMonths(days / MAX_DAY);
		mDay += days % MAX_DAY;
	}

	void AddHours(int hours)
	{
		AddDays(hours / MAX_HOUR);
		mHour += hours % MAX_HOUR;
	}

	void AddMinutes(int minutes)
	{
		AddHours(int(minutes / MAX_MINUTE));
		mMinute += minutes % MAX_MINUTE;
	}

	void AddSeconds(long long seconds)
	{
		AddMinutes(int(seconds / MAX_SECOND));
		mSecond += int(seconds % MAX_SECOND);
	}

	void AddMilliseconds(long long milliseconds)
	{
		AddSeconds(milliseconds / MAX_MILLISECOND);
		mMillisecond += milliseconds % MAX_MILLISECOND;
	}

	static bool IsYearLeapYear(int year)
	{
		return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
	}

	bool IsSummertime() const
	{
		return IsYearLeapYear(mYear);
	}

	bool IsLeapYear() const
	{
		return IsDateSummertime(mDay, mMonth);
	}

	static bool IsDateSummertime(int day, int month)
	{
		// FIXME: include day in calculation
		if (month >= SUMMERTIME_BEGIN_MONTH && month <= SUMMERTIME_END_MONTH)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	std::string GetNameOfDay(int day)
	{
		if (IsValidWeekday(day))
		{
			return mDayNames[day - 1];
		}
		else
		{
			std::string err = NF_FORMAT("Day %d is not in valid weekday range ( %d - %d )", day, MIN_WEEKDAY, MAX_WEEKDAY);
			throw NFException(err);
		}
	}

	std::string GetNameOfMonth(int month)
	{
		if (IsValidMonth(month))
		{
			return mMonthNames[month - 1];
		}
		else
		{
			std::string err = NF_FORMAT("Month %d is not in valid range ( %d - %d )", month, MIN_MONTH, MAX_MONTH);
			throw NFException(err);
		}
	}

	Day GetDayOfWeek() const
	{
		return static_cast<NFDateTime::Day>(GetTMStruct().tm_wday);
	}

	int GetDayOfYear() const
	{
		return GetTMStruct().tm_yday;
	}

	int GetDaysOfMonth(int month)
	{
		return IsValidMonth(month) ? mDaysOfMonth[month-1] : -1;
	}

	bool operator<(const NFDateTime& dt) const
	{
		return GetTimestamp() < dt.GetTimestamp();
	}

	bool operator>(const NFDateTime& dt) const
	{
		return GetTimestamp() > dt.GetTimestamp();
	}

	bool operator<=(const NFDateTime& dt) const
	{
		return GetTimestamp() <= dt.GetTimestamp();
	}

	bool operator>=(const NFDateTime& dt) const
	{
		return GetTimestamp() >= dt.GetTimestamp();
	}

	bool operator==(const NFDateTime& dt) const
	{
		return GetTimestamp() == dt.GetTimestamp();
	}

	NFDateTime operator+(const NFTimeSpan& ts)
	{
		NFDateTime* tmp = this;
		tmp->Add(&ts);
		return *tmp;
	}

	void operator+=(const NFTimeSpan& ts)
	{
		Add(&ts);
	}

	std::string GetAsString() const
	{
		return GetShortDateString() + std::string(" - ") + GetShortTimeString();
	}

	/**
	 * @brief 凭借时间生成一个字符串，精确到分钟
	 * 比如GetTimeStringToMinute("_"), 生成:2018_5_17_18_58
	 *
	 * @param str  用来分割的字符串
	 * @return     返回拼接的时间字符串
	 */
	std::string GetTimeStringToMinute(const std::string& str) const
	{
		std::stringstream stream;
		stream << GetYear() << str << GetMonth() << str << GetDay();
		stream << str << GetHour() << str << GetMinute();
		return stream.str();
	}

	std::string GetDbTimeString() const
	{
		std::stringstream stream;
		stream << GetYear() << "-" << GetMonth() << "-" << GetDay() << " ";
		stream << GetHour() << ":" << GetMinute() << ":" << GetSecond();
		return stream.str();
	}

	std::string GetShortTimeString() const
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss << std::setfill('0') << std::setw(2) << mHour << ":" << std::setw(2) << mMinute << ":" << std::setw(2) << mSecond;
		return ss.str();
	}

	std::string GetLongTimeString() const
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss << std::setfill('0') << std::setw(2) << mHour << ":" << std::setw(2) << mMinute << ":" << std::setw(2) << mSecond << ":" << std::setw(2) << mMillisecond;
		return ss.str();
	}

	std::string GetShortDateString() const
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss << std::setfill('0') << std::setw(2) << mDay << "." << std::setw(2) << mMonth << "." << mYear;
		return ss.str();
	}

	std::string GetLongDateString()
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss << GetNameOfDay(GetDayOfWeek()) << ", " << GetNameOfMonth(mMonth) << " " << mDay << ", " << mYear;
		return ss.str();
	}

protected:
	NFDateTime() : mSummertime(false), mLeapyear(false)
	{
		this->mDay = 0;
		this->mMonth = 0;
		this->mYear = 0;

		this->mHour = 0;
		this->mMinute = 0;
		this->mSecond = 0;
		this->mMillisecond = 0;

		InitMonths();
		InitMonthNames();
		InitDayNames();
	}

	void Init(int day, int month, int year, int hour, int minute, int second, int millisecond)
	{
		this->mYear = year;

		InitMonths();
		InitMonthNames();
		InitDayNames();

		if (!IsValidMonth(month))
		{
			throw NFException("Month %d is not in range", month);
		}

		if (!IsValidDayOfMonth(day, month))
		{
			std::string err = NF_FORMAT("Day %d is not in month %d's range", day, month);
			throw NFException(err);
		}

		if (!IsValidHour(hour))
		{
			std::string err = NF_FORMAT("Hour %d is not in valid range ( %d - %d )", hour, MIN_HOUR, MAX_HOUR);
			throw NFException(err);
		}

		if (!IsValidMinute(minute))
		{
			std::string err = NF_FORMAT("Minute %d is not in valid range ( %d - %d )", minute, MIN_MINUTE, MAX_MINUTE);
			throw NFException(err);
		}

		if (!IsValidSecond(second))
		{
			std::string err = NF_FORMAT("Second %d is not in valid range ( %d - %d )", second, MIN_SECOND, MAX_SECOND);
			throw NFException(err);
		}

		if (!IsValidMillisecond(millisecond))
		{
			std::string err = NF_FORMAT("Millisecond %d is not in valid range ( %d - %d )", millisecond, MIN_MILLISECOND, MAX_MILLISECOND);
			throw NFException(err);
		}

		this->mDay = day;
		this->mMonth = month;

		this->mHour = hour;
		this->mMinute = minute;
		this->mSecond = second;
		this->mMillisecond = millisecond;
	}

	void InitMonths()
	{
		// Perhaps an algorithm would be more efficient
		mDaysOfMonth[Month::January] = 31;
		mDaysOfMonth[Month::February] = IsLeapYear() ? 29 : 28; // In a leapyear 29 else 28
		mDaysOfMonth[Month::March] = 31;
		mDaysOfMonth[Month::April] = 30;
		mDaysOfMonth[Month::May] = 31;
		mDaysOfMonth[Month::June] = 30;
		mDaysOfMonth[Month::July] = 31;
		mDaysOfMonth[Month::August] = 31;
		mDaysOfMonth[Month::September] = 30;
		mDaysOfMonth[Month::October] = 31;
		mDaysOfMonth[Month::November] = 30;
		mDaysOfMonth[Month::December] = 31;
	}

	void InitMonthNames()
	{
		mMonthNames[Month::January] = "January";
		mMonthNames[Month::February] = "February";
		mMonthNames[Month::March] = "March";
		mMonthNames[Month::April] = "April";
		mMonthNames[Month::May] = "May";
		mMonthNames[Month::June] = "June";
		mMonthNames[Month::July] = "July";
		mMonthNames[Month::August] = "August";
		mMonthNames[Month::September] = "September";
		mMonthNames[Month::October] = "October";
		mMonthNames[Month::November] = "November";
		mMonthNames[Month::December] = "December";
	}

	void InitDayNames()
	{
		mDayNames[Day::Monday] = "Monday";
		mDayNames[Day::Thuesday] = "Thuesday";
		mDayNames[Day::Wednesday] = "Wednesday";
		mDayNames[Day::Thursday] = "Thursday";
		mDayNames[Day::Friday] = "Friday";
		mDayNames[Day::Saturday] = "Saturday";
		mDayNames[Day::Sunday] = "Sunday";
	}

	static bool IsValidWeekday(int day)
	{
		return day >= MIN_WEEKDAY && day <= MAX_WEEKDAY;
	}

	bool IsValidDayOfMonth(int day, int month)
	{
		if (IsValidMonth(month))
		{
			return day >= 1 && day <= GetDaysOfMonth(month);
		}
		else
		{
			return false;
		}
	}

	static bool IsValidMonth(int month)
	{
		return month >= MIN_MONTH && month <= MAX_MONTH;
	}

	static bool IsValidYear(int year)
	{
		return year >= 0;
	}

	static bool IsValidHour(int hour)
	{
		return hour >= MIN_HOUR && hour <= MAX_HOUR;
	}

	static bool IsValidMinute(int minute)
	{
		return minute >= MIN_MINUTE && minute <= MAX_MINUTE;
	}

	static bool IsValidSecond(int second)
	{
		return second >= MIN_SECOND && second <= MAX_SECOND;
	}

	static bool IsValidMillisecond(int millisecond)
	{
		return millisecond >= MIN_MILLISECOND && millisecond <= MAX_MILLISECOND;
	}

protected:
	int mDaysOfMonth[MAX_MONTH];
	std::string mMonthNames[MAX_MONTH];
	std::string mDayNames[MAX_DAY];

	int mYear;
	int mMonth;
	int mDay;
	int mHour;
	int mMinute;
	int mSecond;
	int mMillisecond;

	bool mSummertime;
	bool mLeapyear;
};

#endif

