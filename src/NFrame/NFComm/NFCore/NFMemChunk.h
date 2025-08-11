// -------------------------------------------------------------------------
//    @FileName         :    NFMemChunk.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFMemChunk.h
 * @brief 内存块分配器
 * 
 * 此文件提供了高效的固定大小内存块分配器实现。
 * 将连续的内存分成大小相同的块，形成链表结构，
 * 能够快速分配和释放这些固定大小的内存块。
 */

#pragma once

#include <string>
#include <vector>
#include "NFPlatform.h"

/**
 * @brief 原始内存块分配器
 * 
 * NFMemChunk负责管理一块连续内存，将其分割为大小相同的块。
 * 每个块通过链表连接，支持快速的分配和释放操作。
 * 
 * 主要特性：
 * - 固定块大小：所有内存块大小相同，避免内存碎片
 * - 链表管理：使用链表连接空闲块，分配释放O(1)时间复杂度
 * - 内存复用：释放的内存块可以重复使用
 * - 批量分配：一次性分配大块内存，减少系统调用
 * - 共享内存支持：可用于共享内存环境
 * 
 * 设计原理：
 * - 内存布局：[头部信息][块1][块2]...[块N]
 * - 链表结构：空闲块通过指针相互连接
 * - 分配策略：从链表头取出一个空闲块
 * - 释放策略：将释放的块插入链表头
 * 
 * 适用场景：
 * - 对象池实现
 * - 内存池管理
 * - 高频分配/释放相同大小的对象
 * - 减少内存碎片的应用
 * - 实时系统中的确定性内存分配
 * 
 * 使用方法：
 * @code
 * // 计算所需内存大小
 * size_t blockSize = 64;
 * size_t blockCount = 1000;
 * size_t memSize = NFMemChunk::calcMemSize(blockSize, blockCount);
 * 
 * // 分配内存并创建内存块管理器
 * void* memory = malloc(memSize);
 * NFMemChunk chunk;
 * chunk.create(memory, blockSize, blockCount);
 * 
 * // 分配内存块
 * void* block1 = chunk.allocate();
 * void* block2 = chunk.allocate();
 * 
 * // 释放内存块
 * chunk.deallocate(block1);
 * chunk.deallocate(block2);
 * 
 * // 获取状态信息
 * size_t allocated = chunk.getAllocatedSize();
 * size_t available = chunk.getAvailableSize();
 * @endcode
 * 
 * @note 所有块的大小必须相同
 * @note 块大小必须足够存储指针（用于链表连接）
 * @note 线程安全性取决于使用方式，建议加锁保护
 */
class _NFExport NFMemChunk
{
public:

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个未初始化的内存块分配器。
	 * 需要调用create()或connect()方法进行初始化。
	 */
	NFMemChunk();

	/**
	 * @brief 计算所需的内存大小
	 * 
	 * 根据块大小和块数量计算总的内存需求。
	 * 包含头部信息和所有数据块的大小。
	 * 
	 * @param iBlockSize 每个内存块的大小（字节）
	 * @param iBlockCount 内存块的数量
	 * @return size_t 所需的总内存大小（字节）
	 * 
	 * @note 返回的大小包含管理头部的开销
	 * @note 块大小应当至少为sizeof(void*)以存储链表指针
	 */
	static size_t calcMemSize(size_t iBlockSize, size_t iBlockCount);

	/**
	 * @brief 计算可容纳的块数量
	 * 
	 * 根据可用内存大小和块大小计算最多能容纳多少个块。
	 * 
	 * @param iMemSize 可用的总内存大小（字节）
	 * @param iBlockSize 每个内存块的大小（字节）
	 * @return size_t 可容纳的最大块数量
	 * 
	 * @note 计算时会考虑头部信息的开销
	 */
	static size_t calcBlockCount(size_t iMemSize, size_t iBlockSize);

	/**
	 * @brief 获取头部信息大小
	 * 
	 * 返回内存块管理器头部信息所占用的字节数。
	 * 
	 * @return size_t 头部信息的大小（字节）
	 */
	static size_t getHeadSize() { return sizeof(tagChunkHead); }

	/**
	 * @brief 初始化内存块分配器
	 * 
	 * 在指定的内存地址上创建新的内存块分配器。
	 * 会清除原有数据并重新初始化所有结构。
	 * 
	 * @param pAddr 内存起始地址，必须足够大
	 * @param iBlockSize 每个内存块的大小（字节）
	 * @param iBlockCount 内存块的数量
	 * 
	 * @pre pAddr指向的内存大小至少为calcMemSize(iBlockSize, iBlockCount)
	 * @pre iBlockSize >= sizeof(void*) 用于存储链表指针
	 * @pre iBlockCount > 0
	 * 
	 * @note 此操作会覆盖内存中的现有数据
	 * @note 初始化后所有块都处于可分配状态
	 */
	void create(void *pAddr, size_t iBlockSize, size_t iBlockCount);

	/**
	 * @brief 连接到已存在的内存块分配器
	 * 
	 * 连接到已经初始化的内存块分配器，重用其状态。
	 * 
	 * @param pAddr 内存起始地址，应包含有效的头部信息
	 * 
	 * @pre 内存中包含有效的内存块分配器数据结构
	 * @note 不会修改现有的分配状态
	 * @note 适用于跨进程共享内存场景
	 */
	void connect(void *pAddr);

	/**
	 * @brief 获取每个块的大小
	 * 
	 * @return size_t 每个内存块的大小（字节）
	 */
	size_t getBlockSize() const { return _pHead->_iBlockSize; }

	/**
	 * @brief 获取总内存大小
	 * 
	 * 返回整个内存块分配器占用的总内存大小，
	 * 包括头部信息和所有数据块。
	 * 
	 * @return size_t 总内存大小（字节）
	 */
	size_t getMemSize() const { return _pHead->_iBlockSize * _pHead->_iBlockCount + sizeof(tagChunkHead); }

	/**
	 * @brief 获取数据存储容量
	 * 
	 * 返回可用于存储数据的总容量，不包括头部信息。
	 * 
	 * @return size_t 数据存储容量（字节）
	 */
	size_t getCapacity() const { return _pHead->_iBlockSize * _pHead->_iBlockCount; }

	/**
	 * @brief 获取总块数量
	 * 
	 * @return size_t 内存块的总数量
	 */
	size_t getBlockCount() const { return _pHead->_iBlockCount; }

	/**
	 * @brief 是否还有可用block
	 * @return 可用返回true，否则返回false
	 */
	bool isBlockAvailable() const { return _pHead->_blockAvailable > 0; }

	/**
	 * @brief  获取可以利用的block的个数
	 * @return 可用的block的个数
	 */
	size_t getBlockAvailableCount() const { return _pHead->_blockAvailable; }

	/**
	 * @brief 分配一个区块
	 *
	 * @return 指向分配的区块的指针
	 */
	void* allocate();

	/**
	 * @brief 分配一个区块.
	 * 返回以1为基数的区块索引，没有可分配空间时返回 0 ,
	 * 通查索引都是比较小(即使在64位操作系统上), 4个字节以内
	 * 便于节省内存
	 */
	void* allocate2(size_t &iIndex);

	/**
	 * @brief 释放区块
	 * @param 指向要释放区块的指针
	 */
	void deallocate(void *pAddr);

	/**
	 * @brief 根据索引释放区块
	 * @param 区块索引
	 */
	void deallocate2(size_t iIndex);

	/**
	 * @brief 重建
	 */
	void rebuild();

#if NF_PLATFORM == NF_PLATFORM_WIN
#pragma  pack (push,1)
	/**
	* @brief chunk头部
	*/
	struct tagChunkHead
	{
		size_t  _iBlockSize;            /**区块大小*/
		size_t  _iBlockCount;           /**block个数*/
		size_t  _firstAvailableBlock;   /**第一个可用的block索引*/
		size_t  _blockAvailable;        /**可用block个数*/
	};
#pragma pack(pop)
#else
	/**
	* @brief chunk头部
	*/
	struct tagChunkHead
	{
		size_t  _iBlockSize;            /**区块大小*/
		size_t  _iBlockCount;           /**block个数*/
		size_t  _firstAvailableBlock;   /**第一个可用的block索引*/
		size_t  _blockAvailable;        /**可用block个数*/
	}__attribute__((packed));
#endif

	/**
	* @brief 获取头部信息
	*
	* @return 头部信息
	*/
	tagChunkHead getChunkHead() const;

	/**
	* @brief 根据索引获取绝对地址
	*/
	void* getAbsolute(size_t iIndex);

	/**
	* @brief 绝对地址换成索引
	*
	* @param pAddr    绝对地址
	* @return size_t  索引值
	*/
	size_t getRelative(void *pAddr);

protected:
	/**
	* @brief 初始化
	*/
	void init(void *pAddr);

private:

	/**
	* @brief 区块头指针
	*/
	tagChunkHead    *_pHead;

	/**
	* @brief 数据区指针
	*/
	unsigned char   *_pData;
};

/**
* @brief 内存块分配器，提供分配和释放的功能
*
* 只能分配相同大小的内存块,最下层的原始内存块分配,
*
* 内存结构: 内存块长度, 4个字节 ;
*
* Block大 小, 4个字节;
*
* Chunk个数, 4个字节 ;
*
* TC_MemChunk 暂时只支持同一个Block大小的MemChunk
*/
class _NFExport NFMemChunkAllocator
{
public:

	/**
	* @brief 构造函数
	*/
	NFMemChunkAllocator();

	/**
	* @brief 初始化
	* @param pAddr, 地址, 换到应用程序的绝对地址
	* @param iSize, 内存大小
	* @param iBlockSize, block的大小
	*/
	void create(void *pAddr, size_t iSize, size_t iBlockSize);

	/**
	* @brief 连接
	* @param pAddr 地址, 换到应用程序的绝对地址
	*/
	void connect(void *pAddr);

	/**
	* @brief 获取头地址指针
	*/
	void *getHead()    const { return _pHead; }

	/**
	* @brief 每个block的大小
	*
	* @return block的大小
	*/
	size_t getBlockSize()  const { return _pHead->_iBlockSize; }

	/**
	* @brief 总计内存大小
	* @return 内存大小
	*/
	size_t getMemSize()  const { return _pHead->_iSize; }

	/**
	* @brief 可以存放数据的总容量
	* @return 总容量
	*/
	size_t getCapacity() const { return _chunk.getCapacity(); }

	/**
	* @brief 分配一个区块,绝对地址
	*/
	void* allocate();

	/**
	* @brief 分配一个区块，返回以1为基数的区块索引，
	*        没有可分配空间时返回0
	* @param 区块索引
	*/
	void* allocate2(size_t &iIndex);

	/**
	* @brief 释放区块, 绝对地址
	* @param pAddr 区块的绝对地址
	*/
	void deallocate(void *pAddr);

	/**
	* @brief 释放区块
	* @param iIndex 区块索引
	*/
	void deallocate2(size_t iIndex);

	/**
	* @brief 获取所有chunk的区块合计的block的个数
	* @return 合计的block的个数
	*/
	size_t blockCount() const { return _chunk.getBlockCount(); }

	/**
	* @brief 根据索引获取绝对地址
	* @param 索引
	*/
	void* getAbsolute(size_t iIndex) { return _chunk.getAbsolute(iIndex); };

	/**
	* @brief 绝对地址换成索引
	* @param pAddr   绝对地址
	* @return size_t 索引
	*/
	size_t getRelative(void *pAddr) { return _chunk.getRelative(pAddr); };

	/**
	* @brief 获取头部信息
	* @return 头部信息
	*/
	NFMemChunk::tagChunkHead getBlockDetail() const;

	/**
	* @brief 重建
	*/
	void rebuild();
#if NF_PLATFORM == NF_PLATFORM_WIN
#pragma  pack (push,1)
	/**
	* @brief 头部内存块
	*/
	struct tagChunkAllocatorHead
	{
		size_t  _iSize;
		size_t  _iBlockSize;
	};
#pragma pack(pop)
#else
	/**
	* @brief 头部内存块
	*/
	struct tagChunkAllocatorHead
	{
		size_t  _iSize;
		size_t  _iBlockSize;
	}__attribute__((packed));
#endif
	
	/**
	* @brief 取获头部大小
	* @return 头部大小
	*/
	static size_t getHeadSize() { return sizeof(tagChunkAllocatorHead); }

protected:

	/**
	* @brief 初始化
	*/
	void init(void *pAddr);

	/**
	* @brief 初始化
	*/
	void initChunk();

	/**
	* @brief 连接
	*/
	void connectChunk();

	/**
	*@brief 不允许copy构造
	*/
	NFMemChunkAllocator(const NFMemChunkAllocator &);
	/**
	*@brief 不允许赋值
	*/
	NFMemChunkAllocator& operator=(const NFMemChunkAllocator &);
	bool operator==(const NFMemChunkAllocator &mca) const;
	bool operator!=(const NFMemChunkAllocator &mca) const;

private:

	/**
	* 头指针
	*/
	tagChunkAllocatorHead   *_pHead;

	/**
	*  chunk开始的指针
	*/
	void                    *_pChunk;

	/**
	* chunk链表
	*/
	NFMemChunk             _chunk;
};

/**
* @brief 多块分配器,可以分配多个不同大小的块
*
* 内部每种块用NFMemChunkAllocator来分配,
*
* 每种大小不同块的个数是相同的, 内存块分配的策略如下:
*
*  确定需要分配多大内存，假设需要分配A字节的内存；
*
* 分配大小大于>=A的内存块，优先分配大小最接近的；
*
* 如果都没有合适内存块，则分配大小<A的内存块，优先分配大小最接近的；
*
* 如果仍然没有合适内存块，则返回NULL；
*
* 初始化时指定:最小块大 小, 最大块大小, 块间大小比值
*
* 自动计算出块的个数(每种大小块的个数相同)
*/
class _NFExport NFMemMultiChunkAllocator
{
public:

	/**
	* @brief 构造函数
	*/
	NFMemMultiChunkAllocator();

	/**
	* @brief 析够函数
	*/
	~NFMemMultiChunkAllocator();


	/**
	* @brief 初始化
	* @param pAddr          地址, 换到应用程序的绝对地址
	* @param iSize          内存大小
	* @param iMinBlockSize  block的大小下限
	* @param iMaxBlockSize  block的大小上限
	* @param fFactor        因子
	*/
	void create(void *pAddr, size_t iSize, size_t iMinBlockSize, size_t iMaxBlockSize, float fFactor = 1.1);

	/**
	* @brief 连接上
	* @param pAddr 地址, 换到应用程序的绝对地址
	*/
	void connect(void *pAddr);

	/**
	* @brief 扩展空间
	*
	* @param pAddr 已经是空间被扩展之后的地址
	* @param iSize
	*/
	void append(void *pAddr, size_t iSize);

	/**
	* @brief 获取每个block的大小, 包括后续增加的内存块的大小
	*
	* @return vector<size_t>block大小的vector
	*/
	std::vector<size_t> getBlockSize()  const;

	/**
	* @brief 每个block中chunk个数(都是相等的)
	* @return chunk个数
	*/
	size_t getBlockCount() const { return _iBlockCount; }

	/**
	* @brief 获取每个块头部信息, 包括后续增加的内存块的大小
	* @param i
	*
	* @return vector<TC_MemChunk::tagChunkHead>
	*/
	std::vector<NFMemChunk::tagChunkHead> getBlockDetail() const;

	/**
	* @brief 总计内存大小, 包括后续增加的内存块的大小
	*
	* @return size_t
	*/
	size_t getMemSize()  const { return _pHead->_iTotalSize; }

	/**
	* @brief 真正可以放数据的容量, 包括后续增加的内存块的数据容量
	* @return 可以放数据的容量
	*/
	size_t getCapacity() const;

	/**
	* @brief 一个chunk的block个数, 包括后续增加的内存块的
	* @return vector<size_t>block个数
	*/
	std::vector<size_t> singleBlockChunkCount() const;

	/**
	* @brief 所有chunk的区块合计的block的个数
	* @return 合计的block的个数
	*/
	size_t allBlockChunkCount() const;

	/**
	* @brief 分配一个区块,绝对地址
	* @param iNeedSize   需要分配的大小
	* @param iAllocSize  分配的数据块大小
	*/
	void* allocate(size_t iNeedSize, size_t &iAllocSize);

	/**
	* @brief 分配一个区块, 返回区块索引
	* @param iNeedSize    需要分配的大小
	* @param iAllocSize   分配的数据块大小
	* @param               size_t，以1为基数的索引，0表示无效
	*/
	void* allocate2(size_t iNeedSize, size_t &iAllocSize, size_t &iIndex);

	/**
	* @brief 释放区块
	* @param p 绝对地址
	*/
	void deallocate(void *pAddr);

	/**
	* @brief 释放区块
	* @param iIndex 区块索引
	*/
	void deallocate2(size_t iIndex);

	/**
	* @brief 重建
	*/
	void rebuild();

	/**
	* @brief 相对索引换算成绝对地址
	* @param iIndex 相对索引
	* @return       绝对地址指针
	*/
	void *getAbsolute(size_t iIndex);

	/**
	* @brief  绝对地址换成索引地址
	* @param  绝对地址
	* @return 索引地址
	*/
	size_t getRelative(void *pAddr);
#if NF_PLATFORM == NF_PLATFORM_WIN
#pragma  pack (push,1)
	/**
	* @brief 头部内存块
	*/
	struct tagChunkAllocatorHead
	{
		size_t  _iSize;             /**当前块大小*/
		size_t  _iTotalSize;        /**后续分配块合在一起的大小*/
		size_t  _iMinBlockSize;
		size_t  _iMaxBlockSize;
		float   _fFactor;
		size_t  _iNext;             /**下一个分配器地址, 如果没有则为0*/
	};
#pragma pack(pop)
#else
	/**
	* @brief 头部内存块
	*/
	struct tagChunkAllocatorHead
	{
		size_t  _iSize;             /**当前块大小*/
		size_t  _iTotalSize;        /**后续分配块合在一起的大小*/
		size_t  _iMinBlockSize;
		size_t  _iMaxBlockSize;
		float   _fFactor;
		size_t  _iNext;             /**下一个分配器地址, 如果没有则为0*/
	}__attribute__((packed));
#endif

	/**
	* @brief 头部大小
	*
	* @return size_t
	*/
	static size_t getHeadSize() { return sizeof(tagChunkAllocatorHead); }

protected:

	/**
	* @brief 初始化
	*/
	void init(void *pAddr);

	/**
	* @brief 计算
	*/
	void calc();

	/**
	* @brief 清空
	*/
	void clear();

	/**
	* @brief 最后一个分配器
	*
	* @return TC_MemMultiChunkAllocator*
	*/
	NFMemMultiChunkAllocator *lastAlloc();

	/**
	*@brief 不允许copy构造
	*/
	NFMemMultiChunkAllocator(const NFMemMultiChunkAllocator &);
	/**
	* @brief 不允许赋值
	*/
	NFMemMultiChunkAllocator& operator=(const NFMemMultiChunkAllocator &);
	bool operator==(const NFMemMultiChunkAllocator &mca) const;
	bool operator!=(const NFMemMultiChunkAllocator &mca) const;

private:

	/**
	* 头指针
	*/
	tagChunkAllocatorHead   *_pHead;

	/**
	*  chunk开始的指针
	*/
	void                    *_pChunk;

	/**
	* 区块大小
	*/
	std::vector<size_t>          _vBlockSize;

	/**
	* 每个chunk中block的个数
	*/
	size_t                  _iBlockCount;

	/**
	* chunk链表
	*/
	std::vector<NFMemChunkAllocator*>       _allocator;

	/**
	* 所有的索引个数
	*/
	size_t                              _iAllIndex;

	/**
	* 后续的多块分配器
	*/
	NFMemMultiChunkAllocator           *_nallocator;
};
