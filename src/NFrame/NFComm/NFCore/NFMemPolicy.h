// -------------------------------------------------------------------------
//    @FileName         :    NFMemPolicy.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFMemPolicy.h
 * @brief 内存存储策略模板类
 * 
 * 此文件定义了不同的内存存储策略，包括普通内存存储和共享内存存储。
 * 通过策略模式提供统一的接口，支持不同的存储后端，主要用于数据结构
 * 和容器的底层存储抽象。
 */

#pragma once

#include "NFPlatform.h"
#include "NFShm.h"

/**
 * @brief 内存存储策略模板类
 * 
 * NFMemStorePolicy提供了基于普通内存的存储策略实现。
 * 这是最基本的存储策略，数据存储在进程的虚拟内存空间中。
 * 
 * 主要特性：
 * - 快速访问：数据存储在进程内存中，访问速度最快
 * - 简单管理：无需考虑进程间同步问题
 * - 生命周期：随进程结束而消失
 * - 独立性：每个进程拥有独立的数据副本
 * 
 * 适用场景：
 * - 单进程应用的数据存储
 * - 临时数据缓存
 * - 不需要持久化的数据结构
 * - 对性能要求极高的场景
 * 
 * 使用方法：
 * @code
 * // 假设有一个支持内存策略的数据结构MyDataStructure
 * NFMemStorePolicy<MyDataStructure> memPolicy;
 * 
 * // 分配内存空间
 * void* memPtr = malloc(1024 * 1024);  // 1MB
 * 
 * // 创建数据结构
 * memPolicy.create(memPtr, 1024 * 1024);
 * 
 * // 或者连接到已有的内存空间
 * memPolicy.connect(memPtr, 1024 * 1024);
 * @endcode
 * 
 * @tparam T 要存储的数据结构类型，必须支持create和connect方法
 */
template<typename T>
class NFMemStorePolicy
{
public:
	/**
	 * @brief 在指定内存空间创建数据结构
	 * 
	 * 在给定的内存地址上初始化数据结构，通常用于首次创建。
	 * 
	 * @param pAddr 数据结构存储空间的指针，必须是有效的内存地址
	 * @param iSize 可用空间的大小（字节），必须足够容纳数据结构
	 * 
	 * @note 调用者负责确保内存空间的有效性和大小充足
	 * @note 此方法会初始化数据结构，清除原有内容
	 */
	void create(void *pAddr, size_t iSize)
	{
		_t.create(pAddr, iSize);
	}

	/**
	 * @brief 连接到已存在的数据结构
	 * 
	 * 连接到已经在内存中存在的数据结构，通常用于重新连接。
	 * 
	 * @param pAddr 数据结构存储空间的指针，指向已存在的数据结构
	 * @param iSize 可用空间的大小（字节）
	 * 
	 * @note 假设内存中已存在有效的数据结构
	 * @note 不会清除或重新初始化数据结构
	 */
	void connect(void *pAddr, size_t iSize)
	{
		_t.connect(pAddr, iSize);
	}

protected:
	/** @brief 存储的数据结构实例 */
	T   _t;
};

/**
 * @brief 共享内存存储策略模板类
 * 
 * ShmStorePolicy提供了基于共享内存的存储策略实现。
 * 数据存储在系统共享内存中，支持多进程访问。
 * 
 * 主要特性：
 * - 进程间共享：多个进程可以访问同一份数据
 * - 持久性：数据在进程重启后仍然存在
 * - 高效通信：避免了数据拷贝的开销
 * - 同步需求：需要外部机制保证数据一致性
 * 
 * 适用场景：
 * - 多进程数据共享
 * - 高性能进程间通信
 * - 大数据缓存共享
 * - 需要数据持久化的场景
 * 
 * 使用方法：
 * @code
 * // 假设有一个支持共享内存策略的数据结构MyDataStructure
 * ShmStorePolicy<MyDataStructure> shmPolicy;
 * 
 * // 初始化共享内存（如果不存在会创建，存在则连接）
 * key_t shmKey = 12345;
 * size_t size = 1024 * 1024;  // 1MB
 * shmPolicy.initStore(shmKey, size);
 * 
 * // 使用完毕后释放（可选，通常在程序退出时）
 * shmPolicy.release();
 * @endcode
 * 
 * @tparam T 要存储的数据结构类型，必须支持create和connect方法
 */
template<typename T>
class ShmStorePolicy
{
public:
	/**
	 * @brief 初始化共享内存存储
	 * 
	 * 根据指定的key和大小初始化共享内存。如果共享内存不存在，
	 * 会创建新的；如果已存在，则连接到现有的共享内存。
	 * 
	 * @param iShmKey 共享内存键值，用于标识唯一的共享内存段
	 * @param iSize 共享内存大小（字节），必须足够容纳数据结构
	 * 
	 * @throws NFShmException 当共享内存操作失败时抛出异常
	 * 
	 * @note 如果是新创建的共享内存，会调用数据结构的create方法
	 * @note 如果是连接到已存在的共享内存，会调用connect方法
	 */
	void initStore(key_t iShmKey, size_t iSize)
	{
		_shm.init(iSize, iShmKey);
		if (_shm.iscreate())
		{
			_t.create(_shm.getPointer(), iSize);
		}
		else
		{
			_t.connect(_shm.getPointer(), iSize);
		}
	}

	/**
	 * @brief 释放共享内存
	 * 
	 * 删除共享内存段，释放系统资源。
	 * 
	 * @warning 此操作会删除共享内存，其他进程将无法访问
	 * @warning 只有最后一个使用共享内存的进程应该调用此方法
	 * @note 通常在程序退出时调用，或者确定不再需要共享内存时调用
	 */
	void release()
	{
		_shm.del();
	}
	
protected:
	/** @brief 共享内存管理对象 */
	NFShm  _shm;
	/** @brief 存储的数据结构实例 */
	T       _t;
};


