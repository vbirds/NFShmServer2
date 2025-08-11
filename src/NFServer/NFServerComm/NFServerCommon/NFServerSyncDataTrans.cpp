// -------------------------------------------------------------------------
//    @FileName         :    NFServerSyncDataTrans.cpp
//    @Author           :    gaoyi
//    @Date             :    2025-07-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerSyncDataTrans
//    @Desc             :    服务器同步数据事务实现文件，提供服务器间数据同步的事务处理功能实现。
//                          该文件实现了服务器同步数据事务类的方法，包括事务初始化、
//                          同步请求处理、响应处理、事务完成处理。
//                          主要功能包括提供服务器间数据同步的事务处理实现、支持事务初始化、
//                          支持同步请求和响应处理、提供事务完成处理。
//                          服务器同步数据事务实现是NFShmXFrame框架的服务器同步事务组件实现，负责：
//                          - 服务器间数据同步的事务处理实现
//                          - 事务初始化和状态管理实现
//                          - 同步请求和响应处理实现
//                          - 事务完成和错误处理实现
//                          - 跨服务器事务通信实现
//                          - 数据一致性保证实现
//
// -------------------------------------------------------------------------

#include "NFServerSyncDataTrans.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFIServerMessageModule.h"
#include "NFServerSyncDataObjMgr.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFServerSyncDataTrans::NFServerSyncDataTrans()
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
NFServerSyncDataTrans::~NFServerSyncDataTrans()
{
}

/**
 * @brief 创建初始化
 * 
 * 初始化所有成员变量为默认值
 * 
 * @return 0表示成功
 */
int NFServerSyncDataTrans::CreateInit()
{
    m_iLinkedObjID = 0; ///< 关联对象ID为0
    m_iServerType = NF_ST_NONE; ///< 服务器类型为无
    m_iDstServerType = NF_ST_NONE; ///< 目标服务器类型为无
    m_iDstBusId = 0; ///< 目标总线ID为0
    m_iMsgId = 0; ///< 消息ID为0
    m_iModuleId = 0; ///< 模块ID为0
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 从共享内存恢复对象状态
 * 
 * @return 0表示成功
 */
int NFServerSyncDataTrans::ResumeInit()
{
    return 0;
}

/**
 * @brief 初始化事务
 * 
 * 设置事务的所有参数，包括服务器类型、目标服务器、总线ID等
 * 
 * @param type 源服务器类型
 * @param dstType 目标服务器类型
 * @param dstBusId 目标总线ID
 * @param moduleId 模块ID
 * @param msgId 消息ID
 * @param iObjID 关联对象ID
 * @return 0表示成功
 */
int NFServerSyncDataTrans::Init(NF_SERVER_TYPE type, NF_SERVER_TYPE dstType, uint32_t dstBusId, uint32_t moduleId, uint32_t msgId, int iObjID)
{
    m_iServerType = type; ///< 设置源服务器类型
    m_iDstServerType = dstType; ///< 设置目标服务器类型
    m_iDstBusId = dstBusId; ///< 设置目标总线ID
    m_iMsgId = msgId; ///< 设置消息ID
    m_iModuleId = moduleId; ///< 设置模块ID
    m_iLinkedObjID = iObjID; ///< 设置关联对象ID
    return 0;
}

/**
 * @brief 发送同步请求
 * 
 * 通过消息模块发送同步请求到目标服务器
 * 
 * @param pData 要同步的数据
 * @return 0表示成功
 */
int NFServerSyncDataTrans::SyncReq(google::protobuf::Message* pData)
{
    CHECK_NULL(0, pData); ///< 检查数据指针
    NFLogDebug(NF_LOG_DEFAULT, 0, "SyncReq, trans:{} msg:{}", GetGlobalId(), pData->DebugString()); ///< 记录调试日志

    // 通过消息模块发送事务请求
    FindModule<NFIMessageModule>()->SendTrans(m_iServerType, m_iDstServerType, 0, m_iDstBusId, m_iModuleId, m_iMsgId, *pData, GetGlobalId());
    return 0;
}

/**
 * @brief 处理分发服务器响应
 * 
 * 处理来自分发服务器的响应消息
 * 
 * @param nMsgId 消息ID
 * @param packet 数据包
 * @param reqTransId 请求事务ID
 * @param rspTransId 响应事务ID
 * @return 0表示成功
 */
int NFServerSyncDataTrans::HandleDispSvrRes(uint32_t nMsgId, const NFDataPackage& packet, uint32_t reqTransId, uint32_t rspTransId)
{
    // 检查模块ID和消息ID是否匹配
    CHECK_EXPR(packet.mModuleId == m_iModuleId && packet.nMsgId == m_iMsgId, -1, "packet.mModuleId:{} != m_iModuleId:{} || packet.nMsgId:{} == m_iMsgId:{}", packet.mModuleId, m_iModuleId, packet.nMsgId, m_iMsgId);

    // 通知同步数据对象管理器数据已同步
    int iRetCode = NFServerSyncDataObjMgr::Instance()->OnDataSynced(m_iLinkedObjID, packet);
    CHECK_ERR(0, iRetCode, "OnDataSynced Failed"); ///< 检查同步结果
    SetFinished(0); ///< 设置事务完成
    return 0;
}

/**
 * @brief 处理事务完成
 * 
 * 处理事务完成事件，包括成功和失败的情况
 * 
 * @param iRunLogicRetCode 运行逻辑返回码
 * @return 0表示成功
 */
int NFServerSyncDataTrans::HandleTransFinished(int iRunLogicRetCode)
{
    if (iRunLogicRetCode == 0)
    {
        return 0; ///< 成功情况直接返回
    }

    // 处理超时错误
    if (iRunLogicRetCode == NFrame::ERR_CODE_SVR_SYSTEM_TIMEOUT)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "sync data timeout:{}", m_iLinkedObjID); ///< 记录超时错误
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "sync data obj:{} error:{}", m_iLinkedObjID, GetErrorStr(iRunLogicRetCode)); ///< 记录其他错误
    }
    
    // 通知同步数据对象管理器同步失败
    NFServerSyncDataObjMgr::Instance()->OnDataSynced(m_iLinkedObjID, iRunLogicRetCode, nullptr);

    return 0;
}
