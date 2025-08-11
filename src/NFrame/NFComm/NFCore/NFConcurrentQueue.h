// -------------------------------------------------------------------------
//    @FileName         :    NFConcurrentQueue.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFConcurrentQueue.h
//
// -------------------------------------------------------------------------

/**
 * @file NFConcurrentQueue.h
 * @brief 高性能并发队列封装类
 * 
 * 此文件提供了基于moodycamel::ConcurrentQueue的高性能并发队列封装，
 * 支持多线程安全的入队和出队操作，以及对象池管理功能。
 */

#pragma once

#include "common/concurrentqueue/concurrentqueue.h"

/**
 * @brief 高性能并发队列模板类
 * 
 * NFConcurrentQueue是对moodycamel::ConcurrentQueue的封装，提供了简化的接口。
 * 这是一个无锁并发队列，支持多个生产者和多个消费者同时操作。
 * 
 * 主要特性：
 * - 无锁设计：使用原子操作和内存屏障，避免锁竞争
 * - 高性能：针对多核处理器优化，支持高并发场景
 * - 线程安全：支持多个线程同时进行入队和出队操作
 * - 批量操作：支持批量入队和出队，提高吞吐量
 * - 内存高效：动态分配内存，避免预分配大量空间
 * 
 * 适用场景：
 * - 生产者-消费者模式
 * - 多线程任务队列
 * - 高并发消息传递
 * - 无锁数据结构需求
 * 
 * 使用方法：
 * @code
 * NFConcurrentQueue<int> queue;
 * 
 * // 生产者线程
 * std::thread producer([&queue]() {
 *     for (int i = 0; i < 1000; ++i) {
 *         queue.Enqueue(i);
 *     }
 * });
 * 
 * // 消费者线程
 * std::thread consumer([&queue]() {
 *     int value;
 *     while (queue.TryDequeue(value)) {
 *         std::cout << "处理: " << value << std::endl;
 *     }
 * });
 * 
 * // 批量操作
 * std::vector<int> data = {1, 2, 3, 4, 5};
 * queue.EnqueueBulk(data);
 * 
 * std::vector<int> results(10);
 * size_t count = queue.TryDequeueBulk(results);
 * @endcode
 * 
 * @tparam T 队列元素类型，必须支持拷贝构造
 * 
 * @note 基于Cameron Desrochers的moodycamel::ConcurrentQueue实现
 * @note 性能优于std::queue + std::mutex的组合
 */
template<typename T>
class NFConcurrentQueue
{
public:
    /**
     * @brief 默认构造函数
     */
    NFConcurrentQueue() { }
    
    /**
     * @brief 虚析构函数
     */
    virtual ~NFConcurrentQueue() { }
    
public:
    /**
     * @brief 入队操作（可能分配内存）
     * 
     * 将元素添加到队列末尾。如果需要更多内存，会自动分配。
     * 这是线程安全的操作，可能会阻塞以分配内存。
     * 
     * @param item 要入队的元素
     * @return bool 总是返回true（操作成功）
     * 
     * @note 可能会分配内存，因此可能比TryEnqueue稍慢
     * @note 线程安全，支持多个线程同时调用
     */
    inline bool Enqueue(const T& item)
    {
        return m_queue.enqueue(item);
    }

    /**
     * @brief 尝试入队操作（不分配内存）
     * 
     * 尝试将元素添加到队列，但不分配新内存。
     * 如果没有可用空间，操作会失败。
     * 
     * @param item 要入队的元素
     * @return bool 成功返回true，内存不足时返回false
     * 
     * @note 不分配内存，性能更高但可能失败
     * @note 适用于对内存分配敏感的实时系统
     */
    inline bool TryEnqueue(T const& item)
    {
        return m_queue.try_enqueue(item);
    }

    /**
     * @brief 批量入队操作（迭代器版本）
     * 
     * 将指定范围的元素批量添加到队列中。
     * 批量操作比逐个入队更高效。
     * 
     * @tparam It 迭代器类型
     * @param itemFirst 元素范围的起始迭代器
     * @param count 要入队的元素数量
     * @return bool 操作成功返回true
     * 
     * @note 比多次调用Enqueue更高效
     * @note 迭代器必须可解引用count次
     */
    template<typename It>
    bool EnqueueBulk(It itemFirst, size_t count)
    {
        return m_queue.enqueue_bulk(itemFirst, count);
    }

    /**
     * @brief 批量入队操作（vector版本）
     * 
     * 将vector中的所有元素批量添加到队列中。
     * 
     * @param vec 包含要入队元素的vector
     * @return bool 操作成功返回true
     * 
     * @note 会入队vector中的所有元素
     * @note 比逐个入队更高效
     */
    bool EnqueueBulk(const std::vector<T>& vec)
    {
        return m_queue.enqueue_bulk(vec.data(), vec.size());
    }

    /**
     * @brief 尝试出队操作
     * 
     * 尝试从队列头部取出一个元素。
     * 如果队列为空，操作会立即失败而不阻塞。
     * 
     * @tparam U 输出元素类型（通常与T相同）
     * @param item [out] 接收出队元素的变量
     * @return bool 成功返回true，队列为空返回false
     * 
     * @note 非阻塞操作，队列为空时立即返回
     * @note 线程安全，支持多个线程同时调用
     */
    template<typename U>
    bool TryDequeue(U& item)
    {
        return m_queue.try_dequeue(item);
    }

    /**
     * @brief 批量出队操作（迭代器版本）
     * 
     * 尝试从队列中取出多个元素。
     * 实际出队的元素数量可能少于请求的数量。
     * 
     * @tparam It 迭代器类型
     * @param itemFirst 接收元素的起始迭代器
     * @param max 最多出队的元素数量
     * @return size_t 实际出队的元素数量
     * 
     * @note 返回值可能小于max，表示实际出队的元素数
     * @note 比多次调用TryDequeue更高效
     */
    template<typename It>
    size_t TryDequeueBulk(It itemFirst, size_t max)
    {
        return m_queue.try_dequeue_bulk(itemFirst, max);
    }

    /**
     * @brief 批量出队操作（vector版本）
     * 
     * 尝试从队列中取出多个元素到vector中。
     * 会自动调整vector的大小以匹配实际出队的元素数量。
     * 
     * @param vec [in,out] 接收元素的vector，会被调整大小
     * @return size_t 实际出队的元素数量
     * 
     * @note vector的初始大小决定了最多出队的元素数量
     * @note 函数返回后，vector的大小等于实际出队的元素数量
     */
    size_t TryDequeueBulk(std::vector<T>& vec)
    {
        size_t size = m_queue.try_dequeue_bulk(vec.data(), vec.size());
        vec.resize(size);
        return size;
    }

    /**
     * @brief 获取队列大小（近似值）
     * 
     * 返回队列中元素的近似数量。
     * 在并发环境中，这个值可能不是精确的。
     * 
     * @return size_t 队列中元素的近似数量
     * 
     * @note 返回值是近似的，不保证精确性
     * @note 主要用于监控和调试，不应用于逻辑判断
     */
    size_t GetQueueSize() {
        return m_queue.size_approx();
    }

    /**
     * @brief 检查队列是否为空（近似判断）
     * 
     * 判断队列是否可能为空。
     * 在并发环境中，结果可能在返回后立即失效。
     * 
     * @return bool 队列可能为空时返回true
     * 
     * @note 判断结果是近似的，可能存在竞态条件
     * @note 主要用于优化和提示，不应用于逻辑控制
     */
    bool IsQueueEmpty() {
        return m_queue.size_approx() == 0;
    }
    
private:
    /** @brief 底层的moodycamel并发队列实例 */
    moodycamel::ConcurrentQueue<T> m_queue;
};

/**
 * @brief 并发队列对象池模板类
 * 
 * NFConcurrentQueuePool提供了基于并发队列的对象池实现。
 * 主要用于管理可重用的对象，减少内存分配和释放的开销。
 * 
 * 主要特性：
 * - 对象复用：重用已分配的对象，减少new/delete开销
 * - 线程安全：基于并发队列，支持多线程访问
 * - 自动管理：析构时自动清理所有池中的对象
 * - 容量控制：支持设置最大池容量，防止内存无限增长
 * 
 * 适用场景：
 * - 频繁创建和销毁的对象
 * - 内存分配敏感的场景
 * - 网络消息对象的复用
 * - 临时对象的管理
 * 
 * @tparam TYPE 池中对象的类型，必须支持动态分配
 * 
 * @note 池中存储的是对象指针，对象的生命周期由池管理
 */
template<typename TYPE>
class NFConcurrentQueuePool
{
public:
    /**
     * @brief 构造函数
     * 
     * @param maxPoolSize 对象池的最大容量，默认1000
     */
    NFConcurrentQueuePool(int maxPoolSize = 1000): m_maxPoolSize(maxPoolSize)
    {

    }

    /**
     * @brief 析构函数
     * 
     * 自动清理池中的所有对象，防止内存泄漏。
     */
    virtual ~NFConcurrentQueuePool()
    {
        while(!m_msgQueue.IsQueueEmpty())
        {
            std::vector<TYPE*> vec;
            vec.resize(200);
            if (m_msgQueue.TryDequeueBulk(vec))
            {
                for(int i = 0; i < (int)vec.size(); i++)
                {
                    if (vec[i])
                    {
                        NF_SAFE_DELETE(vec[i]);
                    }
                }
            }
        }
    }
public:
    TYPE* Alloc()
    {
        TYPE* pType = NULL;
        if (!m_msgQueue.TryDequeue(pType))
        {
            pType = new TYPE();
        }
        return pType;
    }

    bool Free(TYPE* pType)
    {
        if (m_msgQueue.GetQueueSize() > m_maxPoolSize)
        {
            NF_SAFE_DELETE(pType);
        }
        else {
            if (!m_msgQueue.Enqueue(pType))
            {
                NF_SAFE_DELETE(pType);
            }
        }
        return true;
    }
private:
    NFConcurrentQueue<TYPE*> m_msgQueue;
    std::atomic<int> m_maxPoolSize;
};

