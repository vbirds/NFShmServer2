// -------------------------------------------------------------------------
//    @FileName         :    NFConsistentHash.hpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//
// -------------------------------------------------------------------------

/**
 * @file NFConsistentHash.hpp
 * @brief 一致性哈希算法实现
 * 
 * 此文件提供了经典的一致性哈希算法实现，支持虚拟节点机制。
 * 一致性哈希是分布式系统中常用的负载均衡算法，能够在节点增减时
 * 最小化数据迁移，保持良好的分布性和平衡性。
 */

#pragma once

#include <map>
#include <string>
#include <list>
#include <functional> 
#include <algorithm>
#include <chrono>
#include "NFPlatform.h"
#include "NFCRC32.h"
#include "NFCommon.h"
#include "NFRandom.hpp"

/**
 * @brief 虚拟节点接口基类
 * 
 * NFIVirtualNode定义了虚拟节点的基本接口，为一致性哈希算法提供
 * 统一的节点抽象。每个虚拟节点都有一个唯一的虚拟索引ID。
 * 
 * 虚拟节点的作用：
 * - 提高数据分布的均匀性
 * - 减少节点变化时的数据迁移量
 * - 支持节点权重的灵活配置
 * 
 * 设计模式：
 * - 使用接口分离原则，定义纯虚函数
 * - 支持多态，便于扩展不同类型的节点
 * - 提供标准的字符串表示方法
 */
//template <typename T>
class NFIVirtualNode
{
public:
	/**
	 * @brief 构造函数，创建指定虚拟ID的节点
	 * 
	 * @param nVirID 虚拟节点的唯一标识ID
	 */
	NFIVirtualNode(const int nVirID)
		:nVirtualIndex(nVirID)
	{

	}

	/**
	 * @brief 默认构造函数，虚拟ID初始化为0
	 */
	NFIVirtualNode()
	{
		nVirtualIndex = 0;
	}

	/**
	 * @brief 虚析构函数，确保多态删除的正确性
	 */
	virtual ~NFIVirtualNode()
	{
		nVirtualIndex = 0;
	}

	/**
	 * @brief 获取节点数据的字符串表示
	 * 
	 * 纯虚函数，由派生类实现具体的数据转换逻辑。
	 * 
	 * @return std::string 节点数据的字符串表示
	 */
	virtual std::string GetDataStr() const
	{
		return "";
	}

	/**
	 * @brief 获取虚拟节点的完整字符串表示
	 * 
	 * 将节点数据和虚拟索引组合成唯一的字符串标识，
	 * 格式为："数据字符串-虚拟索引"。
	 * 
	 * @return std::string 虚拟节点的字符串表示
	 * 
	 * @note 此字符串用于计算哈希值，确保虚拟节点的唯一性
	 */
	std::string ToStr() const
	{
		std::ostringstream strInfo;
		strInfo << GetDataStr() << "-" << nVirtualIndex;
		return strInfo.str();
	}

private:
	/** @brief 虚拟节点的索引ID，用于区分同一数据的不同虚拟节点 */
	int nVirtualIndex;
};

/**
 * @brief 模板化的虚拟节点实现类
 * 
 * NFVirtualNode是NFIVirtualNode的模板化实现，支持任意类型的数据。
 * 通过模板参数T，可以存储不同类型的节点数据（如服务器ID、IP地址等）。
 * 
 * @tparam T 节点数据的类型，要求支持NFCommon::tostr()转换
 * 
 * 使用示例：
 * @code
 * // 字符串类型的虚拟节点
 * NFVirtualNode<std::string> strNode("server1", 0);
 * 
 * // 整数类型的虚拟节点  
 * NFVirtualNode<int> intNode(12345, 1);
 * 
 * // 获取节点的字符串表示
 * std::string nodeStr = strNode.ToStr();  // "server1-0"
 * @endcode
 */
template <typename T>
class NFVirtualNode : public NFIVirtualNode
{
public:
	/**
	 * @brief 构造函数，创建包含指定数据和虚拟ID的节点
	 * 
	 * @param tData 节点存储的数据
	 * @param nVirID 虚拟节点的索引ID
	 */
	NFVirtualNode(const T tData, const int nVirID) : NFIVirtualNode(nVirID)
	{
		mxData = tData;
	}
	
	/**
	 * @brief 默认构造函数
	 */
	NFVirtualNode()
	{
	}

	/**
	 * @brief 重写基类方法，获取数据的字符串表示
	 * 
	 * 使用NFCommon::tostr()将模板参数类型的数据转换为字符串。
	 * 
	 * @return std::string 数据的字符串表示
	 */
	virtual std::string GetDataStr() const
	{
		return NFCommon::tostr(mxData);
	}

	/** @brief 存储的节点数据 */
	T mxData;
};

/**
 * @brief 哈希计算器接口
 * 
 * NFIHasher定义了哈希值计算的统一接口，支持不同的哈希算法实现。
 * 通过接口设计，可以灵活地切换和扩展哈希算法。
 */
class NFIHasher
{
public:
	/**
	 * @brief 虚析构函数
	 */
	virtual ~NFIHasher() {}
	
	/**
	 * @brief 计算虚拟节点的哈希值
	 * 
	 * 纯虚函数，由派生类实现具体的哈希计算逻辑。
	 * 
	 * @param vNode 要计算哈希值的虚拟节点
	 * @return uint32_t 32位哈希值
	 */
	virtual uint32_t GetHashValue(const NFIVirtualNode& vNode) = 0;
};

/**
 * @brief 基于CRC32的哈希计算器实现
 * 
 * NFHasher使用CRC32算法计算虚拟节点的哈希值，提供快速稳定的哈希计算。
 * CRC32算法具有良好的分布性和较低的冲突率。
 */
class NFHasher : public NFIHasher
{
public:
	/**
	 * @brief 使用CRC32算法计算虚拟节点的哈希值
	 * 
	 * 将虚拟节点转换为字符串后，使用CRC32算法计算哈希值。
	 * 
	 * @param vNode 要计算哈希值的虚拟节点
	 * @return uint32_t CRC32哈希值
	 */
	virtual uint32_t GetHashValue(const NFIVirtualNode& vNode)
	{
		std::string vnode = vNode.ToStr();
		return NFCRC32::Sum(vnode);
	}
};

/**
 * @brief 一致性哈希接口
 * 
 * NFIConsistentHash定义了一致性哈希算法的标准接口，
 * 包括节点的增删查改和负载均衡选择等核心功能。
 * 
 * @tparam T 节点数据的类型
 */
template <typename T>
class NFIConsistentHash
{
public:
	/** @brief 获取哈希环中的节点总数 */
	virtual std::size_t Size() const = 0;
	/** @brief 检查哈希环是否为空 */
	virtual bool Empty() const = 0;

	/** @brief 清空所有节点 */
	virtual void ClearAll() = 0;
	/** @brief 插入节点（自动创建虚拟节点） */
	virtual void Insert(const T& name) = 0;
	/** @brief 插入指定的虚拟节点 */
	virtual void Insert(const NFVirtualNode<T>& xNode) = 0;

	/** @brief 检查虚拟节点是否存在 */
	virtual bool Exist(const NFVirtualNode<T>& xInNode) = 0;
	/** @brief 删除节点（删除所有相关虚拟节点） */
	virtual void Erase(const T& name) = 0;
	/** @brief 删除指定的虚拟节点 */
	virtual std::size_t Erase(const NFVirtualNode<T>& xNode) = 0;

	/** @brief 随机选择一个节点 */
	virtual bool GetSuitNodeRandom(NFVirtualNode<T>& node) = 0;
	/** @brief 一致性选择节点（从哈希值0开始） */
	virtual bool GetSuitNodeConsistent(NFVirtualNode<T>& node) = 0;
	/** @brief 根据数据选择最适合的节点 */
	virtual bool GetSuitNode(const T& name, NFVirtualNode<T>& node) = 0;

	/** @brief 根据哈希值选择最适合的节点 */
	virtual bool GetSuitNodeForHashValue(uint32_t hashValue, NFVirtualNode<T>& node) = 0;

	/** @brief 获取所有节点的列表 */
	virtual bool GetNodeList(std::list<NFVirtualNode<T>>& nodeList) = 0;
};

/**
 * @brief 一致性哈希算法的具体实现
 * 
 * NFConsistentHash实现了经典的一致性哈希算法，支持虚拟节点机制。
 * 使用std::map作为底层存储结构，提供高效的节点查找和管理。
 * 
 * 算法特点：
 * - 节点增减时数据迁移量最小化（平均只有1/N的数据需要迁移）
 * - 良好的负载均衡性（通过虚拟节点机制）
 * - 支持节点权重（通过虚拟节点数量控制）
 * - 高效的查找性能（O(log N)时间复杂度）
 * 
 * 实现原理：
 * 1. 将每个物理节点映射为多个虚拟节点
 * 2. 将所有虚拟节点的哈希值排列在一个环上
 * 3. 数据的哈希值顺时针查找第一个虚拟节点
 * 4. 返回该虚拟节点对应的物理节点
 * 
 * @tparam T 节点数据的类型
 * 
 * 使用示例：
 * @code
 * // 创建一致性哈希实例
 * NFConsistentHash<std::string> hash;
 * 
 * // 添加服务器节点
 * hash.Insert("server1");
 * hash.Insert("server2");
 * hash.Insert("server3");
 * 
 * // 根据key选择服务器
 * NFVirtualNode<std::string> node;
 * if (hash.GetSuitNode("user123", node)) {
 *     std::cout << "Selected server: " << node.mxData << std::endl;
 * }
 * 
 * // 移除服务器
 * hash.Erase("server2");
 * @endcode
 */
template <typename T>
class NFConsistentHash : public NFIConsistentHash<T>
{
public:
	/**
	 * @brief 构造函数，初始化哈希计算器
	 */
	NFConsistentHash()
	{
		m_pHasher = new NFHasher();
	}

	/**
	 * @brief 析构函数，清理资源
	 */
	virtual ~NFConsistentHash()
	{
		delete m_pHasher;
		m_pHasher = NULL;
	}

public:
	/**
	 * @brief 获取哈希环中虚拟节点的总数
	 * 
	 * @return std::size_t 虚拟节点数量
	 */
	virtual std::size_t Size() const
	{
		return mxNodes.size();
	}

	/**
	 * @brief 检查哈希环是否为空
	 * 
	 * @return bool 空返回true，否则返回false
	 */
	virtual bool Empty() const
	{
		return mxNodes.empty();
	}

	/**
	 * @brief 清空哈希环中的所有节点
	 * 
	 * 移除所有虚拟节点，重置哈希环状态。
	 */
	virtual void ClearAll()
	{
		mxNodes.clear();
	}

	/**
	 * @brief 插入物理节点，自动创建虚拟节点
	 * 
	 * 为指定的物理节点创建mnNodeCount个虚拟节点，
	 * 并将它们分布在哈希环上。
	 * 
	 * @param name 要插入的物理节点数据
	 * 
	 * @note 每个物理节点默认创建500个虚拟节点
	 * @note 虚拟节点越多，负载分布越均匀
	 */
	virtual void Insert(const T& name)
	{
		for (int i = 0; i < mnNodeCount; ++i)
		{
			NFVirtualNode<T> vNode(name, i);
			Insert(vNode);
		}
	}

	/**
	 * @brief 插入指定的虚拟节点
	 * 
	 * 将虚拟节点插入到哈希环中的适当位置。
	 * 如果哈希值冲突，则忽略插入。
	 * 
	 * @param xNode 要插入的虚拟节点
	 * 
	 * @note 哈希值冲突时不会覆盖已存在的节点
	 */
	virtual void Insert(const NFVirtualNode<T>& xNode)
	{
		uint32_t hash = m_pHasher->GetHashValue(xNode);
		auto it = mxNodes.find(hash);
		if (it == mxNodes.end())
		{
			mxNodes.insert(typename std::map<uint32_t, NFVirtualNode<T>>::value_type(hash, xNode));
		}
	}

	/**
	 * @brief 检查虚拟节点是否存在于哈希环中
	 * 
	 * @param xInNode 要检查的虚拟节点
	 * @return bool 存在返回true，否则返回false
	 */
	virtual bool Exist(const NFVirtualNode<T>& xInNode)
	{
		uint32_t hash = m_pHasher->GetHashValue(xInNode);
		typename std::map<uint32_t, NFVirtualNode<T>>::iterator it = mxNodes.find(hash);
		if (it != mxNodes.end())
		{
			return true;
		}

		return false;
	}

	/**
	 * @brief 删除物理节点及其所有虚拟节点
	 * 
	 * 删除指定物理节点对应的所有虚拟节点。
	 * 
	 * @param name 要删除的物理节点数据
	 * 
	 * @note 会删除该物理节点的所有虚拟节点（默认500个）
	 */
	virtual void Erase(const T& name)
	{
		for (int i = 0; i < mnNodeCount; ++i)
		{
			NFVirtualNode<T> vNode(name, i);
			Erase(vNode);
		}
	}

	/**
	 * @brief 删除指定的虚拟节点
	 * 
	 * @param xNode 要删除的虚拟节点
	 * @return std::size_t 删除的节点数量（0或1）
	 */
	virtual std::size_t Erase(const NFVirtualNode<T>& xNode)
	{
		uint32_t hash = m_pHasher->GetHashValue(xNode);
		return mxNodes.erase(hash);
	}

	/**
	 * @brief 随机选择一个节点
	 * 
	 * 基于当前时间和随机数生成一个哈希值，然后选择对应的节点。
	 * 提供了简单的负载均衡机制。
	 * 
	 * @param node [out] 输出选中的虚拟节点
	 * @return bool 选择成功返回true，哈希环为空返回false
	 */
	virtual bool GetSuitNodeRandom(NFVirtualNode<T>& node)
	{
		uint64_t nID = NFGetTime() + NFRandInt(1, 10000);
		return GetSuitNode(nID, node);
	}

	/**
	 * @brief 一致性选择节点
	 * 
	 * 从哈希值0开始，选择顺时针方向的第一个节点。
	 * 提供确定性的节点选择。
	 * 
	 * @param node [out] 输出选中的虚拟节点
	 * @return bool 选择成功返回true，哈希环为空返回false
	 */
	virtual bool GetSuitNodeConsistent(NFVirtualNode<T>& node)
	{
		return GetSuitNodeForHashValue((uint32_t)0, node);
	}

	/**
	 * @brief 根据数据选择最适合的节点
	 * 
	 * 计算数据的哈希值，然后选择哈希环上顺时针方向的第一个节点。
	 * 这是一致性哈希的核心功能。
	 * 
	 * @param name 用于计算哈希值的数据
	 * @param node [out] 输出选中的虚拟节点
	 * @return bool 选择成功返回true，哈希环为空返回false
	 */
	virtual bool GetSuitNode(const T& name, NFVirtualNode<T>& node)
	{
		std::string str = NFCommon::tostr(name);
		uint32_t nCRC32 = NFCRC32::Sum(str);
		return GetSuitNodeForHashValue(nCRC32, node);
	}

	/**
	 * @brief 模板化的节点选择方法
	 * 
	 * 支持不同类型的数据进行节点选择，提供更灵活的使用方式。
	 * 
	 * @tparam TX 数据类型，要求支持NFCommon::tostr()转换
	 * @param name 用于计算哈希值的数据
	 * @param node [out] 输出选中的虚拟节点
	 * @return bool 选择成功返回true，哈希环为空返回false
	 */
	template<typename TX>
    bool GetSuitNode(const TX& name, NFVirtualNode<T>& node)
    {
        std::string str = NFCommon::tostr(name);
        uint32_t nCRC32 = NFCRC32::Sum(str);
        return GetSuitNodeForHashValue(nCRC32, node);
    }

	/**
	 * @brief 根据哈希值选择最适合的节点
	 * 
	 * 一致性哈希算法的核心实现：在哈希环上查找大于等于给定哈希值的
	 * 第一个节点，如果没有找到则选择第一个节点（环形特性）。
	 * 
	 * @param hashValue 哈希值
	 * @param node [out] 输出选中的虚拟节点
	 * @return bool 选择成功返回true，哈希环为空返回false
	 * 
	 * 算法复杂度：O(log N)，其中N是虚拟节点数量
	 */
	virtual bool GetSuitNodeForHashValue(uint32_t hashValue, NFVirtualNode<T>& node)
	{
		if (mxNodes.empty())
		{
			return false;
		}

		typename std::map<uint32_t, NFVirtualNode<T>>::iterator it = mxNodes.lower_bound(hashValue);

		if (it == mxNodes.end())
		{
			it = mxNodes.begin();
		}

		node = it->second;

		return true;
	}

	/**
	 * @brief 获取所有虚拟节点的列表
	 * 
	 * 将哈希环中的所有虚拟节点复制到提供的列表中。
	 * 
	 * @param nodeList [out] 输出的虚拟节点列表
	 * @return bool 操作成功返回true
	 * 
	 * @note 返回的列表按照哈希值排序
	 * @note 这个操作的时间复杂度是O(N)
	 */
	virtual bool GetNodeList(std::list<NFVirtualNode<T>>& nodeList)
	{
		for (typename std::map<uint32_t, NFVirtualNode<T>>::iterator it = mxNodes.begin(); it != mxNodes.end(); ++it)
		{
			nodeList.push_back(it->second);
		}

		return true;
	}

private:
	/** @brief 每个物理节点对应的虚拟节点数量，默认500个 */
	int mnNodeCount = 500;
	/** @brief 哈希环存储结构，键为哈希值，值为虚拟节点 */
	typename std::map<uint32_t, NFVirtualNode<T>> mxNodes;
	/** @brief 哈希计算器指针 */
	NFIHasher* m_pHasher;
};
