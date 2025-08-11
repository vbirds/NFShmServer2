// -------------------------------------------------------------------------
//    @FileName         :    NFNonCopyable.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFNonCopyable.h
 * @brief 不可拷贝基类定义
 * 
 * 此文件定义了一个用于禁止拷贝和赋值操作的基类。通过继承此类，
 * 派生类将自动禁用拷贝构造函数和赋值操作符，确保对象的唯一性。
 */

#pragma once

#include "NFPlatform.h"

/**
 * @brief 不可拷贝基类
 * 
 * NFNonCopyable是一个设计用于禁止拷贝和赋值操作的基类。
 * 任何继承自此类的派生类都将自动禁用拷贝构造函数和赋值操作符。
 * 
 * 设计模式：
 * - 采用私有继承或public继承都可以
 * - 构造函数和析构函数为protected，允许派生类访问
 * - 拷贝构造函数和赋值操作符为private且未实现，禁止拷贝
 * 
 * 适用场景：
 * - 资源管理类（如锁、文件句柄、网络连接等）
 * - 单例模式实现
 * - 唯一性要求的类（如ID生成器）
 * - RAII模式的资源包装类
 * 
 * 使用方法：
 * @code
 * class MyUniqueClass : public NFNonCopyable
 * {
 * public:
 *     MyUniqueClass() { /* 构造逻辑 */ }
 *     ~MyUniqueClass() { /* 析构逻辑 */ }
 *     
 *     void DoSomething() { /* 业务逻辑 */ }
 * };
 * 
 * // 使用示例
 * MyUniqueClass obj;           // OK：默认构造
 * // MyUniqueClass obj2(obj);  // 编译错误：拷贝构造被禁用
 * // MyUniqueClass obj3 = obj; // 编译错误：赋值操作被禁用
 * @endcode
 * 
 * @note 此类遵循了"私有化并且不实现"的C++习惯用法来禁止拷贝
 * @note 在C++11及以后版本中，也可以使用 = delete 来禁止拷贝
 */
class NFNonCopyable
{
protected:
	/**
	 * @brief 受保护的默认构造函数
	 * 
	 * 设置为protected确保只有派生类可以调用，防止直接实例化基类。
	 * 允许派生类进行正常的构造。
	 */
	NFNonCopyable()
	{
	}

	/**
	 * @brief 受保护的析构函数
	 * 
	 * 设置为protected确保只有派生类可以调用。
	 * 允许派生类进行正常的析构。
	 * 
	 * @note 析构函数不是虚函数，因为此类不intended作为多态基类使用
	 */
	~NFNonCopyable()
	{
	}

private:
	/**
	 * @brief 私有的拷贝构造函数（声明但不实现）
	 * 
	 * 通过将拷贝构造函数声明为private且不提供实现，
	 * 任何尝试拷贝继承此类的对象都会导致编译错误。
	 * 
	 * @param other 其他NFNonCopyable对象的引用
	 */
	NFNonCopyable(const NFNonCopyable&);
	
	/**
	 * @brief 私有的赋值操作符（声明但不实现）
	 * 
	 * 通过将赋值操作符声明为private且不提供实现，
	 * 任何尝试赋值继承此类的对象都会导致编译错误。
	 * 
	 * @param other 其他NFNonCopyable对象的引用
	 * @return const NFNonCopyable& 返回自身引用
	 */
	const NFNonCopyable& operator=(const NFNonCopyable&);
};

