// -------------------------------------------------------------------------
//    @FileName         :    NFConsistentHashNew.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFConsistentHashNew.h
 * @brief 新版一致性哈希算法实现
 * 
 * 此文件提供了一致性哈希算法的改进实现，支持多种哈希算法。
 * 一致性哈希是分布式系统中常用的负载均衡算法，能够在节点增减时
 * 最小化数据迁移，保持良好的分布性和平衡性。
 */

#pragma once

#include "NFPlatform.h"
#include "NFMD5.h"
#include "NFHash.hpp"
#include "NFCommon.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>

/**
 * @brief 一致性哈希节点结构
 * 
 * 表示一致性哈希环上的一个节点，包含节点的哈希值和索引。
 * 
 * 设计说明：
 * - 哈希值决定节点在哈希环上的位置
 * - 索引用于标识实际的物理节点
 * - 支持虚拟节点，一个物理节点可以对应多个虚拟节点
 */
struct NFNode_T_New
{
	/**
	 * @brief 节点哈希值
	 * 
	 * 节点在一致性哈希环上的位置，由哈希算法计算得出。
	 * 值越大，在哈希环上的位置越靠后。
	 */
	long iHashCode;

	/**
	 * @brief 节点下标
	 * 
	 * 实际物理节点的索引，用于标识具体的服务器或资源。
	 * 多个虚拟节点可以对应同一个物理节点索引。
	 */
	unsigned int iIndex;
};

/**
 * @brief 哈希算法类型枚举
 * 
 * 定义一致性哈希支持的不同哈希算法类型。
 * 不同的算法有不同的分布特性和性能表现。
 */
enum NF_HashAlgorithmType
{
	/** @brief Ketama哈希算法，具有良好的分布性 */
	E_TC_CONHASH_KETAMAHASH = 0,
	/** @brief 默认哈希算法，计算速度较快 */
	E_TC_CONHASH_DEFAULTHASH = 1
};

/**
 * @brief 哈希算法虚基类
 * 
 * 定义了一致性哈希算法的统一接口。
 * 通过继承此类可以实现不同的哈希算法策略。
 * 
 * 设计模式：策略模式
 * - 封装了哈希算法的实现细节
 * - 支持运行时切换不同的哈希算法
 * - 便于扩展新的哈希算法
 */
class NF_HashAlgorithm
{
public:
	/**
	 * @brief 计算键的哈希值
	 * 
	 * 纯虚函数，由具体的哈希算法实现。
	 * 
	 * @param sKey 要计算哈希值的键字符串
	 * @return long 计算得到的哈希值
	 * 
	 * @note 返回值应该是32位有效的正数
	 */
	virtual long hash(const std::string & sKey) = 0;
	
	/**
	 * @brief 获取哈希算法类型
	 * 
	 * 返回当前算法的类型标识。
	 * 
	 * @return NF_HashAlgorithmType 算法类型枚举值
	 */
	virtual NF_HashAlgorithmType getHashType() = 0;

protected:
	/**
	 * @brief 将哈希值截断为32位
	 * 
	 * 确保哈希值在32位范围内，避免溢出问题。
	 * 
	 * @param hash 原始哈希值
	 * @return long 截断后的32位哈希值
	 */
	long subTo32Bit(long hash)
	{
		return (hash & 0xFFFFFFFFL);
	}

};

/** @brief 哈希算法指针类型定义 */
typedef NF_HashAlgorithm* NF_HashAlgorithmPtr;

/**
 * @brief Ketama哈希算法实现
 * 
 * Ketama是一种广泛使用的一致性哈希算法，具有良好的分布性。
 * 该算法最初由Memcached团队开发，在分布式缓存系统中得到广泛应用。
 * 
 * 算法特点：
 * - 分布均匀性好：数据在节点间分布相对均匀
 * - 稳定性好：节点变化时数据迁移量最小
 * - 兼容性好：与标准Ketama算法兼容
 * 
 * 实现原理：
 * 1. 对键进行MD5哈希，得到16字节的哈希值
 * 2. 取前4个字节，按照little-endian方式组合成32位整数
 * 3. 截断为32位有效值
 */
class NF_KetamaHashAlg : public NF_HashAlgorithm
{
public:
	/**
	 * @brief 计算Ketama哈希值
	 * 
	 * 使用Ketama算法计算字符串的哈希值。
	 * 
	 * @param sKey 输入的键字符串
	 * @return long 计算得到的哈希值
	 * 
	 * 算法步骤：
	 * 1. 对键进行MD5计算得到二进制哈希
	 * 2. 取MD5结果的前4个字节
	 * 3. 按little-endian方式组合成32位整数
	 * 4. 截断为32位有效值返回
	 */
	virtual long hash(const std::string & sKey)
	{
		string sMd5 = NFMD5::md5bin(sKey);
		const char *p = (const char *)sMd5.c_str();

		long hash = ((long)(p[3] & 0xFF) << 24)
			| ((long)(p[2] & 0xFF) << 16)
			| ((long)(p[1] & 0xFF) << 8)
			| ((long)(p[0] & 0xFF));

		return subTo32Bit(hash);
	}

	/**
	 * @brief 获取算法类型
	 * 
	 * @return NF_HashAlgorithmType 返回Ketama算法类型
	 */
	virtual NF_HashAlgorithmType getHashType()
	{
		return E_TC_CONHASH_KETAMAHASH;
	}
};

/**
 * @brief 默认哈希算法实现
 * 
 * 提供一个计算速度较快的默认哈希算法实现。
 * 相比Ketama算法，计算速度更快，但分布性可能略差。
 * 
 * 算法特点：
 * - 计算速度快：使用简单的异或运算
 * - 实现简单：代码简洁，易于理解
 * - 分布性好：虽然不如Ketama，但仍有良好的分布性
 * 
 * 实现原理：
 * 1. 对键进行MD5哈希，得到16字节的哈希值
 * 2. 将16字节分成4个32位整数
 * 3. 对4个整数进行异或运算得到最终哈希值
 * 4. 截断为32位有效值
 */
class NF_DefaultHashAlg : public NF_HashAlgorithm
{
public:
	/**
	 * @brief 计算默认哈希值
	 * 
	 * 使用默认算法计算字符串的哈希值。
	 * 
	 * @param sKey 输入的键字符串
	 * @return long 计算得到的哈希值
	 * 
	 * 算法步骤：
	 * 1. 对键进行MD5计算得到二进制哈希
	 * 2. 将16字节MD5结果分成4个32位整数
	 * 3. 对4个整数进行异或运算
	 * 4. 截断为32位有效值返回
	 */
	virtual long hash(const std::string & sKey)
	{
		std::string sMd5 = NFMD5::md5bin(sKey);
		const char *p = (const char *)sMd5.c_str();

		long hash = (*(int*)(p)) ^ (*(int*)(p + 4)) ^ (*(int*)(p + 8)) ^ (*(int*)(p + 12));

		return subTo32Bit(hash);
	}

	/**
	 * @brief 获取算法类型
	 * 
	 * @return NF_HashAlgorithmType 返回默认算法类型
	 */
	virtual NF_HashAlgorithmType getHashType()
	{
		return E_TC_CONHASH_DEFAULTHASH;
	}
};

/**
 * @brief 哈希算法工厂类
 * 
 * NF_HashAlgFactory提供了创建不同类型哈希算法对象的工厂方法。
 * 使用工厂模式封装了哈希算法对象的创建过程，简化了客户端代码。
 * 
 * 设计模式：工厂模式
 * - 封装对象创建逻辑
 * - 支持多种算法类型
 * - 便于扩展新的算法
 */
class NF_HashAlgFactory
{
public:
	/**
	 * @brief 获取默认哈希算法
	 * 
	 * 创建并返回默认哈希算法对象的指针。
	 * 
	 * @return NF_HashAlgorithm* 默认哈希算法对象指针
	 * 
	 * @note 调用者负责释放返回的指针
	 * @note 默认使用NF_DefaultHashAlg算法
	 */
	static NF_HashAlgorithm *getHashAlg()
	{
		NF_HashAlgorithm *ptrHashAlg = new NF_DefaultHashAlg();

		return ptrHashAlg;
	}

	/**
	 * @brief 根据类型获取哈希算法
	 * 
	 * 根据指定的算法类型创建相应的哈希算法对象。
	 * 
	 * @param hashType 哈希算法类型
	 * @return NF_HashAlgorithm* 指定类型的哈希算法对象指针
	 * 
	 * @note 调用者负责释放返回的指针
	 * @note 不支持的类型会返回默认算法
	 */
	static NF_HashAlgorithm *getHashAlg(NF_HashAlgorithmType hashType)
	{
		NF_HashAlgorithm *ptrHashAlg = NULL;

		switch (hashType)
		{
		case E_TC_CONHASH_KETAMAHASH:
		{
			ptrHashAlg = new NF_KetamaHashAlg();
			break;
		}
		case E_TC_CONHASH_DEFAULTHASH:
		default:
		{
			ptrHashAlg = new NF_DefaultHashAlg();
			break;
		}
		}

		return ptrHashAlg;
	}
};

/**
 * @brief 新版一致性哈希算法实现类
 * 
 * NFConsistentHashNew提供了完整的一致性哈希算法实现，支持多种哈希算法。
 * 一致性哈希是分布式系统中用于数据分片和负载均衡的重要算法。
 * 
 * 主要特性：
 * - 支持多种哈希算法：Ketama、默认算法等
 * - 虚拟节点支持：每个物理节点可以有多个虚拟节点
 * - 动态节点管理：支持运行时添加和删除节点
 * - 高效查找：使用二分查找快速定位目标节点
 * - 良好的分布性：确保数据均匀分布到各个节点
 * 
 * 算法原理：
 * 1. 将所有节点（包括虚拟节点）根据哈希值排列在一个环上
 * 2. 对于任意键，计算其哈希值在环上的位置
 * 3. 顺时针查找第一个节点作为该键的目标节点
 * 4. 当节点增减时，只影响相邻节点间的数据，迁移量最小
 * 
 * 优势：
 * - 最小化数据迁移：节点变化时只需迁移少量数据
 * - 负载均衡：通过虚拟节点实现更好的负载分布
 * - 容错性强：单个节点故障不影响整体服务
 * - 扩展性好：可以动态添加或删除节点
 * 
 * 使用场景：
 * - 分布式缓存系统（如Redis集群）
 * - 分布式数据库分片
 * - 负载均衡器
 * - 分布式文件系统
 * - 微服务路由
 * 
 * 使用方法：
 * @code
 * // 创建一致性哈希对象
 * NFConsistentHashNew hash;
 * 
 * // 初始化（可选择哈希算法）
 * hash.Initialize(100, E_TC_CONHASH_KETAMAHASH); // 每个节点100个虚拟节点
 * 
 * // 添加节点
 * hash.AddNode("server1");
 * hash.AddNode("server2");
 * hash.AddNode("server3");
 * 
 * // 查找键对应的节点
 * int nodeIndex = hash.GetServerIndex("mykey");
 * std::string serverName = hash.GetServerNode("mykey");
 * 
 * // 删除节点
 * hash.DelNode("server2");
 * @endcode
 * 
 * @note 节点索引从0开始，按照添加顺序分配
 * @note 虚拟节点数量影响负载均衡效果，建议设置为100-200
 * @note 线程安全需要外部保证
 */
class _NFExport NFConsistentHashNew
{
public:
	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个空的一致性哈希对象，需要调用Initialize进行初始化。
	 */
	NFConsistentHashNew();
	
	/**
	 * @brief 析构函数
	 * 
	 * 清理内存，释放哈希算法对象。
	 */
	~NFConsistentHashNew();

	/**
	 * @brief 初始化一致性哈希
	 * 
	 * 设置虚拟节点数量和哈希算法类型。
	 * 
	 * @param iVirNodeCount 每个物理节点对应的虚拟节点数量
	 * @param iAlgType 哈希算法类型，默认为Ketama算法
	 * @return int 成功返回0，失败返回错误码
	 * 
	 * @pre iVirNodeCount应大于0，建议设置为100-200
	 * @note 虚拟节点数量越多，负载越均衡，但内存占用也越大
	 * @note 初始化后不建议修改，如需修改请重新创建对象
	 */
	int Initialize(unsigned int iVirNodeCount, NF_HashAlgorithmType iAlgType = E_TC_CONHASH_KETAMAHASH);

	/**
	 * @brief 添加节点
	 * 
	 * 向一致性哈希环中添加一个新的物理节点。
	 * 
	 * @param node 节点标识字符串，通常是服务器地址或名称
	 * @return int 成功返回节点索引（从0开始），失败返回-1
	 * 
	 * @pre node不能为空字符串
	 * @pre node不能与已存在的节点重复
	 * @post 节点被添加到哈希环中，同时创建对应的虚拟节点
	 * 
	 * 使用示例：
	 * @code
	 * int index1 = hash.AddNode("192.168.1.100:8080");
	 * int index2 = hash.AddNode("192.168.1.101:8080");
	 * @endcode
	 */
	int AddNode(const std::string & node);

	/**
	 * @brief 删除节点
	 * 
	 * 从一致性哈希环中删除指定的物理节点。
	 * 
	 * @param node 要删除的节点标识字符串
	 * @return int 成功返回0，失败返回-1
	 * 
	 * @pre node必须是已存在的节点
	 * @post 节点及其所有虚拟节点被从哈希环中移除
	 * @post 原本路由到该节点的键会重新路由到相邻节点
	 * 
	 * @note 删除节点会导致部分数据需要重新分布
	 */
	int DelNode(const std::string & node);

	/**
	 * @brief 获取键对应的节点索引
	 * 
	 * 根据键的哈希值查找其在一致性哈希环上对应的节点索引。
	 * 
	 * @param key 要查找的键字符串
	 * @return int 对应的节点索引，如果没有节点返回-1
	 * 
	 * @pre 必须已添加至少一个节点
	 * @note 相同的键总是返回相同的节点索引（除非节点发生变化）
	 * 
	 * 使用示例：
	 * @code
	 * int nodeIndex = hash.GetServerIndex("user:12345");
	 * if (nodeIndex >= 0) {
	 *     // 使用nodeIndex处理请求
	 * }
	 * @endcode
	 */
	int GetServerIndex(const std::string & key);

	/**
	 * @brief 获取键对应的节点名称
	 * 
	 * 根据键的哈希值查找其在一致性哈希环上对应的节点名称。
	 * 
	 * @param key 要查找的键字符串
	 * @return std::string 对应的节点名称，如果没有节点返回空字符串
	 * 
	 * @pre 必须已添加至少一个节点
	 * @note 这是GetServerIndex的便捷版本，直接返回节点名称
	 */
	std::string GetServerNode(const std::string & key);

	/**
	 * @brief 获取节点数量
	 * 
	 * 返回当前一致性哈希环中的物理节点数量。
	 * 
	 * @return size_t 物理节点数量
	 * 
	 * @note 不包括虚拟节点数量
	 */
	size_t Size();

	/**
	 * @brief 打印哈希环信息
	 * 
	 * 输出当前哈希环的详细信息，用于调试和诊断。
	 * 包括所有节点（物理节点和虚拟节点）的哈希值和位置信息。
	 * 
	 * @note 仅用于调试目的，生产环境不建议使用
	 */
	void PrintHashRing();

	/**
	* @brief 节点比较.
	*
	* @param m1 node_T_new类型的对象，比较节点之一
	* @param m2 node_T_new类型的对象，比较节点之一
	* @return less or not 比较结果，less返回ture，否则返回false
	*/
	static bool less_hash(const NFNode_T_New & m1, const NFNode_T_New & m2)
	{
		return m1.iHashCode < m2.iHashCode;
	}

	/**
	* @brief 增加节点.
	*
	* @param node  节点名称
	* @param index 节点的下标值
	* @return      节点的hash值
	*/
	int sortNode()
	{
		sort(_vHashList.begin(), _vHashList.end(), less_hash);

		return 0;
	}

	/**
	* @brief 打印节点信息
	*
	*/
	void printNode()
	{
		std::map<unsigned int, unsigned int> mapNode;
		size_t size = _vHashList.size();

		for (size_t i = 0; i < size; i++)
		{
			if (i == 0)
			{
				unsigned int value = 0xFFFFFFFF - _vHashList[size - 1].iHashCode + _vHashList[0].iHashCode;
				mapNode[_vHashList[0].iIndex] = value;
			}
			else
			{
				unsigned int value = _vHashList[i].iHashCode - _vHashList[i - 1].iHashCode;

				if (mapNode.find(_vHashList[i].iIndex) != mapNode.end())
				{
					value += mapNode[_vHashList[i].iIndex];
				}

				mapNode[_vHashList[i].iIndex] = value;
			}

			std::cout << "printNode: " << _vHashList[i].iHashCode << "|" << _vHashList[i].iIndex << "|" << mapNode[_vHashList[i].iIndex] << std::endl;
		}

		std::map<unsigned int, unsigned int>::iterator it = mapNode.begin();
		double avg = 100;
		double sum = 0;

		while (it != mapNode.end())
		{
			double tmp = it->second;
			std::cerr << "result: " << it->first << "|" << it->second << "|" << (tmp * 100 * mapNode.size() / 0xFFFFFFFF - avg) << std::endl;
			sum += (tmp * 100 * mapNode.size() / 0xFFFFFFFF - avg) * (tmp * 100 * mapNode.size() / 0xFFFFFFFF - avg);
			it++;
		}

		std::cerr << "variance: " << sum / mapNode.size() << ", size: " << _vHashList.size() << std::endl;
	}

	/**
	* @brief 增加节点.
	*
	* @param node  节点名称
	* @param index 节点的下标值
	* @param weight 节点的权重，默认为1
	* @return      是否成功
	*/
	int addNode(const std::string & node, unsigned int index, int weight = 1)
	{
		if (_ptrHashAlg == NULL)
		{
			return -1;
		}

		NFNode_T_New stItem;
		stItem.iIndex = index;

		for (int j = 0; j < weight; j++)
		{
			std::string virtualNode = node + "_" + NFCommon::tostr<int>(j);

			// TODO: 目前写了2 种hash 算法，可以根据需要选择一种，
			// TODO: 其中KEMATA 为参考memcached client 的hash 算法，default 为原有的hash 算法，测试结论在表格里有
			if (_ptrHashAlg->getHashType() == E_TC_CONHASH_KETAMAHASH)
			{
				std::string sMd5 = NFMD5::md5bin(virtualNode);
				char *p = (char *)sMd5.c_str();

				for (int i = 0; i < 4; i++)
				{
					stItem.iHashCode = ((long)(p[i * 4 + 3] & 0xFF) << 24)
						| ((long)(p[i * 4 + 2] & 0xFF) << 16)
						| ((long)(p[i * 4 + 1] & 0xFF) << 8)
						| ((long)(p[i * 4 + 0] & 0xFF));
					stItem.iIndex = index;
					_vHashList.push_back(stItem);
				}
			}
			else
			{
				stItem.iHashCode = _ptrHashAlg->hash(virtualNode);
				_vHashList.push_back(stItem);
			}
		}

		return 0;
	}

	/**
	* @brief 获取某key对应到的节点node的下标.
	*
	* @param key      key名称
	* @param iIndex  对应到的节点下标
	* @return        0:获取成功   -1:没有被添加的节点
	*/
	int getIndex(const std::string & key, unsigned int & iIndex)
	{
		if (_ptrHashAlg == NULL || _vHashList.size() == 0)
		{
			iIndex = 0;
			return -1;
		}

		long iCode = _ptrHashAlg->hash(NFMD5::md5bin(key));

		return getIndex(iCode, iIndex);
	}

	/**
	* @brief 获取某hashcode对应到的节点node的下标.
	*
	* @param hashcode      hashcode
	* @param iIndex  对应到的节点下标
	* @return        0:获取成功   -1:没有被添加的节点
	*/
	int getIndex(long hashcode, unsigned int & iIndex)
	{
		if (_ptrHashAlg == NULL || _vHashList.size() == 0)
		{
			iIndex = 0;
			return -1;
		}

		// 只保留32位
		long iCode = (hashcode & 0xFFFFFFFFL);

		int low = 0;
		int high = _vHashList.size();

		if (iCode <= _vHashList[0].iHashCode || iCode > _vHashList[high - 1].iHashCode)
		{
			iIndex = _vHashList[0].iIndex;
			return 0;
		}

		while (low < high - 1)
		{
			int mid = (low + high) / 2;
			if (_vHashList[mid].iHashCode > iCode)
			{
				high = mid;
			}
			else
			{
				low = mid;
			}
		}
		iIndex = _vHashList[low + 1].iIndex;
		return 0;
	}

	/**
	* @brief 获取当前hash列表的长度.
	*
	* @return        长度值
	*/
	size_t size()
	{
		return _vHashList.size();
	}

	/**
	* @brief 清空当前的hash列表.
	*
	*/
	void clear()
	{
		_vHashList.clear();
	}

protected:
	std::vector<NFNode_T_New>    _vHashList;
	NF_HashAlgorithmPtr _ptrHashAlg;
};