// -------------------------------------------------------------------------
//    @FileName         :    NFShmIdx.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    共享内存索引实现文件，提供对象索引的管理功能。
//                          该文件实现了共享内存索引的核心功能，包括索引对象的创建和初始化、
//                          索引与对象的关联管理、索引状态的管理、内存缓冲区的管理。
//                          主要功能包括索引初始化、对象关联管理、缓冲区管理、状态维护。
//                          设计特点包括基于共享内存支持跨进程、高效的对象关联、
//                          灵活的内存管理、状态一致性保证
//
// -------------------------------------------------------------------------

#include "NFShmIdx.h"
#include "NFComm/NFObjCommon/NFObject.h"

/**
 * @brief 构造函数
 * 
 * 初始化索引对象，根据共享内存模式选择初始化方式
 */
NFShmIdx::NFShmIdx()
{
	if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
	{
		CreateInit();
	}
	else
	{
		ResumeInit();
	}
}

/**
 * @brief 析构函数
 * 
 * 清理索引对象资源，重新初始化
 */
NFShmIdx::~NFShmIdx()
{
	Initialize();
}

/**
 * @brief 创建初始化
 * 
 * 在创建时进行初始化
 * 
 * @return 0 成功，其他值表示失败
 */
int NFShmIdx::CreateInit()
{
	Initialize();
	return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 在恢复时进行初始化
 * 
 * @return 0 成功，其他值表示失败
 */
int NFShmIdx::ResumeInit()
{
	return 0;
}

/**
 * @brief 初始化索引
 * 
 * 重置索引对象的所有成员变量为初始状态
 */
void NFShmIdx::Initialize()
{
	// 清空关联对象指针
	m_uEntity.m_pAttachedObj = nullptr;
	// 重置索引值为无效
	m_iIndex = -1;
}

/**
 * @brief 获取对象缓冲区
 * 
 * 获取索引关联的对象缓冲区指针
 * 
 * @return 对象缓冲区指针
 */
void* NFShmIdx::GetObjBuf() const
{
	return m_uEntity.m_pBuf;
}

/**
 * @brief 设置对象缓冲区
 * 
 * 设置索引关联的对象缓冲区指针
 * 
 * @param pBuf 对象缓冲区指针
 */
void NFShmIdx::SetObjBuf(void* pBuf)
{
	assert(pBuf);
	m_uEntity.m_pBuf = pBuf;
}

/**
 * @brief 获取关联对象
 * 
 * 获取索引关联的对象指针
 * 
 * @return 关联的对象指针
 */
NFObject* NFShmIdx::GetAttachedObj()
{
	return m_uEntity.m_pAttachedObj;
}

/**
 * @brief 获取关联对象（常量版本）
 * 
 * 获取索引关联的对象指针（只读）
 * 
 * @return 关联的对象指针
 */
const NFObject* NFShmIdx::GetAttachedObj() const
{
	return m_uEntity.m_pAttachedObj;
}

/**
 * @brief 设置关联对象
 * 
 * 设置索引关联的对象指针
 * 
 * @param pObj 要关联的对象指针
 */
void NFShmIdx::SetAttachedObj(NFObject* pObj)
{
	assert(pObj);
	m_uEntity.m_pAttachedObj = pObj;
}

