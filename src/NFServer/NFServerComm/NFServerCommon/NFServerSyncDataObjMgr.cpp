// -------------------------------------------------------------------------
//    @FileName         :    NFServerSyncDataObjMgr.cpp
//    @Author           :    gaoyi
//    @Date             :    2025-07-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerSyncDataObjMgr
//    @Desc             :    服务器同步数据对象管理器实现文件，提供服务器间数据同步的管理功能实现。
//                          该文件实现了服务器同步数据对象管理器类的方法，包括同步对象管理、
//                          数据同步请求、同步状态管理、停服检查。
//                          主要功能包括提供服务器间数据同步的管理实现、支持同步对象管理、
//                          支持数据同步请求和状态管理、提供停服检查功能。
//                          服务器同步数据对象管理器实现是NFShmXFrame框架的服务器同步组件实现，负责：
//                          - 服务器间数据同步的管理实现
//                          - 同步对象管理和状态维护实现
//                          - 数据同步请求和响应处理实现
//                          - 同步进度监控和错误处理实现
//                          - 停服条件检查和准备实现
//                          - 跨服务器数据一致性保证实现
//
// -------------------------------------------------------------------------

#include "NFServerSyncDataObjMgr.h"
#include "NFServerSyncDataObj.h"
#include "NFServerSyncDataTrans.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFError.h"
#include "NFServerSyncDataObj.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFServerSyncDataObjMgr::NFServerSyncDataObjMgr()
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
 */
NFServerSyncDataObjMgr::~NFServerSyncDataObjMgr()
{
}

/**
 * @brief 创建初始化
 * 
 * 初始化所有成员变量为默认值，设置定时器
 * 
 * @return 0表示成功
 */
int NFServerSyncDataObjMgr::CreateInit()
{
    m_iLastSyncObjIndex = 0; ///< 最后同步对象索引为0
    m_iLastTickTime = 0; ///< 最后心跳时间为0
    m_iTransMngObjID = 0; ///< 事务管理器对象ID为0
    SetTimer(1000, 0, 0, 0, 10, 0); ///< 设置定时器，每秒执行一次
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 从共享内存恢复对象状态
 * 
 * @return 0表示成功
 */
int NFServerSyncDataObjMgr::ResumeInit()
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
int NFServerSyncDataObjMgr::OnTimer(int timeId, int callcount)
{
    Tick(); ///< 执行Tick操作
    return 0;
}

/**
 * @brief 同步请求
 * 
 * 为指定的同步数据对象创建事务并发送同步请求
 * 
 * @param pObj 同步数据对象指针
 * @return 0表示成功
 */
int NFServerSyncDataObjMgr::SyncReq(NFServerSyncDataObj* pObj)
{
    CHECK_NULL(0, pObj); ///< 检查对象指针

    // 如果数据未初始化，添加到同步数据列表
    if (!pObj->IsDataInited())
    {
        m_syncDataList.insert(pObj->GetGlobalId()); ///< 添加到同步数据列表
    }

    // 创建同步数据事务
    NFServerSyncDataTrans* pTrans = NFServerSyncDataTrans::CreateTrans();
    CHECK_EXPR(pTrans, -1, "Create NFServerSyncDataTrans Failed! use num:{}", NFServerSyncDataTrans::GetStaticUsedCount()); ///< 检查事务创建是否成功

    // 初始化事务
    int iRet = pTrans->Init(pObj->GetServerType(), pObj->GetDstServerType(), pObj->GetDstBusId(), pObj->GetModuleId(), pObj->GetMsgId(), pObj->GetGlobalId());
    CHECK_EXPR(iRet == 0, -1, "Init Trans Failed!"); ///< 检查事务初始化是否成功

    // 创建临时Protobuf数据
    google::protobuf::Message* pMessage = pObj->CreateTempProtobufData();
    CHECK_NULL(0, pMessage); ///< 检查数据创建是否成功
    
    // 生成同步数据
    iRet = pObj->MakeSyncData(pMessage);
    if (iRet != 0)
    {
        NF_SAFE_DELETE(pMessage); ///< 删除临时数据
        NFLogError(NF_LOG_DEFAULT, 0, "Make SyncData Failed:{} iRet:{}", pObj->GetClassName(), iRet); ///< 记录错误日志
        if (!pObj->IsDataInited())
        {
            m_failedObjList.push_back(pObj->GetGlobalId()); ///< 添加到失败对象列表
        }
        return iRet;
    }

    // 设置同步时间和事务ID
    pObj->SetLastSyncOpTime(NF_ADJUST_TIMENOW()); ///< 设置最后同步操作时间
    pObj->SetTransID(pTrans->GetGlobalId()); ///< 设置事务ID
    
    // 发送同步请求
    iRet = pTrans->SyncReq(pMessage);
    NFLogDebug(NF_LOG_DEFAULT, 0, "sync req, serverType:{} dstServerType:{} dstBusId:{} className:{} transName:{} iRet:{}", pObj->GetServerType(), pObj->GetDstServerType(), pObj->GetDstBusId(), pObj->GetClassName(), pTrans->GetClassName(), iRet); ///< 记录调试日志
    NF_SAFE_DELETE(pMessage); ///< 删除临时数据
    
    if (iRet != 0)
    {
        if (!pObj->IsDataInited())
        {
            m_failedObjList.push_back(pObj->GetGlobalId()); ///< 添加到失败对象列表
        }
        NFLogError(NF_LOG_DEFAULT, 0, "SyncReq Failed:{} iRet:{}", pObj->GetClassName(), iRet); ///< 记录错误日志
        return iRet;
    }
    return 0;
}

NFServerSyncDataObj* NFServerSyncDataObjMgr::GetObj(int iObjID)
{
    return dynamic_cast<NFServerSyncDataObj*>(FindModule<NFIMemMngModule>()->GetObjByGlobalId(EOT_SERVER_SYNC_DATA_OBJ, iObjID, true));
}

int NFServerSyncDataObjMgr::OnDataSynced(int iObjID, const NFDataPackage& packet)
{
    NFServerSyncDataObj* pObj = GetObj(iObjID);
    CHECK_NULL(0, pObj);

    auto pMsg = pObj->CreateTempProtobufData();
    CHECK_NULL(0, pMsg);
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, (*pMsg));
    int iRet = OnDataSynced(iObjID, 0, pMsg);
    NF_SAFE_DELETE(pMsg);
    CHECK_ERR(0, iRet, "OnDataSynced Failed");
    return 0;
}

int NFServerSyncDataObjMgr::OnDataSynced(int iObjID, int32_t err_code, const google::protobuf::Message* pData)
{
    NFServerSyncDataObj* pObj = GetObj(iObjID);
    CHECK_NULL(0, pObj);

    NFLogInfo(NF_LOG_DEFAULT, 0, "objId:{} objClass:{} err_code:{}", iObjID, pObj->GetStaticClassName(), GetErrorStr(err_code));

    pObj->SetTransID(0);
    int iRet = 0;
    bool isDataInited = true;
    if (err_code == 0 && pData)
    {
        pObj->SetRetryTimes(0);
        if (!pObj->IsDataInited())
        {
            iRet = pObj->InitWithSyncData(pData);
            isDataInited = false;
        }
        else
        {
            iRet = pObj->TimerWithSyncData(pData);
        }
    }
    else
    {
        iRet = -1;
    }

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "className:{} sync data faled! iRet:{}", pObj->GetClassName(), iRet);
        switch (pObj->DealWithFailed())
        {
            case EN_DW_SYNC_LOG_FAIL:
            {
                NFLogError(NF_LOG_DEFAULT, 0, "{} Sync From Server:{} Error:{} error code:{}", pObj->GetClassName(), pObj->GetDstServerType(), iRet, err_code);
                break;
            }
            case EN_DW_SYNC_RETRY:
            {
                pObj->SetRetryTimes(pObj->GetRetryTimes() + 1);
                iRet = pObj->DelaySyncReqRetry();
                CHECK_ERR(0, iRet, "DelaySyncReqRetry failed");
                break;
            }
            case EN_DW_SYNC_SHUTDOWN:
            {
                NFLogFatal(NF_LOG_DEFAULT, 0, "className:{} Sync Failed", pObj->GetClassName());
                NFLogFatal(NF_LOG_DEFAULT, 0, "Shutdown Server by obj init faled, className:{}", pObj->GetClassName());
                assert(0);
                return -1;
            }
            case EN_DW_SYNC_RETRY_ANY_SHUTDOWN:
            {
                if (pObj->GetRetryTimes() > MAX_FAIL_RETRY_TIMES)
                {
                    NFLogFatal(NF_LOG_DEFAULT, 0, "className:{} Sync Failed", pObj->GetClassName());
                    NFLogFatal(NF_LOG_DEFAULT, 0, "Shutdown Server by obj init faled, className:{}", pObj->GetClassName());
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
        if (!isDataInited)
        {
            m_runningObjList.push_back(pObj->GetGlobalId());
            m_syncDataFinishList.insert(pObj->GetGlobalId());
        }
        pObj->SetLastSyncOpTime(NF_ADJUST_TIMENOW());
    }

    return 0;
}

int NFServerSyncDataObjMgr::Tick()
{
    for (auto iter = m_failedObjList.begin(); iter != m_failedObjList.end();)
    {
        NFServerSyncDataObj* pObj = GetObj(*iter);
        if (pObj)
        {
            int iRet = SyncReq(pObj);
            if (iRet == 0)
            {
                iter = m_failedObjList.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
        else
        {
            iter = m_failedObjList.erase(iter);
        }
    }

    if (m_iLastSyncObjIndex >= (int)m_runningObjList.size())
    {
        m_iLastSyncObjIndex = 0;
    }

    int iSavedObjNum = 0;
    int idx = 0;
    uint64_t now = NF_ADJUST_TIMENOW();
    for (auto iter = m_runningObjList.begin(); iter != m_runningObjList.end() && iSavedObjNum < MAX_SAVED_OBJ_PRE_SEC;)
    {
        if (idx < m_iLastSyncObjIndex)
        {
            ++idx;
            ++iter;
            continue;
        }

        NFServerSyncDataObj* pObj = GetObj(*iter);
        if (pObj)
        {
            // 不在存储中 + 有修改
            if (pObj->GetTransID() == 0)
            {
                if (pObj->GetLastSyncOpTime() + pObj->GetSyncDis() < now)
                {
                    int iRet = SyncReq(pObj);
                    if (iRet != 0)
                    {
                        LOG_ERR(0, iRet, "SyncReq failed, className:{}", pObj->GetClassName());
                    }

                    ++iSavedObjNum;
                }
            }
            ++iter;
            ++idx;
            ++m_iLastSyncObjIndex;
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Remove running obj:{}", *iter);
            iter = m_runningObjList.erase(iter);
        }
    }

    if (m_syncDataList.size() > 0 && m_syncDataList.size() == m_syncDataFinishList.size())
    {
        m_syncDataList.clear();
        m_syncDataFinishList.clear();
        CheckWhenAllDataLoaded();
        NFGlobalSystem::Instance()->GetGlobalPluginManager()->FinishAppTask(NF_ST_NONE, APP_INIT_SYNC_SERVER_GLOBAL_DATA, APP_INIT_TASK_GROUP_SERVER_SYNC_GLOBAL_DATA);
    }

    return 0;
}

int NFServerSyncDataObjMgr::CheckWhenAllDataLoaded()
{
    for (auto iter = m_runningObjList.begin(); iter != m_runningObjList.end(); iter++)
    {
        NFServerSyncDataObj* pObj = GetObj(*iter);
        if (pObj)
        {
            int iRet = pObj->CheckWhenAllDataLoaded();
            if (iRet != 0)
            {
                NFLogFatal(NF_LOG_DEFAULT, 0, "Shutdown Server by obj check faled, className:{}", pObj->GetClassName());
                assert(0);
            }
        }
    }
    return 0;
}

int NFServerSyncDataObjMgr::CheckStopServer()
{
    return 0;
}

int NFServerSyncDataObjMgr::StopServer()
{
    return 0;
}
