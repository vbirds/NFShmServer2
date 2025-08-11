// -------------------------------------------------------------------------
//    @FileName         :    NFBuffer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFBuffer.h
 * @brief 字节流缓冲区封装类
 * 
 * 此文件提供了高效的字节流缓冲区实现，支持动态扩容、双向读写操作。
 * 主要用于网络数据缓冲、数据流处理等场景。
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include "NFPlatform.h"

/**
 * @brief 字节流缓冲区封装类
 * 
 * NFBuffer是一个高效的动态字节缓冲区实现，提供了类似队列的读写操作。
 * 支持数据的推入、弹出、窥视等操作，并能根据需要自动扩容。
 * 
 * 主要特性：
 * - 动态扩容：根据数据量自动调整缓冲区大小
 * - 双游标设计：独立的读写游标，支持并发读写
 * - 高效操作：支持批量数据操作和零拷贝窥视
 * - 内存管理：自动管理内存分配和释放
 * - 灵活访问：提供多种数据访问方式
 * 
 * 内存布局：
 * ```
 * +------------------------+------------------------+------------------------+
 * |    已读取的数据         |      可读取的数据        |      可写入的空间        |
 * +------------------------+------------------------+------------------------+
 * ^                        ^                        ^                        ^
 * 缓冲区起始                读游标位置                写游标位置               缓冲区结束
 * ```
 * 
 * 适用场景：
 * - 网络数据接收缓冲
 * - 协议解析中的数据缓存
 * - 数据流的暂存和转发
 * - 二进制数据的序列化处理
 * 
 * 使用方法：
 * @code
 * NFBuffer buffer;
 * 
 * // 写入数据
 * const char* data = "Hello World";
 * buffer.PushData(data, strlen(data));
 * 
 * // 读取数据
 * char readBuf[100];
 * size_t readSize = buffer.PopData(readBuf, sizeof(readBuf));
 * 
 * // 窥视数据（不移动读游标）
 * void* peekPtr;
 * size_t peekSize;
 * buffer.PeekData(peekPtr, peekSize);
 * 
 * // 获取缓冲区状态
 * size_t readable = buffer.ReadableSize();
 * size_t writable = buffer.WritableSize();
 * @endcode
 * 
 * @note 此类禁用拷贝构造和赋值操作，确保缓冲区的唯一性
 */
class _NFExport NFBuffer
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 创建一个空的缓冲区，初始化读写游标和内存指针。
	 */
	NFBuffer();

	/**
	 * @brief 析构函数
	 * 
	 * 释放缓冲区占用的内存资源。
	 */
	~NFBuffer();

private:
	/**
	 * @brief 禁用拷贝构造函数
	 * 防止缓冲区被意外拷贝，确保内存管理的正确性
	 */
	NFBuffer(const NFBuffer&);
	
	/**
	 * @brief 禁用赋值操作符
	 * 防止缓冲区被意外赋值，确保内存管理的正确性
	 */
	void operator=(const NFBuffer&);

public:
	/**
	 * @brief 将数据写入缓冲区
	 * 
	 * 将指定的数据写入缓冲区的末尾，如果空间不足会自动扩容。
	 * 写入后会自动调整写游标位置。
	 * 
	 * @param data 要写入的数据起始地址
	 * @param size 要写入的数据字节数
	 * @return std::size_t 实际写入的字节数（通常等于size）
	 * 
	 * @note 如果内存分配失败，返回值可能小于请求的size
	 */
	std::size_t PushData(const void* data, std::size_t size);
	
	/**
	 * @brief 将单个字符写入缓冲区
	 * 
	 * 将一个字符写入缓冲区末尾。
	 * 
	 * @param data 要写入的字符
	 * @return std::size_t 写入的字节数（成功时为1）
	 */
	std::size_t PushData(char data);

	/**
	 * @brief 调整缓冲区写游标
	 * 
	 * 手动向前移动写游标，通常在直接操作缓冲区内存后使用。
	 * 用于告知缓冲区有多少数据已经被写入。
	 * 
	 * @param bytes 要向前移动的字节数
	 * 
	 * @warning 确保移动的字节数不超过实际可写空间
	 */
	void Produce(std::size_t bytes);

	/**
	 * @brief 从缓冲区读取数据（深拷贝）
	 * 
	 * 从缓冲区的读位置复制数据到指定缓冲区，并调整读游标。
	 * 这是一个消费性操作，读取后的数据不再可用。
	 * 
	 * @param buf 接收数据的目标缓冲区起始地址
	 * @param size 目标缓冲区的大小（最大读取字节数）
	 * @return std::size_t 实际读取的字节数，可能小于size
	 * 
	 * @note 如果可读数据少于请求的size，只读取可用的数据
	 */
	std::size_t PopData(void* buf, std::size_t size);

	/**
	 * @brief 窥视缓冲区数据（浅拷贝）
	 * 
	 * 获取缓冲区中可读数据的直接指针，不调整读游标。
	 * 这是一个非消费性操作，数据仍然保留在缓冲区中。
	 * 
	 * @param buf [out] 接收数据指针的引用
	 * @param size [out] 接收可读数据大小的引用
	 * 
	 * @warning 返回的指针指向缓冲区内部内存，不要修改或释放
	 * @warning 在下次缓冲区操作后，指针可能失效
	 */
	void PeekData(void*& buf, std::size_t& size) const;

	/**
	 * @brief 调整缓冲区读游标
	 * 
	 * 手动向前移动读游标，通常在窥视数据并处理后使用。
	 * 用于标记指定数量的数据已经被消费。
	 * 
	 * @param bytes 要向前移动的字节数
	 * 
	 * @warning 确保移动的字节数不超过实际可读数据量
	 */
	void Consume(std::size_t bytes);

	/**
	 * @brief 获取缓冲区可读数据起始地址
	 * 
	 * 返回当前读游标位置的内存地址，可用于直接访问可读数据。
	 * 
	 * @return char* 可读数据的起始地址指针
	 * 
	 * @warning 返回的指针指向缓冲区内部内存，使用时要注意边界
	 */
	char* ReadAddr() const;

	/**
	 * @brief 获取缓冲区可写区域起始地址
	 * 
	 * 返回当前写游标位置的内存地址，可用于直接写入数据。
	 * 
	 * @return char* 可写区域的起始地址指针
	 * 
	 * @warning 返回的指针指向缓冲区内部内存，写入前要检查可用空间
	 */
	char* WriteAddr() const;

	/**
	 * @brief 检查缓冲区是否为空
	 * 
	 * 判断缓冲区中是否有可读数据。当读写游标位置相同时，
	 * 表示缓冲区为空。
	 * 
	 * @return bool 如果缓冲区为空返回true，否则返回false
	 * 
	 * @note 这是一个常量方法，不会修改缓冲区状态
	 */
	bool IsEmpty() const;

	/**
	 * @brief 获取缓冲区可读数据大小
	 * 
	 * 返回当前缓冲区中可读取的数据字节数。
	 * 这等于写游标位置减去读游标位置。
	 * 
	 * @return std::size_t 可读数据的字节数
	 * 
	 * @note 这是一个常量方法，不会修改缓冲区状态
	 */
	std::size_t ReadableSize() const;

	/**
	 * @brief 获取缓冲区可写空间大小
	 * 
	 * 返回当前缓冲区中可写入的空间字节数。
	 * 这等于缓冲区容量减去写游标位置。
	 * 
	 * @return std::size_t 可写空间的字节数
	 * 
	 * @note 如果空间不足，PushData操作会自动扩容
	 */
	std::size_t WritableSize() const;

	/**
	 * @brief 获取缓冲区总容量
	 * 
	 * 返回当前缓冲区分配的总内存大小（字节数）。
	 * 这包括已使用和未使用的内存空间。
	 * 
	 * @return std::size_t 缓冲区总容量的字节数
	 */
	std::size_t Capacity() const;

	/**
	 * @brief 尝试收缩缓冲区内存
	 * 
	 * 当缓冲区内存使用率较低时（低于高水位线），尝试释放多余的内存。
	 * 这个操作会保留数据，但可能会调整内存布局以减少内存占用。
	 * 
	 * @note 此操作不会清除数据，只是优化内存使用
	 * @note 是否执行收缩取决于当前的内存使用率和高水位设置
	 */
	void Shrink();

	/**
	 * @brief 清空缓冲区数据
	 * 
	 * 重置读写游标到初始位置，逻辑上清空所有数据。
	 * 不会释放已分配的内存，以便后续重用。
	 * 
	 * @note 清空后ReadableSize()返回0，WritableSize()返回Capacity()
	 * @note 这是一个快速操作，仅重置游标，不进行内存操作
	 */
	void Clear();

	/**
	 * @brief 交换两个缓冲区的内容
	 * 
	 * 高效地交换当前缓冲区与另一个缓冲区的所有状态，
	 * 包括数据、游标位置、容量等。
	 * 
	 * @param buf 要交换的目标缓冲区
	 * 
	 * @note 这是一个高效操作，只交换指针和状态，不拷贝数据
	 * @note 交换后两个缓冲区的内容完全互换
	 */
	void Swap(NFBuffer& buf);

	/**
	 * @brief 确保缓冲区有足够的写入空间
	 * 
	 * 检查并确保缓冲区有足够的空间来写入指定大小的数据。
	 * 如果空间不足，会自动扩容缓冲区。
	 * 
	 * @param size 需要确保的可写空间字节数
	 * 
	 * @note 扩容操作可能涉及内存重新分配和数据移动
	 * @note 建议在批量写入前调用此方法预分配空间
	 */
	void AssureSpace(std::size_t size);

	/**
	 * @brief 设置内存收缩的高水位百分比
	 * 
	 * 设置触发内存收缩操作的负载率阈值。当实际数据占用容量的
	 * 百分比低于此值时，Shrink()操作才会执行内存收缩。
	 * 
	 * @param percents 负载率百分比，有效范围[10, 100)
	 * 
	 * @note 默认值通常为75%，即数据占用低于75%容量时才收缩
	 * @note 较高的百分比可以减少内存收缩频率，提高性能
	 * @note 较低的百分比可以更积极地释放内存
	 */
	void SetHighWaterPercent(size_t percents);

	/**
	 * @brief 缓冲区大小上限常量
	 * 
	 * 定义了缓冲区允许扩展到的最大大小，防止无限制的内存增长。
	 */
	static const std::size_t kMaxBufferSize;

	/**
	 * @brief 缓冲区默认初始大小常量
	 * 
	 * 定义了新创建缓冲区的默认初始容量大小。
	 */
	static const std::size_t kDefaultSize;
private:
	/** @brief 读游标位置，指向下一个要读取的字节 */
	std::size_t _readPos;

	/** @brief 写游标位置，指向下一个要写入的字节 */
	std::size_t _writePos;

	/** @brief 缓冲区分配的总内存字节数 */
	std::size_t _capacity;

	/** @brief 指向实际内存缓冲区的指针 */
	char* _buffer;

	/** @brief 高水位百分比阈值，控制Shrink()行为 */
	size_t _highWaterPercent;

	/**
	 * @brief 重置缓冲区内存指针
	 * 
	 * 内部方法，用于重新设置缓冲区的内存指针。
	 * 主要在内存重新分配时使用。
	 * 
	 * @param ptr 新的内存指针，nullptr表示释放当前内存
	 */
	void ResetBuffer(void* ptr = nullptr);
};

