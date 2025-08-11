// -------------------------------------------------------------------------
//    @FileName         :    NFMemVector.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFMemVector.h
 * @brief 共享内存向量容器
 * 
 * 此文件提供了基于共享内存的随机访问数据块容器实现。
 * 支持在共享内存中存储固定大小的对象，提供类似std::vector的接口，
 * 但数据存储在共享内存中，可以在多个进程间共享。
 */

#pragma once

#include <sstream>
#include <string.h>
#include "NFPlatform.h"
#include "NFException.hpp"

/**
 * @brief 共享内存向量异常类
 * 
 * 用于NFMemVector操作中可能发生的异常情况。
 * 继承自NFException，提供共享内存操作相关的错误信息。
 */
struct NF_MemVectorException : public NFException
{
	/**
	 * @brief 构造函数
	 * 
	 * @param buffer 异常描述信息
	 */
	NF_MemVectorException(const string &buffer) : NFException(buffer) {};
	
	/**
	 * @brief 构造函数（带错误码）
	 * 
	 * @param buffer 异常描述信息
	 * @param err 错误码
	 */
	NF_MemVectorException(const string &buffer, int err) : NFException(buffer, err) {};
	
	/**
	 * @brief 析构函数
	 */
	~NF_MemVectorException() throw() {};
};

/**
 * @brief 共享内存随机访问数据块容器
 * 
 * NFMemVector提供了基于共享内存的向量容器实现，每块数据大小相等。
 * 模板对象T只能是简单的数据类型，需要具备bit-copy语义。
 * 
 * 主要特性：
 * - 共享内存存储：数据存储在共享内存中，支持多进程访问
 * - 固定大小元素：所有元素都是相同大小的简单数据类型
 * - 随机访问：支持通过索引快速访问元素
 * - STL兼容：提供类似std::vector的接口和迭代器
 * - 内存管理：自动管理共享内存的分配和释放
 * 
 * 设计限制：
 * - 模板类型T必须是POD类型（Plain Old Data）
 * - 不支持包含指针的复杂对象
 * - 元素大小在创建时确定，不可变更
 * - 容量在创建时固定，不支持动态扩容
 * 
 * 适用场景：
 * - 多进程数据共享
 * - 高性能数据缓存
 * - 进程间通信缓冲区
 * - 大数据集的内存映射存储
 * - 服务器集群数据同步
 * 
 * 使用方法：
 * @code
 * // 创建共享内存向量
 * NFMemVector<int> vec;
 * 
 * // 计算所需内存大小
 * size_t memSize = NFMemVector<int>::calcMemSize(1000);
 * 
 * // 分配内存并创建向量
 * void* memory = malloc(memSize);
 * vec.create(memory, memSize);
 * 
 * // 添加元素
 * int value = 42;
 * vec.push_back(value);
 * 
 * // 访问元素
 * int first = vec[0];
 * 
 * // 使用迭代器
 * for (auto it = vec.begin(); it != vec.end(); ++it) {
 *     std::cout << *it << " ";
 * }
 * 
 * // 连接到已存在的共享内存
 * NFMemVector<int> vec2;
 * vec2.connect(memory);
 * @endcode
 * 
 * @tparam T 存储的数据类型，必须是POD类型
 * 
 * @note T类型必须支持bit-copy语义
 * @note 不支持包含指针或复杂对象的类型
 * @note 线程安全性取决于具体的使用场景和同步机制
 */
template<typename T>
class NFMemVector
{
public:

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个未初始化的共享内存向量。
	 * 需要调用create()或connect()方法进行初始化。
	 */
	NFMemVector() : _pHead(NULL), _pData(NULL)
	{
	}

	/**
	 * @brief 拷贝构造函数
	 * 
	 * 创建一个与现有向量共享相同内存的新向量实例。
	 * 
	 * @param mv 要拷贝的源向量
	 * 
	 * @note 这是浅拷贝，两个对象将指向相同的共享内存
	 */
	NFMemVector(const NFMemVector<T> &mv)
		: _pHead(mv._pHead), _pData(mv._pData)
	{

	}

	/**
	 * @brief 相等比较操作符
	 * 
	 * 比较两个向量是否指向相同的共享内存。
	 * 
	 * @param mv 要比较的向量
	 * @return bool 指向相同内存时返回true
	 */
	bool operator==(const NFMemVector<T> &mv)
	{
		return _pHead == mv._pHead && _pData == mv._pData;
	}

	/**
	 * @brief 不等比较操作符
	 * 
	 * 比较两个向量是否指向不同的共享内存。
	 * 
	 * @param mv 要比较的向量
	 * @return bool 指向不同内存时返回true
	 */
	bool operator!=(const NFMemVector<T> &mv)
	{
		return _pHead != mv._pHead || _pData != mv._pData;
	}

	/**
	 * @brief 计算所需的内存空间大小
	 * 
	 * 根据要存储的元素数量计算所需的共享内存大小。
	 * 
	 * @param iCount 要存储的元素数量
	 * @return size_t 所需的内存字节数
	 * 
	 * @note 返回的大小包含头部信息和所有元素的存储空间
	 */
	static size_t calcMemSize(size_t iCount)
	{
		return sizeof(T) * iCount + sizeof(tagMemQueueHead);
	}

	/**
	 * @brief 初始化共享内存向量
	 * 
	 * 在指定的内存空间中创建新的共享内存向量。
	 * 
	 * @param pAddr 共享内存空间的指针
	 * @param iSize 内存空间的大小（字节数）
	 * 
	 * @throws NF_MemVectorException 当内存空间不足时抛出异常
	 * 
	 * @note 会清空现有数据并重新初始化
	 * @note 内存空间必须足够存储至少一个元素
	 */
	void create(void *pAddr, size_t iSize);

	/**
	 * @brief 连接到已存在的共享内存向量
	 * 
	 * 连接到已经初始化的共享内存向量，共享其数据。
	 * 
	 * @param pAddr 共享内存空间的指针
	 * 
	 * @note 假设内存中已经包含有效的向量数据
	 * @note 会验证元素大小是否匹配当前模板类型
	 */
	void connect(void *pAddr) { init(pAddr); assert(_pHead->_iBlockSize == sizeof(T)); }

	/**
	 * @brief 获取向量中元素的数量
	 * 
	 * @return size_t 当前存储的元素数量
	 */
	size_t size() { return _pHead->_iBlockCount; }

	/**
	 * @brief 获取共享内存的总大小
	 * 
	 * @return size_t 共享内存的总字节数
	 */
	size_t getMemSize() { return _pHead->_iSize; }

	/**
	 * @brief 清空向量并重建
	 * 
	 * 移除所有元素，将向量重置为空状态。
	 * 
	 * @note 不释放内存，只是将元素计数重置为0
	 */
	void clear();

	/**
	 * @brief 获取向量的描述信息
	 * 
	 * 返回包含向量状态信息的字符串，用于调试和诊断。
	 * 
	 * @return string 包含向量大小、容量等信息的描述字符串
	 */
	string desc() const;

	/**
	 * @brief 共享内存向量迭代器
	 * 
	 * 提供对共享内存向量元素的随机访问迭代器。
	 * 符合STL随机访问迭代器的要求。
	 */
	class NF_MemVectorIterator : public std::iterator<std::random_access_iterator_tag, T>
	{
	public:
		/**
		 * @brief 迭代器构造函数
		 * 
		 * @param pmv 指向向量的指针
		 * @param iIndex 当前迭代器指向的元素索引
		 */
		NF_MemVectorIterator(NFMemVector *pmv, size_t iIndex) : _pmv(pmv), _iIndex(iIndex)
		{
		}

		/**
		 * @brief 前置递增操作符
		 * 
		 * 将迭代器向前移动一个位置。
		 * 
		 * @return NF_MemVectorIterator& 递增后的迭代器引用
		 */
		NF_MemVectorIterator& operator++()
		{
			++_iIndex;
			return *this;
		}

		/**
		 * @brief 后置递增操作符
		 * 
		 * 将迭代器向前移动一个位置，返回移动前的副本。
		 * 
		 * @return NF_MemVectorIterator 移动前的迭代器副本
		 */
		NF_MemVectorIterator operator++(int)
		{
			NF_MemVectorIterator tmp = *this;

			++_iIndex;
			return tmp;
		}

		/**
		 * @brief 相等比较操作符
		 * 
		 * 比较两个迭代器是否指向相同的位置。
		 * 
		 * @param mv 要比较的迭代器
		 * @return bool 指向相同位置时返回true
		 */
		bool operator==(const NF_MemVectorIterator& mv)
		{
			return _iIndex == mv._iIndex && _pmv == mv._pmv;
		}

		/**
		 * @brief 不等比较操作符
		 * 
		 * 比较两个迭代器是否指向不同的位置。
		 * 
		 * @param mv 要比较的迭代器
		 * @return bool 指向不同位置时返回true
		 */
		bool operator!=(const NF_MemVectorIterator& mv)
		{
			return _iIndex != mv._iIndex || _pmv != mv._pmv;
		}

		/**
		 * @brief 解引用操作符
		 * 
		 * 获取迭代器当前指向的元素的引用。
		 * 
		 * @return T& 当前元素的引用
		 */
		T& operator*() const { return (*_pmv)[_iIndex]; }

		/**
		 * @brief
		 *
		 * @return T*
		 */
		T* operator->() const { return &(*_pmv)[_iIndex]; }

	private:
		/**
		*
		*/
		NFMemVector    *_pmv;

		/**
		*
		*/
		size_t          _iIndex;
	};

	typedef NF_MemVectorIterator iterator;

	/**
	*
	*
	* @return TC_MemVectorIterator
	*/
	NF_MemVectorIterator begin() { return NF_MemVectorIterator(this, 0); }

	/**
	*
	*
	* @return TC_MemVectorIterator
	*/
	NF_MemVectorIterator end() { return NF_MemVectorIterator(this, _pHead->_iBlockCount); }

	/**
	* @brief 获取数据
	* @param pData
	* @param iDataLen
	*/
	T& operator[](size_t iIndex)
	{
		if (iIndex >= _pHead->_iBlockCount)
		{
			ostringstream s;
			s << string("[TC_MemVector::get] index beyond : index = ") << iIndex << " > " << _pHead->_iBlockCount;

			throw NF_MemVectorException(s.str());
		}

		return *(T*)((char*)_pData + iIndex * _pHead->_iBlockSize);
	}

	/**
	* @brief 获取头地址
	*
	* @return void*
	*/
	void *getAddr() { return (void*)_pHead; }

#if NF_PLATFORM == NF_PLATFORM_WIN
#pragma  pack (push,1)
	/**
	*  @brief 队列控制结构
	*/
	struct tagMemQueueHead
	{
		size_t _iSize;          //内存大小
		size_t _iBlockCount;    //元素个数
		size_t _iBlockSize;     //区块大小
	};
#pragma pack(pop)
#else
	/**
	*  @brief 队列控制结构
	*/
	struct tagMemQueueHead
	{
		size_t _iSize;          //内存大小
		size_t _iBlockCount;    //元素个数
		size_t _iBlockSize;     //区块大小
	}__attribute__((packed));
#endif

protected:

	/**
	* @brief
	* @param pAddr
	*/
	void init(void *pAddr)
	{
		_pHead = static_cast<tagMemQueueHead*>(pAddr);
		_pData = (char*)_pHead + sizeof(tagMemQueueHead);
	}


	/**
	* 队列控制快
	*/
	tagMemQueueHead *_pHead;

	/**
	* 共享内存地址
	*/
	void            *_pData;
};

template<typename T>
void NFMemVector<T>::create(void *pAddr, size_t iSize)
{
	size_t iBlockSize = sizeof(T);

	if (iSize <= sizeof(tagMemQueueHead) || ((iSize - sizeof(tagMemQueueHead)) / iBlockSize == 0))
	{
		throw NF_MemVectorException("[TC_MemVector::create] memory size not enough.");
	}

	init(pAddr);

	memset(pAddr, 0x00, iSize);

	_pHead->_iSize = iSize;
	_pHead->_iBlockCount = (iSize - sizeof(tagMemQueueHead)) / iBlockSize;
	_pHead->_iBlockSize = iBlockSize;
}

template<typename T>
void NFMemVector<T>::clear()
{
	assert(_pHead);

	memset(_pData, 0x00, _pHead->_iBlockSize * _pHead->_iBlockCount);
}

template<typename T>
string NFMemVector<T>::desc() const
{
	ostringstream s;
	s << "[TC_MemVector] [_iSize=" << _pHead->_iSize << "] "
		<< "[_iBlockCount=" << _pHead->_iBlockCount << "] "
		<< "[_iBlockSize=" << _pHead->_iBlockSize << "] "
		<< endl;
	s << "[~TC_MemVector]";

	return s.str();
}
