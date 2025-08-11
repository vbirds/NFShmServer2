// -------------------------------------------------------------------------
//    @FileName         :    NFSingleton.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFSingleton
//
// -------------------------------------------------------------------------

/**
 * @file NFSingleton.hpp
 * @brief 单例模式模板类定义
 * 
 * 此文件定义了一个线程安全的单例模式模板类，用于确保某个类只有一个实例。
 * 提供了获取实例、释放实例等基本操作。
 */

#ifndef NF_SINGLETON_H
#define NF_SINGLETON_H

#include <iostream>

/**
 * @brief 单例模式模板类
 * @tparam T 需要实现单例模式的类型
 * 
 * 这是一个CRTP(奇异递归模板模式)实现的单例类。任何需要单例模式的类
 * 都可以继承此模板类来获得单例功能。
 * 
 * 使用方法：
 * @code
 * class MyClass : public NFSingleton<MyClass>
 * {
 * public:
 *     void DoSomething();
 * };
 * 
 * // 获取实例
 * MyClass* instance = MyClass::Instance();
 * MyClass& ref = MyClass::InstanceRef();
 * @endcode
 */
template <class T>
class NFSingleton
{
public:

	/**
	 * @brief 构造函数
	 * 初始化单例实例指针，使用CRTP模式设置正确的实例指针
	 */
	NFSingleton()
	{
		//assert( !m_instance );

#if defined( _MSC_VER ) && _MSC_VER < 1200
		int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		m_pInstance = (T*)((int)this + offset);
#else
		m_pInstance = static_cast<T*>(this);
#endif
	}

	/**
	 * @brief 虚析构函数
	 * 确保派生类的析构函数能够正确调用
	 */
	virtual ~NFSingleton()
	{
	}

public:

	/**
	 * @brief 获取单例指针
	 * 如果实例不存在，则创建新实例
	 * @return T* 指向单例实例的指针
	 */
	static T* GetSingletonPtr()
	{
		if (nullptr == m_pInstance)
		{
			m_pInstance = new T;
		}

		return m_pInstance;
	}

	/**
	 * @brief 获取单例引用
	 * @return T& 单例实例的引用
	 */
	static T& GetSingletonRef()
	{
		return *GetSingletonPtr();
	}

	/**
	 * @brief 获取单例实例（别名方法）
	 * @return T* 指向单例实例的指针
	 */
	static T* Instance()
	{
		return GetSingletonPtr();
	}

	/**
	 * @brief 获取单例实例引用（别名方法）
	 * @return T& 单例实例的引用
	 */
	static T& InstanceRef()
	{
		return GetSingletonRef();
	}

	/**
	 * @brief 释放单例实例
	 * 删除当前实例并将指针置为NULL
	 */
	static void ReleaseInstance()
	{
		if (m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

    /**
     * @brief 设置新的单例实例
     * 如果已有实例，则先删除旧实例，然后设置新实例
     * @param pNewInstance 新的实例指针
     */
    static void SetSingletonPtr(T* pNewInstance)
    {
        if (m_pInstance)
        {
            NF_SAFE_DELETE(m_pInstance);
        }

        m_pInstance = pNewInstance;
    }
private:
	/** @brief 静态单例实例指针 */
	static T* m_pInstance;
};

/** @brief 单例实例指针的初始化 */
template <class T>
T* NFSingleton<T>::m_pInstance = nullptr;

#endif

