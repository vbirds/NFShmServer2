// -------------------------------------------------------------------------
//    @FileName         :    NFCSharedMem.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//    @Desc             :    共享内存封装类实现文件，提供跨平台共享内存管理功能。
//                          该文件实现了跨平台共享内存管理功能，包括共享内存的创建和连接、
//                          内存段管理、内存使用统计、安全销毁机制、版本校验功能。
//                          主要功能包括跨平台共享内存操作、内存分配和释放、
//                          内存状态监控、安全机制保护
//
// -------------------------------------------------------------------------

#include "NFCSharedMem.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFCore/NFServerTime.h"

#define MAYEX_SHM_CAN_BE_DESTROY_SAFE_MAGIC 123456789  ///< 安全销毁魔法数
#define MAYEX_SHM_INIT_SUCCESS_MAGIC 987654321          ///< 初始化成功魔法数
#define MAGIC_SERVER_VER 100001                         ///< 服务器版本魔法数

char *NFCSharedMem::pbCurrentShm = NULL;                ///< 当前共享内存块指针
bool NFCSharedMem::s_bCheckInitSuccessFlag = false;     ///< 共享内存初始标志

/**
 * @brief 获得状态
 * 
 * 获取当前共享内存的初始化模式
 * 
 * @return 初始化模式
 */
EN_OBJ_MODE NFCSharedMem::GetInitMode()
{
	return m_enRunMode;
}

/**
 * @brief 获得状态
 * 
 * 设置共享内存的初始化模式
 * 
 * @param mode 初始化模式
 */
void  NFCSharedMem::SetInitMode(EN_OBJ_MODE mode)
{
	m_enRunMode = mode;
}

/**
 * @brief new一块内存
 * 
 * 重载new操作符，从当前共享内存块分配内存
 * 
 * @param siSize 内存大小
 * @return 内存指针
 */
void *NFCSharedMem::operator new(size_t siSize) throw()
{
	char* pTemp = NULL;

	if (!pbCurrentShm)
	{
		return NULL;
	}

	pTemp = pbCurrentShm;
	return (void *)pTemp;
}

/**
 * @brief 释放一块内存
 * 
 * 重载delete操作符，释放内存
 * 
 * @param pMem 内存指针
 */
void NFCSharedMem::operator delete(void *pMem)
{
    std::cout << "delete ..." << std::endl;
}

/**
 * @brief 构造函数
 * 
 * 根据平台创建共享内存对象
 * 
 * @param nKey 共享内存键值
 * @param siSize 共享内存大小
 * @param enInitFlag 初始化标志
 * @param shemID 共享内存ID
 */
#if NF_PLATFORM == NF_PLATFORM_WIN
NFCSharedMem::NFCSharedMem(key_t nKey, size_t siSize, EN_OBJ_MODE enInitFlag, HANDLE shemID)
#else
NFCSharedMem::NFCSharedMem(key_t nKey, size_t siSize, EN_OBJ_MODE enInitFlag, int shemID)
#endif
{
#if NF_PLATFORM == NF_PLATFORM_WIN
	m_shemID = shemID;
#else
	m_shemID = shemID;
#endif
	m_pbCurrentSegMent = pbCurrentShm + sizeof(NFCSharedMem);

	if (EN_OBJ_MODE_RECOVER == enInitFlag)
	{
		if (m_iServerCheckVersion != MAGIC_SERVER_VER)
		{
			NFLogError(NF_LOG_DEFAULT, 0, "shm check version error,need version {},cur version {}\n", m_iServerCheckVersion, MAGIC_SERVER_VER);
			NFSLEEP(1000);
			exit(0);
		}

		if (s_bCheckInitSuccessFlag)
		{
			if (MAYEX_SHM_INIT_SUCCESS_MAGIC != m_iInitSuccessFlag)
			{
				enInitFlag = EN_OBJ_MODE_INIT;
				NFLogError(NF_LOG_DEFAULT, 0, "shm change to mode INIT from RECOVER because no MAYEX_SHM_INIT_SUCCESS_MAGIC");
			}
		}
	}

	if (EN_OBJ_MODE_INIT == enInitFlag)
	{
		m_siAddrOffset = 0;
		m_pShmAddr = pbCurrentShm;
		m_iObjSeqNum = 0;
		m_iCanBeDestroySafe = 0;
		m_iInitSuccessFlag = 0;
		m_iServerCheckVersion = MAGIC_SERVER_VER;
	}
	else
	{
		m_siAddrOffset = (size_t)(pbCurrentShm - m_pShmAddr);
		NFLogInfo(NF_LOG_DEFAULT, 0, "recover mode,shm address change from {} to {}, addroffset {}", (void*)m_pShmAddr, (void*)pbCurrentShm, m_siAddrOffset);
		m_pShmAddr = pbCurrentShm;
		m_iCanBeDestroySafe = 0;
	}

	Initialize(nKey, siSize);
	m_enRunMode = enInitFlag;
}

NFCSharedMem::~NFCSharedMem()
{

}

int NFCSharedMem::Initialize(key_t nKey, size_t siSize)
{
	m_iCreateTime = NF_ADJUST_TIMENOW();
	m_iLastStamp = m_iCreateTime;

	m_nShmKey = nKey;
	m_siShmSize = siSize;
	m_siCRC = (unsigned int)m_nShmKey ^ m_siShmSize ^ (unsigned int)m_iCreateTime ^ (unsigned int)m_iLastStamp;
	return 0;
}

void *NFCSharedMem::CreateSegment(size_t siSize)
{
	size_t siTempUsedLength = 0;
	char* pTemp;

	if (siSize == 0)
	{
		return NULL;
	}

	siTempUsedLength = (size_t)(m_pbCurrentSegMent - (char*)this);

	if (m_siShmSize - siTempUsedLength < siSize)
	{
		return NULL;
	}

	pTemp = m_pbCurrentSegMent;
	m_pbCurrentSegMent += siSize;
	return (void *)pTemp;
}

void NFCSharedMem::SetStamp()
{
	m_iLastStamp = NF_ADJUST_TIMENOW();
	m_siCRC = (unsigned int)m_nShmKey ^ m_siShmSize ^ (unsigned int)m_iCreateTime ^ (unsigned int)m_iLastStamp;
}

void NFCSharedMem::SetShmCanbeDestroySafe()
{
	m_iCanBeDestroySafe = MAYEX_SHM_CAN_BE_DESTROY_SAFE_MAGIC;
}

void NFCSharedMem::SetShmInitSuccessFlag()
{
	m_iInitSuccessFlag = MAYEX_SHM_INIT_SUCCESS_MAGIC;
}

void NFCSharedMem::ClearShmInitSuccessFlag()
{
	m_iInitSuccessFlag = 0;
}

