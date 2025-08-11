// -------------------------------------------------------------------------
//    @FileName         :    NFRouteAgentServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFRouteAgentServerModule
//    @Desc             :    NFShmXFrame路由代理服务器模块实现
//                          实现路由代理服务器模块的核心功能，包括路由代理和负载均衡
//                          支持服务器间通信路由、连接管理和服务器注册
//
//    @Summary          :    路由代理服务器是分布式游戏服务器架构中的关键组件
//                          负责接收来自各个游戏服务器的消息，并根据路由规则转发到目标服务器
//                          支持本服路由、跨服路由、区服路由等多种路由模式
//                          提供负载均衡、故障转移、连接管理等功能
//
//    @Architecture     :    1. 连接管理：管理与Master服务器、Route服务器、游戏服务器的连接
//                          2. 消息路由：根据目标BusId进行智能路由分发
//                          3. 负载均衡：支持随机、轮询等负载均衡策略
//                          4. 故障处理：检测服务器断开，更新路由表
//                          5. 注册管理：处理服务器注册，维护服务器列表
//
//    @Routing Rules    :    1. LOCAL_ROUTE: 本服路由，发送到同类型服务器的随机一个
//                          2. LOCAL_ROUTE+index: 本服索引路由，发送到指定索引的服务器
//                          3. CROSS_ROUTE: 跨服路由，发送到跨服服务器的随机一个
//                          4. CROSS_ROUTE+index: 跨服索引路由，发送到指定索引的跨服服务器
//                          5. LOCAL_ROUTE_ZONE+zoneId: 区服路由，发送到指定区服的服务器
//                          6. CROSS_ROUTE_ZONE+zoneId: 跨服区服路由，群发到指定区服的所有服务器
//                          7. LOCAL_ALL_ROUTE: 本服群发路由，发送到所有本服同类型服务器
//                          8. CROSS_ALL_ROUTE: 跨服群发路由，发送到所有跨服同类型服务器
//
//    @Error Handling   :    1. 目标服务器不存在时返回ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST
//                          2. 不支持的路由类型返回ERR_CODE_ROUTER_NOT_SUPPORTTED
//                          3. 连接断开时更新服务器状态为EST_CRASH
//                          4. 路由失败时记录详细错误日志
//
//    @Performance      :    1. 使用非固定帧模式，提高处理速度
//                          2. 定时器机制，定期注册服务器信息到路由服务器
//                          3. 连接池管理，复用连接资源
//                          4. 异步消息处理，避免阻塞
// -------------------------------------------------------------------------

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFRouteAgentServerModule.h"

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFCore/NFServerIDUtil.h"

#define ROUTEAGENT_SERVER_CONNECT_MASTER_SERVER "RouteAgentServer Connect MasterServer"
#define ROUTEAGENT_SERVER_CONNECT_ROUTE_SERVER "RouteAgentServer Connect RouteServer"

/**
 * @brief 构造函数
 *
 * 初始化路由代理服务器模块，设置插件管理器
 *
 * @param p 插件管理器指针
 */
NFCRouteAgentServerModule::NFCRouteAgentServerModule(NFIPluginManager* p) : NFIRouteAgentServerModule(p)
{
}

/**
 * @brief 析构函数
 *
 * 清理路由代理服务器模块资源
 */
NFCRouteAgentServerModule::~NFCRouteAgentServerModule()
{
}

/**
 * @brief 模块唤醒 - 路由代理服务器模块初始化
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 设置非固定帧模式，提高处理速度
 * - 注册消息回调，处理服务器注册和报告消息
 * - 注册应用任务，包括连接Master服务器和Route服务器
 * - 绑定服务器，监听客户端连接
 * - 设置定时器和订阅事件，定期注册服务器信息
 *
 * @details 初始化流程：
 * 1. 设置非固定帧模式，避免帧率限制
 * 2. 注册NF_SERVER_TO_SERVER_REGISTER消息回调，处理服务器注册
 * 3. 注册NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER消息回调，处理服务器报告
 * 4. 注册应用任务：连接Master服务器和Route服务器
 * 5. 绑定服务器端口，设置连接事件回调
 * 6. 设置60秒定时器，定期注册服务器信息到路由服务器
 * 7. 订阅服务器死亡和初始化完成事件
 *
 * @return 初始化结果状态码，0表示成功，-1表示失败
 */
int NFCRouteAgentServerModule::Awake()
{
    // 设置非固定帧模式，提高处理速度，避免帧率限制
    m_pObjPluginManager->SetFixedFrame(false);

    // 注册消息回调，处理服务器注册和报告消息
    /////////////master msg/////////////////////////////
    // 注册服务器注册消息回调
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_ROUTE_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, this, &NFCRouteAgentServerModule::OnServerRegisterProcess);
    // 注册服务器报告消息回调
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_ROUTE_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, this, &NFCRouteAgentServerModule::OnHandleServerReport);

    // 注册要完成的服务器启动任务
    RegisterAppTask(NF_ST_ROUTE_AGENT_SERVER, APP_INIT_CONNECT_MASTER, ROUTEAGENT_SERVER_CONNECT_MASTER_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);
    RegisterAppTask(NF_ST_ROUTE_AGENT_SERVER, APP_INIT_CONNECT_ROUTE_SERVER, ROUTEAGENT_SERVER_CONNECT_ROUTE_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT);

    // 获取服务器配置信息
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
    if (pConfig)
    {
        // 设置空闲睡眠时间，优化CPU使用
        m_pObjPluginManager->SetIdleSleepUs(pConfig->IdleSleepUS);
        // 绑定服务器端口，监听客户端连接
        uint64_t unlinkId = FindModule<NFIMessageModule>()->BindServer(NF_ST_ROUTE_AGENT_SERVER, pConfig->Url, pConfig->NetThreadNum, pConfig->MaxConnectNum, PACKET_PARSE_TYPE_INTERNAL);
        if (unlinkId > 0)
        {
            // 注册客户端事件回调
            uint64_t routeAgentServerLinkId = unlinkId;
            FindModule<NFIMessageModule>()->SetServerLinkId(NF_ST_ROUTE_AGENT_SERVER, routeAgentServerLinkId);
            // 设置连接事件回调
            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_ROUTE_AGENT_SERVER, routeAgentServerLinkId, this, &NFCRouteAgentServerModule::OnRouteAgentSocketEvent);
            // 设置消息处理回调
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_ROUTE_AGENT_SERVER, routeAgentServerLinkId, this, &NFCRouteAgentServerModule::OnHandleOtherMessage);
            // 记录绑定成功的日志
            NFLogInfo(NF_LOG_DEFAULT, 0, "route agent server listen success, serverId:{}, ip:{}, port:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
        }
        else
        {
            // 记录绑定失败的日志
            NFLogInfo(NF_LOG_DEFAULT, 0, "route agent listen failed, serverId:{}, ip:{}, port:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
            return -1;
        }
    }
    else
    {
        // 配置获取失败，记录错误日志
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the Game Server config!");
        return -1;
    }

    // 设置60秒定时器，定期注册服务器信息到路由服务器
    SetTimer(1, 60000);
    // 订阅服务器死亡事件
    Subscribe(NF_ST_ROUTE_AGENT_SERVER, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);
    // 订阅服务器初始化完成事件
    Subscribe(NF_ST_ROUTE_AGENT_SERVER, NFrame::NF_EVENT_SERVER_APP_FINISH_INITED, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);
    return 0; // 返回成功状态码
}

/**
 * @brief 连接Master服务器
 *
 * 连接到Master服务器，设置连接回调
 *
 * @param xData 服务器信息报告
 * @return 连接结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::ConnectMasterServer(const NFrame::ServerInfoReport& xData)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
    if (pConfig)
    {
        auto pMasterServerData = FindModule<NFIMessageModule>()->GetMasterData(NF_ST_ROUTE_AGENT_SERVER);
        if (pMasterServerData->mUnlinkId <= 0)
        {
            pMasterServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_ROUTE_AGENT_SERVER, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_ROUTE_AGENT_SERVER, pMasterServerData->mUnlinkId, this, &NFCRouteAgentServerModule::OnMasterSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_ROUTE_AGENT_SERVER, pMasterServerData->mUnlinkId, this, &NFCRouteAgentServerModule::OnHandleMasterOtherMessage);
        }

        pMasterServerData->mServerInfo = xData;
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the Route Agent Server config!");
        return -1;
    }

    return 0;
}

/**
 * @brief 模块初始化
 *
 * 初始化路由代理服务器模块，包括连接Master服务器等
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::Init()
{
#if NF_PLATFORM == NF_PLATFORM_WIN
    NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(NF_ST_ROUTE_AGENT_SERVER);
    int32_t ret = ConnectMasterServer(masterData);
    CHECK_ERR(0, ret, "ConnectMasterServer Failed, url:{}", masterData.DebugString());
#else
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER); if (pConfig && pConfig->RouteConfig.NamingHost.empty())
    {
        NFrame::ServerInfoReport masterData = FindModule<NFIConfigModule>()->GetDefaultMasterInfo(NF_ST_ROUTE_AGENT_SERVER);
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
int NFCRouteAgentServerModule::Tick()
{
    ServerReport();
    return 0;
}

/**
 * @brief 动态插件处理
 *
 * 处理动态插件的加载和卸载，关闭所有连接
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_ROUTE_AGENT_SERVER);
    return 0;
}

/**
 * @brief 定时器回调
 *
 * 处理定时器事件，包括注册服务器信息到路由服务器
 *
 * @param nTimerID 定时器ID
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnTimer(uint32_t nTimerID)
{
    RegisterAllServerInfoToRouteSvr();

    if (nTimerID == 10000)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "kill the exe..................");
        NFSLEEP(1000);
        exit(0);
    }
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
int NFCRouteAgentServerModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage)
{
    CHECK_EXPR(serverType == NF_ST_ROUTE_AGENT_SERVER, -1, "");
    if (bySrcType == NFrame::NF_EVENT_SERVER_TYPE)
    {
        if (nEventID == NFrame::NF_EVENT_SERVER_DEAD_EVENT) { SetTimer(10000, 10000, 0); }
        else if (nEventID == NFrame::NF_EVENT_SERVER_APP_FINISH_INITED) { RegisterMasterServer(NFrame::EST_NARMAL); }
    }
    return 0;
}

/**
 * @brief 处理路由代理Socket事件
 *
 * 处理路由代理相关的Socket连接事件
 *
 * @param nEvent 事件类型
 * @param unLinkId 连接ID
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnRouteAgentSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    if (nEvent == eMsgType_CONNECTED)
    {
    }
    else if (nEvent == eMsgType_DISCONNECTED) { OnHandleServerDisconnect(unLinkId); }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理路由代理服务器的其他消息 - 核心路由分发函数
 *
 * 这是路由代理服务器的核心消息处理函数，负责根据目标BusId进行智能路由分发。
 * 支持多种路由模式：本服路由、跨服路由、区服路由、群发路由等。
 *
 * @param unLinkId 源连接ID，标识消息来源
 * @param packet 数据包，包含源目标信息和消息内容
 *
 * @details 路由规则说明：
 * 1. LOCAL_ROUTE: 本服路由，发送到同类型服务器的随机一个
 * 2. LOCAL_ROUTE+index: 本服索引路由，发送到指定索引的服务器
 * 3. CROSS_ROUTE: 跨服路由，发送到跨服服务器的随机一个
 * 4. CROSS_ROUTE+index: 跨服索引路由，发送到指定索引的跨服服务器
 * 5. LOCAL_ROUTE_ZONE+zoneId: 区服路由，发送到指定区服的服务器
 * 6. CROSS_ROUTE_ZONE+zoneId: 跨服区服路由，群发到指定区服的所有服务器
 * 7. LOCAL_ALL_ROUTE: 本服群发路由，发送到所有本服同类型服务器
 * 8. CROSS_ALL_ROUTE: 跨服群发路由，发送到所有跨服同类型服务器
 *
 * @details 处理流程：
 * 1. 解析源和目标BusId，确定路由类型
 * 2. 根据路由类型选择目标服务器
 * 3. 如果目标服务器不存在，转发到路由服务器
 * 4. 记录详细的转发日志
 * 5. 处理错误情况，返回相应的错误码
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnHandleOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    // 解析源服务器信息：从数据包中提取源服务器的BusId和服务器类型
    uint32_t fromBusId = GetBusIdFromUnlinkId(packet.nSrcId); // 源服务器BusId
    uint32_t fromServerType = GetServerTypeFromUnlinkId(packet.nSrcId); // 源服务器类型

    // 解析目标服务器信息：从数据包中提取目标服务器的类型和BusId
    uint32_t serverType = GetServerTypeFromUnlinkId(packet.nDstId); // 目标服务器类型
    uint32_t destBusId = GetBusIdFromUnlinkId(packet.nDstId); // 目标服务器BusId

    // 获取当前路由代理服务器的配置信息
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
    CHECK_EXPR(pConfig != NULL, -1, "pConfig == NULL");

    // 如果数据包已经包含目标服务器不存在的错误码，直接返回，避免重复处理
    if (packet.nErrCode == NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST) { return 0; }

    // 根据目标BusId范围选择不同的日志格式
    if (destBusId <= LOCAL_AND_CROSS_MAX)
    {
        // 标准路由：记录源服务器到目标服务器的消息转发日志
        NFLogInfo(NF_LOG_DEFAULT, 0, "--{}:{} trans msg from {}:{} to {}:{}, packet:{} --", pConfig->ServerName, pConfig->ServerId, GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType), destBusId, packet.ToString());
    }
    else
    {
        // 高级路由（区服路由等）：记录源服务器到目标服务器的消息转发日志，使用BusName格式
        NFLogInfo(NF_LOG_DEFAULT, 0, "--{}:{} trans msg from {}:{} to {}:{}, packet:{} --", pConfig->ServerName, pConfig->ServerId, GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType),
                  NFServerIDUtil::GetBusNameFromBusID(destBusId), packet.ToString());
    }

    /**
     * @brief 本服路由处理 - LOCAL_ROUTE
     *
     * 当目标busId==LOCAL_ROUTE时，采用本服路由机制。
     * 本服路由机制：除非明确表示要发往跨服服务器，否则就是本服路由
     * (包括跨服服务器的本服路由)
     *
     * 重要约束：需要保证本跨服服务器只能连接跨服route agent，
     * 不跨服服务器只能连接不跨服的route agent
     *
     * LOCAL_ROUTE 只发送到一个目标服务器
     *
     * @details 处理逻辑：
     * 1. 根据服务器类型和跨服标志获取随机服务器
     * 2. 如果找到目标服务器，直接转发消息
     * 3. 如果未找到目标服务器，转发到路由服务器
     * 4. 记录详细的转发日志
     */
    // 本服路由处理：目标BusId为LOCAL_ROUTE(0)时，发送到本服务器内的同类型服务器随机一个
    if (destBusId == LOCAL_ROUTE)
    {
        // 根据服务器类型和跨服标志获取随机服务器（本服路由包含跨服服务器的本服路由）
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
        if (pServerData)
        {
            // 找到目标服务器，直接转发消息
            packet.nSrcId = fromBusId; // 设置源服务器BusId
            packet.nDstId = destBusId; // 设置目标服务器BusId
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet); // 转发消息到目标服务器
        }
        else
        {
            // 未找到目标服务器，转发到路由服务器处理
            // 根据当前路由代理的跨服标志选择对应的路由服务器
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, pConfig->IsCrossServer());
            if (pRouteServerData == NULL)
            {
                // 连路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到路由服务器
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
    }
    /**
     * @brief 本服索引路由  LOCAL_ROUTE+index       本服路由机制，除非明确表示要发往跨服服务器，否则就是本服路由(包过跨服服务器的本服路由) (需要保证本跨服服务器只能连接跨服route agent， 不跨服服务器只能连接不跨服的route agent.)
     */
    // 本服索引路由处理：目标BusId在LOCAL_ROUTE和CROSS_ROUTE之间，发送到指定索引的服务器
    else if (destBusId > LOCAL_ROUTE && destBusId < CROSS_ROUTE)
    {
        // 计算服务器索引：从目标BusId中减去LOCAL_ROUTE得到索引值
        uint32_t index = destBusId - LOCAL_ROUTE;
        // 根据世界ID、区服ID、服务器类型和索引生成真实的服务器BusId
        uint32_t realDestBusId = NFServerIDUtil::MakeProcID(pConfig->GetWorldId(), pConfig->GetZoneId(), serverType, index);
        // 根据真实BusId查找目标服务器
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, realDestBusId);
        if (pServerData)
        {
            // 找到目标服务器，直接转发消息
            packet.nSrcId = fromBusId; // 设置源服务器BusId
            packet.nDstId = realDestBusId; // 设置目标服务器真实BusId
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet); // 转发消息到目标服务器
        }
        else
        {
            // 未找到目标服务器，转发到路由服务器处理
            // 根据当前路由代理的跨服标志选择对应的路由服务器
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, pConfig->IsCrossServer());
            if (pRouteServerData == NULL)
            {
                // 连路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到路由服务器
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
    }
    /**
     * @brief 跨服路由(明确指定要找跨服服务器， 才走跨服路由)
     */
    // 跨服路由处理：目标BusId为CROSS_ROUTE(10000)时，发送到跨服服务器的随机一个
    else if (destBusId == CROSS_ROUTE)
    {
        // 获取跨服服务器的随机一个（明确指定要找跨服服务器）
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, true);
        if (pServerData)
        {
            // 找到跨服服务器，直接转发消息
            packet.nSrcId = fromBusId; // 设置源服务器BusId
            packet.nDstId = destBusId; // 设置目标服务器BusId
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet); // 转发消息到跨服服务器
        }
        else
        {
            // 未找到跨服服务器，转发到跨服路由服务器处理
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
            if (pRouteServerData == NULL)
            {
                // 连跨服路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到跨服路由服务器
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
    }
    // 跨服索引路由处理：目标BusId在CROSS_ROUTE和LOCAL_ROUTE_ZONE之间，发送到指定索引的跨服服务器
    else if (destBusId > CROSS_ROUTE && destBusId < LOCAL_ROUTE_ZONE)
    {
        // 计算服务器索引：从目标BusId中减去CROSS_ROUTE得到索引值
        uint32_t index = destBusId - CROSS_ROUTE;
        // 只有跨服服务器才能处理跨服索引路由
        if (pConfig->IsCrossServer())
        {
            // 根据世界ID、区服ID、服务器类型和索引生成真实的跨服服务器BusId
            uint32_t realDestBusId = NFServerIDUtil::MakeProcID(pConfig->GetWorldId(), pConfig->GetZoneId(), serverType, index);
            // 根据真实BusId查找目标跨服服务器
            NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, realDestBusId);
            if (pServerData)
            {
                // 找到目标跨服服务器，直接转发消息
                packet.nSrcId = fromBusId; // 设置源服务器BusId
                packet.nDstId = realDestBusId; // 设置目标服务器真实BusId
                FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet); // 转发消息到目标跨服服务器
            }
            else
            {
                // 未找到目标跨服服务器，转发到跨服路由服务器处理
                auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
                if (pRouteServerData == NULL)
                {
                    // 连跨服路由服务器都找不到，返回错误码给源服务器
                    packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                    FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                }
                CHECK_NULL(0, pRouteServerData);
                // 转发消息到跨服路由服务器
                FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
            }
        }
        else
        {
            // 非跨服服务器无法处理跨服索引路由，直接转发到跨服路由服务器
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
            if (pRouteServerData == NULL)
            {
                // 连跨服路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到跨服路由服务器
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
    }
    /**
     * @brief 区服路由  区服的zid(1-4096) 只有跨服route server服务器，才有区服路由的能力
     */
    // 区服路由处理：目标BusId在LOCAL_ROUTE_ZONE和CROSS_ROUTE_ZONE之间，发送到指定区服的服务器
    else if (destBusId > LOCAL_ROUTE_ZONE && destBusId < CROSS_ROUTE_ZONE)
    {
        // 计算目标区服ID：从目标BusId中减去LOCAL_ROUTE_ZONE得到区服ID
        uint32_t realDestBusId = destBusId - LOCAL_ROUTE_ZONE;
        // 检查当前路由代理是否在目标区服内
        if (pConfig->GetZoneId() == realDestBusId)
        {
            // 当前路由代理在目标区服内，执行本服路由（包含跨服服务器的本服路由）
            NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
            if (pServerData)
            {
                // 找到目标服务器，直接转发消息
                packet.nSrcId = fromBusId; // 设置源服务器BusId
                packet.nDstId = realDestBusId; // 设置目标区服ID
                FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet); // 转发消息到目标服务器
            }
            else
            {
                // 未找到目标服务器，转发到路由服务器处理
                // 根据当前路由代理的跨服标志选择对应的路由服务器
                auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, pConfig->IsCrossServer());
                if (pRouteServerData == NULL)
                {
                    // 连路由服务器都找不到，返回错误码给源服务器
                    packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                    FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                }
                CHECK_NULL(0, pRouteServerData);
                // 转发消息到路由服务器
                FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
            }
        }
        else
        {
            // 当前路由代理不在目标区服内，需要跨服路由
            // 只有跨服服务器才有区服路由能力
            if (!pConfig->IsCrossServer())
            {
                // 非跨服服务器无法处理区服路由，记录错误并返回不支持错误码
                NFLogError(NF_LOG_DEFAULT, 0, "destBusId:{} zid route error, the pConfig is not cross server", destBusId);
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_NOT_SUPPORTTED;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                return 0;
            }

            // 跨服服务器转发到跨服路由服务器处理
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
            if (pRouteServerData == NULL)
            {
                // 连跨服路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到跨服路由服务器
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
    }
    /**
     * @brief 跨服路由同服务器类型群发路由 (最大分区4096， 所以10000+zoneid, 10001-14096之间)
     */
    // 跨服区服路由处理：目标BusId在CROSS_ROUTE_ZONE和LOCAL_ALL_ROUTE之间，群发到指定区服的所有服务器
    else if (destBusId > CROSS_ROUTE_ZONE && destBusId < LOCAL_ALL_ROUTE)
    {
        // 计算目标区服ID：从目标BusId中减去CROSS_ROUTE_ZONE得到区服ID
        uint32_t realDestBusId = destBusId - CROSS_ROUTE_ZONE;
        // 检查当前路由代理是否在目标区服内
        if (pConfig->GetZoneId() == realDestBusId)
        {
            // 当前路由代理在目标区服内，转发到路由服务器进行群发
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, pConfig->IsCrossServer());
            if (pRouteServerData == NULL)
            {
                // 连路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到路由服务器进行群发
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
        else
        {
            // 当前路由代理不在目标区服内，需要跨服路由
            // 只有跨服服务器才有跨服区服路由能力
            if (!pConfig->IsCrossServer())
            {
                // 非跨服服务器无法处理跨服区服路由，记录错误并返回不支持错误码
                NFLogError(NF_LOG_DEFAULT, 0, "destBusId:{} zid route error, the pConfig is not cross server", destBusId);
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_NOT_SUPPORTTED;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
                return 0;
            }

            // 跨服服务器转发到跨服路由服务器处理
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
            if (pRouteServerData == NULL)
            {
                // 连跨服路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到跨服路由服务器进行群发
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
    }
    /**
    * @brief 当目标busId==LOCAL_ALL_ROUTE, 本服路由机制，除非明确表示要发往跨服服务器，否则就是本服路由(包过跨服服务器的本服路由) (需要保证本跨服服务器只能连接跨服route agent， 不跨服服务器只能连接不跨服的route agent.)
    * LOCAL_ALL_ROUTE 所有同类型的服务器都发一个
    */
    // 本服群发路由处理：目标BusId为LOCAL_ALL_ROUTE(40000)时，发送到所有本服同类型服务器
    else if (destBusId == LOCAL_ALL_ROUTE)
    {
        // 根据当前路由代理的跨服标志选择对应的路由服务器进行群发
        auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, pConfig->IsCrossServer());
        if (pRouteServerData == NULL)
        {
            // 连路由服务器都找不到，返回错误码给源服务器
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
        }
        CHECK_NULL(0, pRouteServerData);
        // 转发消息到路由服务器进行群发
        FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
    }
    /**
     * @brief 跨服路由(明确指定要找跨服服务器， 才走跨服路由)
     * CROSS_ALL_ROUTE 所有同类型的服务器都发一个
     */
    // 跨服群发路由处理：目标BusId为CROSS_ALL_ROUTE(40001)时，发送到所有跨服同类型服务器
    else if (destBusId == CROSS_ALL_ROUTE)
    {
        // 转发到跨服路由服务器进行群发
        auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
        if (pRouteServerData == NULL)
        {
            // 连跨服路由服务器都找不到，返回错误码给源服务器
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
        }
        CHECK_NULL(0, pRouteServerData);
        // 转发消息到跨服路由服务器进行群发
        FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
    }
    // 本服和跨服群发路由处理：目标BusId为LOCAL_AND_CROSS_ALL_ROUTE(40002)时，发送到本服和跨服所有同类型服务器
    else if (destBusId == LOCAL_AND_CROSS_ALL_ROUTE)
    {
        // 如果不是跨服服务器，先发送到本服路由服务器
        if (!pConfig->IsCrossServer())
        {
            // 发送到本服路由服务器
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, false);
            if (pRouteServerData == NULL)
            {
                // 连本服路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到本服路由服务器
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }

        // 发送到跨服路由服务器
        auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
        if (pRouteServerData == NULL)
        {
            // 连跨服路由服务器都找不到，返回错误码给源服务器
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
        }
        CHECK_NULL(0, pRouteServerData);
        // 转发消息到跨服路由服务器
        FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
    }
    // 所有服务器群发路由处理：目标BusId为ALL_LOCAL_AND_ALL_CROSS_ROUTE(40003)时，发送到所有服务器和跨服服务器
    else if (destBusId == ALL_LOCAL_AND_ALL_CROSS_ROUTE)
    {
        // 优先尝试发送到跨服路由服务器
        auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, true);
        if (pRouteServerData == NULL)
        {
            // 如果跨服路由服务器不可用，尝试发送到本服路由服务器
            pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, pConfig->IsCrossServer());
        }
        if (pRouteServerData == NULL)
        {
            // 连路由服务器都找不到，返回错误码给源服务器
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
        }
        CHECK_NULL(0, pRouteServerData);
        // 转发消息到路由服务器进行群发
        FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
    }
    // 默认路由处理：其他所有情况，直接根据BusId查找目标服务器
    else
    {
        // 根据目标BusId直接查找目标服务器
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, destBusId);
        if (pServerData)
        {
            // 找到目标服务器，直接转发消息
            packet.nSrcId = fromBusId; // 设置源服务器BusId
            packet.nDstId = destBusId; // 设置目标服务器BusId
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet); // 转发消息到目标服务器
        }
        else
        {
            // 未找到目标服务器，转发到路由服务器处理
            // 根据当前路由代理的跨服标志选择对应的路由服务器
            auto pRouteServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER, pConfig->IsCrossServer());
            if (pRouteServerData == NULL)
            {
                // 连路由服务器都找不到，返回错误码给源服务器
                packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
                FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            }
            CHECK_NULL(0, pRouteServerData);
            // 转发消息到路由服务器
            FindModule<NFIMessageModule>()->TransPackage(pRouteServerData->mUnlinkId, packet);
        }
    }

    return 0; // 返回成功状态码
}

/**
 * @brief 处理Master服务器Socket事件
 *
 * 处理Master服务器相关的Socket连接事件，包括连接成功和断开连接
 *
 * @param nEvent 事件类型
 * @param unLinkId 连接ID
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    if (nEvent == eMsgType_CONNECTED)
    {
        std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
        NFLogDebug(NF_LOG_DEFAULT, 0, "route agent server connect master success!");
        if (!m_pObjPluginManager->IsInited(NF_ST_ROUTE_AGENT_SERVER)) { RegisterMasterServer(NFrame::EST_INIT); }
        else { RegisterMasterServer(NFrame::EST_NARMAL); }

        //完成服务器启动任务
        if (!m_pObjPluginManager->IsInited(NF_ST_ROUTE_AGENT_SERVER)) { FinishAppTask(NF_ST_ROUTE_AGENT_SERVER, APP_INIT_CONNECT_MASTER, APP_INIT_TASK_GROUP_SERVER_CONNECT); }
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
        NFLogError(NF_LOG_DEFAULT, 0, "route agent server disconnect master success");
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理Master服务器其他消息
 *
 * 处理来自Master服务器的未注册协议消息
 *
 * @param unLinkId 连接ID
 * @param packet 数据包
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnHandleMasterOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
    NFLogWarning(NF_LOG_DEFAULT, 0, "master server other message not handled:packet:{},ip:{}", packet.ToString(), ip);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 注册到Master服务器
 *
 * 向Master服务器注册当前服务器信息
 *
 * @param serverState 服务器状态
 * @return 注册结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::RegisterMasterServer(uint32_t serverState)
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);
        pData->set_server_state(serverState);

        FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_ROUTE_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg);
    }
    return 0;
}

/**
 * @brief 服务器报告
 *
 * 向Master服务器报告当前服务器状态，包括系统信息
 *
 * @return 报告结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::ServerReport()
{
    if (m_pObjPluginManager->IsLoadAllServer()) { return 0; }

    static uint64_t mLastReportTime = m_pObjPluginManager->GetNowTime();
    if (mLastReportTime + 100000 > m_pObjPluginManager->GetNowTime()) { return 0; }

    mLastReportTime = m_pObjPluginManager->GetNowTime();

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
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

        if (pData->proc_cpu() > 0 && pData->proc_mem() > 0) { FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_ROUTE_AGENT_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_MASTER_SERVER_REPORT, xMsg); }
    }
    return 0;
}

/**
 * @brief 处理服务器注册 - 路由代理服务器注册处理
 *
 * 处理来自游戏服务器的注册请求，创建服务器连接并维护路由表。
 * 这是路由代理服务器的核心注册函数，负责管理所有连接的服务器。
 *
 * @param unLinkId 连接ID，标识注册服务器的连接
 * @param packet 数据包，包含服务器信息
 *
 * @details 处理流程：
 * 1. 解析服务器信息报告列表
 * 2. 验证跨服标志是否匹配（跨服服务器只能连接跨服路由代理）
 * 3. 检查服务器是否已存在，如果不存在则创建新的服务器数据
 * 4. 更新服务器的连接ID和服务器信息
 * 5. 创建到服务器的链接
 * 6. 发送注册响应给服务器
 * 7. 将服务器信息注册到路由服务器
 * 8. 记录详细的注册日志
 *
 * @note 重要约束：
 * - 跨服服务器只能连接跨服路由代理
 * - 非跨服服务器只能连接非跨服路由代理
 * - 每个服务器只能注册到一个路由代理
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnServerRegisterProcess(uint64_t unLinkId, NFDataPackage& packet)
{
    // 解析服务器信息报告列表
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    // 获取当前路由代理服务器配置信息
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
    CHECK_EXPR(pConfig != NULL, -1, "pConfig == NULL");

    // 遍历所有要注册的服务器
    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport& xData = xMsg.server_list(i);
        // 验证跨服标志是否匹配：跨服服务器只能连接跨服路由代理，非跨服服务器只能连接非跨服路由代理
        if (xData.is_cross_server() != pConfig->IsCrossServer())
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Server:{} Register Route Agent Server:{}() Failed, cross not match", xData.server_name(), pConfig->ServerName, pConfig->ServerId);
            continue; // 跳过不匹配的服务器，继续处理下一个
        }

        // 检查服务器是否已存在，如果不存在则创建新的服务器数据
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, xData.bus_id());
        if (pServerData == nullptr)
        {
            // 创建新的服务器数据，包含服务器类型和详细信息
            pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_ROUTE_AGENT_SERVER, xData.bus_id(), (NF_SERVER_TYPE)xData.server_type(), xData);
        }

        // 更新服务器的连接ID和服务器信息
        pServerData->mUnlinkId = unLinkId; // 设置连接ID
        pServerData->mServerInfo = xData; // 更新服务器信息
        // 创建到服务器的链接
        FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_ROUTE_AGENT_SERVER, xData.bus_id(), pServerData->mUnlinkId);
        // 记录注册成功的日志
        NFLogInfo(NF_LOG_DEFAULT, 0, "Server:{} Register Route Agent Server:{}({}) Success", xData.server_name(), pConfig->ServerName, pConfig->ServerId);

        // 发送注册响应给服务器
        NFrame::ServerInfoReportList rsp;
        FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER_RSP, rsp, 0);

        // 将服务器信息添加到响应列表中
        ::NFrame::ServerInfoReport* pReport = rsp.add_server_list();
        *pReport = xData;
        // 将服务器信息注册到路由服务器
        RegisterServerInfoToRouteSvr(rsp);
    }
    return 0;
}

int NFCRouteAgentServerModule::OnHandleServerReport(uint64_t unLinkId, NFDataPackage& packet)
{
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_NO_PRINTF(packet, xMsg);

    for (int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport& xData = xMsg.server_list(i);
        switch (xData.server_type())
        {
            case NF_SERVER_TYPE::NF_ST_ROUTE_SERVER: { OnHandleRouteServerReport(xData); }
            break;
            default:
                break;
        }
    }
    return 0;
}

int NFCRouteAgentServerModule::OnHandleRouteServerReport(const NFrame::ServerInfoReport& xData)
{
    CHECK_EXPR(xData.server_type() == NF_ST_ROUTE_SERVER, -1, "xData.server_type() == NF_ST_ROUTE_SERVER");

    auto pRouteServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, xData.bus_id());

    if (pRouteServerData == NULL)
    {
        pRouteServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_ROUTE_AGENT_SERVER, xData.bus_id(), NF_ST_ROUTE_SERVER, xData);

        pRouteServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_ROUTE_AGENT_SERVER, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
        FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_ROUTE_AGENT_SERVER, xData.bus_id(), pRouteServerData->mUnlinkId);

        FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_ROUTE_AGENT_SERVER, pRouteServerData->mUnlinkId, this, &NFCRouteAgentServerModule::OnRouteServerSocketEvent);
        FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_ROUTE_AGENT_SERVER, pRouteServerData->mUnlinkId, this, &NFCRouteAgentServerModule::OnHandleRouteOtherMessage);
    }
    else
    {
        if (pRouteServerData->mUnlinkId > 0 && pRouteServerData->mServerInfo.url() != xData.url())
        {
            NFLogWarning(NF_LOG_DEFAULT, 0, "the server:{} old url:{} changed, new url:{}", pRouteServerData->mServerInfo.server_name(), pRouteServerData->mServerInfo.url(), xData.url());
            FindModule<NFIMessageModule>()->CloseLinkId(pRouteServerData->mUnlinkId);

            pRouteServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_ROUTE_AGENT_SERVER, xData.url(), PACKET_PARSE_TYPE_INTERNAL);
            FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_ROUTE_AGENT_SERVER, xData.bus_id(), pRouteServerData->mUnlinkId);

            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_ROUTE_AGENT_SERVER, pRouteServerData->mUnlinkId, this, &NFCRouteAgentServerModule::OnRouteServerSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_ROUTE_AGENT_SERVER, pRouteServerData->mUnlinkId, this, &NFCRouteAgentServerModule::OnHandleRouteOtherMessage);
        }
    }

    pRouteServerData->mServerInfo = xData;
    return 0;
}

int NFCRouteAgentServerModule::OnRouteServerSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    if (nEvent == eMsgType_CONNECTED)
    {
        NFLogDebug(NF_LOG_DEFAULT, 0, "route agent server connect route server success!");

        RegisterRouteServer(unLinkId);

        //完成服务器启动任务
        if (!m_pObjPluginManager->IsInited(NF_ST_ROUTE_AGENT_SERVER)) { FinishAppTask(NF_ST_ROUTE_AGENT_SERVER, APP_INIT_CONNECT_ROUTE_SERVER, APP_INIT_TASK_GROUP_SERVER_CONNECT); }
    }
    else if (nEvent == eMsgType_DISCONNECTED) { NFLogError(NF_LOG_DEFAULT, 0, "route agent server disconnect route server success"); }
    return 0;
}

int NFCRouteAgentServerModule::OnHandleRouteOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    uint32_t fromBusId = GetBusIdFromUnlinkId(packet.nSrcId);
    uint32_t fromServerType = GetServerTypeFromUnlinkId(packet.nSrcId);

    uint32_t serverType = GetServerTypeFromUnlinkId(packet.nDstId);
    uint32_t destBusId = GetBusIdFromUnlinkId(packet.nDstId);

    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
    CHECK_EXPR(pConfig != NULL, -1, "pConfig == NULL");

    auto pRouteSvrServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_ROUTE_AGENT_SERVER, unLinkId);
    CHECK_EXPR(pRouteSvrServerData != NULL, -1, "pRouteSvrServerData == NULL");

    if (packet.nErrCode == NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST)
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "the trans msg failed, can't find dest server, --{}:{} trans routesvr({}:{}) msg from {}:{} to {}:{}, packet:{} --", pConfig->ServerName, pConfig->ServerId, pRouteSvrServerData->mServerInfo.server_name(), pRouteSvrServerData->mServerInfo.server_id(),
                  GetServerName((NF_SERVER_TYPE)fromServerType), NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType), NFServerIDUtil::GetBusNameFromBusID(destBusId), packet.ToString());

        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, fromBusId);
        if (pServerData)
        {
            packet.nSrcId = fromBusId;
            packet.nDstId = destBusId;
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "the trans msg failed, can't find dest server, the route agent can't find the server, busid:{}, server:{} trans packet:{} failed", destBusId, GetServerName((NF_SERVER_TYPE)serverType), packet.ToString());
        }
        return 0;
    }


    if (destBusId <= LOCAL_AND_CROSS_MAX)
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "--{}:{} trans routesvr({}:{}) msg from {}:{} to {}:{}, packet:{} --", pConfig->ServerName, pConfig->ServerId, pRouteSvrServerData->mServerInfo.server_name(), pRouteSvrServerData->mServerInfo.server_id(), GetServerName((NF_SERVER_TYPE)fromServerType),
                  NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType), destBusId, packet.ToString());
    }
    else
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "--{}:{} trans routesvr({}:{}) msg from {}:{} to {}:{}, packet:{} --", pConfig->ServerName, pConfig->ServerId, pRouteSvrServerData->mServerInfo.server_name(), pRouteSvrServerData->mServerInfo.server_id(), GetServerName((NF_SERVER_TYPE)fromServerType),
                  NFServerIDUtil::GetBusNameFromBusID(fromBusId), GetServerName((NF_SERVER_TYPE)serverType), NFServerIDUtil::GetBusNameFromBusID(destBusId), packet.ToString());
    }

    /**
    * @brief 当目标busId==LOCAL_ROUTE, 本服路由机制，除非明确表示要发往跨服服务器，否则就是本服路由(包过跨服服务器的本服路由) (需要保证本跨服服务器只能连接跨服route agent， 不跨服服务器只能连接不跨服的route agent.)
    */
    if (destBusId == LOCAL_ROUTE)
    {
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
        if (pServerData)
        {
            packet.nSrcId = fromBusId;
            packet.nDstId = destBusId;
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "the route agent can't find the server, busid:{}, server:{} trans packet:{} failed", destBusId, GetServerName((NF_SERVER_TYPE)serverType), packet.ToString());
        }
    }
    /**
     * @brief 本服索引路由  LOCAL_ROUTE+index       本服路由机制，除非明确表示要发往跨服服务器，否则就是本服路由(包过跨服服务器的本服路由) (需要保证本跨服服务器只能连接跨服route agent， 不跨服服务器只能连接不跨服的route agent.)
     */
    else if (destBusId > LOCAL_ROUTE && destBusId < CROSS_ROUTE)
    {
        uint32_t index = destBusId - LOCAL_ROUTE;
        uint32_t realDestBusId = NFServerIDUtil::MakeProcID(pConfig->GetWorldId(), pConfig->GetZoneId(), serverType, index);
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, realDestBusId);
        if (pServerData)
        {
            packet.nSrcId = fromBusId;
            packet.nDstId = realDestBusId;
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "the route agent can't find the server, busid:{}, server:{} trans packet:{} failed", realDestBusId, GetServerName((NF_SERVER_TYPE)serverType), packet.ToString());
        }
    }
    /**
     * @brief 跨服路由处理 - CROSS_ROUTE (路由代理到路由服务器)
     *
     * 当目标busId==CROSS_ROUTE时，采用跨服路由机制。
     * 跨服路由：明确指定要找跨服服务器，才走跨服路由
     *
     * @details 处理逻辑：
     * 1. 根据服务器类型获取跨服服务器的随机一个
     * 2. 如果找到跨服服务器，直接转发消息
     * 3. 如果未找到跨服服务器，返回错误码
     * 4. 记录详细的转发日志
     *
     * @note 这是路由代理到路由服务器的跨服路由处理
     */
    else if (destBusId == CROSS_ROUTE)
    {
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, true);
        if (pServerData)
        {
            packet.nSrcId = fromBusId;
            packet.nDstId = destBusId;
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "the route agent can't find the server, busid:{}, server:{} trans packet:{} failed", destBusId, GetServerName((NF_SERVER_TYPE)serverType), packet.ToString());
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
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, realDestBusId);
        if (pServerData)
        {
            packet.nSrcId = fromBusId;
            packet.nDstId = realDestBusId;
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "the route agent can't find the server, busid:{}, server:{} trans packet:{} failed", realDestBusId, GetServerName((NF_SERVER_TYPE)serverType), packet.ToString());
        }
    }
    /**
     * @brief 区服路由  区服的zid(1-4096) 只有跨服route server服务器，才有区服路由的能力
     */
    else if (destBusId > LOCAL_ROUTE_ZONE && destBusId < CROSS_ROUTE_ZONE)
    {
        uint32_t realDestBusId = destBusId - LOCAL_ROUTE_ZONE;
        CHECK_EXPR(pConfig->GetZoneId() == realDestBusId, -1, "the config zoneId:{} != destBusId:{}", pConfig->GetZoneId(), realDestBusId);
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetRandomServerByServerType(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
        if (pServerData)
        {
            packet.nSrcId = fromBusId;
            packet.nDstId = realDestBusId;
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "the route agent can't find the server, busid:{}, server:{} trans packet:{} failed", realDestBusId, GetServerName((NF_SERVER_TYPE)serverType), packet.ToString());
        }
    }
    /**
     * @brief 区服路由 跨服路由同服务器类型群发路由 (最大分区4096， 所以10000+zoneid, 10001-14096之间)
     */
    else if (destBusId > CROSS_ROUTE_ZONE && destBusId < LOCAL_ALL_ROUTE)
    {
        uint32_t realDestBusId = destBusId - CROSS_ROUTE_ZONE;
        CHECK_EXPR(pConfig->GetZoneId() == realDestBusId, -1, "the config zoneId:{} != destBusId:{}", pConfig->GetZoneId(), destBusId);
        auto vecServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
        if (vecServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecServerData.size(); i++)
            {
                auto pServerData = vecServerData[i];
                if (pServerData)
                {
                    packet.nSrcId = fromBusId;
                    packet.nDstId = realDestBusId;
                    FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
                }
            }
        }
    }
    else if (destBusId == LOCAL_ALL_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
        if (vecServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecServerData.size(); i++)
            {
                auto pServerData = vecServerData[i];
                packet.nSrcId = fromBusId;
                packet.nDstId = destBusId;
                FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
            }
        }
    }
    else if (destBusId == CROSS_ALL_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, true);
        if (vecServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecServerData.size(); i++)
            {
                auto pServerData = vecServerData[i];
                packet.nSrcId = fromBusId;
                packet.nDstId = destBusId;
                FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
            }
        }
    }
    else if (destBusId == LOCAL_AND_CROSS_ALL_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType, pConfig->IsCrossServer());
        if (vecServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecServerData.size(); i++)
            {
                auto pServerData = vecServerData[i];
                packet.nSrcId = fromBusId;
                packet.nDstId = destBusId;
                FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
            }
        }
    }
    else if (destBusId == ALL_LOCAL_AND_ALL_CROSS_ROUTE)
    {
        std::vector<NF_SHARE_PTR<NFServerData>> vecServerData = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_AGENT_SERVER, (NF_SERVER_TYPE)serverType);
        if (vecServerData.size() > 0)
        {
            for (int i = 0; i < (int)vecServerData.size(); i++)
            {
                auto pServerData = vecServerData[i];
                packet.nSrcId = fromBusId;
                packet.nDstId = destBusId;
                FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
            }
        }
    }
    else
    {
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_ROUTE_AGENT_SERVER, destBusId);
        if (pServerData)
        {
            packet.nSrcId = fromBusId;
            packet.nDstId = destBusId;
            FindModule<NFIMessageModule>()->TransPackage(pServerData->mUnlinkId, packet);
        }
        else
        {
            packet.nErrCode = NFrame::ERR_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEXIST;
            FindModule<NFIMessageModule>()->TransPackage(unLinkId, packet);
            NFLogError(NF_LOG_DEFAULT, 0, "the route agent can't find the server, busid:{}, server:{} trans packet:{} failed", destBusId, GetServerName((NF_SERVER_TYPE)serverType), packet.ToString());
        }
    }
    return 0;
}

int NFCRouteAgentServerModule::RegisterRouteServer(uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_ROUTE_AGENT_SERVER);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        NFServerCommon::WriteServerInfo(pData, pConfig);
        pData->set_server_state(NFrame::EST_NARMAL);

        FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg, 0);

        RegisterAllServerInfoToRouteSvr();
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCRouteAgentServerModule::RegisterServerInfoToRouteSvr(const NFrame::ServerInfoReportList& xData)
{
    //NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::vector<NF_SHARE_PTR<NFServerData>> vec = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_AGENT_SERVER, NF_ST_ROUTE_SERVER); //GetRouteData(NF_ST_ROUTE_AGENT_SERVER);
    for (int i = 0; i < (int)vec.size(); i++)
    {
        NF_SHARE_PTR<NFServerData> pRouteServerData = vec[i];
        if (pRouteServerData && pRouteServerData->mUnlinkId > 0) { FindModule<NFIMessageModule>()->Send(pRouteServerData->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_ROUTER_CMD_INTERNAL_C2R_REG_RAASSOCAPPSVS, xData, 0); }
    }

    //NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCRouteAgentServerModule::RegisterAllServerInfoToRouteSvr()
{
    NFrame::ServerInfoReportList xData;
    std::vector<NF_SHARE_PTR<NFServerData>> vec = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_ROUTE_AGENT_SERVER);
    for (size_t i = 0; i < vec.size(); i++)
    {
        NF_SHARE_PTR<NFServerData> pRouteServerData = vec[i];
        if (pRouteServerData)
        {
            if (pRouteServerData->mServerInfo.server_type() != NF_ST_MASTER_SERVER && pRouteServerData->mServerInfo.server_type() != NF_ST_ROUTE_SERVER && pRouteServerData->mServerInfo.server_type() != NF_ST_ROUTE_AGENT_SERVER)
            {
                auto pData = xData.add_server_list();
                *pData = pRouteServerData->mServerInfo;
            }
        }
    }
    RegisterServerInfoToRouteSvr(xData);
    return 0;
}

/**
 * @brief 处理服务器断开连接 - 路由代理服务器断开处理
 *
 * 处理服务器断开连接事件，更新服务器状态并清理连接资源。
 * 这是路由代理服务器的故障处理函数，负责处理服务器异常断开。
 *
 * @param unLinkId 断开连接的ID，标识断开的服务器连接
 *
 * @details 处理流程：
 * 1. 根据连接ID查找对应的服务器数据
 * 2. 更新服务器状态为EST_CRASH（崩溃状态）
 * 3. 清空服务器的连接ID
 * 4. 记录详细的断开日志，包括服务器名称、BusId、IP、端口等信息
 * 5. 删除服务器链接，清理连接资源
 *
 * @note 重要说明：
 * - 服务器断开后，该服务器将无法接收消息
 * - 路由表会自动更新，避免向断开的服务器发送消息
 * - 如果服务器重新连接，会重新注册并更新路由表
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCRouteAgentServerModule::OnHandleServerDisconnect(uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    // 根据连接ID查找对应的服务器数据
    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_ROUTE_AGENT_SERVER, unLinkId);
    if (pServerData)
    {
        // 更新服务器状态为崩溃状态
        pServerData->mServerInfo.set_server_state(NFrame::EST_CRASH);
        // 清空服务器的连接ID
        pServerData->mUnlinkId = 0;

        // 记录详细的断开日志，包含服务器名称、BusId、IP、端口等信息
        NFLogError(NF_LOG_DEFAULT, 0, "the {0} disconnect from route agent server, serverName:{0}, busid:{1}, serverIp:{2}, serverPort:{3}", pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_ip(), pServerData->mServerInfo.server_port());
    }

    // 删除服务器链接，清理连接资源
    FindModule<NFIMessageModule>()->DelServerLink(NF_ST_ROUTE_AGENT_SERVER, unLinkId);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0; // 返回成功状态码
}
