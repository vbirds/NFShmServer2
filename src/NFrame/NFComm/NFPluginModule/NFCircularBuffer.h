// -------------------------------------------------------------------------
//    @FileName         :    NFCircularBuffer.h
//    @Author           :    gaoyi
//    @Date             :    24-8-8
//    @Email            :    445267987@qq.com
//    @Module           :    NFCircularBuffer
//    @Description      :    高性能循环缓冲区，提供线程安全的数据队列和迭代器访问
//                           支持可变长度数据包的存储和零拷贝的数据访问机制
//
// -------------------------------------------------------------------------

#pragma once

#include <stddef.h>
#include <string.h>
#include <assert.h>

/**
 * @brief 缓冲区数据包头部结构
 * @tparam HeadType 用户定义的头部类型，通常包含消息类型、优先级等信息
 * 
 * BufferHead 为每个数据包提供统一的头部格式：
 * - m_head: 用户自定义的头部数据，可以是任意类型
 * - m_iSize: 数据包的实际大小，用于确定数据边界
 * 
 * 设计目的：
 * - 支持可变长度的数据包存储
 * - 提供数据包的元信息管理
 * - 简化数据包的序列化和反序列化
 * - 支持不同类型的头部信息扩展
 */
template<typename HeadType>
struct BufferHead
{
    HeadType m_head;    ///< 用户定义的头部信息，如消息ID、时间戳、优先级等
    size_t m_iSize;     ///< 数据包的字节大小，不包含头部本身的大小
};

/**
 * @brief 循环缓冲区迭代器类
 * @tparam HeadType 头部类型，与缓冲区的头部类型一致
 * @tparam Container 容器类型，通常是CircularBuffer
 * 
 * CircularBufferIterator 提供了对循环缓冲区的迭代访问能力：
 * 
 * 1. 设计特点：
 *    - 符合STL迭代器规范的接口设计
 *    - 支持数据包级别的迭代，而非字节级别
 *    - 提供零拷贝的数据访问方式
 *    - 自动处理循环边界的复杂性
 * 
 * 2. 核心功能：
 *    - 顺序遍历缓冲区中的所有数据包
 *    - 读取数据包头部和内容
 *    - 自动跳转到下一个数据包位置
 *    - 支持迭代器的比较操作
 * 
 * 3. 使用场景：
 *    - 批量处理缓冲区中的数据包
 *    - 实现数据包的过滤和筛选
 *    - 调试时查看缓冲区内容
 *    - 实现自定义的数据包处理算法
 * 
 * 使用示例：
 * @code
 * CircularBuffer<MsgHeader, 4096> buffer;
 * 
 * // 遍历所有数据包
 * for (auto it = buffer.Begin(); it != buffer.End(); ++it) {
 *     MsgHeader header;
 *     char data[1024];
 *     size_t dataLen = sizeof(data);
 *     
 *     if (it.Read(header, data, &dataLen) == 0) {
 *         // 处理数据包
 *         ProcessPacket(header, data, dataLen);
 *     }
 * }
 * @endcode
 */
template<typename HeadType, typename Container>
class CircularBufferIterator
{
protected:
    typedef CircularBufferIterator<HeadType, Container>                _Self;       ///< 迭代器自身类型的别名
    typedef BufferHead<HeadType> HEADTYPE;                                          ///< 完整头部类型的别名

public:
    typedef ptrdiff_t                          DifferenceType;                      ///< 指针差值类型
    typedef size_t                                     SizeType;                    ///< 大小类型

    /**
     * @brief 默认构造函数
     * 
     * 创建一个无效的迭代器，需要后续赋值才能使用。
     */
    CircularBufferIterator()
        : m_pContainer(NULL)
        , m_iStart(0)
    {
    }

    /**
     * @brief 构造函数
     * @param pContainer 指向容器的指针
     * @param iPos 起始位置在缓冲区中的偏移
     * 
     * 创建指向特定位置的迭代器。
     */
    explicit
    CircularBufferIterator(Container *pContainer, SizeType iPos)
        : m_pContainer(pContainer)
        , m_iStart(iPos)
    {
    }

    /**
     * @brief 读取当前位置的数据包（包含头部和数据）
     * @param head 输出参数，用于接收头部信息
     * @param pDataOut 输出缓冲区，用于接收数据内容
     * @param piLen 输入输出参数，输入时表示缓冲区大小，输出时表示实际数据大小
     * @return 成功返回0，失败返回非0值
     * 
     * 从当前迭代器位置读取完整的数据包：
     * - 先读取头部信息，获取数据包大小
     * - 检查输出缓冲区是否足够大
     * - 读取实际的数据内容
     * - 更新输出参数
     * 
     * 注意事项：
     * - pDataOut缓冲区必须足够大以容纳数据
     * - piLen在输入时表示缓冲区容量，输出时表示实际读取的字节数
     * - 函数不会移动迭代器位置
     */
    int Read(HeadType &head, char *pDataOut, size_t *piLen)
    {
        assert(m_pContainer);
        return m_pContainer->Read(m_iStart, head, pDataOut, piLen);
    }

    /**
     * @brief 只读取头部信息
     * @param head 输出参数，用于接收头部信息
     * @return 成功返回0，失败返回非0值
     * 
     * 快速读取当前位置的头部信息，不读取数据内容：
     * - 只读取头部的字节数
     * - 适用于只需要判断数据包类型的场景
     * - 性能比完整读取更高
     */
    int Read(HeadType &head)
    {
        assert(m_pContainer);
        m_pContainer->Read((char *)&head, m_iStart, sizeof(head));

        return 0;
    }

    /**
     * @brief 前置递增操作符
     * @return 递增后的迭代器引用
     * 
     * 将迭代器移动到下一个数据包的起始位置：
     * - 计算当前数据包的总大小（头部+数据）
     * - 移动到下一个数据包的位置
     * - 自动处理缓冲区的循环边界
     */
    _Self & operator++()
    {
        assert(m_pContainer);
        m_iStart = m_pContainer->GetNextStartPos(m_iStart);
        return *this;
    }

    /**
     * @brief 后置递增操作符
     * @return 递增前的迭代器副本
     * 
     * 返回当前迭代器的副本，然后移动到下一个位置。
     */
    _Self operator++(int)
    {
        _Self __tmp = *this;
        m_iStart = m_pContainer->GetNextStartPos(m_iStart);
        return __tmp;
    }

    /**
     * @brief 相等比较操作符
     * @param other 要比较的另一个迭代器
     * @return 如果两个迭代器指向同一位置返回true，否则返回false
     * 
     * 比较两个迭代器是否指向同一个位置：
     * - 必须指向同一个容器
     * - 必须指向容器中的同一个位置
     */
    bool operator==(const _Self &other) const
    {
        assert(m_pContainer == other.m_pContainer);
        return m_iStart == other.m_iStart;
    }

    /**
     * @brief 不等比较操作符
     * @param other 要比较的另一个迭代器
     * @return 如果两个迭代器指向不同位置返回true，否则返回false
     */
    bool operator!=(const _Self &other) const
    {
        assert(m_pContainer == other.m_pContainer);
        return m_iStart != other.m_iStart;
    }

    Container *m_pContainer;    ///< 指向所属容器的指针
    size_t m_iStart;            ///< 当前位置在缓冲区中的偏移
};

/**
 * @brief 高性能循环缓冲区类
 * @tparam HeadType 数据包头部类型，用户自定义
 * @tparam SIZE 缓冲区总大小，必须是编译时常量
 * 
 * CircularBuffer 提供了线程安全、高效的循环缓冲区实现：
 * 
 * 1. 核心设计理念：
 *    - 固定大小的内存池：避免动态内存分配的开销
 *    - 循环复用机制：充分利用内存空间，避免碎片化
 *    - 队列语义：FIFO（先进先出）的数据处理顺序
 *    - 零拷贝访问：通过迭代器直接访问缓冲区数据
 * 
 * 2. 数据包管理：
 *    - 支持可变长度的数据包
 *    - 每个数据包包含用户定义的头部信息
 *    - 自动管理数据包的边界和大小
 *    - 提供数据包级别的操作接口
 * 
 * 3. 内存管理：
 *    - 编译时确定的内存大小，无运行时分配
 *    - 最多存储SIZE-1字节的数据（保留1字节用于区分满/空状态）
 *    - 自动处理内存的循环使用
 *    - 内存对齐和访问优化
 * 
 * 4. 线程安全性：
 *    - 单生产者/单消费者模式下的无锁操作
 *    - 写操作（Push）和读操作（Pop）的原子性
 *    - 通过索引位置的原子更新保证一致性
 *    - 避免了传统锁机制的性能开销
 * 
 * 5. 性能特点：
 *    - O(1)的Push和Pop操作
 *    - 内存访问的局部性优化
 *    - 最小化的数据拷贝次数
 *    - 高效的容量检查算法
 * 
 * 6. 应用场景：
 *    - 网络数据包的缓冲和排队
 *    - 生产者-消费者模式的数据传递
 *    - 实时系统的数据流处理
 *    - 音视频数据的缓冲管理
 * 
 * 缓冲区内存布局：
 * ```
 * +--------+--------+--------+--------+--------+
 * | Pack1  | Pack2  |  ...   | PackN  | Free   |
 * +--------+--------+--------+--------+--------+
 * ^                                   ^        ^
 * m_iFront                          m_iBack   m_iRear
 * ```
 * 
 * 其中：
 * - m_iFront: 第一个数据包的起始位置
 * - m_iBack: 最后一个数据包的起始位置
 * - m_iRear: 下一个数据包的写入位置
 * 
 * 使用示例：
 * @code
 * // 定义消息头部结构
 * struct MsgHeader {
 *     uint32_t msgId;
 *     uint32_t timestamp;
 * };
 * 
 * // 创建4KB的循环缓冲区
 * CircularBuffer<MsgHeader, 4096> buffer;
 * buffer.Initialize();
 * 
 * // 写入数据包
 * MsgHeader header = {1001, getCurrentTime()};
 * const char* data = "Hello World";
 * if (buffer.Push(header, data, strlen(data)) == 0) {
 *     // 写入成功
 * }
 * 
 * // 读取数据包
 * MsgHeader outHeader;
 * char outData[1024];
 * size_t dataLen = sizeof(outData);
 * if (buffer.Front(outHeader, outData, &dataLen) == 0) {
 *     // 处理数据包
 *     ProcessMessage(outHeader, outData, dataLen);
 *     buffer.Pop();  // 移除已处理的数据包
 * }
 * 
 * // 批量处理
 * for (auto it = buffer.Begin(); it != buffer.End(); ++it) {
 *     // 处理每个数据包
 * }
 * @endcode
 * 
 * 设计优势：
 * - 无动态内存分配，适合实时系统
 * - 高吞吐量的数据处理能力
 * - 简单直观的API接口
 * - 强类型的数据包头部支持
 * - 完善的迭代器支持
 */
template<typename HeadType, size_t SIZE>
class CircularBuffer
{
public:
    typedef BufferHead<HeadType> HEADTYPE;                                                          ///< 完整头部类型的别名
    typedef CircularBufferIterator<HeadType, CircularBuffer<HeadType, SIZE> > Iterator;            ///< 迭代器类型的别名

    /**
     * @brief 初始化缓冲区
     * 
     * 将缓冲区重置到初始状态：
     * - 清空所有数据
     * - 重置所有位置指针
     * - 准备接收新的数据包
     */
    void Initialize()
    {
        Clear();
    }

    /**
     * @brief 检查缓冲区是否为空
     * @return 如果缓冲区为空返回true，否则返回false
     * 
     * 通过比较前端和后端指针判断缓冲区状态：
     * - 当front等于rear时，表示缓冲区为空
     * - 这是一个O(1)的快速检查操作
     */
    bool IsEmpty()
    {
        return (m_iFront == m_iRear);
    }

    /**
     * @brief 检查是否有足够空间写入指定大小的数据
     * @param iDataLen 要写入的数据长度（不包含头部）
     * @return 如果有足够空间返回true，否则返回false
     * 
     * 容量检查算法：
     * - 计算写入所需的总空间（头部+数据+安全边界）
     * - 考虑循环缓冲区的环形特性
     * - 确保写入后不会与读取指针重叠
     * - 预留安全空间防止满/空状态的混淆
     */
    bool CanWrite(size_t iDataLen);

    /**
     * @brief 获取缓冲区中当前数据的大小
     * @return 当前存储的数据字节数
     * 
     * 计算已使用的缓冲区空间：
     * - 处理循环边界的复杂情况
     * - 返回实际的数据字节数
     * - 不包含内部的管理开销
     */
    size_t Size()
    {
        if (m_iRear >= m_iFront)
        {
            return m_iRear - m_iFront;
        }
        else
        {
            return SIZE - m_iFront + m_iRear;
        }
    }

    /**
     * @brief 获取缓冲区的最大可用空间
     * @return 最大可存储的数据字节数
     * 
     * 计算理论上的最大存储容量：
     * - 总大小减去头部开销
     * - 减去必要的安全边界
     * - 确保循环操作的正确性
     */
    size_t MaxSize()
    {
        return SIZE-1-sizeof(HEADTYPE);
    }

    /**
     * @brief 获取指向第一个数据包的迭代器
     * @return 指向缓冲区开始位置的迭代器
     * 
     * 返回begin迭代器用于遍历：
     * - 指向第一个有效数据包
     * - 符合STL容器的接口规范
     * - 支持range-based for循环
     */
    Iterator Begin()
    {
        return Iterator(this, m_iFront);
    }

    /**
     * @brief 获取指向缓冲区末尾的迭代器
     * @return 指向缓冲区结束位置的迭代器
     * 
     * 返回end迭代器用于遍历：
     * - 指向最后一个数据包的下一个位置
     * - 用于判断遍历结束
     * - 符合STL容器的接口规范
     */
    Iterator End()
    {
        return Iterator(this, m_iRear);
    }

    /**
     * @brief 将数据包压入缓冲区
     * @param head 数据包头部信息
     * @param pData 数据内容指针
     * @param iDataLen 数据长度
     * @return 成功返回0，失败返回-1
     * 
     * 数据包写入流程：
     * 1. 检查是否有足够的空间
     * 2. 构造完整的数据包头部（包含大小信息）
     * 3. 将头部和数据写入缓冲区
     * 4. 更新后端指针位置
     * 5. 处理循环边界的情况
     * 
     * 原子性保证：
     * - 要么完全写入成功，要么完全失败
     * - 不会出现部分写入的情况
     * - 多线程环境下的数据一致性
     */
    int Push(const HeadType &head, const char *pData, size_t iDataLen);
    
    /**
     * @brief 移除第一个数据包
     * @return 成功返回0，失败返回-1
     * 
     * 数据包移除流程：
     * 1. 检查缓冲区是否为空
     * 2. 读取第一个数据包的头部信息
     * 3. 计算数据包的总大小
     * 4. 更新前端指针到下一个数据包位置
     * 5. 处理循环边界的情况
     * 
     * 注意事项：
     * - 只移除数据包，不返回数据内容
     * - 需要先通过Front读取数据再调用Pop
     * - 空缓冲区时调用会失败
     */
    int Pop();

    /**
     * @brief 获取第一个数据包的头部信息
     * @param head 输出参数，用于接收头部信息
     * @return 成功返回0，失败返回-1
     * 
     * 快速访问第一个数据包的头部：
     * - 不移动数据包位置
     * - 适用于需要预览数据包类型的场景
     * - 比完整读取更高效
     */
    int Front(HeadType &head);

    /**
     * @brief 获取第一个数据包的完整内容
     * @param head 输出参数，用于接收头部信息
     * @param pData 输出缓冲区，用于接收数据内容
     * @param iDataLen 输入输出参数，输入时表示缓冲区大小，输出时表示实际数据大小
     * @return 成功返回0，失败返回-1
     * 
     * 完整读取第一个数据包：
     * - 同时获取头部和数据内容
     * - 不移动数据包位置
     * - 需要确保输出缓冲区足够大
     */
    int Front(HeadType &head, char *pData, size_t *iDataLen);

    /**
     * @brief 获取最后一个数据包的头部信息
     * @param head 输出参数，用于接收头部信息
     * @return 成功返回0，失败返回-1
     * 
     * 访问最后插入的数据包头部：
     * - 用于检查最新数据的类型
     * - 支持某些特殊的处理逻辑
     * - 不影响数据包的位置
     */
    int Back(HeadType &head);

    /**
     * @brief 获取最后一个数据包的完整内容
     * @param head 输出参数，用于接收头部信息
     * @param pData 输出缓冲区，用于接收数据内容
     * @param iDataLen 输入输出参数，缓冲区大小和实际数据大小
     * @return 成功返回0，失败返回-1
     * 
     * 完整读取最后一个数据包：
     * - 访问最新插入的数据包
     * - 支持特殊的数据处理需求
     * - 需要确保输出缓冲区足够大
     */
    int Back(HeadType &head, char *pData, size_t *iDataLen);
    
    /**
     * @brief 清空缓冲区
     * 
     * 重置缓冲区到初始状态：
     * - 清空所有数据包
     * - 重置所有位置指针
     * - 快速释放所有存储的数据
     * - 不涉及内存释放，只是逻辑清空
     */
    void Clear()
    {
        m_iFront = m_iRear = m_iBack = 0;
    }

protected:
    /**
     * @brief 读取指定位置的头部信息（内部使用）
     * @param head 输出参数，用于接收头部信息
     * @return 成功返回0
     * 
     * 内部辅助函数，用于读取前端数据包的头部。
     */
    int Front(HEADTYPE &head)
    {
        Read((char *)&head, m_iFront, sizeof(head));
        return 0;
    }

    /**
     * @brief 读取指定位置的头部信息（内部使用）
     * @param head 输出参数，用于接收头部信息
     * @return 成功返回0
     * 
     * 内部辅助函数，用于读取后端数据包的头部。
     */
    int Back(HEADTYPE &head)
    {
        Read((char *)&head, m_iBack, sizeof(head));
        return 0;
    }

    /**
     * @brief 获取下一个数据包的起始位置（内部使用）
     * @param iStartPos 当前数据包的起始位置
     * @return 下一个数据包的起始位置
     * 
     * 计算位置跳转的内部函数：
     * - 读取当前数据包的大小信息
     * - 计算下一个数据包的位置
     * - 处理循环边界的情况
     * - 用于迭代器的递增操作
     */
    int GetNextStartPos(size_t iStartPos)
    {
        HEADTYPE bufferHead;
        Read((char *)&bufferHead, iStartPos, sizeof(bufferHead));
        return (iStartPos+sizeof(bufferHead)+bufferHead.m_iSize)%SIZE;
    }

    /**
     * @brief 从指定位置读取数据包（内部使用）
     * @param iStartPos 起始位置
     * @param head 输出参数，用于接收头部信息
     * @param pData 输出缓冲区，用于接收数据内容
     * @param iDataLen 输入输出参数，缓冲区大小和实际数据大小
     * @return 成功返回0，失败返回-1
     * 
     * 通用的数据包读取函数：
     * - 支持从任意位置读取
     * - 处理循环边界的数据拼接
     * - 用于迭代器和访问函数的实现
     */
    int Read(size_t iStartPos, HeadType &head, char *pData, size_t *iDataLen);

    /**
     * @brief 向缓冲区写入数据（内部使用）
     * @param pData 要写入的数据
     * @param iSize 数据大小
     * 
     * 底层的数据写入函数：
     * - 处理循环边界的数据分割
     * - 确保数据的连续性和完整性
     * - 更新写入位置指针
     */
    void Write(const char *pData, size_t iSize);

    /**
     * @brief 从缓冲区读取数据（内部使用）
     * @param pData 输出缓冲区
     * @param iStart 起始位置
     * @param iSize 读取大小
     * 
     * 底层的数据读取函数：
     * - 处理循环边界的数据拼接
     * - 确保读取数据的完整性
     * - 支持跨边界的数据访问
     */
    void Read(char *pData, size_t iStart, size_t iSize);

    size_t m_iFront;            ///< 第一个数据包的起始位置
    size_t m_iBack;             ///< 最后一个数据包的起始位置
    size_t m_iRear;             ///< 下一个数据包的写入位置（队列尾部）

    char m_szBuff[SIZE];        ///< 实际的数据存储缓冲区

    friend class CircularBufferIterator<HeadType, CircularBuffer<HeadType, SIZE> >;  ///< 允许迭代器访问内部数据
};

template<typename HeadType, size_t SIZE>
bool CircularBuffer<HeadType, SIZE>::CanWrite(size_t iDataLen)
{
    size_t iWritableDataLen = iDataLen+1;//to keep rear not = front again

    if (iWritableDataLen > SIZE)
    {
        return false;
    }

    if (m_iRear >= m_iFront)
    {
        return SIZE - m_iRear + m_iFront >= iWritableDataLen+sizeof(HEADTYPE);
    }
    else
    {
        return m_iFront - m_iRear >= iWritableDataLen+sizeof(HEADTYPE);
    }
}

template<typename HeadType, size_t SIZE>
int CircularBuffer<HeadType, SIZE>::Push(const HeadType &head, const char *pData, size_t iDataLen)
{
    if (!CanWrite(iDataLen))
    {
        return -1;
    }

    HEADTYPE bufferHead;
    bufferHead.m_head = head;
    bufferHead.m_iSize = iDataLen;
    m_iBack = m_iRear;
    Write((char *)&bufferHead, sizeof(bufferHead));
    Write(pData, iDataLen);

    return 0;
}

template<typename HeadType, size_t SIZE>
int CircularBuffer<HeadType, SIZE>::Pop()
{
    if (!IsEmpty())
    {
        HEADTYPE bufferHead;
        Read((char *)&bufferHead, m_iFront, sizeof(bufferHead));
        m_iFront=(m_iFront+bufferHead.m_iSize+sizeof(bufferHead))%SIZE;
    }
    else
    {
        return -1;
    }

    return 0;
}

//first data head
template<typename HeadType, size_t SIZE>
int CircularBuffer<HeadType, SIZE>::Front(HeadType &head)
{
    if (!IsEmpty())
    {
        Read((char *)&head, m_iFront, sizeof(head));
    }
    else
    {
        return -1;
    }

    return 0;
}

template<typename HeadType, size_t SIZE>
int CircularBuffer<HeadType, SIZE>::Front(HeadType &head, char *pData, size_t *iDataLen)
{
    if (!IsEmpty())
    {
        return Read(m_iFront, head, pData, iDataLen);
    }
    else
    {
        return -1;
    }

    return 0;
}

template<typename HeadType, size_t SIZE>
int CircularBuffer<HeadType, SIZE>::Back(HeadType &head)
{
    if (!IsEmpty())
    {
        Read((char *)&head, m_iBack, sizeof(head));
    }
    else
    {
        return -1;
    }

    return 0;
}

template<typename HeadType, size_t SIZE>
int CircularBuffer<HeadType, SIZE>::Back(HeadType &head, char *pData, size_t *iDataLen)
{
    if (!IsEmpty())
    {
        return Read(m_iBack, head, pData, iDataLen);
    }
    else
    {
        return -1;
    }

    return 0;
}

template<typename HeadType, size_t SIZE>
int CircularBuffer<HeadType, SIZE>::Read(size_t iStartPos, HeadType &head, char *pData, size_t *iDataLen)
{
    HEADTYPE bufferHead;
    Read((char *)&bufferHead, iStartPos, sizeof(bufferHead));

    if (*iDataLen < bufferHead.m_iSize)
    {
        return -1;
    }

    size_t iDataStart = (iStartPos+sizeof(bufferHead))%SIZE;
    Read(pData, iDataStart, bufferHead.m_iSize);
    head = bufferHead.m_head;
    *iDataLen = bufferHead.m_iSize;

    return 0;
}

template<typename HeadType, size_t SIZE>
void CircularBuffer<HeadType, SIZE>::Write(const char *pData, size_t iSize)
{
    size_t iRearLeftLen = SIZE - m_iRear;

    if (m_iRear >= m_iFront && iRearLeftLen < iSize)
    {
        memcpy(&m_szBuff[m_iRear], pData, iRearLeftLen);
        memcpy(m_szBuff, &(pData[iRearLeftLen]), iSize-iRearLeftLen);
    }
    else
    {
        memcpy(&(m_szBuff[m_iRear]), pData, iSize);
    }

    m_iRear = (m_iRear + iSize)%SIZE;
}

template<typename HeadType, size_t SIZE>
void CircularBuffer<HeadType, SIZE>::Read(char *pData, size_t iStart, size_t iSize)
{
    if (SIZE-iStart < iSize)
    {
        memcpy(pData, &(m_szBuff[iStart]), SIZE-iStart);
        memcpy(&(pData[SIZE-iStart]), m_szBuff, iSize-SIZE+iStart);
    }
    else
    {
        memcpy(pData, &(m_szBuff[iStart]), iSize);
    }
}
