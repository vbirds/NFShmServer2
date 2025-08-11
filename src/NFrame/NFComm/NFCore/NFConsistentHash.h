// -------------------------------------------------------------------------
//    @FileName         :    NFConsistentHash.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFConsistentHash.h
 * @brief 一致性哈希算法实现
 * 
 * 此文件提供了一致性哈希（Consistent Hash）算法的实现，主要用于分布式系统中
 * 的负载均衡和数据分片。一致性哈希能够在节点数量变化时最小化数据迁移。
 */

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "NFPlatform.h"
#include "NFMD5.h"

/**
 * @brief 哈希环节点结构体
 * 
 * 表示一致性哈希环上的一个节点，包含节点的哈希值和对应的索引。
 */
struct NFNode_T
{
	/** @brief 节点在哈希环上的哈希值 */
	unsigned int iHashCode;

	/** @brief 节点在原始节点列表中的索引 */
	unsigned int iIndex;
};

/**
 * @brief 一致性哈希算法实现类
 * 
 * NFConsistentHash实现了一致性哈希算法，用于在分布式系统中进行负载均衡。
 * 该算法通过将节点和数据映射到一个虚拟的哈希环上，实现数据的均匀分布。
 * 
 * 主要特性：
 * - 最小化迁移：添加或删除节点时，只影响相邻节点的数据
 * - 负载均衡：数据在节点间相对均匀分布
 * - 高效查找：使用二分查找快速定位节点
 * - MD5哈希：使用MD5算法生成均匀的哈希值
 * - 动态调整：支持动态添加和删除节点
 * 
 * 算法原理：
 * 1. 将节点通过哈希函数映射到0~2^32-1的哈希环上
 * 2. 将数据key也通过相同哈希函数映射到哈希环上
 * 3. 数据沿哈希环顺时针方向找到的第一个节点即为负责节点
 * 4. 节点变化时，只有相邻区域的数据需要迁移
 * 
 * 适用场景：
 * - 分布式缓存系统
 * - 数据库分片
 * - 负载均衡器
 * - 分布式存储系统
 * - 微服务路由
 * 
 * 使用方法：
 * @code
 * NFConsistentHash hashRing;
 * 
 * // 添加节点
 * hashRing.addNode("server1", 0);
 * hashRing.addNode("server2", 1);
 * hashRing.addNode("server3", 2);
 * 
 * // 查找数据应该分配到哪个节点
 * unsigned int nodeIndex;
 * if (hashRing.getIndex("user_123", nodeIndex) == 0) {
 *     std::cout << "user_123 分配到节点: " << nodeIndex << std::endl;
 * }
 * 
 * // 删除节点
 * hashRing.removeNode("server2");
 * 
 * // 重新查找（部分数据会迁移到相邻节点）
 * if (hashRing.getIndex("user_123", nodeIndex) == 0) {
 *     std::cout << "删除节点后，user_123 现在分配到: " << nodeIndex << std::endl;
 * }
 * @endcode
 * 
 * @note 使用MD5哈希确保节点在环上的均匀分布
 * @note 支持虚拟节点以进一步提高负载均衡效果
 * @note 节点变更后会自动重新排序哈希环
 */
class NFConsistentHash
{
public:

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个空的一致性哈希环。
	 */
	NFConsistentHash()
	{
	}

	/**
	 * @brief 节点比较函数
	 * 
	 * 用于对哈希环上的节点进行排序的比较函数。
	 * 按照哈希值从小到大排序。
	 * 
	 * @param m1 第一个节点
	 * @param m2 第二个节点
	 * @return bool m1的哈希值小于m2时返回true，否则返回false
	 * 
	 * @note 这是一个静态方法，用于std::sort算法
	 */
	static bool less_hash(const NFNode_T & m1, const NFNode_T & m2)
	{
		return m1.iHashCode < m2.iHashCode;
	}

	/**
	 * @brief 添加节点到哈希环
	 * 
	 * 将一个新节点添加到一致性哈希环上。节点会根据其名称计算MD5哈希值，
	 * 然后插入到哈希环的相应位置。
	 * 
	 * @param node 节点名称，用于计算哈希值的唯一标识
	 * @param index 节点索引，用于标识实际的服务器或资源
	 * @return unsigned int 节点在哈希环上的哈希值
	 * 
	 * @note 添加节点后会自动重新排序哈希环
	 * @note 相同名称的节点会产生相同的哈希值
	 * @note 建议使用唯一的节点名称避免冲突
	 */
	unsigned addNode(const std::string & node, unsigned int index)
	{
		NFNode_T stItem;
		stItem.iHashCode = hash_md5(NFMD5::md5bin(node));
		stItem.iIndex = index;
		vHashList.push_back(stItem);

		std::sort(vHashList.begin(), vHashList.end(), less_hash);

		return stItem.iHashCode;
	}

	/**
	 * @brief 从哈希环中删除节点
	 * 
	 * 根据节点名称从一致性哈希环中删除对应的节点。
	 * 删除节点后，原本分配给该节点的数据会重新分配给相邻节点。
	 * 
	 * @param node 要删除的节点名称
	 * @return int 删除结果
	 *         - 0: 删除成功
	 *         - -1: 节点不存在
	 * 
	 * @note 删除不存在的节点不会产生错误，但会返回-1
	 * @note 删除节点会导致部分数据重新分配
	 */
	int removeNode(const std::string & node)
	{
		unsigned iHashCode = hash_md5(NFMD5::md5bin(node));
		std::vector<NFNode_T>::iterator it;
		for (it = vHashList.begin(); it != vHashList.end(); it++)
		{
			if (it->iHashCode == iHashCode)
			{
				vHashList.erase(it);
				return 0;
			}
		}
		return -1;
	}

	/**
	 * @brief 获取数据key对应的节点索引
	 * 
	 * 根据数据key计算其在一致性哈希环上的位置，并返回负责该数据的节点索引。
	 * 使用顺时针方向查找第一个大于等于key哈希值的节点。
	 * 
	 * @param key 数据key，用于计算哈希值
	 * @param iIndex [out] 输出参数，返回负责该key的节点索引
	 * @return int 查找结果
	 *         - 0: 查找成功
	 *         - -1: 没有可用节点
	 * 
	 * @note 使用二分查找算法，时间复杂度为O(log n)
	 * @note 如果key的哈希值大于所有节点，会分配给第一个节点（环形特性）
	 * @note 当没有节点时，iIndex会被设置为0但返回-1
	 */
	int getIndex(const std::string & key, unsigned int & iIndex)
	{
		unsigned iCode = hash_md5(NFMD5::md5bin(key));
		if (vHashList.size() == 0)
		{
			iIndex = 0;
			return -1;
		}

		unsigned low = 0;
		unsigned high = vHashList.size();

		if (iCode <= vHashList[0].iHashCode || iCode > vHashList[high - 1].iHashCode)
		{
			iIndex = vHashList[0].iIndex;
			return 0;
		}

		while (low < high - 1)
		{
			unsigned mid = (low + high) / 2;
			if (vHashList[mid].iHashCode > iCode)
			{
				high = mid;
			}
			else
			{
				low = mid;
			}
		}
		iIndex = vHashList[low + 1].iIndex;
		return 0;
	}

protected:
	/**
	 * @brief 计算MD5值的哈希
	 * 
	 * 将16字节的MD5值转换为32位的哈希值，分布范围在0~2^32-1。
	 * 使用XOR操作将MD5的4个32位段合并为一个32位值。
	 * 
	 * @param sMd5 16字节的MD5二进制数据
	 * @return unsigned int 32位哈希值
	 * 
	 * @note 这是内部方法，用于将MD5结果映射到哈希环空间
	 * @note 使用XOR操作确保哈希值的均匀分布
	 */
	unsigned int hash_md5(const std::string & sMd5)
	{
		char *p = (char *)sMd5.c_str();
		return (*(int*)(p)) ^ (*(int*)(p + 4)) ^ (*(int*)(p + 8)) ^ (*(int*)(p + 12));
	}

	/** @brief 哈希环节点列表，按哈希值排序 */
	std::vector<NFNode_T> vHashList;
};
