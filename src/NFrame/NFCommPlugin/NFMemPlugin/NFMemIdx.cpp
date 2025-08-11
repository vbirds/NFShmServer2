// -------------------------------------------------------------------------
//    @FileName         :    NFMemIdx.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    内存索引实现文件，负责管理内存中的对象索引，包括索引对象的创建和初始化、
//                          索引与对象的关联管理、索引状态的管理、内存缓冲区的管理。
//                          该文件实现了NFShmXFrame框架的内存索引类，提供索引初始化、对象关联管理、
//                          缓冲区管理、状态维护等功能。
//                          主要功能包括高效的对象关联、灵活的内存管理、状态一致性保证
//
// -------------------------------------------------------------------------

#include "NFMemIdx.h"
#include "NFComm/NFObjCommon/NFObject.h"

/**
 * @brief 构造函数
 * 
 * 在构造时调用初始化函数，设置默认值
 */
NFMemIdx::NFMemIdx()
{
	Initialize();
}

/**
 * @brief 析构函数
 * 
 * 在析构时调用初始化函数，清理资源
 */
NFMemIdx::~NFMemIdx()
{
	Initialize();
}

/**
 * @brief 初始化函数
 * 
 * 该函数用于初始化内存索引对象，设置默认值：
 * - 将关联对象指针设置为nullptr
 * - 将索引值设置为-1
 */
void NFMemIdx::Initialize()
{
	m_uEntity.m_pAttachedObj = nullptr;
    m_iIndex = -1;
}

/**
 * @brief 获取对象缓冲区指针
 * 
 * 该函数返回索引关联的对象缓冲区指针。
 * 
 * @return 对象缓冲区指针
 */
void* NFMemIdx::GetObjBuf() const
{
	return m_uEntity.m_pBuf;
}

/**
 * @brief 设置对象缓冲区指针
 * 
 * 该函数设置索引关联的对象缓冲区指针。
 * 
 * @param pBuf 对象缓冲区指针
 */
void NFMemIdx::SetObjBuf(void* pBuf)
{
	assert(pBuf);
	m_uEntity.m_pBuf = pBuf;
}

/**
 * @brief 获取关联的对象指针
 * 
 * 该函数返回索引关联的对象指针。
 * 
 * @return 关联的对象指针
 */
NFObject* NFMemIdx::GetAttachedObj()
{
	return m_uEntity.m_pAttachedObj;
}

/**
 * @brief 获取关联的对象指针（常量版本）
 * 
 * 该函数返回索引关联的对象指针的常量版本。
 * 
 * @return 关联的对象指针
 */
const NFObject* NFMemIdx::GetAttachedObj() const
{
	return m_uEntity.m_pAttachedObj;
}

/**
 * @brief 设置关联的对象指针
 * 
 * 该函数设置索引关联的对象指针。
 * 
 * @param pObj 要关联的对象指针
 */
void NFMemIdx::SetAttachedObj(NFObject* pObj)
{
	assert(pObj);
	m_uEntity.m_pAttachedObj = pObj;
}

