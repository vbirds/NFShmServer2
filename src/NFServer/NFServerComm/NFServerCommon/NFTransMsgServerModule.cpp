// -------------------------------------------------------------------------
//    @FileName         :    NFTransMsgServerModule.cpp
//    @Author           :    gaoyi
//    @Date             :    22-10-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFTransMsgServerModule
//    @Desc             :    事务消息服务器模块实现文件，提供事务消息服务器的通用功能实现。
//                          该文件实现了事务消息服务器模块类的方法，包括服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供事务消息服务器的通用功能实现、支持服务器连接管理、
//                          支持消息处理和路由分发、提供网络事件处理。
//                          事务消息服务器模块实现是NFShmXFrame框架的事务处理组件实现，负责：
//                          - 事务消息服务器的通用功能实现
//                          - 服务器连接管理和状态维护实现
//                          - 事务处理和状态管理实现
//                          - 消息处理和路由分发实现
//                          - 网络事件处理和回调实现
//                          - 跨服务器事务通信支持实现
//
// -------------------------------------------------------------------------
#include "NFTransMsgServerModule.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFServerCommonDefine.h"
#include "NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"

#define SERVER_CONNECT_MASTER_SERVER "Server Connect MasterServer"

#define SERVER_REPORT_TO_MASTER_SERVER_TIMER_ID 101
#define SERVER_SERVER_DEAD_TIMER_ID 102

/**
 * @brief 检查是否连接主服务器
 * 
 * @return true表示连接主服务器，false表示不连接
 */
bool NFTransMsgServerModule::IsConnectMasterServer() const
{
    return m_connectMasterServer;
}

/**
 * @brief 设置是否连接主服务器
 * 
 * @param connectMasterServer 是否连接主服务器
 */
void NFTransMsgServerModule::SetConnectMasterServer(bool connectMasterServer)
{
    m_connectMasterServer = connectMasterServer;
}

/**
 * @brief 绑定服务器
 * 
 * 初始化服务器配置，绑定网络端口，注册消息回调
 * 
 * @return 0表示成功
 */
int NFTransMsgServerModule::BindServer()
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType); ///< 获取服务器配置
    CHECK_EXPR_ASSERT(pConfig, -1, "GetAppConfig Failed, server type:{}", m_serverType); ///< 检查配置获取是否成功

    //////////////////////master msg//////////////////////////
    // 注册主服务器消息回调
    FindModule<NFIMessageModule>()->AddMessageCallBack(m_serverType, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, this,
                                                       &NFTransMsgServerModule::OnHandleServerReportFromMasterServer);

    if (!m_pObjPluginManager->IsLoadAllServer())
    {
        CHECK_EXPR_ASSERT(pConfig->ServerType == m_serverType, -1, "server config error, server id not match the server type!:{}", m_serverType); ///< 检查服务器类型匹配
    }

    //注册要完成的服务器启动任务
    if (m_connectMasterServer)
    {
        RegisterAppTask(m_serverType, APP_INIT_CONNECT_MASTER,
                                             NF_FORMAT("{}_{}", pConfig->ServerName, SERVER_CONNECT_MASTER_SERVER), APP_INIT_TASK_GROUP_SERVER_CONNECT); ///< 注册连接主服务器任务
    }

    // 绑定服务器端口
    uint64_t serverLinkId = FindModule<NFIMessageModule>()->BindServer(m_serverType, pConfig->Url, pConfig->NetThreadNum, pConfig->MaxConnectNum,
                                                                       PACKET_PARSE_TYPE_INTERNAL);
    CHECK_EXPR_ASSERT(serverLinkId > 0, -1, "Server:{} Listen Failed, ServerId:{}, Ip:{}, Port:{}", pConfig->ServerName, pConfig->ServerId,
                      pConfig->ServerIp, pConfig->ServerPort); ///< 检查服务器绑定是否成功

    FindModule<NFIMessageModule>()->SetServerLinkId(m_serverType, serverLinkId); ///< 设置服务器链接ID
    FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, serverLinkId, this, &NFTransMsgServerModule::OnServerSocketEvent); ///< 添加事件回调
    FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, serverLinkId, this,
                                                     &NFTransMsgServerModule::OnHandleServerOtherMessage); ///< 添加其他消息回调
    NFLogInfo(NF_LOG_DEFAULT, 0, "Server:{} Listen Success, ServerId:{}, Ip:{}, Port:{}", pConfig->ServerName, pConfig->ServerId, pConfig->ServerIp,
              pConfig->ServerPort); ///< 记录服务器启动成功日志

    Subscribe(m_serverType, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__); ///< 订阅服务器死亡事件
    Subscribe(m_serverType, NFrame::NF_EVENT_SERVER_APP_FINISH_INITED, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__); ///< 订阅服务器初始化完成事件

    SetTimer(SERVER_REPORT_TO_MASTER_SERVER_TIMER_ID, 10000); ///< 设置向主服务器报告定时器
    return 0;
}

/**
 * @brief 执行事件处理
 * 
 * 处理服务器事件，包括服务器死亡和初始化完成事件
 * 
 * @param serverType 服务器类型
 * @param nEventID 事件ID
 * @param bySrcType 源类型
 * @param nSrcID 源ID
 * @param pMessage 消息数据
 * @return 0表示成功
 */
int NFTransMsgServerModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID,
                                      const google::protobuf::Message *pMessage)
{
    CHECK_EXPR(serverType == m_serverType, 0, ""); ///< 检查服务器类型匹配
    if (bySrcType == NFrame::NF_EVENT_SERVER_TYPE)
    {
        if (nEventID == NFrame::NF_EVENT_SERVER_DEAD_EVENT)
        {
            SetTimer(SERVER_SERVER_DEAD_TIMER_ID, 10000, 0); ///< 设置服务器死亡定时器
        }
        else if (nEventID == NFrame::NF_EVENT_SERVER_APP_FINISH_INITED)
        {
            RegisterMasterServer(NFrame::EST_NARMAL); ///< 注册主服务器
        }
    }

    return 0;
}

/**
 * @brief 定时器回调
 * 
 * 处理各种定时器事件
 * 
 * @param nTimerID 定时器ID
 * @return 0表示成功
 */
int NFTransMsgServerModule::OnTimer(uint32_t nTimerID)
{
    if (nTimerID == SERVER_REPORT_TO_MASTER_SERVER_TIMER_ID)
    {
        ServerReportToMasterServer(); ///< 向主服务器报告状态
    }
    else if (nTimerID == SERVER_SERVER_DEAD_TIMER_ID)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "kill the exe.................."); ///< 记录错误日志
        NFSLEEP(1000); ///< 等待1秒
        exit(0); ///< 退出程序
    }
    return 0;
}

int NFTransMsgServerModule::ConnectMasterServer()
{
    if (m_connectMasterServer == false)
    {
        return 0;
    }


    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
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

int NFTransMsgServerModule::OnServerSocketEvent(eMsgType nEvent, uint64_t unLinkId)
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

int NFTransMsgServerModule::OnHandleServerOtherMessage(uint64_t unLinkId, NFDataPackage &packet)
{
    NFLogWarning(NF_LOG_DEFAULT, 0, "msg:{} not handled!", packet.ToString());
    return 0;
}

int NFTransMsgServerModule::OnHandleServerDisconnect(uint64_t unLinkId)
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
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

int NFTransMsgServerModule::ConnectMasterServer(const NFrame::ServerInfoReport &xData)
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", m_serverType);
    auto pMsterServerData = FindModule<NFIMessageModule>()->GetMasterData(m_serverType);
    if (pMsterServerData->mUnlinkId <= 0)
    {
        pMsterServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(m_serverType, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
        FindModule<NFIMessageModule>()->AddEventCallBack(m_serverType, pMsterServerData->mUnlinkId, this,
                                                         &NFTransMsgServerModule::OnMasterSocketEvent);
        FindModule<NFIMessageModule>()->AddOtherCallBack(m_serverType, pMsterServerData->mUnlinkId, this,
                                                         &NFTransMsgServerModule::OnHandleMasterOtherMessage);
    }

    pMsterServerData->mServerInfo = xData;

    return 0;
}

int NFTransMsgServerModule::RegisterMasterServer(uint32_t serverState)
{
    if (m_connectMasterServer == false)
    {
        return 0;
    }

    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport *pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);

        pData->set_server_state(serverState);
        FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(m_serverType, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg);
    }
    return 0;
}

/*
	处理Master服务器链接事件
*/
int NFTransMsgServerModule::OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
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

        //完成服务器启动任务
        if (!m_pObjPluginManager->IsInited(m_serverType))
        {
            FinishAppTask(m_serverType, APP_INIT_CONNECT_MASTER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
        }
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "server:{} disconnect master success", pConfig->ServerName);
    }
    return 0;
}

/*
	处理Master服务器未注册协议
*/
int NFTransMsgServerModule::OnHandleMasterOtherMessage(uint64_t unLinkId, NFDataPackage &packet)
{
    NFLogWarning(NF_LOG_DEFAULT, 0, "master server other message not handled:msgId:{}", packet.ToString());
    return 0;
}

int NFTransMsgServerModule::OnHandleServerReportFromMasterServer(uint64_t unLinkId, NFDataPackage &packet)
{
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_NO_PRINTF(packet, xMsg);

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport &xData = xMsg.server_list(i);
        switch (xData.server_type())
        {
            default:
            {
                OnHandleOtherServerReportFromMasterServer(xData);
            }
                break;
        }
    }
    return 0;
}

int NFTransMsgServerModule::OnHandleOtherServerReportFromMasterServer(const NFrame::ServerInfoReport &xData)
{
    return 0;
}

int NFTransMsgServerModule::ServerReportToMasterServer()
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport *pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);
        pData->set_server_state(NFrame::EST_NARMAL);

        NFIMonitorModule *pMonitorModule = m_pObjPluginManager->FindModule<NFIMonitorModule>();
        if (pMonitorModule)
        {
            const NFSystemInfo &systemInfo = pMonitorModule->GetSystemInfo();
            NFServerCommon::WriteServerInfo(pData, systemInfo);
        }

        if (pData->proc_cpu() > 0 && pData->proc_mem() > 0)
        {
            FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(m_serverType, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_MASTER_SERVER_REPORT, xMsg);
        }
    }
    return 0;
}