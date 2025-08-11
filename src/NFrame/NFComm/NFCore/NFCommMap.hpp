// -------------------------------------------------------------------------
//    @FileName         :    NFCommMap.hpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//
// -------------------------------------------------------------------------

/**
 * @file NFCommMap.hpp
 * @brief 通信映射容器模板
 * 
 * 此文件提供了一个基于原始指针的简单映射容器模板类。
 * 相比NFCommMapEx，这是一个更轻量的实现，不使用智能指针，
 * 提供基本的键值映射和迭代功能。适用于简单的对象管理场景。
 */

#pragma once

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <typeinfo>
#include "NFCommMapEx.hpp"

/**
 * @brief 通信映射容器模板类
 * 
 * NFCommMap提供了一个轻量级的映射容器实现，基于std::map存储键值对。
 * 与NFCommMapEx不同，此类使用原始指针而非智能指针，适用于简单的
 * 对象管理场景，性能开销更小。
 * 
 * 主要特性：
 * - 轻量设计：使用原始指针，内存开销小
 * - 快速查找：基于std::map实现O(log n)查找性能
 * - 迭代支持：提供First/Next方式的迭代访问
 * - 类型安全：使用模板确保类型安全
 * - 简单易用：接口简洁，易于理解和使用
 * 
 * 设计原理：
 * - 内部使用std::map<T, TD*>存储键值对
 * - 使用原始指针存储对象，不涉及生命周期管理
 * - 提供标准的增删查改操作
 * - 支持迭代器模式的遍历
 * 
 * 与NFCommMapEx的区别：
 * - 不使用智能指针，内存管理由调用者负责
 * - 更轻量，性能开销更小
 * - 不提供自动内存管理和引用计数
 * - 适用于简单的、生命周期明确的对象管理
 * 
 * 适用场景：
 * - 简单的对象注册和查找
 * - 临时对象的映射管理
 * - 性能敏感的轻量级容器需求
 * - 对象生命周期由外部明确管理的场景
 * - 不需要复杂内存管理的映射容器
 * 
 * 使用方法：
 * @code
 * NFCommMap<std::string, MyObject> container;
 * 
 * // 添加对象（对象生命周期由调用者管理）
 * MyObject* obj1 = new MyObject("data1");
 * MyObject* obj2 = new MyObject("data2");
 * container.AddElement("key1", obj1);
 * container.AddElement("key2", obj2);
 * 
 * // 查找对象
 * MyObject* found = container.GetElement("key1");
 * if (found) {
 *     found->DoSomething();
 * }
 * 
 * // 迭代访问
 * for (auto* obj = container.First(); obj != nullptr; obj = container.Next()) {
 *     obj->Process();
 * }
 * 
 * // 移除对象（返回原始指针，需要手动删除）
 * MyObject* removed = container.RemoveElement("key1");
 * delete removed;  // 调用者负责内存管理
 * 
 * // 清理所有对象
 * for (auto* obj = container.First(); obj != nullptr; obj = container.Next()) {
 *     delete obj;  // 手动删除每个对象
 * }
 * container.ClearAll();
 * @endcode
 * 
 * @tparam T 键类型，必须支持std::map的键类型要求（可比较）
 * @tparam TD 值类型，存储对象的类型
 * 
 * @warning 对象的生命周期完全由调用者管理，容器不会自动删除对象
 * @note 此实现不是线程安全的，多线程使用时需要外部同步
 * @note 相比NFCommMapEx，此类更轻量但需要手动管理内存
 */
template <typename T, typename TD>
class NFCommMap
{
public:
	/** @brief 内部映射容器类型定义，使用原始指针存储对象 */
	typedef std::map<T, TD*> NFMapOBJECT;

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个空的通信映射容器。
	 */
	NFCommMap()
	{
	};

	/**
	 * @brief 虚析构函数
	 * 
	 * 销毁容器，但不删除存储的对象。
	 * 
	 * @warning 调用者需要在销毁容器前手动管理所有对象的内存
	 * @note 容器销毁时不会自动删除存储的对象指针
	 */
	virtual ~NFCommMap()
	{
		//mObjectList.clear();
		//DeleteAllElement();
	};

	/**
	 * @brief 添加元素到容器
	 * 
	 * 将键值对添加到容器中。如果键已存在，则添加失败。
	 * 
	 * @param name 元素的键值
	 * @param data 要存储的对象指针，调用者需确保对象有效性
	 * @return bool 添加成功返回true，键已存在返回false
	 * 
	 * @pre data应为有效的对象指针（可以为nullptr，但不推荐）
	 * @post 成功添加后，容器大小增加1
	 * @note 对象的生命周期由调用者管理
	 */
	virtual bool AddElement(const T& name, TD* data)
	{
		typename NFMapOBJECT::iterator itr = mObjectMap.find(name);
		if (itr == mObjectMap.end())
		{
			mObjectMap.insert(typename NFMapOBJECT::value_type(name, data));
			// mObjectList[name] = data;
			return true;
		}

		return false;
	}

	/**
	 * @brief 从容器中移除元素
	 * 
	 * 根据键值移除元素，返回被移除的对象指针。
	 * 
	 * @param name 要移除的元素的键值
	 * @return TD* 被移除的对象指针，如果键不存在返回nullptr
	 * 
	 * @note 此方法只从容器中移除元素，不删除对象本身
	 * @note 调用者需要负责返回指针指向对象的内存管理
	 * @post 成功移除后，容器大小减少1
	 */
	virtual TD* RemoveElement(const T& name)
	{
		TD* pData = nullptr;
		typename NFMapOBJECT::iterator itr = mObjectMap.find(name);
		if (itr != mObjectMap.end())
		{
			pData = itr->second;
			mObjectMap.erase(itr);
		}

		return pData;
	}

	/**
	 * @brief 根据键值获取元素
	 * 
	 * 通过键值快速查找对应的元素。
	 * 
	 * @param name 要查找的元素的键值
	 * @return TD* 找到的对象指针，如果键不存在返回nullptr
	 * 
	 * @note 返回的指针仍由调用者管理，不要在容器外删除正在使用的对象
	 */
	virtual TD* GetElement(const T& name)
	{
		typename NFMapOBJECT::iterator itr = mObjectMap.find(name);
		if (itr != mObjectMap.end())
		{
			return itr->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取第一个元素并重置迭代器
	 * 
	 * 将内部迭代器重置到第一个元素，并返回该元素的对象指针。
	 * 
	 * @return TD* 第一个元素的对象指针，容器为空时返回nullptr
	 * 
	 * @note 此方法会影响内部迭代器状态，与Next()方法配合使用
	 */
	virtual TD* First()
	{
		if (mObjectMap.size() <= 0)
		{
			return nullptr;
		}

		mObjectCurIter = mObjectMap.begin();
		if (mObjectCurIter != mObjectMap.end())
		{
			return mObjectCurIter->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取下一个元素
	 * 
	 * 将内部迭代器移动到下一个元素，并返回该元素的对象指针。
	 * 
	 * @return TD* 下一个元素的对象指针，已到末尾时返回nullptr
	 * 
	 * @note 必须先调用First()初始化迭代器
	 * @note 此方法与First()配合使用进行完整遍历
	 */
	virtual TD* Next()
	{
		if (mObjectCurIter == mObjectMap.end())
		{
			return nullptr;
		}

		++mObjectCurIter;
		if (mObjectCurIter != mObjectMap.end())
		{
			return mObjectCurIter->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取第一个元素及其键值
	 * 
	 * 将内部迭代器重置到第一个元素，返回该元素的对象指针并输出键值。
	 * 
	 * @param name [out] 输出第一个元素的键值
	 * @return TD* 第一个元素的对象指针，容器为空时返回nullptr
	 * 
	 * @note 当返回nullptr时，name参数的值未定义
	 */
	virtual TD* First(T& name)
	{
		if (mObjectMap.size() <= 0)
		{
			return nullptr;
		}

		mObjectCurIter = mObjectMap.begin();
		if (mObjectCurIter != mObjectMap.end())
		{
			name = mObjectCurIter->first;
			return mObjectCurIter->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取下一个元素及其键值
	 * 
	 * 将内部迭代器移动到下一个元素，返回该元素的对象指针并输出键值。
	 * 
	 * @param name [out] 输出下一个元素的键值
	 * @return TD* 下一个元素的对象指针，已到末尾时返回nullptr
	 * 
	 * @note 当返回nullptr时，name参数的值未定义
	 */
	virtual TD* Next(T& name)
	{
		if (mObjectCurIter == mObjectMap.end())
		{
			return nullptr;
		}

		++mObjectCurIter;
		if (mObjectCurIter != mObjectMap.end())
		{
			name = mObjectCurIter->first;
			return mObjectCurIter->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取容器中元素的数量
	 * 
	 * 返回容器中当前存储的元素数量。
	 * 
	 * @return int 元素数量
	 */
	int Count()
	{
		return mObjectMap.size();
	}

	/**
	 * @brief 清空容器
	 * 
	 * 移除容器中的所有元素，但不删除对象本身。
	 * 
	 * @return bool 始终返回true
	 * 
	 * @warning 调用者需要在清空前手动管理所有对象的内存
	 * @post 容器大小变为0
	 * @post 内部迭代器状态被重置
	 * 
	 * 使用示例：
	 * @code
	 * // 先释放所有对象的内存
	 * for (auto* obj = container.First(); obj != nullptr; obj = container.Next()) {
	 *     delete obj;
	 * }
	 * // 再清空容器
	 * container.ClearAll();
	 * @endcode
	 */
	bool ClearAll()
	{
		mObjectMap.clear();
		return true;
	}

private:
	/** @brief 内部映射容器，用于存储键值对 */
	NFMapOBJECT mObjectMap;
	/** @brief 当前迭代器位置，用于First/Next遍历 */
	typename NFMapOBJECT::iterator mObjectCurIter;
};

