// -------------------------------------------------------------------------
//    @FileName         :    NFCRouteServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCRouteServerModule
//    @Desc             :    NFShmXFrame路由服务器模块实现
//                          实现路由服务器模块的核心功能，包括消息路由和负载均衡
//                          支持跨服通信、路由分发和连接管理
//
//    @Summary          :    路由服务器是分布式游戏服务器架构中的核心路由组件
//                          负责接收来自路由代理服务器的消息，并根据路由规则转发到目标路由代理
//                          支持跨服路由、区服路由、群发路由等多种高级路由功能
//                          提供路由表管理、负载均衡、故障转移等核心功能
//
//    @Architecture     :    1. 路由表管理：维护所有路由代理服务器的路由表
//                          2. 消息转发：根据目标BusId转发消息到对应的路由代理
//                          3. 跨服路由：处理跨服服务器间的消息路由
//                          4. 区服路由：支持按区服ID进行消息路由
//                          5. 群发路由：支持向多个目标同时发送消息
//                          6. 故障处理：检测路由代理断开，更新路由表
//
//    @Routing Rules    :    1. LOCAL_ROUTE: 本服路由，转发到本服路由代理的随机一个
//                          2. LOCAL_ROUTE+index: 本服索引路由，转发到指定索引的路由代理
//                          3. CROSS_ROUTE: 跨服路由，转发到跨服路由代理的随机一个
//                          4. CROSS_ROUTE+index: 跨服索引路由，转发到指定索引的跨服路由代理
//                          5. LOCAL_ROUTE_ZONE+zoneId: 区服路由，转发到指定区服的路由代理
//                          6. CROSS_ROUTE_ZONE+zoneId: 跨服区服路由，群发到指定区服的所有路由代理
//                          7. LOCAL_ALL_ROUTE: 本服群发路由，转发到所有本服路由代理
//                          8. CROSS_ALL_ROUTE: 跨服群发路由，转发到所有跨服路由代理
//
//    @Error Handling   :    1. 目标路由代理不存在时返回ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST
//                          2. 不支持的路由类型返回ERR_CODE_ROUTER_NOT_SUPPORTTED
//                          3. 路由代理断开时更新连接状态
//                          4. 路由失败时记录详细错误日志并尝试回退
//
//    @Performance      :    1. 高效的路由表查找算法
//                          2. 连接池管理，复用连接资源
//                          3. 异步消息处理，避免阻塞
//                          4. 定时心跳检测，及时发现问题
// -------------------------------------------------------------------------

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFCRouteServerModule.h"

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFCore/NFServerIDUtil.h"

#define ROUTE_SERVER_CONNECT_MASTER_SERVER "RouteServer Connect MasterServer"

/**
 * @brief 构造函数
 *
 * 初始化路由服务器模块，设置插件管理器
 *
 * @param p 插件管理器指针
 */
NFCRouteServerModule::NFCRouteServerModule(NFIPluginManager* p) : NFIRouteServerModule(p)
{
}

/**
 * @brief 析构函数
 *
 * 清理路由服务器模块资源
 */
NFCRouteServerModule::~NFCRouteServerModule()
{
}

/**
 * @brief 模块唤醒 - 路由服务器模块初始化
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 注册消息回调，处理服务器注册、报告和路由代理注册消息
 * - 注册应用任务，包括连接Master服务器
 * - 绑定服务器，监听路由代理连接
 * - 订阅事件，处理服务器状态变化
 *
 * @details 初始化流程：
 * 1. 注册NF_SERVER_TO_SERVER_REGISTER消息回调，处理服务器注册
 * 2. 注册NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER消息回调，处理服务器报告
 * 3. 注册NF_ROUTER_CMD_INTERNAL_C2R_REG_RAASSOCAPPSVS消息回调，处理路由代理注册
 * 4. 注册应用任务：连接Master服务器
 * 5. 绑定服务器端口，设置连接事件回调
 * 6. 订阅服务器死亡和初始化完成事件
 *
 * @return 初始化结果状态码，0表示成功，-1表示失败
 */
int NFCRouteServerModule::Awake()
{
    //不需要固定帧，需要尽可能跑得快
    //m_pObjPluginManager->SetFixedFrame(false);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_ROUTE_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, this, &NFCRouteServerModule::OnServerRegisterProcess);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_ROUTE_SERVER, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, this, &NFCRouteServerModule::OnHandleServerReport);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_ROUTE_SERVER, NF_MODULE_FRAME, NFrame::NF_ROUTER_CMD_INTERNAL_C2R_REG_RAASSOCAPPSVS, this, &NFCRouteServerModule::OnHandleServerRegisterRouteAgent);

    //注册要完成的服务器启动任务
    RegisterAppTask(NF_ST_ROUTE_SERVER, APP_INIT_CONNECT_MASTER, ROUTE_SERVER_CONNECT_MASTER_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER);
    if (pConfig)
    {
        m_pObjPluginManager->SetIdleSleepUs(pConfig->IdleSleepUS);
        uint64_t unlinkId = FindModule<NFIMessageModule>()->BindServer(NF_ST_ROUTE_SERVER, pConfig->Url, pConfig->NetThreadNum, pConfig->MaxConnectNum, PACKET_PARSE_TYPE_INTERNAL);
        if (unlinkId > 0)
        {
            /*
                注册客户端事件
            */
            uint64_t routeServerLinkId = unlinkId;
            FindModule<NFIMessageModule>()->SetServerLinkId(NF_ST_ROUTE_SERVER, routeServerLinkId);
            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_ROUTE_SERVER, routeServerLinkId, this, &NFCRouteServerModule::OnRouteSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_ROUTE_SERVER, routeServerLinkId, this, &NFCRouteServerModule::OnHandleOtherMessage);
            NFLogInfo(NF_LOG_DEFAULT, 0, "route server listen success, serverId:{}, ip:{}, port:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
        }
        else
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "route server listen failed, serverId:{}, ip:{}, port:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
            return -1;
        }
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the Game Server config!");
        return -1;
    }

    Subscribe(NF_ST_ROUTE_SERVER, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);
    Subscribe(NF_ST_ROUTE_SERVER, NFrame::NF_EVENT_SERVER_APP_FINISH_INITED, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);

    return 0;
}

/**
 * @brief 执行事件
 *
 * 处理服务器事件执行，包括服务器死亡事件和初始化完成事件
 *
 * @param serverType 服务器类型
 * @param nEventID 事件ID
 * @param bySrcType 源类型
 * @param nSrcID 源ID
 * @param pMessage 消息数据
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteServerModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage)
{
    CHECK_EXPR(serverType == NF_ST_ROUTE_SERVER, -1, "");
    if (bySrcType == NFrame::NF_EVENT_SERVER_TYPE)
    {
        if (nEventID == NFrame::NF_EVENT_SERVER_DEAD_EVENT) { SetTimer(10000, 10000, 0); }
        else if (nEventID == NFrame::NF_EVENT_SERVER_APP_FINISH_INITED) { RegisterMasterServer(NFrame::EST_NARMAL); }
    }

    return 0;
}

/**
 * @brief 定时器回调
 *
 * 处理定时器事件，包括服务器死亡处理
 *
 * @param nTimerID 定时器ID
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteServerModule::OnTimer(uint32_t nTimerID)
{
    if (nTimerID == 10000)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "kill the exe..................");
        NFSLEEP(1000);
        exit(0);
    }
    return 0;
}

/**
 * @brief 连接Master服务器
 *
 * 连接到Master服务器，设置连接回调
 *
 * @param xData 服务器信息报告
 * @return 连接结果状态码，0表示成功
 */
int NFCRouteServerModule::ConnectMasterServer(const NFrame::ServerInfoReport& xData)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER);
    if (pConfig)
    {
        auto pMasterServerData = FindModule<NFIMessageModule>()->GetMasterData(NF_ST_ROUTE_SERVER);
        if (pMasterServerData->mUnlinkId <= 0)
        {
            pMasterServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_ROUTE_SERVER, xData.url(), PACKET_PARSE_TYPE_INTERNAL);

            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_ROUTE_SERVER, pMasterServerData->mUnlinkId, this, &NFCRouteServerModule::OnMasterSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_ROUTE_SERVER, pMasterServerData->mUnlinkId, this, &NFCRouteServerModule::OnHandleMasterOtherMessage);
        }

        pMasterServerData->mServerInfo = xData;
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the route Server config!");
        return -1;
    }

    return 0;
}

/**
 * @brief 模块初始化
 *
 * 初始化路由服务器模块，包括连接Master服务器等
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCRouteServerModule::Init()
{
#if NF_PLATFORM == NF_PLATFORM_WIN
    NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(NF_ST_ROUTE_SERVER);
    int32_t ret = ConnectMasterServer(masterData);
    CHECK_ERR(0, ret, "ConnectMasterServer Failed, url:{}", masterData.DebugString());
#else
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER); if (pConfig && pConfig->RouteConfig.NamingHost.empty())
    {
        NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(NF_ST_ROUTE_SERVER);
        int32_t ret = ConnectMasterServer(masterData);
        CHECK_ERR(0, ret, "ConnectMasterServer Failed, url:{}", masterData.DebugString());
    }
#endif

    return 0;
}

/**
 * @brief 模块定时更新
 *
 * 每帧调用，处理模块的定时任务，包括服务器报告
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteServerModule::Tick()
{
    ServerReport();
    return 0;
}

int NFCRouteServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_ROUTE_SERVER);
    return 0;
}

int NFCRouteServerModule::OnRouteSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    if (nEvent == eMsgType_CONNECTED)
    {
    }
    else if (nEvent == eMsgType_DISCONNECTED) { OnHandleServerDisconnect(unLinkId); }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCRouteServerModule::OnHandleServerDisconnect(uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_ROUTE_SERVER, unLinkId);
    if (pServerData)
    {
        pServerData->mServerInfo.set_server_state(NFrame::EST_CRASH);
        pServerData->mUnlinkId = 0;

        NFLogError(NF_LOG_DEFAULT, 0, "the server disconnect from route server, serverName:{}, busid:{}, busname:{}. serverIp:{}, serverPort:{}", pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_id(), pServerData->mServerInfo.server_ip(),
                   pServerData->mServerInfo.server_port());
    }

    FindModule<NFIMessageModule>()->DelServerLink(NF_ST_ROUTE_SERVER, unLinkId);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理路由服务器的其他消息 - 核心路由转发函数
 *
 * 这是路由服务器的核心消息处理函数，负责接收来自路由代理的消息并转发到目标路由代理。
 * 支持多种高级路由功能：跨服路由、区服路由、群发路由等。
 *
 * @param unLinkId 源连接ID，标识消息来源的路由代理
 * @param packet 数据包，包含源目标信息和消息内容
 *
 * @details 路由转发规则：
 * 1. LOCAL_ROUTE: 本服路由，转发到本服路由代理的随机一个
 * 2. LOCAL_ROUTE+index: 本服索引路由，转发到指定索引的路由代理
 * 3. CROSS_ROUTE: 跨服路由，转发到跨服路由代理的随机一个
 * 4. CROSS_ROUTE+index: 跨服索引路由，转发到指定索引的跨服路由代理
 * 5. LOCAL_ROUTE_ZONE+zoneId: 区服路由，转发到指定区服的路由代理
 * 6. CROSS_ROUTE_ZONE+zoneId: 跨服区服路由，群发到指定区服的所有路由代理
 * 7. LOCAL_ALL_ROUTE: 本服群发路由，转发到所有本服路由代理
 * 8. CROSS_ALL_ROUTE: 跨服群发路由，转发到所有跨服路由代理
 *
 * @details 处理流程：
 * 1. 解析源和目标BusId，确定路由类型
 * 2. 根据路由类型选择目标路由代理
 * 3. 如果目标路由代理不存在，返回错误码
 * 4. 记录详细的转发日志
 * 5. 处理错误情况，尝试回退到源路由代理
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteServerModule::OnHandleOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    uint32_t fromBusId = GetBusIdFromUnlinkId(packet.nSrcId);
    uint32_t fromServerType = GetServerTypeFromUnlinkId(packet.nSrcId);

    uint32_t serverType = GetServerTypeFromUnlinkId(packet.nDstId);
    uint32_t destBusId = GetBusIdFromUnlinkId(packet.nDstId);

    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER);
    CHECK_EXPR(pConfig != NULL, -1, "pConfig == NULL");

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_ROUTE_SERVER, unLinkId);
    CHECK_EXPR(pServerData != NULL, -1, "pServer == NULL");

    if (packet.nErrCode == NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "the trans msg failed, can't find dest server, --{}:{} cant' trans route agent server({}:{}) msg from {}:{} to {}:{}, packet:{}", pConfig->ServerName, pConfig->ServerId, pServerData->mServerInfo.server_name(), pServerData->mServerInfo.server_id(),
                   GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType), NFServerIDUtil::GetBusNameFromBusID(destBusId), packet.ToString());

        NF_SHARE_PTR<NFServerData> pRegServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, fromBusId);
        if (pRegServerData)
        {
            NF_SHARE_PTR<NFServerData> pRouteAgent = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
            if (pRouteAgent) { FindModule<NFIMessageModule>()->TransPackage(pRouteAgent->mUnlinkId, packet); }
            else
            {
                NFLogError(NF_LOG_DEFAULT, 0, "the trans msg failed, can't find dest server, packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId),
                           GetServerName((NF_SERVER_TYPE)serverType), NFServerIDUtil::GetBusNameFromBusID(destBusId));
            }
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "the trans msg failed, can't find dest server, packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId),
                       GetServerName((NF_SERVER_TYPE)serverType), NFServerIDUtil::GetBusNameFromBusID(destBusId));
        }
        return 0;
    }

    if (destBusId <= LOCAL_AND_CROSS_MAX)
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "--{}:{} trans route agent server({}:{}) msg from {}:{} to {}:{}, packet:{} --", pConfig->ServerName, pConfig->ServerId, pServerData->mServerInfo.server_name(), pServerData->mServerInfo.server_id(), GetServerName((NF_SERVER_TYPE)fromServerType),
                  NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType), destBusId, packet.ToString());
    }
    else
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "--{}:{} trans route agent server({}:{}) msg from {}:{} to {}:{}, packet:{} --", pConfig->ServerName, pConfig->ServerId, pServerData->mServerInfo.server_name(), pServerData->mServerInfo.server_id(), GetServerName((NF_SERVER_TYPE)fromServerType),
                  NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType), NFServerIDUtil::GetBusNameFromBusID(destBusId), packet.ToString());
    }

    /**
     * @brief 本服路由处理 - LOCAL_ROUTE (路由服务器)
     *
     * 当目标busId==LOCAL_ROUTE时，采用本服路由机制。
     * 本服路由机制：除非明确表示要发往跨服服务器，否则就是本服路由
     * (包括跨服服务器的本服路由)
     *
     * 重要约束：需要保证本跨服服务器只能连接跨服route agent，
     * 不跨服服务器只能连接不跨服的route agent
     *
     * @details 处理逻辑：
     * 1. 根据服务器类型和跨服标志获取随机服务器
     * 2. 通过服务器的路由代理BusId找到对应的路由代理
     * 3. 如果找到目标路由代理，转发消息
     * 4. 如果未找到目标路由代理，返回错误码
     * 5. 记录详细的转发日志
     *
     * @note 这是路由服务器的本服路由处理，需要转发到对应的路由代理
     */
    if (destBusId == LOCAL_ROUTE)
    {
        NF_SHARE_PTR<NFServerData> pRegServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
        if (pRegServerData)
        {
            NF_SHARE_PTR<NFServerData> pRouteAgent = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
            if (pRouteAgent) { FindModule<NFIMessageModule>()->TransPackage(pRouteAgent->mUnlinkId, packet); }
            else
            {
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                NFLogError(NF_LOG_DEFAULT, 0, "packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                           NFServerIDUtil::GetBusNameFromBusID(destBusId));
            }
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "can't find destBusId, packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                       NFServerIDUtil::GetBusNameFromBusID(destBusId));
        }
    }
    /**
     * @brief 本服索引路由  LOCAL_ROUTE+index       本服路由机制，除非明确表示要发往跨服服务器，否则就是本服路由(包过跨服服务器的本服路由) (需要保证本跨服服务器只能连接跨服route agent， 不跨服服务器只能连接不跨服的route agent.)
     */
    else if (destBusId > LOCAL_ROUTE && destBusId < CROSS_ROUTE)
    {
        uint32_t index = destBusId - LOCAL_ROUTE;
        uint32_t realDestBusId = NFServerIDUtil::MakeProcID(pConfig->GetWorldId(), pConfig->GetZoneId(), serverType, index);
        NF_SHARE_PTR<NFServerData> pRegServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, realDestBusId);
        if (pRegServerData)
        {
            NF_SHARE_PTR<NFServerData> pRouteAgent = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
            if (pRouteAgent) { FindModule<NFIMessageModule>()->TransPackage(pRouteAgent->mUnlinkId, packet); }
            else
            {
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                NFLogError(NF_LOG_DEFAULT, 0, "packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                           NFServerIDUtil::GetBusNameFromBusID(realDestBusId));
            }
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "can't find destBusId, packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                       NFServerIDUtil::GetBusNameFromBusID(realDestBusId));
        }
    }
    /**
     * @brief 跨服路由处理 - CROSS_ROUTE (路由服务器)
     *
     * 当目标busId==CROSS_ROUTE时，采用跨服路由机制。
     * 跨服路由：明确指定要找跨服服务器，才走跨服路由
     *
     * @details 处理逻辑：
     * 1. 根据服务器类型获取跨服服务器的随机一个
     * 2. 通过服务器的路由代理BusId找到对应的路由代理
     * 3. 如果找到目标路由代理，转发消息
     * 4. 如果未找到目标路由代理，返回错误码
     * 5. 记录详细的转发日志
     *
     * @note 这是路由服务器的跨服路由处理，需要转发到对应的跨服路由代理
     */
    else if (destBusId == CROSS_ROUTE)
    {
        NF_SHARE_PTR<NFServerData> pRegServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_SERVER, (NF_SERVER_TYPE)serverType, true);
        if (pRegServerData)
        {
            NF_SHARE_PTR<NFServerData> pRouteAgent = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
            if (pRouteAgent) { FindModule<NFIMessageModule>()->TransPackage(pRouteAgent->mUnlinkId, packet); }
            else
            {
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                NFLogError(NF_LOG_DEFAULT, 0, "packet:{} trans failed, fromServer:{}:{} to destServer:{}:CROSS_ROUTE", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType));
            }
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "can't find destBusId, packet:{} trans failed, fromServer:{}:{} to destServer:{}:CROSS_ROUTE", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType));
        }
    }
    //跨服索引路由 CROSS_ROUTE+index          明确指定要找跨服服务器， 才走跨服路由
    else if (destBusId > CROSS_ROUTE && destBusId < LOCAL_ROUTE_ZONE)
    {
        if (!pConfig->IsCrossServer())
        {
            NFLogError(NF_LOG_DEFAULT, 0, "destBusId:{} route error, the pConfig is not cross server", destBusId);
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_NOT_SUPPORTTED;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            return 0;
        }
        uint32_t index = destBusId - CROSS_ROUTE;
        uint32_t realDestBusId = NFServerIDUtil::MakeProcID(pConfig->GetWorldId(), pConfig->GetZoneId(), serverType, index);
        NF_SHARE_PTR<NFServerData> pRegServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, realDestBusId);
        if (pRegServerData)
        {
            NF_SHARE_PTR<NFServerData> pRouteAgent = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
            if (pRouteAgent) { FindModule<NFIMessageModule>()->TransPackage(pRouteAgent->mUnlinkId, packet); }
            else
            {
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                NFLogError(NF_LOG_DEFAULT, 0, "packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                           NFServerIDUtil::GetBusNameFromBusID(realDestBusId));
            }
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "can't find destBusId, packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                       NFServerIDUtil::GetBusNameFromBusID(realDestBusId));
        }
    }
    /**
     * @brief 区服路由  LOCAL_ROUTE_ZONE+区服的zid(1-4096) 只有跨服route server服务器，才有区服路由的能力
     */
    else if (destBusId > LOCAL_ROUTE_ZONE && destBusId < CROSS_ROUTE_ZONE)
    {
        uint32_t realDestBusId = destBusId - LOCAL_ROUTE_ZONE;
        if (pConfig->GetZoneId() != realDestBusId)
        {
            if (!pConfig->IsCrossServer())
            {
                NFLogError(NF_LOG_DEFAULT, 0, "destBusId:{} zid route error, the pConfig is not cross server", realDestBusId);
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_NOT_SUPPORTTED;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                return 0;
            }
        }

        bool find = false;
        std::vector<NF_SHARE_PTR<NFServerData>> allServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_SERVER, (NF_SERVER_TYPE)serverType);
        for (int i = 0; i < (int)allServer.size(); i++)
        {
            auto pRegServerData = allServer[i];
            if (pRegServerData && NFServerIDUtil::GetZoneID(pRegServerData->mServerInfo.bus_id()) == realDestBusId)
            {
                NF_SHARE_PTR<NFServerData> pRouteAgent = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
                if (pRouteAgent)
                {
                    FindModule<NFIMessageModule>()->TransPackage(pRouteAgent->mUnlinkId, packet);
                    find = true;
                }
                else
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                               NFServerIDUtil::GetBusNameFromBusID(realDestBusId));
                }
                break;
            }
        }

        if (!find)
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "can't find destBusId, packet:{} trans failed, fromServer:{}:{} to destServer:{}, Zid:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                       realDestBusId)
        }
    }
    /**
     * @brief 跨服路由同服务器类型群发路由 (最大分区4096， 所以CROSS_ROUTE_ZONE+zoneid
     */
    else if (destBusId > CROSS_ROUTE_ZONE && destBusId < LOCAL_ALL_ROUTE)
    {
        uint32_t realDestBusId = destBusId - CROSS_ROUTE_ZONE;
        if (pConfig->GetZoneId() != realDestBusId)
        {
            if (!pConfig->IsCrossServer())
            {
                NFLogError(NF_LOG_DEFAULT, 0, "destBusId:{} zid route error, the pConfig is not cross server", destBusId);
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_NOT_SUPPORTTED;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                return 0;
            }
        }

        std::vector<NF_SHARE_PTR<NFServerData>> allServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_SERVER, NF_ST_ROUTE_AGENT_SERVER);
        for (int i = 0; i < (int)allServer.size(); i++)
        {
            auto pRouteServerData = allServer[i];
            if (pRouteServerData && NFServerIDUtil::GetZoneID(pRouteServerData->mServerInfo.bus_id()) == realDestBusId) { FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet); }
        }
    }
    else if (destBusId == LOCAL_ALL_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecRegServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_SERVER, NF_ST_ROUTE_AGENT_SERVER, pConfig->IsCrossServer());
        if (vecRegServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecRegServerData.size(); i++)
            {
                auto pRegServerData = vecRegServerData[i];
                if (pRegServerData) { FindModule<NFIMessageModule>()->TransPackage(pRegServerData->mUnlinkId, packet); }
            }
        }
    }
    else if (destBusId == CROSS_ALL_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecRegServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_SERVER, NF_ST_ROUTE_AGENT_SERVER, true);
        if (vecRegServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecRegServerData.size(); i++)
            {
                auto pRegServerData = vecRegServerData[i];
                if (pRegServerData) { FindModule<NFIMessageModule>()->TransPackage(pRegServerData->mUnlinkId, packet); }
            }
        }
    }
    else if (destBusId == LOCAL_AND_CROSS_ALL_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecRegServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_SERVER, NF_ST_ROUTE_AGENT_SERVER, pConfig->IsCrossServer());
        if (vecRegServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecRegServerData.size(); i++)
            {
                auto pRegServerData = vecRegServerData[i];
                if (pRegServerData) { FindModule<NFIMessageModule>()->TransPackage(pRegServerData->mUnlinkId, packet); }
            }
        }
    }
    else if (destBusId == ALL_LOCAL_AND_ALL_CROSS_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecRegServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_SERVER, NF_ST_ROUTE_AGENT_SERVER);
        if (vecRegServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecRegServerData.size(); i++)
            {
                auto pRegServerData = vecRegServerData[i];
                if (pRegServerData) { FindModule<NFIMessageModule>()->TransPackage(pRegServerData->mUnlinkId, packet); }
            }
        }
    }
    else
    {
        NF_SHARE_PTR<NFServerData> pRegServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, destBusId);
        if (pRegServerData)
        {
            NF_SHARE_PTR<NFServerData> pRouteAgent = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
            if (pRouteAgent) { FindModule<NFIMessageModule>()->TransPackage(pRouteAgent->mUnlinkId, packet); }
            else
            {
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                NFLogError(NF_LOG_DEFAULT, 0, "packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                           NFServerIDUtil::GetBusNameFromBusID(destBusId));
            }
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "can't find destBusId, packet:{} trans failed, fromServer:{}:{} to destServer:{}:{}", packet.ToString(), GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                       NFServerIDUtil::GetBusNameFromBusID(destBusId));
        }
    }

    return 0;
}

/*
	处理Master服务器链接事件
*/
int NFCRouteServerModule::OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    if (nEvent == eMsgType_CONNECTED)
    {
        std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
        NFLogDebug(NF_LOG_DEFAULT, 0, "route server connect master success!");

        if (!m_pObjPluginManager->IsInited(NF_ST_ROUTE_SERVER)) { RegisterMasterServer(NFrame::EST_INIT); }
        else { RegisterMasterServer(NFrame::EST_NARMAL); }

        //完成服务器启动任务
        if (!m_pObjPluginManager->IsInited(NF_ST_ROUTE_SERVER)) { FinishAppTask(NF_ST_ROUTE_SERVER, APP_INIT_CONNECT_MASTER, APP_INIT_TASK_GROUP_SERVER_CONNECT); }
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
        NFLogError(NF_LOG_DEFAULT, 0, "route server disconnect master success");
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/*
	处理Master服务器未注册协议
*/
int NFCRouteServerModule::OnHandleMasterOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
    NFLogWarning(NF_LOG_DEFAULT, 0, "master server other message not handled:packet:{},ip:{}", packet.ToString(), ip);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCRouteServerModule::RegisterMasterServer(uint32_t serverState)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);
        pData->set_server_state(serverState);

        FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_ROUTE_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCRouteServerModule::ServerReport()
{
    if (m_pObjPluginManager->IsLoadAllServer()) { return 0; }

    static uint64_t mLastReportTime = m_pObjPluginManager->GetNowTime();
    if (mLastReportTime + 100000 > m_pObjPluginManager->GetNowTime()) { return 0; }

    mLastReportTime = m_pObjPluginManager->GetNowTime();

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER);
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

        if (pData->proc_cpu() > 0 && pData->proc_mem() > 0) { FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_ROUTE_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_MASTER_SERVER_REPORT, xMsg); }
    }
    return 0;
}

int NFCRouteServerModule::OnServerRegisterProcess(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport& xData = xMsg.server_list(i);
        switch (xData.server_type())
        {
            case NF_SERVER_TYPE::NF_ST_ROUTE_AGENT_SERVER: { OnHandleRouteAgentRegister(xData, unLinkId); }
            break;
            default:
                break;
        }
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCRouteServerModule::OnHandleServerReport(uint64_t unLinkId, NFDataPackage& packet) { return 0; }

int NFCRouteServerModule::OnHandleServerRegisterRouteAgent(uint64_t unLinkId, NFDataPackage& packet)
{
    //    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_NO_PRINTF(packet, xMsg);

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER);
    CHECK_NULL(0, pConfig);

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_ROUTE_SERVER, unLinkId);
    CHECK_EXPR(pServerData != NULL, -1, "pServerData == NULL");
    CHECK_EXPR(pServerData->mServerInfo.server_type() == NF_ST_ROUTE_AGENT_SERVER, -1, "pServerData server type error");

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport& xData = xMsg.server_list(i);
        NF_SHARE_PTR<NFServerData> pRegServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, xData.bus_id());
        if (pRegServerData)
        {
            if (pRegServerData->mRouteAgentBusId == pServerData->mServerInfo.bus_id()) { pRegServerData->mServerInfo = xData; }
            else
            {
                NF_SHARE_PTR<NFServerData> pOldServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, pRegServerData->mRouteAgentBusId);
                if (pOldServerData)
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "{}({}) has register {}({}), now register {}({})", xData.server_name(), xData.bus_id(), pOldServerData->mServerInfo.server_name(), pOldServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id());
                }
                else
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "{}({}) has register {}(can't find), now register {}({})", xData.server_name(), xData.bus_id(), pRegServerData->mRouteAgentBusId, pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id());
                }

                pRegServerData->mRouteAgentBusId = pServerData->mServerInfo.bus_id();
                pRegServerData->mServerInfo = xData;
            }
        }
        else
        {
            pRegServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_ROUTE_SERVER, xData.bus_id(), (NF_SERVER_TYPE)xData.server_type(), xData);
            pRegServerData->mRouteAgentBusId = pServerData->mServerInfo.bus_id();
            pRegServerData->mServerInfo = xData;

            NFLogInfo(NF_LOG_DEFAULT, 0, "{}({}) register route agent:{}({}) trans to route svr:{}({}) success", pRegServerData->mServerInfo.server_name(), pRegServerData->mServerInfo.server_id(), pServerData->mServerInfo.server_name(), pServerData->mServerInfo.server_id(), pConfig->ServerName,
                      pConfig->ServerId);
        }
    }

    //    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理路由代理服务器注册 - 路由服务器注册处理
 *
 * 处理来自路由代理服务器的注册请求，创建路由代理连接并维护路由表。
 * 这是路由服务器的核心注册函数，负责管理所有连接的路由代理。
 *
 * @param xData 服务器信息报告，包含路由代理的详细信息
 * @param unlinkId 连接ID，标识注册路由代理的连接
 *
 * @details 处理流程：
 * 1. 验证服务器类型是否为路由代理服务器
 * 2. 检查路由代理是否已存在，如果不存在则创建新的服务器数据
 * 3. 更新路由代理的连接ID和服务器信息
 * 4. 创建到路由代理的链接
 * 5. 记录详细的注册日志
 *
 * @note 重要说明：
 * - 路由代理服务器是路由服务器的直接下级
 * - 路由代理负责管理游戏服务器，路由服务器负责管理路由代理
 * - 路由服务器通过路由代理间接管理所有游戏服务器
 *
 * @return 处理结果状态码，0表示成功，-1表示失败
 */
int NFCRouteServerModule::OnHandleRouteAgentRegister(const NFrame::ServerInfoReport& xData, uint64_t unlinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(xData.server_type() == NF_ST_ROUTE_AGENT_SERVER, -1, "xData.server_type() == NF_ST_ROUTE_AGENT_SERVER");
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_SERVER);
    CHECK_NULL(0, pConfig);

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_SERVER, xData.bus_id());
    if (!pServerData) { pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_ROUTE_SERVER, xData.bus_id(), NF_ST_ROUTE_AGENT_SERVER, xData); }

    pServerData->mUnlinkId = unlinkId;
    pServerData->mServerInfo = xData;

    FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_ROUTE_SERVER, xData.bus_id(), pServerData->mUnlinkId);

    NFLogInfo(NF_LOG_DEFAULT, 0, "Route Agent Server:{}({}) Register Route Server:{}({}) Success", pServerData->mServerInfo.server_name(), pServerData->mServerInfo.server_id(), pConfig->ServerName, pConfig->ServerId);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}



