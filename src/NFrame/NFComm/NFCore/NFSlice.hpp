// -------------------------------------------------------------------------
//    @FileName         :    NFSlice.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFSlice.hpp
 * @brief 内存切片类定义
 * 
 * 此文件定义了一个轻量级的内存切片类，用于高效地引用内存中的数据段。
 * 该实现从LevelDB项目移植而来，提供了对内存数据的无拷贝引用。
 * 
 * 基于LevelDB项目：
 * @see https://github.com/google/leveldb/blob/master/include/leveldb/slice.h
 */

#pragma once

#include <string.h>
#include <assert.h>
#include <string>
#include "NFPlatform.h"

// Copy from leveldb project
// @see https://github.com/google/leveldb/blob/master/include/leveldb/slice.h

/**
 * @brief 内存切片类
 * 
 * NFSlice是一个轻量级的字符串类，提供对内存中连续字节序列的引用。
 * 它不拥有数据的所有权，只是提供一个视图，因此创建和销毁的开销很小。
 * 
 * 主要特性：
 * - 零拷贝：不复制数据，只引用现有内存
 * - 轻量级：只包含指针和长度信息
 * - 兼容性：可以从std::string、C字符串等构造
 * - 高效比较：提供高效的字典序比较
 * 
 * 注意事项：
 * - 不负责内存管理，调用者需确保被引用的内存在切片生命周期内有效
 * - 修改原始数据会影响切片的内容
 * 
 * 使用方法：
 * @code
 * std::string str = "Hello World";
 * NFSlice slice(str);
 * NFSlice sub_slice(slice.data() + 6, 5); // "World"
 * @endcode
 */
class NFSlice
{
public:
	/** @brief 值类型定义 */
	typedef char value_type;

public:
	/**
	 * @brief 默认构造函数
	 * 创建一个空的切片，指向空字符串
	 */
	NFSlice() : data_(""), size_(0)
	{
	}

	/**
	 * @brief 构造函数
	 * 创建一个引用指定内存区域的切片
	 * @param d 指向内存数据的指针
	 * @param n 数据长度（字节数）
	 */
	NFSlice(const char* d, size_t n) : data_(d), size_(n)
	{
	}

	/**
	 * @brief 从std::string构造
	 * 创建一个引用std::string内容的切片
	 * @param s 要引用的字符串
	 */
	NFSlice(const std::string& s) : data_(s.data()), size_(s.size())
	{
	}

	/**
	 * @brief 从C字符串构造
	 * 创建一个引用C字符串的切片，自动计算长度
	 * @param s 以null结尾的C字符串
	 */
	NFSlice(const char* s) : data_(s), size_(strlen(s))
	{
	}

	/**
	 * @brief 获取数据指针
	 * @return const char* 指向引用数据开头的指针
	 */
	const char* data() const
	{
		return data_;
	}

	/**
	 * @brief 获取数据长度
	 * @return size_t 引用数据的长度（字节数）
	 */
	size_t size() const
	{
		return size_;
	}

	/**
	 * @brief 检查是否为空
	 * @return bool 如果引用数据长度为零则返回true
	 */
	bool empty() const
	{
		return size_ == 0;
	}

	/**
	 * @brief 下标操作符
	 * 返回引用数据中第n个字节
	 * @param n 字节索引
	 * @return char 第n个字节的值
	 * @note 要求：n < size()
	 */
	char operator[](size_t n) const
	{
		assert(n < size());
		return data_[n];
	}

	/**
	 * @brief 清空切片
	 * 将此切片重置为引用空数组
	 */
	void clear()
	{
		data_ = "";
		size_ = 0;
	}

	/**
	 * @brief 移除前缀
	 * 从此切片的开头删除前n个字节
	 * @param n 要删除的字节数
	 * @note 要求：n <= size()
	 */
	void remove_prefix(size_t n)
	{
		assert(n <= size());
		data_ += n;
		size_ -= n;
	}

	/**
	 * @brief 转换为字符串
	 * 返回包含引用数据副本的字符串
	 * @return std::string 数据的字符串副本
	 */
	std::string ToString() const
	{
		return std::string(data_, size_);
	}

	/**
	 * @brief 三路比较
	 * 
	 * 执行字典序比较，返回值：
	 * - < 0 如果 "*this" < "b"
	 * - == 0 如果 "*this" == "b"  
	 * - > 0 如果 "*this" > "b"
	 * 
	 * @param b 要比较的另一个切片
	 * @return int 比较结果
	 */
	int compare(const NFSlice& b) const;

private:
	/** @brief 指向数据的指针 */
	const char* data_;
	/** @brief 数据长度 */
	size_t size_;
};

/**
 * @brief 相等比较操作符
 * @param x 第一个切片
 * @param y 第二个切片
 * @return bool 如果两个切片内容相等则返回true
 */
inline bool operator==(const NFSlice& x, const NFSlice& y)
{
	return ((x.size() == y.size()) &&
		(memcmp(x.data(), y.data(), x.size()) == 0));
}

/**
 * @brief 不等比较操作符
 * @param x 第一个切片
 * @param y 第二个切片
 * @return bool 如果两个切片内容不相等则返回true
 */
inline bool operator!=(const NFSlice& x, const NFSlice& y)
{
	return !(x == y);
}

/**
 * @brief 小于比较操作符
 * @param x 第一个切片
 * @param y 第二个切片
 * @return bool 如果x在字典序上小于y则返回true
 */
inline bool operator<(const NFSlice& x, const NFSlice& y)
{
	return x.compare(y) < 0;
}

/**
 * @brief 三路比较实现
 * 
 * 使用memcmp进行高效的字节级比较，对于相同长度的前缀，
 * 较短的切片被认为小于较长的切片。
 * 
 * @param b 要比较的切片
 * @return int 比较结果：负数表示小于，0表示等于，正数表示大于
 */
inline int NFSlice::compare(const NFSlice& b) const
{
	const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
	int r = memcmp(data_, b.data_, min_len);

	if (r == 0)
	{
		if (size_ < b.size_)
		{
			r = -1;
		}
		else if (size_ > b.size_)
		{
			r = +1;
		}
	}

	return r;
}

