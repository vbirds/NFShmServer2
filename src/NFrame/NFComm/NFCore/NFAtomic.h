// -------------------------------------------------------------------------
//    @FileName         :    NFAtomic.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFAtomic.h
 * @brief 原子操作模板类
 * 
 * 此文件定义了基于std::atomic的原子操作模板类，提供了线程安全的
 * 原子操作封装。支持各种内存序列模型，适用于无锁编程和多线程同步。
 */

#pragma once

#include "NFPlatform.h"
#include <atomic>


/**
 * @brief 原子操作模板类
 * 
 * NFAtomic是基于std::atomic的模板类封装，提供了类型安全的原子操作接口。
 * 支持各种基本数据类型的原子操作，包括加载、存储、交换、比较交换等。
 * 
 * 主要特性：
 * - 线程安全：所有操作都是原子的，不需要额外的同步机制
 * - 内存序列：支持多种内存序列模型（sequential consistency、acquire、release等）
 * - 类型安全：通过模板提供类型安全的接口
 * - 高性能：直接基于硬件原子指令实现
 * 
 * 适用场景：
 * - 无锁数据结构
 * - 计数器和统计
 * - 状态标志
 * - 多线程协调
 * 
 * 使用方法：
 * @code
 * NFAtomic<int> counter(0);
 * 
 * // 原子递增
 * counter++;
 * 
 * // 原子加载
 * int value = counter.load();
 * 
 * // 原子存储
 * counter.store(100);
 * 
 * // 原子交换
 * int old_value = counter.exchange(200);
 * 
 * // 原子比较交换
 * int expected = 200;
 * bool success = counter.compare_exchange_strong(expected, 300);
 * @endcode
 * 
 * @tparam Ty 原子变量的数据类型，默认为int
 * @note 此类禁用拷贝构造和赋值操作，确保原子性
 */
template <typename Ty = int>
class NFAtomic
{
public:
	/** @brief 值类型定义 */
	typedef Ty value_type;
private:
	/** @brief 底层的std::atomic对象 */
	::std::atomic<value_type> data_;
	/** @brief 禁用拷贝构造函数 */
	NFAtomic(const NFAtomic &);
#ifndef _MSC_VER
	/** @brief 禁用赋值操作符（非MSVC编译器） */
	NFAtomic &operator=(const NFAtomic &);
	/** @brief 禁用volatile赋值操作符（非MSVC编译器） */
	NFAtomic &operator=(const NFAtomic &) volatile;
#endif

public:
	/**
	 * @brief 默认构造函数
	 * 创建一个值初始化的原子变量
	 */
	NFAtomic() noexcept  : data_() {}
	
	/**
	 * @brief 值构造函数
	 * 创建一个具有指定初始值的原子变量
	 * @param desired 初始值
	 */
	NFAtomic(value_type desired) noexcept : data_(desired) {}

	/**
	 * @brief 原子存储操作
	 * 
	 * 原子地将指定值存储到原子变量中
	 * 
	 * @param desired 要存储的值
	 * @param order 内存序列模型，默认为sequential consistency
	 */
	inline void store(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept
    {
		data_.store(desired, order);
	}

	/**
	 * @brief 原子存储操作（volatile版本）
	 * 
	 * @param desired 要存储的值
	 * @param order 内存序列模型，默认为sequential consistency
	 */
	inline void store(value_type desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		data_.store(desired, order);
	}

	/**
	 * @brief 原子加载操作
	 * 
	 * 原子地从原子变量中加载值
	 * 
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 加载的值
	 */
	inline value_type load(std::memory_order order = std::memory_order_seq_cst) const  noexcept
	{
		return data_.load(order);
	}

	/**
	 * @brief 原子加载操作（volatile版本）
	 * 
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 加载的值
	 */
	inline value_type load(std::memory_order order = std::memory_order_seq_cst) const volatile noexcept
	{
		return data_.load(order);
	}

	/**
	 * @brief 隐式转换操作符
	 * 自动转换为值类型，相当于调用load()
	 * @return value_type 当前存储的值
	 */
	inline operator value_type() const  noexcept { return load(); }
	
	/**
	 * @brief 隐式转换操作符（volatile版本）
	 * @return value_type 当前存储的值
	 */
	inline operator value_type() const volatile  noexcept { return load(); }

	/**
	 * @brief 赋值操作符
	 * 相当于调用store(desired)
	 * @param desired 要赋值的值
	 * @return value_type 赋值的值
	 */
	inline value_type operator=(value_type desired) noexcept
	{
		store(desired);
		return desired;
	}

	/**
	 * @brief 赋值操作符（volatile版本）
	 * @param desired 要赋值的值
	 * @return value_type 赋值的值
	 */
	inline value_type operator=(value_type desired) volatile noexcept
	{
		store(desired);
		return desired;
	}

	/**
	 * @brief 前置自增操作符
	 * 原子地递增值并返回新值
	 * @return value_type 递增后的值
	 */
	inline value_type operator++()  noexcept { return ++data_; }
	
	/**
	 * @brief 前置自增操作符（volatile版本）
	 * @return value_type 递增后的值
	 */
	inline value_type operator++() volatile  noexcept { return ++data_; }
	
	/**
	 * @brief 后置自增操作符
	 * 原子地递增值并返回旧值
	 * @return value_type 递增前的值
	 */
	inline value_type operator++(int)  noexcept { return data_++; }
	
	/**
	 * @brief 后置自增操作符（volatile版本）
	 * @return value_type 递增前的值
	 */
	inline value_type operator++(int) volatile  noexcept { return data_++; }
	
	/**
	 * @brief 前置自减操作符
	 * 原子地递减值并返回新值
	 * @return value_type 递减后的值
	 */
	inline value_type operator--()  noexcept { return --data_; }
	
	/**
	 * @brief 前置自减操作符（volatile版本）
	 * @return value_type 递减后的值
	 */
	inline value_type operator--() volatile  noexcept { return --data_; }
	
	/**
	 * @brief 后置自减操作符
	 * 原子地递减值并返回旧值
	 * @return value_type 递减前的值
	 */
	inline value_type operator--(int)  noexcept { return data_--; }
	
	/**
	 * @brief 后置自减操作符（volatile版本）
	 * @return value_type 递减前的值
	 */
	inline value_type operator--(int) volatile  noexcept { return data_--; }

	/**
	 * @brief 原子交换操作
	 * 
	 * 原子地将新值存储到原子变量中，并返回旧值
	 * 
	 * @param desired 要存储的新值
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 交换前的旧值
	 */
	inline value_type exchange(value_type desired, std::memory_order order = std::memory_order_seq_cst)  noexcept
	{
		return data_.exchange(desired, order);
	}

	/**
	 * @brief 原子交换操作（volatile版本）
	 * 
	 * @param desired 要存储的新值
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 交换前的旧值
	 */
	inline value_type exchange(value_type desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.exchange(desired, order);
	}

	/**
	 * @brief 弱比较交换操作
	 * 
	 * 原子地比较原子变量的值与expected，如果相等则将desired存储到原子变量中。
	 * 弱版本允许虚假失败，即使值相等也可能失败，但性能更好。
	 * 
	 * @param expected [in,out] 期望的值，如果比较失败会被更新为实际值
	 * @param desired 要存储的新值
	 * @param success 成功时的内存序列模型
	 * @param failure 失败时的内存序列模型
	 * @return bool 如果交换成功返回true，否则返回false
	 */
	inline bool compare_exchange_weak(value_type &expected, value_type desired, std::memory_order success, std::memory_order failure) noexcept
	{
		return data_.compare_exchange_weak(expected, desired, success, failure);
	}

	/**
	 * @brief 弱比较交换操作（volatile版本）
	 */
	inline bool compare_exchange_weak(value_type &expected, value_type desired, std::memory_order success, std::memory_order failure) volatile noexcept
	{
		return data_.compare_exchange_weak(expected, desired, success, failure);
	}

	/**
	 * @brief 弱比较交换操作（简化版本）
	 * 
	 * @param expected [in,out] 期望的值
	 * @param desired 要存储的新值
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return bool 如果交换成功返回true，否则返回false
	 */
	inline bool compare_exchange_weak(value_type &expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return data_.compare_exchange_weak(expected, desired, order);
	}

	/**
	 * @brief 弱比较交换操作（简化版本，volatile）
	 */
	inline bool compare_exchange_weak(value_type &expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.compare_exchange_weak(expected, desired, order);
	}

	/**
	 * @brief 强比较交换操作
	 * 
	 * 原子地比较原子变量的值与expected，如果相等则将desired存储到原子变量中。
	 * 强版本不允许虚假失败，只有在值真正不相等时才会失败。
	 * 
	 * @param expected [in,out] 期望的值，如果比较失败会被更新为实际值
	 * @param desired 要存储的新值
	 * @param success 成功时的内存序列模型
	 * @param failure 失败时的内存序列模型
	 * @return bool 如果交换成功返回true，否则返回false
	 */
	inline bool compare_exchange_strong(value_type &expected, value_type desired, std::memory_order success, std::memory_order failure) noexcept
	{
		return data_.compare_exchange_strong(expected, desired, success, failure);
	}

	/**
	 * @brief 强比较交换操作（volatile版本）
	 */
	inline bool compare_exchange_strong(value_type &expected, value_type desired, std::memory_order success, std::memory_order failure) volatile noexcept
	{
		return data_.compare_exchange_strong(expected, desired, success, failure);
	}

	/**
	 * @brief 强比较交换操作（简化版本）
	 * 
	 * @param expected [in,out] 期望的值
	 * @param desired 要存储的新值
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return bool 如果交换成功返回true，否则返回false
	 */
	inline bool compare_exchange_strong(value_type &expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return data_.compare_exchange_strong(expected, desired, order);
	}

	/**
	 * @brief 强比较交换操作（简化版本，volatile）
	 */
	inline bool compare_exchange_strong(value_type &expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.compare_exchange_strong(expected, desired, order);
	}

	/**
	 * @brief 原子获取并加法操作
	 * 
	 * 原子地将arg加到原子变量的值上，并返回操作前的值
	 * 
	 * @param arg 要添加的值
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 加法操作前的值
	 */
	inline value_type fetch_add(value_type arg, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return data_.fetch_add(arg, order);
	}

	/**
	 * @brief 原子获取并加法操作（volatile版本）
	 */
	inline value_type fetch_add(value_type arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.fetch_add(arg, order);
	}

	/**
	 * @brief 原子获取并减法操作
	 * 
	 * 原子地从原子变量的值中减去arg，并返回操作前的值
	 * 
	 * @param arg 要减去的值
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 减法操作前的值
	 */
	inline value_type fetch_sub(value_type arg, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return data_.fetch_sub(arg, order);
	}

	/**
	 * @brief 原子获取并减法操作（volatile版本）
	 */
	inline value_type fetch_sub(value_type arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.fetch_sub(arg, order);
	}

	/**
	 * @brief 原子获取并按位与操作
	 * 
	 * 原子地对原子变量执行按位与操作，并返回操作前的值
	 * 
	 * @param arg 按位与操作的操作数
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 按位与操作前的值
	 */
	inline value_type fetch_and(value_type arg, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return data_.fetch_and(arg, order);
	}

	/**
	 * @brief 原子获取并按位与操作（volatile版本）
	 */
	inline value_type fetch_and(value_type arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.fetch_and(arg, order);
	}

	/**
	 * @brief 原子获取并按位或操作
	 * 
	 * 原子地对原子变量执行按位或操作，并返回操作前的值
	 * 
	 * @param arg 按位或操作的操作数
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 按位或操作前的值
	 */
	inline value_type fetch_or(value_type arg, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return data_.fetch_or(arg, order);
	}

	/**
	 * @brief 原子获取并按位或操作（volatile版本）
	 */
	inline value_type fetch_or(value_type arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.fetch_or(arg, order);
	}

	/**
	 * @brief 原子获取并按位异或操作
	 * 
	 * 原子地对原子变量执行按位异或操作，并返回操作前的值
	 * 
	 * @param arg 按位异或操作的操作数
	 * @param order 内存序列模型，默认为sequential consistency
	 * @return value_type 按位异或操作前的值
	 */
	inline value_type fetch_xor(value_type arg, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return data_.fetch_xor(arg, order);
	}

	/**
	 * @brief 原子获取并按位异或操作（volatile版本）
	 */
	inline value_type fetch_xor(value_type arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
	{
		return data_.fetch_xor(arg, order);
	}
};
