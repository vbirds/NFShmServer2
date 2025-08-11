// -------------------------------------------------------------------------
//    @FileName         :    NFList.hpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFList.hpp
 * @brief 通用链表容器模板类
 * 
 * 此文件提供了一个基于std::list封装的通用链表容器，支持基本的增删改查操作，
 * 并提供了迭代器风格的遍历接口。主要用于需要顺序存储和遍历的场景。
 */

#pragma once

#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include "NFPlatform.h"

/**
 * @brief 通用链表容器模板类
 * 
 * NFList是一个基于std::list的封装容器，提供了简化的链表操作接口。
 * 支持元素的添加、删除、查找和遍历操作。
 * 
 * 主要特性：
 * - 泛型设计：支持任意可比较的数据类型
 * - 简化接口：提供比STL更简洁的操作方法
 * - 遍历支持：提供First/Next风格的迭代接口
 * - 随机访问：支持按索引获取元素
 * - 自动管理：虚析构函数支持继承
 * 
 * 适用场景：
 * - 需要保持插入顺序的数据集合
 * - 频繁插入和删除的场景
 * - 需要遍历所有元素的处理
 * - 简单的数据容器需求
 * 
 * 使用方法：
 * @code
 * NFList<int> intList;
 * 
 * // 添加元素
 * intList.Add(10);
 * intList.Add(20);
 * intList.Add(30);
 * 
 * // 查找元素
 * if (intList.Find(20)) {
 *     std::cout << "找到元素20" << std::endl;
 * }
 * 
 * // 遍历所有元素
 * int value;
 * if (intList.First(value)) {
 *     std::cout << "第一个元素: " << value << std::endl;
 *     while (intList.Next(value)) {
 *         std::cout << "下一个元素: " << value << std::endl;
 *     }
 * }
 * 
 * // 按索引访问
 * int element;
 * if (intList.Get(1, element)) {
 *     std::cout << "索引1的元素: " << element << std::endl;
 * }
 * @endcode
 * 
 * @tparam T 元素类型，必须支持==操作符进行比较
 * 
 * @note 此类不是线程安全的，多线程访问时需要外部同步
 * @note 添加操作不检查重复元素，允许添加相同的值
 */
template <typename T>
class NFList
{
public:
	/**
	 * @brief 虚析构函数
	 * 
	 * 支持多态继承，确保派生类能正确析构。
	 */
	virtual ~NFList()
	{
	}

	/**
	 * @brief 添加元素到链表末尾
	 * 
	 * 将指定元素添加到链表的末尾，不检查重复元素。
	 * 
	 * @param id 要添加的元素
	 * @return bool 总是返回true（添加成功）
	 * 
	 * @note 允许添加重复元素
	 * @note 新元素总是添加到链表末尾
	 */
	bool Add(const T& id);
	
	/**
	 * @brief 查找指定元素
	 * 
	 * 在链表中查找是否存在指定的元素。
	 * 
	 * @param id 要查找的元素
	 * @return bool 找到返回true，否则返回false
	 * 
	 * @note 使用==操作符进行比较
	 * @note 时间复杂度为O(n)
	 */
	bool Find(const T& id);
	
	/**
	 * @brief 删除指定元素
	 * 
	 * 从链表中删除第一个匹配的元素。
	 * 
	 * @param id 要删除的元素
	 * @return bool 删除成功返回true，元素不存在返回false
	 * 
	 * @note 只删除第一个匹配的元素
	 * @note 如果有多个相同元素，只删除第一个
	 */
	bool Remove(const T& id);
	
	/**
	 * @brief 清空所有元素
	 * 
	 * 删除链表中的所有元素，链表变为空。
	 * 
	 * @return bool 总是返回true（清空成功）
	 */
	bool ClearAll();

	/**
	 * @brief 获取第一个元素并设置遍历位置
	 * 
	 * 获取链表的第一个元素，并将内部迭代器设置到第一个位置，
	 * 为后续的Next()调用做准备。
	 * 
	 * @param id [out] 接收第一个元素的变量
	 * @return bool 获取成功返回true，链表为空返回false
	 * 
	 * @note 此方法会重置内部迭代器位置
	 * @note 调用后可以使用Next()继续遍历
	 */
	bool First(T& id);
	
	/**
	 * @brief 获取下一个元素
	 * 
	 * 获取当前迭代器位置的下一个元素，并向前移动迭代器。
	 * 必须在调用First()之后使用。
	 * 
	 * @param id [out] 接收下一个元素的变量
	 * @return bool 获取成功返回true，已到达末尾返回false
	 * 
	 * @note 必须先调用First()设置初始位置
	 * @note 到达链表末尾后返回false
	 */
	bool Next(T& id);
	
	/**
	 * @brief 按索引获取元素
	 * 
	 * 根据索引位置获取对应的元素，索引从0开始。
	 * 
	 * @param index 元素索引（0开始）
	 * @param id [out] 接收元素的变量
	 * @return bool 获取成功返回true，索引超出范围返回false
	 * 
	 * @note 索引从0开始计算
	 * @note 时间复杂度为O(n)，因为需要遍历到指定位置
	 */
	bool Get(const int32_t index, T& id);
	
	/**
	 * @brief 获取元素数量
	 * 
	 * 返回当前链表中元素的数量。
	 * 
	 * @return int 链表中元素的数量
	 */
	int Count();

protected:
	/** @brief 链表类型定义 */
	typedef std::list<T> TLISTOBJCONFIGLIST;
	/** @brief 存储元素的链表容器 */
	TLISTOBJCONFIGLIST mtObjConfigList;
	/** @brief 当前遍历位置的迭代器 */
	typename std::list<T>::iterator mCurIter;

private:
};

template <typename T>
bool NFList<T>::Add(const T& id)
{
	//if (!Find(id))  // 注释掉的重复检查
	{
		mtObjConfigList.push_back(id);
		return true;
	}

	return false;
}

template <typename T>
bool NFList<T>::Remove(const T& id)
{
	if (Find(id))
	{
		mtObjConfigList.remove(id);
		return true;
	}

	return false;
}

template <typename T>
bool NFList<T>::ClearAll()
{
	mtObjConfigList.clear();
	return true;
}

template <typename T>
bool NFList<T>::First(T& id)
{
	if (mtObjConfigList.size() <= 0)
	{
		return false;
	}

	mCurIter = mtObjConfigList.begin();
	if (mCurIter != mtObjConfigList.end())
	{
		id = *mCurIter;
		return true;
	}

	return false;
}

template <typename T>
bool NFList<T>::Next(T& id)
{
	if (mCurIter == mtObjConfigList.end())
	{
		return false;
	}

	++mCurIter;
	if (mCurIter != mtObjConfigList.end())
	{
		id = *mCurIter;
		return true;
	}

	return false;
}

template <typename T>
bool NFList<T>::Find(const T& id)
{
	typename TLISTOBJCONFIGLIST::iterator it = std::find(mtObjConfigList.begin(), mtObjConfigList.end(), id);
	if (it != mtObjConfigList.end())
	{
		return true;
	}

	return false;
}

template <typename T>
bool NFList<T>::Get(const int32_t index, T& id)
{
	if (index >= mtObjConfigList.size())
	{
		return false;
	}

	typename std::list<T>::iterator it = this->mtObjConfigList.begin();
	std::advance(it, index);

	id = *it;

	return true;
}

template <typename T>
int NFList<T>::Count()
{
	return (int)(mtObjConfigList.size());
}

