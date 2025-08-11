// -------------------------------------------------------------------------
//    @FileName         :    NFDBObjMgr.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDBObjMgr.cpp
//    @Desc             :    数据库对象管理器实现文件，提供数据库对象的管理功能实现。
//                          该文件实现了数据库对象管理器类的方法，包括对象管理接口、
//                          数据加载接口、数据保存接口、事务处理接口。
//                          主要功能包括提供数据库对象的管理实现、支持数据加载和保存、
//                          支持事务处理和状态管理、提供对象生命周期管理。
//                          数据库对象管理器实现是NFShmXFrame框架的数据库管理核心组件实现，负责：
//                          - 数据库对象的管理和调度实现
//                          - 数据加载和保存的协调实现
//                          - 事务处理和状态管理实现
//                          - 对象生命周期管理实现
//                          - 数据库操作的批量处理实现
//                          - 错误处理和重试机制实现
//
// -------------------------------------------------------------------------

#include "NFDBObjMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFBaseDBObj.h"
#include "NFDBObjTrans.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFError.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFDBObjMgr::NFDBObjMgr()
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
 * 清理定时器资源
 */
NFDBObjMgr::~NFDBObjMgr()
{
    if (m_iTimer != INVALID_ID)
    {
        DeleteTimer(m_iTimer); ///< 删除定时器
        m_iTimer = INVALID_ID; ///< 清除定时器ID
    }
}

/**
 * @brief 创建初始化
 * 
 * 初始化所有成员变量为默认值，设置定时器
 * 
 * @return 0表示成功
 */
int NFDBObjMgr::CreateInit()
{
    m_iLastSavingObjIndex = 0; ///< 最后保存对象索引为0
    m_iLastTickTime = 0; ///< 最后心跳时间为0
    m_iTransMngObjID = 0; ///< 事务管理器对象ID为0
    m_iTimer = INVALID_ID; ///< 定时器ID为无效
    m_iTimer = SetTimer(1000, 0, 0, 0, 10, 0); ///< 设置定时器，每秒执行一次
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 从共享内存恢复对象状态
 * 
 * @return 0表示成功
 */
int NFDBObjMgr::ResumeInit()
{
    return 0;
}

/**
 * @brief 定时器回调
 * 
 * 处理定时器事件，执行Tick操作
 * 
 * @param timeId 定时器ID
 * @param callcount 调用次数
 * @return 0表示成功
 */
int NFDBObjMgr::OnTimer(int timeId, int callcount)
{
    if (m_iTimer == timeId)
    {
        Tick(); ///< 执行Tick操作
    }
    return 0;
}

/**
 * @brief 定时处理
 * 
 * 定期处理数据库对象的加载和保存操作
 * 
 * @return 0表示成功
 */
int NFDBObjMgr::Tick()
{
    // 处理失败的对象列表，尝试重新加载
    for (auto iter = m_failedObjList.begin(); iter != m_failedObjList.end();)
    {
        NFBaseDBObj* pObj = GetObj(*iter); ///< 获取对象
        if (pObj)
        {
            int iRet = LoadFromDB(pObj); ///< 从数据库加载对象
            if (iRet == 0)
            {
                iter = m_failedObjList.erase(iter); ///< 加载成功，从失败列表中移除
            }
            else
            {
                iter++; ///< 加载失败，继续下一个
            }
        }
        else
        {
            iter = m_failedObjList.erase(iter); ///< 对象不存在，从失败列表中移除
        }
    }

    // 重置保存索引
    if (m_iLastSavingObjIndex >= (int)m_runningObjList.size())
    {
        m_iLastSavingObjIndex = 0; ///< 重置保存索引
    }

    int iSavedObjNum = 0; ///< 已保存对象数量
    int idx = 0; ///< 当前索引
    uint64_t now = NF_ADJUST_TIMENOW(); ///< 当前时间
    for (auto iter = m_runningObjList.begin(); iter != m_runningObjList.end() && iSavedObjNum < MAX_SAVED_OBJ_PRE_SEC;)
    {
        if (idx < m_iLastSavingObjIndex)
        {
            ++idx; ///< 跳过已处理的对象
            ++iter;
            continue;
        }

        NFBaseDBObj* pObj = GetObj(*iter);
        if (pObj)
        {
            // 不在存储中 + 有修改
            if (pObj->GetTransID() == 0 && pObj->IsUrgentNeedSave())
            {
                if (pObj->GetLastDBOpTime() + pObj->GetSaveDis() < now)
                {
                    int iRet = SaveToDB(pObj);
                    NFLogTrace(NF_LOG_DEFAULT, 0, "save obj ret:{} className{} key:{}", iRet, pObj->GetClassName(), pObj->GetModeKey());

                    ++iSavedObjNum;
                }
            }
            ++iter;
            ++idx;
            ++m_iLastSavingObjIndex;
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Remove running obj:{}", *iter);
            iter = m_runningObjList.erase(iter);
        }
    }

    if (m_loadDBList.size() > 0 && m_loadDBList.size() == m_loadDBFinishList.size())
    {
        m_loadDBList.clear();
        m_loadDBFinishList.clear();
        CheckWhenAllDataLoaded();
        NFGlobalSystem::Instance()->GetGlobalPluginManager()->FinishAppTask(NF_ST_NONE, APP_INIT_LOAD_GLOBAL_DATA_DB, APP_INIT_TASK_GROUP_SERVER_LOAD_OBJ_FROM_DB);
    }

    return 0;
}

int NFDBObjMgr::CheckWhenAllDataLoaded()
{
    for (auto iter = m_runningObjList.begin(); iter != m_runningObjList.end(); iter++)
    {
        NFBaseDBObj* pObj = GetObj(*iter);
        if (pObj)
        {
            int iRet = pObj->CheckWhenAllDataLoaded();
            if (iRet != 0)
            {
                NFLogFatal(NF_LOG_DEFAULT, 0, "Shutdown Server by obj check faled, key:{} className:{}", pObj->GetModeKey(), pObj->GetClassName());
                assert(0);
            }
        }
    }
    return 0;
}

int NFDBObjMgr::LoadFromDB(NFBaseDBObj* pObj)
{
    CHECK_NULL(0, pObj);

    m_loadDBList.insert(pObj->GetGlobalId());

    if (pObj->IsDataInited())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "data already inited:{} name:{}", pObj->GetGlobalId(), pObj->GetClassName())
        return 0;
    }

    NFDBObjTrans* pTrans = NFDBObjTrans::CreateTrans();
    CHECK_EXPR(pTrans, -1, "Create NFDBObjTrans:EOT_TRANS_DB_OBJ Failed! use num:{}", NFDBObjTrans::GetStaticUsedCount());

    int iRet = pTrans->Init(pObj->GetServerType(), pObj->GetGlobalId(), pObj->GetCurSeq());
    CHECK_EXPR(iRet == 0, -1, "Init Trans Failed!");

    google::protobuf::Message* pMessage = pObj->CreateTempProtobufData();
    CHECK_NULL(0, pMessage);
    iRet = pObj->MakeLoadData(pMessage);
    if (iRet != 0)
    {
        NF_SAFE_DELETE(pMessage);
        NFLogError(NF_LOG_DEFAULT, 0, "Make LoadData Failed:{} iRet:{}", pObj->GetClassName(), iRet);
        m_failedObjList.push_back(pObj->GetGlobalId());
        return iRet;
    }

    pObj->SetLastDBOpTime(NF_ADJUST_TIMENOW());
    pObj->SetTransID(pTrans->GetGlobalId());
    iRet = pTrans->Load(pObj->GetModeKey(), pMessage);
    NFLogDebug(NF_LOG_DEFAULT, 0, "Load db ob from key:{} className:{} transName:{} iRet:{}", pObj->GetModeKey(), pObj->GetClassName(), pTrans->GetClassName(), iRet);
    NF_SAFE_DELETE(pMessage);
    if (iRet != 0)
    {
        m_failedObjList.push_back(pObj->GetGlobalId());
        NFLogError(NF_LOG_DEFAULT, 0, "Make LoadData Failed:{} iRet:{}", pObj->GetClassName(), iRet);
        return iRet;
    }
    return 0;
}

int NFDBObjMgr::OnDataLoaded(int iObjID, int32_t err_code, const google::protobuf::Message* pData)
{
    if (pData)
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "objId:{} Date Loaded:{} err_code:{}", iObjID, pData->GetTypeName(), GetErrorStr(err_code));
    }
    else
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "objId:{} pData:nullptr err_code:{}", iObjID, GetErrorStr(err_code));
    }

    NFBaseDBObj* pObj = GetObj(iObjID);
    CHECK_NULL(0, pObj);

    pObj->SetTransID(0);
    int iRet = 0;
    if (err_code == 0)
    {
        if (pData)
        {
            pObj->SetRetryTimes(0);
            iRet = pObj->InitWithDBData(pData);
        }
        else
        {
            iRet = -1;
            NFLogError(NF_LOG_DEFAULT, 0, "objId:{} pData:nullptr err_code:{}", iObjID, GetErrorStr(err_code));
        }
    }
    else if ((int)err_code == NFrame::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY)
    {
        pObj->SetRetryTimes(0);
        pObj->SetNeedInsertDB(true);
        iRet = pObj->InitWithoutDBData();
    }
    else
    {
        iRet = -1;
    }

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "className:{} key:{} load faled! iRet:{}", pObj->GetClassName(), pObj->GetModeKey(), iRet);
        switch (pObj->DealWithFailed())
        {
            case EN_DW_LOG_FAIL:
            {
                pObj->SetInRecycle(true);
                NFLogError(NF_LOG_DEFAULT, 0, "Load From DB Error:{} DB error code:{}", iRet, err_code);
                break;
            }
            case EN_DW_RETRY:
            {
                pObj->SetRetryTimes(pObj->GetRetryTimes() + 1);
                iRet = pObj->DelaySyncReqRetry();
                CHECK_ERR(0, iRet, "DelaySyncReqRetry failed");
                break;
            }
            case EN_DW_SHUTDOWN:
            {
                NFLogFatal(NF_LOG_DEFAULT, 0, "className:{} Load Failed", pObj->GetClassName());
                NFLogFatal(NF_LOG_DEFAULT, 0, "Shutdown Server by obj init faled, key:{} className:{}", pObj->GetModeKey(), pObj->GetClassName());
                assert(0);
                return -1;
            }
            case EN_DW_RETRY_ANY_SHUTDOWN:
            {
                if (pObj->GetRetryTimes() > MAX_FAIL_RETRY_TIMES)
                {
                    NFLogFatal(NF_LOG_DEFAULT, 0, "className:{} Load Failed", pObj->GetClassName());
                    NFLogFatal(NF_LOG_DEFAULT, 0, "Shutdown Server by obj init faled, key:{} className:{}", pObj->GetModeKey(), pObj->GetClassName());
                    NF_ASSERT(false);
                    return -1;
                }
                pObj->SetRetryTimes(pObj->GetRetryTimes() + 1);
                iRet = pObj->DelaySyncReqRetry();
                CHECK_ERR(0, iRet, "DelaySyncReqRetry failed");
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        m_runningObjList.push_back(pObj->GetGlobalId());
        m_loadDBFinishList.insert(pObj->GetGlobalId());
    }

    return 0;
}

int NFDBObjMgr::OnDataInserted(NFDBObjTrans* pTrans, bool success)
{
    CHECK_NULL(0, pTrans);
    NFLogInfo(NF_LOG_DEFAULT, 0, "Data Inserted:{} {}", pTrans->GetLinkedObjID(), pTrans->GetClassName());
    NFBaseDBObj* pObj = GetObj(pTrans->GetLinkedObjID());
    CHECK_NULL(0, pObj);

    pObj->SetTransID(0);
    if (success)
    {
        pObj->SetLastDBOpTime(NF_ADJUST_TIMENOW());
        pObj->SetNeedInsertDB(false);
        if (pTrans->GetObjSeqOP() == pObj->GetCurSeq())
        {
            pObj->ClearUrgent();
        }
    }
    return 0;
}

int NFDBObjMgr::OnDataSaved(NFDBObjTrans* pTrans, bool success)
{
    CHECK_NULL(0, pTrans);
    NFBaseDBObj* pObj = GetObj(pTrans->GetLinkedObjID());
    CHECK_NULL(0, pObj);

    pObj->SetTransID(0);
    if (success)
    {
        pObj->SetLastDBOpTime(NF_ADJUST_TIMENOW());
        if (pTrans->GetObjSeqOP() == pObj->GetCurSeq())
        {
            pObj->ClearUrgent();
        }
    }
    return 0;
}

NFBaseDBObj* NFDBObjMgr::GetObj(int iObjID)
{
    return dynamic_cast<NFBaseDBObj*>(FindModule<NFIMemMngModule>()->GetObjByGlobalId(EOT_BASE_DB_OBJ, iObjID, true));
}

int NFDBObjMgr::SaveToDB(NFBaseDBObj* pObj)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--begin--");
    CHECK_NULL(0, pObj);

    if (!pObj->IsDataInited())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "data not init:{} name:{}", pObj->GetGlobalId(), pObj->GetClassName())
        return -1;
    }

    NFDBObjTrans* pTrans = NFDBObjTrans::CreateTrans();
    CHECK_EXPR(pTrans, -1, "Create NFDBObjTrans:EOT_TRANS_DB_OBJ Failed! use num:{}", NFDBObjTrans::GetStaticUsedCount());

    int iRet = pTrans->Init(pObj->GetServerType(), pObj->GetGlobalId(), pObj->GetCurSeq());
    CHECK_EXPR(iRet == 0, -1, "Init Trans Failed!");

    google::protobuf::Message* pMessage = pObj->CreateTempProtobufData();
    CHECK_NULL(0, pMessage);
    iRet = pObj->MakeSaveData(pMessage);
    if (iRet != 0)
    {
        NF_SAFE_DELETE(pMessage);
        NFLogError(NF_LOG_DEFAULT, 0, "Make save Failed:{} iRet:{}", pObj->GetClassName(), iRet);
        return iRet;
    }

    pObj->SetTransID(pTrans->GetGlobalId());
    if (pObj->GetNeedInsertDB())
    {
        iRet = pTrans->Insert(pObj->GetModeKey(), pMessage);
    }
    else
    {
        iRet = pTrans->Save(pObj->GetModeKey(), pMessage);
    }

    NF_SAFE_DELETE(pMessage);
    CHECK_RET(iRet, "SaveToDB Failed, key:{} pObj:{}", pObj->GetModeKey(), pObj->GetClassName());
    NFLogTrace(NF_LOG_DEFAULT, 0, "--end--");
    return 0;
}

int NFDBObjMgr::CheckStopServer()
{
    for (auto iter = m_runningObjList.begin(); iter != m_runningObjList.end();)
    {
        NFBaseDBObj* pObj = GetObj(*iter);
        if (pObj)
        {
            // 不在存储中 + 有修改
            if (pObj->IsDataInited() && pObj->IsUrgentNeedSave())
            {
                return -1;
            }
            ++iter;
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Remove running obj:{}", *iter);
            iter = m_runningObjList.erase(iter);
        }
    }
    return 0;
}

int NFDBObjMgr::StopServer()
{
    int iSavedObjNum = 0;
    for (auto iter = m_runningObjList.begin(); iter != m_runningObjList.end() && iSavedObjNum < MAX_SAVED_OBJ_PRE_SEC;)
    {
        NFBaseDBObj* pObj = GetObj(*iter);
        if (pObj)
        {
            // 不在存储中 + 有修改
            if (pObj->IsDataInited() && pObj->GetTransID() == 0 && pObj->IsUrgentNeedSave())
            {
                int iRet = SaveToDB(pObj);
                ++iSavedObjNum;
                NFLogTrace(NF_LOG_DEFAULT, 0, "save obj ret:{} className{} key:{}", iRet, pObj->GetClassName(), pObj->GetModeKey());
            }
            ++iter;
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Remove running obj:{}", *iter);
            iter = m_runningObjList.erase(iter);
        }
    }
    return 0;
}


