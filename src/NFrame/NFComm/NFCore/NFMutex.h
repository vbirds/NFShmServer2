// -------------------------------------------------------------------------
//    @FileName         :    NFMutex.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFMutex.h
 * @brief 互斥锁和互斥锁守卫类定义
 * 
 * 此文件定义了基于标准库std::mutex的互斥锁类和RAII风格的互斥锁守卫类。
 * 提供了简单易用的线程同步机制。
 */

#pragma once

#include "NFPlatform.h"
#include <thread>
#include <mutex>


/**
 * @brief 互斥锁类
 * 
 * 基于std::mutex的简单互斥锁封装，提供了基本的锁定和解锁功能。
 * 用于在多线程环境中保护共享资源的访问。
 * 
 * 使用方法：
 * @code
 * NFMutex mutex;
 * mutex.Lock();
 * // 临界区代码
 * mutex.Unlock();
 * @endcode
 * 
 * @note 建议使用NFMutexGuard进行RAII风格的自动锁管理
 */
class _NFExport NFMutex
{
public:

	friend class NFCondition;
	friend class NFLock;

	/**
	 * @brief 构造函数
	 * 创建一个新的互斥锁实例
	 */
	NF_FORCEINLINE NFMutex()
	{
	}

	/**
	 * @brief 析构函数
	 * 销毁互斥锁实例
	 */
	NF_FORCEINLINE ~NFMutex()
	{

	}

	/**
	 * @brief 锁定互斥锁
	 * 如果互斥锁已被其他线程锁定，则当前线程将阻塞直到获得锁
	 */
	NF_FORCEINLINE void Lock()
	{
		mMutex.lock();
	}

	/**
	 * @brief 解锁互斥锁
	 * 释放互斥锁，允许其他等待的线程获得锁
	 */
	NF_FORCEINLINE void Unlock()
	{
		mMutex.unlock();
	}

private:

	/** @brief 禁用拷贝构造函数 */
	NFMutex(const NFMutex &other);
	/** @brief 禁用赋值操作符 */
	NFMutex &operator=(const NFMutex &other);

	/** @brief 标准库互斥锁实例 */
	std::mutex mMutex;
};

/**
 * @brief 互斥锁守卫类
 * 
 * RAII风格的互斥锁管理类，在构造时自动锁定互斥锁，在析构时自动解锁。
 * 这种方式可以确保即使在异常情况下也能正确释放锁。
 * 
 * 使用方法：
 * @code
 * {
 *     NFMutexGuard guard; // 自动锁定
 *     // 临界区代码
 * } // 自动解锁
 * @endcode
 * 
 * @note 每个NFMutexGuard实例都有自己的互斥锁，不同实例之间不会互相影响
 */
class _NFExport NFMutexGuard
{
public:

	friend class NFCondition;
	friend class NFLock;

	/**
	 * @brief 构造函数
	 * 创建守卫并自动锁定内部的互斥锁
	 */
	NF_FORCEINLINE NFMutexGuard()
	{
		mMutex.Lock();
	}

	/**
	 * @brief 析构函数
	 * 自动解锁内部的互斥锁
	 */
	NF_FORCEINLINE ~NFMutexGuard()
	{
		mMutex.Unlock();
	}

private:

	/** @brief 禁用拷贝构造函数 */
	NFMutexGuard(const NFMutexGuard &other);
	/** @brief 禁用赋值操作符 */
	NFMutexGuard &operator=(const NFMutexGuard &other);

	/** @brief 内部互斥锁实例 */
	NFMutex mMutex;
};

