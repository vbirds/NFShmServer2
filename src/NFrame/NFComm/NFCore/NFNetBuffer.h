// -------------------------------------------------------------------------
//    @FileName         :    NFNetBuffer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFNetBuffer.h
 * @brief 网络缓冲区类
 * 
 * 此文件定义了一个高效的网络数据缓冲区类，用于网络I/O操作中的数据缓存。
 * 该缓冲区支持动态扩容、预留空间、零拷贝读写等特性，适用于高性能网络编程。
 */

#pragma once

#include "NFPlatform.h"
#include "NFSlice.hpp"
#include <algorithm>

#if NF_PLATFORM == NF_PLATFORM_WIN

#include <ws2tcpip.h>
#include <WinSock2.h>
#include <io.h>
#include <ws2ipdef.h>

#else

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/uio.h>

#endif

/**
 * @brief 网络缓冲区类
 * 
 * 高效的网络数据缓冲区实现，主要特性包括：
 * - 动态扩容：根据需要自动扩展缓冲区大小
 * - 预留空间：在缓冲区前部预留空间，便于添加协议头
 * - 零拷贝：支持直接读写，减少内存拷贝
 * - 线性化：提供连续的内存空间用于高效的数据处理
 * - 兼容性：支持标准的网络I/O接口
 * 
 * 内存布局：
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 * 
 * 使用方法：
 * @code
 * NFNetBuffer buffer;
 * buffer.Append("Hello", 5);  // 写入数据
 * std::string data = buffer.NextString(5);  // 读取数据
 * @endcode
 */
class _NFExport NFNetBuffer
{
public:
	/** @brief 默认的预留前置空间大小 */
	static const size_t kCheapPrependSize;
	/** @brief 默认的初始缓冲区大小 */
	static const size_t kInitialSize;

	/**
	 * @brief 构造函数
	 * 
	 * 创建一个新的网络缓冲区实例，并分配指定大小的内存空间。
	 * 
	 * @param initial_size 初始缓冲区大小，默认为kInitialSize
	 * @param reserved_prepend_size 预留的前置空间大小，默认为kCheapPrependSize
	 */
	explicit NFNetBuffer(size_t initial_size = kInitialSize, size_t reserved_prepend_size = kCheapPrependSize)
		: capacity_(reserved_prepend_size + initial_size)
		  , read_index_(reserved_prepend_size)
		  , write_index_(reserved_prepend_size)
		  , reserved_prepend_size_(reserved_prepend_size)
	{
		buffer_ = new char[capacity_];
		assert(length() == 0);
		assert(WritableBytes() == initial_size);
		assert(PrependableBytes() == reserved_prepend_size);
	}

	/**
	 * @brief 析构函数
	 * 释放缓冲区内存
	 */
	~NFNetBuffer()
	{
		delete[] buffer_;
		buffer_ = nullptr;
		capacity_ = 0;
	}

	/**
	 * @brief 交换两个缓冲区的内容
	 * 
	 * 高效地交换两个NFNetBuffer对象的所有成员变量，
	 * 包括缓冲区指针、容量、读写索引等。
	 * 
	 * @param rhs 要交换的另一个缓冲区对象
	 */
	void Swap(NFNetBuffer& rhs)
	{
		std::swap(buffer_, rhs.buffer_);
		std::swap(capacity_, rhs.capacity_);
		std::swap(read_index_, rhs.read_index_);
		std::swap(write_index_, rhs.write_index_);
		std::swap(reserved_prepend_size_, rhs.reserved_prepend_size_);
	}

	/**
	 * @brief 跳过指定长度的数据
	 * 
	 * 向前移动读索引，跳过指定长度的数据。如果跳过的长度大于等于
	 * 当前可读数据长度，则重置整个缓冲区。
	 * 
	 * @param len 要跳过的字节数
	 */
	void Skip(size_t len)
	{
		if (len < length())
		{
			read_index_ += len;
		}
		else
		{
			Reset();
		}
	}

	/**
	 * @brief 检索并跳过指定长度的数据
	 * 
	 * 这是Skip方法的别名，功能完全相同。
	 * 向前移动读索引，跳过指定长度的数据。
	 * 
	 * @param len 要检索跳过的字节数
	 */
	void Retrieve(size_t len)
	{
		Skip(len);
	}

	/**
	 * @brief 截断缓冲区数据
	 * 
	 * 丢弃除前n个未读字节之外的所有数据，但继续使用相同的已分配存储。
	 * 如果n大于缓冲区的长度，则不执行任何操作。
	 * 
	 * @param n 要保留的字节数，0表示清空所有数据
	 */
	void Truncate(size_t n)
	{
		if (n == 0)
		{
			read_index_ = reserved_prepend_size_;
			write_index_ = reserved_prepend_size_;
		}
		else if (write_index_ > read_index_ + n)
		{
			write_index_ = read_index_ + n;
		}
	}

	/**
	 * @brief 重置缓冲区
	 * 
	 * 将缓冲区重置为空，但保留底层存储以供将来写入。
	 * 重置与Truncate(0)相同。
	 */
	void Reset()
	{
		Truncate(0);
	}

	/**
	 * @brief 增加缓冲区容量
	 * 
	 * 将容器容量增加到大于或等于len的值。如果len大于当前容量，
	 * 则分配新的存储空间，否则不执行任何操作。
	 * 
	 * @param len 所需的容量
	 */
	void Reserve(size_t len)
	{
		if (capacity_ >= len + reserved_prepend_size_)
		{
			return;
		}

		// TODO add the implementation logic here
		grow(len + reserved_prepend_size_);
	}

	/**
	 * @brief 确保有足够的可写空间
	 * 
	 * 确保缓冲区有足够的可写空间来追加长度为len的数据。
	 * 如果当前可写空间不足，则扩展缓冲区。
	 * 
	 * @param len 要追加的数据长度
	 */
	void EnsureWritableBytes(size_t len)
	{
		if (WritableBytes() < len)
		{
			grow(len);
		}

		assert(WritableBytes() >= len);
	}

	/**
	 * @brief 将缓冲区转换为文本
	 * 
	 * 在缓冲区末尾追加一个'\0'字符，将底层数据转换为C风格字符串文本。
	 * 不会改变缓冲区长度。
	 */
	void ToText()
	{
		AppendInt8('\0');
		UnwriteBytes(1);
	}

	// TODO XXX Little-Endian/Big-Endian problem.
#define evppbswap_64(x)                          \
    ((((x) & 0xff00000000000000ull) >> 56)       \
     | (((x) & 0x00ff000000000000ull) >> 40)     \
     | (((x) & 0x0000ff0000000000ull) >> 24)     \
     | (((x) & 0x000000ff00000000ull) >> 8)      \
     | (((x) & 0x00000000ff000000ull) << 8)      \
     | (((x) & 0x0000000000ff0000ull) << 24)     \
     | (((x) & 0x000000000000ff00ull) << 40)     \
     | (((x) & 0x00000000000000ffull) << 56))

	// Write
public:
	/**
	 * @brief 写入数据
	 * 
	 * 将指定长度的数据写入缓冲区。
	 * 
	 * @param d 指向数据的指针
	 * @param len 数据长度
	 */
	void Write(const void* /*restrict*/ d, size_t len)
	{
		EnsureWritableBytes(len);
		memcpy(WriteBegin(), d, len);
		assert(write_index_ + len <= capacity_);
		write_index_ += len;
	}

	/**
	 * @brief 追加数据
	 * 
	 * 将NFSlice对象中的数据追加到缓冲区。
	 * 
	 * @param str NFSlice对象
	 */
	void Append(const NFSlice& str)
	{
		Write(str.data(), str.size());
	}

	/**
	 * @brief 追加数据
	 * 
	 * 将指定长度的字符串数据追加到缓冲区。
	 * 
	 * @param d 指向字符串数据的指针
	 * @param len 字符串长度
	 */
	void Append(const char* /*restrict*/ d, size_t len)
	{
		Write(d, len);
	}

	/**
	 * @brief 追加数据
	 * 
	 * 将指定长度的数据追加到缓冲区。
	 * 
	 * @param d 指向数据的指针
	 * @param len 数据长度
	 */
	void Append(const void* /*restrict*/ d, size_t len)
	{
		Write(d, len);
	}

	// Append int64_t/int32_t/int16_t with network endian
	/**
	 * @brief 追加64位整数
	 * 
	 * 将64位整数以网络字节序（大端）追加到缓冲区。
	 * 
	 * @param x 要追加的64位整数
	 */
	void AppendInt64(int64_t x)
	{
		int64_t be = evppbswap_64(x);
		Write(&be, sizeof be);
	}

	/**
	 * @brief 追加32位整数
	 * 
	 * 将32位整数以网络字节序（大端）追加到缓冲区。
	 * 
	 * @param x 要追加的32位整数
	 */
	void AppendInt32(int32_t x)
	{
		int32_t be32 = htonl(x);
		Write(&be32, sizeof be32);
	}

	/**
	 * @brief 追加16位整数
	 * 
	 * 将16位整数以网络字节序（大端）追加到缓冲区。
	 * 
	 * @param x 要追加的16位整数
	 */
	void AppendInt16(int16_t x)
	{
		int16_t be16 = htons(x);
		Write(&be16, sizeof be16);
	}

	/**
	 * @brief 追加8位整数
	 * 
	 * 将8位整数追加到缓冲区。
	 * 
	 * @param x 要追加的8位整数
	 */
	void AppendInt8(int8_t x)
	{
		Write(&x, sizeof x);
	}

	// Prepend int64_t/int32_t/int16_t with network endian
	/**
	 * @brief 预先追加64位整数
	 * 
	 * 将64位整数以网络字节序（大端）预先追加到缓冲区。
	 * 
	 * @param x 要预先追加的64位整数
	 */
	void PrependInt64(int64_t x)
	{
		int64_t be = evppbswap_64(x);
		Prepend(&be, sizeof be);
	}

	/**
	 * @brief 预先追加32位整数
	 * 
	 * 将32位整数以网络字节序（大端）预先追加到缓冲区。
	 * 
	 * @param x 要预先追加的32位整数
	 */
	void PrependInt32(int32_t x)
	{
		int32_t be32 = htonl(x);
		Prepend(&be32, sizeof be32);
	}

	/**
	 * @brief 预先追加16位整数
	 * 
	 * 将16位整数以网络字节序（大端）预先追加到缓冲区。
	 * 
	 * @param x 要预先追加的16位整数
	 */
	void PrependInt16(int16_t x)
	{
		int16_t be16 = htons(x);
		Prepend(&be16, sizeof be16);
	}

	/**
	 * @brief 预先追加8位整数
	 * 
	 * 将8位整数预先追加到缓冲区。
	 * 
	 * @param x 要预先追加的8位整数
	 */
	void PrependInt8(int8_t x)
	{
		Prepend(&x, sizeof x);
	}

	// Insert content, specified by the parameter, into the front of reading index
	/**
	 * @brief 在读索引前插入内容
	 * 
	 * 将指定长度的数据插入到读索引之前。
	 * 
	 * @param d 指向数据的指针
	 * @param len 数据长度
	 */
	void Prepend(const void* /*restrict*/ d, size_t len)
	{
		assert(len <= PrependableBytes());
		read_index_ -= len;
		const char* p = static_cast<const char*>(d);
		memcpy(begin() + read_index_, p, len);
	}

	/**
	 * @brief 取消写入指定字节数
	 * 
	 * 向前移动写索引，丢弃最后n个字节。
	 * 
	 * @param n 要取消写入的字节数
	 */
	void UnwriteBytes(size_t n)
	{
		assert(n <= length());
		write_index_ -= n;
	}

	/**
	 * @brief 写入指定字节数
	 * 
	 * 向前移动写索引，写入指定字节数。
	 * 
	 * @param n 要写入的字节数
	 */
	void WriteBytes(size_t n)
	{
		assert(n <= WritableBytes());
		write_index_ += n;
	}

	//Read
public:
	// Peek int64_t/int32_t/int16_t/int8_t with network endian
	/**
	 * @brief 读取64位整数
	 * 
	 * 读取并返回缓冲区中的下一个64位整数。
	 * 
	 * @return 读取到的64位整数
	 */
	int64_t ReadInt64()
	{
		int64_t result = PeekInt64();
		Skip(sizeof result);
		return result;
	}

	/**
	 * @brief 读取32位整数
	 * 
	 * 读取并返回缓冲区中的下一个32位整数。
	 * 
	 * @return 读取到的32位整数
	 */
	int32_t ReadInt32()
	{
		int32_t result = PeekInt32();
		Skip(sizeof result);
		return result;
	}

	/**
	 * @brief 读取16位整数
	 * 
	 * 读取并返回缓冲区中的下一个16位整数。
	 * 
	 * @return 读取到的16位整数
	 */
	int16_t ReadInt16()
	{
		int16_t result = PeekInt16();
		Skip(sizeof result);
		return result;
	}

	/**
	 * @brief 读取8位整数
	 * 
	 * 读取并返回缓冲区中的下一个8位整数。
	 * 
	 * @return 读取到的8位整数
	 */
	int8_t ReadInt8()
	{
		int8_t result = PeekInt8();
		Skip(sizeof result);
		return result;
	}

	/**
	 * @brief 转换为NFSlice
	 * 
	 * 将缓冲区转换为NFSlice对象，包含未读取的数据。
	 * 
	 * @return NFSlice对象
	 */
	NFSlice ToSlice() const
	{
		return NFSlice(data(), length());
	}

	/**
	 * @brief 转换为字符串
	 * 
	 * 将缓冲区中的未读数据转换为字符串。
	 * 
	 * @return 字符串
	 */
	std::string ToString() const
	{
		return std::string(data(), length());
	}

	/**
	 * @brief 收缩缓冲区
	 * 
	 * 将当前缓冲区收缩到指定大小，并保留指定大小的预留空间。
	 * 
	 * @param reserve 预留空间大小
	 */
	void Shrink(size_t reserve)
	{
		NFNetBuffer other(length() + reserve);
		other.Append(ToSlice());
		Swap(other);
	}

	// Next returns a slice containing the next n bytes from the buffer,
	// advancing the buffer as if the bytes had been returned by Read.
	// If there are fewer than n bytes in the buffer, Next returns the entire buffer.
	// The slice is only valid until the next call to a read or write method.
	/**
	 * @brief 获取下一个数据切片
	 * 
	 * 从缓冲区中获取下一个长度为len的数据切片，并向前移动读索引。
	 * 如果缓冲区中的数据不足len，则返回整个缓冲区。
	 * 切片在下次调用读写方法之前有效。
	 * 
	 * @param len 要获取的数据长度
	 * @return 数据切片
	 */
	NFSlice Next(size_t len)
	{
		if (len < length())
		{
			NFSlice result(data(), len);
			read_index_ += len;
			return result;
		}

		return NextAll();
	}

	// NextAll returns a slice containing all the unread portion of the buffer,
	// advancing the buffer as if the bytes had been returned by Read.
	/**
	 * @brief 获取所有未读数据切片
	 * 
	 * 从缓冲区中获取所有未读数据，并重置缓冲区。
	 * 
	 * @return 数据切片
	 */
	NFSlice NextAll()
	{
		NFSlice result(data(), length());
		Reset();
		return result;
	}

	/**
	 * @brief 读取下一个字符串
	 * 
	 * 从缓冲区中读取下一个长度为len的字符串。
	 * 
	 * @param len 要读取的字符串长度
	 * @return 读取到的字符串
	 */
	std::string NextString(size_t len)
	{
		NFSlice s = Next(len);
		return std::string(s.data(), s.size());
	}

	/**
	 * @brief 读取所有未读字符串
	 * 
	 * 从缓冲区中读取所有未读数据，并转换为字符串。
	 * 
	 * @return 字符串
	 */
	std::string NextAllString()
	{
		NFSlice s = NextAll();
		return std::string(s.data(), s.size());
	}

	// ReadByte reads and returns the next byte from the buffer.
	// If no byte is available, it returns '\0'.
	/**
	 * @brief 读取下一个字节
	 * 
	 * 从缓冲区中读取并返回下一个字节。
	 * 如果缓冲区中没有字节，则返回'\0'。
	 * 
	 * @return 读取到的字节
	 */
	char ReadByte()
	{
		assert(length() >= 1);

		if (length() == 0)
		{
			return '\0';
		}

		return buffer_[read_index_++];
	}

	// UnreadBytes unreads the last n bytes returned
	// by the most recent read operation.
	/**
	 * @brief 取消读取最后n个字节
	 * 
	 * 取消读取最近一次读取操作返回的最后n个字节。
	 * 
	 * @param n 要取消读取的字节数
	 */
	void UnreadBytes(size_t n)
	{
		assert(n < read_index_);
		read_index_ -= n;
	}

	// Peek
public:
	// Peek int64_t/int32_t/int16_t/int8_t with network endian

	/**
	 * @brief 预览64位整数
	 * 
	 * 预览缓冲区中的下一个64位整数。
	 * 
	 * @return 预览到的64位整数
	 */
	int64_t PeekInt64() const
	{
		assert(length() >= sizeof(int64_t));
		int64_t be64 = 0;
		::memcpy(&be64, data(), sizeof be64);
		return evppbswap_64(be64);
	}

	/**
	 * @brief 预览32位整数
	 * 
	 * 预览缓冲区中的下一个32位整数。
	 * 
	 * @return 预览到的32位整数
	 */
	int32_t PeekInt32() const
	{
		assert(length() >= sizeof(int32_t));
		int32_t be32 = 0;
		::memcpy(&be32, data(), sizeof be32);
		return ntohl(be32);
	}

	/**
	 * @brief 预览16位整数
	 * 
	 * 预览缓冲区中的下一个16位整数。
	 * 
	 * @return 预览到的16位整数
	 */
	int16_t PeekInt16() const
	{
		assert(length() >= sizeof(int16_t));
		int16_t be16 = 0;
		::memcpy(&be16, data(), sizeof be16);
		return ntohs(be16);
	}

	/**
	 * @brief 预览8位整数
	 * 
	 * 预览缓冲区中的下一个8位整数。
	 * 
	 * @return 预览到的8位整数
	 */
	int8_t PeekInt8() const
	{
		assert(length() >= sizeof(int8_t));
		int8_t x = *data();
		return x;
	}

public:
	// data returns a pointer of length Buffer.length() holding the unread portion of the buffer.
	// The data is valid for use only until the next buffer modification (that is,
	// only until the next call to a method like Read, Write, Reset, or Truncate).
	// The data aliases the buffer content at least until the next buffer modification,
	// so immediate changes to the slice will affect the result of future reads.
	/**
	 * @brief 获取未读数据指针
	 * 
	 * 获取指向缓冲区中未读数据的指针。
	 * 该数据在下次缓冲区修改之前有效（例如，调用Read、Write、Reset或Truncate）。
	 * 该数据至少在下次缓冲区修改之前别名缓冲区内容，因此对切片的立即更改将影响未来的读取结果。
	 * 
	 * @return 未读数据指针
	 */
	const char* data() const
	{
		return buffer_ + read_index_;
	}

	/**
	 * @brief 获取可写数据起始指针
	 * 
	 * 获取指向缓冲区可写数据起始位置的指针。
	 * 
	 * @return 可写数据起始指针
	 */
	char* WriteBegin()
	{
		return begin() + write_index_;
	}

	/**
	 * @brief 获取可写数据起始指针（const）
	 * 
	 * 获取指向缓冲区可写数据起始位置的指针（const）。
	 * 
	 * @return 可写数据起始指针（const）
	 */
	const char* WriteBegin() const
	{
		return begin() + write_index_;
	}

	// length returns the number of bytes of the unread portion of the buffer
	/**
	 * @brief 获取未读数据长度
	 * 
	 * 获取缓冲区中未读数据的长度。
	 * 
	 * @return 未读数据长度
	 */
	size_t length() const
	{
		assert(write_index_ >= read_index_);
		return write_index_ - read_index_;
	}

	// size returns the number of bytes of the unread portion of the buffer.
	// It is the same as length().
	/**
	 * @brief 获取未读数据大小
	 * 
	 * 获取缓冲区中未读数据的大小。
	 * 它与length()相同。
	 * 
	 * @return 未读数据大小
	 */
	size_t size() const
	{
		return length();
	}

	// capacity returns the capacity of the buffer's underlying byte slice, that is, the
	// total space allocated for the buffer's data.
	/**
	 * @brief 获取缓冲区容量
	 * 
	 * 获取缓冲区底层字节切片的容量，即分配给缓冲区数据的总量。
	 * 
	 * @return 缓冲区容量
	 */
	size_t capacity() const
	{
		return capacity_;
	}

	/**
	 * @brief 获取可写数据长度
	 * 
	 * 获取缓冲区中可写数据的长度。
	 * 
	 * @return 可写数据长度
	 */
	size_t WritableBytes() const
	{
		assert(capacity_ >= write_index_);
		return capacity_ - write_index_;
	}

	/**
	 * @brief 获取预留空间大小
	 * 
	 * 获取缓冲区中预留空间的大小。
	 * 
	 * @return 预留空间大小
	 */
	size_t PrependableBytes() const
	{
		return read_index_;
	}

	// Helpers
public:
	/**
	 * @brief 查找CRLF
	 * 
	 * 在缓冲区中查找"\r\n"（CRLF）。
	 * 
	 * @return 找到的CRLF位置，如果未找到则返回nullptr
	 */
	const char* FindCRLF() const
	{
		const char* crlf = std::search(data(), WriteBegin(), kCRLF, kCRLF + 2);
		return crlf == WriteBegin() ? nullptr : crlf;
	}

	/**
	 * @brief 查找CRLF
	 * 
	 * 在缓冲区中从指定位置开始查找"\r\n"（CRLF）。
	 * 
	 * @param start 开始查找的位置
	 * @return 找到的CRLF位置，如果未找到则返回nullptr
	 */
	const char* FindCRLF(const char* start) const
	{
		assert(data() <= start);
		assert(start <= WriteBegin());
		const char* crlf = std::search(start, WriteBegin(), kCRLF, kCRLF + 2);
		return crlf == WriteBegin() ? nullptr : crlf;
	}

	/**
	 * @brief 查找EOL
	 * 
	 * 在缓冲区中查找'\n'（EOL）。
	 * 
	 * @return 找到的EOL位置，如果未找到则返回nullptr
	 */
	const char* FindEOL() const
	{
		const void* eol = memchr(data(), '\n', length());
		return static_cast<const char*>(eol);
	}

	/**
	 * @brief 查找EOL
	 * 
	 * 在缓冲区中从指定位置开始查找'\n'（EOL）。
	 * 
	 * @param start 开始查找的位置
	 * @return 找到的EOL位置，如果未找到则返回nullptr
	 */
	const char* FindEOL(const char* start) const
	{
		assert(data() <= start);
		assert(start <= WriteBegin());
		const void* eol = memchr(start, '\n', WriteBegin() - start);
		return static_cast<const char*>(eol);
	}

private:

	char* begin()
	{
		return buffer_;
	}

	const char* begin() const
	{
		return buffer_;
	}

	void grow(size_t len)
	{
		if (WritableBytes() + PrependableBytes() < len + reserved_prepend_size_)
		{
			//grow the capacity
			size_t n = (capacity_ << 1) + len;
			size_t m = length();
			char* d = new char[n];
			memcpy(d + reserved_prepend_size_, begin() + read_index_, m);
			write_index_ = m + reserved_prepend_size_;
			read_index_ = reserved_prepend_size_;
			capacity_ = n;
			delete[] buffer_;
			buffer_ = d;
		}
		else
		{
			// move readable data to the front, make space inside buffer
			assert(reserved_prepend_size_ < read_index_);
			size_t readable = length();
			memmove(begin() + reserved_prepend_size_, begin() + read_index_, length());
			read_index_ = reserved_prepend_size_;
			write_index_ = read_index_ + readable;
			assert(readable == length());
			assert(WritableBytes() >= len);
		}
	}

private:
	char* buffer_;
	size_t capacity_;
	size_t read_index_;
	size_t write_index_;
	size_t reserved_prepend_size_;
	static const char kCRLF[];
};

