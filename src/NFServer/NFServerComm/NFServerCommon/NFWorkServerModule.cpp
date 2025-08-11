// -------------------------------------------------------------------------
//    @FileName         :    NFWorkServerModule.cpp
//    @Author           :    gaoyi
//    @Date             :    22-10-26
//    @Email			:    445267987@qq.com
//    @Module           :    NFWorkServerModule
//    @Desc             :    工作服务器模块实现文件，提供业务服务器的通用功能实现。
//                          该文件实现了工作服务器模块类，包括服务器连接管理、
//                          消息处理实现、服务器注册功能、网络事件处理。
//                          主要功能包括提供业务服务器的通用功能实现、支持多种服务器类型、
//                          支持服务器间通信、支持Lua脚本加载、提供网络事件处理。
//                          工作服务器模块是NFShmXFrame框架的核心业务组件实现，负责：
//                          - 业务服务器的通用功能实现
//                          - 服务器连接管理和状态维护
//                          - 消息处理和路由分发
//                          - 服务器注册和发现
//                          - Lua脚本加载和管理
//                          - 网络事件处理和回调
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#include "NFWorkServerModule.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFServerCommonDefine.h"
#include "NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"

#define SERVER_CONNECT_MASTER_SERVER "Server Connect MasterServer"
#define SERVER_CONNECT_ROUTEAGENT_SERVER "Server Connect RouteAgentServer"
#define SERVER_CONNECT_PROXYSERVER_SERVER "Server Connect ProxyAgentServer"
#define SERVER_CHECK_STORE_SERVER "Server Check Store Server"
#define SERVER_CHECK_WORLD_SERVER "Server Check World Server"
#define SERVER_CHECK_CENTER_SERVER "Server Check Center Server"

#define SERVER_REPORT_TO_MASTER_SERVER_TIMER_ID 101
#define SERVER_SERVER_DEAD_TIMER_ID 102

/**
 * @brief 检查是否连接主服务器
 * 
 * 返回当前是否连接主服务器的状态
 * 
 * @return 是否连接主服务器
 */
bool NFWorkServerModule::IsConnectMasterServer() const
{
    return m_connectMasterServer;
}

/**
 * @brief 设置是否连接主服务器
 * 
 * 设置是否连接主服务器，如果设置为连接且当前不是主服务器，
 * 则注册连接主服务器的任务
 * 
 * @param connectMasterServer 是否连接主服务器
 */
void NFWorkServerModule::SetConnectMasterServer(bool connectMasterServer)
{
    m_connectMasterServer = connectMasterServer;
    if (connectMasterServer && m_serverType != NF_ST_MASTER_SERVER && !IsHasAppTask(m_serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT, APP_INIT_CONNECT_MASTER))
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
        CHECK_EXPR_ASSERT(pConfig, , "GetAppConfig Failed, server type:{}", m_serverType);

        RegisterAppTask(m_serverType, APP_INIT_CONNECT_MASTER,
                        NF_FORMAT("{} {}", pConfig->ServerName, SERVER_CONNECT_MASTER_SERVER), APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }
}

/**
 * @brief 检查是否连接路由代理服务器
 * 
 * 返回当前是否连接路由代理服务器的状态
 * 
 * @return 是否连接路由代理服务器
 */
bool NFWorkServerModule::IsConnectRouteAgentServer() const
{
    return m_connectRouteAgentServer;
}

/**
 * @brief 设置是否连接路由代理服务器
 * 
 * 设置是否连接路由代理服务器，如果设置为连接且当前不是路由代理服务器，
 * 则注册连接路由代理服务器的任务
 * 
 * @param connectRouteAgentServer 是否连接路由代理服务器
 */
void NFWorkServerModule::SetConnectRouteAgentServer(bool connectRouteAgentServer)
{
    m_connectRouteAgentServer = connectRouteAgentServer;
    if (connectRouteAgentServer && m_serverType != NF_ST_ROUTE_AGENT_SERVER && !IsHasAppTask(m_serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT, APP_INIT_CONNECT_ROUTE_AGENT_SERVER))
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
        CHECK_EXPR_ASSERT(pConfig, , "GetAppConfig Failed, server type:{}", m_serverType);

        RegisterAppTask(m_serverType, APP_INIT_CONNECT_ROUTE_AGENT_SERVER,
                        NF_FORMAT("{} {}", pConfig->ServerName, SERVER_CONNECT_ROUTEAGENT_SERVER), APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }
}

/**
 * @brief 检查是否连接代理代理服务器
 * 
 * 返回当前是否连接代理代理服务器的状态
 * 
 * @return 是否连接代理代理服务器
 */
bool NFWorkServerModule::IsConnectProxyAgentServer() const
{
    return m_connectProxyAgentServer;
}

/**
 * @brief 设置是否连接代理代理服务器
 * 
 * 设置是否连接代理代理服务器，目前此功能被注释掉
 * 
 * @param connectProxyAgentServer 是否连接代理代理服务器
 */
void NFWorkServerModule::SetConnectProxyAgentServer(bool connectProxyAgentServer)
{
    m_connectProxyAgentServer = connectProxyAgentServer;
    /*    if (connectProxyAgentServer && m_serverType != NF_ST_PROXY_AGENT_SERVER && !IsHasAppTask(m_serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT, APP_INIT_CONNECT_PROXY_AGENT_SERVER))
        {
            NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
            CHECK_EXPR_ASSERT(pConfig, , "GetAppConfig Failed, server type:{}", m_serverType);

            RegisterAppTask(m_serverType, APP_INIT_CONNECT_PROXY_AGENT_SERVER,
                            NF_FORMAT("{} {}", pConfig->ServerName, SERVER_CONNECT_PROXYSERVER_SERVER), APP_INIT_TASK_GROUP_SERVER_CONNECT);
        }*/
}

/**
 * @brief 检查是否检查存储服务器
 * 
 * 返回当前是否检查存储服务器的状态
 * 
 * @return 是否检查存储服务器
 */
bool NFWorkServerModule::IsCheckStoreServer() const
{
    return m_checkStoreServer;
}

/**
 * @brief 设置是否检查存储服务器
 * 
 * 设置是否检查存储服务器，如果设置为检查且当前不是存储服务器，
 * 则注册检查存储服务器的任务
 * 
 * @param checkStoreServer 是否检查存储服务器
 */
void NFWorkServerModule::SetCheckStoreServer(bool checkStoreServer)
{
    m_checkStoreServer = checkStoreServer;
    if (checkStoreServer && m_serverType != NF_ST_STORE_SERVER && !IsHasAppTask(m_serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT, APP_INIT_NEED_STORE_SERVER))
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
        CHECK_EXPR_ASSERT(pConfig, , "GetAppConfig Failed, server type:{}", m_serverType);

        RegisterAppTask(m_serverType, APP_INIT_NEED_STORE_SERVER,
                        NF_FORMAT("{} {}", pConfig->ServerName, SERVER_CHECK_STORE_SERVER), APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }
}

/**
 * @brief 检查是否检查世界服务器
 * 
 * 返回当前是否检查世界服务器的状态
 * 
 * @return 是否检查世界服务器
 */
bool NFWorkServerModule::IsCheckWorldServer() const
{
    return m_checkWorldServer;
}

/**
 * @brief 设置是否检查世界服务器
 * 
 * 设置是否检查世界服务器，如果设置为检查且当前不是世界服务器，
 * 则注册检查世界服务器的任务
 * 
 * @param checkWorldServer 是否检查世界服务器
 */
void NFWorkServerModule::SetCheckWorldServer(bool checkWorldServer)
{
    m_checkWorldServer = checkWorldServer;
    if (checkWorldServer && m_serverType != NF_ST_WORLD_SERVER && !IsHasAppTask(m_serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT, APP_INIT_NEED_WORLD_SERVER))
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
        CHECK_EXPR_ASSERT(pConfig, , "GetAppConfig Failed, server type:{}", m_serverType);

        RegisterAppTask(m_serverType, APP_INIT_NEED_WORLD_SERVER,
                        NF_FORMAT("{} {}", pConfig->ServerName, SERVER_CHECK_WORLD_SERVER), APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }
}

/**
 * @brief 检查是否检查中心服务器
 * 
 * 返回当前是否检查中心服务器的状态
 * 
 * @return 是否检查中心服务器
 */
bool NFWorkServerModule::IsCheckCenterServer() const
{
    return m_checkCenterServer;
}

/**
 * @brief 设置是否检查中心服务器
 * 
 * 设置是否检查中心服务器，如果设置为检查且当前不是中心服务器，
 * 则注册检查中心服务器的任务
 * 
 * @param checkCenterServer 是否检查中心服务器
 */
void NFWorkServerModule::SetCheckCenterServer(bool checkCenterServer)
{
    m_checkCenterServer = checkCenterServer;
    if (checkCenterServer && m_serverType != NF_ST_CENTER_SERVER && !IsHasAppTask(m_serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT, APP_INIT_NEED_CENTER_SERVER))
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
        CHECK_EXPR_ASSERT(pConfig, , "GetAppConfig Failed, server type:{}", m_serverType);

        RegisterAppTask(m_serverType, APP_INIT_NEED_CENTER_SERVER,
                        NF_FORMAT("{} {}", pConfig->ServerName, SERVER_CHECK_CENTER_SERVER), APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }
}

/**
 * @brief 获取服务器类型
 * 
 * 返回当前工作服务器模块的服务器类型
 * 
 * @return 服务器类型
 */
NF_SERVER_TYPE NFWorkServerModule::GetServerType() const
{
    return m_serverType;
}

/**
 * @brief 设置服务器类型
 * 
 * 设置工作服务器模块的服务器类型
 * 
 * @param serverType 服务器类型
 */
void NFWorkServerModule::SetServerType(NF_SERVER_TYPE serverType)
{
    m_serverType = serverType;
}

/**
 * @brief 绑定服务器
 * 
 * 绑定工作服务器到指定服务器类型，并注册消息回调、事件订阅、定时器等。
 * 
 * @return 绑定结果
 */
int NFWorkServerModule::BindServer()
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, -1, "GetAppConfig Failed, server type:{}", m_serverType);

    //////////////////////master msg//////////////////////////
    FindModule<NFIMessageModule>()->AddMessageCallBack(m_serverType, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, this,
                                                       &NFWorkServerModule::OnHandleServerReportFromMasterServer);

    /////////////////route agent msg///////////////////////////////////////
    FindModule<NFIMessageModule>()->AddMessageCallBack(m_serverType, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER_RSP, this,
                                                       &NFWorkServerModule::OnRegisterRouteAgentServerRspProcess);

    //////////////////////proxy server msg//////////////////////////
    FindModule<NFIMessageModule>()->AddMessageCallBack(m_serverType, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, this,
                                                       &NFWorkServerModule::OnServerRegisterProcessFromProxyServer);

    if (!m_pObjPluginManager->IsLoadAllServer())
    {
        CHECK_EXPR_ASSERT(pConfig->ServerType == m_serverType, -1, "server config error, server id not match the server type!:{}", m_serverType);
    }

    uint64_t serverLinkId = FindModule<NFIMessageModule>()->BindServer(m_serverType, pConfig->Url, pConfig->NetThreadNum, pConfig->MaxConnectNum,
                                                                       PACKET_PARSE_TYPE_INTERNAL);
    CHECK_EXPR_ASSERT(serverLinkId > 0, -1, "Server:{} Listen Failed, ServerId:{}, Ip:{}, Port:{}", pConfig->ServerName, pConfig->ServerId,
                      pConfig->ServerIp, pConfig->ServerPort);

    FindModule<NFIMessageModule>()->SetServerLinkId(m_serverType, serverLinkId);
    FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, serverLinkId, this, &NFWorkServerModule::OnServerSocketEvent);
    FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, serverLinkId, this,
                                                     &NFWorkServerModule::OnHandleServerOtherMessage);
    NFLogInfo(NF_LOG_DEFAULT, 0, "Server:{} Listen Success, ServerId:{}, Ip:{}, Port:{}", pConfig->ServerName, pConfig->ServerId, pConfig->ServerIp,
              pConfig->ServerPort);

    Subscribe(m_serverType, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);
    Subscribe(m_serverType, NFrame::NF_EVENT_SERVER_APP_FINISH_INITED, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);

    SetTimer(SERVER_REPORT_TO_MASTER_SERVER_TIMER_ID, 10000);
    return 0;
}

/**
 * @brief 初始化加载Lua脚本
 * 
 * 工作服务器模块初始化时加载Lua脚本，目前为空实现。
 * 
 * @return 初始化结果
 */
int NFWorkServerModule::InitLoadLua()
{
    return 0;
}

/**
 * @brief 执行定时器回调
 * 
 * 处理定时器回调，包括向主服务器报告状态、处理服务器死亡定时器。
 * 
 * @param nTimerID 定时器ID
 * @return 处理结果
 */
int NFWorkServerModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage)
{
    CHECK_EXPR(serverType == m_serverType, -1, "");
    if (bySrcType == NFrame::NF_EVENT_SERVER_TYPE)
    {
        if (nEventID == NFrame::NF_EVENT_SERVER_DEAD_EVENT)
        {
            SetTimer(SERVER_SERVER_DEAD_TIMER_ID, 10000, 0);
        }
        else if (nEventID == NFrame::NF_EVENT_SERVER_APP_FINISH_INITED)
        {
            RegisterMasterServer(NFrame::EST_NARMAL);
        }
    }

    return 0;
}

/**
 * @brief 执行定时器回调
 * 
 * 处理定时器回调，包括向主服务器报告状态、处理服务器死亡定时器。
 * 
 * @param nTimerID 定时器ID
 * @return 处理结果
 */
int NFWorkServerModule::OnTimer(uint32_t nTimerID)
{
    if (nTimerID == SERVER_REPORT_TO_MASTER_SERVER_TIMER_ID)
    {
        ServerReportToMasterServer();
    }
    else if (nTimerID == SERVER_SERVER_DEAD_TIMER_ID)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "kill the exe..................");
        NFSLEEP(1000);
        exit(0);
    }
    return 0;
}

/**
 * @brief 连接主服务器
 * 
 * 如果当前未连接主服务器，则尝试连接主服务器。
 * 
 * @return 连接结果
 */
int NFWorkServerModule::ConnectMasterServer()
{
    if (m_connectMasterServer == false)
    {
        return 0;
    }

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, -1, "GetAppConfig Failed, server type:{}", m_serverType);

#if NF_PLATFORM == NF_PLATFORM_WIN
    NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(m_serverType);
    int32_t ret = ConnectMasterServer(masterData);
    CHECK_EXPR(ret == 0, false, "ConnectMasterServer Failed, url:{}", masterData.DebugString());
#else
    if (pConfig->RouteConfig.NamingHost.empty())
    {
        NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(m_serverType);
        int32_t ret = ConnectMasterServer(masterData);
        CHECK_EXPR(ret == 0, -1, "ConnectMasterServer Failed, url:{}", masterData.DebugString());
    }
#endif

    return 0;
}

/**
 * @brief 处理服务器Socket事件
 * 
 * 处理服务器Socket的连接、断开等事件。
 * 
 * @param nEvent 事件类型
 * @param unLinkId 链接ID
 * @return 处理结果
 */
int NFWorkServerModule::OnServerSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    if (nEvent == eMsgType_CONNECTED)
    {
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        OnHandleServerDisconnect(unLinkId);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理服务器其他消息
 * 
 * 处理未知的或不直接处理的消息。
 * 
 * @param unLinkId 链接ID
 * @param packet 消息包
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleServerOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogWarning(NF_LOG_DEFAULT, 0, "msg:{} not handled!", packet.ToString());
    return 0;
}

/**
 * @brief 处理服务器断开连接
 * 
 * 当服务器断开连接时，更新服务器状态并清理链接。
 * 
 * @param unLinkId 链接ID
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleServerDisconnect(uint64_t unLinkId)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", m_serverType);

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(m_serverType, unLinkId);
    if (pServerData)
    {
        pServerData->mServerInfo.set_server_state(NFrame::EST_CRASH);
        pServerData->mUnlinkId = 0;

        NFLogError(NF_LOG_DEFAULT, 0, "the server:{} disconnect from server:{}, serverName:{}, busid:{}, serverIp:{}, serverPort:{}",
                   pServerData->mServerInfo.server_name(), pConfig->ServerName, pServerData->mServerInfo.server_name(),
                   pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_ip(), pServerData->mServerInfo.server_port());
    }

    FindModule<NFIMessageModule>()->DelServerLink(m_serverType, unLinkId);
    return 0;
}

/**
 * @brief 连接主服务器
 * 
 * 尝试连接主服务器，并注册相关回调。
 * 
 * @param xData 服务器信息
 * @return 连接结果
 */
int NFWorkServerModule::ConnectMasterServer(const NFrame::ServerInfoReport& xData)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", m_serverType);
    auto pMsterServerData = FindModule<NFIMessageModule>()->GetMasterData(m_serverType);
    if (pMsterServerData->mUnlinkId <= 0)
    {
        pMsterServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(m_serverType, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
        FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, pMsterServerData->mUnlinkId, this, &NFWorkServerModule::OnMasterSocketEvent);
        FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, pMsterServerData->mUnlinkId, this,
                                                         &NFWorkServerModule::OnHandleMasterOtherMessage);
    }

    pMsterServerData->mServerInfo = xData;

    return 0;
}

/**
 * @brief 注册主服务器
 * 
 * 如果当前连接主服务器，则向主服务器发送注册请求。
 * 
 * @param serverState 服务器状态
 * @return 注册结果
 */
int NFWorkServerModule::RegisterMasterServer(uint32_t serverState)
{
    if (m_connectMasterServer == false)
    {
        return 0;
    }

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);

        pData->set_server_state(serverState);
        FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER>(m_serverType, NF_ST_MASTER_SERVER, 0, xMsg, [this](int rpcRetCode, NFrame::ServerInfoReportListRespne& respone)
        {
            if (rpcRetCode != 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "Register Master Failed......, kill the exe");
                NFrame::NFEventNoneData event;
                FireExecute(m_serverType, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, event);
                return;
            }

            //完成服务器启动任务
            if (!m_pObjPluginManager->IsInited(m_serverType))
            {
                FinishAppTask(m_serverType, APP_INIT_CONNECT_MASTER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
            }
        });
    }
    return 0;
}

/**
 * @brief 处理Master服务器链接事件
 * 
 * 当Master服务器连接或断开时，更新相关状态并注册回调。
 * 
 * @param nEvent 事件类型
 * @param unLinkId 链接ID
 * @return 处理结果
 */
int NFWorkServerModule::OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", m_serverType);

    if (nEvent == eMsgType_CONNECTED)
    {
        std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
        NFLogDebug(NF_LOG_DEFAULT, 0, "server:{} connect master success!", pConfig->ServerName);

        if (!m_pObjPluginManager->IsInited(m_serverType))
        {
            RegisterMasterServer(NFrame::EST_INIT);
        }
        else
        {
            RegisterMasterServer(NFrame::EST_NARMAL);
        }
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "server:{} disconnect master success", pConfig->ServerName);
    }
    return 0;
}

/**
 * @brief 处理Master服务器未注册协议
 * 
 * 处理Master服务器发送的未注册协议消息。
 * 
 * @param unLinkId 链接ID
 * @param packet 消息包
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleMasterOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogWarning(NF_LOG_DEFAULT, 0, "master server other message not handled:msgId:{}", packet.ToString());
    return 0;
}

/**
 * @brief 处理从Master服务器接收到的服务器报告
 * 
 * 解析Master服务器发送的服务器报告，并根据服务器类型调用相应的处理函数。
 * 
 * @param unLinkId 链接ID
 * @param packet 消息包
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleServerReportFromMasterServer(uint64_t unLinkId, NFDataPackage& packet)
{
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_NO_PRINTF(packet, xMsg);

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport& xData = xMsg.server_list(i);
        switch (xData.server_type())
        {
            case NF_SERVER_TYPE::NF_ST_ROUTE_AGENT_SERVER:
            {
                OnHandleRouteAgentServerReport(xData);
            }
            break;
            case NF_SERVER_TYPE::NF_ST_STORE_SERVER:
            {
                OnHandleStoreServerReport(xData);
            }
            break;
            case NF_SERVER_TYPE::NF_ST_WORLD_SERVER:
            {
                OnHandleWorldServerReport(xData);
                break;
            }
            case NF_SERVER_TYPE::NF_ST_CENTER_SERVER:
            {
                OnHandleCenterServerReport(xData);
                break;
            }
            case NF_SERVER_TYPE::NF_ST_PROXY_AGENT_SERVER:
            {
                OnHandleProxyAgentServerReport(xData);
            }
            break;
            default:
            {
                OnHandleOtherServerReportFromMasterServer(xData);
            }
            break;
        }
    }
    return 0;
}

/**
 * @brief 处理其他类型的服务器报告
 * 
 * 处理从Master服务器接收到的未知服务器类型的报告。
 * 
 * @param xData 服务器信息
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleOtherServerReportFromMasterServer(const NFrame::ServerInfoReport& xData)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_NULL(0, pConfig);

    FindModule<NFIMessageModule>()->CreateServerByServerId(m_serverType, xData.bus_id(), (NF_SERVER_TYPE)xData.server_type(), xData);
    return 0;
}

/**
 * @brief 向Master服务器报告状态
 * 
 * 将当前工作服务器的状态报告给Master服务器。
 * 
 * @return 报告结果
 */
int NFWorkServerModule::ServerReportToMasterServer()
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);
        pData->set_server_state(NFrame::EST_NARMAL);

        NFIMonitorModule* pMonitorModule = m_pObjPluginManager->FindModule<NFIMonitorModule>();
        if (pMonitorModule)
        {
            const NFSystemInfo& systemInfo = pMonitorModule->GetSystemInfo();
            NFServerCommon::WriteServerInfo(pData, systemInfo);
        }

        if (pData->proc_cpu() > 0 && pData->proc_mem() > 0)
        {
            FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(m_serverType, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_MASTER_SERVER_REPORT, xMsg);
        }
    }
    return 0;
}

/**
 * @brief 处理代理代理服务器报告
 * 
 * 当代理代理服务器报告其状态时，更新其链接并注册回调。
 * 
 * @param xData 服务器信息
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleProxyAgentServerReport(const NFrame::ServerInfoReport& xData)
{
    if (m_connectProxyAgentServer == false)
    {
        return 0;
    }

    CHECK_EXPR(xData.server_type() == NF_ST_PROXY_AGENT_SERVER, -1, "xData.server_type() == NF_ST_PROXY_AGENT_SERVER");
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_NULL(0, pConfig);

    if (pConfig->RouteConfig.RouteAgent != xData.route_svr())
    {
        return 0;
    }

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(m_serverType, xData.bus_id());
    if (pServerData == nullptr)
    {
        pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(m_serverType, xData.bus_id(), NF_ST_PROXY_AGENT_SERVER, xData);

        pServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(m_serverType, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
        FindModule<NFIMessageModule>()->CreateLinkToServer(m_serverType, xData.bus_id(), pServerData->mUnlinkId);

        FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, pServerData->mUnlinkId, this,
                                                         &NFWorkServerModule::OnProxyAgentServerSocketEvent);
        FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, pServerData->mUnlinkId, this,
                                                         &NFWorkServerModule::OnHandleProxyAgentServerOtherMessage);
    }
    else
    {
        if (pServerData->mUnlinkId > 0 && pServerData->mServerInfo.url() != xData.url())
        {
            NFLogWarning(NF_LOG_DEFAULT, 0, "the server:{} old url:{} changed, new url:{}", pServerData->mServerInfo.server_name(),
                         pServerData->mServerInfo.url(), xData.url());
            FindModule<NFIMessageModule>()->CloseLinkId(pServerData->mUnlinkId);

            pServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(m_serverType, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
            FindModule<NFIMessageModule>()->CreateLinkToServer(m_serverType, xData.bus_id(), pServerData->mUnlinkId);

            FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, pServerData->mUnlinkId, this,
                                                             &NFWorkServerModule::OnProxyAgentServerSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, pServerData->mUnlinkId, this,
                                                             &NFWorkServerModule::OnHandleProxyAgentServerOtherMessage);
        }
    }

    pServerData->mServerInfo = xData;

    return 0;
}

/**
 * @brief 处理代理代理服务器Socket事件
 * 
 * 当代理代理服务器连接或断开时，更新相关状态并注册回调。
 * 
 * @param nEvent 事件类型
 * @param unLinkId 链接ID
 * @return 处理结果
 */
int NFWorkServerModule::OnProxyAgentServerSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", m_serverType);

    if (nEvent == eMsgType_CONNECTED)
    {
        NFLogDebug(NF_LOG_DEFAULT, 0, "server:{} connect proxy agent server success!", pConfig->ServerName);
        RegisterProxyAgentServer(unLinkId);
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "server:{} disconnect proxy agent server", pConfig->ServerName);
    }
    return 0;
}

/**
 * @brief 处理代理代理服务器其他消息
 * 
 * 处理代理代理服务器发送的未注册协议消息。
 * 
 * @param unLinkId 链接ID
 * @param packet 消息包
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleProxyAgentServerOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogWarning(NF_LOG_DEFAULT, 0, "msg:{} not handled!", packet.ToString());
    return 0;
}

/**
 * @brief 注册代理代理服务器
 * 
 * 向代理代理服务器发送注册请求。
 * 
 * @param unLinkId 链接ID
 * @return 注册结果
 */
int NFWorkServerModule::RegisterProxyAgentServer(uint64_t unLinkId)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);

        std::set<uint32_t> allMsg = FindModule<NFIMessageModule>()->GetAllMsg(m_serverType, NF_MODULE_CLIENT);
        for (auto iter = allMsg.begin(); iter != allMsg.end(); ++iter)
        {
            pData->add_msg_id(*iter);
        }
        pData->set_server_state(NFrame::EST_NARMAL);

        FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg, 0);
    }

    return 0;
}

/**
 * @brief 处理代理服务器注册请求
 * 
 * 当代理服务器发送注册请求时，更新其链接并注册回调。
 * 
 * @param unLinkId 链接ID
 * @param packet 消息包
 * @return 处理结果
 */
int NFWorkServerModule::OnServerRegisterProcessFromProxyServer(uint64_t unLinkId, NFDataPackage& packet)
{
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport& xData = xMsg.server_list(i);
        switch (xData.server_type())
        {
            case NF_SERVER_TYPE::NF_ST_PROXY_SERVER:
            {
                OnHandleProxyServerRegister(xData, unLinkId);
            }
            break;
            default: break;
        }
    }
    return 0;
}

/**
 * @brief 处理代理服务器注册
 * 
 * 当代理服务器注册成功时，更新其状态并完成启动任务。
 * 
 * @param xData 服务器信息
 * @param unlinkId 链接ID
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleProxyServerRegister(const NFrame::ServerInfoReport& xData, uint64_t unlinkId)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", m_serverType);

    CHECK_EXPR(xData.server_type() == NF_ST_PROXY_SERVER, -1, "xData.server_type() == NF_ST_PROXY_SERVER");

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(m_serverType, xData.bus_id());
    if (!pServerData)
    {
        pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(m_serverType, xData.bus_id(), NF_ST_PROXY_SERVER, xData);
    }

    pServerData->mUnlinkId = unlinkId;
    pServerData->mServerInfo = xData;

    NFLogInfo(NF_LOG_DEFAULT, 0, "Proxy Server Register Server:{} Success, serverName:{}, busid:{}, ip:{}, port:{}", pConfig->ServerName,
              pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.external_server_ip(),
              pServerData->mServerInfo.external_server_port());

    //完成服务器启动任务
    /*    if (!m_pObjPluginManager->IsInited(m_serverType))
        {
            FinishAppTask(m_serverType, APP_INIT_CONNECT_PROXY_AGENT_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
        }*/

    return 0;
}

/**
 * @brief 处理存储服务器报告
 * 
 * 当存储服务器报告其状态时，更新其链接并注册回调。
 * 
 * @param xData 服务器信息
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleStoreServerReport(const NFrame::ServerInfoReport& xData)
{
    if (m_checkStoreServer == false)
    {
        return 0;
    }

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_NULL(0, pConfig);

    FindModule<NFIMessageModule>()->CreateServerByServerId(m_serverType, xData.bus_id(), NF_ST_STORE_SERVER, xData);

    bool has = false;
    for (int i = 0; i < (int)xData.db_name_list_size(); i++)
    {
        if (pConfig->DefaultDBName == xData.db_name_list(i))
        {
            has = true;
            break;
        }
    }

    if (has)
    {
        if (m_serverType != NF_ST_STORE_SERVER)
        {
            FinishAppTask(m_serverType, APP_INIT_NEED_STORE_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
        }
    }

    return 0;
}

/**
 * @brief 处理世界服务器报告
 * 
 * 当世界服务器报告其状态时，更新其链接并注册回调。
 * 
 * @param xData 服务器信息
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleWorldServerReport(const NFrame::ServerInfoReport& xData)
{
    if (m_checkWorldServer == false)
    {
        return 0;
    }

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_NULL(0, pConfig);

    FindModule<NFIMessageModule>()->CreateServerByServerId(m_serverType, xData.bus_id(), NF_ST_WORLD_SERVER, xData);

    if (m_serverType != NF_ST_WORLD_SERVER)
    {
        FinishAppTask(m_serverType, APP_INIT_NEED_WORLD_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }

    return 0;
}

/**
 * @brief 处理中心服务器报告
 * 
 * 当中心服务器报告其状态时，更新其链接并注册回调。
 * 
 * @param xData 服务器信息
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleCenterServerReport(const NFrame::ServerInfoReport& xData)
{
    if (m_checkCenterServer == false)
    {
        return 0;
    }

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_NULL(0, pConfig);

    FindModule<NFIMessageModule>()->CreateServerByServerId(m_serverType, xData.bus_id(), NF_ST_CENTER_SERVER, xData);

    if (m_serverType != NF_ST_CENTER_SERVER)
    {
        FinishAppTask(m_serverType, APP_INIT_NEED_CENTER_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }

    return 0;
}

/**
 * @brief 处理路由代理服务器报告
 * 
 * 当路由代理服务器报告其状态时，更新其链接并注册回调。
 * 
 * @param xData 服务器信息
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleRouteAgentServerReport(const NFrame::ServerInfoReport& xData)
{
    if (m_connectRouteAgentServer == false)
    {
        return 0;
    }

    CHECK_EXPR(xData.server_type() == NF_ST_ROUTE_AGENT_SERVER, -1, "xData.server_type() == NF_ST_ROUTE_AGENT_SERVER");

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_NULL(0, pConfig);

    if (pConfig->RouteConfig.RouteAgent != xData.server_id())
    {
        return 0;
    }

    auto pRouteAgentServerData = FindModule<NFIMessageModule>()->GetRouteData(m_serverType);

    if (pRouteAgentServerData->mUnlinkId == 0)
    {
        pRouteAgentServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(m_serverType, xData.url(), PACKET_PARSE_TYPE_INTERNAL);

        FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, pRouteAgentServerData->mUnlinkId, this,
                                                         &NFWorkServerModule::OnRouteAgentServerSocketEvent);
        FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, pRouteAgentServerData->mUnlinkId, this,
                                                         &NFWorkServerModule::OnHandleRouteAgentServerOtherMessage);
    }
    else
    {
        if (pRouteAgentServerData->mUnlinkId > 0 && pRouteAgentServerData->mServerInfo.url() != xData.url())
        {
            NFLogWarning(NF_LOG_DEFAULT, 0, "the server:{} old url:{} changed, new url:{}",
                         pRouteAgentServerData->mServerInfo.server_name(), pRouteAgentServerData->mServerInfo.url(),
                         xData.url());
            FindModule<NFIMessageModule>()->CloseLinkId(pRouteAgentServerData->mUnlinkId);

            pRouteAgentServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(m_serverType, xData.url(), PACKET_PARSE_TYPE_INTERNAL);

            FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, pRouteAgentServerData->mUnlinkId, this,
                                                             &NFWorkServerModule::OnRouteAgentServerSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, pRouteAgentServerData->mUnlinkId, this,
                                                             &NFWorkServerModule::OnHandleRouteAgentServerOtherMessage);
        }
    }

    pRouteAgentServerData->mServerInfo = xData;

    return 0;
}

/**
 * @brief 处理路由代理服务器Socket事件
 * 
 * 当路由代理服务器连接或断开时，更新相关状态并注册回调。
 * 
 * @param nEvent 事件类型
 * @param unLinkId 链接ID
 * @return 处理结果
 */
int NFWorkServerModule::OnRouteAgentServerSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", m_serverType);

    if (nEvent == eMsgType_CONNECTED)
    {
        NFLogDebug(NF_LOG_DEFAULT, 0, "server:{} connect route agent server success!", pConfig->ServerName);

        RegisterRouteAgentServer(unLinkId);
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "server:{} disconnect route agent server success", pConfig->ServerName);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理路由代理服务器其他消息
 * 
 * 处理路由代理服务器发送的未注册协议消息。
 * 
 * @param unLinkId 链接ID
 * @param packet 消息包
 * @return 处理结果
 */
int NFWorkServerModule::OnHandleRouteAgentServerOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogWarning(NF_LOG_DEFAULT, 0, "msg:{} not handled!", packet.ToString());
    return 0;
}

/**
 * @brief 注册路由代理服务器
 * 
 * 向路由代理服务器发送注册请求。
 * 
 * @param unLinkId 链接ID
 * @return 注册结果
 */
int NFWorkServerModule::RegisterRouteAgentServer(uint64_t unLinkId)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);

        pData->set_server_state(NFrame::EST_NARMAL);

        FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg, 0);
    }
    return 0;
}

/**
 * @brief 处理路由代理服务器注册响应
 * 
 * 当路由代理服务器注册成功时，完成启动任务。
 * 
 * @param unLinkId 链接ID
 * @param packet 消息包
 * @return 处理结果
 */
int NFWorkServerModule::OnRegisterRouteAgentServerRspProcess(uint64_t unLinkId, NFDataPackage& packet)
{
    //完成服务器启动任务
    if (!m_pObjPluginManager->IsInited(m_serverType))
    {
        FinishAppTask(m_serverType, APP_INIT_CONNECT_ROUTE_AGENT_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
    }

    return 0;
}




