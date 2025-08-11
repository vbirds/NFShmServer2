// -------------------------------------------------------------------------
//    @FileName         :    NFThreadUtils.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFThreadUtils.h
 * @brief 线程工具类
 * 
 * 此文件提供了各种线程操作的工具函数，包括线程让渡、退避算法、
 * 睡眠等功能。这些函数主要用于优化多线程程序的性能，特别是
 * 在无锁编程和自旋等待场景中。
 */

#pragma once

#include "NFPlatform.h"

#include <thread>

/**
 * @brief 静态线程工具函数接口类
 * 
 * NFThreadUtils提供了一系列静态方法来优化多线程程序的性能。
 * 主要功能包括：
 * 
 * - 智能退避算法：在自旋等待时避免浪费CPU资源
 * - 线程让渡：在不同层级上让出CPU执行权
 * - 睡眠控制：精确控制线程睡眠时间
 * 
 * 这些函数特别适用于：
 * - 无锁数据结构的实现
 * - 自旋锁的优化
 * - 高性能并发算法
 * 
 * 使用方法：
 * @code
 * uint32_t backoff = 0;
 * while (!tryLock()) {
 *     NFThreadUtils::Backoff(backoff);  // 智能退避
 * }
 * @endcode
 */
class _NFExport NFThreadUtils
{
public:

	/**
	 * @brief 智能退避算法
	 * 
	 * 在共享内存资源上避免忙等待，浪费一些CPU周期。
	 * 该函数实现了一个渐进式退避策略，从简单的CPU让渡逐步升级到线程睡眠。
	 * 
	 * 退避策略：
	 * - 第1-9次：让渡给同一核心的超线程
	 * - 第10-19次：连续让渡50次给超线程
	 * - 第20-21次：让渡给同一核心的其他线程
	 * - 第22次以上：让渡给任意其他线程
	 * 
	 * @param backoff [in,out] 退避计数器，函数会自动递增
	 * @note 此函数有意不强制内联，以保持调用开销
	 * @note 基于 http://www.1024cores.net/home/lock-free-algorithms/tricks/spinning 的实现
	 */
	inline static void Backoff(uint32_t &backoff);

	/**
	 * @brief 让渡给同一核心的超线程
	 * 
	 * 将当前线程的执行权让渡给同一CPU核心上的另一个超线程。
	 * 这是最轻量级的让渡操作，通常编译为一个轻量级的NOP指令
	 * 而不是重量级的系统调用。
	 * 
	 * 平台实现：
	 * - Windows: 使用YieldProcessor()函数
	 * - ARM: 使用NOP汇编指令
	 * - x86_64: 使用pause汇编指令
	 */
	inline static void YieldToHyperthread();

	/**
	 * @brief 让渡给同一核心的任意线程
	 * 
	 * 将当前线程的执行权让渡给同一CPU核心上的任意可用线程。
	 * 比让渡给超线程的范围更广，但仍限制在同一核心内。
	 */
	inline static void YieldToLocalThread();

	/**
	 * @brief 让渡给任意其他线程
	 * 
	 * 将当前线程的执行权让渡给系统中任意其他可用的线程。
	 * 这是最重量级的让渡操作，可能导致线程在不同核心间迁移。
	 */
	inline static void YieldToAnyThread();

	/**
	 * @brief 让调用线程睡眠指定毫秒数
	 * 
	 * 使当前线程进入睡眠状态指定的毫秒数。这是一个精确的睡眠控制，
	 * 适用于需要定时操作的场景。
	 * 
	 * @param milliseconds 睡眠时间（毫秒），必须小于1000
	 * @note 参数必须小于1000毫秒，否则会触发断言
	 */
	inline static void SleepThread(const uint32_t milliseconds);
private:

	/** @brief 禁用拷贝构造函数 */
	NFThreadUtils(const NFThreadUtils &other);
	/** @brief 禁用赋值操作符 */
	NFThreadUtils &operator=(const NFThreadUtils &other);
};


/**
 * @brief 智能退避算法实现
 * 
 * 实现了渐进式的退避策略，随着退避次数增加逐步升级退避强度。
 * 这种设计可以在短时间等待时保持响应性，在长时间等待时节省CPU资源。
 */
inline void NFThreadUtils::Backoff(uint32_t &backoff)
{
	// The backoff scales from a simple 'nop' to putting the calling thread to sleep.
	// This implementation is based roughly on http://www.1024cores.net/home/lock-free-algorithms/tricks/spinning
	if (++backoff < 10)
	{
		YieldToHyperthread();
	}
	else if (backoff < 20)
	{
		for (uint32_t i = 0; i < 50; ++i)
		{
			YieldToHyperthread();
		}
	}
	else if (backoff < 22)
	{
		YieldToLocalThread();
	}
	else
	{
		YieldToAnyThread();
	}
}


/**
 * @brief 超线程让渡实现
 * 
 * 向调度器表明当前线程没有进行有用的工作，因此同一核心上
 * 能够运行的其他线程应该被给予优先权。
 */
NF_FORCEINLINE void NFThreadUtils::YieldToHyperthread()
{

	// The intention of this code is to indicate to the scheduler
	// that the thread is not doing useful work, so another thread
	// able to run on the same core should be given priority.
	// It's intended to evaluate to a lightweight NOP instruction
	// rather than a heavyweight system call. On Windows the
	// YieldProcessor() function can be called to this effect;
	// on other systems we try to issue an explicit NOP or pause
	// instruction - but only some architectures are implemented.

#if NF_COMPILER == NF_COMPILER_MSVC

	YieldProcessor();

#elif NF_GCC

#ifdef __arm__
	__asm__ __volatile__("NOP");
#elif __X86_64__
	__asm__ __volatile__("pause");
#endif

#endif

}


/**
 * @brief 本地线程让渡实现
 * 
 * 使用标准库的thread::yield()来让渡执行权给同一核心的其他线程。
 */
NF_FORCEINLINE void NFThreadUtils::YieldToLocalThread()
{
	std::this_thread::yield();
}


/**
 * @brief 全局线程让渡实现
 * 
 * 使用标准库的thread::yield()来让渡执行权给系统中任意其他线程。
 */
NF_FORCEINLINE void NFThreadUtils::YieldToAnyThread()
{
	std::this_thread::yield();
}


/**
 * @brief 线程睡眠实现
 * 
 * 使当前线程睡眠指定的毫秒数，内部转换为微秒精度以提高准确性。
 */
NF_FORCEINLINE void NFThreadUtils::SleepThread(const uint32_t milliseconds)
{
	NF_ASSERT(milliseconds < 1000);

	std::this_thread::sleep_for(std::chrono::microseconds(milliseconds * 1000));
}