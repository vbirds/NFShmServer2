// -------------------------------------------------------------------------
//    @FileName         :    NFCProxyAgentServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCProxyAgentServerModule
//
// -------------------------------------------------------------------------

#include "NFProxyAgentServerModule.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <NFComm/NFPluginModule/NFEventDefine.h>
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFServerComm/NFServerCommon/NFIProxyClientModule.h"
#include "NFComm/NFCore/NFServerIDUtil.h"

#define PROXY_AGENT_SERVER_CONNECT_MASTER_SERVER "ProxyAgentServer Connect MasterServer"

NFCProxyAgentServerModule::NFCProxyAgentServerModule(NFIPluginManager *p): NFIDynamicModule(p)
{
}

NFCProxyAgentServerModule::~NFCProxyAgentServerModule()
{
}

bool NFCProxyAgentServerModule::Awake()
{
    //不需要固定帧，需要尽可能跑得快
    //m_pObjPluginManager->SetFixedFrame(false);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_PROXY_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, this, &NFCProxyAgentServerModule::OnServerRegisterProcess);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_PROXY_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, this, &NFCProxyAgentServerModule::OnHandleServerReport);

    //注册要完成的服务器启动任务
    RegisterAppTask(NF_ST_PROXY_AGENT_SERVER, APP_INIT_CONNECT_MASTER, PROXY_AGENT_SERVER_CONNECT_MASTER_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);

    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
    if (pConfig)
    {
        m_pObjPluginManager->SetIdleSleepUs(pConfig->IdleSleepUS);

        uint64_t extern_unlinkId = FindModule<NFIMessageModule>()->BindServer(NF_ST_PROXY_AGENT_SERVER, pConfig->Url, pConfig->NetThreadNum, pConfig->MaxConnectNum,
                                                                              PACKET_PARSE_TYPE_INTERNAL);
        if (extern_unlinkId > 0)
        {
            /*
            注册服务器事件
            */
            uint64_t proxyServerLinkId = extern_unlinkId;
            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_PROXY_AGENT_SERVER, proxyServerLinkId, this,
                                                             &NFCProxyAgentServerModule::OnProxyAgentServerSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_PROXY_AGENT_SERVER, proxyServerLinkId, this,
                                                             &NFCProxyAgentServerModule::OnHandleProxyAgentServerOtherMessage);
            NFLogInfo(NF_LOG_DEFAULT, 0, "proxy agent server listen success, serverId:{}, ip:{}, port:{}",
                      pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
        }
        else
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "proxy agent server listen failed!, serverId:{}, ip:{}, port:{}",
                      pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
            return false;
        }
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the Proxy Agent Server config!");
        return false;
    }

    Subscribe(NF_ST_PROXY_AGENT_SERVER, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);
    Subscribe(NF_ST_PROXY_AGENT_SERVER, NFrame::NF_EVENT_SERVER_APP_FINISH_INITED, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);
    return true;
}

int NFCProxyAgentServerModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage)
{
    CHECK_EXPR(serverType == NF_ST_PROXY_AGENT_SERVER, -1, "");
    if (bySrcType == NFrame::NF_EVENT_SERVER_TYPE)
    {
        if (nEventID == NFrame::NF_EVENT_SERVER_DEAD_EVENT)
        {
            SetTimer(10000, 10000, 0);
        }
        else if (nEventID == NFrame::NF_EVENT_SERVER_APP_FINISH_INITED)
        {
            RegisterMasterServer(NFrame::EST_NARMAL);
        }
    }

    return 0;
}

int NFCProxyAgentServerModule::OnTimer(uint32_t nTimerID)
{
    if (nTimerID == 10000)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "kill the exe..................");
        NFSLEEP(1000);
        exit(0);
    }
    return 0;
}

int NFCProxyAgentServerModule::ConnectMasterServer(const NFrame::ServerInfoReport &xData)
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
    if (pConfig)
    {
        auto pMsterServerData = FindModule<NFIMessageModule>()->GetMasterData(NF_ST_PROXY_AGENT_SERVER);
        if (pMsterServerData->mUnlinkId <= 0)
        {
            pMsterServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_PROXY_AGENT_SERVER, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_PROXY_AGENT_SERVER, pMsterServerData->mUnlinkId, this, &NFCProxyAgentServerModule::OnMasterSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_PROXY_AGENT_SERVER, pMsterServerData->mUnlinkId, this, &NFCProxyAgentServerModule::OnHandleMasterOtherMessage);
        }

        pMsterServerData->mServerInfo = xData;
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the Proxy Agent Server config!");
        return -1;
    }

    return 0;
}

bool NFCProxyAgentServerModule::Init()
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
    NF_ASSERT(pConfig);
#if NF_PLATFORM == NF_PLATFORM_WIN
    NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(NF_ST_PROXY_AGENT_SERVER);
	int32_t ret = ConnectMasterServer(masterData);
	CHECK_EXPR(ret == 0, false, "ConnectMasterServer Failed, url:{}", masterData.DebugString());
#else
    if (pConfig->RouteConfig.NamingHost.empty())
    {
        NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(NF_ST_PROXY_AGENT_SERVER);
        int32_t ret = ConnectMasterServer(masterData);
        CHECK_EXPR(ret == 0, false, "ConnectMasterServer Failed, url:{}", masterData.DebugString());
    }
#endif

    return true;
}

bool NFCProxyAgentServerModule::Execute()
{
    ServerReport();
    return true;
}

bool NFCProxyAgentServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_PROXY_AGENT_SERVER);

    return true;
}

/*
	处理Master服务器链接事件
*/
int NFCProxyAgentServerModule::OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
    CHECK_EXPR(pConfig, -1, "pConfig == NULL");
    if (nEvent == eMsgType_CONNECTED)
    {
        NFLogDebug(NF_LOG_DEFAULT, 0, "proxy agent server connect master success!");
        if (!m_pObjPluginManager->IsInited(NF_ST_PROXY_AGENT_SERVER))
        {
            RegisterMasterServer(NFrame::EST_INIT);
        }
        else
        {
            RegisterMasterServer(NFrame::EST_NARMAL);
        }

        //完成服务器启动任务
        if (!m_pObjPluginManager->IsInited(NF_ST_PROXY_AGENT_SERVER))
        {
            m_pObjPluginManager->FinishAppTask(NF_ST_PROXY_AGENT_SERVER, APP_INIT_CONNECT_MASTER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
        }
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "proxy agent server disconnect master success");
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/*
	处理Master服务器未注册协议
*/
int NFCProxyAgentServerModule::OnHandleMasterOtherMessage(uint64_t unLinkId, NFDataPackage &packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
    NFLogWarning(NF_LOG_DEFAULT, 0, "master server other message not handled:packet:{},ip:{}", packet.ToString(), ip);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::RegisterMasterServer(uint32_t serverState)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport *pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);
        pData->set_server_state(serverState);

        FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_PROXY_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg);
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::ServerReport()
{
    if (m_pObjPluginManager->IsLoadAllServer())
    {
        return 0;
    }

    static uint64_t mLastReportTime = m_pObjPluginManager->GetNowTime();
    if (mLastReportTime + 100000 > m_pObjPluginManager->GetNowTime())
    {
        return 0;
    }

    mLastReportTime = m_pObjPluginManager->GetNowTime();

    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
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
            FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_PROXY_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_MASTER_SERVER_REPORT, xMsg);
        }
    }
    return 0;
}

int NFCProxyAgentServerModule::OnHandleServerReport(uint64_t unLinkId, NFDataPackage &packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_NO_PRINTF(packet, xMsg);

    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
    CHECK_NULL(0, pConfig);

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport &xData = xMsg.server_list(i);
        switch (xData.server_type())
        {
            case NF_SERVER_TYPE::NF_ST_PROXY_SERVER:
            {
                OnHandleProxyReport(xData);
            }
            break;
            default:
                break;
        }
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");

    return 0;
}

int NFCProxyAgentServerModule::OnOtherServerRegisterProcess(const NFrame::ServerInfoReport &xData, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(xData.server_type() > NF_ST_NONE && xData.server_type() <= NF_ST_MAX, -1, "xData.server_type() > NF_ST_NONE && xData.server_type() <= NF_ST_MAX");

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_PROXY_AGENT_SERVER, xData.bus_id());
    if (pServerData == nullptr)
    {
        pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_PROXY_AGENT_SERVER, xData.bus_id(), (NF_SERVER_TYPE) xData.server_type(), xData);
    }

    pServerData->mUnlinkId = unLinkId;
    pServerData->mServerInfo = xData;
    FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_PROXY_AGENT_SERVER, xData.bus_id(), pServerData->mUnlinkId);
    NFLogInfo(NF_LOG_DEFAULT, 0, "{} Register Proxy Agent Server, serverName:{} busName:{}", xData.server_name(), xData.server_name(),
              xData.server_id());

    std::vector<NF_SHARE_PTR<NFServerData> > vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_PROXY_AGENT_SERVER, NF_ST_PROXY_SERVER);
    for (int i = 0; i < (int) vecServer.size(); i++)
    {
        auto pProxyServerData = vecServer[i];
        if (pProxyServerData)
        {
            NFrame::ServerInfoReportList xMsg;
            NFrame::ServerInfoReport *pData = xMsg.add_server_list();
            *pData = pProxyServerData->mServerInfo;
            FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg, 0);

            auto pOtherData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_PROXY_AGENT_SERVER, unLinkId);
            if (pOtherData)
            {
                NFrame::ServerInfoReportList reportMsg;
                NFrame::ServerInfoReport *pReportData = reportMsg.add_server_list();
                *pReportData = pOtherData->mServerInfo;
                FindModule<NFIMessageModule>()->Send(pProxyServerData->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, reportMsg, 0);
            }
            else
            {
                NFLogError(NF_LOG_DEFAULT, 0, "GetServerByUnlinkId Failed!");
            }
        }
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::OnHandleProxyReport(const NFrame::ServerInfoReport &xData)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(xData.server_type() == NF_ST_PROXY_SERVER, -1, "xData.server_type() == NF_ST_PROXY_SERVER");

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_PROXY_AGENT_SERVER, xData.bus_id());
    if (pServerData == nullptr)
    {
        pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_PROXY_AGENT_SERVER, xData.bus_id(), NF_ST_PROXY_SERVER, xData);

        pServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_PROXY_AGENT_SERVER, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
        FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_PROXY_AGENT_SERVER, xData.bus_id(), pServerData->mUnlinkId);

        FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_PROXY_AGENT_SERVER, pServerData->mUnlinkId, this, &NFCProxyAgentServerModule::OnProxyServerSocketEvent);
        FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_PROXY_AGENT_SERVER, pServerData->mUnlinkId, this, &NFCProxyAgentServerModule::OnHandleProxyOtherMessage);
    }
    else
    {
        if (pServerData->mUnlinkId > 0 && pServerData->mServerInfo.url() != xData.url())
        {
            NFLogWarning(NF_LOG_DEFAULT, 0, "the server:{} old url:{} changed, new url:{}", pServerData->mServerInfo.server_name(), pServerData->mServerInfo.url(), xData.url());
            FindModule<NFIMessageModule>()->CloseLinkId(pServerData->mUnlinkId);
            pServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_PROXY_AGENT_SERVER, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
            FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_PROXY_AGENT_SERVER, xData.bus_id(), pServerData->mUnlinkId);

            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_PROXY_AGENT_SERVER, pServerData->mUnlinkId, this, &NFCProxyAgentServerModule::OnProxyServerSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_PROXY_AGENT_SERVER, pServerData->mUnlinkId, this, &NFCProxyAgentServerModule::OnHandleProxyOtherMessage);
        }
    }

    pServerData->mServerInfo = xData;
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::OnProxyServerSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    if (nEvent == eMsgType_CONNECTED)
    {
        NFLogDebug(NF_LOG_DEFAULT, 0, "proxy agent server connect proxy server success!");
        RegisterProxyServer(unLinkId);
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "proxy agent server disconnect proxy server");
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::OnHandleProxyOtherMessage(uint64_t unLinkId, NFDataPackage &packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_SHARE_PTR<NFServerData> pProxyServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_PROXY_AGENT_SERVER, unLinkId);
    if (pProxyServerData && pProxyServerData->mServerInfo.server_type() == NF_ST_PROXY_SERVER)
    {
        uint64_t dstBusId = packet.nDstId;
        auto pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_PROXY_AGENT_SERVER, dstBusId);
        if (pServerData)
        {
            NFLogTrace(NF_LOG_DEFAULT, 0, "Trans ProxyServer:{} packet:{} to OtherServer:{}({})",
                       pProxyServerData->mServerInfo.server_id(), packet.ToString(), pServerData->mServerInfo.server_name(),
                       pServerData->mServerInfo.server_id());
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Can't find the busId:{} busName:{} packet:{}", dstBusId, NFServerIDUtil::GetBusNameFromBusID(dstBusId), packet.ToString());
        }
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "unLinkId:{} can't find proxy! packet:{}", unLinkId, packet.ToString());
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::RegisterProxyServer(uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_AGENT_SERVER);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport *pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);
        pData->set_server_state(NFrame::EST_NARMAL);

        FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg, 0);
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}


int NFCProxyAgentServerModule::OnProxyAgentServerSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    if (nEvent == eMsgType_CONNECTED)
    {
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        OnHandleProxyServerDisconnect(unLinkId);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::OnHandleProxyAgentServerOtherMessage(uint64_t unLinkId, NFDataPackage &packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_PROXY_AGENT_SERVER, unLinkId);
    if (pServerData)
    {
        uint64_t dstBusId = packet.nDstId;
        if (dstBusId <= LOCAL_AND_CROSS_MAX)
        {
            auto vecProxyServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_PROXY_AGENT_SERVER, NF_ST_PROXY_SERVER);
            if (vecProxyServerData.size() > 0)
            {
                for (int i = 0; i < (int) vecProxyServerData.size(); i++)
                {
                    auto pProxyServerData = vecProxyServerData[i];
                    NFLogTrace(NF_LOG_DEFAULT, 0, "Trans Server:{}({}) packet:{} to ProxyServer:{}({})", pServerData->mServerInfo.server_name(),
                               pServerData->mServerInfo.server_id(), packet.ToString(), pProxyServerData->mServerInfo.server_name(),
                               pProxyServerData->mServerInfo.server_id());
                    FindModule<NFIMessageModule>()->TransPackage(pProxyServerData->mUnlinkId, packet);
                }
            }
            else
            {
                NFLogError(NF_LOG_DEFAULT, 0, "Can't find the busId:{} busName:{} packet:{}", dstBusId, NFServerIDUtil::GetBusNameFromBusID(dstBusId), packet.ToString());
            }
        }
        else
        {
            auto pProxyServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_PROXY_AGENT_SERVER, dstBusId);
            if (pProxyServerData && pProxyServerData->mServerInfo.server_type() == NF_ST_PROXY_SERVER)
            {
                NFLogTrace(NF_LOG_DEFAULT, 0, "Trans Server:{}({}) packet:{} to ProxyServer:{}({})", pServerData->mServerInfo.server_name(),
                           pServerData->mServerInfo.server_id(), packet.ToString(), pProxyServerData->mServerInfo.server_name(),
                           pProxyServerData->mServerInfo.server_id());
                FindModule<NFIMessageModule>()->TransPackage(pProxyServerData->mUnlinkId, packet);
            }
            else
            {
                NFLogError(NF_LOG_DEFAULT, 0, "Can't find the busId:{} busName:{} packet:{}", dstBusId, NFServerIDUtil::GetBusNameFromBusID(dstBusId), packet.ToString());
            }
        }
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "unLinkId:{} can't find server! packet:{}", unLinkId, packet.ToString());
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::OnHandleProxyServerDisconnect(uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_PROXY_AGENT_SERVER, unLinkId);
    if (pServerData)
    {
        pServerData->mServerInfo.set_server_state(NFrame::EST_CRASH);
        pServerData->mUnlinkId = 0;

        NFLogError(NF_LOG_DEFAULT, 0, "the server disconnect from proxy agent server, serverName:{}, busid:{}, serverIp:{}, serverPort:{}"
                   , pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_ip(), pServerData->mServerInfo.server_port());
    }

    FindModule<NFIMessageModule>()->DelServerLink(NF_ST_PROXY_AGENT_SERVER, unLinkId);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCProxyAgentServerModule::OnServerRegisterProcess(uint64_t unLinkId, NFDataPackage &packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport &xData = xMsg.server_list(i);
        switch (xData.server_type())
        {
            case NF_SERVER_TYPE::NF_ST_PROXY_SERVER:
            {
                NF_SHARE_PTR<NFServerData> pProxyServer = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_PROXY_AGENT_SERVER, xData.bus_id());
                if (pProxyServer)
                {
                    pProxyServer->mServerInfo = xData;
                }

                std::vector<NF_SHARE_PTR<NFServerData> > vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_PROXY_AGENT_SERVER);
                for (int j = 0; j < (int) vecServer.size(); j++)
                {
                    auto pServerData = vecServer[j];
                    NF_SERVER_TYPE server_type = (NF_SERVER_TYPE)pServerData->mServerInfo.server_type();
                    if (IsWorkServer(server_type))
                    {
                        FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg, 0);

                        NFrame::ServerInfoReportList reportMsg;
                        NFrame::ServerInfoReport *pData = reportMsg.add_server_list();
                        *pData = pServerData->mServerInfo;
                        FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, reportMsg, 0);
                    }
                }
            }
            break;
            default:
            {
                OnOtherServerRegisterProcess(xData, unLinkId);
            }
            break;
        }
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}
