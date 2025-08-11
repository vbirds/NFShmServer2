// -------------------------------------------------------------------------
//    @FileName         :    NFCDescStoreModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCDescStoreModule
//    @Desc             :    NFShmXFrame描述存储模块实现
//                          实现配置表数据的存储、加载和管理功能
//                          支持数据库存储和文件存储两种方式，提供统一的访问接口
//
// -------------------------------------------------------------------------

#include "NFCDescStoreModule.h"

#include <NFServerComm/NFServerCommon/NFDBObjMgr.h>

#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFCore/NFFileUtility.h"
#include "NFComm/NFCore/NFMD5.h"
#include "NFResFileDb.h"
#include "NFComm/NFPluginModule/NFIMysqlModule.h"
#include "NFComm/NFPluginModule/NFIAsyDBModule.h"
#include "NFResMysqlDb.h"
#include "NFComm/NFPluginModule/NFICoroutineModule.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"


/**
 * @brief 描述存储模块构造函数
 * 
 * 初始化描述存储模块，设置资源数据库指针和状态标志
 * 
 * @param p 插件管理器指针
 */
NFCDescStoreModule::NFCDescStoreModule(NFIPluginManager* p) : NFIDescStoreModule(p)
{
    m_pResFileDB = NULL;
    m_pResSqlDB = NULL;
    m_bFinishReloaded = false;
    m_bFinishLoaded = false;
    m_bLoadingDB = false;
}

/**
 * @brief 描述存储模块析构函数
 * 
 * 清理描述存储模块资源，释放资源数据库对象
 */
NFCDescStoreModule::~NFCDescStoreModule()
{
    if (m_pResFileDB)
    {
        NF_SAFE_DELETE(m_pResFileDB);
    }
    if (m_pResSqlDB)
    {
        NF_SAFE_DELETE(m_pResSqlDB);
    }
}

/**
 * @brief 加载配置
 * 
 * 加载描述存储模块的配置文件，初始化模块并加载文件描述存储
 * 
 * @return 加载结果状态码，0表示成功
 */
int NFCDescStoreModule::LoadConfig()
{
    Initialize();
    if (!HasDBDescStore())
    {
        int iRetCode = LoadFileDescStore();
        if (iRetCode != 0)
        {
            LOG_ERR(0, iRetCode, "LoadFileDescStore failed");
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 模块唤醒
 * 
 * 在模块初始化完成后调用，进行必要的初始化工作
 * 
 * @return 初始化结果状态码，0表示成功
 */
int NFCDescStoreModule::Awake()
{
    return 0;
}

/**
 * @brief 模块定时更新
 * 
 * 每帧调用，处理模块的定时任务，包括：
 * - 检查描述存储加载状态
 * - 处理数据库加载完成后的文件加载
 * - 处理重载完成后的状态更新
 * 
 * @return 处理结果状态码，0表示成功
 */
int NFCDescStoreModule::Tick()
{
    if (!m_bFinishLoaded)
    {
        if (IsAllDescStoreDBLoaded())
        {
            if (HasDBDescStore())
            {
                int iRet = LoadFileDescStore();
                CHECK_EXPR_ASSERT(iRet == 0, false, "LoadFileDescStore Failed");
            }
            m_bLoadingDB = false;
        }
        if (IsAllDescStoreLoaded())
        {
            FinishAppTask(NF_ST_NONE, APP_INIT_DESC_STORE_LOAD, APP_INIT_TASK_GROUP_SERVER_LOAD_DESC_STORE);

            m_bFinishLoaded = true;
        }
    }

    if (!m_bFinishReloaded)
    {
        if (IsAllDescStoreDBLoaded())
        {
            if (HasDBDescStore())
            {
                int iRet = ReLoadFileDescStore();
                CHECK_ERR(0, iRet, "ReLoadFileDescStore Failed");
            }
            m_bFinishReloaded = true;
        }
    }

    return 0;
}

/**
 * @brief 重新加载配置
 * 
 * 当配置文件发生变化时，重新加载配置
 * 
 * @return 重载结果状态码，0表示成功
 */
int NFCDescStoreModule::OnReloadConfig()
{
    Reload();
    return 0;
}

/**
 * @brief 初始化模块
 * 
 * 初始化描述存储模块，包括资源加载和对象创建：
 * - 设置加载状态标志
 * - 创建文件资源数据库和SQL资源数据库
 * - 初始化所有描述存储对象
 * 
 * @return 初始化结果状态码，0表示成功
 */
int NFCDescStoreModule::Initialize()
{
    m_bFinishLoaded = false;
    m_bFinishReloaded = true;
    m_pResFileDB = CreateResDBFromFiles(m_pObjPluginManager->GetConfigPath() + "/"  + m_pObjPluginManager->GetGame() + "/Data");
    m_pResSqlDB = CreateResDBFromRealDB();

    InitAllDescStore();
    InitAllDescStoreEx();

    return 0;
}

/**
 * @brief 加载文件描述存储
 * 
 * 从文件系统加载所有描述存储数据，包括：
 * - 加载所有文件描述存储
 * - 检查所有数据加载完成后的处理
 * - 加载所有扩展描述存储
 * 
 * @return 加载结果状态码，0表示成功
 */
int NFCDescStoreModule::LoadFileDescStore()
{
    int iRet = 0;
    if (FindModule<NFIMemMngModule>()->GetInitMode() == EN_OBJ_MODE_INIT)
    {
        iRet = LoadAllFileDescStore();
        CHECK_ERR(0, iRet, "LoadAllFileDescStore Failed");

        iRet = CheckWhenAllDataLoaded();
        CHECK_ERR(0, iRet, "CheckWhenAllDataLoaded Failed");

        iRet = LoadAllDescStoreEx();
        CHECK_ERR(0, iRet, "LoadAllDescStoreEx Failed");

        iRet = CheckExWhenAllDataLoaded();
        CHECK_ERR(0, iRet, "CheckExWhenAllDataLoaded Failed");
    }
    else
    {
        iRet = CheckWhenAllDataLoaded();
        CHECK_ERR(0, iRet, "CheckWhenAllDataLoaded Failed");

        iRet = CheckExWhenAllDataLoaded();
        CHECK_ERR(0, iRet, "CheckExWhenAllDataLoaded Failed");
    }

    return iRet;
}

/**
 * @brief 从数据库加载描述存储数据
 * 
 * 当需要从数据库加载配置表时，应在各服务器的AfterAllConnectFinish回调中调用此接口
 * 
 * @return 加载结果状态码，0表示成功
 */
int NFCDescStoreModule::LoadDBDescStore()
{
    if (m_bLoadingDB) return 0;
    if (FindModule<NFIMemMngModule>()->GetInitMode() != EN_OBJ_MODE_INIT) return 0;
    if (!HasDBDescStore()) return 0;

    m_bLoadingDB = true;
    int iRet = LoadDB();
    CHECK_ERR(0, iRet, "LoadDB Failed");
    return iRet;
}

/**
 * @brief 初始化所有描述存储对象
 * 
 * 遍历所有已注册的描述存储对象，逐个进行初始化
 */
void NFCDescStoreModule::InitAllDescStore()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    for (int i = 0; i < (int)mDescStoreRegisterList.size(); i++)
    {
        std::string name = mDescStoreRegisterList[i];
        NFIDescStore* pDescStore = dynamic_cast<NFIDescStore*>(FindModule<NFIMemMngModule>()->GetHeadObj(mDescStoreRegister[name]));
        CHECK_EXPR_CONTINUE(pDescStore, "can' get NFIDescStore:{} ptr from shm", name);

        int iRet = InitDescStore(name, pDescStore);
        CHECK_EXPR_CONTINUE(iRet == 0, "InitDescStore:{} Failed!", name);

        NFLogTrace(NF_LOG_DEFAULT, 0, "Init Desc Store:{} Success", name);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
}

/**
 * @brief 初始化所有扩展描述存储对象
 * 
 * 遍历所有已注册的扩展描述存储对象，逐个进行初始化
 */
void NFCDescStoreModule::InitAllDescStoreEx()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    for (int i = 0; i < (int)mDescStoreExRegisterList.size(); i++)
    {
        std::string name = mDescStoreExRegisterList[i];
        NFIDescStoreEx* pDescStoreEx = dynamic_cast<NFIDescStoreEx*>(FindModule<NFIMemMngModule>()->GetHeadObj(mDescStoreExRegister[name]));
        CHECK_EXPR_CONTINUE(pDescStoreEx, "can' get NFIDescStoreEx:{} ptr from shm", name);

        int iRet = InitDescStoreEx(name, pDescStoreEx);
        CHECK_EXPR_CONTINUE(iRet == 0, "InitDescStoreEx:{} Failed!", name);

        NFLogTrace(NF_LOG_DEFAULT, 0, "Init Desc Store Ex:{} Success", name);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
}

/**
 * @brief 检查所有描述存储是否已加载
 * 
 * 遍历所有描述存储对象，检查是否都已加载完成
 * 
 * @return 如果所有描述存储都已加载返回true，否则返回false
 */
bool NFCDescStoreModule::IsAllDescStoreLoaded()
{
    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFIDescStore* pDescStore = iter->second;
        if (!pDescStore->IsLoaded())
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief 检查所有数据库描述存储是否已加载
 * 
 * 遍历所有描述存储对象，检查数据库数据是否都已加载完成
 * 
 * @return 如果所有数据库描述存储都已加载返回true，否则返回false
 */
bool NFCDescStoreModule::IsAllDescStoreDBLoaded()
{
    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFIDescStore* pDescStore = iter->second;
        if (!pDescStore->IsDBLoaded())
        {
            return false;
        }
    }
    return true;
}

int NFCDescStoreModule::InitDescStore(const std::string& descClass, NFIDescStore* pDescStore)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(pDescStore, -1, "pDescStore = NULL");

    pDescStore->SetValid();
    if (FindModule<NFIMemMngModule>()->GetInitMode() == EN_OBJ_MODE_INIT)
    {
        if (pDescStore->IsFileLoad())
        {
            pDescStore->SetDBLoaded(true);
        }
        else
        {
            pDescStore->SetDBLoaded(false);
        }
        pDescStore->SetLoaded(false);
        pDescStore->SetChecked(false);
        pDescStore->SetReLoading(false);
    }
    else
    {
        pDescStore->SetDBLoaded(true);
        pDescStore->SetLoaded(true);
        pDescStore->SetChecked(false);
        pDescStore->SetReLoading(false);
    }

    AddDescStore(descClass, pDescStore);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::InitDescStoreEx(const std::string& descClass, NFIDescStoreEx* pDescStoreEx)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(pDescStoreEx, -1, "pDescStore = NULL");

    pDescStoreEx->SetValid();
    if (FindModule<NFIMemMngModule>()->GetInitMode() == EN_OBJ_MODE_INIT)
    {
        pDescStoreEx->SetLoaded(false);
        pDescStoreEx->SetChecked(false);
        pDescStoreEx->SetReLoading(false);
    }
    else
    {
        pDescStoreEx->SetLoaded(true);
        pDescStoreEx->SetChecked(false);
        pDescStoreEx->SetReLoading(false);
    }

    AddDescStoreEx(descClass, pDescStoreEx);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::LoadFileDescStore(NFIDescStore* pDescStore)
{
    CHECK_NULL(0, pDescStore);

    if (pDescStore->IsLoaded())
    {
        return 0;
    }

    int iRet = 0;
    std::string filePathName = pDescStore->GetFilePathName();
    if (filePathName.empty() && !pDescStore->GetFileName().empty())
    {
        filePathName = m_pObjPluginManager->GetConfigPath()  + "/"  + m_pObjPluginManager->GetGame() + "/Data/" + pDescStore->GetFileName() + ".bin";
        pDescStore->SetFilePathName(filePathName);
    }

    iRet = pDescStore->Load(m_pResFileDB);
    CHECK_EXPR(iRet == 0, iRet, "Desc Store:{} Load Failed!", pDescStore->GetFileName());

    pDescStore->SetLoaded(true);

    std::string fileMd5;
    if (!pDescStore->GetFilePathName().empty() && NFFileUtility::IsFileExist(pDescStore->GetFilePathName()))
    {
        iRet = GetFileContainMD5(pDescStore->GetFilePathName(), fileMd5);
        CHECK_EXPR(iRet == 0, iRet, "GetFileContainMD5 Failed, file:{}.bin", pDescStore->GetFileName());

        pDescStore->SetMD5(fileMd5.c_str());
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store End Load:{}, iRet={}, fileMd5:{}", pDescStore->GetFileName(), iRet, fileMd5);

    if (!pDescStore->IsFileLoad())
    {
        pDescStore->StartSaveTimer();
    }

    return 0;
}

int NFCDescStoreModule::LoadDBDescStore(NFIDescStore* pDescStore)
{
    CHECK_NULL(0, pDescStore);

    if (pDescStore->IsLoaded() || pDescStore->IsDBLoaded())
    {
        return 0;
    }

    int iRet = 0;
    std::string filePathName = pDescStore->GetFilePathName();
    if (filePathName.empty() && !pDescStore->GetFileName().empty())
    {
        filePathName = m_pObjPluginManager->GetConfigPath()  + "/"  + m_pObjPluginManager->GetGame() + "/Data/" + pDescStore->GetFileName() + ".bin";
        pDescStore->SetFilePathName(filePathName);
    }

    if (!pDescStore->IsFileLoad())
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Reload LoadDB:{}", pDescStore->GetFileName());
        iRet = pDescStore->LoadDB(m_pResSqlDB);
        CHECK_EXPR(iRet == 0, iRet, "Desc Store:{} Load Failed!", pDescStore->GetFileName());
    }

    return 0;
}

int NFCDescStoreModule::LoadAllFileDescStore()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    for (int i = 0; i < (int)mDescStoreRegisterList.size(); i++)
    {
        const std::string& name = mDescStoreRegisterList[i];
        NFIDescStore* pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Begin Load:{}", pDescStore->GetFileName());

        pDescStore->SetLoaded(false);
        pDescStore->SetChecked(false);
        pDescStore->SetDBLoaded(true);
        int ret = LoadFileDescStore(pDescStore);
        NF_ASSERT_MSG(ret == 0, "Load Desc Store:" + pDescStore->GetFileName() + " Failed!");
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Load:{} Sucess", pDescStore->GetFileName());
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

bool NFCDescStoreModule::HasDBDescStore()
{
    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFIDescStore* pDescStore = iter->second;
        assert(pDescStore);

        if (pDescStore->IsFileLoad())
            continue;

        return true;
    }
    return false;
}

int NFCDescStoreModule::LoadDB()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    for (int i = 0; i < (int)mDescStoreRegisterList.size(); i++)
    {
        const std::string& name = mDescStoreRegisterList[i];
        NFIDescStore* pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        if (pDescStore->IsFileLoad())
            continue;

        NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Begin Load:{}", pDescStore->GetFileName());

        FindModule<NFICoroutineModule>()->MakeCoroutine([pDescStore, this]
        {
            pDescStore->SetLoaded(false);
            pDescStore->SetChecked(false);
            pDescStore->SetDBLoaded(false);
            int ret = LoadDBDescStore(pDescStore);
            NF_ASSERT_MSG(ret == 0, "Load Desc Store:" + pDescStore->GetFileName() + " Failed!");
            NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Load:{} Sucess", pDescStore->GetFileName());
        });
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::ReLoadFileDescStore(NFIDescStore* pDescStore)
{
    CHECK_NULL(0, pDescStore);

    int iRet = 0;
    std::string fileMd5;
    if (!pDescStore->GetFilePathName().empty() && NFFileUtility::IsFileExist(pDescStore->GetFilePathName()))
    {
        iRet = GetFileContainMD5(pDescStore->GetFilePathName(), fileMd5);
        if (iRet == 0 && fileMd5 == std::string(pDescStore->GetFileMD5()))
        {
            pDescStore->SetLoaded(true);
            pDescStore->SetChecked(true);
            return 0;
        }
    }
    else
    {
        if (!pDescStore->IsNeedReload())
        {
            pDescStore->SetLoaded(true);
            pDescStore->SetChecked(true);
            return 0;
        }
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "File {}.bin Changed, Reload {}", pDescStore->GetFileName());
    pDescStore->SetReLoading(true);
    pDescStore->SetDBLoaded(true);
    pDescStore->SetLoaded(false);
    pDescStore->SetChecked(false);
    iRet = pDescStore->Reload(m_pResFileDB);
    CHECK_EXPR(iRet == 0, iRet, "Desc Store Reload table:{} error", pDescStore->GetFileName());

    pDescStore->SetLoaded(true);

    if (!pDescStore->GetFilePathName().empty() && NFFileUtility::IsFileExist(pDescStore->GetFilePathName()))
    {
        if (fileMd5.size() > 0)
        {
            pDescStore->SetMD5(fileMd5.c_str());
        }
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store End Reload:{}", pDescStore->GetFileName());
    return 0;
}

int NFCDescStoreModule::ReLoadFileDescStore()
{
    for (int i = 0; i < (int)mDescStoreRegisterList.size(); i++)
    {
        const std::string& name = mDescStoreRegisterList[i];
        NFIDescStore* pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        int ret = ReLoadFileDescStore(pDescStore);
        CHECK_EXPR_ASSERT(ret == 0, ret, "ReLoad Desc Store Failed!");
    }

    for (int i = 0; i < (int)mDescStoreRegisterList.size(); i++)
    {
        const std::string& name = mDescStoreRegisterList[i];
        NFIDescStore* pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        if (!pDescStore->IsChecked())
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Begin Check:{}", name);
            pDescStore->CheckWhenAllDataLoaded();
            pDescStore->SetChecked(true);
        }
    }

    for (int i = 0; i < (int)mDescStoreExRegisterList.size(); i++)
    {
        const std::string& name = mDescStoreExRegisterList[i];
        NFIDescStoreEx* pDescStoreEx = mDescStoreExMap[name];
        assert(pDescStoreEx);

        if (pDescStoreEx->IsNeedReload())
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Ex Begin Reload:{}", name);
            pDescStoreEx->SetLoaded(false);
            pDescStoreEx->SetChecked(false);
            pDescStoreEx->SetReLoading(true);
            pDescStoreEx->Reload();
            pDescStoreEx->SetLoaded(true);
        }
    }

    for (int i = 0; i < (int)mDescStoreExRegisterList.size(); i++)
    {
        const std::string& name = mDescStoreExRegisterList[i];
        NFIDescStoreEx* pDescStoreEx = mDescStoreExMap[name];
        assert(pDescStoreEx);

        if (!pDescStoreEx->IsChecked())
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Ex Begin Check:{}", name);
            pDescStoreEx->CheckWhenAllDataLoaded();
            pDescStoreEx->SetChecked(true);
        }
    }

    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFIDescStore* pDescStore = iter->second;
        assert(pDescStore);
        pDescStore->SetReLoading(false);
    }

    for (auto iter = mDescStoreExMap.begin(); iter != mDescStoreExMap.end(); iter++)
    {
        NFIDescStoreEx* pDescStoreEx = iter->second;
        assert(pDescStoreEx);
        pDescStoreEx->SetReLoading(false);
    }

    return 0;
}

int NFCDescStoreModule::ReLoadDBDescStore()
{
    for (int i = 0; i < (int)mDescStoreRegisterList.size(); i++)
    {
        const std::string& name = mDescStoreRegisterList[i];
        NFIDescStore* pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        if (pDescStore->IsFileLoad())
            continue;

        FindModule<NFICoroutineModule>()->MakeCoroutine([pDescStore, this]
        {
            pDescStore->SetDBLoaded(false);
            pDescStore->SetLoaded(false);
            pDescStore->SetChecked(false);
            pDescStore->SetReLoading(true);
            int ret = LoadDBDescStore(pDescStore);
            NF_ASSERT_MSG(ret == 0, "ReLoad Desc Store Failed!");
        });
    }

    return 0;
}

int NFCDescStoreModule::Reload()
{
    m_bFinishReloaded = false;
    m_bFinishLoaded = true;
    if (!HasDBDescStore())
    {
        ReLoadFileDescStore();
    }
    else
    {
        ReLoadDBDescStore();
    }

    return 0;
}

int NFCDescStoreModule::CheckWhenAllDataLoaded()
{
    int iRet = 0;
    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Begin CheckWhenAllDataLoaded:{}", iter->first);
        NFIDescStore* pDescStore = iter->second;
        assert(pDescStore);
        if (pDescStore->IsChecked()) continue;

        iRet = pDescStore->CheckWhenAllDataLoaded();

        CHECK_ERR(0, iRet, "Desc Store:{} CheckWhenAllDataLoaded Failed!", iter->first);

        pDescStore->SetChecked(true);
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store End CheckWhenAllDataLoaded:{}", iter->first);
    }
    return 0;
}

/**
 * @brief 加载所有扩展描述存储
 * 
 * 遍历所有扩展描述存储对象，逐个进行加载
 * 
 * @return 加载结果状态码，0表示成功
 */
int NFCDescStoreModule::LoadAllDescStoreEx()
{
    int iRet = 0;
    for (auto iter = mDescStoreExMap.begin(); iter != mDescStoreExMap.end(); iter++)
    {
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex Begin Loaded:{}", iter->first);
        NFIDescStoreEx* pDescStoreEx = iter->second;
        assert(pDescStoreEx);
        iRet = pDescStoreEx->Load();

        CHECK_EXPR_ASSERT(iRet == 0, iRet, "Desc Store Ex:{} Loaded Failed!", iter->first);

        pDescStoreEx->SetLoaded(true);
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex End Loaded:{}", iter->first);
    }
    return 0;
}

/**
 * @brief 检查所有扩展数据加载完成后的处理
 * 
 * 当所有扩展数据加载完成后进行必要的处理
 * 
 * @return 处理结果状态码，0表示成功
 */
int NFCDescStoreModule::CheckExWhenAllDataLoaded()
{
    int iRet = 0;
    for (auto iter = mDescStoreExMap.begin(); iter != mDescStoreExMap.end(); iter++)
    {
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex Begin CheckWhenAllDataLoaded:{}", iter->first);
        NFIDescStoreEx* pDescStoreEx = iter->second;
        assert(pDescStoreEx);
        if (pDescStoreEx->IsChecked()) continue;

        iRet = pDescStoreEx->CheckWhenAllDataLoaded();

        CHECK_EXPR_ASSERT(iRet == 0, iRet, "Desc Store Ex:{} CheckWhenAllDataLoaded Failed!", iter->first);

        pDescStoreEx->SetChecked(true);
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex End CheckWhenAllDataLoaded:{}", iter->first);
    }
    return 0;
}

/**
 * @brief 获取文件MD5值
 * 
 * 计算指定文件的MD5值，用于文件完整性校验
 * 
 * @param strFileName 文件名
 * @param fileMd5 输出MD5值
 * @return 操作结果状态码，0表示成功
 */
int NFCDescStoreModule::GetFileContainMD5(const std::string& strFileName, std::string& fileMd5)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    bool exist = NFFileUtility::IsFileExist(strFileName);
    CHECK_EXPR(exist, -1, "strFileName:{} not exist", strFileName);

    fileMd5 = NFMD5::md5file(strFileName);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 注册描述存储模块（指定数据库）
 * 
 * 注册描述存储模块，支持指定数据库名称
 * 
 * @param strDescName 类名
 * @param objType 对象类型
 * @param dbName 数据库名称
 */
void NFCDescStoreModule::RegisterDescStore(const std::string& strDescName, int objType, const std::string& dbName)
{
    if (mDescStoreRegister.find(strDescName) != mDescStoreRegister.end()) return;
    mDescStoreRegister.insert(std::make_pair(strDescName, objType));
    mDescStoreRegisterList.push_back(strDescName);
    mDescStoreDBNameMap.insert(std::make_pair(strDescName, dbName));
}

/**
 * @brief 注册描述存储模块（默认数据库）
 * 
 * 注册描述存储模块，使用默认数据库
 * 
 * @param strDescName 类名
 * @param objType 对象类型
 */
void NFCDescStoreModule::RegisterDescStore(const std::string& strDescName, int objType)
{
    if (mDescStoreRegister.find(strDescName) != mDescStoreRegister.end()) return;
    mDescStoreRegister.insert(std::make_pair(strDescName, objType));
    mDescStoreRegisterList.push_back(strDescName);
}

/**
 * @brief 注册扩展描述存储模块
 * 
 * 注册扩展描述存储模块，提供额外的功能
 * 
 * @param strDescName 类名
 * @param objType 对象类型
 */
void NFCDescStoreModule::RegisterDescStoreEx(const std::string& strDescName, int objType)
{
    if (mDescStoreExRegister.find(strDescName) != mDescStoreExRegister.end()) return;
    mDescStoreExRegister.insert(std::make_pair(strDescName, objType));
    mDescStoreExRegisterList.push_back(strDescName);
}

void NFCDescStoreModule::UnRegisterDescStore(const std::string& strDescName)
{
    // 检查描述存储是否存在，不存在则直接返回
    if (mDescStoreRegister.find(strDescName) == mDescStoreRegister.end()) return;
    
    // 从注册表中移除描述存储
    mDescStoreRegister.erase(strDescName);
    
    // 从注册列表中移除描述存储
    for (auto iter = mDescStoreRegisterList.begin(); iter != mDescStoreRegisterList.end(); ++iter)
    {
        if (*iter == strDescName)
        {
            mDescStoreRegisterList.erase(iter);
            break;
        }
    }
    
    // 从数据库名称映射中移除相关条目
    mDescStoreDBNameMap.erase(strDescName);
}

void NFCDescStoreModule::UnRegisterDescStoreEx(const std::string& strDescName)
{
    // 检查扩展描述存储是否存在，不存在则直接返回
    if (mDescStoreExRegister.find(strDescName) == mDescStoreExRegister.end()) return;
    
    // 从扩展注册表中移除描述存储
    mDescStoreExRegister.erase(strDescName);
    
    // 从扩展注册列表中移除描述存储
    for (auto iter = mDescStoreExRegisterList.begin(); iter != mDescStoreExRegisterList.end(); ++iter)
    {
        if (*iter == strDescName)
        {
            mDescStoreExRegisterList.erase(iter);
            break;
        }
    }
}

void NFCDescStoreModule::AddDescStore(const std::string& strDescName, NFIDescStore* pDesc)
{
    mDescStoreMap.insert(std::make_pair(strDescName, pDesc));
    mDescStoreFileMap.insert(std::make_pair(pDesc->GetFileName(), pDesc));
    auto it = mDescStoreDBNameMap.find(strDescName);
    if (it != mDescStoreDBNameMap.end())
    {
        pDesc->SetDBName(it->second);
    }
}

void NFCDescStoreModule::AddDescStoreEx(const std::string& strDescName, NFIDescStoreEx* pDescEx)
{
    mDescStoreExMap.insert(std::make_pair(strDescName, pDescEx));
}

int NFCDescStoreModule::SaveDescStoreByFileName(const std::string& dbName, const std::string& strDescFileName, const google::protobuf::Message* pMessage)
{
    NFIDescStore* pDescStore = FindDescStoreByFileName(strDescFileName);
    CHECK_EXPR(pDescStore, -1, "FindDescStoreByFileName(strDescName) Failed! strDescName:{}", strDescFileName);
    if (pDescStore->IsFileLoad() == false)
    {
        NFResTable* pResTable = m_pResSqlDB->GetTable(pDescStore->GetFileName());
        CHECK_EXPR(pResTable != NULL, -1, "pTable == NULL, GetTable:{} Error", pDescStore->GetFileName());

        int iRet = pResTable->SaveOneRecord(pDescStore->GetDBName(), pMessage);
        CHECK_EXPR(iRet == 0, -1, "pResTable->SaveDescStore Failed!");
        return 0;
    }
    return 0;
}

int NFCDescStoreModule::InsertDescStoreByFileName(const std::string& dbName, const std::string& strDescFileName,
                                                  const google::protobuf::Message* pMessage)
{
    NFIDescStore* pDescStore = FindDescStoreByFileName(strDescFileName);
    CHECK_EXPR(pDescStore, -1, "FindDescStoreByFileName(strDescName) Failed! strDescName:{}", strDescFileName);
    if (pDescStore->IsFileLoad() == false)
    {
        NFResTable* pResTable = m_pResSqlDB->GetTable(pDescStore->GetFileName());
        CHECK_EXPR(pResTable != NULL, -1, "pTable == NULL, GetTable:{} Error", pDescStore->GetFileName());

        int iRet = pResTable->InsertOneRecord(pDescStore->GetDBName(), pMessage);
        CHECK_EXPR(iRet == 0, -1, "pResTable->SaveDescStore Failed!");
        return 0;
    }
    return 0;
}

int NFCDescStoreModule::DeleteDescStoreByFileName(const std::string& dbName, const std::string& strDescFileName,
                                                  const google::protobuf::Message* pMessage)
{
    NFIDescStore* pDescStore = FindDescStoreByFileName(strDescFileName);
    CHECK_EXPR(pDescStore, -1, "FindDescStoreByFileName(strDescName) Failed! strDescName:{}", strDescFileName);
    if (pDescStore->IsFileLoad() == false)
    {
        NFResTable* pResTable = m_pResSqlDB->GetTable(pDescStore->GetFileName());
        CHECK_EXPR(pResTable != NULL, -1, "pTable == NULL, GetTable:{} Error", pDescStore->GetFileName());

        int iRet = pResTable->DeleteOneRecord(pDescStore->GetDBName(), pMessage);
        CHECK_EXPR(iRet == 0, -1, "pResTable->SaveDescStore Failed!");
        return 0;
    }
    return 0;
}

void NFCDescStoreModule::RemoveDescStore(const std::string& strDescName)
{
    auto iter = mDescStoreMap.find(strDescName);
    if (iter != mDescStoreMap.end())
    {
        mDescStoreMap.erase(strDescName);
    }
}

/**
 * @brief 根据文件名查找描述存储对象
 * 
 * 根据文件名查找对应的描述存储对象
 * 
 * @param strDescName 文件名
 * @return 描述存储对象指针，如果未找到返回nullptr
 */
NFIDescStore* NFCDescStoreModule::FindDescStoreByFileName(const std::string& strDescName)
{
    auto it = mDescStoreFileMap.find(strDescName);
    if (it != mDescStoreFileMap.end())
    {
        return it->second;
    }

    return nullptr;
}

/**
 * @brief 查找描述存储对象
 * 
 * 根据描述名称查找对应的描述存储对象，支持跨平台文件名处理
 * 
 * @param strDescName 描述名称
 * @return 描述存储对象指针，如果未找到返回nullptr
 */
NFIDescStore* NFCDescStoreModule::FindDescStore(const std::string& strDescName)
{
    std::string strSubDescName = strDescName;

#if NF_PLATFORM == NF_PLATFORM_WIN
    std::size_t position = strSubDescName.find(' ');
    if (string::npos != position)
    {
        strSubDescName = strSubDescName.substr(position + 1, strSubDescName.length());
    }
#else
    for (int i = 0; i < (int) strSubDescName.length(); i++)
    {
        std::string s = strSubDescName.substr(0, i + 1);
        int n = atof(s.c_str());
        if ((int) strSubDescName.length() == i + 1 + n)
        {
            strSubDescName = strSubDescName.substr(i + 1, strSubDescName.length());
            break;
        }
    }
#endif

    auto it = mDescStoreMap.find(strSubDescName);
    if (it != mDescStoreMap.end())
    {
        return it->second;
    }

    return nullptr;
}

/**
 * @brief 从真实数据库创建资源数据库
 * 
 * 基于真实数据库创建资源数据库对象
 * 
 * @return 资源数据库对象指针
 */
NFResDb* NFCDescStoreModule::CreateResDBFromRealDB()
{
    return new NFResMysqlDB(m_pObjPluginManager);
}

/**
 * @brief 从文件创建资源数据库
 * 
 * 基于指定目录的文件创建资源数据库对象
 * 
 * @param dir 文件目录
 * @return 资源数据库对象指针
 */
NFResDb* NFCDescStoreModule::CreateResDBFromFiles(const std::string& dir)
{
    return new NFFileResDB(m_pObjPluginManager, dir);
}

/**
 * @brief 通过RPC获取描述存储数据
 * 
 * 通过RPC调用从指定服务器获取描述存储数据
 * 
 * @param eType 服务器类型
 * @param dbName 数据库名称
 * @param table_name 表名
 * @param pMessage 输出消息数据
 * @return 操作结果状态码
 */
int NFCDescStoreModule::GetDescStoreByRpc(NF_SERVER_TYPE eType, const std::string& dbName, const std::string& table_name,
                                          google::protobuf::Message* pMessage)
{
    return FindModule<NFIServerMessageModule>()->GetRpcDescStoreService(eType, std::hash<std::string>()(table_name), pMessage, std::vector<std::string>(),
                                                                        "", 100, 0, dbName);
}

/**
 * @brief 共享内存初始化后运行
 * 
 * 在共享内存初始化完成后执行必要的操作
 */
void NFCDescStoreModule::runAfterShmInit()
{
    Initialize();
}

/**
 * @brief 检查停服条件
 * 
 * 停服之前，检查服务器是否满足停服条件
 * 
 * @return 检查结果状态码，0表示成功
 */
int NFCDescStoreModule::CheckStopServer()
{
    return 0;
}

/**
 * @brief 停服处理
 * 
 * 停服之前，做一些操作，满足停服条件
 * 
 * @return 处理结果状态码，0表示成功
 */
int NFCDescStoreModule::StopServer()
{
    return 0;
}

/**
 * @brief 所有连接完成后的处理
 * 
 * 当所有服务器连接建立完成后调用，处理数据库加载等任务
 * 
 * @param serverType 服务器类型
 * @return 处理结果状态码，0表示成功
 */
int NFCDescStoreModule::AfterAllConnectFinish(NF_SERVER_TYPE serverType)
{
    if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
    {
        if (IsWorkServer(serverType))
        {
            if (IsHasAppTask(serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT, APP_INIT_NEED_STORE_SERVER))
            {
                LoadDBDescStore();
            }
        }
    }

    return 0;
}

