// -------------------------------------------------------------------------
//    @FileName         :    NFSpinLock.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFSpinLock.h
 * @brief 自旋锁实现和相关宏定义
 * 
 * 此文件实现了高效的自旋锁机制，包括平台特定的CPU暂停指令、
 * 线程让渡和睡眠机制。自旋锁适用于短时间的临界区保护。
 */

#pragma once

#include "NFAtomic.h"

#if defined(_MSC_VER)

#include <Windows.h> // YieldProcessor

#include <Processthreadsapi.h>
#include <Synchapi.h> // Windows server
#include <intrin.h>

#endif

/**
 * ==============================================
 * ======            asm pause             ======
 * ==============================================
 */
#if defined(_MSC_VER)

 /*
  * See: http://msdn.microsoft.com/en-us/library/windows/desktop/ms687419(v=vs.85).aspx
  * Not for intel c++ compiler, so ignore http://software.intel.com/en-us/forums/topic/296168
  */

#ifdef YieldProcessor
/** @brief Windows平台的CPU暂停指令宏，用于自旋锁中减少CPU功耗 */
#define __UTIL_LOCK_SPIN_LOCK_PAUSE() YieldProcessor()
#endif

#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
 /**
  * See: Intel(R) 64 and IA-32 Architectures Software Developer's Manual V2
  * PAUSE-Spin Loop Hint, 4-57
  * http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.html?wapkw=instruction+set+reference
  * 
  * @brief x86/x64平台的PAUSE指令，在自旋循环中提供性能提示
  */
#define __UTIL_LOCK_SPIN_LOCK_PAUSE() __asm__ __volatile__("pause")
#elif defined(__ia64__) || defined(__ia64)
 /**
  * See: Intel(R) Itanium(R) Architecture Developer's Manual, Vol.3
  * hint - Performance Hint, 3:145
  * http://www.intel.com/content/www/us/en/processors/itanium/itanium-architecture-vol-3-manual.html
  * 
  * @brief IA64平台的暂停指令
  */
#define __UTIL_LOCK_SPIN_LOCK_PAUSE() __asm__ __volatile__("hint @pause")
#elif defined(__arm__) && !defined(__ANDROID__)
 /**
  * See: ARM Architecture Reference Manuals (YIELD)
  * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.subset.architecture.reference/index.html
  * 
  * @brief ARM平台的YIELD指令，建议调度器运行其他线程
  */
#define __UTIL_LOCK_SPIN_LOCK_PAUSE() __asm__ __volatile__("yield")
#endif

#endif /*compilers*/

  // set pause do nothing
#if !defined(__UTIL_LOCK_SPIN_LOCK_PAUSE)
/** @brief 如果平台不支持特定的暂停指令，则定义为空操作 */
#define __UTIL_LOCK_SPIN_LOCK_PAUSE()
#endif /*!defined(CAPO_SPIN_LOCK_PAUSE)*/


/**
 * ==============================================
 * ======            cpu yield             ======
 * ==============================================
 */
#if 0 && defined(_MSC_VER)

 // SwitchToThread only can be used in desktop system

/** @brief Windows桌面系统的线程切换宏（当前被禁用） */
#define __UTIL_LOCK_SPIN_LOCK_CPU_YIELD() SwitchToThread()

#elif defined(__linux__) || defined(__unix__)
#include <sched.h>
/** @brief Linux/Unix系统的CPU让渡宏，主动放弃CPU时间片 */
#define __UTIL_LOCK_SPIN_LOCK_CPU_YIELD() sched_yield()
#endif

#ifndef __UTIL_LOCK_SPIN_LOCK_CPU_YIELD
/** @brief 默认的CPU让渡操作，使用暂停指令 */
#define __UTIL_LOCK_SPIN_LOCK_CPU_YIELD() __UTIL_LOCK_SPIN_LOCK_PAUSE()
#endif

 /**
  * ==============================================
  * ======           thread yield           ======
  * ==============================================
  */
#if defined(__UTIL_LOCK_ATOMIC_INT_TYPE_ATOMIC_STD)
#include <chrono>
#include <thread>
/** @brief 标准库的线程让渡操作 */
#define __UTIL_LOCK_SPIN_LOCK_THREAD_YIELD() ::std::this_thread::yield()
/** @brief 标准库的线程睡眠操作，睡眠1毫秒 */
#define __UTIL_LOCK_SPIN_LOCK_THREAD_SLEEP() ::std::this_thread::sleep_for(::std::chrono::milliseconds(1))
#elif defined(_MSC_VER)
#define __UTIL_LOCK_SPIN_LOCK_THREAD_YIELD() Sleep(0)
#define __UTIL_LOCK_SPIN_LOCK_THREAD_SLEEP() Sleep(1)
#endif

#ifndef __UTIL_LOCK_SPIN_LOCK_THREAD_YIELD
#define __UTIL_LOCK_SPIN_LOCK_THREAD_YIELD()
#define __UTIL_LOCK_SPIN_LOCK_THREAD_SLEEP() __UTIL_LOCK_SPIN_LOCK_CPU_YIELD()
#endif

  /**
   * ==============================================
   * ======           spin lock wait         ======
   * ==============================================
   * @note
   *   1. busy-wait
   *   2. asm pause
   *   3. thread give up cpu time slice but will not switch to another process
   *   4. thread give up cpu time slice (may switch to another process)
   *   5. sleep (will switch to another process when necessary)
   */

#define __UTIL_LOCK_SPIN_LOCK_WAIT(x)                                 \
    {                                                                 \
        unsigned char try_lock_times = static_cast<unsigned char>(x); \
        if (try_lock_times < 4) {                                     \
        } else if (try_lock_times < 16) {                             \
            __UTIL_LOCK_SPIN_LOCK_PAUSE();                            \
        } else if (try_lock_times < 32) {                             \
            __UTIL_LOCK_SPIN_LOCK_THREAD_YIELD();                     \
        } else if (try_lock_times < 64) {                             \
            __UTIL_LOCK_SPIN_LOCK_CPU_YIELD();                        \
        } else {                                                      \
            __UTIL_LOCK_SPIN_LOCK_THREAD_SLEEP();                     \
        }                                                             \
    }

/**
* @brief 自旋锁类
* 
* NFSpinLock实现了高性能的自旋锁机制，基于原子操作提供线程间同步。
* 自旋锁适用于保护短时间的临界区，避免了线程上下文切换的开销。
* 
* 自旋锁特点：
* - 低延迟：不涉及系统调用和线程调度
* - 高效率：适用于短暂锁定的场景
* - CPU敏感：长时间持锁会浪费CPU资源
* - 非递归：同一线程不能重复获取锁
* 
* 适用场景：
* - 保护简单的共享数据结构
* - 临界区执行时间很短（微秒级别）
* - 多核系统下的高频访问同步
* - 对性能要求极高的场景
* 
* 不适用场景：
* - 临界区执行时间较长
* - 单核系统或CPU资源紧张
* - 可能发生阻塞IO的代码段
* 
* 实现原理：
* - 使用std::atomic<unsigned int>作为锁状态
* - 采用compare-and-swap（CAS）操作
* - 支持acquire-release内存顺序保证
* - 内置退避策略减少CPU竞争
* 
* 使用方法：
* @code
* NFSpinLock spinLock;
* 
* // 方式1：手动加锁解锁
* spinLock.Lock();
* try {
*     // 临界区代码
*     shared_data++;
* } catch (...) {
*     spinLock.UnLock();
*     throw;
* }
* spinLock.UnLock();
* 
* // 方式2：使用RAII（推荐）
* {
*     NFSpinLockHolder holder(spinLock);
*     // 临界区代码，自动解锁
*     shared_data++;
* }
* 
* // 方式3：尝试加锁
* if (spinLock.TryLock()) {
*     // 成功获取锁
*     shared_data++;
*     spinLock.UnLock();
* } else {
*     // 锁已被占用，执行其他逻辑
* }
* @endcode
* 
* @note 避免在持有自旋锁时执行可能阻塞的操作
* @note 临界区代码应该尽可能简短
* @warning 同一线程不能重复获取同一个自旋锁
*/
class NFSpinLock {
private:
	/** @brief 锁状态枚举：0表示未锁定，1表示已锁定 */
	typedef enum { UNLOCKED = 0, LOCKED = 1 } LockStateType;
	/** @brief 原子锁状态变量，确保线程安全 */
	std::atomic<unsigned int> mLockState;
public:
	/**
	 * @brief 默认构造函数
	 * 
	 * 初始化自旋锁为未锁定状态。
	 */
	NFSpinLock() { mLockState.store(UNLOCKED); }

	/**
	 * @brief 获取自旋锁
	 * 
	 * 尝试获取自旋锁，如果锁已被其他线程占用，则进入自旋等待。
	 * 使用内置的退避策略，在等待过程中逐步降低CPU使用率。
	 * 
	 * 退避策略：
	 * 1. 前4次尝试：忙等待
	 * 2. 4-16次：使用CPU暂停指令
	 * 3. 16-32次：线程让渡
	 * 4. 32-64次：CPU时间片让渡
	 * 5. 64次以上：线程睡眠1毫秒
	 * 
	 * @note 此方法会阻塞直到成功获取锁
	 * @note 使用acquire-release内存顺序保证内存可见性
	 * @warning 不要在持有锁的线程中再次调用此方法
	 */
	void Lock() 
	{
		unsigned char try_times = 0;
		while (mLockState.exchange(static_cast<unsigned int>(LOCKED), std::memory_order_acq_rel) == LOCKED)
			__UTIL_LOCK_SPIN_LOCK_WAIT(try_times++); /* busy-wait */
	}

	/**
	 * @brief 释放自旋锁
	 * 
	 * 释放当前线程持有的自旋锁，允许其他等待的线程获取锁。
	 * 使用release内存顺序确保临界区内的所有操作都完成。
	 * 
	 * @note 只有持有锁的线程才应该调用此方法
	 * @warning 释放未持有的锁会导致未定义行为
	 */
	void UnLock() 
	{ 
		mLockState.store(static_cast<unsigned int>(UNLOCKED), std::memory_order_release); 
	}

	/**
	 * @brief 检查锁是否被占用
	 * 
	 * 查询锁的当前状态，不会修改锁状态。
	 * 
	 * @return bool 如果锁被占用返回true，否则返回false
	 * 
	 * @note 此方法不保证返回结果的时效性
	 * @note 返回false不代表TryLock()一定会成功
	 */
	bool IsLocked() 
	{ 
		return mLockState.load(std::memory_order_acquire) == LOCKED;
	}

	/**
	 * @brief 尝试获取自旋锁（非阻塞）
	 * 
	 * 尝试获取自旋锁，如果锁当前可用则立即获取，
	 * 如果锁被占用则立即返回失败。
	 * 
	 * @return bool 成功获取锁返回true，锁被占用返回false
	 * 
	 * @note 此方法不会阻塞
	 * @note 返回true表示成功获取锁，需要配对调用UnLock()
	 * 
	 * 使用示例：
	 * @code
	 * if (spinLock.TryLock()) {
	 *     // 成功获取锁，执行临界区代码
	 *     critical_section();
	 *     spinLock.UnLock();
	 * } else {
	 *     // 锁被占用，执行替代逻辑
	 *     handle_lock_busy();
	 * }
	 * @endcode
	 */
	bool TryLock() 
	{
		return mLockState.exchange(static_cast<unsigned int>(LOCKED), std::memory_order_acq_rel) == UNLOCKED;
	}

	/**
	 * @brief 尝试释放自旋锁
	 * 
	 * 尝试释放自旋锁，如果当前锁被占用则释放并返回成功，
	 * 如果锁未被占用则返回失败。
	 * 
	 * @return bool 成功释放锁返回true，锁未被占用返回false
	 * 
	 * @note 此方法主要用于特殊场景，一般使用UnLock()
	 * @warning 只应该由持有锁的线程调用
	 */
	bool TryUnLock() {
		return mLockState.exchange(static_cast<unsigned int>(UNLOCKED), std::memory_order_acq_rel) == LOCKED;
	}
};

/**
 * @brief 自旋锁RAII管理器
 * 
 * NFSpinLockHolder提供了自旋锁的RAII（Resource Acquisition Is Initialization）
 * 管理，确保在任何情况下都能正确释放锁，包括异常退出的情况。
 * 
 * RAII的优势：
 * - 自动管理：构造时获取锁，析构时释放锁
 * - 异常安全：即使发生异常也能正确释放锁
 * - 简化代码：减少手动管理锁的复杂性
 * - 防止死锁：避免忘记释放锁导致的死锁
 * 
 * 使用场景：
 * - 需要在作用域内持有锁的所有情况
 * - 可能抛出异常的临界区代码
 * - 复杂的控制流场景
 * - 推荐的标准用法
 * 
 * 使用方法：
 * @code
 * NFSpinLock globalLock;
 * 
 * void protected_function() {
 *     NFSpinLockHolder holder(globalLock);  // 自动获取锁
 *     
 *     // 临界区代码
 *     if (some_condition) {
 *         throw std::runtime_error("error");  // 异常时也会自动释放锁
 *     }
 *     
 *     shared_resource.modify();
 *     
 * }  // 作用域结束，自动释放锁
 * 
 * // 嵌套作用域使用
 * {
 *     NFSpinLockHolder holder(globalLock);
 *     // 第一段临界区
 *     operation1();
 *     
 *     {
 *         // 内层作用域可以安全使用，但要注意重入问题
 *         operation2();
 *     }
 *     
 *     operation3();
 * }  // 锁在这里释放
 * @endcode
 * 
 * @note 不支持锁的转移和复制
 * @note 持有器的生命周期决定了锁的持有时间
 * @warning 避免长时间持有锁以防止性能问题
 */
class NFSpinLockHolder
{
public:
	/**
	 * @brief 构造函数，自动获取指定的自旋锁
	 * 
	 * 在构造时立即尝试获取传入的自旋锁，如果锁被占用则会阻塞等待。
	 * 
	 * @param lock 要管理的自旋锁引用
	 * 
	 * @note 构造函数会阻塞直到成功获取锁
	 * @note 传入的锁引用必须在持有器生命周期内有效
	 */
	NFSpinLockHolder(NFSpinLock& lock) :mLockFlag(&lock)
	{
		mLockFlag->Lock();
	}

	/**
	 * @brief 析构函数，自动释放管理的自旋锁
	 * 
	 * 在析构时自动释放在构造函数中获取的锁，确保锁总是被正确释放。
	 * 即使在异常情况下，析构函数也会被调用，保证锁的释放。
	 * 
	 * @note 析构函数不会抛出异常
	 * @note 确保在锁有效的情况下释放
	 */
	~NFSpinLockHolder()
	{
		mLockFlag->UnLock();
	}

	/**
	 * @brief 检查持有器是否有效
	 * 
	 * 检查持有器是否持有一个有效的锁指针。
	 * 
	 * @return bool 如果持有器有效返回true，否则返回false
	 * 
	 * @note 此方法主要用于调试和断言检查
	 * @note 正常使用情况下持有器总是有效的
	 */
	bool IsAvailable() const { return mLockFlag != NULL; }
private:
	/** @brief 指向被管理的自旋锁的指针 */
	NFSpinLock* mLockFlag;
};