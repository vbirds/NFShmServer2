// -------------------------------------------------------------------------
//    @FileName         :    NFIniReader.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFIniReader.h
 * @brief INI配置文件读取器
 * 
 * 此文件提供了INI格式配置文件的解析和读取功能。支持标准的INI文件格式，
 * 包括节（section）、键值对（key=value）的解析，以及多种数据类型的获取。
 */

#pragma once

#include <cstdio>
#include <map>
#include <set>
#include <string>

#include "NFPlatform.h"

/**
 * @brief INI配置文件读取器
 * 
 * NFINIReader提供了完整的INI文件解析功能，支持读取各种数据类型的配置项。
 * 
 * INI文件格式：
 * ```
 * ; 这是注释
 * [section1]
 * key1=value1
 * key2=123
 * key3=true
 * 
 * [section2]
 * host=127.0.0.1
 * port=8080
 * ```
 * 
 * 主要特性：
 * - 支持标准INI格式解析
 * - 支持多种数据类型（字符串、整数、浮点数、布尔值）
 * - 提供默认值机制
 * - 支持十六进制数字
 * - 大小写不敏感的布尔值
 * - 错误信息提示
 * - 节和字段名枚举
 * 
 * 支持的数据类型：
 * - 字符串：任意文本
 * - 整数：十进制（123）、十六进制（0x7B）
 * - 浮点数：标准浮点格式
 * - 布尔值：true/false、yes/no、on/off、1/0（不区分大小写）
 * 
 * 使用方法：
 * @code
 * NFINIReader reader;
 * 
 * // 解析INI文件
 * if (reader.Parse("config.ini") == 0) {
 *     // 读取配置项
 *     std::string host = reader.Get("server", "host", "localhost");
 *     int port = reader.GetInt32("server", "port", 8080);
 *     bool debug = reader.GetBoolean("debug", "enabled", false);
 * 
 *     // 获取所有节名
 *     for (const auto& section : reader.GetSections()) {
 *         std::cout << "Section: " << section << std::endl;
 *     }
 * } else {
 *     std::cout << "解析失败: " << reader.GetLastError() << std::endl;
 * }
 * @endcode
 */
class _NFExport NFINIReader
{
public:
	/**
	 * @brief 构造函数
	 * 初始化INI读取器，清除错误信息
	 */
	explicit NFINIReader()
	{
		memset(m_last_error, 0, sizeof(m_last_error));
	}

	/**
	 * @brief 析构函数
	 */
	~NFINIReader()
	{
	}

	/**
	 * @brief 解析INI文件
	 * 
	 * 从指定的文件路径解析INI格式的配置文件。
	 * 每次调用会清除上一次的解析结果。
	 * 
	 * @param filename INI文件的路径
	 * @return int32_t 0表示成功，小于0表示失败
	 * 
	 * @note 每次调用会清除上一次的解析结果
	 * @note 失败时可通过GetLastError()获取错误信息
	 */
	int32_t Parse(const std::string& filename);

	/**
	 * @brief 清除解析结果
	 * 
	 * 清除当前存储的所有解析结果，包括节、字段和错误信息。
	 */
	void Clear();

	/**
	 * @brief 获取字符串类型字段的值
	 * 
	 * 从指定节中获取字符串类型的配置项值。
	 * 如果字段未配置或节不存在，则使用默认值。
	 * 
	 * @param section 节名
	 * @param name 字段名
	 * @param default_value 默认值，当字段不存在时返回
	 * @return std::string 字段值或默认值
	 */
	std::string Get(const std::string& section, const std::string& name,
	                const std::string& default_value);

	/**
	 * @brief 获取32位有符号整数值
	 * 
	 * 从INI文件中获取整数值，支持十进制（如"1234"、"-1234"）
	 * 和十六进制（如"0x4d2"）格式。
	 * 
	 * @param section 节名
	 * @param name 字段名
	 * @param default_value 默认值，当字段不存在或无效时返回
	 * @return int32_t 字段值或默认值
	 */
	int32_t GetInt32(const std::string& section, const std::string& name,
	                 int32_t default_value);

	/**
	 * @brief 获取32位无符号整数值
	 * 
	 * 从INI文件中获取无符号整数值，支持十进制和十六进制格式。
	 * 
	 * @param section 节名
	 * @param name 字段名
	 * @param default_value 默认值，当字段不存在或无效时返回
	 * @return uint32_t 字段值或默认值
	 */
	uint32_t GetUInt32(const std::string& section, const std::string& name,
	                   uint32_t default_value);

	/**
	 * @brief 获取64位有符号整数值
	 * 
	 * 从INI文件中获取64位整数值，支持十进制和十六进制格式。
	 * 
	 * @param section 节名
	 * @param name 字段名
	 * @param default_value 默认值，当字段不存在或无效时返回
	 * @return int64_t 字段值或默认值
	 */
	int64_t GetInt64(const std::string& section, const std::string& name,
	                 int64_t default_value);

	/**
	 * @brief 获取64位无符号整数值
	 * 
	 * 从INI文件中获取64位无符号整数值，支持十进制和十六进制格式。
	 * 
	 * @param section 节名
	 * @param name 字段名
	 * @param default_value 默认值，当字段不存在或无效时返回
	 * @return uint64_t 字段值或默认值
	 */
	uint64_t GetUInt64(const std::string& section, const std::string& name,
	                   uint64_t default_value);

	/**
	 * @brief 获取浮点数值
	 * 
	 * 从INI文件中获取双精度浮点数值，使用strtod()进行解析。
	 * 
	 * @param section 节名
	 * @param name 字段名
	 * @param default_value 默认值，当字段不存在或无效时返回
	 * @return double 字段值或默认值
	 */
	double GetReal(const std::string& section, const std::string& name, double default_value);

	/**
	 * @brief 获取布尔值
	 * 
	 * 从INI文件中获取布尔值。支持的真值："true"、"yes"、"on"、"1"；
	 * 支持的假值："false"、"no"、"off"、"0"（不区分大小写）。
	 * 
	 * @param section 节名
	 * @param name 字段名
	 * @param default_value 默认值，当字段不存在或无效时返回
	 * @return bool 字段值或默认值
	 */
	bool GetBoolean(const std::string& section, const std::string& name, bool default_value);

	/**
	 * @brief 获取所有节名
	 * 
	 * 返回INI文件中所有的节名，按字母顺序排序，但保持原始大小写。
	 * 
	 * @return const std::set<std::string>& 所有节名的集合
	 */
	const std::set<std::string>& GetSections() const;

	/**
	 * @brief 获取指定节中的所有字段名
	 * 
	 * 返回指定节中所有的字段名，按字母顺序排序，但保持原始大小写。
	 * 如果节不存在，返回空集合。
	 * 
	 * @param section 节名
	 * @return std::set<std::string> 字段名的集合，节不存在时返回空集合
	 */
	std::set<std::string> GetFields(const std::string& section);

	/**
	 * @brief 获取最后的错误信息
	 * 
	 * 返回最后一次操作失败时的错误描述字符串。
	 * 
	 * @return const char* 错误信息的C字符串，无错误时返回空字符串
	 */
	const char* GetLastError() const
	{
		return m_last_error;
	}

private:
	/**
	 * @brief 解析文件流
	 * 
	 * 内部方法，解析已打开的文件流。
	 * 
	 * @param file 已打开的文件指针
	 * @return int32_t 0表示成功，小于0表示失败
	 */
	int32_t ParseFile(FILE* file);

private:
	/** @brief 错误信息缓冲区 */
	char m_last_error[256];
	/** @brief 存储所有节名的集合 */
	std::set<std::string> m_sections;
	/** @brief 存储所有字段的嵌套映射：节名 -> (字段名 -> 字段值) */
	std::map<std::string, std::map<std::string, std::string>> m_fields;
};

