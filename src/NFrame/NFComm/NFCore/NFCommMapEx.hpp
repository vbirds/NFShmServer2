// -------------------------------------------------------------------------
//    @FileName         :    NFCommMapEx.hpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//
// -------------------------------------------------------------------------

/**
 * @file NFCommMapEx.hpp
 * @brief 扩展通信映射容器模板
 * 
 * 此文件提供了一个基于智能指针的映射容器模板类。
 * 通过使用智能指针自动管理对象的生命周期，提供更安全的对象存储和访问机制。
 * 适用于需要自动内存管理的对象容器场景。
 */

#pragma once

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <typeinfo>
#include <memory>
#include "NFPlatform.h"
#include "NFConsistentHash.hpp"

/**
 * @brief 扩展通信映射容器模板类
 * 
 * NFCommMapEx提供了基于智能指针的映射容器实现，结合了std::map的快速查找
 * 和智能指针的自动内存管理功能。所有存储的对象都通过智能指针管理，
 * 确保内存安全和异常安全。
 * 
 * 主要特性：
 * - 智能指针管理：使用NF_SHARE_PTR自动管理对象生命周期
 * - 类型安全：模板实现确保类型安全
 * - 异常安全：智能指针提供异常安全保证
 * - 引用计数：支持对象的共享访问
 * - 快速查找：基于std::map实现O(log n)查找性能
 * 
 * 设计原理：
 * - 内部使用std::map<T, NF_SHARE_PTR<TD>>存储键值对
 * - 智能指针自动处理对象的创建和销毁
 * - 支持对象的安全共享访问
 * - 提供原始指针和智能指针两种访问方式
 * 
 * 适用场景：
 * - 需要自动内存管理的对象容器
 * - 多线程环境下的安全对象共享
 * - 复杂对象的生命周期管理
 * - 避免内存泄漏的高级容器
 * - 组件管理系统
 * 
 * 使用方法：
 * @code
 * NFCommMapEx<std::string, MyComponent> container;
 * 
 * // 创建并添加对象
 * auto component = NF_SHARE_PTR<MyComponent>(new MyComponent());
 * container.AddElement("comp1", component);
 * 
 * // 检查对象是否存在
 * if (container.ExistElement("comp1")) {
 *     // 获取智能指针
 *     auto ptr = container.GetElement("comp1");
 *     if (ptr) {
 *         ptr->DoSomething();
 *     }
 *     
 *     // 或获取原始指针
 *     MyComponent* rawPtr = container.GetElementNude("comp1");
 *     if (rawPtr) {
 *         rawPtr->DoSomething();
 *     }
 * }
 * 
 * // 遍历所有对象
 * for (auto ptr = container.First(); ptr != nullptr; ptr = container.Next()) {
 *     ptr->Process();
 * }
 * 
 * // 移除对象（自动释放内存）
 * container.RemoveElement("comp1");
 * @endcode
 * 
 * @tparam T 键类型，必须支持std::map的键类型要求（可比较）
 * @tparam TD 值类型，存储对象的类型
 * 
 * @note 对象通过智能指针自动管理，无需手动删除
 * @note 此实现不是线程安全的，多线程使用时需要外部同步
 * @note 智能指针提供引用计数，支持对象的安全共享
 */
template <typename T, typename TD>
class NFCommMapEx
{
public:
	/** @brief 内部映射容器类型定义，使用智能指针存储对象 */
	typedef std::map<T, NF_SHARE_PTR<TD> > NFMapOBJECT;

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个空的扩展映射容器。
	 */
	NFCommMapEx()
	{
	};
	
	/**
	 * @brief 虚析构函数
	 * 
	 * 销毁容器，智能指针会自动处理所有对象的释放。
	 */
	virtual ~NFCommMapEx()
	{
	};

	/**
	 * @brief 检查元素是否存在
	 * 
	 * 检查指定键值的元素是否存在于容器中。
	 * 
	 * @param name 要检查的元素键值
	 * @return bool 存在返回true，不存在返回false
	 * 
	 * @note 此方法只检查键是否存在，不验证对象指针是否有效
	 */
	virtual bool ExistElement(const T& name)
	{
		typename NFMapOBJECT::iterator itr = mObjectList.find(name);
		if (itr != mObjectList.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	/*
	virtual NF_SHARE_PTR<TD> AddElement(const T& name)
	{
	typename NFMapOBJECT::iterator itr = mObjectList.find(name);
	if (itr == mObjectList.end())
	{
	NF_SHARE_PTR<TD> data(NF_NEW TD());
	mObjectList.insert(typename NFMapOBJECT::value_type(name, data));
	return data;
	}

	return NF_SHARE_PTR<TD>();
	}
	*/
	
	/**
	 * @brief 添加元素到容器
	 * 
	 * 将智能指针管理的对象添加到容器中。如果键已存在，则添加失败。
	 * 
	 * @param name 元素的键值
	 * @param data 要存储的智能指针，不能为nullptr
	 * @return bool 添加成功返回true，键已存在或data为nullptr返回false
	 * 
	 * @pre data不能为nullptr
	 * @post 成功添加后，容器大小增加1
	 * @post 对象的引用计数增加
	 */
	virtual bool AddElement(const T& name, const NF_SHARE_PTR<TD> data)
	{
		if (data == nullptr)
		{
			std::cout << "AddElement failed : " << std::endl;
			return false;
		}

		typename NFMapOBJECT::iterator itr = mObjectList.find(name);
		if (itr == mObjectList.end())
		{
			mObjectList.insert(typename NFMapOBJECT::value_type(name, data));

			return true;
		}

		return false;
	}

	/**
	 * @brief 从容器中移除元素
	 * 
	 * 根据键值移除元素。智能指针会自动处理对象的释放。
	 * 
	 * @param name 要移除的元素的键值
	 * @return bool 移除成功返回true，键不存在返回false
	 * 
	 * @note 移除元素会减少对象的引用计数，可能导致对象被自动销毁
	 * @post 成功移除后，容器大小减少1
	 */
	virtual bool RemoveElement(const T& name)
	{
		typename NFMapOBJECT::iterator itr = mObjectList.find(name);
		if (itr != mObjectList.end())
		{
			mObjectList.erase(itr);

			return true;
		}

		return false;
	}

	/**
	 * @brief 获取元素的原始指针
	 * 
	 * 通过键值获取对应元素的原始指针。
	 * 
	 * @param name 要查找的元素的键值
	 * @return TD* 找到的对象原始指针，如果键不存在返回nullptr
	 * 
	 * @warning 返回的原始指针不受智能指针保护，使用时需要确保对象仍然有效
	 * @note 推荐使用GetElement()获取智能指针以确保安全性
	 */
	virtual TD* GetElementNude(const T& name)
	{
		typename NFMapOBJECT::iterator itr = mObjectList.find(name);
		if (itr != mObjectList.end())
		{
			return itr->second.get();
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * @brief 获取元素的智能指针
	 * 
	 * 通过键值获取对应元素的智能指针。
	 * 
	 * @param name 要查找的元素的键值
	 * @return NF_SHARE_PTR<TD> 找到的对象智能指针，如果键不存在返回nullptr
	 * 
	 * @note 智能指针提供引用计数，支持对象的安全共享
	 */
	virtual NF_SHARE_PTR<TD> GetElement(const T& name)
	{
		typename NFMapOBJECT::iterator itr = mObjectList.find(name);
		if (itr != mObjectList.end())
		{
			return itr->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取第一个元素的原始指针
	 * 
	 * 获取容器中第一个元素的原始指针，并返回其键值。
	 * 
	 * @param name [out] 返回的键值
	 * @return TD* 找到的对象原始指针，如果容器为空返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual TD* FirstNude(T& name)
	{
		if (mObjectList.size() <= 0)
		{
			return NULL;
		}

		mObjectCurIter = mObjectList.begin();
		if (mObjectCurIter != mObjectList.end())
		{
			name = mObjectCurIter->first;
			return mObjectCurIter->second.get();
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * @brief 获取下一个元素的原始指针
	 * 
	 * 获取容器中下一个元素的原始指针，并返回其键值。
	 * 
	 * @param name [out] 返回的键值
	 * @return TD* 找到的对象原始指针，如果迭代器已到达末尾返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual TD* NextNude(T& name)
	{
		if (mObjectCurIter == mObjectList.end())
		{
			return NULL;
		}

		mObjectCurIter++;
		if (mObjectCurIter != mObjectList.end())
		{
			name = mObjectCurIter->first;
			return mObjectCurIter->second.get();
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * @brief 获取第一个元素的智能指针
	 * 
	 * 获取容器中第一个元素的智能指针。
	 * 
	 * @return NF_SHARE_PTR<TD> 找到的对象智能指针，如果容器为空返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual TD* FirstNude()
	{
		if (mObjectList.size() <= 0)
		{
			return NULL;
		}

		mObjectCurIter = mObjectList.begin();
		if (mObjectCurIter != mObjectList.end())
		{
			return mObjectCurIter->second.get();
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * @brief 获取下一个元素的智能指针
	 * 
	 * 获取容器中下一个元素的智能指针。
	 * 
	 * @return NF_SHARE_PTR<TD> 找到的对象智能指针，如果迭代器已到达末尾返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual TD* NextNude()
	{
		if (mObjectCurIter == mObjectList.end())
		{
			return NULL;
		}

		mObjectCurIter++;
		if (mObjectCurIter != mObjectList.end())
		{
			return mObjectCurIter->second.get();
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * @brief 获取第一个元素的智能指针
	 * 
	 * 获取容器中第一个元素的智能指针，并返回其键值。
	 * 
	 * @param name [out] 返回的键值
	 * @return NF_SHARE_PTR<TD> 找到的对象智能指针，如果容器为空返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual NF_SHARE_PTR<TD> First()
	{
		if (mObjectList.size() <= 0)
		{
			return nullptr;
		}

		mObjectCurIter = mObjectList.begin();
		if (mObjectCurIter != mObjectList.end())
		{
			return mObjectCurIter->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取下一个元素的智能指针
	 * 
	 * 获取容器中下一个元素的智能指针。
	 * 
	 * @return NF_SHARE_PTR<TD> 找到的对象智能指针，如果迭代器已到达末尾返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual NF_SHARE_PTR<TD> Next()
	{
		if (mObjectCurIter == mObjectList.end())
		{
			return nullptr;
		}

		++mObjectCurIter;
		if (mObjectCurIter != mObjectList.end())
		{
			return mObjectCurIter->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief 获取第一个元素的智能指针，并返回其键值
	 * 
	 * 获取容器中第一个元素的智能指针，并返回其键值。
	 * 
	 * @param name [out] 返回的键值
	 * @return NF_SHARE_PTR<TD> 找到的对象智能指针，如果容器为空返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual NF_SHARE_PTR<TD> First(T& name)
	{
		if (mObjectList.size() <= 0)
		{
			return nullptr;
		}

		mObjectCurIter = mObjectList.begin();
		if (mObjectCurIter != mObjectList.end())
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
	 * @brief 获取下一个元素的智能指针，并返回其键值
	 * 
	 * 获取容器中下一个元素的智能指针，并返回其键值。
	 * 
	 * @param name [out] 返回的键值
	 * @return NF_SHARE_PTR<TD> 找到的对象智能指针，如果迭代器已到达末尾返回nullptr
	 * 
	 * @note 此方法会更新迭代器位置
	 */
	virtual NF_SHARE_PTR<TD> Next(T& name)
	{
		if (mObjectCurIter == mObjectList.end())
		{
			return nullptr;
		}

		mObjectCurIter++;
		if (mObjectCurIter != mObjectList.end())
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
	 * @brief 清空所有元素
	 * 
	 * 移除容器中所有元素，并释放所有对象的内存。
	 * 
	 * @return bool 清空成功返回true
	 * 
	 * @post 容器大小变为0
	 * @post 所有对象的引用计数减少
	 */
	virtual bool ClearAll()
	{
		mObjectList.clear();
		return true;
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
		return (int)mObjectList.size();
	}

    typename NFMapOBJECT::iterator Begin()
    {
	    return mObjectList.begin();
    }

    typename NFMapOBJECT::iterator End()
    {
        return mObjectList.end();
    }
protected:
	NFMapOBJECT     mObjectList;
	typename NFMapOBJECT::iterator mObjectCurIter;
};

template <typename T, typename TD>
class NFConsistentCommMapEx : public NFCommMapEx<T, TD>
{
public:
	virtual NF_SHARE_PTR<TD> GetElementBySuitRandom()
	{
		NFVirtualNode<T> vNode;
		if (mxConsistentHash.GetSuitNodeRandom(vNode))
		{
			typename NFCommMapEx<T, TD>::NFMapOBJECT::iterator itr = NFCommMapEx<T, TD>::mObjectList.find(vNode.mxData);
			if (itr != NFCommMapEx<T, TD>::mObjectList.end())
			{
				return itr->second;
			}
		}

		return NULL;
	}

	virtual NF_SHARE_PTR<TD> GetElementBySuitConsistent()
	{
		NFVirtualNode<T> vNode;
		if (mxConsistentHash.GetSuitNodeConsistent(vNode))
		{
			typename NFCommMapEx<T, TD>::NFMapOBJECT::iterator itr = NFCommMapEx<T, TD>::mObjectList.find(vNode.mxData);
			if (itr != NFCommMapEx<T, TD>::mObjectList.end())
			{
				return itr->second;
			}
		}

		return NULL;
	}

	virtual NF_SHARE_PTR<TD> GetElementBySuit(const T& name)
	{
		NFVirtualNode<T> vNode;
		if (mxConsistentHash.GetSuitNode(name, vNode))
		{
			typename NFCommMapEx<T, TD>::NFMapOBJECT::iterator itr = NFCommMapEx<T, TD>::mObjectList.find(vNode.mxData);
			if (itr != NFCommMapEx<T, TD>::mObjectList.end())
			{
				return itr->second;
			}
		}

		return NULL;
	}

	template<typename TX>
    NF_SHARE_PTR<TD> GetElementBySuit(const TX& name)
    {
        NFVirtualNode<T> vNode;
        if (mxConsistentHash.GetSuitNode(name, vNode))
        {
            typename NFCommMapEx<T, TD>::NFMapOBJECT::iterator itr = NFCommMapEx<T, TD>::mObjectList.find(vNode.mxData);
            if (itr != NFCommMapEx<T, TD>::mObjectList.end())
            {
                return itr->second;
            }
        }

        return NULL;
    }

	virtual bool AddElement(const T& name, const NF_SHARE_PTR<TD> data) override
	{
		if (data == nullptr)
		{
			return false;
		}

		typename NFCommMapEx<T, TD>::NFMapOBJECT::iterator itr = NFCommMapEx<T, TD>::mObjectList.find(name);
		if (itr == NFCommMapEx<T, TD>::mObjectList.end())
		{
			NFCommMapEx<T, TD>::mObjectList.insert(typename NFCommMapEx<T, TD>::NFMapOBJECT::value_type(name, data));

			mxConsistentHash.Insert(name);

			return true;
		}

		return false;
	}

	virtual bool RemoveElement(const T& name) override
	{
		typename NFCommMapEx<T, TD>::NFMapOBJECT::iterator itr = NFCommMapEx<T, TD>::mObjectList.find(name);
		if (itr != NFCommMapEx<T, TD>::mObjectList.end())
		{
			NFCommMapEx<T, TD>::mObjectList.erase(itr);
			mxConsistentHash.Erase(name);

			return true;
		}

		return false;
	}

	virtual bool ClearAll() override
	{
		NFCommMapEx<T, TD>::mObjectList.clear();
		mxConsistentHash.ClearAll();
		return true;
	}

private:
	NFConsistentHash<T> mxConsistentHash;
};
