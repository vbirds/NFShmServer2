// -------------------------------------------------------------------------
//    @FileName         :    NFDBObjTrans.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDBObjTrans.cpp
//    @Desc             :    数据库对象事务实现文件，提供数据库对象的事务处理功能实现。
//                          该文件实现了数据库对象事务类的方法，包括事务初始化、数据操作、
//                          事务超时处理、事务完成处理。
//                          主要功能包括提供数据库对象的事务处理实现、支持数据插入和保存、
//                          支持数据加载、提供事务超时和完成处理。
//                          数据库对象事务实现是NFShmXFrame框架的数据库事务处理组件实现，负责：
//                          - 数据库对象的事务处理实现
//                          - 数据插入、保存、加载操作实现
//                          - 事务超时和完成处理实现
//                          - 事务状态管理和生命周期实现
//                          - 数据库操作的原子性保证实现
//                          - 跨服务器事务同步实现
//
// -------------------------------------------------------------------------

#include "NFDBObjTrans.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFDBObjMgr.h"
#include "NFIServerMessageModule.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFDBObjTrans::NFDBObjTrans()
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
NFDBObjTrans::~NFDBObjTrans()
{
}

/**
 * @brief 创建初始化
 * 
 * 在对象创建时调用，初始化数据库对象事务的所有成员变量
 * 
 * @return 初始化结果
 */
int NFDBObjTrans::CreateInit()
{
    m_iLinkedObjID = 0;       // 关联对象ID为0
    m_iObjSeqOP = 0;          // 对象序列操作为0
    m_iDBOP = 0;              // 数据库操作为0
    m_iServerType = NF_ST_NONE; // 服务器类型为无
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 在对象恢复时调用，恢复数据库对象事务状态
 * 
 * @return 初始化结果
 */
int NFDBObjTrans::ResumeInit()
{
    return 0;
}

/**
 * @brief 初始化事务
 * 
 * 初始化数据库对象事务，设置服务器类型、对象ID和序列操作
 * 
 * @param eType 服务器类型
 * @param iObjID 对象ID
 * @param iSeqOP 序列操作
 * @return 初始化结果
 */
int NFDBObjTrans::Init(NF_SERVER_TYPE eType, int iObjID, uint32_t iSeqOP)
{
    m_iLinkedObjID = iObjID;  // 设置关联对象ID
    m_iObjSeqOP = iSeqOP;     // 设置对象序列操作
    m_iServerType = eType;     // 设置服务器类型
    return 0;
}

/**
 * @brief 插入数据
 * 
 * 向数据库插入数据，通过RPC调用存储服务器
 * 
 * @param iModKey 模块键值
 * @param data 数据对象
 * @return 插入结果
 */
int NFDBObjTrans::Insert(uint64_t iModKey, google::protobuf::Message* data)
{
    CHECK_NULL(0, data);
    NFLogDebug(NF_LOG_DEFAULT, 0, "InsertToDB, tableName:{} trans:{} msg:{}", data->GetTypeName(), GetGlobalId(), data->DebugString());

    // 设置数据库操作类型为插入对象
    m_iDBOP = NFrame::NF_STORESVR_C2S_INSERTOBJ;
    
    // 调用RPC插入对象服务
    m_rpcId = FindModule<NFIServerMessageModule>()->GetRpcInsertObjService(m_iServerType, iModKey, *data, [this](int rpcRetCode)
    {
        if (rpcRetCode == NFrame::ERR_CODE_SVR_OK)
        {
            NFDBObjMgr::Instance()->OnDataInserted(this, true);
        }

        SetFinished(rpcRetCode);
    });

    if (m_rpcId == INVALID_ID)
    {
        SetFinished(0);
        return NFrame::ERR_CODE_RPC_SYSTEM_ERROR;
    }
    return 0;
}

/**
 * @brief 保存数据
 * 
 * 保存数据到数据库，通过RPC调用存储服务器
 * 
 * @param iModKey 模块键值
 * @param data 数据对象
 * @return 保存结果
 */
int NFDBObjTrans::Save(uint64_t iModKey, google::protobuf::Message* data)
{
    CHECK_NULL(0, data);
    NFLogTrace(NF_LOG_DEFAULT, 0, "SaveToDB, tableName:{} trans:{} ", data->GetTypeName(), GetGlobalId());

    // 设置数据库操作类型为修改对象
    m_iDBOP = NFrame::NF_STORESVR_C2S_MODIFYOBJ;
    
    // 调用RPC修改对象服务
    m_rpcId = FindModule<NFIServerMessageModule>()->GetRpcModifyObjService(m_iServerType, iModKey, *data, [this](int rpcRetCode)
    {
        if (rpcRetCode == NFrame::ERR_CODE_SVR_OK)
        {
            NFDBObjMgr::Instance()->OnDataSaved(this, true);
        }

        SetFinished(rpcRetCode);
    });

    if (m_rpcId == INVALID_ID)
    {
        SetFinished(0);
        return NFrame::ERR_CODE_RPC_SYSTEM_ERROR;
    }
    return 0;
}

int NFDBObjTrans::Load(uint64_t iModKey, google::protobuf::Message* data)
{
    CHECK_NULL(0, data);
    NFLogDebug(NF_LOG_DEFAULT, 0, "LoadFromDB, tableName:{} trans:{} msg:{}", data->GetTypeName(), GetGlobalId(), data->DebugString());

    m_iDBOP = NFrame::NF_STORESVR_C2S_SELECTOBJ;
    m_rpcId = FindModule<NFIServerMessageModule>()->GetRpcSelectObjService(m_iServerType, iModKey, *data, [this](int rpcRetCode, google::protobuf::Message &respone)
    {
        if (rpcRetCode == NFrame::ERR_CODE_SVR_OK)
        {
            NFDBObjMgr::Instance()->OnDataLoaded(m_iLinkedObjID, rpcRetCode, &respone);
            SetFinished(0);
        }
        else if (rpcRetCode == NFrame::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY)
        {
            NFDBObjMgr::Instance()->OnDataLoaded(m_iLinkedObjID, rpcRetCode, NULL);
            SetFinished(0);
        }
        else
        {
            SetFinished(rpcRetCode);
        }
    });

    if (m_rpcId == INVALID_ID)
    {
        SetFinished(0);
        return NFrame::ERR_CODE_RPC_SYSTEM_ERROR;
    }
    return 0;
}

int NFDBObjTrans::OnTimeOut()
{
    return 0;
}

int NFDBObjTrans::HandleTransFinished(int iRunLogicRetCode)
{
    if (iRunLogicRetCode == 0)
    {
        return 0;
    }

    switch (m_iDBOP)
    {
        case NFrame::NF_STORESVR_C2S_SELECTOBJ:
        {
            if (iRunLogicRetCode == NFrame::ERR_CODE_SVR_SYSTEM_TIMEOUT)
            {
                NFDBObjMgr::Instance()->OnDataLoaded(m_iLinkedObjID, NFrame::ERR_CODE_STORESVR_ERRCODE_BUSY, NULL);
                NFLogError(NF_LOG_DEFAULT, 0, "load obj timeout:{}", m_iLinkedObjID);
            }
            else
            {
                NFDBObjMgr::Instance()->OnDataLoaded(m_iLinkedObjID, NFrame::ERR_CODE_STORESVR_ERRCODE_UNKNOWN, NULL);
                NFLogError(NF_LOG_DEFAULT, 0, "load obj:{} error:{}", m_iLinkedObjID, GetErrorStr(iRunLogicRetCode));
            }
            break;
        }
        case NFrame::NF_STORESVR_C2S_INSERTOBJ:
        {
            NFDBObjMgr::Instance()->OnDataInserted(this, false);
            if (iRunLogicRetCode == NFrame::ERR_CODE_SVR_SYSTEM_TIMEOUT)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "insert obj timeout:{}", m_iLinkedObjID);
            }
            else
            {
                NFLogError(NF_LOG_DEFAULT, 0, "insert obj:{} error:{}", m_iLinkedObjID, GetErrorStr(iRunLogicRetCode));
            }
            break;
        }
        case NFrame::NF_STORESVR_C2S_MODIFYOBJ:
        {
            NFDBObjMgr::Instance()->OnDataSaved(this, false);
            if (iRunLogicRetCode == NFrame::ERR_CODE_SVR_SYSTEM_TIMEOUT)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "save obj timeout:{}", m_iLinkedObjID);
            }
            else
            {
                NFLogError(NF_LOG_DEFAULT, 0, "save obj:{} error:{}", m_iLinkedObjID, GetErrorStr(iRunLogicRetCode));
            }

            break;
        }
        default:
        {
            NFLogFatal(NF_LOG_DEFAULT, 0, "unkown db op:{} obj_id:{}", m_iDBOP, m_iLinkedObjID);
            break;
        }
    }

    return 0;
}

