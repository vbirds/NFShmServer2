// -------------------------------------------------------------------------
//    @FileName         :    NFCDescStoreModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCDescStoreModule
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


NFCDescStoreModule::NFCDescStoreModule(NFIPluginManager *p) : NFIDescStoreModule(p)
{
    m_pResFileDB = NULL;
    m_pResSqlDB = NULL;
    m_bStartInit = false;
    m_bFinishAllLoaded = false;
}

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

bool NFCDescStoreModule::AfterInitShmMem()
{
    Initialize();
    LoadFileDestSotre();
    if (!HasDBDescStore())
    {
        if (FindModule<NFIMemMngModule>()->GetInitMode() == EN_OBJ_MODE_INIT)
        {
            int iRet = CheckWhenAllDataLoaded();
            if (iRet != 0)
            {
                return false;
            }
        }
    }
    return true;
}

bool NFCDescStoreModule::Awake()
{
    for (int i = 0; i < (int) NF_ST_MAX; i++)
    {
        Subscribe((NF_SERVER_TYPE)i, NFrame::NF_EVENT_SERVER_TASK_GROUP_FINISH, NFrame::NF_EVENT_SERVER_TYPE, APP_INIT_TASK_GROUP_SERVER_CONNECT,
                  __FUNCTION__);
    }
    return true;
}

int
NFCDescStoreModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage)
{
    if (nEventID == NFrame::NF_EVENT_SERVER_TASK_GROUP_FINISH && bySrcType == NFrame::NF_EVENT_SERVER_TYPE &&
        nSrcID == APP_INIT_TASK_GROUP_SERVER_CONNECT)
    {
        if (serverType == NF_ST_NONE || m_pObjPluginManager->IsHasAppTask((NF_SERVER_TYPE) serverType, APP_INIT_TASK_GROUP_SERVER_LOAD_DESC_STORE))
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "{} Start LoadDBDestSotre.........", GetServerName((NF_SERVER_TYPE)serverType));
            LoadDBDestSotre();
        }
    }
    return 0;
}

bool NFCDescStoreModule::Execute()
{
    if (m_bStartInit && m_bFinishAllLoaded == false)
    {
        if (IsAllDescStoreLoad())
        {
            if (HasDBDescStore())
            {
                if (FindModule<NFIMemMngModule>()->GetInitMode() == EN_OBJ_MODE_INIT)
                {
                    int iRet = CheckWhenAllDataLoaded();
                    CHECK_EXPR_ASSERT(iRet == 0, false, "CheckWhenAllDataLoaded Failed");
                }
            }

            FinishAppTask(NF_ST_NONE, APP_INIT_DESC_STORE_LOAD, APP_INIT_TASK_GROUP_SERVER_LOAD_DESC_STORE);

            m_bFinishAllLoaded = true;
        }
    }

    return true;
}

bool NFCDescStoreModule::OnReloadConfig()
{
    Reload();
    return true;
}

int NFCDescStoreModule::Initialize()
{
    m_pResFileDB = CreateResDBFromFiles(m_pObjPluginManager->GetConfigPath() + "/Data");
    m_pResSqlDB = CreateResDBFromRealDB();

    InitAllDescStore();
    InitAllDescStoreEx();

    return 0;
}

int NFCDescStoreModule::LoadFileDestSotre()
{
    if (m_bStartInit) return 0;

    int iRet = -1;
    if (FindModule<NFIMemMngModule>()->GetInitMode() == EN_OBJ_MODE_INIT)
    {
        LoadFile();
    }
    else
    {
        iRet = ExtraInitializeWhenRecover();
    }

    if (!HasDBDescStore())
    {
        m_bStartInit = true;
    }
    return iRet;
}

int NFCDescStoreModule::LoadDBDestSotre()
{
    if (m_bStartInit) return 0;

    int iRet = -1;
    if (FindModule<NFIMemMngModule>()->GetInitMode() == EN_OBJ_MODE_INIT)
    {
        LoadDB();
    }
    else
    {
        iRet = ExtraInitializeWhenRecover();
    }

    m_bStartInit = true;
    return iRet;
}

void NFCDescStoreModule::InitAllDescStore()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    for (int i = 0; i < (int) mDescStoreRegisterList.size(); i++)
    {
        std::string name = mDescStoreRegisterList[i];
        NFIDescStore *pDescStore = dynamic_cast<NFIDescStore *>(FindModule<NFIMemMngModule>()->GetHeadObj(mDescStoreRegister[name]));
        CHECK_EXPR_CONTINUE(pDescStore, "can' get NFIDescStore:{} ptr from shm", name);

        int iRet = InitDescStore(name, pDescStore);
        CHECK_EXPR_CONTINUE(iRet == 0, "InitDescStore:{} Failed!", name);

        NFLogTrace(NF_LOG_DEFAULT, 0, "Init Desc Store:{} Success", name);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
}

void NFCDescStoreModule::InitAllDescStoreEx()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    for (int i = 0; i < (int) mDescStoreExRegisterList.size(); i++)
    {
        std::string name = mDescStoreExRegisterList[i];
        NFIDescStoreEx *pDescStoreEx = dynamic_cast<NFIDescStoreEx *>(FindModule<NFIMemMngModule>()->GetHeadObj(mDescStoreExRegister[name]));
        CHECK_EXPR_CONTINUE(pDescStoreEx, "can' get NFIDescStoreEx:{} ptr from shm", name);

        int iRet = InitDescStoreEx(name, pDescStoreEx);
        CHECK_EXPR_CONTINUE(iRet == 0, "InitDescStoreEx:{} Failed!", name);

        NFLogTrace(NF_LOG_DEFAULT, 0, "Init Desc Store Ex:{} Success", name);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
}

bool NFCDescStoreModule::IsAllDescStoreLoad()
{
    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFIDescStore *pDescStore = iter->second;
        if (!pDescStore->IsLoaded())
        {
            return false;
        }
    }
    return true;
}

int NFCDescStoreModule::InitDescStore(const std::string &descClass, NFIDescStore *pDescStore)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(pDescStore, -1, "pDescStore = NULL");

    pDescStore->SetValid();
    AddDescStore(descClass, pDescStore);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::InitDescStoreEx(const std::string& descClass, NFIDescStoreEx* pDescStoreEx)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(pDescStoreEx, -1, "pDescStore = NULL");

    pDescStoreEx->SetValid();
    AddDescStoreEx(descClass, pDescStoreEx);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::ExtraInitializeWhenRecover()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    //Reload(m_pResDB);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::LoadDescStore(NFIDescStore *pDescStore)
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
        filePathName = m_pObjPluginManager->GetConfigPath() + "/Data/" + pDescStore->GetFileName() + ".bin";
        pDescStore->SetFilePathName(filePathName);
    }

    if (pDescStore->IsFileLoad())
    {
        iRet = pDescStore->Load(m_pResFileDB);
        CHECK_EXPR(iRet == 0, iRet, "Desc Store:{} Load Failed!", pDescStore->GetFileName());
    }
    else
    {
        iRet = pDescStore->Load(m_pResSqlDB);
        CHECK_EXPR(iRet == 0, iRet, "Desc Store:{} Load Failed!", pDescStore->GetFileName());
    }

    pDescStore->SetLoaded(true);
    pDescStore->SetChecked(false);

    if (pDescStore->IsFileLoad())
    {
        std::string fileMd5;
        if (!pDescStore->GetFilePathName().empty() && NFFileUtility::IsFileExist(pDescStore->GetFilePathName()))
        {
            iRet = GetFileContainMD5(pDescStore->GetFilePathName(), fileMd5);
            CHECK_EXPR(iRet == 0, iRet, "GetFileContainMD5 Failed, file:{}.bin", pDescStore->GetFileName());

            pDescStore->SetMD5(fileMd5.c_str());
        }
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store End Load:{}, iRet={}, fileMd5:{}", pDescStore->GetFileName(), iRet, fileMd5);
    }
    else
    {
        pDescStore->StartSaveTimer();
    }

    return 0;
}

int NFCDescStoreModule::LoadFile()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    for (int i = 0; i < (int) mDescStoreRegisterList.size(); i++)
    {
        const std::string &name = mDescStoreRegisterList[i];
        NFIDescStore *pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        if (!pDescStore->IsFileLoad())
            continue;
        
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Begin Load:{}", pDescStore->GetFileName());

        int ret = LoadDescStore(pDescStore);
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
        NFIDescStore *pDescStore = iter->second;
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

    for (int i = 0; i < (int) mDescStoreRegisterList.size(); i++)
    {
        const std::string &name = mDescStoreRegisterList[i];
        NFIDescStore *pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        if (pDescStore->IsFileLoad())
            continue;

        NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Begin Load:{}", pDescStore->GetFileName());

        FindModule<NFICoroutineModule>()->MakeCoroutine([pDescStore, this]
                                                        {
                                                            int ret = LoadDescStore(pDescStore);
                                                            NF_ASSERT_MSG(ret == 0, "Load Desc Store:" + pDescStore->GetFileName() + " Failed!");
                                                            NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Load:{} Sucess", pDescStore->GetFileName());
                                                        });
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::ReLoadDescStore(NFIDescStore *pDescStore)
{
    CHECK_NULL(0, pDescStore);

    int iRet = 0;
    if (pDescStore->IsFileLoad())
    {
        std::string fileMd5;
        if (!pDescStore->GetFilePathName().empty() && NFFileUtility::IsFileExist(pDescStore->GetFilePathName()))
        {
            iRet = GetFileContainMD5(pDescStore->GetFilePathName(), fileMd5);
            if (iRet == 0 && fileMd5 == std::string(pDescStore->GetFileMD5()))
            {
                return 0;
            }
        }
        else
        {
            if (!pDescStore->IsNeedReload())
            {
                return 0;
            }
        }

        NFLogInfo(NF_LOG_DEFAULT, 0, "File {}.bin Changed, Reload.", pDescStore->GetFileName());
        pDescStore->SetReLoading(true);
        iRet = pDescStore->Reload(m_pResFileDB);
        CHECK_EXPR(iRet == 0, iRet, "Desc Store Reload table:{} error", pDescStore->GetFileName());

        pDescStore->SetLoaded(true);
        pDescStore->SetChecked(false);

        if (!pDescStore->GetFilePathName().empty() && NFFileUtility::IsFileExist(pDescStore->GetFilePathName()))
        {
            if (fileMd5.size() > 0)
            {
                pDescStore->SetMD5(fileMd5.c_str());
            }
        }

    }
    else
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "db table:{} Changed, Reload.", pDescStore->GetFileName());
        pDescStore->SetReLoading(true);
        iRet = pDescStore->Reload(m_pResSqlDB);
        CHECK_EXPR(iRet == 0, iRet, "Desc Store:{} Reload Failed!", pDescStore->GetFileName());

        pDescStore->SetLoaded(true);
        pDescStore->SetChecked(false);
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store End Reload:{}", pDescStore->GetFileName());
    return 0;
}

int NFCDescStoreModule::Reload()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    for (int i = 0; i < (int) mDescStoreRegisterList.size(); i++)
    {
        const std::string &name = mDescStoreRegisterList[i];
        NFIDescStore *pDescStore = mDescStoreMap[name];
        assert(pDescStore);

        NFLogInfo(NF_LOG_DEFAULT, 0, "Desc Store Begin Reload:{}", name);

        if (pDescStore->IsFileLoad())
        {
            int ret = ReLoadDescStore(pDescStore);
            CHECK_EXPR_ASSERT(ret == 0, ret, "ReLoad Desc Store Failed!");
        }
        else
        {
            FindModule<NFICoroutineModule>()->MakeCoroutine([pDescStore, this]
                                                            {
                                                                int ret = ReLoadDescStore(pDescStore);
                                                                NF_ASSERT_MSG(ret == 0, "ReLoad Desc Store Failed!");
                                                            });
        }
    }

    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFIDescStore *pDescStore = iter->second;
        assert(pDescStore);
        pDescStore->SetReLoading(false);
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::CheckWhenAllDataLoaded()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    int iRet = 0;
    for (auto iter = mDescStoreMap.begin(); iter != mDescStoreMap.end(); iter++)
    {
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Begin CheckWhenAllDataLoaded:{}", iter->first);
        NFIDescStore *pDescStore = iter->second;
        assert(pDescStore);
        iRet = pDescStore->CheckWhenAllDataLoaded();

        CHECK_EXPR_ASSERT(iRet == 0, iRet, "Desc Store:{} CheckWhenAllDataLoaded Failed!", iter->first);

        pDescStore->SetChecked(true);
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store End CheckWhenAllDataLoaded:{}", iter->first);
    }
    for (auto iter = mDescStoreExMap.begin(); iter != mDescStoreExMap.end(); iter++)
    {
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex Begin Loaded:{}", iter->first);
        NFIDescStoreEx *pDescStoreEx = iter->second;
        assert(pDescStoreEx);
        iRet = pDescStoreEx->Load();

        CHECK_EXPR_ASSERT(iRet == 0, iRet, "Desc Store Ex:{} Loaded Failed!", iter->first);

        pDescStoreEx->SetLoaded(true);
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex End Loaded:{}", iter->first);
    }
    for (auto iter = mDescStoreExMap.begin(); iter != mDescStoreExMap.end(); iter++)
    {
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex Begin CheckWhenAllDataLoaded:{}", iter->first);
        NFIDescStoreEx *pDescStoreEx = iter->second;
        assert(pDescStoreEx);
        iRet = pDescStoreEx->CheckWhenAllDataLoaded();

        CHECK_EXPR_ASSERT(iRet == 0, iRet, "Desc Store Ex:{} CheckWhenAllDataLoaded Failed!", iter->first);

        pDescStoreEx->SetChecked(true);
        NFLogTrace(NF_LOG_DEFAULT, 0, "Desc Store Ex End CheckWhenAllDataLoaded:{}", iter->first);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCDescStoreModule::GetFileContainMD5(const std::string &strFileName, std::string &fileMd5)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    bool exist = NFFileUtility::IsFileExist(strFileName);
    CHECK_EXPR(exist, -1, "strFileName:{} not exist", strFileName);

    fileMd5 = NFMD5::md5file(strFileName);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

void NFCDescStoreModule::RegisterDescStore(const std::string &strDescName, int objType, const std::string &dbName)
{
    if (mDescStoreRegister.find(strDescName) != mDescStoreRegister.end()) return;
    mDescStoreRegister.insert(std::make_pair(strDescName, objType));
    mDescStoreRegisterList.push_back(strDescName);
    mDescStoreDBNameMap.insert(std::make_pair(strDescName, dbName));
}

void NFCDescStoreModule::RegisterDescStore(const std::string &strDescName, int objType)
{
    if (mDescStoreRegister.find(strDescName) != mDescStoreRegister.end()) return;
    mDescStoreRegister.insert(std::make_pair(strDescName, objType));
    mDescStoreRegisterList.push_back(strDescName);
}

void NFCDescStoreModule::RegisterDescStoreEx(const std::string& strDescName, int objType)
{
    if (mDescStoreExRegister.find(strDescName) != mDescStoreExRegister.end()) return;
    mDescStoreExRegister.insert(std::make_pair(strDescName, objType));
    mDescStoreExRegisterList.push_back(strDescName);
}

void NFCDescStoreModule::AddDescStore(const std::string &strDescName, NFIDescStore *pDesc)
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

int NFCDescStoreModule::SaveDescStoreByFileName(const std::string &dbName, const std::string &strDescFileName, const google::protobuf::Message *pMessage)
{
    NFIDescStore *pDescStore = FindDescStoreByFileName(strDescFileName);
    CHECK_EXPR(pDescStore, -1, "FindDescStoreByFileName(strDescName) Failed! strDescName:{}", strDescFileName);
    if (pDescStore->IsFileLoad() == false)
    {
        NFResTable *pResTable = m_pResSqlDB->GetTable(pDescStore->GetFileName());
        CHECK_EXPR(pResTable != NULL, -1, "pTable == NULL, GetTable:{} Error", pDescStore->GetFileName());

        int iRet = pResTable->SaveOneRecord(pDescStore->GetDBName(), pMessage);
        CHECK_EXPR(iRet == 0, -1, "pResTable->SaveDescStore Failed!");
        return 0;
    }
    return 0;
}

int NFCDescStoreModule::InsertDescStoreByFileName(const std::string &dbName, const std::string &strDescFileName,
                                                  const google::protobuf::Message *pMessage)
{
    NFIDescStore *pDescStore = FindDescStoreByFileName(strDescFileName);
    CHECK_EXPR(pDescStore, -1, "FindDescStoreByFileName(strDescName) Failed! strDescName:{}", strDescFileName);
    if (pDescStore->IsFileLoad() == false)
    {
        NFResTable *pResTable = m_pResSqlDB->GetTable(pDescStore->GetFileName());
        CHECK_EXPR(pResTable != NULL, -1, "pTable == NULL, GetTable:{} Error", pDescStore->GetFileName());

        int iRet = pResTable->InsertOneRecord(pDescStore->GetDBName(), pMessage);
        CHECK_EXPR(iRet == 0, -1, "pResTable->SaveDescStore Failed!");
        return 0;
    }
    return 0;
}

int NFCDescStoreModule::DeleteDescStoreByFileName(const std::string &dbName, const std::string &strDescFileName,
                                                  const google::protobuf::Message *pMessage)
{
    NFIDescStore *pDescStore = FindDescStoreByFileName(strDescFileName);
    CHECK_EXPR(pDescStore, -1, "FindDescStoreByFileName(strDescName) Failed! strDescName:{}", strDescFileName);
    if (pDescStore->IsFileLoad() == false)
    {
        NFResTable *pResTable = m_pResSqlDB->GetTable(pDescStore->GetFileName());
        CHECK_EXPR(pResTable != NULL, -1, "pTable == NULL, GetTable:{} Error", pDescStore->GetFileName());

        int iRet = pResTable->DeleteOneRecord(pDescStore->GetDBName(), pMessage);
        CHECK_EXPR(iRet == 0, -1, "pResTable->SaveDescStore Failed!");
        return 0;
    }
    return 0;
}

void NFCDescStoreModule::RemoveDescStore(const std::string &strDescName)
{
    auto iter = mDescStoreMap.find(strDescName);
    if (iter != mDescStoreMap.end())
    {
        mDescStoreMap.erase(strDescName);
    }
}

NFIDescStore *NFCDescStoreModule::FindDescStoreByFileName(const std::string &strDescName)
{
    auto it = mDescStoreFileMap.find(strDescName);
    if (it != mDescStoreFileMap.end())
    {
        return it->second;
    }

    return nullptr;
}

NFIDescStore *NFCDescStoreModule::FindDescStore(const std::string &strDescName)
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

NFResDb *
NFCDescStoreModule::CreateResDBFromRealDB()
{
    return new NFResMysqlDB(m_pObjPluginManager);
}

NFResDb *NFCDescStoreModule::CreateResDBFromFiles(const std::string &dir)
{
    return new NFFileResDB(m_pObjPluginManager, dir);
}

int NFCDescStoreModule::GetDescStoreByRpc(NF_SERVER_TYPE eType, const std::string &dbName, const std::string &table_name,
                                          google::protobuf::Message *pMessage)
{
    return FindModule<NFIServerMessageModule>()->GetRpcDescStoreService(eType, std::hash<std::string>()(table_name), pMessage, std::vector<std::string>(),
                                                                 "", 100, 0, dbName);
}

void NFCDescStoreModule::runAfterShmInit()
{
    Initialize();
}

bool NFCDescStoreModule::CheckStopServer()
{
    if (!NFDBObjMgr::Instance()->CheckStopServer())
    {
        return false;
    }
    return true;
}

bool NFCDescStoreModule::StopServer()
{
    if (!NFDBObjMgr::Instance()->StopServer())
    {
        return false;
    }
    return true;
}

