// -------------------------------------------------------------------------
//    @FileName         :    NFLock.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFLock.h
 * @brief 可移植的锁同步原语
 * 
 * 此文件定义了一个RAII风格的锁管理类，基于std::unique_lock实现，
 * 提供了自动的锁获取和释放机制，以及手动的锁控制功能。
 */

#pragma once

#include "NFPlatform.h"
#include "NFMutex.h"
#include <thread>

/**
 * @brief 可移植的锁同步原语
 * 
 * NFLock是一个帮助类，允许在作用域内自动锁定和解锁互斥对象。
 * 采用RAII（Resource Acquisition Is Initialization）设计模式，
 * 确保在任何情况下（包括异常抛出）都能正确释放锁。
 * 
 * 主要特性：
 * - 自动锁管理：构造时自动获取锁，析构时自动释放锁
 * - 手动控制：支持临时解锁和重新锁定
 * - 异常安全：即使发生异常也能正确释放锁
 * - 条件变量支持：可与NFCondition配合使用
 * 
 * 使用场景：
 * - 临界区保护
 * - 条件变量等待
 * - 复杂的锁控制逻辑
 * 
 * 使用方法：
 * @code
 * NFMutex mutex;
 * 
 * // 基本用法：自动锁管理
 * {
 *     NFLock lock(mutex);  // 自动获取锁
 *     // 临界区代码
 * }  // 自动释放锁
 * 
 * // 高级用法：手动控制
 * {
 *     NFLock lock(mutex);  // 获取锁
 *     // 一些操作
 *     lock.Unlock();       // 临时释放锁
 *     // 非临界区操作
 *     lock.Relock();       // 重新获取锁
 *     // 更多操作
 * }  // 自动释放锁
 * @endcode
 * 
 * @note 此类禁用拷贝构造和赋值操作，确保锁的唯一所有权
 */
class _NFExport NFLock
{
public:

	friend class NFCondition;

	/**
	 * @brief 构造函数
	 * 
	 * 创建锁对象并立即锁定给定的互斥对象。
	 * 如果互斥对象已被其他线程锁定，当前线程将阻塞直到获得锁。
	 * 
	 * @param mutex 要锁定的NFMutex对象的引用
	 */
	NF_FORCEINLINE explicit NFLock(NFMutex &mutex) :
		mLock(mutex.mMutex)
	{

	}

	/**
	 * @brief 析构函数
	 * 
	 * 自动解锁在构造时锁定的互斥对象。
	 * 这确保了即使在异常情况下也能正确释放锁，实现异常安全的RAII模式。
	 */
	NF_FORCEINLINE ~NFLock()
	{

	}

	/**
	 * @brief 显式临时解锁
	 * 
	 * 临时解锁已锁定的互斥对象，允许其他线程获得锁。
	 * 通常用于在持有锁的情况下需要执行可能耗时的非临界区操作。
	 * 
	 * @note 调用者应该在此调用后和对象销毁前始终调用Relock()
	 * @note 只能在锁当前被持有的状态下调用
	 */
	NF_FORCEINLINE void Unlock()
	{
		NF_COMM_ASSERT(mLock.owns_lock() == true);
		mLock.unlock();
	}

	/**
	 * @brief 重新锁定
	 * 
	 * 重新锁定关联的互斥对象，该对象必须之前通过Unlock()解锁。
	 * 如果互斥对象已被其他线程锁定，当前线程将阻塞直到获得锁。
	 * 
	 * @note 只能在先前调用Unlock()之后调用
	 * @note 只能在锁当前未被持有的状态下调用
	 */
	NF_FORCEINLINE void Relock()
	{
		NF_COMM_ASSERT(mLock.owns_lock() == false);
		mLock.lock();
	}

private:

	/** @brief 禁用拷贝构造函数，确保锁的唯一所有权 */
	NFLock(const NFLock &other);
	/** @brief 禁用赋值操作符，确保锁的唯一所有权 */
	NFLock &operator=(const NFLock &other);

	/** @brief 底层的unique_lock对象，管理实际的锁操作 */
	std::unique_lock<std::mutex> mLock;
};