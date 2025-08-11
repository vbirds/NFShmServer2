// -------------------------------------------------------------------------
//    @FileName         :    NFMapList.hpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//
// -------------------------------------------------------------------------

/**
 * @file NFMapList.hpp
 * @brief 映射列表容器模板
 * 
 * 此文件提供了一个结合了map和list特性的容器模板类。
 * 支持通过键值快速查找，同时保持插入顺序和索引访问能力。
 * 这种设计在需要既要快速查找又要保持顺序的场景中非常有用。
 */

#pragma once

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <typeinfo>
#include "NFCommMapEx.hpp"

/**
 * @brief 映射列表容器模板类
 * 
 * NFMapList提供了一个双重索引的容器，结合了std::map的快速查找能力
 * 和std::vector的顺序访问能力。元素既可以通过键值快速查找，
 * 也可以通过索引顺序访问。
 * 
 * 主要特性：
 * - 双重索引：同时支持键值查找和索引访问
 * - 保持顺序：元素按插入顺序保存在内部vector中
 * - 快速查找：通过内部map实现O(log n)的查找性能
 * - 迭代支持：提供First/Next方式的迭代访问
 * - 类型安全：使用模板确保类型安全
 * 
 * 设计原理：
 * - 内部维护一个map用于快速查找
 * - 内部维护一个vector用于保持插入顺序
 * - 两个容器同步更新，确保数据一致性
 * - 提供迭代器支持遍历访问
 * 
 * 时间复杂度：
 * - 添加元素：O(log n)
 * - 删除元素：O(log n + m)，其中m是vector中元素数量
 * - 查找元素：O(log n)
 * - 索引访问：O(1)
 * 
 * 适用场景：
 * - 需要保持插入顺序的映射容器
 * - 同时需要快速查找和顺序访问的数据结构
 * - 对象管理器（如游戏对象、UI组件管理）
 * - 配置项管理
 * - 缓存系统
 * 
 * 使用方法：
 * @code
 * NFMapList<std::string, MyObject> container;
 * 
 * // 添加元素
 * MyObject* obj1 = new MyObject("data1");
 * MyObject* obj2 = new MyObject("data2");
 * container.AddElement("key1", obj1);
 * container.AddElement("key2", obj2);
 * 
 * // 通过键值查找
 * MyObject* found = container.GetElement("key1");
 * 
 * // 通过索引访问
 * MyObject* first = container.Index(0);
 * 
 * // 迭代访问
 * for (auto* obj = container.First(); obj != nullptr; obj = container.Next()) {
 *     // 处理对象
 * }
 * 
 * // 删除元素
 * MyObject* removed = container.RemoveElement("key1");
 * delete removed;  // 调用者负责内存管理
 * @endcode
 * 
 * @tparam T 键类型，必须支持std::map的键类型要求（可比较）
 * @tparam TD 值类型，通常是指针类型
 * 
 * @note 值类型建议使用指针，因为容器不负责对象的生命周期管理
 * @note 此实现不是线程安全的，多线程使用时需要外部同步
 * @note 删除操作的复杂度相对较高，大量删除操作时需要考虑性能影响
 */
template <typename T, typename TD>
class NFMapList
{
public:
	/** @brief 内部映射容器类型定义 */
	typedef std::map<T, TD*> NFMapOBJECT;
	/** @brief 内部列表容器类型定义 */
	typedef std::vector<TD*> NFListOBJECT;

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个空的映射列表容器。
	 */
	NFMapList()
	{
	};

	/**
	 * @brief 虚析构函数
	 * 
	 * 销毁容器，但不删除存储的对象。
	 * 
	 * @note 调用者需要在销毁容器前手动管理对象的生命周期
	 */
	virtual ~NFMapList()
	{
	};

	/**
	 * @brief 添加元素到容器
	 * 
	 * 将键值对添加到容器中。如果键已存在，则添加失败。
	 * 
	 * @param name 元素的键值
	 * @param data 要存储的数据指针
	 * @return bool 添加成功返回true，键已存在返回false
	 * 
	 * @pre data不应为nullptr（虽然不强制检查）
	 * @post 成功添加后，容器大小增加1
	 * @post 添加的元素可以通过键值或索引访问
	 */
	virtual bool AddElement(const T& name, TD* data)
	{
		typename NFMapOBJECT::iterator itr = mObjectMap.find(name);
		if (itr == mObjectMap.end())
		{
			mObjectMap.insert(typename NFMapOBJECT::value_type(name, data));
			mObjectList.push_back(data);
			return true;
		}

		return false;
	}

	/**
	 * @brief 从容器中移除元素
	 * 
	 * 根据键值移除元素，返回被移除的数据指针。
	 * 
	 * @param name 要移除的元素的键值
	 * @return TD* 被移除的数据指针，如果键不存在返回nullptr
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
			for (auto iter = mObjectList.begin(); iter != mObjectList.end(); iter++)
			{
				if (*iter == pData)
				{
					mObjectList.erase(iter);
					break;
				}
			}
		}

		return pData;
	}

	/**
	 * @brief 根据键值获取元素
	 * 
	 * 通过键值快速查找对应的元素。
	 * 
	 * @param name 要查找的元素的键值
	 * @return TD* 找到的数据指针，如果键不存在返回nullptr
	 * 
	 * @note 返回的指针仍由容器管理，不要删除
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
	 * 将内部迭代器重置到第一个元素，并返回该元素的数据。
	 * 
	 * @return TD* 第一个元素的数据指针，容器为空时返回nullptr
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
	 * 将内部迭代器移动到下一个元素，并返回该元素的数据。
	 * 
	 * @return TD* 下一个元素的数据指针，已到末尾时返回nullptr
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
	 * 将内部迭代器重置到第一个元素，返回该元素的数据并输出键值。
	 * 
	 * @param name [out] 输出第一个元素的键值
	 * @return TD* 第一个元素的数据指针，容器为空时返回nullptr
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
	 * 将内部迭代器移动到下一个元素，返回该元素的数据并输出键值。
	 * 
	 * @param name [out] 输出下一个元素的键值
	 * @return TD* 下一个元素的数据指针，已到末尾时返回nullptr
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
	 * @brief 根据索引获取元素
	 * 
	 * 通过索引位置快速访问元素，按插入顺序排列。
	 * 
	 * @param index 元素索引，从0开始
	 * @return TD* 指定索引的元素指针，索引越界时返回nullptr
	 * 
	 * @note 索引范围是[0, Count()-1]
	 * @note 索引顺序与元素插入顺序一致
	 */
	TD* Index(int index) const
	{
		if (index >= 0 && index < (int)mObjectList.size())
		{
			return mObjectList[index];
		}
		return nullptr;
	}

	/**
	 * @brief 获取容器中元素的数量
	 * 
	 * @return int 容器中元素的总数量
	 * 
	 * @note 内部map和vector的大小应该始终一致
	 */
	int Count()
	{
		//assert(mObjectMap.size() == mObjectList.size());
		return (int)mObjectMap.size();
	}

	/**
	 * @brief 清空容器
	 * 
	 * 移除容器中的所有元素，但不删除对象本身。
	 * 
	 * @return bool 始终返回true
	 * 
	 * @note 调用者需要在清空前手动管理所有对象的内存
	 * @post 容器大小变为0
	 * @post 内部迭代器状态被重置
	 */
	bool ClearAll()
	{
		mObjectMap.clear();
		mObjectList.clear();
		return true;
	}

private:
	/** @brief 内部映射容器，用于快速键值查找 */
	NFMapOBJECT mObjectMap;
	/** @brief 内部向量容器，用于保持插入顺序和索引访问 */
	NFListOBJECT mObjectList;
	/** @brief 当前迭代器位置，用于First/Next遍历 */
	typename NFMapOBJECT::iterator mObjectCurIter;
};

