// -------------------------------------------------------------------------
//    @FileName         :    NFMagicTimeUtil.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMagicTimeUtil.cpp
//
// -------------------------------------------------------------------------

#include "NFTimeUtility.h"
#include "NFTime.h"
// xxxx 从km抄过来的
// 判断是否为闰年
#define IS_LEAP_YEAR(year) \
    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
//  每个月最后一天距离1月1日的天数
static const int YearDayOffset[2][13] =
        {
                {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
                {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
        };
// 本算法适用于 2013-01-01 00:00:00 ~ 2038年1月19日03:14:07 之间的time_t的转化
// 起始时间2013-01-01 00:00:00的time_t
#define START_SEC   1356969600
#define DAY_SECONDS 86400
#define NONLEAP_YEAR_DAYS  365

/**
* @Function            :  计算iCurTime为哪一年的哪一月的哪一天
* @param [input]     :  iCurTime:time_t时间
* @param [output]     :  iYear:
* @return             : 0 sucess , -1 failure
*/
int NFTimeUtility::GetCurDate(unsigned int iCurTime,
                              int &iYear, int &iMonth, int &iMonthDay) {
    //
    int iTime = iCurTime - START_SEC;
    int iDay = iTime / DAY_SECONDS;
    // 从2013年开始计算，为了减少while的循环数量
    int iStartYear = 2013;
    int iStartDay = 0;

    do {
        iStartDay += NONLEAP_YEAR_DAYS;

        if (IS_LEAP_YEAR(iStartYear)) {
            ++iStartDay;
        }

        iStartYear++;
    } while (iStartDay <= iDay);

    --iStartYear;
    iStartDay -= NONLEAP_YEAR_DAYS;
    int iIsLeapYear = IS_LEAP_YEAR(iStartYear);

    if (iIsLeapYear) {
        --iStartDay;
    }

    int iYearDay = iDay - iStartDay;
    int iMon = 0;

    while (iMon < (int) sizeof(YearDayOffset[0])
           && YearDayOffset[iIsLeapYear][iMon] <= iYearDay) { //-V557
        ++iMon;
    }

    iYear = iStartYear;
    iMonth = iMon;

    if (iMon <= 0) {
        return -1;
    }

    iMonthDay = iYearDay - YearDayOffset[iIsLeapYear][iMon - 1] + 1;
    return 0;
}

int NFTimeUtility::GetCurTime(unsigned int iCurTime, int &week, int &hour, int &minute, int &second) {
    week = GetWeekDay127(iCurTime);
    iCurTime -= GetTodayStartTime(iCurTime);

    hour = iCurTime / 3600;
    iCurTime -= 3600 * hour;
    minute = iCurTime / 60;
    iCurTime -= 60 * minute;
    second = iCurTime;

    return 0;
}

bool NFTimeUtility::IsSameHour(time_t tA, time_t tB) {
    if (tA == tB) {
        return true;
    }

    time_t tAHours = tA / SECONDS_AHOUR;
    time_t tBHours = tB / SECONDS_AHOUR;

    if (tAHours == tBHours) {
        return true;
    }

    return false;
}

bool NFTimeUtility::IsSameDayStd(time_t tCur, time_t tBefore) {
    time_t tNowLocal = tCur + LOCAL_TIME_CORRECTION;
    time_t tTimeLocal = tBefore + LOCAL_TIME_CORRECTION;
    time_t tNowDays = tNowLocal / SECONDS_ADAY;
    time_t tTimeDays = tTimeLocal / SECONDS_ADAY;
    return tNowDays == tTimeDays;
}

bool NFTimeUtility::IsSameWeekStd(time_t tNow, time_t tBeforeTime) {
    return GetThisWeekStartTime1(tNow, 0) == GetThisWeekStartTime1(tBeforeTime, 0);
}


bool NFTimeUtility::IsSameDay(time_t tCur, time_t tBefore) {
    return GetDayElapse(tCur, tBefore, GAME_RESET_HOUR_EVERYDAY) == 0;
    /*
        tm* ptmCur = localtime(&tCur);
        time_t tMin = 0;
        ptmCur->tm_sec = GAME_RESET_SEC_EVERYDAY;
        ptmCur->tm_min = GAME_RESET_MIN_EVERYDAY;
        if(ptmCur->tm_hour >= GAME_RESET_HOUR_EVERYDAY)
        {
            ptmCur->tm_hour = GAME_RESET_HOUR_EVERYDAY;
            tMin = mktime(ptmCur);
        }
        else
        {
            ptmCur->tm_hour = GAME_RESET_HOUR_EVERYDAY;
            tMin = mktime(ptmCur) - SECONDS_ADAY;
        }
        return tBefore >= tMin && tBefore < tMin + SECONDS_ADAY;
    */
}

time_t NFTimeUtility::GetTodayStartTime(time_t tTimeNow) // 获取今天零点的绝对时间秒值;
{
    return GetTodayStartTime(tTimeNow, 0);
}

time_t NFTimeUtility::GetTodayStartTimeByMin(time_t tTimeNow, int iHour, int iMin) {
    int iMinSecond = iHour * 3600 + 60 * iMin;
    time_t tNowLocal = tTimeNow + LOCAL_TIME_CORRECTION - iMinSecond;
    time_t tNowDays = tNowLocal / SECONDS_ADAY;
    time_t tNowDayStart = tNowDays * SECONDS_ADAY;
    tNowDayStart = tNowDayStart - LOCAL_TIME_CORRECTION + iMinSecond;
    return tNowDayStart;
}

bool NFTimeUtility::IsSameMonth(time_t tCur, time_t tBefore) {
    if (tCur <= tBefore) {
        return true;
    }
    int iYear1, iMonth1, iMonthDay1;
    int iYear2, iMonth2, iMonthDay2;
    int iRet = GetCurDate(tCur, iYear1, iMonth1, iMonthDay1);

    if (0 == iRet) {
        iRet = GetCurDate(tBefore, iYear2, iMonth2, iMonthDay2);
    }

    if (0 == iRet) {
        return (iYear1 == iYear2 && iMonth1 == iMonth2);
    }

    return false;
}

int NFTimeUtility::GetWeekDay127(time_t tTime) {
    time_t iLocalTime = tTime + LOCAL_TIME_CORRECTION;
    time_t iWeekDay = ((iLocalTime / SECONDS_ADAY + 3) % 7) + 1;
    return iWeekDay;
}

int NFTimeUtility::GetMonthDay(time_t tTime)
{
    int iYear = 0;
    int iMonth = 0;
    int iMonthDay = 1;
    NFTimeUtility::GetCurDate(tTime, iYear, iMonth, iMonthDay);
    return iMonthDay;
}

int NFTimeUtility::GetOffsetWeeks(time_t tTime, time_t tNow) {
    if (tTime >= tNow) {
        return 0;
    }

    time_t tTimeWeekStart = GetThisWeekStartTime1(tTime);
    time_t tNowWeekStart = GetThisWeekStartTime1(tNow);

    if (tTimeWeekStart >= tNowWeekStart) {
        return 0;
    }

    return (tNowWeekStart - tTimeWeekStart) / SECONDS_AWEEK;
}

time_t NFTimeUtility::GetThisWeekStartTime1(time_t tTime, int iHour) {
    int iHourSecond = iHour * 3600;
    time_t iLocalTime = tTime + LOCAL_TIME_CORRECTION - iHourSecond;
    time_t iLocalDayStartTime = (iLocalTime / SECONDS_ADAY) * SECONDS_ADAY;
    int iWeekDay = GetWeekDay127(tTime - iHourSecond);
    time_t iLocalThisWeekStartTime1 = iLocalDayStartTime - (iWeekDay - 1) * SECONDS_ADAY;
    return iLocalThisWeekStartTime1 - LOCAL_TIME_CORRECTION + iHourSecond;//to GMT
}

time_t NFTimeUtility::GetThisWeekEndTime(time_t tTime, int iHour) {
    int iHourSecond = iHour * 3600;
    time_t iLocalTime = tTime + LOCAL_TIME_CORRECTION - iHourSecond;
    time_t iLocalDayStartTime = (iLocalTime / SECONDS_ADAY) * SECONDS_ADAY;
    int iWeekDay = GetWeekDay127(tTime - iHourSecond);
    time_t iLocalThisWeekEndTime = iLocalDayStartTime + (7 - (iWeekDay - 1)) * SECONDS_ADAY;
    return iLocalThisWeekEndTime - LOCAL_TIME_CORRECTION + iHourSecond;
}

bool NFTimeUtility::IsSameWeek127(time_t tNow, time_t tBeforeTime, int iHour) {
    //时间往回调整的情况
    if (tNow <= tBeforeTime) {
        return true;
    }

    return GetThisWeekStartTime1(tNow, iHour) == GetThisWeekStartTime1(tBeforeTime, iHour);
}

int NFTimeUtility::GetWeekCycleChainIndex(time_t tNow, int iChainLength) {
    time_t tWeekStartTime = GetThisWeekStartTime1(tNow, GAME_RESET_HOUR_EVERYDAY);
    //逻辑上只要保证算法一致就行，不要求非常硬性的规则
    int iIndex = (tWeekStartTime / SECONDS_AWEEK) % iChainLength;
    //1,2,3...iChainLength
    return iIndex + 1;
}

time_t NFTimeUtility::GetTodayStartTime2(time_t tTimeNow, int iOffset) {
    int iMinSecond = iOffset;
    time_t tNowLocal = tTimeNow + LOCAL_TIME_CORRECTION - iMinSecond;
    time_t tNowDays = tNowLocal / SECONDS_ADAY;
    time_t tNowDayStart = tNowDays * SECONDS_ADAY;
    tNowDayStart = tNowDayStart - LOCAL_TIME_CORRECTION + iMinSecond;
    return tNowDayStart;
}

time_t NFTimeUtility::GetThisWeekStartTime2(time_t tTime, int iOffset) {
    int iHourSecond = iOffset;
    time_t iLocalTime = tTime + LOCAL_TIME_CORRECTION - iHourSecond;
    time_t iLocalDayStartTime = (iLocalTime / SECONDS_ADAY) * SECONDS_ADAY;
    int iWeekDay = GetWeekDay127(tTime - iHourSecond);
    time_t iLocalThisWeekStartTime1 = iLocalDayStartTime - (iWeekDay - 1) * SECONDS_ADAY;
    return iLocalThisWeekStartTime1 - LOCAL_TIME_CORRECTION + iHourSecond;//to GMT
}

time_t NFTimeUtility::GetWeekDay2(time_t tTime, int iOffset) {
    return (tTime - NFTimeUtility::GetThisWeekStartTime2(tTime, iOffset)) / SECONDS_ADAY + 1;
}

void NFTimeUtility::GetThisMonthStartEndTime(time_t tTime, time_t &tBegin, time_t &tEnd, int iHour) {
    int iHourSecond = iHour * 3600;
    time_t tHourTime = tTime - iHourSecond;
    tm nowtm = *localtime(&tHourTime);
    nowtm.tm_hour = 0;
    nowtm.tm_min = 0;
    nowtm.tm_sec = 0;
    nowtm.tm_mday = 1;
    time_t tStartTime = mktime(&nowtm);
    int iMonAddDay = 30;

    if (nowtm.tm_mon == 1) //2月
    {
        iMonAddDay = 28;
    }

    time_t tEndTime = tStartTime + (iMonAddDay * SECONDS_ADAY);
    tm MonthEndtm = *localtime(&tEndTime);

    if (MonthEndtm.tm_mon == nowtm.tm_mon) {
        tEndTime += SECONDS_ADAY;
    }

    tBegin = tStartTime + iHourSecond;
    tEnd = tEndTime + iHourSecond;
}

/*
这个函数确定是正确的
tNow ,tTime是GMT（UTC）时间,+8后才是本地时间
*/
bool NFTimeUtility::IsAfterDayByUTCTime(time_t tNow, time_t tTime, int iHour) {
    if (tNow <= tTime) {
        return false;
    }

    int iHourSecond = iHour * 3600;
    //UTC转成本地时间
    tNow += LOCAL_TIME_CORRECTION - iHourSecond;
    tTime += LOCAL_TIME_CORRECTION - iHourSecond;
    time_t tNowDays = tNow / SECONDS_ADAY;
    time_t tTimeDays = tTime / SECONDS_ADAY;

    if (tNowDays > tTimeDays) {
        return true;
    }

    return false;
}

int NFTimeUtility::GetDayElapse(time_t tNow, time_t tTime, int iHour) {
    if (tNow <= tTime) {
        return 0;
    }

    int iHourSecond = iHour * 3600;
    //UTC转成本地时间
    time_t tNowLocal = tNow + LOCAL_TIME_CORRECTION - iHourSecond;
    time_t tTimeLocal = tTime + LOCAL_TIME_CORRECTION - iHourSecond;
    time_t tNowDays = tNowLocal / SECONDS_ADAY;
    time_t tTimeDays = tTimeLocal / SECONDS_ADAY;
    return tNowDays - tTimeDays;
}

uint32_t NFTimeUtility::GetAbsDay(time_t tNow, int iHour) {
    return (tNow + LOCAL_TIME_CORRECTION - iHour * SECONDS_AHOUR) / SECONDS_ADAY;
}

time_t NFTimeUtility::GetTodayStartTime(time_t tTimeNow, int iHour) {
    int iHourSecond = iHour * 3600;
    //UTC转成本地时间
    time_t tNowLocal = tTimeNow + LOCAL_TIME_CORRECTION - iHourSecond;
    time_t tNowDays = tNowLocal / SECONDS_ADAY;
    time_t tNowDayStart = tNowDays * SECONDS_ADAY;
    tNowDayStart = tNowDayStart - LOCAL_TIME_CORRECTION + iHourSecond;
    return tNowDayStart;
}

time_t NFTimeUtility::GetShiTuoFengYinBreakTime(time_t tBreakTime) {
    time_t tStandar = tBreakTime;
    tm *ptm = localtime(&tStandar);

    if (ptm->tm_hour >= 12) //当日12点-24点完成开门条件将在次日晚上8点开门
    {
        tStandar += SECONDS_ADAY;
    } else //当日0点-12点完成开门条件将在本日晚上8点开门
    {
        //do nothing
    }

    return GetTodayStartTime(tStandar) + 20 * 3600;
}

const char *NFTimeUtility::GetWeekDayStringByStartZero(uint8_t bWeekDayIndex) {
#if NF_PLATFORM != NF_PLATFORM_WIN
    switch (bWeekDayIndex) {
        case 0:
            return "星期一";

        case 1:
            return "星期二";

        case 2:
            return "星期三";

        case 3:
            return "星期四";

        case 4:
            return "星期五";

        case 5:
            return "星期六";

        case 6:
            return "星期日";
    }
#endif
    return "--";
}

int NFTimeUtility::GetWeekDayIndexWithStartHour(uint32_t dwTimeNow, uint8_t bStartHour) {
    return GetWeekDay127(dwTimeNow - bStartHour * 3600);
}

bool NFTimeUtility::IsOKWithWeekDayCtrl(uint8_t bWeekDayCtrlFlag, uint32_t dwTimeNow) {
    int iWeekDay = GetWeekDayIndexWithStartHour(dwTimeNow, GAME_RESET_HOUR_EVERYDAY);
    uint8_t bFlag = (0x1 << (iWeekDay - 1));

    if (BIT_ENABLED(bWeekDayCtrlFlag, bFlag)) {
        return true;
    }

    return false;
}

time_t NFTimeUtility::ToTimestamp(int iYear, int iMon, int iMDay/*day of the month */, int iHour, int iMin, int iSec) {
    tm timeinfo;
    memset(&timeinfo, 0, sizeof(timeinfo));

    timeinfo.tm_year = iYear - 1900;
    timeinfo.tm_mon = iMon - 1;

    timeinfo.tm_mday = iMDay;

    timeinfo.tm_hour = iHour;
    timeinfo.tm_min = iMin;
    timeinfo.tm_sec = iSec;

    time_t tTimestamp = mktime(&timeinfo);
    return tTimestamp;
    /*
    tm_wday  和 tm_yday  不需要设置，mktime会回填他们正确的值
	 from http://stackoverflow.com/questions/9575131/why-is-mktime-changing-the-year-day-of-my-tm-struct
    The original values of the tm_wday and tm_yday components of the structure are ignored, and the original values of the other components are not restricted to the ranges indicated above. On successful completion, the values of the tm_wday and tm_yday components of the structure are set appropriately,

    */

}

short NFTimeUtility::GetCurMonthDay(int year, int month) {
    switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12: {
            return 31;
        }
        case 4:
        case 6:
        case 9:
        case 11: {
            return 30;
        }
        case 2: {
            if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
                return 29;
            }

            break;
        }
        default : {
            return 0;
        }
    }

    return 28;
}

int64_t NFTimeUtility::GetNextMonthDay(int year, int month, int day, int &nMonth) {
    int64_t diffTime = 0;
    int j = 1;
    int tyear = year;

    for (int i = month; i <= 12 && j <= 12; ++i, ++j) {
        int tmon = i + 1;
        if (tmon > 12) {
            tmon = 1;
            tyear = year + 1;
        }

        int tday = (int) GetCurMonthDay(tyear, tmon);
        if (tday >= day) {//下个月的天数比当前的日期大或者等于，那么就出循环
            nMonth = tmon;

            break;
        } else {
            diffTime += (int64_t) tday * 24 * 3600 * 1000;
        }
    }

    if (j > 12) {
        diffTime = 0;
    } else {
        diffTime += (int64_t) GetCurMonthDay(year, month) * 24 * 3600 * 1000;
    }

    return diffTime;
}

int NFTimeUtility::GetOffsetToNextHour(time_t tCur) {
    int offset = 0;

    time_t tCurHour = tCur % SECONDS_AHOUR;

    offset = SECONDS_AHOUR - tCurHour;

    return offset;
}

int NFTimeUtility::GetOffsetToNextHalfHour(time_t tCur) {
    int offset = 0;

    time_t tCurHalfHour = tCur % SECONDS_HALF_AHOUR;

    offset = SECONDS_HALF_AHOUR - tCurHalfHour;

    return offset;
}

int NFTimeUtility::GetOffsetToNextDay(time_t tCur) {
    int offset = 0;

    time_t tCurDay = (tCur + LOCAL_TIME_CORRECTION) % SECONDS_ADAY;

    offset = SECONDS_ADAY - tCurDay;

    return offset;
}

int NFTimeUtility::GetYearByTimestamp(time_t tCur) {
    tm *ptm = localtime(&tCur);
    if (ptm) {
        return ptm->tm_year + 1900;
    }

    return 0;
}

int NFTimeUtility::GetDaysDelta(time_t tOne, time_t tTwo) {
    time_t tOneLocal = tOne + LOCAL_TIME_CORRECTION;
    time_t tTwoLocal = tTwo + LOCAL_TIME_CORRECTION;
    time_t tOneDays = tOneLocal / SECONDS_ADAY;
    time_t tTwoDays = tTwoLocal / SECONDS_ADAY;
    return tOneDays - tTwoDays;
}

time_t NFTimeUtility::GetThisWeekDayTime(time_t tNow, int iWeekDay) {
    iWeekDay -= 1;
    if (iWeekDay < 0) {
        iWeekDay = 0;
    } else if (iWeekDay >= 7) {
        iWeekDay %= 7;
    }

    time_t tStartTime = GetThisWeekStartTime1(tNow);

    return tStartTime + iWeekDay * 86400;
}

uint64_t NFTimeUtility::GetTimeByWeekDay(uint64_t ullWeek, int iDay) {
    return ullWeek + (iDay - 1) * SECONDS_ADAY;
}


// 获取时差，秒数
uint32_t NFTimeUtility::GetGMTSec()
{
    return NFTime::GetCurTimeZone()*3600;
}

// 获取本地时间（经过的秒数）
uint32_t NFTimeUtility::GetLocalTime()
{
    return NF_ADJUST_TIMENOW() + GetGMTSec();
}

// 起始日：1970年1月1日 星期四 为第0天
uint32_t NFTimeUtility::GetLocalDay()
{
    return GetLocalDay(NF_ADJUST_TIMENOW());
}

uint32_t NFTimeUtility::GetLocalDay(uint64_t unixSec)
{
    return floor((unixSec + GetGMTSec()) / (3600 * 24));
}

// 1970年1月1日 星期四 返回0
// 1970年1月4日 星期日 返回0
// 1970年1月5日 星期一 返回1
uint32_t NFTimeUtility::GetLocalWeek()
{
    return GetLocalWeek(NF_ADJUST_TIMENOW());
}

uint32_t NFTimeUtility::GetLocalWeek(uint64_t unixSec)
{
    uint32_t localDay = GetLocalDay(unixSec);
    return floor((localDay + 3) / 7);
}

uint32_t NFTimeUtility::GetLocalMonth(uint64_t unixSec)
{
    NFDate date;
    NFTime localTime(unixSec, 0);
    localTime.LocalDate(&date);
    return date.mon;
}

// 起始日：1970年1月1日 星期四
// 返回值  0 - 6(星期天、星期一到星期六）
uint32_t NFTimeUtility::GetLocalWeekDay()
{
    NFDate date;
    NFTime now = NFTime::Now();
    now.LocalDate(&date);
    return date.wday;
}

// 起始日：1970年1月1日 星期四
// 返回值  0 - 6(星期天、星期一到星期六）
uint32_t NFTimeUtility::GetLocalWeekDay(uint64_t unixSec)
{
    NFDate date;
    NFTime localTime(unixSec, 0);
    localTime.LocalDate(&date);
    return date.wday;
}

uint32_t NFTimeUtility::GetLocalMonthDay()
{
    NFDate date;
    NFTime now = NFTime::Now();
    now.LocalDate(&date);
    return date.mday;
}

uint32_t NFTimeUtility::GetDaysOfMonth()
{
    NFDate date;
    NFTime now = NFTime::Now();
    now.LocalDate(&date);
    
    return GetDaysOfMonth(date.year, date.mon);
}

uint32_t NFTimeUtility::GetDaysOfMonth(int year, int month)
{
    if (month < 1 || month > 12) return 0;
    static uint32_t days[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (2 == month && IsLeapYear(year)) return 29;
    return days[month - 1];
}

bool NFTimeUtility::LocalDateFormat(const char* fmt, char* timeStr, size_t len)
{
    return NFTime::Now().LocalDateFormat(fmt, timeStr, len);
}

std::string NFTimeUtility::GetLocalTimeStamp()
{
    char timeStamp[64] = {0};
    NFTime::Now().LocalDateFormat("%Y%m%d%H%M%S", timeStamp, 64);
    return timeStamp;
}

std::string NFTimeUtility::GetTimeStamp(uint64_t unixMSec)
{
    char timeStamp[64] = { 0 };
    NFTime(unixMSec).LocalDateFormat("%Y%m%d%H%M%S", timeStamp, 64);
    return timeStamp;
}

std::string NFTimeUtility::GetLocalTimeStampPrint()
{
    char timeStamp[64] = { 0 };
    NFTime::Now().LocalDateFormat("%Y_%m_%d %H:%M:%S", timeStamp, 64);
    return timeStamp;
}

std::string NFTimeUtility::GetTimeStampPrint(uint64_t unixMSec)
{
    char timeStamp[64] = { 0 };
    NFTime(unixMSec).LocalDateFormat("%Y_%m_%d %H:%M:%S", timeStamp, 64);
    return timeStamp;
}

// timestamp格式为：20150911111926
uint64_t NFTimeUtility::LocalTimeStampToUnixSec(const char* timestamp)
{
    char fmt[] = "%4d%2d%2d%2d%2d%2d";
    return LocalTimeStampToUnixSec(fmt, timestamp);
}

uint64_t NFTimeUtility::LocalTimeStampToUnixSec(const char* format, const char* timestamp)
{
    struct tm t;
    memset(&t, 0, sizeof(t));
    uint64_t unixSec = 0;
    
    int result = sscanf(timestamp, format,
                        &t.tm_year,
                        &t.tm_mon,
                        &t.tm_mday,
                        &t.tm_hour,
                        &t.tm_min,
                        &t.tm_sec);
    
    if (result != 6)
    {
        return 0;
    }
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    unixSec = (uint64_t)mktime(&t);
    
    return unixSec;
}

bool NFTimeUtility::CheckDayChange(uint64_t curUnixSec, uint64_t lastUnixSec, bool* weekChange)
{
    bool isDayChange = false;
    bool isWeekChange = false;
    
    uint32_t curDay = GetLocalDay(curUnixSec);
    uint32_t curWeek = GetLocalWeek(curUnixSec);
    uint32_t lastDay = GetLocalDay(lastUnixSec);
    uint32_t lastWeek = GetLocalWeek(lastUnixSec);
    
    // 避免不同LogicServer之间时差导致重刷并且方便测试后调时间时也能处理
    // 后调时间需要至少后调2天才会重新触发换天，正常情况下LogicServer之间时差不会超过5分钟
    if (curDay > lastDay || curDay < lastDay - 1)
    {
        isDayChange = true;
        if (curWeek != lastWeek)
        {
            isWeekChange = true;
        }
    }
    if (weekChange != nullptr)
    {
        *weekChange = isWeekChange;
    }
    return isDayChange;
}

// 检查是否是同一周
bool NFTimeUtility::CheckSameWeek(uint64_t curUnixSec, uint64_t lastUnixSec)
{
    return (GetLocalWeek(curUnixSec) == GetLocalWeek(lastUnixSec));
}

// 检查是否是同一天
bool NFTimeUtility::CheckSameDay(uint64_t curUnixSec, uint64_t lastUnixSec)
{
    return (GetLocalDay(curUnixSec) == GetLocalDay(lastUnixSec));
}

// 检查是否是同一个月
bool NFTimeUtility::CheckSameMonth(uint64_t curUnixSec, uint64_t lastUnixSec)
{
    return (GetLocalMonth(curUnixSec) == GetLocalMonth(lastUnixSec));
}


// 检查是否需要每日更新 返回值：true 需要每日更新，false 不需要每日更新
bool NFTimeUtility::CheckDayUpdate(uint64_t curUnixSec, uint64_t lastUnixSec)
{
    int32_t nHour = 0;//默认0点刷新
/*    const ConstantConstantCfgInfo *pConstCfg = g_GetConstantConstantCfgTable()->GetConstantConstantCfgInfo(0);
    if (nullptr != pConstCfg)
    {
        nHour = pConstCfg->constantdata;
        //这里校验一次范围,如果范围错误，直接设置成默认刷新时间
        if (nHour < 0 || nHour > 23)
        {
            nHour = 0;
        }
    }*/
    
    return CheckDayUpdate(curUnixSec, lastUnixSec, nHour);
}
//检查是否是同一周
bool NFTimeUtility::CheckWeekUpdate(uint64_t curUnixSec, uint64_t lastUnixSec)
{
    int32_t nHour = 0;//默认0点刷新
/*    const ConstantConstantCfgInfo *pConstCfg = g_GetConstantConstantCfgTable()->GetConstantConstantCfgInfo(0);
    if (nullptr != pConstCfg)
    {
        nHour = pConstCfg->constantdata;
        //这里校验一次范围,如果范围错误，直接设置成默认刷新时间
        if (nHour < 0 || nHour > 23)
        {
            nHour = 0;
        }
    }*/
    
    return CheckWeekUpdate(curUnixSec, lastUnixSec, nHour);
}

//通过当前时间获取每日更新的时间
uint64_t NFTimeUtility::GetDayUpdateTime(uint64_t curUnixSec)
{
    int32_t nHour = 0;//默认0点刷新
/*    const ConstantConstantCfgInfo *pConstCfg = g_GetConstantConstantCfgTable()->GetConstantConstantCfgInfo(0);
    if (nullptr != pConstCfg)
    {
        nHour = pConstCfg->constantdata;
        //这里校验一次范围,如果范围错误，直接设置成默认刷新时间
        if (nHour < 0 || nHour > 23)
        {
            nHour = 0;
        }
    }*/
    return NFTime::Now().GetDayUpdateTime(curUnixSec, nHour);
}
//通过当前时间获取每周更新的时间
uint64_t NFTimeUtility::GetWeekUpdateTime(uint64_t curUnixSec)
{
    int32_t nHour = 0;//默认0点刷新
/*    const ConstantConstantCfgInfo *pConstCfg = g_GetConstantConstantCfgTable()->GetConstantConstantCfgInfo(0);
    if (nullptr != pConstCfg)
    {
        nHour = pConstCfg->constantdata;
        //这里校验一次范围,如果范围错误，直接设置成默认刷新时间
        if (nHour < 0 || nHour > 23)
        {
            nHour = 0;
        }
    }*/
    return NFTime::Now().GetWeekUpdateTime(curUnixSec, nHour);
}

//检查是否是同一周
bool NFTimeUtility::CheckWeekUpdate(uint64_t curUnixSec, uint64_t lastUnixSec, int32_t nHour)
{
    //当前每周更新时间
    uint64_t curWeekUpdateTime = NFTime::Now().GetWeekUpdateTime(curUnixSec, nHour);
    if (CheckSameWeek(curUnixSec, lastUnixSec))
    {
        //如果当前时间大于 当前每周刷新时间，并且 最近一次的时间 小于当前每周更新时间，则需要每日更新
        if ((curUnixSec >= curWeekUpdateTime && lastUnixSec < curWeekUpdateTime))
        {
            return true;
        }
    }
    else if (curUnixSec >= (lastUnixSec + 3600 * 24 * 7))
    {
        //当前时间和上次时间不在同一周并且相差 超过一周，需要每周更新
        return true;
    }
    else if (curUnixSec >= curWeekUpdateTime)
    {
        //当前时间和上次时间不在同一周并且相差没有超过一周,判断当前时间是否超过了 当前时间的每周刷新时间，如果超过则更新，如果没超过，则不更新
        return true;
    }
    
    return false;
}

//检查是否需要每日更新 返回值：true 需要每日更新，false 不需要每日更新
bool NFTimeUtility::CheckDayUpdate(uint64_t curUnixSec, uint64_t lastUnixSec, int32_t nHour)
{
    //当前每日更新时间
    uint64_t curDayUpdateTime = NFTime::Now().GetDayUpdateTime(curUnixSec, nHour);
    if (CheckSameDay(curUnixSec, lastUnixSec))
    {
        //如果当前时间大于 当前每日刷新时间，并且 最近一次的时间 小于当前每日更新时间，则需要每日更新
        if (curUnixSec >= curDayUpdateTime && lastUnixSec < curDayUpdateTime)
        {
            return true;
        }
    }
    else if (curUnixSec >= (lastUnixSec + 3600 * 24))
    {
        //当前时间和上次时间不在同一天并且相差 超过一天，需要每日更新
        return true;
    }
    else if (curUnixSec >= curDayUpdateTime)
    {
        //当前时间和上次时间不在同一天并且相差没有超过一天,判断当前时间是否超过了 当前每日刷新时间，如果超过则更新，如果没超过，则不更新
        return true;
    }
    return false;
}

bool NFTimeUtility::BetweenDate(uint32_t startDate, uint32_t endData)
{
    NFDate date;
    NFTime now = NFTime::Now();
    now.LocalDate(&date);
    uint32_t totalDate = date.year * 10000 + date.mon * 100 + date.mday;
    return totalDate >= startDate && totalDate <= endData;
}