// -------------------------------------------------------------------------
//    @FileName         :    NFSimpleBuffer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFSimpleBuffer.h
 * @brief 简单字节缓冲区类
 * 
 * 此文件提供了一个轻量级的字节缓冲区实现，支持数据的存储、访问和管理。
 * 主要用于小数据块的临时存储、数据传输等场景。
 */

#pragma once

#include <string.h>
#include <assert.h>
#include <string>
#include "NFPlatform.h"

/**
 * @brief 简单字节缓冲区类
 * 
 * NFSimpleBuffer提供了一个简单的字节数据缓冲区实现。
 * 类似于数据切片（slice），但拥有数据的所有权，负责内存的分配和释放。
 * 
 * 主要特性：
 * - 内存管理：自动管理内存分配和释放
 * - RAII设计：构造时分配，析构时释放
 * - 深拷贝：拷贝数据而非引用，保证数据独立性
 * - 简单接口：提供基本的访问和查询方法
 * - 类型安全：支持二进制数据存储
 * 
 * 与NFBuffer的区别：
 * - NFSimpleBuffer更简单，只支持一次性数据存储
 * - NFBuffer支持动态读写操作，适合流式处理
 * - NFSimpleBuffer适合静态数据块的封装
 * 
 * 适用场景：
 * - 数据块的临时存储
 * - 网络消息的封装
 * - 文件内容的内存表示
 * - 二进制数据的传递
 * 
 * 使用方法：
 * @code
 * // 创建空缓冲区
 * NFSimpleBuffer buffer1;
 * 
 * // 从数据创建缓冲区
 * const char* data = "Hello World";
 * NFSimpleBuffer buffer2(data, strlen(data));
 * 
 * // 初始化指定大小的缓冲区
 * NFSimpleBuffer buffer3;
 * buffer3.Init(1024);  // 分配1KB空间
 * 
 * // 访问数据
 * if (!buffer2.empty()) {
 *     char* ptr = buffer2.data();
 *     size_t len = buffer2.size();
 *     std::string str = buffer2.ToString();
 * }
 * 
 * // 清空缓冲区
 * buffer2.clear();
 * @endcode
 * 
 * @note 此类不可拷贝，确保数据所有权的唯一性
 * @note 数据存储为原始字节，不自动添加字符串终止符
 */
class NFSimpleBuffer
{
public:
	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个空的缓冲区，不分配任何内存。
	 */
	NFSimpleBuffer() : data_(nullptr), size_(0)
	{
	}

	/**
	 * @brief 带数据的构造函数
	 * 
	 * 创建缓冲区并拷贝指定的数据。
	 * 
	 * @param d 源数据指针，可以为nullptr（当n为0时）
	 * @param n 数据长度（字节数）
	 * 
	 * @note 会分配新内存并拷贝数据，而不是引用原数据
	 */
	NFSimpleBuffer(const char* d, size_t n)
	{
		Init(d, n);
	}

	/**
	 * @brief 使用数据初始化缓冲区
	 * 
	 * 清除现有数据，然后拷贝新数据到缓冲区。
	 * 
	 * @param d 源数据指针，不能为nullptr（当n>0时）
	 * @param n 数据长度（字节数），0表示不分配内存
	 * 
	 * @note 会先清除现有数据再分配新内存
	 * @note 进行深拷贝，修改原数据不会影响缓冲区
	 */
	void Init(const char* d, size_t n)
	{
		clear();
		if (n > 0)
		{
			data_ = new char[n];
			size_ = n;
			memcpy(static_cast<void*>(data_), d, n);
		}
	}

	/**
	 * @brief 初始化指定大小的缓冲区
	 * 
	 * 清除现有数据，分配指定大小的内存空间，但不初始化内容。
	 * 
	 * @param n 要分配的字节数，0表示不分配内存
	 * 
	 * @note 新分配的内存内容是未定义的
	 * @note 适用于需要预分配空间后再填充数据的场景
	 */
	void Init(size_t n)
	{
		clear();
		if (n > 0)
		{
			data_ = new char[n];
			size_ = n;
		}
	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 自动释放分配的内存，确保无内存泄漏。
	 */
	virtual ~NFSimpleBuffer()
	{
		clear();
	}

	/**
	 * @brief 检查缓冲区是否为空
	 * 
	 * 判断缓冲区是否没有分配任何数据。
	 * 
	 * @return bool 缓冲区为空返回true，否则返回false
	 * 
	 * @note 这里的"空"指没有分配内存，而不是内容为空
	 */
	bool empty() const
	{
		return data_ == nullptr;
	}

	/**
	 * @brief 清空缓冲区
	 * 
	 * 释放分配的内存，将缓冲区重置为空状态。
	 * 
	 * @note 调用后data()返回nullptr，size()返回0
	 * @note 多次调用是安全的
	 */
	void clear()
	{
		if (data_)
		{
			delete[] data_;
			size_ = 0;
			data_ = nullptr;
		}
	}

	/**
	 * @brief 获取数据指针
	 * 
	 * 返回指向缓冲区数据起始位置的指针。
	 * 
	 * @return char* 数据指针，缓冲区为空时返回nullptr
	 * 
	 * @warning 返回的指针指向内部数据，不要在缓冲区生命周期外使用
	 * @warning 不要手动释放此指针
	 */
	char* data() const
	{
		return data_;
	}

	/**
	 * @brief 获取数据大小
	 * 
	 * 返回缓冲区中数据的字节数。
	 * 
	 * @return size_t 数据大小（字节数），空缓冲区返回0
	 */
	size_t size() const
	{
		return size_;
	}

	/**
	 * @brief 转换为字符串
	 * 
	 * 将缓冲区数据拷贝到std::string对象中。
	 * 
	 * @return std::string 包含缓冲区数据拷贝的字符串
	 * 
	 * @note 这是一个拷贝操作，修改返回的字符串不会影响缓冲区
	 * @note 如果数据包含'\0'字符，字符串会包含这些字符
	 * @note 空缓冲区返回空字符串
	 */
	std::string ToString() const
	{
		return std::string(data_, size_);
	}

private:
	/** @brief 数据指针 */
	char* data_;
	/** @brief 数据大小（字节数） */
	size_t size_;
};

