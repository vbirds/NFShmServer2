// -------------------------------------------------------------------------
//    @FileName         :    NFMemIdx.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    内存索引头文件，负责管理内存中的对象索引，包括索引对象的创建和初始化、
//                          索引与对象的关联管理、索引状态的管理、内存缓冲区的管理。
//                          该文件定义了NFShmXFrame框架的内存索引类，提供索引初始化、对象关联管理、
//                          缓冲区管理、状态维护等功能。
//                          主要功能包括高效的对象关联、灵活的内存管理、状态一致性保证
//
// -------------------------------------------------------------------------
#pragma once

#define MAXDSINFOITEM  2
#define IDXNULL			-1
#include <NFComm/NFCore/NFHash.hpp>

class NFObject;

/**
 * @brief 内存索引类，负责管理内存中的对象索引
 * 
 * 该类负责管理内存中的对象索引，包括索引对象的创建和初始化、
 * 索引与对象的关联管理、索引状态的管理、内存缓冲区的管理。
 * 提供索引初始化、对象关联管理、缓冲区管理、状态维护等功能。
 * 主要功能包括高效的对象关联、灵活的内存管理、状态一致性保证。
 */
class NFMemIdx
{
public:
	/**
	 * @brief 默认构造函数
	 */
	NFMemIdx();

	/**
	 * @brief 拷贝构造函数
	 * @param idx 要拷贝的索引对象
	 */
	NFMemIdx(const NFMemIdx& idx)
	{
		if (this != &idx)
		{
			m_uEntity.m_pBuf = idx.m_uEntity.m_pBuf;
			m_iIndex = idx.m_iIndex;
		}
	}

	/**
	 * @brief 析构函数
	 */
	~NFMemIdx();

	/**
	 * @brief 初始化索引对象
	 */
	void Initialize();
	
	/**
	 * @brief 获取关联的对象
	 * @return 关联的对象指针
	 */
	NFObject* GetAttachedObj();
	
	/**
	 * @brief 获取关联的对象（常量版本）
	 * @return 关联的对象指针
	 */
	const NFObject* GetAttachedObj() const;
	
	/**
	 * @brief 设置关联的对象
	 * @param pObj 要关联的对象指针
	 */
	void SetAttachedObj(NFObject* pObj);
	
	/**
	 * @brief 获取对象缓冲区
	 * @return 对象缓冲区指针
	 */
	void* GetObjBuf() const;
	
	/**
	 * @brief 设置对象缓冲区
	 * @param pBuf 对象缓冲区指针
	 */
	void SetObjBuf(void* pBuf);
	
	/**
	 * @brief 获取索引值
	 * @return 索引值
	 */
	int GetIndex() const { return m_iIndex; }
	
	/**
	 * @brief 设置索引值
	 * @param iIndex 索引值
	 */
	void SetIndex(int iIndex) { m_iIndex = iIndex; }

	/**
	 * @brief 赋值操作符
	 * @param idx 要赋值的索引对象
	 * @return 当前对象的引用
	 */
	NFMemIdx& operator=(const NFMemIdx& idx)
	{
		if (this != &idx)
		{
			m_uEntity.m_pBuf = idx.m_uEntity.m_pBuf;
			m_iIndex = idx.m_iIndex;
		}

		return *this;
	}

private:
	/**
	 * @brief 联合体，用于存储对象指针或缓冲区指针
	 */
	union
	{
		NFObject* m_pAttachedObj; ///< 关联的对象指针
		void* m_pBuf;             ///< 对象缓冲区指针
	} m_uEntity;

	int m_iIndex; ///< 索引值
};


