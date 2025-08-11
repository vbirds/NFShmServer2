// -------------------------------------------------------------------------
//    @FileName         :    NFCShmMngModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFKernelPlugin
//    @Desc             :    共享内存管理模块实现文件，提供共享内存的核心管理功能。
//                          该文件实现了共享内存管理模块的核心功能，包括共享内存的分配和管理、
//                          对象的创建和销毁、对象段的管理、全局ID管理、定时器管理、
//                          事件管理、事务管理。主要功能包括共享内存初始化、对象生命周期管理、
//                          内存分配和释放、对象查找和遍历、定时器调度、事件处理、事务管理。
//                          设计特点包括基于共享内存支持跨进程、高效的内存管理、
//                          完整的对象生命周期、多种管理功能集成、自动资源回收
//
// -------------------------------------------------------------------------

#include "NFCShmMngModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"

#include <cassert>
#include <errno.h>
#include "NFComm/NFCore/NFStringUtility.h"
#include "NFComm/NFCore/NFFileUtility.h"
#include "NFShmGlobalId.h"
#include "NFShmObjSeg.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFObjCommon/NFTypeDefines.h"
#include "NFComm/NFObjCommon/NFRawObject.h"
#include "NFShmIdx.h"
#include "NFShmTransMng.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFShmEventMgr.h"
#include "NFShmTimer.h"
#include "NFShmTimerMng.h"
#include "NFComm/NFCore/NFServerTime.h"

/**
 * @brief 构造函数
 * 
 * 初始化共享内存管理模块，设置基本参数
 * 
 * @param p 插件管理器指针
 */
NFCShmMngModule::NFCShmMngModule(NFIPluginManager* p) : NFIMemMngModule(p)
{
    // 设置插件管理器
    m_pObjPluginManager = p;
    // 初始化共享内存管理器
    m_pSharedMemMgr = nullptr;
    // 设置运行模式和创建模式
    m_enRunMode = EN_OBJ_MODE_RECOVER;
    m_enCreateMode = EN_OBJ_MODE_RECOVER;
    // 初始化共享内存大小
    m_siShmSize = 0;
    // 设置运行时文件ID
    m_nRunTimeFileId = p->GetAppID();

    // 初始化对象段统计信息
    m_iObjSegSizeTotal = 0;
    m_iTotalObjCount = 0;
    // 初始化对象段交换计数器
    m_nObjSegSwapCounter.resize(EOT_MAX_TYPE);
    // 初始化全局ID管理器
    m_pGlobalId = nullptr;
}

/**
 * @brief 析构函数
 * 
 * 清理共享内存管理模块资源
 */
NFCShmMngModule::~NFCShmMngModule()
{
}

/**
 * @brief 加载所有插件后处理
 * 
 * 在所有插件加载完成后进行初始化操作
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::AfterLoadAllPlugin()
{
    // 获取所有插件列表
    std::list<NFIPlugin*> listPlugin = m_pObjPluginManager->GetListPlugin();

    // 遍历所有插件，初始化共享内存对象注册
    for (auto iter = listPlugin.begin(); iter != listPlugin.end(); ++iter)
    {
        NFIPlugin* pPlugin = *iter;
        if (pPlugin)
        {
            pPlugin->InitShmObjectRegister();
        }
    }

    /*
        分配共享内存
    */
    int iRetCode = AllocShm();
    CHECK_EXPR_ASSERT(iRetCode == 0, false, "AllocShm Failed");

    /*
        初始化共享内存里的对象
    */
    InitializeAllObj();

    /*
        创建一些全局性对象
    */
    InitShmObjectGlobal();

    return 0;
}

/**
 * @brief 心跳处理
 * 
 * 处理定时器和事务的心跳
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::Tick()
{
    // 处理定时器管理器心跳
    auto pTimerMng = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pTimerMng)
    {
        pTimerMng->OnTick(NF_ADJUST_TIMENOW_MS());
    }
    
    // 处理事务管理器心跳
    auto pTransManager = dynamic_cast<NFShmTransMng*>(GetHeadObj(EOT_TRANS_MNG));
    if (pTransManager)
    {
        pTransManager->TickNow(m_pObjPluginManager->GetCurFrameCount());
    }
    return 0;
}

/**
 * @brief 最终化处理
 * 
 * 销毁共享内存资源
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::Finalize()
{
    DestroyShareMem();
    return true;
}

/**
 * @brief 读取运行模式
 * 
 * 根据插件管理器状态确定运行模式
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::ReadRunMode()
{
    if (m_pObjPluginManager->IsInitShm())
    {
        m_enRunMode = EN_OBJ_MODE_INIT;
    }
    else
    {
        m_enRunMode = EN_OBJ_MODE_RECOVER;
    }
    return 0;
}

/**
 * @brief 分配共享内存
 * 
 * 使用默认参数分配共享内存
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::AllocShm()
{
    return AllocShm(NFServerIDUtil::GetShmObjKey(m_pObjPluginManager->GetBusName()), GetAllObjSize());
}

/**
 * @brief 分配共享内存
 * 
 * 使用指定参数分配共享内存
 * 
 * @param iKey 共享内存键值
 * @param siShmSize 共享内存大小
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::AllocShm(int iKey, size_t siShmSize)
{
    int iRet = 0;
    ReadRunMode();

    if (m_enRunMode == EN_OBJ_MODE_RECOVER)
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "run by RECOVER mode");
    }
    else
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "run by INIT mode");
    }

    m_siShmSize = siShmSize;
    m_pSharedMemMgr = CreateShareMem(iKey, m_siShmSize, m_enRunMode, true);
    if (!m_pSharedMemMgr)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "create shm fail");
        iRet = -1;
    }

    m_enRunMode = m_pSharedMemMgr->m_enRunMode;
    NFShmMgr::Instance()->SetRunMode(m_enRunMode);

    NFLogInfo(NF_LOG_DEFAULT, 0, "--end-- ret {}", iRet);

    return iRet;
}

/**
* 从共享内存里获得一块内存
*/
void* NFCShmMngModule::CreateSegment(size_t siSize) const
{
    if (m_pSharedMemMgr)
    {
        return m_pSharedMemMgr->CreateSegment(siSize);
    }
    NF_ASSERT_MSG(m_pSharedMemMgr, "m_pSharedMemMgr == nullptr");
    return nullptr;
}

/**
* 共享内存模式
*/
EN_OBJ_MODE NFCShmMngModule::GetInitMode()
{
    if (m_pSharedMemMgr)
    {
        return m_pSharedMemMgr->GetInitMode();
    }
    NF_ASSERT_MSG(m_pSharedMemMgr, "m_pSharedMemMgr == nullptr");
    return EN_OBJ_MODE_INIT;
}

void NFCShmMngModule::SetInitMode(EN_OBJ_MODE mode)
{
    if (m_pSharedMemMgr)
    {
        return m_pSharedMemMgr->SetInitMode(mode);
    }
    NF_ASSERT_MSG(m_pSharedMemMgr, "m_pSharedMemMgr == nullptr");
}

/**
* 共享内存创建对象模式
*/
EN_OBJ_MODE NFCShmMngModule::GetCreateMode()
{
    return m_enCreateMode;
}

/**
* 共享内存创建对象模式
*/
void NFCShmMngModule::SetCreateMode(EN_OBJ_MODE mode)
{
    m_enCreateMode = mode;
}

EN_OBJ_MODE NFCShmMngModule::GetRunMode()
{
    return m_enRunMode;
}

/**
* @brief  对象seq自增
*/
int NFCShmMngModule::IncreaseObjSeqNum()
{
    if (m_pSharedMemMgr)
    {
        return m_pSharedMemMgr->IncreaseObjSeqNum();
    }
    NF_ASSERT_MSG(m_pSharedMemMgr, "m_pSharedMemMgr == nullptr");
    return 0;
}

/**
* @brief  获得偏移地址
*/
size_t NFCShmMngModule::GetAddrOffset() const
{
    if (m_pSharedMemMgr)
    {
        return m_pSharedMemMgr->GetAddrOffset();
    }
    NF_ASSERT_MSG(m_pSharedMemMgr, "m_pSharedMemMgr == nullptr");
    return 0;
}

/**
* @brief  NFShmGlobalId
*/
NFShmGlobalId* NFCShmMngModule::GetGlobalId() const
{
    return m_pGlobalId;
}

/**
* @brief  设置功能内存初始化成功
*/
void NFCShmMngModule::SetShmInitSuccessFlag()
{
    if (m_pSharedMemMgr)
    {
        m_pSharedMemMgr->SetShmInitSuccessFlag();
    }
    NF_ASSERT_MSG(m_pSharedMemMgr, "m_pSharedMemMgr == nullptr");
}

NFCSharedMem* NFCShmMngModule::CreateShareMem(int iKey, size_t siSize, EN_OBJ_MODE enInitFlag, bool bCheckShmInitSuccessFlag) const
{
    NFCSharedMem* pShm = nullptr;
    size_t siTempShmSize = 0;
    void* pAddr = nullptr;
#if NF_PLATFORM == NF_PLATFORM_WIN
    HANDLE hShmId = nullptr;
    long sPageSize = 4096;
#else
    int hShmId = 0;
    long sPageSize = getpagesize();;
#endif

    siTempShmSize = siSize;
    siTempShmSize += sizeof(NFCSharedMem);


    if (siTempShmSize % sPageSize)
    {
        siTempShmSize = (siTempShmSize / sPageSize + 1) * sPageSize;
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "--begin-- key:{},  size:{}M, pagesize:{}, mode:{} ", iKey, siTempShmSize / 1024.0 / 1024.0, sPageSize,
              static_cast<int>(enInitFlag));

#if NF_PLATFORM == NF_PLATFORM_WIN
    std::string tempFileName = NF_FORMAT("{}_shm_key_{}.bus", m_pObjPluginManager->GetAppName(), iKey);
    std::wstring shmFileName = NFStringUtility::s2ws(tempFileName);
    hShmId = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, shmFileName.c_str());

    if (hShmId == nullptr)
    {
        HANDLE hFileId = CreateFile(shmFileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

        if (hFileId == nullptr)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem CreateFile error:{}, {}", errno, strerror(errno));
            NFSLEEP(1);
            exit(-1);
        }

        hShmId = CreateFileMapping(hFileId, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(siTempShmSize), shmFileName.c_str());

        if (hShmId == nullptr)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem CreateFile error:{}, {}", errno, strerror(errno));
            NFSLEEP(1);
            exit(-1);
        }

        NFLogInfo(NF_LOG_DEFAULT, 0, "shm ori mode {} change to mode {}(mode 1:Init, 2:Recover)", enInitFlag, EN_OBJ_MODE_INIT);

        enInitFlag = EN_OBJ_MODE_INIT;
    }

    pAddr = MapViewOfFile(hShmId, FILE_MAP_ALL_ACCESS, 0, 0, siTempShmSize);
    if (pAddr == nullptr)
    {
        CloseHandle(hShmId);
        NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem MapViewOfFile error:{}, {}", errno, strerror(errno));
        NFSLEEP(1);
        exit(-1);
    }
    NFCSharedMem::pbCurrentShm = static_cast<char*>(pAddr);
    NFCSharedMem::s_bCheckInitSuccessFlag = enInitFlag;
    pShm = new NFCSharedMem(iKey, siTempShmSize, enInitFlag, hShmId);

#else

    //注意_bCreate的赋值位置:保证多线程用一个对象的时候也不会有问题
    //试图创建
    if ((hShmId = shmget(iKey, siTempShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "CreateShareMem failed for error:{}, {}, server will try to attach it", errno, strerror(errno));
        //no space left
        if (errno == 28 || errno == 12)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem failed for error:{}, {}", errno, strerror(errno));
            NFSLEEP(1000);
            exit(-1);
        }

        //有可能是已经存在同样的key_shm,则试图连接
        NFLogInfo(NF_LOG_DEFAULT, 0, "same shm  exist, now try to attach it ... ");
        if ((hShmId = shmget(iKey, siTempShmSize, 0666)) < 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem failed for error:{}, {}", errno, strerror(errno));
            if ((hShmId = shmget(iKey, 0, 0666)) < 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem failed for error:{}, {}", errno, strerror(errno));
                NFSLEEP(1000);
                exit(-1);
            }
            else
            {
                NFLogInfo(NF_LOG_DEFAULT, 0, "rm the exsit shm ...");
                if (EN_OBJ_MODE_INIT == enInitFlag)
                {
                    if (shmctl(hShmId, IPC_RMID, nullptr))
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem rm failed for {}, {}", errno, strerror(errno));
                        NFSLEEP(1000);
                        exit(-1);
                    }

                    if ((hShmId = shmget(iKey, siTempShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem alloc failed for  {}, {}", errno, strerror(errno));
                        NFSLEEP(1000);
                        exit(-1);
                    }
                }
                else
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem shm already exist, but size not match, alloc failed for  {}, {}", errno,
                               strerror(errno));
                    NFSLEEP(1000);
                    exit(-1);
                }
            }
        }
        else
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "attach succ ");
        }
    }
    else
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "shm ori mode {} change to mode {}(mode 1:Init, 2:Recover)", enInitFlag, EN_OBJ_MODE_INIT);
        enInitFlag = EN_OBJ_MODE_INIT;
    }

    struct shmid_ds stDs;

    int iRetCode = shmctl(hShmId, IPC_STAT, &stDs);
    if (iRetCode != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "CSharedMem shmctl Failed");
        return nullptr;
    }

    if (siTempShmSize != stDs.shm_segsz)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "CSharedMem Invalid ReqShmSize With Shm, ReqShmSize:{}  ActShmSize:{} ShmID:{} ShmKey:{}", siTempShmSize,
                   stDs.shm_segsz, hShmId, iKey);
        return nullptr;
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "CSharedMem ReqShmSize:{}M ActShmSize:{}M ShmID:{} ShmKey:{}", siTempShmSize / 1024.0 / 1024.0,
              stDs.shm_segsz / 1024.0 / 1024.0, hShmId, iKey);
    //try to access shm
    if ((pAddr = shmat(hShmId, nullptr, 0)) == (char *) -1)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "CreateShareMem shmat failed for  {}, {}", errno, strerror(errno));
        NFSLEEP(1000);
        exit(-1);
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "shmat return {}", (void *) pAddr);

    if (pAddr != (void *) -1)
    {
        NFCSharedMem::pbCurrentShm = (char *) pAddr;
        NFCSharedMem::s_bCheckInitSuccessFlag = enInitFlag;
        pShm = new NFCSharedMem(iKey, siTempShmSize, enInitFlag, hShmId);
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "shmat failed for  {}, {}", errno, strerror(errno));
        NFSLEEP(1000);
        exit(-1);
    }
#endif

    NFLogInfo(NF_LOG_DEFAULT, 0, " --end-- pShm:{}", static_cast<void*>(pShm));

    return pShm;
}

/**
* 摧毁共享内存
*/
int NFCShmMngModule::DestroyShareMem()
{
    // 	if (m_pSharedMemMgr == nullptr)
    // 	{
    // 		return 0;
    // 	}
    //
    // #if NF_PLATFORM == NF_PLATFORM_WIN
    // 	if (m_pSharedMemMgr->pbCurrentShm != nullptr)
    // 	{
    // 		UnmapViewOfFile(m_pSharedMemMgr->pbCurrentShm);
    // 		CloseHandle(m_pSharedMemMgr->m_shemID);
    //
    // 		m_pSharedMemMgr->m_shemID = nullptr;
    // 		m_pSharedMemMgr->pbCurrentShm = nullptr;
    // 	}
    // #else
    // 	int hShmId = shmget(m_pSharedMemMgr->m_nShmKey, 0, 0666);
    // 	if (((int64_t)hShmId) < 0)
    // 	{
    // 		NFLogError(NF_LOG_DEFAULT, 0, "Error, touch to shm failed {}", strerror(errno));
    // 		return -1;
    // 	}
    // 	else
    // 	{
    // 		NFLogInfo(NF_LOG_DEFAULT, 0, "Now remove the exist share memory:{}", hShmId);
    //
    // 		if (shmctl(hShmId, IPC_RMID, nullptr))
    // 		{
    // 			NFLogError(NF_LOG_DEFAULT, 0, "Remove share memory failed : {}", strerror(errno));
    // 			return -1;
    // 		}
    //
    // 		NFLogDebug(NF_LOG_DEFAULT, 0, "Remove shared memory(id = {}, key = {}) succeed.", hShmId, m_pSharedMemMgr->m_nShmKey);
    // 	}
    //
    // #endif

    return 0;
}


std::string NFCShmMngModule::GetClassName(int bType)
{
    NF_ASSERT_MSG((int) bType < (int) m_nObjSegSwapCounter.size(), "bType < (int)m_nObjSegSwapCounter.size()");
    return m_nObjSegSwapCounter[bType].m_szClassName;
}

int NFCShmMngModule::GetClassType(int bType)
{
    NF_ASSERT_MSG((int) bType < (int) m_nObjSegSwapCounter.size(), "bType < (int)m_nObjSegSwapCounter.size()");
    return m_nObjSegSwapCounter[bType].m_iObjType;
}

NFShmObjSegSwapCounter* NFCShmMngModule::CreateCounterObj(int bType)
{
    NF_ASSERT_MSG((int) bType < (int) m_nObjSegSwapCounter.size(), "bType < (int)m_nObjSegSwapCounter.size()");
    return &m_nObjSegSwapCounter[bType];
}

int NFCShmMngModule::InitAllObjSeg()
{
    NFLogInfo(NF_LOG_DEFAULT, 0, "total obj shm size: {}M", m_iObjSegSizeTotal / 1024.0 / 1024.0);
    int iRet = 0;

    for (int i = 0; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        m_nObjSegSwapCounter[i].m_iObjType = i;
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSeg* pObjSeg = NFShmObjSeg::CreateObjSeg(this);
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            pObjSegSwapCounter->SetObjSeg(pObjSeg);
            NFShmMgr::Instance()->m_iType = i;
            iRet = pObjSeg->SetAndInitObj(this, pObjSegSwapCounter->m_nObjSize,
                                          pObjSegSwapCounter->m_iItemCount,
                                          pObjSegSwapCounter->m_pResumeFn, pObjSegSwapCounter->m_iUseHash);

            NFShmMgr::Instance()->m_iType = INVALID_ID;

            if (iRet)
            {
                NFLogInfo(NF_LOG_DEFAULT, 0, "NFShmObjSeg::InitAllObj failed!");
                return iRet;
            }
            size_t sObjSegSize = pObjSeg->GetMemSize();

            if (sObjSegSize / 1024.0 / 1024.0 >= 1.0)
            {
                NFLogWarning(NF_LOG_DEFAULT, 0, "{} count {}   --- ObjStart:{}, size:{}({}MB)", pObjSegSwapCounter->m_szClassName,
                             pObjSegSwapCounter->m_iItemCount,
                             static_cast<void*>(pObjSeg->m_pObjs), sObjSegSize, static_cast<float>(sObjSegSize) / 1024.0 / 1024.0);
            }
            else
            {
                NFLogTrace(NF_LOG_DEFAULT, 0, "{} count {}   --- ObjStart:{}, size:{}({}MB)", pObjSegSwapCounter->m_szClassName,
                           pObjSegSwapCounter->m_iItemCount,
                           static_cast<void*>(pObjSeg->m_pObjs), sObjSegSize, static_cast<float>(sObjSegSize) / 1024.0 / 1024.0);
            }
        }
    }

    return 0;
}

void NFCShmMngModule::RegisterClassToObjSeg(int bType, size_t nObjSize, int iItemCount, NFObject*(*pfResumeObj)(void*),
                                            NFObject*(*pCreateFn)(),
                                            void (*pDestroy)(NFObject*), int parentType, const std::string& pszClassName,
                                            bool useHash, bool singleton)
{
    NFShmObjSegSwapCounter* pCounter = CreateCounterObj(bType);
    bool add = false;
    if (pCounter->m_nObjSize > 0)
    {
        add = true;
        NF_ASSERT_MSG(pCounter->m_nObjSize == nObjSize, "bType:{} className:{}", bType, pszClassName);
        NF_ASSERT_MSG(pCounter->m_iObjType == bType, "bType:{} className:{}", bType, pszClassName);
        NF_ASSERT_MSG(pCounter->m_singleton == singleton, "bType:{} className:{}", bType, pszClassName);
        NF_ASSERT_MSG(pCounter->m_pResumeFn == pfResumeObj, "bType:{} className:{}", bType, pszClassName);
        NF_ASSERT_MSG(pCounter->m_szClassName == pszClassName, "bType:{} className:{}", bType, pszClassName);
        NF_ASSERT_MSG(pCounter->m_pCreateFn == pCreateFn, "bType:{} className:{}", bType, pszClassName);
        NF_ASSERT_MSG(pCounter->m_pDestroyFn == pDestroy, "bType:{} className:{}", bType, pszClassName);

        NF_ASSERT_MSG(pCounter->m_iUseHash == useHash, "bType:{} className:{}", bType, pszClassName);
        if (singleton)
        {
            return;
        }
    }
    pCounter->m_nObjSize = nObjSize;
    pCounter->m_iItemCount += iItemCount;
    pCounter->m_iObjType = bType;
    pCounter->m_singleton = singleton;

    if (pCounter->m_iItemCount < 0)
    {
        pCounter->m_iItemCount = 0;
    }

    if (singleton)
    {
        pCounter->m_iItemCount = 1;
    }

    pCounter->m_pResumeFn = pfResumeObj;
    pCounter->m_szClassName = pszClassName;

    pCounter->m_pCreateFn = pCreateFn;
    pCounter->m_pDestroyFn = pDestroy;
    if (parentType < 0)
    {
        pCounter->m_pParent = nullptr;
    }
    else
    {
        pCounter->m_pParent = GetObjSegSwapCounter(parentType);
        NFShmObjSegSwapCounter* pParentClass = pCounter->m_pParent;
        while (pParentClass)
        {
            pParentClass->m_childrenObjType.insert(bType);
            if (pCounter->m_parentObjType.find(pParentClass->m_iObjType) != pCounter->m_parentObjType.end())
            {
                NFLogError(NF_LOG_DEFAULT, 0, "class {} has the parent class:{}, circle dead", pszClassName, pParentClass->m_iObjType);
                NF_ASSERT(false);
            }
            pCounter->m_parentObjType.insert(pParentClass->m_iObjType);
            pParentClass = pParentClass->m_pParent;
        }
    }
    pCounter->m_pObjSeg = nullptr;
    pCounter->m_szClassName = pszClassName;
    pCounter->m_iUseHash = useHash;

    size_t siThisObjSegTotal = 0;
    if (!add)
    {
        siThisObjSegTotal += sizeof(NFShmObjSeg);
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
        siThisObjSegTotal += NFShmDyVector<NFShmIdx>::CountSize(pCounter->m_iItemCount);
        siThisObjSegTotal += NFShmDyList<int>::CountSize(pCounter->m_iItemCount);
#else
        siThisObjSegTotal += NFShmDyList<NFShmIdx>::CountSize(pCounter->m_iItemCount);
#endif
        if (pCounter->m_iUseHash)
        {
            siThisObjSegTotal += NFShmObjSeg::GetHashSize(pCounter->m_iItemCount);
        }
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
        siThisObjSegTotal += (sizeof(int) + pCounter->m_nObjSize) * pCounter->m_iItemCount;
#else
        siThisObjSegTotal += pCounter->m_nObjSize * pCounter->m_iItemCount;
#endif

        m_iObjSegSizeTotal += siThisObjSegTotal;
        m_iTotalObjCount += pCounter->m_iItemCount;

        if (siThisObjSegTotal / 1024.0 / 1024.0 >= 1)
        {
            NFLogWarning(NF_LOG_DEFAULT, 0, "class {} objsize {} M count {} tablesize {} M total obj count {}", pszClassName,
                         pCounter->m_nObjSize / 1024.0 / 1024.0, pCounter->m_iItemCount, siThisObjSegTotal / 1024.0 / 1024.0, m_iTotalObjCount);
        }
        else
        {
            NFLogTrace(NF_LOG_DEFAULT, 0, "class {} objsize {} byte count {} tablesize {} M total obj count {}", pszClassName,
                       pCounter->m_nObjSize, pCounter->m_iItemCount, siThisObjSegTotal / 1024.0 / 1024.0, m_iTotalObjCount);
        }
    }
    else
    {
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
        siThisObjSegTotal += NFShmDyVector<NFShmIdx>::CountSize(iItemCount);
        siThisObjSegTotal += NFShmDyList<int>::CountSize(iItemCount);
#else
        siThisObjSegTotal += NFShmDyList<NFShmIdx>::CountSize(iItemCount);
#endif
        if (pCounter->m_iUseHash)
        {
            siThisObjSegTotal += NFShmObjSeg::GetHashSize(iItemCount);
        }
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
        siThisObjSegTotal += (sizeof(int) + pCounter->m_nObjSize) * iItemCount;
#else
        siThisObjSegTotal += pCounter->m_nObjSize * iItemCount;
#endif

        m_iObjSegSizeTotal += siThisObjSegTotal;
        m_iTotalObjCount += iItemCount;

        if (siThisObjSegTotal / 1024.0 / 1024.0 >= 10)
        {
            NFLogWarning(NF_LOG_DEFAULT, 0, "append class {} objsize {} M count {} tablesize {} M total obj count {}", pszClassName,
                         pCounter->m_nObjSize / 1024.0 / 1024.0, iItemCount, siThisObjSegTotal / 1024.0 / 1024.0, m_iTotalObjCount);
        }
        else
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "append class {} objsize {} byte count {} tablesize {} M total obj count {}", pszClassName,
                      pCounter->m_nObjSize, iItemCount, siThisObjSegTotal / 1024.0 / 1024.0, m_iTotalObjCount);
        }
    }

    CHECK_EXPR_RE_VOID(m_iTotalObjCount < MAX_GLOBALID_NUM * 0.8, "the shm obj too much, m_iTotalObjCount:{} < MAX_GLOBALID_NUM:{}*0.8",
                       m_iTotalObjCount, MAX_GLOBALID_NUM);
}

void NFCShmMngModule::UnRegisterClassToObjSeg(int bType)
{
    // 获取对象类型对应的计数器
    NFShmObjSegSwapCounter* pCounter = CreateCounterObj(bType);

    // 如果存在父类关系，需要清理继承链
    if (pCounter->m_pParent != nullptr)
    {
        NFShmObjSegSwapCounter* pParentClass = pCounter->m_pParent;
        while (pParentClass)
        {
            // 从父类的子类列表中移除当前类型
            pParentClass->m_childrenObjType.erase(bType);
            // 从当前类型的父类列表中移除父类
            pCounter->m_parentObjType.erase(pParentClass->m_iObjType);
            pParentClass = pParentClass->m_pParent;
        }
    }

    // 计算并回收该对象类型占用的内存大小
    size_t siThisObjSegTotal = 0;
    {
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
        siThisObjSegTotal += NFShmDyVector<NFShmIdx>::CountSize(pCounter->m_iItemCount);
        siThisObjSegTotal += NFShmDyList<int>::CountSize(pCounter->m_iItemCount);
#else
        siThisObjSegTotal += NFShmDyList<NFShmIdx>::CountSize(pCounter->m_iItemCount);
#endif
        // 如果使用了哈希表，计算哈希表大小
        if (pCounter->m_iUseHash)
        {
            siThisObjSegTotal += NFShmObjSeg::GetHashSize(pCounter->m_iItemCount);
        }
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
        siThisObjSegTotal += (sizeof(int) + pCounter->m_nObjSize) * pCounter->m_iItemCount;
#else
        siThisObjSegTotal += pCounter->m_nObjSize * pCounter->m_iItemCount;
#endif

        // 从总内存统计中减去该对象类型占用的内存
        m_iObjSegSizeTotal -= siThisObjSegTotal;
        m_iTotalObjCount -= pCounter->m_iItemCount;

        // 根据内存大小输出不同级别的日志信息
        if (siThisObjSegTotal / 1024.0 / 1024.0 >= 10)
        {
            NFLogWarning(NF_LOG_DEFAULT, 0, "unregister class {} objsize {} M count {} tablesize {} M total obj count {}", pCounter->m_szClassName,
                         pCounter->m_nObjSize / 1024.0 / 1024.0, pCounter->m_iItemCount, siThisObjSegTotal / 1024.0 / 1024.0, m_iTotalObjCount);
        }
        else
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "unregister class {} objsize {} byte count {} tablesize {} M total obj count {}", pCounter->m_szClassName,
                      pCounter->m_nObjSize, pCounter->m_iItemCount, siThisObjSegTotal / 1024.0 / 1024.0, m_iTotalObjCount);
        }
    }
    
    // 清理计数器对象，释放相关资源
    pCounter->clear();
}

void NFShmObjSegSwapCounter::SetObjSeg(NFShmObjSeg* pObjSeg)
{
    m_pObjSeg = pObjSeg;
}

//////////////////////////////////////////////////////////////////////////
int NFCShmMngModule::InitializeAllObj()
{
    int iRet = InitAllObjSeg();

    if (iRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "failed!");
        return -1;
    }

    //对象内存分配完毕后，统一把创建方式改为Init.
    NFShmMgr::Instance()->SetCreateMode(EN_OBJ_MODE_INIT);
    SetCreateMode(EN_OBJ_MODE_INIT);
    return 0;
}

size_t NFCShmMngModule::GetAllObjSize() const
{
    return m_iObjSegSizeTotal;
}

int NFCShmMngModule::InitShmObjectGlobal()
{
    int iRet = 0;

    if (GetInitMode() == EN_OBJ_MODE_RECOVER)
    {
        m_pGlobalId = dynamic_cast<NFShmGlobalId*>(FindModule<NFIMemMngModule>()->GetObjByObjId(EOT_GLOBAL_ID, 0));
        NF_ASSERT(m_pGlobalId != NULL);
    }
    else
    {
        m_pGlobalId = dynamic_cast<NFShmGlobalId*>(NFShmGlobalId::CreateObject());
        NF_ASSERT(m_pGlobalId != NULL);
        for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
        {
            if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
            {
                NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
                if (pObjSegSwapCounter->m_singleton)
                {
                    CreateObj(i);
                    NFLogTrace(NF_LOG_DEFAULT, 0, "Create Shm Global Obj:{}", pObjSegSwapCounter->m_szClassName);
                }
            }
        }
    }

    InitSpecialShmObj();

    return iRet;
}

int NFCShmMngModule::InitSpecialShmObj()
{
    auto pManager = dynamic_cast<NFShmTransMng*>(GetHeadObj(EOT_TRANS_MNG));
    if (pManager)
    {
        /**
         * @brief 平衡处理，大概一帧处理200个trans
         */
        pManager->InitTick(1, 200);
    }

    return 0;
}

NFShmObjSeg* NFCShmMngModule::GetObjSeg(int iType) const
{
    if (iType >= 0 && iType < static_cast<int>(m_nObjSegSwapCounter.size())
        && m_nObjSegSwapCounter[iType].m_pObjSeg)
    {
        return m_nObjSegSwapCounter[iType].m_pObjSeg;
    }

    return nullptr;
}

NFShmObjSegSwapCounter* NFCShmMngModule::GetObjSegSwapCounter(int iType)
{
    if (iType >= 0 && iType < static_cast<int>(m_nObjSegSwapCounter.size()))
    {
        return &m_nObjSegSwapCounter[iType];
    }

    return nullptr;
}

int NFCShmMngModule::GetItemCount(int iType)
{
    NFShmObjSeg* pObjSeg = GetObjSeg(iType);
    if (pObjSeg)
    {
        return pObjSeg->GetItemCount();
    }
    NFLogError(NF_LOG_DEFAULT, 0, "now GetItemCount iType:{} null objseg", iType);
    return 0;
}

int NFCShmMngModule::GetUsedCount(int iType)
{
    NFShmObjSeg* pObjSeg = GetObjSeg(iType);
    if (pObjSeg)
    {
        return pObjSeg->GetUsedCount();
    }
    NFLogError(NF_LOG_DEFAULT, 0, "now GetUsedCount iType:{} null objseg", iType);
    return 0;
}

int NFCShmMngModule::GetFreeCount(int iType)
{
    NFShmObjSeg* pObjSeg = GetObjSeg(iType);
    if (pObjSeg)
    {
        return pObjSeg->GetFreeCount();
    }
    NFLogError(NF_LOG_DEFAULT, 0, "now GetFreeCount iType:{} null objseg", iType);
    return 0;
}

void* NFCShmMngModule::AllocMemForObject(int iType)
{
    NFShmObjSeg* pObjSeg = GetObjSeg(iType);
    if (pObjSeg)
    {
        void* p = pObjSeg->AllocMemForObject();

        NFLogTrace(NF_LOG_DEFAULT, 0, "{} type:{} objsize:{} AllocMem:{} usedCount:{} allCount:{}", m_nObjSegSwapCounter[iType].m_szClassName,
                   m_nObjSegSwapCounter[iType].m_iObjType, m_nObjSegSwapCounter[iType].m_nObjSize, p, pObjSeg->GetUsedCount(), pObjSeg->m_iItemCount);
        return p;
    }
    NFLogError(NF_LOG_DEFAULT, 0, "now AllocMemForObject iType:{} null objseg", iType);
    return nullptr;
}

void NFCShmMngModule::FreeMemForObject(int iType, void* pMem)
{
    NFShmObjSeg* pObjSeg = GetObjSeg(iType);
    if (pObjSeg)
    {
        int iRet = pObjSeg->FreeMemForObject(pMem);
        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "nFreeMemForObject Failed, the pMem is no the class data, classType:{}, className:{}", iType, m_nObjSegSwapCounter[iType].m_szClassName);
        }
        return;
    }
    NFLogError(NF_LOG_DEFAULT, 0, "now FreeMemForObject iType:{} null objseg", iType);
}

NFObject* NFCShmMngModule::GetObjByObjId(int iType, int iIndex)
{
    NFShmObjSeg* pObjSeg = GetObjSeg(iType);
    if (pObjSeg)
    {
        return pObjSeg->GetObj(iIndex);
    }
    NFLogError(NF_LOG_DEFAULT, 0, "now GetObjByObjId iType:{} null objseg", iType);
    return nullptr;
}

bool NFCShmMngModule::IsEnd(int iType, int iIndex)
{
    assert(IsTypeValid(iType));

    if (m_nObjSegSwapCounter[iType].m_pObjSeg)
    {
        return m_nObjSegSwapCounter[iType].m_pObjSeg->IsEnd(iIndex);
    }

    return true;
}

void NFCShmMngModule::SetSecOffSet(int iOffset)
{
    if (m_pGlobalId)
    {
        m_pGlobalId->SetSecOffSet(iOffset);
    }
}

int NFCShmMngModule::GetSecOffSet() const
{
    if (m_pGlobalId)
    {
        return m_pGlobalId->GetSecOffSet();
    }
    return 0;
}

size_t NFCShmMngModule::IterIncr(int iType, size_t iPos)
{
    assert(IsTypeValid(iType));

    return m_nObjSegSwapCounter[iType].m_pObjSeg->IterIncr(iPos);
}

size_t NFCShmMngModule::IterDecr(int iType, size_t iPos)
{
    assert(IsTypeValid(iType));

    return m_nObjSegSwapCounter[iType].m_pObjSeg->IterDecr(iPos);
}

NFCShmMngModule::iterator NFCShmMngModule::IterBegin(int iType)
{
    assert(IsTypeValid(iType));

    return iterator(this, iType, m_nObjSegSwapCounter[iType].m_pObjSeg->IterBegin());
}

NFCShmMngModule::iterator NFCShmMngModule::IterEnd(int iType)
{
    assert(IsTypeValid(iType));

    return iterator(this, iType, m_nObjSegSwapCounter[iType].m_pObjSeg->IterEnd());
}

NFCShmMngModule::const_iterator NFCShmMngModule::IterBegin(int iType) const
{
    assert(IsTypeValid(iType));

    return const_iterator(this, iType, m_nObjSegSwapCounter[iType].m_pObjSeg->IterBegin());
}

NFCShmMngModule::const_iterator NFCShmMngModule::IterEnd(int iType) const
{
    assert(IsTypeValid(iType));

    return const_iterator(this, iType, m_nObjSegSwapCounter[iType].m_pObjSeg->IterEnd());
}

NFCShmMngModule::iterator NFCShmMngModule::Erase(iterator iter)
{
    iterator nextIter(this, iter.m_type, m_nObjSegSwapCounter[iter.m_type].m_pObjSeg->IterNext(iter.m_pos));
    if (iter != IterEnd(iter.m_type))
    {
        DestroyObj(iter.GetObj());
    }
    return nextIter;
}

bool NFCShmMngModule::IsValid(iterator iter)
{
    return GetIterObj(iter.m_type, iter.m_pos) != nullptr;
}

NFObject* NFCShmMngModule::GetIterObj(int iType, size_t iPos)
{
    assert(IsTypeValid(iType));

    return m_nObjSegSwapCounter[iType].m_pObjSeg->GetIterObj(iPos);
}


const NFObject* NFCShmMngModule::GetIterObj(int iType, size_t iPos) const
{
    assert(IsTypeValid(iType));

    return m_nObjSegSwapCounter[iType].m_pObjSeg->GetIterObj(iPos);
}

bool NFCShmMngModule::IsTypeValid(int iType) const
{
    if (iType < 0 || iType >= static_cast<int>(m_nObjSegSwapCounter.size()))
    {
        return false;
    }

    return true;
}

NFTransBase* NFCShmMngModule::CreateTrans(int iType)
{
    CHECK_EXPR(IsTypeValid(iType), nullptr, "iType:{} is not valid", iType);

    auto pManager = dynamic_cast<NFShmTransMng*>(GetHeadObj(EOT_TRANS_MNG));
    if (pManager)
    {
        return pManager->CreateTrans(iType);
    }
    return nullptr;
}

NFTransBase* NFCShmMngModule::GetTrans(uint64_t ullTransId)
{
    auto* pManager = dynamic_cast<NFShmTransMng*>(GetHeadObj(EOT_TRANS_MNG));
    if (pManager)
    {
        return pManager->GetTransBase(ullTransId);
    }
    return nullptr;
}

NFObject* NFCShmMngModule::CreateObjByHashKey(int iType, NFObjectHashKey hashKey)
{
    assert(IsTypeValid(iType));

    NFObject* pObj = nullptr;
    if (!m_nObjSegSwapCounter[iType].m_iUseHash)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "the obj can't use hash");
        return nullptr;
    }

    if (m_nObjSegSwapCounter[iType].m_pObjSeg->HashFind(hashKey))
    {
        NFLogError(NF_LOG_DEFAULT, 0, "the hash key:{} exist.........", hashKey);
        return nullptr;
    }

    NFShmMgr::Instance()->m_iType = iType;
    pObj = m_nObjSegSwapCounter[iType].m_pCreateFn();
    NFShmMgr::Instance()->m_iType = INVALID_ID;
    if (pObj)
    {
        int iGlobalId = pObj->GetGlobalId();
        int iObjId = pObj->GetObjId();

        if (iGlobalId >= 0 && iObjId >= 0)
        {
            if (m_nObjSegSwapCounter[iType].m_pObjSeg)
            {
                int hashRet = m_nObjSegSwapCounter[iType].m_pObjSeg->HashInsert(hashKey, iObjId);
                if (hashRet < 0)
                {
                    NFLogDebug(NF_LOG_DEFAULT, hashKey, "CreateObjByHashKey Fail! hashKey:{} type:{} className:{} GlobalID:{} UsedCount:{} AllCount:{}", hashKey, iType, m_nObjSegSwapCounter[iType].m_szClassName, iGlobalId, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(),
                               m_nObjSegSwapCounter[iType].m_iItemCount);
                    m_nObjSegSwapCounter[iType].m_pDestroyFn(pObj);
                    pObj = nullptr;
                    NF_ASSERT(false);
                }
                else
                {
                    pObj->SetHashKey(hashKey);
                }

                NFLogDebug(NF_LOG_DEFAULT, hashKey, "CreateObjByHashKey Success! hashKey:{} type:{} className:{} GlobalID:{} objId:{} UsedCount:{} AllCount:{}", hashKey, iType, m_nObjSegSwapCounter[iType].m_szClassName, iGlobalId, iObjId, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(),
                           m_nObjSegSwapCounter[iType].m_iItemCount);
            }
            else
            {
                NFLogDebug(NF_LOG_DEFAULT, hashKey, "CreateObjByHashKey Fail! hashKey:{} type:{} className:{} GlobalID:{} UsedCount:{} AllCount:{}", hashKey, iType, m_nObjSegSwapCounter[iType].m_szClassName, iGlobalId, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(),
                           m_nObjSegSwapCounter[iType].m_iItemCount);
            }
        }
        else
        {
            NFLogDebug(NF_LOG_DEFAULT, hashKey, "CreateObjByHashKey Fail! hashKey:{} type:{} className:{} GlobalID:{} UsedCount:{} AllCount:{}", hashKey, iType, m_nObjSegSwapCounter[iType].m_szClassName, iGlobalId, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(),
                       m_nObjSegSwapCounter[iType].m_iItemCount);
            m_nObjSegSwapCounter[iType].m_pDestroyFn(pObj);
            pObj = nullptr;
            NF_ASSERT(false);
        }
    }
    else
    {
        NFLogDebug(NF_LOG_DEFAULT, hashKey, "CreateObjByHashKey Fail! hashKey:{} type:{} className:{} UsedCount:{} AllCount:{}", hashKey, iType, m_nObjSegSwapCounter[iType].m_szClassName, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(), m_nObjSegSwapCounter[iType].m_iItemCount);
    }

    return pObj;
}

/**
 * @brief 创建对象
 * 
 * 创建指定类型的对象，不使用哈希键
 * 
 * @param iType 对象类型
 * @return 创建的对象指针，失败返回nullptr
 */
NFObject* NFCShmMngModule::CreateObj(int iType)
{
    assert(IsTypeValid(iType));

    NFObject* pObj = nullptr;
    if (m_nObjSegSwapCounter[iType].m_iUseHash)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "the obj use hash, create obj use CreateObjByHashKey(uint64_t hashKey, int iType)");
        return nullptr;
    }

    // 设置当前创建的对象类型
    NFShmMgr::Instance()->m_iType = iType;
    pObj = m_nObjSegSwapCounter[iType].m_pCreateFn();
    NFShmMgr::Instance()->m_iType = INVALID_ID;
    if (pObj)
    {
        int iId = pObj->GetGlobalId();

        if (iId >= 0)
        {
            // 创建成功
        }
        else
        {
            m_nObjSegSwapCounter[iType].m_pDestroyFn(pObj);
            pObj = nullptr;
            NFLogError(NF_LOG_DEFAULT, 0, "No global id available, CreateObj Failed! type:{} className:{}", iType, m_nObjSegSwapCounter[iType].m_szClassName);
            NF_ASSERT(false);
        }

        NFLogDebug(NF_LOG_DEFAULT, 0, "CreateObj Success! type:{} className:{} GlobalID:{} UsedCount:{} AllCount:{}", iType, m_nObjSegSwapCounter[iType].m_szClassName, iId, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(), m_nObjSegSwapCounter[iType].m_iItemCount);
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "CreateObj Failed! type:{} className:{}", iType, m_nObjSegSwapCounter[iType].m_szClassName);
    }

    return pObj;
}

/**
 * @brief 根据哈希键获取对象
 * 
 * 根据哈希键查找指定类型的对象
 * 
 * @param iType 对象类型
 * @param hashKey 哈希键
 * @return 找到的对象指针，失败返回nullptr
 */
NFObject* NFCShmMngModule::GetObjByHashKey(int iType, NFObjectHashKey hashKey)
{
    NF_ASSERT(IsTypeValid(iType));
    if (!m_nObjSegSwapCounter[iType].m_iUseHash)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "the obj not use hash, get obj use GetObjByGlobalId");
        return nullptr;
    }

    return m_nObjSegSwapCounter[iType].m_pObjSeg->HashFind(hashKey);
}

/**
 * @brief 根据全局ID获取对象（无检查）
 * 
 * 直接根据全局ID获取对象，不进行类型检查
 * 
 * @param iGlobalId 全局ID
 * @return 找到的对象指针，失败返回nullptr
 */
NFObject* NFCShmMngModule::GetObjByGlobalIdWithNoCheck(int iGlobalId)
{
    NFObject* pObj = m_pGlobalId->GetObj(iGlobalId);
    return pObj;
}

/**
 * @brief 获取子类型集合
 * 
 * 获取指定类型的子类型集合
 * 
 * @param iType 对象类型
 * @return 子类型集合的引用
 */
const std::unordered_set<int>& NFCShmMngModule::GetChildrenType(int iType)
{
    NF_ASSERT(IsTypeValid(iType));

    return m_nObjSegSwapCounter[iType].m_childrenObjType;
}

/**
 * @brief 根据全局ID获取对象
 * 
 * 根据全局ID和类型获取对象，可选择是否包含子类型
 * 
 * @param iType 对象类型
 * @param iGlobalId 全局ID
 * @param withChildrenType 是否包含子类型
 * @return 找到的对象指针，失败返回nullptr
 */
NFObject* NFCShmMngModule::GetObjByGlobalId(int iType, int iGlobalId, bool withChildrenType)
{
    NFObject* pObj = m_pGlobalId->GetObj(iGlobalId);
    if (!pObj)
    {
        return nullptr;
    }

    int iRealType;

#ifdef NF_DEBUG_MODE
    pObj->CheckMemMagicNum();
#endif

    if (iType < 0)
    {
        return pObj;
    }

    iRealType = pObj->GetClassType();

    if (iRealType != iType)
    {
        if (!withChildrenType)
        {
            LOG_ERR(0, iRealType, "GetObjByGlobalId Fail! type:{} iRealType:{} GlobalID:{}", iType, iRealType, iGlobalId);
            return nullptr;
        }
    }

    auto pObjSeqSwapCounter = iRealType >= 0 && iRealType < static_cast<int>(m_nObjSegSwapCounter.size()) ? &m_nObjSegSwapCounter[iRealType] : nullptr;

    if (!pObjSeqSwapCounter)
    {
        return nullptr;
    }

    if (pObjSeqSwapCounter->m_iObjType == iType)
    {
        return pObj;
    }

    if (pObjSeqSwapCounter->m_parentObjType.find(iType) != pObjSeqSwapCounter->m_parentObjType.end())
    {
        return pObj;
    }

    LOG_ERR(0, iRealType, "GetObjByGlobalId Fail! type:{} className:{} GlobalID:{}", iType, pObjSeqSwapCounter->m_szClassName, iGlobalId);

    return nullptr;
}

/*static */
NFObject* NFCShmMngModule::GetObjByMiscId(int iMiscId, int iType)
{
    int iTypeInId = -1;

    if (iMiscId == -1)
    {
        return nullptr;
    }

    //MiscID is globalid
    if (iMiscId >= 0)
    {
        //带globalId的对象尽量不要通过GetObjFromMiscID来获取对象
#if defined( _DEBUG_) | defined(_DEBUG)
        NFLogError(NF_LOG_DEFAULT, 0, "advice:don't use GetObjByMiscId get object with globalId. {} ,type {}", iMiscId, iType);
#endif
        return GetObjByGlobalId(iType, iMiscId);
    }
    int iIndexInId = -1;
    iTypeInId = (iMiscId & 0x7f800000) >> 23;
    iIndexInId = iMiscId & 0x007fffff;
    NFObject* pObj = GetObjByObjId(iTypeInId, iIndexInId);

    if (!pObj)
    {
        return nullptr;
    }

#if defined(_DEBUG) | defined(_DEBUG_)
    pObj->CheckMemMagicNum();
#endif

    int iRealType = pObj->GetClassType();

    //不需要GlobalId的扩展类请不要通过CIDRuntimeClass创建对象
    assert(iRealType == iTypeInId);

    if (iRealType == iType)
    {
        return pObj;
    }

    if (iType >= 0)
    {
        auto pObjSegSwapCounter = iRealType >= 0 && iRealType < static_cast<int>(m_nObjSegSwapCounter.size()) ? &m_nObjSegSwapCounter[iType] : nullptr;

        if (!pObjSegSwapCounter)
        {
            assert(0);
            return nullptr;
        }

        if (pObjSegSwapCounter->m_iObjType == iType)
        {
            return pObj;
        }

        while (pObjSegSwapCounter->m_pParent)
        {
            if (pObjSegSwapCounter->m_pParent->m_iObjType == iType)
            {
                return pObj;
            }

            pObjSegSwapCounter = pObjSegSwapCounter->m_pParent;
        }

#if defined( _DEBUG_) | defined(_DEBUG)
        assert(0);
#endif
        NFLogError(NF_LOG_DEFAULT, 0, "Want Type:{} Real Type:{}", iType, iRealType)

        return nullptr;
    }

    return pObj;
}

NFObject* NFCShmMngModule::GetHeadObj(int iType)
{
    if (!IsTypeValid(iType))
    {
        return nullptr;
    }

    if (m_nObjSegSwapCounter[iType].m_pObjSeg)
    {
        return m_nObjSegSwapCounter[iType].m_pObjSeg->GetHeadObj();
    }

    return nullptr;
}

NFObject* NFCShmMngModule::GetNextObj(int iType, NFObject* pObj)
{
    if (!IsTypeValid(iType) || !pObj)
    {
        return nullptr;
    }

    if (m_nObjSegSwapCounter[iType].m_pObjSeg)
    {
        return m_nObjSegSwapCounter[iType].m_pObjSeg->GetNextObj(pObj);
    }

    return nullptr;
}

int NFCShmMngModule::GetGlobalId(int iType, int iIndex, NFObject* pObj)
{
    if (m_pGlobalId)
    {
        return m_pGlobalId->GetGlobalId(iType, iIndex, pObj);
    }
    return INVALID_ID;
}

int NFCShmMngModule::GetObjId(int iType, NFObject* pObj)
{
    NFShmObjSeg* pObjSeg = GetObjSeg(iType);
    if (pObjSeg)
    {
        return pObjSeg->GetObjId(pObj);
    }
    NFLogError(NF_LOG_DEFAULT, 0, "now GetObjID iType:{} null objseg", iType);
    return -1;
}

int NFCShmMngModule::DestroyObjAutoErase(int iType, int maxNum, const DESTROY_OBJECT_AUTO_ERASE_FUNCTION& func)
{
    if (m_nObjSegSwapCounter[iType].m_pObjSeg)
    {
        if (m_nObjSegSwapCounter[iType].m_iUseHash)
        {
            std::vector<NFObject*> vecObj;
            auto& getList = m_nObjSegSwapCounter[iType].m_pObjSeg->m_hashMgr.get_list();
            for (auto getIter = getList.begin(); getIter != getList.end(); ++getIter)
            {
                auto objIter = m_nObjSegSwapCounter[iType].m_pObjSeg->m_hashMgr.get_iterator(*getIter);
                NFObject* pObj = m_nObjSegSwapCounter[iType].m_pObjSeg->GetObj(objIter->second);
                if (func)
                {
                    if (func(pObj))
                    {
                        vecObj.push_back(pObj);
                        NFLogInfo(NF_LOG_DEFAULT, 0, "DestroyObjAutoErase Data, key:{} objId:{} type:{}", objIter->first, objIter->second, iType);
                    }
                }
                else
                {
                    vecObj.push_back(pObj);
                    NFLogInfo(NF_LOG_DEFAULT, 0, "DestroyObjAutoErase Data, key:{} objId:{} type:{}", objIter->first, objIter->second, iType);
                }

                if (maxNum > 0 && static_cast<int>(vecObj.size()) >= maxNum)
                {
                    break;
                }
            }

            for (int i = 0; i < static_cast<int>(vecObj.size()); i++)
            {
                DestroyObj(vecObj[i]);
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

void NFCShmMngModule::ClearAllObj(int iType)
{
    if (!IsTypeValid(iType)) return;

    if (m_nObjSegSwapCounter[iType].m_pObjSeg)
    {
        std::vector<NFObject*> vec;
        for (auto iter = IterBegin(iType); iter != IterBegin(iType); ++iter)
        {
            vec.push_back(&*iter);
        }

        for (auto iter = vec.begin(); iter != vec.end(); ++iter)
        {
            DestroyObj(*iter);
        }
    }
}

void NFCShmMngModule::DestroyObj(NFObject* pObj)
{
    int iType = -1;
    int iIndex = -1;
    int iGlobalId = -1;
    int64_t iHashId = -1;

    iType = pObj->GetClassType();
    iIndex = pObj->GetObjId();
    iGlobalId = pObj->GetGlobalId();
    iHashId = pObj->GetHashKey();
    std::string className = pObj->GetClassName();

    if (iType < 0 || iType >= static_cast<int>(m_nObjSegSwapCounter.size()))
    {
        return;
    }

    if (iIndex < 0)
    {
        return;
    }

    if (m_nObjSegSwapCounter[iType].m_pObjSeg)
    {
        if (m_nObjSegSwapCounter[iType].m_iUseHash)
        {
            NFObject* pTempObj = m_nObjSegSwapCounter[iType].m_pObjSeg->HashFind(iHashId);
            if (pTempObj != pObj)
            {
                NFLogError(NF_LOG_DEFAULT, iHashId, "DestroyObj {} Error, key:{} globalId:{} type:{} index:{}", className, iHashId,
                           iGlobalId, iType, iIndex, iHashId);
            }
            int ret = m_nObjSegSwapCounter[iType].m_pObjSeg->HashErase(iHashId);
            if (ret != 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "HashErase:{} Failed!", iHashId);
            }
        }

        m_pGlobalId->ReleaseId(iGlobalId);

        m_nObjSegSwapCounter[iType].m_pDestroyFn(pObj);

        if (m_nObjSegSwapCounter[iType].m_iUseHash)
        {
            NFLogDebug(NF_LOG_DEFAULT, iHashId, "DestroyObj {}, key:{} globalId:{} type:{} index:{} iHashID:{} UsedNum:{} AllNum:{}", className, iHashId, iGlobalId, iType, iIndex,
                       iHashId, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(), m_nObjSegSwapCounter[iType].m_iItemCount);
        }
        else
        {
            NFLogDebug(NF_LOG_DEFAULT, 0, "DestroyObj {}, globalId:{} type:{} index:{} UsedNum:{} AllNum:{}", className, iGlobalId, iType, iIndex, m_nObjSegSwapCounter[iType].m_pObjSeg->GetUsedCount(), m_nObjSegSwapCounter[iType].m_iItemCount);
        }
    }
}

/**
 * @brief 删除所有定时器
 * 
 * 删除对象的所有定时器
 * 
 * @param pObj 对象指针
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::DeleteAllTimer(NFObject* pObj)
{
    return NFShmTimerMng::Instance()->ClearAllTimer(pObj);
}

/**
 * @brief 删除所有定时器（原始对象）
 * 
 * 删除原始对象的所有定时器
 * 
 * @param pObj 对象指针
 * @param pRawShmObj 原始共享内存对象指针
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::DeleteAllTimer(NFObject* pObj, NFRawObject* pRawShmObj)
{
    return NFShmTimerMng::Instance()->ClearAllTimer(pObj, pRawShmObj);
}

/**
 * @brief 删除指定定时器
 * 
 * 删除对象的指定定时器
 * 
 * @param pObj 对象指针
 * @param timeObjId 定时器对象ID
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::DeleteTimer(NFObject* pObj, int timeObjId)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        NFShmTimer* pShmTimer = pManager->GetTimer(timeObjId);
        if (pShmTimer)
        {
            if (pShmTimer->GetTimerShmObj() == pObj)
            {
                return pManager->Delete(timeObjId);
            }
            NFLogError(NF_LOG_DEFAULT, 0, "timeObjId:{} pShmTimer->GetTimerShmObj:{} != pObj:{} is not the obj timer..............", timeObjId,
                       static_cast<void*>(pShmTimer->GetTimerShmObj()), static_cast<void*>(pObj));
            NFSLEEP(1)
            NF_ASSERT(false);
        }
    }
    return -1;
}

/**
 * @brief 设置距离现在多少时间执行一次的定时器
 * 
 * hour minutes second microSec为第一次执行距离现在的时分秒毫秒，只执行一次
 * 
 * @param pObj 对象指针
 * @param hour 小时
 * @param minutes 分钟
 * @param second 秒
 * @param microSec 微秒
 * @param pRawShmObj 原始共享内存对象指针
 * @return 定时器ID，失败返回-1
 */
int NFCShmMngModule::SetTimer(NFObject* pObj, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetTimer(pObj, hour, minutes, second, microSec, pRawShmObj);
    }
    return -1;
}

/**
 * @brief 设置某一个时间点执行一次的定时器
 * 
 * hour minutes second为第一次执行的时间点时分秒，只执行一次
 * 
 * @param pObj 对象指针
 * @param hour 小时
 * @param minutes 分钟
 * @param second 秒
 * @param pRawShmObj 原始共享内存对象指针
 * @return 定时器ID，失败返回-1
 */
int NFCShmMngModule::SetCalender(NFObject* pObj, int hour, int minutes, int second, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetCalender(pObj, hour, minutes, second, pRawShmObj);
    }
    return -1;
}

/**
 * @brief 设置某一个时间点执行一次的定时器
 * 
 * timestamp为第一次执行的时间点的时间戳，单位是秒，只执行一次
 * 
 * @param pObj 对象指针
 * @param timestamp 时间戳
 * @param pRawShmObj 原始共享内存对象指针
 * @return 定时器ID，失败返回-1
 */
int NFCShmMngModule::SetCalender(NFObject* pObj, uint64_t timestamp, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetCalender(pObj, timestamp, pRawShmObj);
    }
    return -1;
}

//注册循环执行定时器（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,  interval 为循环间隔时间，为毫秒）
int NFCShmMngModule::SetTimer(NFObject* pObj, int interval, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetTimer(pObj, interval, callCount, hour, minutes, second, microSec, pRawShmObj);
    }
    return -1;
}

//注册循环执行定时器（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒）
int NFCShmMngModule::SetDayTime(NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetDayTime(pObj, callCount, hour, minutes, second, microSec, pRawShmObj);
    }
    return -1;
}

//注册某一个时间点日循环执行定时器（hour  minutes  second为一天中开始执行的时间点，    23：23：23     每天23点23分23秒执行）
int NFCShmMngModule::SetDayCalender(NFObject* pObj, int callCount, int hour, int minutes, int second, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetDayCalender(pObj, callCount, hour, minutes, second, pRawShmObj);
    }
    return -1;
}

//周循环（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒）
int NFCShmMngModule::SetWeekTime(NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetWeekTime(pObj, callCount, hour, minutes, second, microSec, pRawShmObj);
    }
    return -1;
}

//注册某一个时间点周循环执行定时器（ weekDay  hour  minutes  second 为一周中某一天开始执行的时间点）
int NFCShmMngModule::SetWeekCalender(NFObject* pObj, int callCount, int weekDay, int hour, int minutes, int second, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetWeekCalender(pObj, callCount, weekDay, hour, minutes, second, pRawShmObj);
    }
    return -1;
}

//月循环（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,最好是同一天）
int NFCShmMngModule::SetMonthTime(NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetMonthTime(pObj, callCount, hour, minutes, second, microSec, pRawShmObj);
    }
    return -1;
}

//注册某一个时间点月循环执行定时器（ day  hour  minutes  second 为一月中某一天开始执行的时间点）
int NFCShmMngModule::SetMonthCalender(NFObject* pObj, int callCount, int day, int hour, int minutes, int second, NFRawObject* pRawShmObj/* = nullptr*/)
{
    auto pManager = dynamic_cast<NFShmTimerMng*>(GetHeadObj(EOT_TYPE_TIMER_MNG));
    if (pManager)
    {
        return pManager->SetMonthCalender(pObj, callCount, day, hour, minutes, second, pRawShmObj);
    }
    return -1;
}

/**
 * @brief 触发事件执行
 * 
 * 触发指定事件的执行
 * 
 * @param serverType 服务器类型
 * @param eventId 事件ID
 * @param srcType 源类型
 * @param srcId 源ID
 * @param message 消息内容
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::FireExecute(NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message)
{
    return NFShmEventMgr::Instance()->Fire(serverType, eventId, srcType, srcId, message);
}

/**
 * @brief 订阅事件
 * 
 * 为对象订阅指定的事件
 * 
 * @param pObj 对象指针
 * @param serverType 服务器类型
 * @param eventId 事件ID
 * @param srcType 源类型
 * @param srcId 源ID
 * @param desc 事件描述
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::Subscribe(NFObject* pObj, NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const string& desc)
{
    return NFShmEventMgr::Instance()->Subscribe(pObj, serverType, eventId, srcType, srcId, desc);
}

/**
 * @brief 取消订阅事件
 * 
 * 取消对象的指定事件订阅
 * 
 * @param pObj 对象指针
 * @param serverType 服务器类型
 * @param eventId 事件ID
 * @param srcType 源类型
 * @param srcId 源ID
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::UnSubscribe(NFObject* pObj, NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId)
{
    return NFShmEventMgr::Instance()->UnSubscribe(pObj, serverType, eventId, srcType, srcId);
}

/**
 * @brief 取消所有事件订阅
 * 
 * 取消对象的所有事件订阅
 * 
 * @param pObj 对象指针
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::UnSubscribeAll(NFObject* pObj)
{
    return NFShmEventMgr::Instance()->UnSubscribeAll(pObj);
}

/**
 * @brief 模块唤醒
 * 
 * 唤醒所有单例对象
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmMngModule::Awake()
{
    int iRet = 0;
    // 遍历所有对象类型，唤醒单例对象
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->Awake();
                    CHECK_ERR(0, iRet, "class:{} Awake Failed", pObject->GetClassName());
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::Init()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->Init();
                    CHECK_ERR(0, iRet, "class:{} Init Failed", pObject->GetClassName());
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::Shut()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->Shut();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} Shut Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterOnReloadConfig()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterOnReloadConfig();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterOnReloadConfig Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::HotfixServer()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->HotfixServer();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} HotfixServer Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::CheckStopServer()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    int iRetTemp = pObject->CheckStopServer();
                    if (iRetTemp != 0)
                    {
                        iRet = iRetTemp;
                    }
                }
            }
        }
    }
    return iRet;
}

int NFCShmMngModule::StopServer()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->StopServer();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} StopServer Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::OnServerKilling()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    int iRetTemp = pObject->OnServerKilling();
                    if (iRetTemp != 0)
                    {
                        iRet = iRetTemp;
                    }
                }
            }
        }
    }
    return iRet;
}

int NFCShmMngModule::AfterAllConnectFinish()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAllConnectFinish();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAllConnectFinish Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterAllDescStoreLoaded()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAllDescStoreLoaded();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAllDescStoreLoaded Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterAllConnectAndAllDescStore()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAllDescStoreLoaded();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAllDescStoreLoaded Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterObjFromDBLoaded()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterObjFromDBLoaded();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterObjFromDBLoaded Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterServerSyncData()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterServerSyncData();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterServerSyncData Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterAppInitFinish()
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAppInitFinish();
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAppInitFinish Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }

    SetShmInitSuccessFlag();

    if (GetSecOffSet() > 0)
    {
        NFServerTime::Instance()->SetSecOffSet(GetSecOffSet());
    }
    return 0;
}

int NFCShmMngModule::AfterAllConnectFinish(NF_SERVER_TYPE serverType)
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAllConnectFinish(serverType);
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAllConnectFinish Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterAllDescStoreLoaded(NF_SERVER_TYPE serverType)
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAllDescStoreLoaded(serverType);
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAllDescStoreLoaded Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterAllConnectAndAllDescStore(NF_SERVER_TYPE serverType)
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAllConnectAndAllDescStore(serverType);
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAllConnectAndAllDescStore Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterObjFromDBLoaded(NF_SERVER_TYPE serverType)
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterObjFromDBLoaded(serverType);
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterObjFromDBLoaded Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterServerSyncData(NF_SERVER_TYPE serverType)
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterServerSyncData(serverType);
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterServerSyncData Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}

int NFCShmMngModule::AfterAppInitFinish(NF_SERVER_TYPE serverType)
{
    int iRet = 0;
    for (int i = EOT_GLOBAL_ID + 1; i < static_cast<int>(m_nObjSegSwapCounter.size()); i++)
    {
        if (m_nObjSegSwapCounter[i].m_nObjSize > 0 && m_nObjSegSwapCounter[i].m_iItemCount > 0)
        {
            NFShmObjSegSwapCounter* pObjSegSwapCounter = &m_nObjSegSwapCounter[i];
            if (pObjSegSwapCounter->m_singleton)
            {
                NFObject* pObject = GetHeadObj(i);
                if (pObject)
                {
                    iRet = pObject->AfterAppInitFinish(serverType);
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "class:{} AfterAppInitFinish Failed", pObject->GetClassName());
                    }
                }
            }
        }
    }
    return 0;
}
