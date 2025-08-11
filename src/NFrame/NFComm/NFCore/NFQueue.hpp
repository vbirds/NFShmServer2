// -------------------------------------------------------------------------
//    @FileName         :    NFQueue.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :
//
// -------------------------------------------------------------------------

/**
 * @file NFQueue.hpp
 * @brief 线程安全队列模板类定义
 * 
 * 此文件提供了两种线程安全的队列实现：基于vector的NFQueueVector和
 * 基于list的NFQueue。都使用互斥锁保证线程安全，适用于多线程环境下的
 * 数据传递和缓冲。
 */

#ifndef NF_QUEUE_H
#define NF_QUEUE_H

#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include "NFPlatform.h"
#include "NFMutex.h"
#include "NFLock.h"

/**
 * @brief 基于vector的线程安全队列
 * 
 * NFQueueVector是一个基于std::vector实现的线程安全队列。
 * 使用互斥锁保证线程安全，支持多线程并发访问。
 * 
 * 主要特性：
 * - 线程安全：所有操作都使用互斥锁保护
 * - 批量操作：支持批量弹出所有元素
 * - 内存连续：基于vector实现，内存布局连续
 * - RAII设计：使用NFLock确保异常安全
 * 
 * 适用场景：
 * - 生产者-消费者模式
 * - 任务队列
 * - 数据缓冲
 * - 需要批量处理的场景
 * 
 * 使用方法：
 * @code
 * NFQueueVector<int> queue;
 * 
 * // 生产者线程
 * queue.Push(42);
 * queue.Push(100);
 * 
 * // 消费者线程 - 单个元素
 * int value;
 * if (queue.Pop(value)) {
 *     std::cout << "取出: " << value << std::endl;
 * }
 * 
 * // 消费者线程 - 批量处理
 * std::vector<int> allItems;
 * queue.Pop(allItems);
 * for (int item : allItems) {
 *     std::cout << "处理: " << item << std::endl;
 * }
 * @endcode
 * 
 * @tparam T 队列元素的类型
 * @note 由于基于vector实现，中间删除效率较低，适合后进先出或批量处理场景
 */
template <typename T>
class NFQueueVector
{
public:
	/**
	 * @brief 默认构造函数
	 */
	NFQueueVector()
	{
	}

	/**
	 * @brief 虚析构函数
	 * 确保派生类能正确析构
	 */
	virtual ~NFQueueVector()
	{
	}

	/**
	 * @brief 向队列中添加元素
	 * 
	 * 线程安全地向队列尾部添加一个元素。
	 * 
	 * @param object 要添加的元素（通过const引用传递）
	 * @return bool 总是返回true（操作成功）
	 * 
	 * @note 此操作是线程安全的
	 * @note 元素通过拷贝方式添加到队列中
	 */
	bool Push(const T& object)
	{
		NFLock lock(mMutex);
		mList.push_back(object);
		return true;
	}

	/**
	 * @brief 批量弹出所有元素
	 * 
	 * 线程安全地将队列中的所有元素移动到提供的vector中，
	 * 原队列变为空。使用swap操作提高效率。
	 * 
	 * @param vecObj [out] 接收所有元素的vector容器
	 * @return bool 总是返回true（操作成功）
	 * 
	 * @note 此操作是线程安全的
	 * @note 使用swap操作，效率很高
	 * @note 调用后原队列为空
	 */
	bool Pop(std::vector<T>& vecObj)
	{
		NFLock lock(mMutex);
		mList.swap(vecObj);
		return true;
	}

	/**
	 * @brief 弹出单个元素
	 * 
	 * 线程安全地从队列头部弹出一个元素。
	 * 如果队列为空，则操作失败。
	 * 
	 * @param object [out] 接收弹出元素的变量
	 * @return bool 成功返回true，队列为空时返回false
	 * 
	 * @note 此操作是线程安全的
	 * @note 空队列时不会阻塞，直接返回false
	 */
	bool Pop(T& object)
	{
		NFLock lock(mMutex);
		if (mList.empty())
		{
			return false;
		}
		object = mList.front();
		mList.pop_front();

		return true;
	}

	/**
	 * @brief 获取队列中元素的数量
	 * 
	 * 线程安全地返回当前队列中元素的数量。
	 * 
	 * @return uint32_t 队列中元素的数量
	 * 
	 * @note 此操作是线程安全的
	 * @note 返回值是调用时刻的快照，可能在返回后立即改变
	 */
	uint32_t Count()
	{
		NFLock lock(mMutex);
		return (uint32_t)mList.size();
	}
private:
	/** @brief 存储队列元素的vector容器 */
	std::vector<T> mList;
	/** @brief 保护队列操作的互斥锁 */
	NFMutex mMutex;
};

/**
 * @brief 基于list的线程安全队列
 * 
 * NFQueue是一个基于std::list实现的线程安全队列。
 * 使用互斥锁保证线程安全，支持多线程并发访问。
 * 
 * 主要特性：
 * - 线程安全：所有操作都使用互斥锁保护
 * - 高效插入删除：基于双向链表，头尾操作效率高
 * - 批量操作：支持批量弹出所有元素
 * - RAII设计：使用NFLock确保异常安全
 * 
 * 适用场景：
 * - 生产者-消费者模式
 * - 消息队列
 * - 事件队列
 * - 频繁插入删除的场景
 * 
 * 使用方法：
 * @code
 * NFQueue<std::string> messageQueue;
 * 
 * // 生产者线程
 * messageQueue.Push("Hello");
 * messageQueue.Push("World");
 * 
 * // 消费者线程 - 单个消息
 * std::string message;
 * if (messageQueue.Pop(message)) {
 *     std::cout << "收到消息: " << message << std::endl;
 * }
 * 
 * // 消费者线程 - 批量处理
 * std::list<std::string> allMessages;
 * messageQueue.Pop(allMessages);
 * for (const auto& msg : allMessages) {
 *     std::cout << "处理消息: " << msg << std::endl;
 * }
 * @endcode
 * 
 * @tparam T 队列元素的类型
 * @note 基于list实现，插入删除效率高，但内存不连续
 */
template <typename T>
class NFQueue
{
public:
	/**
	 * @brief 默认构造函数
	 */
	NFQueue()
	{
	}

	/**
	 * @brief 虚析构函数
	 * 确保派生类能正确析构
	 */
	virtual ~NFQueue()
	{
	}

	/**
	 * @brief 向队列中添加元素
	 * 
	 * 线程安全地向队列尾部添加一个元素。
	 * 
	 * @param object 要添加的元素（通过const引用传递）
	 * @return bool 总是返回true（操作成功）
	 * 
	 * @note 此操作是线程安全的
	 * @note 元素通过拷贝方式添加到队列中
	 */
	bool Push(const T& object)
	{
		NFLock lock(mMutex);
		mList.push_back(object);
		return true;
	}

	/**
	 * @brief 批量弹出所有元素
	 * 
	 * 线程安全地将队列中的所有元素移动到提供的list中，
	 * 原队列变为空。使用swap操作提高效率。
	 * 
	 * @param vecObj [out] 接收所有元素的list容器
	 * @return bool 总是返回true（操作成功）
	 * 
	 * @note 此操作是线程安全的
	 * @note 使用swap操作，效率很高
	 * @note 调用后原队列为空
	 */
	bool Pop(std::list<T>& vecObj)
	{
		NFLock lock(mMutex);
		mList.swap(vecObj);
		return true;
	}

	/**
	 * @brief 弹出单个元素
	 * 
	 * 线程安全地从队列头部弹出一个元素。
	 * 如果队列为空，则操作失败。
	 * 
	 * @param object [out] 接收弹出元素的变量
	 * @return bool 成功返回true，队列为空时返回false
	 * 
	 * @note 此操作是线程安全的
	 * @note 空队列时不会阻塞，直接返回false
	 */
	bool Pop(T& object)
	{
		NFLock lock(mMutex);
		if (mList.empty())
		{
			return false;
		}

		object = mList.front();
		mList.pop_front();

		return true;
	}

	/**
	 * @brief 获取队列中元素的数量
	 * 
	 * 线程安全地返回当前队列中元素的数量。
	 * 
	 * @return uint32_t 队列中元素的数量
	 * 
	 * @note 此操作是线程安全的
	 * @note 返回值是调用时刻的快照，可能在返回后立即改变
	 */
	uint32_t Count()
	{
		NFLock lock(mMutex);
		return (uint32_t)mList.size();
	}
private:
	/** @brief 存储队列元素的list容器 */
	std::list<T> mList;
	/** @brief 保护队列操作的互斥锁 */
	NFMutex mMutex;
};

#endif

