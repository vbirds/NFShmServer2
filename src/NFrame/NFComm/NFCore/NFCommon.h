// -------------------------------------------------------------------------
//    @FileName         :    NFCommon.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFCommon.h
 * @brief 通用工具函数集合
 * 
 * 此文件提供了一套全面的通用工具函数，涵盖字符串处理、类型转换、
 * 时间操作、数据编码解码、网络工具等多个方面。所有函数都是静态函数，
 * 可以直接调用而无需实例化对象。
 */

#pragma once

#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <stack>
#include <vector>
#include <regex>
#include <unordered_map>
#include <set>
#include "NFPlatform.h"

/**
 * @brief 通用工具类
 * 
 * NFCommon提供了一套完整的通用工具函数集合，专为高性能应用设计。
 * 所有函数都是静态函数，按功能分组，涵盖了日常开发中最常用的操作。
 * 
 * 主要功能模块：
 * - 位操作：GetBit、SetBit等位级操作函数
 * - 字符串处理：trim、大小写转换、分割、替换等字符串操作
 * - 时间处理：时间格式转换、解析、计算等时间相关操作
 * - 类型转换：各种数据类型之间的安全转换
 * - 编码解码：二进制数据与字符串的相互转换
 * - 网络工具：IP地址处理、网络数据转换等
 * - 数学计算：素数判断、数值计算等数学工具
 * - 容器操作：STL容器的序列化和反序列化
 * 
 * 设计特点：
 * - 纯静态函数：所有函数都是静态的，无需实例化
 * - 类型安全：使用模板和类型检查确保类型安全
 * - 异常安全：适当的异常处理和错误检查
 * - 性能优化：针对高频使用场景进行性能优化
 * - 跨平台：支持Windows、Linux等多种平台
 * 
 * 适用场景：
 * - 服务器开发中的通用工具需求
 * - 游戏开发中的数据处理
 * - 网络编程中的协议处理
 * - 系统编程中的底层操作
 * - 通用应用开发中的工具函数
 * 
 * 使用方法：
 * @code
 * // 字符串操作
 * std::string cleaned = NFCommon::trim("  hello world  ");
 * std::string upper = NFCommon::upper("hello");
 * 
 * // 时间操作
 * std::string timeStr = NFCommon::tm2str(tm);
 * time_t timestamp = NFCommon::str2time("2023-01-01 00:00:00");
 * 
 * // 类型转换
 * int value = NFCommon::strto<int>("123");
 * std::string str = NFCommon::tostr(456);
 * 
 * // 编码解码
 * std::string binStr = NFCommon::bin2str(data, len);
 * std::string decoded = NFCommon::str2bin(binStr);
 * 
 * // 位操作
 * bool bit = NFCommon::GetBit(value, 3);
 * NFCommon::SetBit(value, 3, true);
 * @endcode
 * 
 * @note 所有函数都是线程安全的，除非特别说明
 * @note 大部分函数都有异常安全保证
 * @note 字符串函数支持UTF-8编码
 */
class _NFExport NFCommon
{
public:

	/**
	 * @name 位操作函数
	 * @brief 提供位级别的数据操作功能
	 * @{
	 */
	
	/**
	 * @brief 获取指定位置的位值
	 * 
	 * 检查32位整数中指定位置的位是否为1。
	 * 
	 * @param src 源32位整数
	 * @param pos 位位置（0-31，0表示最低位）
	 * @return bool 指定位为1返回true，为0返回false
	 * 
	 * @pre pos必须在0-31范围内
	 * 
	 * 使用示例：
	 * @code
	 * uint32_t value = 0x0F; // 二进制: 00001111
	 * bool bit2 = NFCommon::GetBit(value, 2); // true
	 * bool bit5 = NFCommon::GetBit(value, 5); // false
	 * @endcode
	 */
	static bool GetBit(uint32_t src, uint32_t pos);
	
	/**
	 * @brief 设置指定位置的位值
	 * 
	 * 设置32位整数中指定位置的位为指定值。
	 * 
	 * @param src [in,out] 要修改的32位整数引用
	 * @param pos 位位置（0-31，0表示最低位）
	 * @param flag 要设置的位值，true表示1，false表示0
	 * 
	 * @pre pos必须在0-31范围内
	 * 
	 * 使用示例：
	 * @code
	 * uint32_t value = 0x00; // 二进制: 00000000
	 * NFCommon::SetBit(value, 2, true);  // 设置第2位为1
	 * NFCommon::SetBit(value, 5, false); // 设置第5位为0（本来就是0）
	 * // 现在value为0x04 (二进制: 00000100)
	 * @endcode
	 */
	static void SetBit(uint32_t &src, uint32_t pos, bool flag);

	/** @} */

	/**
	 * @name 字符串清理函数
	 * @brief 移除字符串首尾或指定位置的空白字符或指定字符
	 * @{
	 */

	/**
	 * @brief 去掉头部以及尾部的字符或字符串
	 * 
	 * 从字符串的开头和结尾移除指定的字符或字符串。
	 * 
	 * @param sStr 输入字符串
	 * @param s 需要去掉的字符或字符串，默认为空白字符
	 * @param bChar 如果为true，则去掉s中每个字符；如果为false，则去掉s字符串
	 * @return std::string 返回去掉后的字符串
	 * 
	 * 使用示例：
	 * @code
	 * std::string result1 = NFCommon::trim("  hello world  "); // "hello world"
	 * std::string result2 = NFCommon::trim("***hello***", "*", false); // "hello"
	 * std::string result3 = NFCommon::trim("abchelloabc", "abc", true); // "hello"
	 * @endcode
	 */
	static std::string trim(const std::string &sStr, const std::string &s = " \r\n\t", bool bChar = true);

	/**
	 * @brief 去掉左边的字符或字符串
	 * 
	 * 从字符串的开头移除指定的字符或字符串。
	 * 
	 * @param sStr 输入字符串
	 * @param s 需要去掉的字符或字符串，默认为空白字符
	 * @param bChar 如果为true，则去掉s中每个字符；如果为false，则去掉s字符串
	 * @return std::string 返回去掉后的字符串
	 */
	static std::string trimleft(const std::string &sStr, const std::string &s = " \r\n\t", bool bChar = true);

	/**
	 * @brief 去掉右边的字符或字符串
	 * 
	 * 从字符串的结尾移除指定的字符或字符串。
	 * 
	 * @param sStr 输入字符串
	 * @param s 需要去掉的字符或字符串，默认为空白字符
	 * @param bChar 如果为true，则去掉s中每个字符；如果为false，则去掉s字符串
	 * @return std::string 返回去掉后的字符串
	 */
	static std::string trimright(const std::string &sStr, const std::string &s = " \r\n\t", bool bChar = true);

	/** @} */

	/**
	 * @name 大小写转换函数
	 * @brief 字符串大小写转换功能
	 * @{
	 */

	/**
	 * @brief 字符串转换成小写
	 * 
	 * 将字符串中的所有大写字母转换为小写字母。
	 * 
	 * @param sString 输入字符串
	 * @return std::string 转换后的小写字符串
	 * 
	 * @note 只转换ASCII字符，非ASCII字符保持不变
	 */
	static std::string lower(const std::string &sString);

	/**
	 * @brief 字符串转换成大写
	 * 
	 * 将字符串中的所有小写字母转换为大写字母。
	 * 
	 * @param sString 输入字符串
	 * @return std::string 转换后的大写字符串
	 * 
	 * @note 只转换ASCII字符，非ASCII字符保持不变
	 */
	static std::string upper(const std::string &sString);

	/** @} */

	/**
	 * @name 字符串验证函数
	 * @brief 字符串内容格式验证
	 * @{
	 */

	/**
	 * @brief 检查字符串是否都是数字
	 * 
	 * 检查字符串是否只包含数字字符（0-9）。
	 * 
	 * @param sInput 要检查的字符串
	 * @return bool 如果字符串只包含数字返回true，否则返回false
	 * 
	 * @note 空字符串返回false
	 * @note 不支持负号和小数点
	 */
	static bool isdigit(const std::string &sInput);

	/** @} */

	/**
	 * @name 时间转换函数
	 * @brief 时间格式转换和解析功能
	 * @{
	 */

	/**
	 * @brief 字符串转换成时间结构
	 * 
	 * 根据指定格式将时间字符串解析为tm结构。
	 * 
	 * @param sString 时间字符串
	 * @param sFormat 时间格式字符串，使用strptime格式
	 * @param stTm [out] 输出的时间结构
	 * @return int 成功返回0，失败返回-1
	 * 
	 * 使用示例：
	 * @code
	 * struct tm tm;
	 * int result = NFCommon::str2tm("2023-01-01 12:30:45", "%Y-%m-%d %H:%M:%S", tm);
	 * @endcode
	 */
	static int str2tm(const std::string &sString, const std::string &sFormat, struct tm &stTm);

	/**
	 * @brief GMT格式的时间转化为时间结构
	 * 
	 * 将GMT格式的时间字符串（如"Sat, 06 Feb 2010 09:29:29 GMT"）转换为tm结构。
	 * 
	 * @param sString GMT格式的时间字符串
	 * @param stTm [out] 转换后的时间结构
	 * @return int 成功返回0，失败返回-1
	 * 
	 * @note 可以用mktime(&stTm) - timezone转换成本地时间的time_t值
	 * @note 需要extern long timezone声明
	 */
	static int strgmt2tm(const std::string &sString, struct tm &stTm);

	/**
	 * @brief 时间结构转换成字符串
	 * 
	 * 将tm结构格式化为指定格式的时间字符串。
	 * 
	 * @param stTm 时间结构
	 * @param sFormat 目标格式字符串，默认为紧凑格式"%Y%m%d%H%M%S"
	 * @return std::string 转换后的时间字符串
	 * 
	 * 使用示例：
	 * @code
	 * struct tm tm = {...};
	 * std::string timeStr1 = NFCommon::tm2str(tm); // "20230101123045"
	 * std::string timeStr2 = NFCommon::tm2str(tm, "%Y-%m-%d %H:%M:%S"); // "2023-01-01 12:30:45"
	 * @endcode
	 */
	static std::string tm2str(const struct tm &stTm, const std::string &sFormat = "%Y%m%d%H%M%S");

	/** @} */

	/**
	* @brief  时间转换成字符串.
	*
	* @param t        时间结构
	* @param sFormat  需要转换的目标格式，默认为紧凑格式
	* @return string  转换后的时间字符串
	*/
	static std::string tm2str(const time_t &t, const std::string &sFormat = "%Y%m%d%H%M%S");

	/**
	* @brief  当前时间转换成紧凑格式字符串
	* @param sFormat 格式，默认为紧凑格式
	* @return string 转换后的时间字符串
	*/
	static std::string now2str(const std::string &sFormat = "%Y%m%d%H%M%S");

	/**
	* @brief  时间转换成GMT字符串，GMT格式:Fri, 12 Jan 2001 18:18:18 GMT
	* @param stTm    时间结构
	* @return string GMT格式的时间字符串
	*/
	static std::string tm2GMTstr(const struct tm &stTm);

	/**
	* @brief  时间转换成GMT字符串，GMT格式:Fri, 12 Jan 2001 18:18:18 GMT
	* @param stTm    时间结构
	* @return string GMT格式的时间字符串
	*/
	static std::string tm2GMTstr(const time_t &t);

	/**
	* @brief  当前时间转换成GMT字符串，GMT格式:Fri, 12 Jan 2001 18:18:18 GMT
	* @return string GMT格式的时间字符串
	*/
	static std::string now2GMTstr();

	/**
	* @brief  当前的日期(年月日)转换成字符串(%Y%m%d).
	*
	* @return string (%Y%m%d)格式的时间字符串
	*/
	static std::string nowdate2str();

	/**
	* @brief  当前的时间(时分秒)转换成字符串(%H%M%S).
	*
	* @return string (%H%M%S)格式的时间字符串
	*/
	static std::string nowtime2str();

	/**
	* @brief  获取当前时间的的毫秒数.
	*
	* @return int64_t 当前时间的的毫秒数
	*/
	static int64_t now2ms();

	/**
	* @brief  取出当前时间的微秒.
	*
	* @return int64_t 取出当前时间的微秒
	*/
	static int64_t now2us();

	/**
	* @brief  字符串转化成T型，如果T是数值类型, 如果str为空,则T为0.
	*
	* @param sStr  要转换的字符串
	* @return T    T型类型
	*/
	template<typename T>
	static T strto(const std::string &sStr);

	/**
	* @brief  字符串转化成T型.
	*
	* @param sStr      要转换的字符串
	* @param sDefault  缺省值
	* @return T        转换后的T类型
	*/
	template<typename T>
	static T strto(const std::string &sStr, const std::string &sDefault);

	typedef bool(*depthJudge)(const std::string& str1, const std::string& str2);

	/**
	* @brief  解析字符串,用分隔符号分隔,保存在vector里
	*
	* 例子: |a|b||c|
	*
	* 如果withEmpty=true时, 采用|分隔为:"","a", "b", "", "c", ""
	*
	* 如果withEmpty=false时, 采用|分隔为:"a", "b", "c"
	*
	* 如果T类型为int等数值类型, 则分隔的字符串为"", 则强制转化为0
	*
	* @param sStr      输入字符串
	* @param sSep      分隔字符串(每个字符都算为分隔符)
	* @param withEmpty true代表空的也算一个元素, false时空的过滤
	* @param depthJudge 对分割后的字符再次进行判断
	* @return          解析后的字符vector
	*/
	template<typename T>
	static std::vector<T> sepstr(const std::string &sStr, const std::string &sSep, bool withEmpty = false, depthJudge judge = nullptr);

	/**
	* @brief T型转换成字符串，只要T能够使用ostream对象用<<重载,即可以被该函数支持
	* @param t 要转换的数据
	* @return  转换后的字符串
	*/
	template<typename T>
	static std::string tostr(const T &t);

	/**
	* @brief  vector转换成string.
	*
	* @param t 要转换的vector型的数据
	* @return  转换后的字符串
	*/
	template<typename T>
	static std::string tostr(const std::vector<T> &t);

    template<typename T>
    static std::string tostr(const std::set<T> &t);

    template<typename T>
    static std::string tostr(const std::multiset<T> &t);

	/**
	* @brief  把map输出为字符串.
	*
	* @param map<K, V, D, A>  要转换的map对象
	* @return                    string 输出的字符串
	*/
	template<typename K, typename V, typename D, typename A>
	static std::string tostr(const std::map<K, V, D, A> &t);

	/**
	* @brief  map输出为字符串.
	*
	* @param multimap<K, V, D, A>  map对象
	* @return                      输出的字符串
	*/
	template<typename K, typename V, typename D, typename A>
	static std::string tostr(const std::multimap<K, V, D, A> &t);

    /**
    * @brief  把map输出为字符串.
    *
    * @param map<K, V, D, A>  要转换的map对象
    * @return                    string 输出的字符串
    */
    template<typename K, typename V, typename D, typename A>
    static std::string tostr(const std::unordered_map<K, V, D, A> &t);

    /**
    * @brief  map输出为字符串.
    *
    * @param multimap<K, V, D, A>  map对象
    * @return                      输出的字符串
    */
    template<typename K, typename V, typename D, typename A>
    static std::string tostr(const std::unordered_multimap<K, V, D, A> &t);

	/**
	* @brief  pair 转化为字符串，保证map等关系容器可以直接用tostr来输出
	* @param pair<F, S> pair对象
	* @return           输出的字符串
	*/
	template<typename F, typename S>
	static std::string tostr(const std::pair<F, S> &itPair);

	/**
	* @brief  container 转换成字符串.
	*
	* @param iFirst  迭代器
	* @param iLast   迭代器
	* @param sSep    两个元素之间的分隔符
	* @return        转换后的字符串
	*/
	template <typename InputIter>
	static std::string tostr(InputIter iFirst, InputIter iLast, const std::string &sSep = "|");


	/**
	* @brief  二进制数据转换成字符串.
	*
	* @param buf     二进制buffer
	* @param len     buffer长度
	* @param sSep    分隔符
	* @param lines   多少个字节换一行, 默认0表示不换行
	* @return        转换后的字符串
	*/
	static std::string bin2str(const void *buf, size_t len, const std::string &sSep = "", size_t lines = 0);

	/**
	* @brief  二进制数据转换成字符串.
	*
	* @param sBinData  二进制数据
	* @param sSep     分隔符
	* @param lines    多少个字节换一行, 默认0表示不换行
	* @return         转换后的字符串
	*/
	static std::string bin2str(const std::string &sBinData, const std::string &sSep = "", size_t lines = 0);

	/**
	* @brief   字符串转换成二进制.
	*
	* @param psAsciiData 字符串
	* @param sBinData    二进制数据
	* @param iBinSize    需要转换的字符串长度
	* @return            转换长度，小于等于0则表示失败
	*/
	static int str2bin(const char *psAsciiData, unsigned char *sBinData, int iBinSize);

	/**
	* @brief  字符串转换成二进制.
	*
	* @param sBinData  要转换的字符串
	* @param sSep      分隔符
	* @param lines     多少个字节换一行, 默认0表示不换行
	* @return          转换后的二进制数据
	*/
	static std::string str2bin(const std::string &sBinData, const std::string &sSep = "", size_t lines = 0);

	/**
	* @brief  替换字符串.
	*
	* @param sString  输入字符串
	* @param sSrc     原字符串
	* @param sDest    目的字符串
	* @return string  替换后的字符串
	*/
	static std::string replace(const std::string &sString, const std::string &sSrc, const std::string &sDest);

	/**
	* @brief  批量替换字符串.
	*
	* @param sString  输入字符串
	* @param mSrcDest  map<原字符串,目的字符串>
	* @return string  替换后的字符串
	*/
	static std::string replace(const std::string &sString, const std::map<std::string, std::string>& mSrcDest);

	/**
	* @brief 匹配以.分隔的字符串，pat中*则代表通配符，代表非空的任何字符串
	* s为空, 返回false ，pat为空, 返回true
	* @param s    普通字符串
	* @param pat  带*则被匹配的字符串，用来匹配ip地址
	* @return     是否匹配成功
	*/
	static bool matchPeriod(const std::string& s, const std::string& pat);

	/**
	* @brief  匹配以.分隔的字符串.
	*
	* @param s   普通字符串
	* @param pat vector中的每个元素都是带*则被匹配的字符串，用来匹配ip地址
	* @return    是否匹配成功
	*/
	static bool matchPeriod(const std::string& s, const std::vector<std::string>& pat);

	/**
	* @brief  判断一个数是否为素数.
	*
	* @param n  需要被判断的数据
	* @return   true代表是素数，false表示非素数
	*/
	static bool isPrimeNumber(size_t n);

	/**
	* @brief  daemon
	*/
	static void daemon();

	/**
	* @brief  忽略管道异常
	*/
	static void ignorePipe();

	/**
	* @brief  将一个string类型转成一个字节 .
	*
	* @param sWhat 要转换的字符串
	* @return char    转换后的字节
	*/
	static char x2c(const std::string &sWhat);

	/**
	* @brief  大小字符串换成字节数，支持K, M, G两种 例如: 1K, 3M, 4G, 4.5M, 2.3G
	* @param s            要转换的字符串
	* @param iDefaultSize 格式错误时, 缺省的大小
	* @return             字节数
	*/
	static size_t toSize(const std::string &s, size_t iDefaultSize);

	static bool IsValidPhone(const std::string& phone);
};

namespace p
{
	template<typename D>
	struct strto1
	{
		D operator()(const std::string &sStr)
		{
			std::string s = "0";

			if (!sStr.empty())
			{
				s = sStr;
			}

			istringstream sBuffer(s);

			D t;
			sBuffer >> t;

			return t;
		}
	};

	template<>
	struct strto1<char>
	{
		char operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return sStr[0];
			}
			return 0;
		}
	};

	template<>
	struct strto1<short>
	{
		short operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return atoi(sStr.c_str());
			}
			return 0;
		}
	};

	template<>
	struct strto1<unsigned short>
	{
		unsigned short operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return (unsigned short)strtoul(sStr.c_str(), NULL, 10);
			}
			return 0;
		}
	};

	template<>
	struct strto1<int>
	{
		int operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return atoi(sStr.c_str());
			}
			return 0;
		}
	};

	template<>
	struct strto1<unsigned int>
	{
		unsigned int operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return strtoul(sStr.c_str(), NULL, 10);
			}
			return 0;
		}
	};

	template<>
	struct strto1<long>
	{
		long operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return atol(sStr.c_str());
			}
			return 0;
		}
	};

	template<>
	struct strto1<long long>
	{
		long long operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return atoll(sStr.c_str());
			}
			return 0;
		}
	};

	template<>
	struct strto1<unsigned long>
	{
		unsigned long operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return strtoul(sStr.c_str(), NULL, 10);
			}
			return 0;
		}
	};

	template<>
	struct strto1<float>
	{
		float operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return (float)atof(sStr.c_str());
			}
			return 0;
		}
	};

	template<>
	struct strto1<double>
	{
		double operator()(const std::string &sStr)
		{
			if (!sStr.empty())
			{
				return atof(sStr.c_str());
			}
			return 0;
		}
	};

	template<typename D>
	struct strto2
	{
		D operator()(const std::string &sStr)
		{
			istringstream sBuffer(sStr);

			D t;
			sBuffer >> t;

			return t;
		}
	};

	template<>
	struct strto2<std::string>
	{
		std::string operator()(const std::string &sStr)
		{
			return sStr;
		}
	};

    template<typename D>
    struct strto3
    {
        std::string operator()(const D &sStr)
        {
            stringstream sBuffer;
            sBuffer << sStr;
            return sBuffer.str();
        }
    };

    template <>
    struct strto3<bool>
    {
        std::string operator()(const bool &t)
        {
            char buf[2];
            buf[0] = t ? '1' : '0';
            buf[1] = '\0';
            return std::string(buf);
        }
    };


    template <>
    struct strto3<char>
    {
        std::string operator()(const char &t)
        {
            char buf[2];
            snprintf(buf, 2, "%c", t);
            return std::string(buf);
        }
    };

    template <>
    struct strto3<unsigned char>
    {
        std::string operator()(const unsigned char &t)
        {
            char buf[2];
            snprintf(buf, 2, "%c", t);
            return std::string(buf);
        }
    };


    template <>
    struct strto3<short>
    {
        std::string operator()(const short &t)
        {
            char buf[16];
            snprintf(buf, 16, "%d", t);
            return std::string(buf);
        }
    };

    template <>
    struct strto3<unsigned short>
    {
        std::string operator()(const unsigned short &t)
        {
            char buf[16];
            snprintf(buf, 16, "%u", t);
            return std::string(buf);
        }
    };

    template <>
    struct strto3<int>
    {
        std::string operator()(const int &t)
        {
            char buf[16];
            snprintf(buf, 16, "%d", t);
            return std::string(buf);
        }
    };

    template <>
    struct strto3<unsigned int>
    {
        std::string operator()(const unsigned int &t)
        {
            char buf[16];
            snprintf(buf, 16, "%u", t);
            return std::string(buf);
        }
    };


    template <>
    struct strto3<long>
    {
        std::string operator()(const long &t)
        {
            char buf[32];
            snprintf(buf, 32, "%ld", t);
            return std::string(buf);
        }
    };

    template <>
    struct strto3<long long>
    {
        std::string operator()(const long long &t)
        {
            char buf[32];
            snprintf(buf, 32, "%lld", t);
            return std::string(buf);
        }
    };

    template <>
    struct strto3<unsigned long>
    {
        std::string operator()(const unsigned long &t)
        {
            char buf[32];
            snprintf(buf, 32, "%lu", t);
            return std::string(buf);
        }
    };

    template <>
    struct strto3<float>
    {
        std::string operator()(const float &t)
        {
            char buf[32];
            snprintf(buf, 32, "%.5f", t);
            std::string s(buf);

            //去掉无效0, eg. 1.0300 -> 1.03;1.00 -> 1
            bool bFlag = false;
            int pos = int(s.size() - 1);
            for (; pos > 0; --pos)
            {
                if (s[pos] == '0')
                {
                    bFlag = true;
                    if (s[pos - 1] == '.')
                    {
                        //-2为了去掉"."号
                        pos -= 2;
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            if (bFlag)
                s = s.substr(0, pos + 1);

            return s;
        }
    };


    template <>
    struct strto3<double>
    {
        std::string operator()(const double &t)
        {
            char buf[32];
            snprintf(buf, 32, "%.5f", t);
            std::string s(buf);

            //去掉无效0, eg. 1.0300 -> 1.03;1.00 -> 1
            bool bFlag = false;
            int pos = int(s.size() - 1);
            for (; pos > 0; --pos)
            {
                if (s[pos] == '0')
                {
                    bFlag = true;
                    if (s[pos - 1] == '.')
                    {
                        //-2为了去掉"."号
                        pos -= 2;
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            if (bFlag)
                s = s.substr(0, pos + 1);

            return s;

        }
    };


    template <>
    struct strto3<long double>
    {
        std::string operator()(const long double &t)
        {
            char buf[32];
            snprintf(buf, 32, "%Lf", t);
            std::string s(buf);

            //去掉无效0, eg. 1.0300 -> 1.03;1.00 -> 1
            bool bFlag = false;
            int pos = int(s.size() - 1);
            for (; pos > 0; --pos)
            {
                if (s[pos] == '0')
                {
                    bFlag = true;
                    if (s[pos - 1] == '.')
                    {
                        //-2为了去掉"."号
                        pos -= 2;
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            if (bFlag)
                s = s.substr(0, pos + 1);

            return s;
        }
    };

    template<typename D>
    struct strto4
    {
        std::string operator()(const D &d)
        {
            stringstream sBuffer;
            sBuffer << d;

            return sBuffer.str();
        }
    };

    template<>
    struct strto4<std::string>
    {
        std::string operator()(const std::string &t)
        {
            return t;
        }
    };
}

template<typename T>
T NFCommon::strto(const std::string &sStr)
{
	using strto_type = typename std::conditional<std::is_arithmetic<T>::value, p::strto1<T>, p::strto2<T>>::type;

	return strto_type()(sStr);
}

template<typename T>
std::string NFCommon::tostr(const T &t)
{
    using strto_type = typename std::conditional<std::is_arithmetic<T>::value, p::strto3<T>, p::strto4<T>>::type;

    return strto_type()(t);
}

template<typename T>
T NFCommon::strto(const std::string &sStr, const std::string &sDefault)
{
	string s;

	if (!sStr.empty())
	{
		s = sStr;
	}
	else
	{
		s = sDefault;
	}

	return strto<T>(s);
}


template<typename T>
std::vector<T> NFCommon::sepstr(const std::string &sStr, const std::string &sSep, bool withEmpty, NFCommon::depthJudge judge)
{
	std::vector<T> vt;

	std::string::size_type pos = 0;
	std::string::size_type pos1 = 0;
	int pos_tmp = -1;

	while (true)
	{
		std::string s;
		std::string s1;
		pos1 = sStr.find_first_of(sSep, pos);
		if (pos1 == std::string::npos)
		{
			if (pos + 1 <= sStr.length())
			{
				s = sStr.substr(-1 != pos_tmp ? pos_tmp : pos);
				s1 = "";
			}
		}
		else if (pos1 == pos && (pos1 + 1 == sStr.length()))
		{
			s = "";
			s1 = "";
		}
		else
		{
			s = sStr.substr(-1 != pos_tmp ? pos_tmp : pos, pos1 - (-1 != pos_tmp ? pos_tmp : pos));
			s1 = sStr.substr(pos1 + 1);
			if (-1 == pos_tmp)
				pos_tmp = pos;
			pos = pos1;
		}

		if (nullptr == judge || judge(s, s1))
		{
			if (withEmpty)
			{
				vt.push_back(strto<T>(s));
			}
			else
			{
				if (!s.empty())
				{
					T tmp = strto<T>(s);
					vt.push_back(tmp);
				}
			}
			pos_tmp = -1;
		}

		if (pos1 == string::npos)
		{
			break;
		}

		pos++;
	}

	return vt;
}


template<typename T>
std::string NFCommon::tostr(const std::vector<T> &t)
{
	string s;
	for (size_t i = 0; i < t.size(); i++)
	{
		s += tostr(t[i]);
		s += " ";
	}
	return s;
}

template<typename T>
std::string NFCommon::tostr(const std::set<T> &t)
{
    string s;
    for (auto iter = t.begin(); iter != t.end(); iter++)
    {
        s += tostr(*iter);
        s += " ";
    }
    return s;
}

template<typename T>
std::string NFCommon::tostr(const std::multiset<T> &t)
{
    string s;
    for (auto iter = t.begin(); iter != t.end(); iter++)
    {
        s += tostr(*iter);
        s += " ";
    }
    return s;
}

template<typename K, typename V, typename D, typename A>
std::string NFCommon::tostr(const std::map<K, V, D, A> &t)
{
	string sBuffer;
	typename map<K, V, D, A>::const_iterator it = t.begin();
	while (it != t.end())
	{
		sBuffer += " [";
		sBuffer += tostr(it->first);
		sBuffer += "]=[";
		sBuffer += tostr(it->second);
		sBuffer += "] ";
		++it;
	}
	return sBuffer;
}

template<typename K, typename V, typename D, typename A>
std::string NFCommon::tostr(const std::multimap<K, V, D, A> &t)
{
	string sBuffer;
	typename multimap<K, V, D, A>::const_iterator it = t.begin();
	while (it != t.end())
	{
		sBuffer += " [";
		sBuffer += tostr(it->first);
		sBuffer += "]=[";
		sBuffer += tostr(it->second);
		sBuffer += "] ";
		++it;
	}
	return sBuffer;
}

template<typename K, typename V, typename D, typename A>
std::string NFCommon::tostr(const std::unordered_map<K, V, D, A> &t)
{
    string sBuffer;
    typename unordered_map<K, V, D, A>::const_iterator it = t.begin();
    while (it != t.end())
    {
        sBuffer += " [";
        sBuffer += tostr(it->first);
        sBuffer += "]=[";
        sBuffer += tostr(it->second);
        sBuffer += "] ";
        ++it;
    }
    return sBuffer;
}

template<typename K, typename V, typename D, typename A>
std::string NFCommon::tostr(const std::unordered_multimap<K, V, D, A> &t)
{
    string sBuffer;
    typename unordered_multimap<K, V, D, A>::const_iterator it = t.begin();
    while (it != t.end())
    {
        sBuffer += " [";
        sBuffer += tostr(it->first);
        sBuffer += "]=[";
        sBuffer += tostr(it->second);
        sBuffer += "] ";
        ++it;
    }
    return sBuffer;
}

template<typename F, typename S>
std::string NFCommon::tostr(const std::pair<F, S> &itPair)
{
	string sBuffer;
	sBuffer += "[";
	sBuffer += tostr(itPair.first);
	sBuffer += "]=[";
	sBuffer += tostr(itPair.second);
	sBuffer += "]";
	return sBuffer;
}

template <typename InputIter>
std::string NFCommon::tostr(InputIter iFirst, InputIter iLast, const std::string &sSep)
{
	std::string sBuffer;
	InputIter it = iFirst;

	while (it != iLast)
	{
		sBuffer += tostr(*it);
		++it;

		if (it != iLast)
		{
			sBuffer += sSep;
		}
		else
		{
			break;
		}
	}

	return sBuffer;
}

/**
 * @brief 分布式唯一ID生成器
 * 
 * NFIdGenerator提供了基于时间戳和服务器ID的分布式唯一ID生成功能。
 * 生成的ID具有时间有序性和全局唯一性，适用于分布式系统中的唯一标识需求。
 * 
 * ID生成原理：
 * - 高位：时间戳信息，保证时间有序性
 * - 低位：服务器ID和序列号，保证同一时刻的唯一性
 * - 组合：确保全局唯一且递增
 * 
 * 主要特性：
 * - 全局唯一：结合时间戳和服务器ID确保全局唯一性
 * - 时间有序：生成的ID按时间递增，支持时间排序
 * - 高性能：内存操作，无需网络通信或数据库查询
 * - 分布式安全：不同服务器ID保证分布式环境下的唯一性
 * - 故障恢复：服务重启后可以继续生成唯一ID
 * 
 * 适用场景：
 * - 分布式系统中的订单号生成
 * - 用户ID、商品ID等业务唯一标识
 * - 数据库主键（无需依赖数据库自增）
 * - 消息队列中的消息ID
 * - 日志追踪ID生成
 * 
 * 使用方法：
 * @code
 * // 每个服务器实例使用不同的server_id
 * NFIdGenerator generator(1001);  // 服务器ID为1001
 * 
 * // 生成唯一ID
 * uint64_t id1 = generator.GenId();
 * uint64_t id2 = generator.GenId();
 * uint64_t id3 = generator.GenId();
 * 
 * // ID保证递增：id1 < id2 < id3
 * assert(id1 < id2 && id2 < id3);
 * @endcode
 * 
 * @note 不同服务器实例必须使用不同的server_id以确保全局唯一性
 * @note 服务器时钟回拨可能影响ID的单调性，需要注意时间同步
 * @note 建议将server_id作为配置参数，避免硬编码
 */
class NFIdGenerator
{
private:
	/** @brief 服务器唯一标识ID */
	uint64_t m_nServerID;
	/** @brief 上次生成ID时的时间戳，用于检测时间变化 */
	uint64_t m_nLastTime;
	/** @brief 上次生成的ID值，用于确保同一时刻的唯一性 */
	uint64_t m_nLastID;

public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化ID生成器，设置服务器唯一标识。
	 * 每个服务器实例应该使用不同的server_id以确保生成ID的全局唯一性。
	 * 
	 * @param server_id 服务器唯一标识ID
	 * 
	 * @note server_id必须在所有服务器实例中保持唯一
	 * @note 建议server_id使用配置文件管理，避免冲突
	 */
	NFIdGenerator(uint64_t server_id);
	
	/**
	 * @brief 生成唯一ID
	 * 
	 * 生成一个64位的全局唯一ID。生成的ID具有以下特性：
	 * - 全局唯一：在所有服务器实例中都不重复
	 * - 时间有序：后生成的ID数值大于先生成的ID
	 * - 高性能：纯内存操作，无外部依赖
	 * 
	 * 生成算法：
	 * 1. 获取当前时间戳
	 * 2. 如果时间发生变化，重置序列号
	 * 3. 如果时间相同，序列号递增
	 * 4. 组合时间戳、服务器ID和序列号生成最终ID
	 * 
	 * @return uint64_t 生成的唯一ID
	 * 
	 * @note 此方法是线程安全的（如果类设计考虑了线程安全）
	 * @note 如果系统时钟回拨，可能会影响ID的单调性
	 * @note 高并发场景下建议评估性能是否满足需求
	 */
	uint64_t GenId();
};





