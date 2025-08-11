// -------------------------------------------------------------------------
//    @FileName         :    NFServerMessageModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerMessageModule
//    @Desc             :    NFShmXFrame服务器消息模块实现文件
//    
//    该文件实现了NFShmXFrame框架中服务器消息处理模块的具体功能。
//    NFServerMessageModule是NFIServerMessageModule接口的具体实现，
//    负责处理服务器间的消息传递、数据同步、事务处理等核心功能。
//    
//    主要实现内容：
//    - 服务器间消息传递和路由
//    - 跨服务器数据同步
//    - 事务处理和状态管理
//    - 多种消息格式支持（Protobuf、字符串、二进制）
//    - 支持多种服务器类型（Master、World、Center、Game、Logic等）
//    - 代理服务器消息转发
//    - 存储服务器数据操作
//    - 消息重定向和广播
//
// -------------------------------------------------------------------------

#include "NFServerMessageModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFICoroutineModule.h"
#include "NFComm/NFPluginModule/NFProtobufCommon.h"

/**
 * @brief 构造函数
 * 
 * 初始化服务器消息模块，设置插件管理器
 * 
 * @param pPluginManager 插件管理器指针
 */
NFServerMessageModule::NFServerMessageModule(NFIPluginManager* pPluginManager) : NFIServerMessageModule(pPluginManager)
{
}

/**
 * @brief 析构函数
 * 
 * 清理服务器消息模块资源
 */
NFServerMessageModule::~NFServerMessageModule()
{
}

/**
 * @brief 向主服务器发送消息
 * 
 * 通过消息模块向主服务器发送Protobuf消息
 * 
 * @param eSendType 发送服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToMasterServer(NF_SERVER_TYPE eSendType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1,
                                                 uint64_t nParam2)
{
    auto pServerData = FindModule<NFIMessageModule>()->GetMasterData(eSendType);
    CHECK_EXPR(pServerData, -1, "pServerData == NULL, eType error:{}", (int) eSendType);

    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nMsgId, xData, nParam1, nParam2);
    return 0;
}

/**
 * @brief 向主服务器发送消息（带模块ID）
 * 
 * 通过消息模块向主服务器发送Protobuf消息，指定模块ID
 * 
 * @param eSendType 发送服务器类型
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToMasterServer(NF_SERVER_TYPE eSendType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1,
                                                 uint64_t nParam2)
{
    auto pServerData = FindModule<NFIMessageModule>()->GetMasterData(eSendType);
    CHECK_EXPR(pServerData, -1, "pServerData == NULL, eType error:{}", (int) eSendType);

    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, xData, nParam1, nParam2);
    return 0;
}

/**
 * @brief 通过Bus ID向代理服务器发送消息
 * 
 * 通过消息模块向指定Bus ID的代理服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标Bus ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1
 * @param nParam2 参数2
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                               uint64_t nParam1, uint64_t nParam2)
{
    return SendProxyMsgByBusId(eType, nDstId, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 通过Bus ID向代理服务器发送消息（带模块ID）
 * 
 * 通过消息模块向指定Bus ID的代理服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标Bus ID
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId,
                                               const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2)
{
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
    CHECK_EXPR(pConfig, -1, "pConfig == NULL");

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(eType, nDstId);
    CHECK_EXPR(pServerData, -1, "pServerData == NULL, busId:{}", nDstId);

    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, xData, nParam1, nParam2, pConfig->BusId, nDstId);
    return 0;
}

/**
 * @brief 通过Bus ID向代理服务器发送二进制消息
 * 
 * 通过消息模块向指定Bus ID的代理服务器发送二进制消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标Bus ID
 * @param nMsgId 消息ID
 * @param msg 消息数据
 * @param nLen 消息长度
 * @param nParam1 参数1
 * @param nParam2 参数2
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const char* msg, uint32_t nLen, uint64_t nParam1,
                                               uint64_t nParam2)
{
    return SendProxyMsgByBusId(eType, nDstId, NF_MODULE_SERVER, nMsgId, msg, nLen, nParam1, nParam2);
}

/**
 * @brief 通过Bus ID向代理服务器发送二进制消息（带模块ID）
 * 
 * 通过消息模块向指定Bus ID的代理服务器发送二进制消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标Bus ID
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param msg 消息数据
 * @param nLen 消息长度
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const char* msg,
                                               uint32_t nLen, uint64_t nParam1, uint64_t nParam2)
{
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
    CHECK_EXPR(pConfig, -1, "pConfig == NULL");

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(eType, nDstId);
    CHECK_EXPR(pServerData, -1, "pServerData == NULL, busId:{}", nDstId);

    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, msg, nLen, nParam1, nParam2, pConfig->BusId, nDstId);
    return 0;
}

/**
 * @brief 向代理服务器发送重定向消息
 * 
 * 向指定代理服务器发送重定向消息，支持指定多个目标ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标代理服务器ID
 * @param ids 目标ID集合
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendRedirectMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, const std::unordered_set<uint64_t>& ids, uint32_t nMsgId,
                                                        const google::protobuf::Message& xData)
{
    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(nMsgId);
    svrPkg.set_msg_data(xData.SerializePartialAsString());

    for (auto iter = ids.begin(); iter != ids.end(); iter++)
    {
        svrPkg.mutable_redirect_info()->add_id(*iter);
    }

    SendMsgToProxyServer(eType, nDstId, NF_MODULE_FRAME, NFrame::NF_SERVER_REDIRECT_MSG_TO_PROXY_SERVER_CMD, svrPkg);
    return 0;
}

/**
 * @brief 向所有代理服务器发送重定向消息
 * 
 * 向指定服务器类型的所有代理服务器发送重定向消息，支持指定多个目标ID
 * 
 * @param eType 服务器类型
 * @param ids 目标ID集合
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendRedirectMsgToAllProxyServer(NF_SERVER_TYPE eType, const std::unordered_set<uint64_t>& ids, uint32_t nMsgId,
                                                           const google::protobuf::Message& xData)
{
    CHECK_EXPR(ids.size() > 0, 0, "ids empty");

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(nMsgId);
    svrPkg.set_msg_data(xData.SerializePartialAsString());

    for (auto iter = ids.begin(); iter != ids.end(); iter++)
    {
        svrPkg.mutable_redirect_info()->add_id(*iter);
    }

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(eType, NF_ST_PROXY_SERVER);
    for (int i = 0; i < (int)vecServer.size(); i++)
    {
        auto pServerData = vecServer[i];
        if (pServerData)
        {
            SendMsgToProxyServer(eType, pServerData->mServerInfo.bus_id(), NF_MODULE_FRAME, NFrame::NF_SERVER_REDIRECT_MSG_TO_PROXY_SERVER_CMD, svrPkg);
        }
    }

    return 0;
}

/**
 * @brief 向所有代理服务器发送重定向消息（指定所有）
 * 
 * 向指定服务器类型的所有代理服务器发送重定向消息，并设置为所有代理服务器
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendRedirectMsgToAllProxyServer(NF_SERVER_TYPE eType, uint32_t nMsgId,
                                                           const google::protobuf::Message& xData)
{
    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(nMsgId);
    svrPkg.set_msg_data(xData.SerializePartialAsString());

    svrPkg.mutable_redirect_info()->set_all(true);

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(eType, NF_ST_PROXY_SERVER);
    for (int i = 0; i < (int)vecServer.size(); i++)
    {
        auto pServerData = vecServer[i];
        if (pServerData)
        {
            SendMsgToProxyServer(eType, pServerData->mServerInfo.bus_id(), NF_MODULE_FRAME, NFrame::NF_SERVER_REDIRECT_MSG_TO_PROXY_SERVER_CMD, svrPkg);
        }
    }

    return 0;
}

/**
 * @brief 向代理服务器发送消息
 * 
 * 通过消息模块向代理服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标代理服务器ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                                uint64_t nParam1, uint64_t nParam2)
{
    return SendMsgToProxyServer(eType, nDstId, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向代理服务器发送消息（带模块ID）
 * 
 * 通过消息模块向代理服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标代理服务器ID
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId,
                                                const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2)
{
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
    CHECK_EXPR(pConfig, -1, "pConfig == NULL");

    auto pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(eType, NF_ST_PROXY_AGENT_SERVER);
    if (pServerData)
    {
        FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, xData, nParam1, nParam2, pConfig->BusId, nDstId);
        return 0;
    }

    pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(eType, nDstId);
    CHECK_EXPR(pServerData, -1, "pServerData == NULL, busId:{}", nDstId);
    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, xData, nParam1, nParam2, pConfig->BusId, nDstId);
    return 0;
}

/**
 * @brief 向代理服务器发送消息（字符串）
 * 
 * 通过消息模块向代理服务器发送字符串消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标代理服务器ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const std::string& xData,
                                                uint64_t nParam1, uint64_t nParam2)
{
    return SendMsgToProxyServer(eType, nDstId, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向代理服务器发送消息（带模块ID，字符串）
 * 
 * 通过消息模块向代理服务器发送字符串消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标代理服务器ID
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId,
                                                const std::string& xData, uint64_t nParam1, uint64_t nParam2)
{
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
    CHECK_EXPR(pConfig, -1, "pConfig == NULL");

    auto pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(eType, NF_ST_PROXY_AGENT_SERVER);
    if (pServerData)
    {
        FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, xData, nParam1, nParam2, pConfig->BusId, nDstId);
        return 0;
    }

    pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(eType, nDstId);
    CHECK_EXPR(pServerData, -1, "pServerData == NULL, busId:{}", nDstId);
    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, xData, nParam1, nParam2, pConfig->BusId, nDstId);
    return 0;
}

/**
 * @brief 向代理服务器发送消息（二进制）
 * 
 * 通过消息模块向代理服务器发送二进制消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标代理服务器ID
 * @param nMsgId 消息ID
 * @param pData 消息数据
 * @param dataLen 消息长度
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const char* pData, int dataLen,
                                                uint64_t nParam1, uint64_t nParam2)
{
    return SendMsgToProxyServer(eType, nDstId, NF_MODULE_SERVER, nMsgId, pData, dataLen, nParam1, nParam2);
}

/**
 * @brief 向代理服务器发送消息（带模块ID，二进制）
 * 
 * 通过消息模块向代理服务器发送二进制消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标代理服务器ID
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param pData 消息数据
 * @param dataLen 消息长度
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId,
                                                const char* pData, int dataLen, uint64_t nParam1, uint64_t nParam2)
{
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
    CHECK_EXPR(pConfig, -1, "pConfig == NULL");

    auto pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(eType, NF_ST_PROXY_AGENT_SERVER);
    if (pServerData)
    {
        FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, pData, dataLen, nParam1, nParam2, pConfig->BusId, nDstId);
        return 0;
    }

    pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(eType, nDstId);
    CHECK_EXPR(pServerData, -1, "pServerData == NULL, busId:{}", nDstId);
    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, nModuleId, nMsgId, pData, dataLen, nParam1, nParam2, pConfig->BusId, nDstId);
    return 0;
}

/**
 * @brief 向世界服务器发送消息
 * 
 * 通过消息模块向世界服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToWorldServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1,
                                                uint64_t nParam2)
{
    return SendMsgToWorldServer(eType, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向世界服务器发送消息（带模块ID）
 * 
 * 通过消息模块向世界服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToWorldServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                                uint64_t nParam1, uint64_t nParam2)
{
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_WORLD_SERVER, 0, 0, nModuleId, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向世界服务器发送事务消息
 * 
 * 通过消息模块向世界服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param req_trans_id 请求事务ID
 * @param rsp_trans_id 响应事务ID
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToWorldServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id,
                                                  uint32_t rsp_trans_id)
{
    return FindModule<NFIMessageModule>()->SendTrans(eType, NF_ST_WORLD_SERVER, 0, 0, nMsgId, xData, req_trans_id, rsp_trans_id);
}

/**
 * @brief 向中心服务器发送消息
 * 
 * 通过消息模块向中心服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1,
                                                 uint64_t nParam2)
{
    return SendMsgToCenterServer(eType, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向中心服务器发送消息（带模块ID）
 * 
 * 通过消息模块向中心服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToCenterServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                                 uint64_t nParam1, uint64_t nParam2)
{
    auto pServerData = FindModule<NFIMessageModule>()->GetFirstServerByServerType(eType, NF_ST_CENTER_SERVER, false);
    CHECK_NULL(0, pServerData);
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_CENTER_SERVER, 0, pServerData->mServerInfo.bus_id(), nModuleId, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向中心服务器发送事务消息
 * 
 * 通过消息模块向中心服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param req_trans_id 请求事务ID
 * @param rsp_trans_id 响应事务ID
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id,
                                                   uint32_t rsp_trans_id)
{
    auto pServerData = FindModule<NFIMessageModule>()->GetFirstServerByServerType(eType, NF_ST_CENTER_SERVER, false);
    CHECK_NULL(0, pServerData);
    return FindModule<NFIMessageModule>()->SendTrans(eType, NF_ST_CENTER_SERVER, 0, pServerData->mServerInfo.bus_id(), nMsgId, xData, req_trans_id, rsp_trans_id);
}

/**
 * @brief 向跨中心服务器发送消息
 * 
 * 通过消息模块向跨中心服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1,
                                                      uint64_t nParam2)
{
    return SendMsgToCrossCenterServer(eType, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向跨中心服务器发送消息（带模块ID）
 * 
 * 通过消息模块向跨中心服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                                      uint64_t nParam1, uint64_t nParam2)
{
    auto pServerData = FindModule<NFIMessageModule>()->GetFirstServerByServerType(eType, NF_ST_CENTER_SERVER, true);
    CHECK_NULL(0, pServerData);
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_CENTER_SERVER, 0, pServerData->mServerInfo.bus_id(), nModuleId, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向跨中心服务器发送事务消息
 * 
 * 通过消息模块向跨中心服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param req_trans_id 请求事务ID
 * @param rsp_trans_id 响应事务ID
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id,
                                                        uint32_t rsp_trans_id)
{
    auto pServerData = FindModule<NFIMessageModule>()->GetFirstServerByServerType(eType, NF_ST_CENTER_SERVER, true);
    CHECK_NULL(0, pServerData);
    return FindModule<NFIMessageModule>()->SendTrans(eType, NF_ST_CENTER_SERVER, 0, pServerData->mServerInfo.bus_id(), nMsgId, xData, req_trans_id, rsp_trans_id);
}

/**
 * @brief 向社交服务器发送消息
 * 
 * 通过消息模块向社交服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param snsType 社交服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToSnsServerBySnsType(NF_SERVER_TYPE eType, NF_SNS_SERVER_TYPE snsType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2)
{
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_SNS_SERVER, 0, snsType, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向社交服务器发送消息
 * 
 * 通过消息模块向社交服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToSnsServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1,
                                              uint64_t nParam2)
{
    return SendMsgToSnsServer(eType, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向社交服务器发送消息（带模块ID）
 * 
 * 通过消息模块向社交服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToSnsServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                              uint64_t nParam1, uint64_t nParam2)
{
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_SNS_SERVER, 0, 0, nModuleId, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向社交服务器发送事务消息
 * 
 * 通过消息模块向社交服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param req_trans_id 请求事务ID
 * @param rsp_trans_id 响应事务ID
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToSnsServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id,
                                                uint32_t rsp_trans_id)
{
    return FindModule<NFIMessageModule>()->SendTrans(eType, NF_ST_SNS_SERVER, 0, 0, nMsgId, xData, req_trans_id, rsp_trans_id);
}

/**
 * @brief 向在线服务器发送消息
 * 
 * 通过消息模块向在线服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToOnlineServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2)
{
    return SendMsgToOnlineServer(eType, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向在线服务器发送消息（带模块ID）
 * 
 * 通过消息模块向在线服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToOnlineServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2)
{
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_ONLINE_SERVER, 0, 0, nModuleId, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向在线服务器发送事务消息
 * 
 * 通过消息模块向在线服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param req_trans_id 请求事务ID
 * @param rsp_trans_id 响应事务ID
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToOnlineServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id, uint32_t rsp_trans_id)
{
    return FindModule<NFIMessageModule>()->SendTrans(eType, NF_ST_ONLINE_SERVER, 0, 0, nMsgId, xData, req_trans_id, rsp_trans_id);
}

/**
 * @brief 向游戏服务器发送消息
 * 
 * 通过消息模块向游戏服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标游戏服务器ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                               uint64_t nParam1, uint64_t nParam2)
{
    return SendMsgToGameServer(eType, nDstId, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向游戏服务器发送消息（带模块ID）
 * 
 * 通过消息模块向游戏服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标游戏服务器ID
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId,
                                               const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2)
{
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_GAME_SERVER, 0, nDstId, nModuleId, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向游戏服务器发送事务消息
 * 
 * 通过消息模块向游戏服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标游戏服务器ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param req_trans_id 请求事务ID
 * @param rsp_trans_id 响应事务ID
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                                 uint32_t req_trans_id, uint32_t rsp_trans_id)
{
    return FindModule<NFIMessageModule>()->SendTrans(eType, NF_ST_GAME_SERVER, 0, nDstId, nMsgId, xData, req_trans_id, rsp_trans_id);
}

/**
 * @brief 向逻辑服务器发送消息
 * 
 * 通过消息模块向逻辑服务器发送Protobuf消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标逻辑服务器ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                                uint64_t nParam1, uint64_t nParam2)
{
    return SendMsgToLogicServer(eType, nDstId, NF_MODULE_SERVER, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向逻辑服务器发送消息（带模块ID）
 * 
 * 通过消息模块向逻辑服务器发送Protobuf消息，指定模块ID
 * 
 * @param eType 服务器类型
 * @param nDstId 目标逻辑服务器ID
 * @param nModuleId 模块ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param nParam1 参数1，默认为0
 * @param nParam2 参数2，默认为0
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendMsgToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId,
                                                const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2)
{
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_LOGIC_SERVER, 0, nDstId, nModuleId, nMsgId, xData, nParam1, nParam2);
}

/**
 * @brief 向逻辑服务器发送事务消息
 * 
 * 通过消息模块向逻辑服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param nDstId 目标逻辑服务器ID
 * @param nMsgId 消息ID
 * @param xData 消息数据
 * @param req_trans_id 请求事务ID
 * @param rsp_trans_id 响应事务ID
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData,
                                                  uint32_t req_trans_id, uint32_t rsp_trans_id)
{
    return FindModule<NFIMessageModule>()->SendTrans(eType, NF_ST_LOGIC_SERVER, 0, nDstId, nMsgId, xData, req_trans_id, rsp_trans_id);
}

/**
 * @brief 向存储服务器发送事务消息
 * 
 * 通过消息模块向存储服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param dstBusId 目标存储服务器Bus ID
 * @param cmd 命令类型
 * @param table_id 表ID
 * @param dbname 数据库名
 * @param table_name 表名
 * @param xData 数据
 * @param vk_list 键值对列表
 * @param where_addtional_conds 附加条件
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param mod_key 模块键
 * @param cls_name 类名
 * @param packet_type 数据包类型
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id, const std::string& dbname,
                                                  const std::string& table_name, const google::protobuf::Message& xData,
                                                  const std::vector<NFrame::storesvr_vk>& vk_list,
                                                  const std::string& where_addtional_conds, int trans_id, uint32_t seq,
                                                  uint64_t mod_key, const std::string& cls_name, uint8_t packet_type)
{
    CHECK_EXPR(cmd == NFrame::NF_STORESVR_C2S_MODIFY || cmd == NFrame::NF_STORESVR_C2S_UPDATE, -1, "error cmd:{}", cmd);
    CHECK_EXPR(NFrame::PacketDispType_IsValid(packet_type), -1, "error msg_type:{}", packet_type);


    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(cmd);
    svrPkg.mutable_disp_info()->set_type((NFrame::PacketDispType)packet_type);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    NFLogTrace(NF_LOG_DEFAULT, mod_key, "cmd:{} table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               cmd, table_id, table_name, trans_id, seq, mod_key);

    switch (cmd)
    {
        case NFrame::NF_STORESVR_C2S_MODIFY:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_modifybycond(dbname, table_name, mod_key, xData, vk_list, where_addtional_conds, cls_name));
            break;
        }
        case NFrame::NF_STORESVR_C2S_UPDATE:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_updatebycond(dbname, table_name, mod_key, xData, vk_list, where_addtional_conds, cls_name));
            break;
        }
        default:
        {
            break;
        }
    }

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId,
                                                           NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}

/**
 * @brief 向存储服务器发送事务消息
 * 
 * 通过消息模块向存储服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param dstBusId 目标存储服务器Bus ID
 * @param cmd 命令类型
 * @param table_id 表ID
 * @param dbname 数据库名
 * @param table_name 表名
 * @param vecFields 字段列表
 * @param vk_list 键值对列表
 * @param where_addtional_conds 附加条件
 * @param max_records 最大记录数
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param mod_key 模块键
 * @param cls_name 类名
 * @param packet_type 数据包类型
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id, const std::string& dbname,
                                                  const std::string& table_name, const std::vector<std::string>& vecFields,
                                                  const std::vector<NFrame::storesvr_vk>& vk_list,
                                                  const std::string& where_addtional_conds, int max_records, int trans_id, uint32_t seq,
                                                  uint64_t mod_key, const std::string& cls_name, uint8_t packet_type)
{
    CHECK_EXPR(cmd == NFrame::NF_STORESVR_C2S_SELECT || cmd == NFrame::NF_STORESVR_C2S_DELETE, -1, "error cmd:{}", cmd);
    CHECK_EXPR(NFrame::PacketDispType_IsValid(packet_type), -1, "error msg_type:{}", packet_type);

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(cmd);
    svrPkg.mutable_disp_info()->set_type((NFrame::PacketDispType)packet_type);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    NFLogTrace(NF_LOG_DEFAULT, mod_key, "cmd:{} table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               cmd, table_id, table_name, trans_id, seq, mod_key);;

    switch (cmd)
    {
        case NFrame::NF_STORESVR_C2S_SELECT:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_selectbycond(dbname, table_name, mod_key, vecFields, vk_list, where_addtional_conds, max_records, cls_name));
            break;
        }
        case NFrame::NF_STORESVR_C2S_DELETE:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_deletebycond(dbname, table_name, mod_key, vk_list, cls_name));
            break;
        }
        default:
        {
            break;
        }
    }

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId,
                                                           NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}

/**
 * @brief 向存储服务器发送事务消息
 * 
 * 通过消息模块向存储服务器发送事务消息
 * 
 * @param eType 服务器类型
 * @param dstBusId 目标存储服务器Bus ID
 * @param cmd 命令类型
 * @param table_id 表ID
 * @param dbname 数据库名
 * @param table_name 表名
 * @param xData 数据
 * @param max_records 最大记录数
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param mod_key 模块键
 * @param cls_name 类名
 * @param packet_type 数据包类型
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
                                                  const std::string& dbname, const std::string& table_name, const std::string& xData, int max_records,
                                                  int trans_id, uint32_t seq,
                                                  uint64_t mod_key, const std::string& cls_name, uint8_t packet_type)
{
    CHECK_EXPR(cmd == NFrame::NF_STORESVR_C2S_EXECUTE_MORE, -1, "error cmd:{}", cmd);
    CHECK_EXPR(NFrame::PacketDispType_IsValid(packet_type), -1, "error msg_type:{}", packet_type);

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(cmd);
    svrPkg.mutable_disp_info()->set_type((NFrame::PacketDispType)packet_type);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    NFLogTrace(NF_LOG_DEFAULT, mod_key, "cmd:{} table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               cmd, table_id, table_name, trans_id, seq, mod_key);

    //NFLogTrace(NF_LOG_DEFAULT, 0, "message:{}", xData.DebugString());
    switch (cmd)
    {
        case NFrame::NF_STORESVR_C2S_EXECUTE_MORE:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_execute_more(dbname, table_name, mod_key, xData, max_records, cls_name));
        }
        break;
        default:
        {
        }
        break;
    }
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType,
                                                           NF_ST_STORE_SERVER,
                                                           0,
                                                           dstBusId,
                                                           NF_MODULE_FRAME,
                                                           NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD,
                                                           svrPkg);
}

/**
 * @brief 向存储服务器发送事务消息（字符串数据）
 * 
 * 通过消息模块向存储服务器发送事务消息，使用字符串格式的数据
 * 
 * @param eType 服务器类型
 * @param dstBusId 目标存储服务器Bus ID
 * @param cmd 命令类型
 * @param table_id 表ID
 * @param dbname 数据库名
 * @param table_name 表名
 * @param xData 字符串数据
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param mod_key 模块键
 * @param cls_name 类名
 * @param packet_type 数据包类型
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
                                                   const std::string& dbname, const std::string& table_name, const std::string& xData,
                                                   int trans_id, uint32_t seq,
                                                   uint64_t mod_key, const std::string& cls_name, uint8_t packet_type)
{
    CHECK_EXPR(cmd == NFrame::NF_STORESVR_C2S_EXECUTE, -1, "error cmd:{}", cmd);
    CHECK_EXPR(NFrame::PacketDispType_IsValid(packet_type), -1, "error msg_type:{}", packet_type);

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(cmd);
    svrPkg.mutable_disp_info()->set_type((NFrame::PacketDispType)packet_type);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    NFLogTrace(NF_LOG_DEFAULT, mod_key, "cmd:{} table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               cmd, table_id, table_name, trans_id, seq, mod_key);

    //NFLogTrace(NF_LOG_DEFAULT, 0, "message:{}", xData.DebugString());
    switch (cmd)
    {
        case NFrame::NF_STORESVR_C2S_EXECUTE:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_execute(dbname, table_name, mod_key, xData));
        }
        break;
        default:
        {
        }
        break;
    }
    return FindModule<NFIMessageModule>()->SendMsgToServer(eType,
                                                           NF_ST_STORE_SERVER,
                                                           0,
                                                           dstBusId,
                                                           NF_MODULE_FRAME,
                                                           NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD,
                                                           svrPkg);
}

int NFServerMessageModule::SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id, const std::string& dbname,
                                                  const std::string& table_name, const google::protobuf::Message& xData, int trans_id,
                                                  uint32_t seq,
                                                  uint64_t mod_key, const std::string& cls_name, uint8_t packet_type)
{
    CHECK_EXPR(!(cmd == NFrame::NF_STORESVR_C2S_SELECT || cmd == NFrame::NF_STORESVR_C2S_DELETE || cmd == NFrame::NF_STORESVR_C2S_MODIFY ||
                   cmd == NFrame::NF_STORESVR_C2S_UPDATE),
               -1,
               "error cmd:{}",
               cmd);
    CHECK_EXPR(NFrame::PacketDispType_IsValid(packet_type), -1, "error msg_type:{}", packet_type);

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(cmd);
    svrPkg.mutable_disp_info()->set_type((NFrame::PacketDispType)packet_type);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    NFLogTrace(NF_LOG_DEFAULT, mod_key, "cmd:{} table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               cmd, table_id, table_name, trans_id, seq, mod_key);

    //NFLogTrace(NF_LOG_DEFAULT, 0, "message:{}", xData.DebugString());
    switch (cmd)
    {
        case NFrame::NF_STORESVR_C2S_SELECT:
        {
            std::vector<NFrame::storesvr_vk> vk_list;
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_selectbycond(dbname, table_name, mod_key, std::vector<std::string>(), vk_list, "", 100, cls_name));
        }
        break;
        case NFrame::NF_STORESVR_C2S_SELECTOBJ:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_selectobj(dbname, table_name, mod_key, xData, cls_name));
        }
        break;
        case NFrame::NF_STORESVR_C2S_INSERTOBJ:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_insertobj(dbname, table_name, mod_key, xData, cls_name));
        }
        break;
        case NFrame::NF_STORESVR_C2S_DELETE:
        {
            std::vector<NFrame::storesvr_vk> vk_list;
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_deletebycond(dbname, table_name, mod_key, vk_list, cls_name));
        }
        break;
        case NFrame::NF_STORESVR_C2S_DELETEOBJ:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_deleteobj(dbname, table_name, mod_key, xData, cls_name));
        }
        break;
        case NFrame::NF_STORESVR_C2S_MODIFYOBJ:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_modifyobj(dbname, table_name, mod_key, xData, cls_name));
        }
        break;
        case NFrame::NF_STORESVR_C2S_UPDATEOBJ:
        {
            svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_updateobj(dbname, table_name, mod_key, xData, cls_name));
        }
        break;
        default:
        {
        }
        break;
    }

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}

/**
 * @brief 发送存储服务器查询事务
 * 
 * 通过消息模块向存储服务器发送查询事务消息
 * 
 * @param eType 服务器类型
 * @param mod_key 模块键
 * @param data 数据
 * @param table_id 表ID
 * @param trans_id 事务ID
 * @param vecFields 字段列表
 * @param vk_list 键值对列表
 * @param where_addtional_conds 附加条件
 * @param max_records 最大记录数
 * @param dstBusId 目标存储服务器Bus ID
 * @param dbname 数据库名
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendSelectTrans(NF_SERVER_TYPE eType, uint64_t mod_key, const google::protobuf::Message& data, uint32_t table_id, int trans_id,
                                           const std::vector<std::string>& vecFields, const std::vector<NFrame::storesvr_vk>& vk_list, const std::string& where_addtional_conds,
                                           int max_records, uint32_t dstBusId,
                                           const std::string& dbname)
{
    std::string tempDBName = dbname;
    if (dbname.empty())
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
        if (pConfig)
        {
            tempDBName = pConfig->DefaultDBName;
        }
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    if (dstBusId == 0)
    {
        auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
        if (pDbServer)
        {
            dstBusId = pDbServer->mServerInfo.bus_id();
        }
    }

    std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
    std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
    CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_SELECT);
    svrPkg.mutable_disp_info()->set_type(NFrame::E_DISP_TYPE_BY_TRANSACTION);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_selectbycond(tempDBName, tbname, mod_key, vecFields, vk_list, where_addtional_conds, max_records,
                                              tbname, packageName));

    NFLogTrace(NF_LOG_DEFAULT, 0,
               "cmd:NFrame::E_STORESVR_C2S_SELECT table_id:{} table_name:{} trans_id:{} mod_key:{}", table_id, tempDBName, trans_id, mod_key);

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId,
                                                           NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}

/**
 * @brief 发送存储服务器查询事务
 * 
 * 通过消息模块向存储服务器发送查询事务消息
 * 
 * @param eType 服务器类型
 * @param mod_key 模块键
 * @param data 数据
 * @param table_id 表ID
 * @param trans_id 事务ID
 * @param vecFields 字段列表
 * @param privateKeys 私有键列表
 * @param max_records 最大记录数
 * @param dstBusId 目标存储服务器Bus ID
 * @param dbname 数据库名
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendSelectTrans(NF_SERVER_TYPE eType, uint64_t mod_key, const google::protobuf::Message& data, uint32_t table_id, int trans_id,
                                           const std::vector<std::string>& vecFields, const std::vector<std::string>& privateKeys, int max_records, uint32_t dstBusId,
                                           const std::string& dbname)
{
    std::string tempDBName = dbname;
    if (dbname.empty())
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
        if (pConfig)
        {
            tempDBName = pConfig->DefaultDBName;
        }
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    if (dstBusId == 0)
    {
        auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
        if (pDbServer)
        {
            dstBusId = pDbServer->mServerInfo.bus_id();
        }
    }

    std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
    std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
    CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_SELECT);
    svrPkg.mutable_disp_info()->set_type(NFrame::E_DISP_TYPE_BY_TRANSACTION);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_selectbycond(tempDBName, tbname, mod_key, vecFields, privateKeys, max_records,
                                              tbname, packageName));

    NFLogTrace(NF_LOG_DEFAULT, 0,
               "cmd:NFrame::E_STORESVR_C2S_SELECT table_id:{} table_name:{} trans_id:{} mod_key:{}", table_id, tempDBName, trans_id, mod_key);

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId,
                                                           NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}


/**
 * @brief 发送存储服务器查询对象事务
 * 
 * 通过消息模块向存储服务器发送查询对象事务消息
 * 
 * @param eType 服务器类型
 * @param mod_key 模块键
 * @param data 数据
 * @param table_id 表ID
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param vecFields 字段列表
 * @param dstBusId 目标存储服务器Bus ID
 * @param dbname 数据库名
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendSelectObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id/* = 0*/, int trans_id/* = 0*/, uint32_t seq/* = 0*/,
                                              const std::vector<std::string>& vecFields/* = std::vector<std::string>()*/, uint32_t dstBusId/* = 0*/,
                                              const std::string& dbname/* = ""*/)
{
    std::string tempDBName = dbname;
    if (dbname.empty())
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
        if (pConfig)
        {
            tempDBName = pConfig->DefaultDBName;
        }
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
    std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
    CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_SELECTOBJ);
    svrPkg.mutable_disp_info()->set_type(NFrame::E_DISP_TYPE_BY_TRANSACTION);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_selectobj(tempDBName, tbname, mod_key, data, tbname, packageName, vecFields));

    NFLogTrace(NF_LOG_DEFAULT, 0,
               "cmd:NFrame::E_STORESVR_C2S_SELECTOBJ table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               table_id, tempDBName, trans_id, seq, mod_key);

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId,
                                                           NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}

/**
 * @brief 发送存储服务器插入对象事务
 * 
 * 通过消息模块向存储服务器发送插入对象事务消息
 * 
 * @param eType 服务器类型
 * @param mod_key 模块键
 * @param data 数据
 * @param table_id 表ID
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param dstBusId 目标存储服务器Bus ID
 * @param dbname 数据库名
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendInsertObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id/* = 0*/, int trans_id/* = 0*/, uint32_t seq/* = 0*/, uint32_t dstBusId/* = 0*/, const std::string& dbname/* = ""*/)
{
    std::string tempDBName = dbname;
    if (dbname.empty())
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
        if (pConfig)
        {
            tempDBName = pConfig->DefaultDBName;
        }
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
    std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
    CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_INSERTOBJ);
    svrPkg.mutable_disp_info()->set_type(NFrame::E_DISP_TYPE_BY_TRANSACTION);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_insertobj(tempDBName, tbname, mod_key, data, tbname, packageName));

    NFLogTrace(NF_LOG_DEFAULT, 0,
               "cmd:NFrame::NF_STORESVR_C2S_INSERTOBJ table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               table_id, tempDBName, trans_id, seq, mod_key);

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}

/**
 * @brief 发送存储服务器修改对象事务
 * 
 * 通过消息模块向存储服务器发送修改对象事务消息
 * 
 * @param eType 服务器类型
 * @param mod_key 模块键
 * @param data 数据
 * @param table_id 表ID
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param dstBusId 目标存储服务器Bus ID
 * @param dbname 数据库名
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendModifyObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id, int trans_id, uint32_t seq, uint32_t dstBusId, const std::string& dbname)
{
    std::string tempDBName = dbname;
    if (dbname.empty())
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
        if (pConfig)
        {
            tempDBName = pConfig->DefaultDBName;
        }
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
    std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
    CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_MODIFYOBJ);
    svrPkg.mutable_disp_info()->set_type(NFrame::E_DISP_TYPE_BY_TRANSACTION);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_modifyobj(tempDBName, tbname, mod_key, data, tbname, packageName));

    NFLogTrace(NF_LOG_DEFAULT, 0,
               "cmd:NFrame::NF_STORESVR_C2S_MODIFYOBJ table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               table_id, tempDBName, trans_id, seq, mod_key);

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}

/**
 * @brief 发送存储服务器删除对象事务
 * 
 * 通过消息模块向存储服务器发送删除对象事务消息
 * 
 * @param eType 服务器类型
 * @param mod_key 模块键
 * @param data 数据
 * @param table_id 表ID
 * @param trans_id 事务ID
 * @param seq 序列号
 * @param dstBusId 目标存储服务器Bus ID
 * @param dbname 数据库名
 * @return 操作结果，0表示成功
 */
int NFServerMessageModule::SendDeleteObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id, int trans_id, uint32_t seq, uint32_t dstBusId, const std::string& dbname)
{
    std::string tempDBName = dbname;
    if (dbname.empty())
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
        if (pConfig)
        {
            tempDBName = pConfig->DefaultDBName;
        }
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
    std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
    CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

    NFrame::Proto_FramePkg svrPkg;
    svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_DELETEOBJ);
    svrPkg.mutable_disp_info()->set_type(NFrame::E_DISP_TYPE_BY_TRANSACTION);
    svrPkg.mutable_disp_info()->set_seq(seq);
    svrPkg.mutable_store_info()->set_table_id(table_id);
    svrPkg.mutable_store_info()->set_id(trans_id);

    svrPkg.set_msg_data(NFStoreProtoCommon::storesvr_deleteobj(tempDBName, tbname, mod_key, data, tbname, packageName));

    NFLogTrace(NF_LOG_DEFAULT, 0,
               "cmd:NFrame::NF_STORESVR_C2S_DELETEOBJ table_id:{} table_name:{} trans_id:{} seq:{} mod_key:{}",
               table_id, tempDBName, trans_id, seq, mod_key);

    return FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, 0, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD, svrPkg);
}