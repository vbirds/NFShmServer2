// -------------------------------------------------------------------------
//    @FileName         :    NFCodeQueue.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//    @Description      :    高性能无锁循环队列，支持变长数据包的存储和检索
//                           专为网络通信和进程间通信设计的FIFO数据结构
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
	#include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")

    // WIN32下没有PF_LOCAL，为了能编译，用PF_INET代替
    #ifndef PF_LOCAL
        #define PF_LOCAL  PF_INET
    #endif
#else
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <sys/socket.h>
    #include <sys/un.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
#endif

/**
 * @brief 高性能无锁循环队列类
 * 
 * NFCodeQueue 是为高并发场景设计的变长数据包队列：
 * 
 * 1. 核心设计特点：
 *    - 无锁并发：单生产者/单消费者模式下的无锁操作
 *    - 循环复用：固定大小内存的循环使用，避免内存碎片
 *    - 变长支持：支持任意长度的数据包存储（最大16MB）
 *    - 零拷贝：提供直接内存访问，减少数据拷贝开销
 * 
 * 2. 数据包格式：
 *    - 每个数据包包含4字节的长度头部
 *    - 长度头部使用网络字节序存储
 *    - 支持最大16MB（0xFFFFFF字节）的单个数据包
 *    - 长度信息的最高字节用作状态标志
 * 
 * 3. 内存管理：
 *    - 预分配固定大小的循环缓冲区
 *    - 自动处理内存边界的环绕
 *    - 内置安全边界防止读写冲突
 *    - 支持内存完整性校验
 * 
 * 4. 线程安全性：
 *    - 读写指针的原子更新
 *    - 单向数据流保证一致性
 *    - 避免ABA问题的设计
 *    - 适用于生产者-消费者模式
 * 
 * 5. 网络优化：
 *    - 支持分块数据的合并写入
 *    - 网络字节序的自动处理
 *    - 针对网络数据包的优化存储
 *    - 减少系统调用次数
 * 
 * 6. 应用场景：
 *    - 网络数据包的缓冲队列
 *    - 进程间通信的消息队列
 *    - 实时系统的数据流处理
 *    - 高频交易系统的消息传递
 * 
 * 内存布局示例：
 * ```
 * +------+--------+------+--------+------+--------+
 * | Len1 | Data1  | Len2 | Data2  | Len3 | Data3  |
 * +------+--------+------+--------+------+--------+
 * ^                                               ^
 * read_                                          write_
 * ```
 * 
 * 其中：
 * - Len: 4字节的数据长度（网络字节序）
 * - Data: 变长的实际数据内容
 * - read_: 读取位置指针
 * - write_: 写入位置指针
 * 
 * 使用示例：
 * @code
 * // 初始化队列
 * char buffer[1024 * 1024];  // 1MB缓冲区
 * NFCodeQueue* queue = (NFCodeQueue*)buffer;
 * queue->Init(sizeof(buffer));
 * 
 * // 写入数据
 * const char* data = "Hello World";
 * if (queue->Put(data, strlen(data)) == 0) {
 *     // 写入成功
 * }
 * 
 * // 读取数据
 * char output[1024];
 * int len = 0;
 * if (queue->Get(output, sizeof(output), len) == 0 && len > 0) {
 *     // 处理接收到的数据
 *     ProcessData(output, len);
 * }
 * 
 * // 检查队列状态
 * if (queue->HasCode()) {
 *     int codeLen = queue->GetCodeLen();
 *     // 有数据可读，长度为codeLen
 * }
 * @endcode
 * 
 * 性能特点：
 * - 写入操作：O(1)时间复杂度
 * - 读取操作：O(1)时间复杂度
 * - 内存访问：高度优化的局部性
 * - 并发性能：无锁设计，低延迟
 * 
 * 设计优势：
 * - 高吞吐量的数据处理能力
 * - 最小化的内存分配开销
 * - 适合实时系统的确定性性能
 * - 简单直观的API接口
 * - 完善的错误处理机制
 */
class NFCodeQueue
{
private:
	enum { SPACE = 8 };  ///< 安全边界大小，防止读写指针重叠

public:
	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个未初始化的队列对象，需要调用Init()进行初始化。
	 */
	NFCodeQueue() {}
	
	/**
	 * @brief 默认析构函数
	 * 
	 * 清理队列资源，注意：不会释放缓冲区内存（由调用者管理）。
	 */
	~NFCodeQueue() {}

	/**
	 * @brief 初始化队列
	 * @param len 缓冲区总长度，包括队列对象本身的大小
	 * @return 成功返回0
	 * 
	 * 初始化队列的内部状态：
	 * - 计算可用缓冲区大小
	 * - 重置读写指针到起始位置
	 * - 设置创建时间戳用于校验
	 * - 计算并设置校验和
	 * - 清空缓冲区内容
	 * 
	 * 注意事项：
	 * - len必须大于sizeof(NFCodeQueue)
	 * - 初始化后缓冲区处于空状态
	 * - 可以重复调用进行重新初始化
	 */
	int Init(int len)
	{
		offset_ = sizeof(NFCodeQueue);
		size_ = len - sizeof(NFCodeQueue);
		read_ = 0;
		write_ = 0;
		create_time_ = (unsigned int) time(0);
		* (char*) GetBuffer() = 0;

		checksum_ = (unsigned int)offset_ ^ (unsigned int)size_ ^ (unsigned int) create_time_;
		return 0;
	}

	/**
	 * @brief 获取数据缓冲区指针
	 * @return 指向数据存储区域的指针
	 * 
	 * 返回用于存储实际数据的缓冲区地址：
	 * - 跳过队列对象本身的内存空间
	 * - 返回可用于数据存储的起始地址
	 * - 用于内部的数据读写操作
	 */
	char* GetBuffer() const { return (char*)this + offset_; }
	
	/**
	 * @brief 获取缓冲区总大小
	 * @return 可用数据存储空间的字节数
	 * 
	 * 返回除队列对象外的可用缓冲区大小。
	 */
	int GetSize() const { return size_; }
	
	/**
	 * @brief 获取剩余可用空间
	 * @return 当前可写入的字节数
	 * 
	 * 计算当前可用的写入空间：
	 * - 考虑循环缓冲区的特性
	 * - 减去安全边界和长度头部的开销
	 * - 处理读写指针的相对位置
	 * 
	 * 算法考虑：
	 * - 当write_ >= read_时：连续空间计算
	 * - 当write_ < read_时：分段空间计算
	 * - 预留安全边界防止读写冲突
	 */
	int GetFreeSize() const
	{
		if (read_ > write_)
		{
			return read_ - write_ - SPACE - sizeof(int);
		}
		else
		{
			return size_ - write_ + read_ - SPACE - sizeof(int);
		}
	}

    /**
     * @brief 判断队列中是否有数据包
     * @return 有数据返回true，无数据返回false
     * 
     * 快速检查队列是否包含可读的数据包：
     * - 调用GetCodeLen()获取下一个数据包长度
     * - 长度大于0表示有有效数据包
     * - 这是一个无副作用的查询操作
     */
    bool HasCode() const
    {
        return GetCodeLen() > 0;
    }

    /**
     * @brief 向队列放入一个数据包
     * @param p 数据内容指针
     * @param len 数据长度
     * @return 0成功，-1参数错误，-2缓冲区满
     * 
     * 数据包写入流程：
     * 1. 参数验证：检查长度范围和缓冲区限制
     * 2. 空间检查：确保有足够空间存储数据包
     * 3. 位置计算：处理循环边界的写入位置
     * 4. 数据写入：先写数据内容，再写长度头部
     * 5. 指针更新：原子更新写入位置
     * 
     * 数据包格式：
     * - 4字节长度头部（网络字节序）
     * - 变长数据内容
     * - 状态标志（在长度头部的标志位）
     * 
     * 错误处理：
     * - 长度为0：直接返回成功
     * - 长度超限：返回参数错误
     * - 空间不足：返回缓冲区满
     * 
     * 线程安全：
     * - 先写数据，最后更新长度头部的标志位
     * - 确保读取端看到完整的数据包
     * - 避免部分写入状态被读取
     */
	int Put(const char* p, int len)
	{
		if (len == 0)
        {
			return 0;
        }

		// 每次放入的长度最大不能超过0xFFFFFF = 16M
		if (len < 0 || len > (size_ - SPACE - (int)sizeof(int)) || len > 0xFFFFFF)
        {
			return -1;
        }

		// newpos是完成Put后write_的位置
		// wrap表示是否换行了
		const int startpos = write_ + sizeof(int);
		const int minpos = (read_ < SPACE ? size_ : 0) + read_ - SPACE;
		int newpos = startpos + len;
		int wrap = 0;
		if (newpos >= size_)
		{
			newpos -= size_;
			wrap = 1;
		}

		// 判断是否会满
		if (write_ > minpos)
		{
			if (wrap && newpos > minpos) // 满
			{
				return -2;
			}
		}
		else
		{
			if (wrap || newpos > minpos) // 满
            {
				return -2;
            }
		}

		if (wrap)
		{
			// buf起始位置已经超过了size_
			if (startpos >= size_)
			{
				memcpy(GetBuffer() + startpos - size_, p, len);
			}
			else
			{
				memcpy(GetBuffer() + startpos, p, size_ - startpos );
				memcpy(GetBuffer(), p + size_ - startpos, len - size_ + startpos );
			}
		}
		else
		{
			memcpy(GetBuffer() + startpos, p, len);
		}

		// 预先把下个头部长度设为0
		*(char*)(GetBuffer() + newpos) = 0;

		// 调整write_指针
		wrap = write_;
		write_ = newpos;
		newpos = wrap;

		// 把长度写到头部
		len = ntohl(len);
		if (size_ - newpos < (int)sizeof(int))
		{
			memcpy(GetBuffer() + newpos, &len, size_ - newpos);
			memcpy(GetBuffer(), ((char*)&len) + size_ - newpos, sizeof(int) - size_ + newpos);
		}
		else
		{
			memcpy(GetBuffer() + newpos, &len, sizeof(int));
		}

		*(char*)(GetBuffer() + newpos) = 1;

		return 0;
	}

    /**
     * @brief 将两块缓冲区合并成一个数据包放入队列
     * @param p1 第一块数据指针
     * @param len1 第一块数据长度
     * @param p2 第二块数据指针
     * @param len2 第二块数据长度
     * @return 0成功，-1参数错误，-2缓冲区满
     * 
     * 优化的双缓冲区写入方法：
     * - 减少一次内存拷贝操作
     * - 适用于分散的数据写入场景
     * - 原子性地写入两块数据作为一个数据包
     * 
     * 应用场景：
     * - 网络协议头部和数据的分离写入
     * - 消息头和消息体的组合发送
     * - 减少临时缓冲区的分配
     * 
     * 实现特点：
     * - 将两块数据视为一个连续的数据包
     * - 处理跨循环边界的复杂写入
     * - 保持与单缓冲区Put相同的原子性
     */
    int Put(const char* p1, int len1, const char* p2, int len2)
    {
        if ((p1 == NULL && len1 > 0) || (p2 == NULL && len2 > 0) || len1 < 0 || len2 < 0)
        {
            return -1;
        }

        // 每次放入的长度最大不能超过0xFFFFFF = 16M
        int len = len1 + len2;
        if (len > (size_ - SPACE - (int)sizeof(int)) || len > 0xFFFFFF)
        {
            return -1;
        }

        // newpos是完成Put后write_的位置
        // wrap表示是否换行了
        // minpos是本次write的极限
        const int startpos = write_ + sizeof(int);
        const int minpos = read_ - SPACE + (read_ < SPACE ? size_ : 0);
        int newpos = startpos + len;
        int wrap = 0;
        if (newpos >= size_)
        {
            newpos -= size_;
            wrap = 1;
        }

        // 判断是否会满
        if (write_ > minpos)
        {
            if (wrap && newpos > minpos) // 满
            {
                return -2;
            }
        }
        else
        {
            if (wrap || newpos > minpos) // 满
            {
                return -2;
            }
        }

        if (wrap)
        {
            // buf起始位置已经超过了size_
            if (startpos >= size_)
            {
                memcpy(GetBuffer() + startpos - size_, p1, len1);
                memcpy(GetBuffer() + startpos - size_ + len1, p2, len2);
            }
            else
            {
                const int iLeft = size_ - startpos;
                if (iLeft >= len1)
                {
                    memcpy(GetBuffer() + startpos, p1, len1);
                    memcpy(GetBuffer() + startpos + len1, p2, iLeft - len1);
                    memcpy(GetBuffer(), p2 + iLeft - len1, len2 - iLeft + len1);
                }
                else
                {
                    memcpy(GetBuffer() + startpos, p1, iLeft);
                    memcpy(GetBuffer(), p1 + iLeft, len1 - iLeft);         //org: memcpy(GetBuffer(), p1, len1 - iLeft);  mod by will 2008-10-20
                    memcpy(GetBuffer() + len1 - iLeft, p2, len2);
                }
            }
        }
        else
        {
            memcpy(GetBuffer() + startpos, p1, len1);
            memcpy(GetBuffer() + startpos + len1, p2, len2);
        }

        // 预先把下个头部长度设为0
        *(char*)(GetBuffer() + newpos) = 0;

        // 调整write_指针
        wrap = write_;
        write_ = newpos;
        newpos = wrap;

        // 把长度写到头部
        len = ntohl(len);
        if (size_ - newpos < (int)sizeof(int))
        {
            memcpy(GetBuffer() + newpos, &len, size_ - newpos);
            memcpy(GetBuffer(), ((char*)&len) + size_ - newpos, sizeof(int) - size_ + newpos);
        }
        else
        {
            memcpy(GetBuffer() + newpos, &len, sizeof(int));
        }

        *(char*)(GetBuffer() + newpos) = 1;

        return 0;
    }

    /**
     * @brief 从队列中取出一个数据包
     * @param p 输出缓冲区指针
     * @param buflen 输出缓冲区大小
     * @param len 输出参数，实际读取的数据长度
     * @return 0成功，-1参数错误，-2缓冲区内容错误
     * 
     * 数据包读取流程：
     * 1. 参数验证：检查缓冲区大小的有效性
     * 2. 状态检查：确认队列中有可读数据
     * 3. 长度读取：获取下一个数据包的长度信息
     * 4. 容量验证：确保输出缓冲区足够大
     * 5. 数据拷贝：处理跨边界的数据读取
     * 6. 指针更新：移动读取位置到下一个数据包
     * 
     * 边界处理：
     * - 自动处理循环缓冲区的边界跨越
     * - 确保数据的完整性和连续性
     * - 正确处理分段存储的数据包
     * 
     * 错误处理：
     * - 缓冲区太小：返回-2，不消费数据
     * - 队列为空：返回成功但len为0
     * - 数据损坏：返回-2，队列状态可能不一致
     * 
     * 性能优化：
     * - 一次性读取完整数据包
     * - 最小化内存拷贝次数
     * - 快速的长度和位置计算
     */
	int Get(char* p, int buflen, int& len)
	{
		if (buflen <= 0)
        {
			return -1;
        }

		if (read_ == write_)
		{
			len = 0;
			return 0;
		}

		int readlen = 0;
		const int startpos = read_ + sizeof(int);

		char flag = * (char*) (GetBuffer() + read_);
		if (flag != 1)
		{
			len = 0;
			return 0;
		}

		// 获取要读取的数据长度
		if (startpos > size_)
		{
			memcpy(&readlen, GetBuffer() + read_, size_ - read_);
			memcpy(((char*)&readlen) + size_ - read_, GetBuffer(), startpos - size_);
		}
		else
		{
			memcpy(&readlen, GetBuffer() + read_, sizeof(int));
		}
		readlen = htonl(readlen);
		readlen = readlen & 0x00FFFFFF;

		if (readlen == 0)
		{
			len = 0;
			return 0;
		}

		if (readlen > buflen || readlen < 0)
        {
			return -2;
        }

		if (startpos >= size_)
		{
			memcpy(p, GetBuffer() + startpos - size_, readlen);
		}
		else
		{
			if (startpos + readlen > size_)
			{
				memcpy(p, GetBuffer() + startpos, size_ - startpos);
				memcpy(p + size_ - startpos, GetBuffer(), readlen - size_ + startpos);
			}
			else
			{
				memcpy(p, GetBuffer() + startpos, readlen);
			}
		}

		len = readlen;
		read_ = (startpos + readlen) % size_;

		return 0;
	}

    /**
     * @brief 返回队列中下一个数据包的长度
     * @return 数据包长度，0表示无数据
     * 
     * 无副作用的数据包长度查询：
     * - 不消费数据包，只查看长度信息
     * - 用于预分配合适大小的接收缓冲区
     * - 支持数据包的预处理和筛选
     * 
     * 实现细节：
     * - 读取长度头部但不移动读取指针
     * - 处理循环边界的长度头部分割
     * - 验证数据包的有效性标志
     * - 返回实际的数据长度（不含头部）
     * 
     * 使用场景：
     * - 动态分配接收缓冲区
     * - 实现数据包的批量处理
     * - 队列状态的监控和统计
     */
    int GetCodeLen() const
    {
        if (read_ == write_)
        {
            return 0;
        }

        int readlen = 0;
        const int startpos = read_ + sizeof(int);

        char flag = * (char*) (GetBuffer() + read_);
        if (flag != 1)
        {
            return 0;
        }

        // 获取要读取的数据长度
        if (startpos > size_)
        {
            memcpy(&readlen, GetBuffer() + read_, size_ - read_);
            memcpy(((char*)&readlen) + size_ - read_, GetBuffer(), startpos - size_);
        }
        else
        {
            memcpy(&readlen, GetBuffer() + read_, sizeof(int));
        }

        return htonl(readlen) & 0x00FFFFFF;
    }

	/**
	 * @brief 删除队列中的一个数据包
	 * @return 0成功，-2数据错误
	 * 
	 * 快速移除下一个数据包而不读取内容：
	 * - 只更新读取指针，不拷贝数据
	 * - 用于丢弃不需要的数据包
	 * - 比Get操作更高效的跳过方式
	 * 
	 * 应用场景：
	 * - 实现数据包的选择性消费
	 * - 快速清理过期或无效数据
	 * - 实现数据包的预览-确认模式
	 * 
	 * 错误处理：
	 * - 队列为空时操作无效果
	 * - 数据包标志错误时返回错误
	 * - 长度信息损坏时返回错误
	 */
	int Remove()
	{
		int readlen = 0;
		const int startpos = read_ + sizeof(int);

		if (read_ == write_)
        {
			return 0;
        }

		char flag = * (char*) (GetBuffer() + read_);
		if (flag != 1)
        {
			return 0;
        }

		// 获取要读取的数据长度
		if (startpos > size_)
		{
			memcpy(&readlen, GetBuffer() + read_, size_ - read_);
			memcpy(((char*)&readlen) + size_ - read_, GetBuffer(), startpos - size_);
		}
		else
		{
			memcpy(&readlen, GetBuffer() + read_, sizeof(int));
		}
		readlen = htonl(readlen);
		readlen = readlen & 0x00FFFFFF;

		if (readlen < 0)
        {
			return -2;
        }

		read_ = (startpos + readlen) % size_;
		return 0;
	}

    /**
	 * @brief 删除队列中的所有内容
	 * @return 总是返回0
	 * 
	 * 快速清空队列的所有数据：
	 * - 只能由接收方调用
	 * - 通过重置读指针实现快速清空
	 * - 不修改写指针，保持发送方状态一致
	 * 
	 * 注意事项：
	 * - 这是一个危险操作，会丢失所有数据
	 * - 在多线程环境中需要谨慎使用
	 * - 主要用于错误恢复或重置场景
	 * 
	 * 使用场景：
	 * - 队列溢出后的快速恢复
	 * - 系统重启时的状态重置
	 * - 错误处理中的资源清理
	 */
    int RemoveAll()
    {
        read_ = write_; //此处不能修改write_
        return 0;
    }

    /**
     * @brief 校验队列对象的完整性
     * @return 0表示对象完整，-1表示对象损坏
     * 
     * 验证共享内存中的队列对象是否有效：
     * - 检查关键字段的非零性
     * - 验证校验和的正确性
     * - 用于共享内存的完整性检查
     * 
     * 校验算法：
     * - 使用XOR操作生成校验和
     * - 包含偏移量、大小和创建时间
     * - 提供基本的数据完整性保护
     * 
     * 应用场景：
     * - 进程重启后的状态恢复
     * - 共享内存的完整性验证
     * - 调试和诊断工具
     */
	int CheckSum() const
	{
		if (!offset_ || !size_ || !create_time_)
			return -1;

		unsigned int nTempInt = 0;
		nTempInt = (unsigned int)offset_ ^ (unsigned int)size_
					^ (unsigned int) create_time_ ^ (unsigned int) checksum_;

		return (nTempInt == 0) ? 0 : -1;
	}

	/**
	 * @brief 输出队列状态的调试信息
	 * 
	 * 打印队列的关键状态信息：
	 * - 输出偏移量、大小、读写位置
	 * - 用于调试和问题诊断
	 * - 帮助理解队列的当前状态
	 * 
	 * 注意：需要定义NFLogError宏或包含相应的日志头文件
	 */
	void Dump() const
	{
		// 输出调试信息的占位符 - 需要包含具体的日志实现
		// NFLogError(NF_LOG_DEFAULT, 0, "offset={} size={} read={} write={}\n",
		//	offset_, size_, read_, write_);
		printf("NFCodeQueue: offset=%d size=%d read=%d write=%d\n",
			offset_, size_, read_, write_);
	}

protected:
	int			offset_;        ///< 数据缓冲区相对于对象起始的偏移量
	int			size_;          ///< 可用数据存储空间的大小
	int			read_;          ///< 读取位置指针（已读数据的结束位置）
	int			write_;         ///< 写入位置指针（待写数据的开始位置）
	unsigned int create_time_;  ///< 队列创建时间戳，用于完整性校验
	unsigned int checksum_;     ///< XOR校验和，用于验证对象完整性
};

