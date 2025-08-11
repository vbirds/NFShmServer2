/**
 * @file NFJson.h
 * @brief JSON数据处理类
 * 
 * 此文件提供了轻量级、高性能的JSON数据处理功能。支持JSON的解析、
 * 构造、序列化和反序列化操作。采用现代C++设计，提供类型安全的接口。
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <memory>
#include <initializer_list>
#include <type_traits>

#include "NFPlatform.h"

/**
 * @name Visual Studio兼容性定义
 * @brief 为老版本Visual Studio提供兼容性支持
 * @{
 */
#ifdef _MSC_VER
#if _MSC_VER <= 1800 // VS 2013

#ifndef noexcept
/** @brief 为VS2013提供noexcept支持 */
#define noexcept throw()
#endif

#ifndef snprintf
/** @brief 为VS2013提供安全sprintf支持 */
#define snprintf _snprintf_s
#endif
#endif
#endif
/** @} */

/**
 * @brief JSON解析模式枚举
 * 
 * 定义JSON解析时的不同模式，控制解析器的行为。
 */
enum JsonParse
{
	/** @brief 标准模式：严格按照JSON标准解析 */
	STANDARD,
	/** @brief 注释模式：允许解析包含注释的JSON */
	COMMENTS
};

class JsonValue;

/**
 * @brief JSON数据处理类
 * 
 * NFJson是一个轻量级、高性能的JSON库，提供完整的JSON数据处理功能。
 * 支持所有标准JSON数据类型，并提供现代C++风格的API。
 * 
 * 主要特性：
 * - 类型安全：编译时类型检查，运行时类型验证
 * - 高性能：零拷贝优化，移动语义支持
 * - 易用性：直观的API设计，支持链式调用
 * - 兼容性：支持标准JSON和带注释的JSON
 * - 现代C++：使用C++11/14特性，RAII设计
 * 
 * 支持的JSON类型：
 * - null: 空值
 * - number: 数字（整数和浮点数）
 * - bool: 布尔值
 * - string: 字符串
 * - array: 数组
 * - object: 对象
 * 
 * 使用方法：
 * @code
 * // 创建JSON对象
 * NFJson obj = NFJson::object {
 *     {"name", "张三"},
 *     {"age", 25},
 *     {"skills", NFJson::array {"C++", "Python", "Java"}}
 * };
 * 
 * // 访问数据
 * std::string name = obj["name"].string_value();
 * int age = obj["age"].int_value();
 * 
 * // 序列化
 * std::string jsonStr = obj.dump();
 * 
 * // 解析JSON字符串
 * std::string err;
 * NFJson parsed = NFJson::parse(jsonStr, err);
 * @endcode
 */
class _NFExport NFJson final
{
public:
	/**
	 * @brief JSON数据类型枚举
	 * 
	 * 定义NFJson支持的所有数据类型。
	 */
	enum Type
	{
		/** @brief 空值类型 */
		NUL,
		/** @brief 数字类型（整数或浮点数） */
		NUMBER,
		/** @brief 布尔类型 */
		BOOL,
		/** @brief 字符串类型 */
		STRING,
		/** @brief 数组类型 */
		ARRAY,
		/** @brief 对象类型 */
		OBJECT
	};

	/** @brief JSON数组类型定义 */
	typedef std::vector<NFJson> array;
	/** @brief JSON对象类型定义 */
	typedef std::map<std::string, NFJson> object;

	/**
	 * @name 构造函数
	 * @brief 各种JSON数据类型的构造函数
	 * @{
	 */
	
	/**
	 * @brief 默认构造函数，创建null类型的JSON对象
	 */
	NFJson() noexcept;
	
	/**
	 * @brief nullptr构造函数，创建null类型的JSON对象
	 * @param nullptr_t 空指针类型
	 */
	NFJson(std::nullptr_t) noexcept;
	
	/**
	 * @brief 双精度浮点数构造函数
	 * @param value 浮点数值
	 */
	NFJson(double value);
	
	/**
	 * @brief 整数构造函数
	 * @param value 整数值
	 */
	NFJson(int value);
	
	/**
	 * @brief 布尔值构造函数
	 * @param value 布尔值
	 */
	NFJson(bool value);
	
	/**
	 * @brief 字符串构造函数（拷贝语义）
	 * @param value 字符串值
	 */
	NFJson(const std::string& value);
	
	/**
	 * @brief 字符串构造函数（移动语义）
	 * @param value 字符串值（右值引用）
	 */
	NFJson(std::string&& value);
	
	/**
	 * @brief C字符串构造函数
	 * @param value C风格字符串
	 */
	NFJson(const char* value);
	
	/**
	 * @brief 数组构造函数（拷贝语义）
	 * @param values JSON数组
	 */
	NFJson(const array& values);
	
	/**
	 * @brief 数组构造函数（移动语义）
	 * @param values JSON数组（右值引用）
	 */
	NFJson(array&& values);
	
	/**
	 * @brief 对象构造函数（拷贝语义）
	 * @param values JSON对象
	 */
	NFJson(const object& values);
	
	/**
	 * @brief 对象构造函数（移动语义）
	 * @param values JSON对象（右值引用）
	 */
	NFJson(object&& values);

	/**
	 * @brief 隐式构造函数，支持有to_json()方法的类型
	 * 
	 * 任何有to_json()成员函数的类型都可以自动转换为NFJson。
	 * 
	 * @tparam T 源类型，必须有to_json()方法
	 * @param t 要转换的对象
	 */
	template <class T, class = decltype(&T::to_json)>
	NFJson(const T& t) : NFJson(t.to_json())
	{
	}

	/**
	 * @brief 禁用void指针构造函数
	 * 
	 * 防止NFJson(some_pointer)意外产生bool类型。
	 * 如需要该行为，请使用NFJson(bool(some_pointer))。
	 */
	NFJson(void*) = delete;

	/** @} */

	/**
	 * @name 类型查询方法
	 * @brief 查询JSON对象的类型
	 * @{
	 */
	
	/**
	 * @brief 获取JSON对象的类型
	 * @return Type 对象的类型枚举值
	 */
	Type type() const;

	/**
	 * @brief 检查是否为null类型
	 * @return bool 如果是null类型返回true
	 */
	bool is_null() const
	{
		return type() == NUL;
	}

	/**
	 * @brief 检查是否为数字类型
	 * @return bool 如果是数字类型返回true
	 */
	bool is_number() const
	{
		return type() == NUMBER;
	}

	/**
	 * @brief 检查是否为布尔类型
	 * @return bool 如果是布尔类型返回true
	 */
	bool is_bool() const
	{
		return type() == BOOL;
	}

	/**
	 * @brief 检查是否为字符串类型
	 * @return bool 如果是字符串类型返回true
	 */
	bool is_string() const
	{
		return type() == STRING;
	}

	/**
	 * @brief 检查是否为数组类型
	 * @return bool 如果是数组类型返回true
	 */
	bool is_array() const
	{
		return type() == ARRAY;
	}

	/**
	 * @brief 检查是否为对象类型
	 * @return bool 如果是对象类型返回true
	 */
	bool is_object() const
	{
		return type() == OBJECT;
	}

	/**
	 * @brief 获取数字类型的值
	 * @return double 如果类型为NUMBER，返回数值；否则返回0
	 */
	double number_value() const;
	/**
	 * @brief 获取整数类型的值
	 * @return int 如果类型为NUMBER，返回数值；否则返回0
	 */
	int int_value() const;

	/**
	 * @brief 获取布尔类型的值
	 * @return bool 如果类型为BOOL，返回值；否则返回false
	 */
	bool bool_value() const;
	/**
	 * @brief 获取字符串类型的值
	 * @return const std::string& 如果类型为STRING，返回字符串；否则返回空字符串
	 */
	const std::string& string_value() const;
	/**
	 * @brief 获取数组类型的值
	 * @return const array& 如果类型为ARRAY，返回数组；否则返回空数组
	 */
	const array& array_items() const;
	/**
	 * @brief 获取对象类型的值
	 * @return const object& 如果类型为OBJECT，返回对象；否则返回空对象
	 */
	const object& object_items() const;

	/** @} */

	/**
	 * @name 访问元素方法
	 * @brief 通过索引或键访问JSON对象的元素
	 * @{
	 */
	
	/**
	 * @brief 通过索引访问数组元素
	 * @param i 数组索引
	 * @return const NFJson& 如果类型为ARRAY，返回索引处的元素；否则返回null对象
	 */
	const NFJson& operator[](size_t i) const;
	
	/**
	 * @brief 通过键访问对象元素
	 * @param key 键名
	 * @return const NFJson& 如果类型为OBJECT，返回键对应的元素；否则返回null对象
	 */
	const NFJson& operator[](const std::string& key) const;

	/** @} */

	/**
	 * @name 序列化方法
	 * @brief 将JSON对象序列化为字符串
	 * @{
	 */
	
	/**
	 * @brief 将JSON对象序列化为字符串
	 * @param out 输出字符串
	 */
	void dump(std::string& out) const;

	/**
	 * @brief 将JSON对象序列化为字符串
	 * @return std::string 序列化后的字符串
	 */
	std::string dump() const
	{
		std::string out;
		dump(out);
		return out;
	}

	/** @} */

	/**
	 * @name 解析方法
	 * @brief 解析JSON字符串
	 * @{
	 */
	
	/**
	 * @brief 解析JSON字符串
	 * @param in JSON字符串
	 * @param err 错误信息
	 * @param strategy 解析策略
	 * @return NFJson 解析后的JSON对象
	 */
	static NFJson parse(const std::string& in,
	                    std::string& err,
	                    JsonParse strategy = JsonParse::STANDARD);

	/**
	 * @brief 解析JSON字符串
	 * @param in JSON字符串
	 * @param err 错误信息
	 * @param strategy 解析策略
	 * @return NFJson 解析后的JSON对象
	 */
	static NFJson parse(const char* in,
	                    std::string& err,
	                    JsonParse strategy = JsonParse::STANDARD)
	{
		if (in)
		{
			return parse(std::string(in), err, strategy);
		}
		else
		{
			err = "null input";
			return nullptr;
		}
	}

	/**
	 * @brief 解析多个JSON对象
	 * @param in JSON字符串
	 * @param parser_stop_pos 解析停止位置
	 * @param err 错误信息
	 * @param strategy 解析策略
	 * @return std::vector<NFJson> 解析后的JSON对象数组
	 */
	static std::vector<NFJson> parse_multi(
		const std::string& in,
		std::string::size_type& parser_stop_pos,
		std::string& err,
		JsonParse strategy = JsonParse::STANDARD);

	/**
	 * @brief 解析多个JSON对象
	 * @param in JSON字符串
	 * @param err 错误信息
	 * @param strategy 解析策略
	 * @return std::vector<NFJson> 解析后的JSON对象数组
	 */
	static inline std::vector<NFJson> parse_multi(
		const std::string& in,
		std::string& err,
		JsonParse strategy = JsonParse::STANDARD)
	{
		std::string::size_type parser_stop_pos;
		return parse_multi(in, parser_stop_pos, err, strategy);
	}

	/** @} */

	/**
	 * @name 比较运算符
	 * @brief 比较两个NFJson对象
	 * @{
	 */
	
	/**
	 * @brief 等于运算符
	 * @param rhs 右值NFJson对象
	 * @return bool 如果两个对象相等返回true
	 */
	bool operator==(const NFJson& rhs) const;
	
	/**
	 * @brief 小于运算符
	 * @param rhs 右值NFJson对象
	 * @return bool 如果当前对象小于右值返回true
	 */
	bool operator<(const NFJson& rhs) const;

	/**
	 * @brief 不等于运算符
	 * @param rhs 右值NFJson对象
	 * @return bool 如果两个对象不相等返回true
	 */
	bool operator!=(const NFJson& rhs) const
	{
		return !(*this == rhs);
	}

	/**
	 * @brief 小于等于运算符
	 * @param rhs 右值NFJson对象
	 * @return bool 如果当前对象小于等于右值返回true
	 */
	bool operator<=(const NFJson& rhs) const
	{
		return !(rhs < *this);
	}

	/**
	 * @brief 大于运算符
	 * @param rhs 右值NFJson对象
	 * @return bool 如果当前对象大于右值返回true
	 */
	bool operator>(const NFJson& rhs) const
	{
		return (rhs < *this);
	}

	/**
	 * @brief 大于等于运算符
	 * @param rhs 右值NFJson对象
	 * @return bool 如果当前对象大于等于右值返回true
	 */
	bool operator>=(const NFJson& rhs) const
	{
		return !(*this < rhs);
	}

	/** @} */

	/**
	 * @name 形状检查方法
	 * @brief 检查JSON对象是否符合指定形状
	 * @{
	 */
	
	/**
	 * @brief 形状定义
	 * 
	 * 用于描述JSON对象的期望形状，包含键名和对应的类型。
	 */
	typedef std::initializer_list<std::pair<std::string, Type>> shape;

	/**
	 * @brief 检查JSON对象是否符合形状
	 * 
	 * 如果JSON对象是对象类型，并且对于types中的每一项，
	 * 对象都包含一个键名与该类型匹配的键，则返回true。
	 * 否则返回false，并设置err为描述性消息。
	 * 
	 * @param types 形状定义
	 * @param err 错误信息
	 * @return bool 如果符合形状返回true，否则返回false
	 */
	bool has_shape(const shape& types, std::string& err) const;

	/** @} */

private:
	std::shared_ptr<JsonValue> m_ptr;
};

// Internal class hierarchy - JsonValue objects are not exposed to users of this API.
class JsonValue
{
protected:
	friend class NFJson;
	friend class JsonInt;
	friend class JsonDouble;
	virtual NFJson::Type type() const = 0;
	virtual bool equals(const JsonValue* other) const = 0;
	virtual bool less(const JsonValue* other) const = 0;
	virtual void dump(std::string& out) const = 0;
	virtual double number_value() const;
	virtual int int_value() const;
	virtual bool bool_value() const;
	virtual const std::string& string_value() const;
	virtual const NFJson::array& array_items() const;
	virtual const NFJson& operator[](size_t i) const;
	virtual const NFJson::object& object_items() const;
	virtual const NFJson& operator[](const std::string& key) const;

	virtual ~JsonValue()
	{
	}
};

