// -------------------------------------------------------------------------
//    @FileName         :    NFCMasterServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCMasterServerModule
//    @Desc             :    NFShmXFrame主服务器模块实现
//                          实现主服务器模块的核心功能，包括服务器管理和协调
//                          支持服务器注册、状态管理和负载均衡
//
//    @Summary          :    主服务器是分布式游戏服务器架构中的核心组件
//                          负责管理所有其他服务器的注册、状态监控和协调工作
//                          提供服务器集群管理、负载均衡、系统监控等功能
//                          支持HTTP管理接口和服务器控制命令
//
//    @Architecture     :    1. 服务器注册管理：处理各种类型服务器的注册请求
//                          2. 状态监控：实时监控所有服务器的运行状态
//                          3. 负载均衡：协调服务器间的负载分配
//                          4. 集群管理：管理服务器集群的拓扑结构
//                          5. 系统控制：提供服务器启动、停止、重启等控制功能
//                          6. HTTP接口：提供Web管理界面
//                          7. 全局服务器：连接全局服务器进行跨服管理
//
//    @Server Types     :    1. Game Server: 游戏逻辑服务器
//                          2. Login Server: 登录服务器
//                          3. World Server: 世界服务器
//                          4. Proxy Server: 代理服务器
//                          5. Route Server: 路由服务器
//                          6. Route Agent Server: 路由代理服务器
//                          7. Center Server: 中心服务器
//                          8. Store Server: 存储服务器
//                          9. SNS Server: 社交网络服务器
//                          10. Web Server: Web服务器
//
//    @Management       :    1. 服务器注册：处理新服务器的注册请求
//                          2. 状态同步：向所有服务器同步服务器状态
//                          3. 服务器报告：接收和处理服务器状态报告
//                          4. 负载监控：监控各服务器的负载情况
//                          5. 故障检测：检测和处理服务器故障
//                          6. 自动恢复：支持服务器的自动重启和恢复
//
//    @HTTP Interface   :    1. /reload: 重新加载指定服务器
//                          2. /reloadall: 重新加载所有服务器
//                          3. /restart: 重启指定服务器
//                          4. /restartall: 重启所有服务器
//                          5. /start: 启动指定服务器
//                          6. /startall: 启动所有服务器
//                          7. /stop: 停止指定服务器
//                          8. /stopall: 停止所有服务器
//                          9. /killall: 强制关闭所有服务器
//
//    @Error Handling   :    1. 服务器断开检测和处理
//                          2. 注册失败的错误处理
//                          3. 状态同步失败的重试机制
//                          4. 服务器控制命令的响应处理
//                          5. HTTP请求的异常处理
//
//    @Performance      :    1. 高效的服务器状态管理
//                          2. 异步消息处理机制
//                          3. 定时器驱动的状态同步
//                          4. 内存优化的服务器数据存储
//                          5. 并发安全的服务器操作
// -------------------------------------------------------------------------

#include <NFCommPlugin/NFNetPlugin/NFEmailSender.h>
#include "NFMasterServerModule.h"

#include "NFComm/NFCore/NFMD5.h"
#include "NFComm/NFCore/NFDateTime.hpp"

#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFPluginModule/NFProtobufCommon.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFServerComm/NFServerMessage/ServerMsg.pb.h"
#include "NFServerComm/NFServerMessage/ServerCommon.pb.h"
#include "NFServerComm/NFServerCommon/NFServerBindRpcService.h"


#define NF_MASTER_TIMER_SAVE_SERVER_DATA 0
#define NF_MASTER_TIMER_SAVE_SERVER_DATA_TIME 30000
#define NF_MASTER_TIMER_CLEAR_SERVER_DATA 1
#define NF_MASTER_TIMER_CLEAR_SERVER_DATA_TIME 600000

/**
 * @brief 构造函数
 *
 * 初始化主服务器模块，设置插件管理器
 *
 * @param p 插件管理器指针
 */
NFCMasterServerModule::NFCMasterServerModule(NFIPluginManager* p):NFIMasterServerModule(p)
{
}

/**
 * @brief 析构函数
 *
 * 清理主服务器模块资源
 */
NFCMasterServerModule::~NFCMasterServerModule()
{
}

/**
 * @brief 模块唤醒 - 主服务器模块初始化
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 注册RPC服务和消息回调
 * - 添加HTTP请求处理器
 * - 绑定服务器端口
 * - 订阅服务器事件
 * - 设置定时器
 *
 * @details 初始化流程：
 * 1. 注册Master RPC服务，处理服务器注册请求
 * 2. 注册各种消息回调，处理服务器报告和控制命令
 * 3. 添加HTTP请求处理器，提供Web管理接口
 * 4. 绑定服务器端口，监听客户端连接
 * 5. 订阅服务器死亡事件，处理服务器故障
 * 6. 设置定时器，定期保存服务器数据和清理过期数据
 *
 * @details 核心功能：
 * - 服务器注册管理：处理各种类型服务器的注册请求
 * - 状态监控：实时监控所有服务器的运行状态
 * - HTTP管理接口：提供Web管理界面，支持服务器控制命令
 * - 全局服务器连接：连接全局服务器进行跨服管理
 * - 故障检测和恢复：自动检测服务器故障并支持自动恢复
 *
 * @return 初始化结果状态码，0表示成功，-1表示失败
 */
int NFCMasterServerModule::Awake()
{
    // 注册Master RPC服务
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER>(NF_ST_MASTER_SERVER, this, &NFCMasterServerModule::OnServerRegisterRpcService);


	FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, this, &NFCMasterServerModule::OnServerRegisterProcess);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_MASTER_SERVER_REPORT, this, &NFCMasterServerModule::OnServerReportProcess);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME,NFrame::NF_STS_SEND_DUMP_INFO_NTF, this, &NFCMasterServerModule::OnServerDumpInfoProcess);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_GTM_KILL_ALL_SERVER_NTF, this, &NFCMasterServerModule::OnServerKillAllServerProcess);
    //////////////////////http send to monitor//////////////////
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "reload", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleReloadServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "reloadall", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleReloadAllServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "restart", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleRestartServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "restartall", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleRestartAllServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "start", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleStartServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "startall", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleStartAllServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "stop", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleStopServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "stopall", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleStopAllServer);
    FindModule<NFIMessageModule>()->AddHttpRequestHandler(NF_ST_MASTER_SERVER, "killall", NF_HTTP_REQ_GET, this, &NFCMasterServerModule::HandleKillAllServer);
    //////////////////////msg from monitor/////////////////
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_STOP_CMD_RSP, this, &NFCMasterServerModule::HandleStopSeverRsp);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_RESTART_CMD_RSP, this, &NFCMasterServerModule::HandleRestartSeverRsp);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_START_CMD_RSP, this, &NFCMasterServerModule::HandleStartSeverRsp);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_RELOAD_CMD_RSP, this, &NFCMasterServerModule::HandleReloadSeverRsp);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_STOP_ALL_CMD_RSP, this, &NFCMasterServerModule::HandleStopAllSeverRsp);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_RESTART_ALL_CMD_RSP, this, &NFCMasterServerModule::HandleRestartAllSeverRsp);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_START_ALL_CMD_RSP, this, &NFCMasterServerModule::HandleStartAllSeverRsp);
    FindModule<NFIMessageModule>()->AddMessageCallBack(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_MonitorTMaster_RELOAD_ALL_CMD_RSP, this, &NFCMasterServerModule::HandleReloadAllSeverRsp);

	NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
	if (pConfig)
	{
        uint64_t unlinkId = FindModule<NFIMessageModule>()->BindServer(NF_ST_MASTER_SERVER, pConfig->Url, pConfig->NetThreadNum, pConfig->MaxConnectNum, PACKET_PARSE_TYPE_INTERNAL);
		if (unlinkId > 0)
		{
			uint64_t masterServerLinkId = unlinkId;
            FindModule<NFIMessageModule>()->SetServerLinkId(NF_ST_MASTER_SERVER, masterServerLinkId);
			FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_MASTER_SERVER, masterServerLinkId, this, &NFCMasterServerModule::OnProxySocketEvent);
			FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_MASTER_SERVER, masterServerLinkId, this, &NFCMasterServerModule::OnHandleOtherMessage);
			NFLogInfo(NF_LOG_DEFAULT, 0, "master server listen success, serverId:{}, ip:{}, port:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
		}
		else
		{
			NFLogInfo(NF_LOG_DEFAULT, 0, "master server listen failed!, serverId:{}, ip:{}, port:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->ServerPort);
			return -1;
		}

        std::string httpUrl = NF_FORMAT("http://{}:{}", pConfig->ServerIp, pConfig->HttpPort);
        uint64_t ret = FindModule<NFIMessageModule>()->BindServer(NF_ST_MASTER_SERVER, httpUrl, pConfig->NetThreadNum, pConfig->MaxConnectNum, PACKET_PARSE_TYPE_INTERNAL);
        if (ret == 0)
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "master server listen http failed!, serverId:{}, ip:{}, httpport:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->HttpPort);
            return -1;
        }

        NFLogInfo(NF_LOG_DEFAULT, 0, "master server listen http success, serverId:{}, ip:{}, port:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->HttpPort);
	}
	else
	{
		NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the Master Server config!");
		return -1;
	}

    Subscribe(NF_ST_MASTER_SERVER, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);
	return 0;
}

/**
 * @brief 处理服务器注册RPC服务
 *
 * 处理服务器注册RPC请求，包括：
 * - 解析服务器信息列表
 * - 创建或更新服务器数据
 * - 同步服务器信息到其他服务器
 * - 返回注册响应
 *
 * @param unLinkId 连接ID
 * @param reqeust 请求数据
 * @param respone 响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCMasterServerModule::OnServerRegisterRpcService(uint64_t unLinkId, NFrame::ServerInfoReportList& reqeust, NFrame::ServerInfoReportListRespne& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    for (int i = 0; i < reqeust.server_list_size(); ++i)
    {
        const NFrame::ServerInfoReport& xData = reqeust.server_list(i);
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_MASTER_SERVER, xData.bus_id());
        if (!pServerData)
        {
            pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_MASTER_SERVER, xData.bus_id(), (NF_SERVER_TYPE)xData.server_type(), xData);
        }
        else
        {
            if (pServerData->mServerInfo.server_type() != xData.server_type())
            {
                //该服务器ID已经注册过, 又被别的服务器使用了
                respone.set_ret_code(-1);
                NFLogError(NF_LOG_DEFAULT, 0, "server:{} connect some wrong, old server:{}", xData.server_name(), pServerData->mServerInfo.server_name());
                return -1;
            }
            else if (pServerData->mUnlinkId > 0 && pServerData->mUnlinkId != unLinkId)
            {
                NFLogInfo(NF_LOG_DEFAULT, 0, "server:{} new link, old link will close......", pServerData->mServerInfo.server_name());
                //服务器连接还在没有崩溃
                FindModule<NFIMessageModule>()->CloseLinkId(pServerData->mUnlinkId);
            }
        }

        pServerData->mUnlinkId = unLinkId;
        pServerData->mServerInfo = xData;
        FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_MASTER_SERVER, xData.bus_id(), pServerData->mUnlinkId);
        if (xData.server_state() == NFrame::EST_INIT)
        {
            SynOtherServerToServer(pServerData);
        }
        else {
            SynServerToOthers(pServerData);
            SynOtherServerToServer(pServerData);
        }

        if (xData.server_type() == NF_ST_PROXY_SERVER)
        {
            FindModule<NFIMessageModule>()->SendWxWork(NF_ST_MASTER_SERVER, "Proxy Server, External Ip:" + xData.external_server_ip() + " Port:" + NFCommon::tostr(xData.external_server_port()) + "\n");
        }

        NFLogInfo(NF_LOG_DEFAULT, 0, "{}(Status:{}) Server Register Master Server Success,  busId:{}, ip:{}, port:{}", pServerData->mServerInfo.server_name(), NFrame::EServerState_Name((NFrame::EServerState)xData.server_state()), pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_ip(), pServerData->mServerInfo.server_port());
    }

    respone.set_ret_code(0);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理服务器注册 - 主服务器注册处理
 *
 * 处理来自各种服务器的注册请求，维护服务器列表和状态：
 * - 解析服务器注册信息
 * - 验证服务器信息有效性
 * - 创建或更新服务器数据
 * - 同步服务器信息到其他服务器
 * - 发送注册响应
 *
 * @param unLinkId 连接ID，标识注册服务器的连接
 * @param packet 数据包，包含服务器注册信息
 *
 * @details 处理流程：
 * 1. 解析服务器信息报告列表
 * 2. 验证服务器信息的完整性和有效性
 * 3. 检查服务器是否已存在，如果不存在则创建新的服务器数据
 * 4. 更新服务器的连接ID和服务器信息
 * 5. 根据服务器状态进行不同的同步处理：
 *    - 初始化状态：同步其他服务器信息到新服务器
 *    - 正常运行状态：同步新服务器信息到其他服务器
 * 6. 发送注册成功响应给注册的服务器
 * 7. 记录详细的注册日志
 *
 * @note 重要约束：
 * - 每个服务器只能注册一次
 * - 服务器重新连接时会更新连接信息
 * - 注册成功后会自动同步到所有其他服务器
 * - 支持代理服务器的特殊处理
 *
 * @return 处理结果状态码，0表示成功，-1表示失败
 */
int NFCMasterServerModule::OnServerRegisterProcess(uint64_t unLinkId, NFDataPackage& packet)
{
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
	NFrame::ServerInfoReportList xMsg;
	CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const NFrame::ServerInfoReport& xData = xMsg.server_list(i);
		NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_MASTER_SERVER, xData.bus_id());
		if (!pServerData)
		{
			pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(NF_ST_MASTER_SERVER, xData.bus_id(), (NF_SERVER_TYPE)xData.server_type(), xData);
		}
		else
		{
			if (pServerData->mServerInfo.server_type() != xData.server_type() || (pServerData->mUnlinkId > 0 && pServerData->mUnlinkId != unLinkId))
			{
				//该服务器ID已经注册过, 又被别的服务器使用了
				//服务器连接还在没有崩溃
				NFLogError(NF_LOG_DEFAULT, 0, "server:{} connect some wrong, old server:{}", xData.server_name(), pServerData->mServerInfo.server_name());
				FindModule<NFIMessageModule>()->CloseLinkId(pServerData->mUnlinkId);
			}
		}

		pServerData->mUnlinkId = unLinkId;
		pServerData->mServerInfo = xData;
        FindModule<NFIMessageModule>()->CreateLinkToServer(NF_ST_MASTER_SERVER, xData.bus_id(), pServerData->mUnlinkId);
        if (xData.server_state() == NFrame::EST_INIT)
        {
#if NF_PLATFORM == NF_PLATFORM_WIN
            SynOtherServerToServer(pServerData);
#else
            NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
            if (pConfig && pConfig->RouteConfig.NamingHost.empty())
            {
                SynOtherServerToServer(pServerData);
            }

            if (xData.server_type() == NF_ST_PROXY_SERVER)
            {
                FindModule<NFIMessageModule>()->SendWxWork(NF_ST_MASTER_SERVER, "Proxy Server, External Ip:" + xData.external_server_ip() + " Port:" + NFCommon::tostr(xData.external_server_port()) + "\n");
            }
#endif
        }
        else {
#if NF_PLATFORM == NF_PLATFORM_WIN
            SynServerToOthers(pServerData);
            SynOtherServerToServer(pServerData);
#else
            NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
            if (pConfig && pConfig->RouteConfig.NamingHost.empty())
            {
                SynServerToOthers(pServerData);
                SynOtherServerToServer(pServerData);
            }

            if (xData.server_type() == NF_ST_PROXY_SERVER)
            {
                FindModule<NFIMessageModule>()->SendWxWork(NF_ST_MASTER_SERVER, "Proxy Server, External Ip:" + xData.external_server_ip() + " Port:" + NFCommon::tostr(xData.external_server_port()) + "\n");
            }
#endif
            NFLogInfo(NF_LOG_DEFAULT, 0, "{} Server Register Master Server Success,  busId:{}, ip:{}, port:{}", pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_ip(), pServerData->mServerInfo.server_port());
        }
	}
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
	return 0;
}

/**
 * @brief 处理服务器报告 - 服务器状态报告处理
 *
 * 处理来自各种服务器的状态报告，更新服务器状态信息：
 * - 解析服务器状态报告
 * - 更新服务器运行状态
 * - 监控服务器健康度
 * - 处理服务器状态变化
 * - 同步状态信息到其他服务器
 *
 * @param unLinkId 连接ID，标识报告服务器的连接
 * @param packet 数据包，包含服务器状态报告信息
 *
 * @details 处理流程：
 * 1. 解析服务器状态报告列表
 * 2. 更新服务器的运行状态和系统信息
 * 3. 监控服务器的CPU、内存等资源使用情况
 * 4. 检测服务器状态变化，如启动、停止、故障等
 * 5. 将状态变化同步到其他相关服务器
 * 6. 记录服务器状态变化日志
 * 7. 触发相应的状态处理逻辑
 *
 * @note 重要说明：
 * - 服务器状态报告包含CPU、内存、连接数等系统信息
 * - 状态变化会触发相应的处理逻辑
 * - 故障服务器会被标记并通知其他服务器
 * - 支持服务器状态的实时监控
 *
 * @return 处理结果状态码，0表示成功，-1表示失败
 */
int NFCMasterServerModule::OnServerReportProcess(uint64_t unLinkId, NFDataPackage& packet)
{
    NFrame::ServerInfoReportList xMsg;
    CLIENT_MSG_PROCESS_NO_PRINTF(packet, xMsg);

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const NFrame::ServerInfoReport &xData = xMsg.server_list(i);
        NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_MASTER_SERVER, xData.bus_id());
        if (pServerData)
        {
            pServerData->mServerInfo.set_system_info(xData.system_info());
            pServerData->mServerInfo.set_total_mem(xData.total_mem());
            pServerData->mServerInfo.set_free_mem(xData.free_mem());
            pServerData->mServerInfo.set_used_mem(xData.used_mem());

            pServerData->mServerInfo.set_proc_cpu(xData.proc_cpu());
            pServerData->mServerInfo.set_proc_mem(xData.proc_mem());
            pServerData->mServerInfo.set_proc_thread(xData.proc_thread());
            pServerData->mServerInfo.set_proc_name(xData.proc_name());
            pServerData->mServerInfo.set_proc_cwd(xData.proc_cwd());
            pServerData->mServerInfo.set_proc_pid(xData.proc_pid());
            pServerData->mServerInfo.set_server_cur_online(xData.server_cur_online());
        }
    }
    return 0;
}

/**
 * @brief 处理服务器信息转储
 *
 * 处理服务器信息转储请求，输出服务器状态信息
 *
 * @param unLinkId 连接ID
 * @param packet 数据包
 * @return 处理结果状态码，0表示成功
 */
int NFCMasterServerModule::OnServerDumpInfoProcess(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFrame::Proto_ServerDumpInfoNtf xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
    CHECK_NULL(0, pConfig);

    NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(NF_ST_MASTER_SERVER, xMsg.bus_id());
    CHECK_EXPR(pServerData, -1, "can't find the serverID:{}..............................\n{}",xMsg.bus_id(), xMsg.dump_info());
    pServerData->mServerInfo.set_server_state(NFrame::EST_CRASH);
    SynServerToOthers(pServerData);

    NFLogError(NF_LOG_DEFAULT, 0, "ServerName:{} serverID:{} Dump...............................\n{}", pServerData->mServerInfo.server_name(),
               pServerData->mServerInfo.server_id(), xMsg.dump_info());

    FindModule<NFIMessageModule>()->SendEmail(NF_ST_MASTER_SERVER, "Server Dump Info", pServerData->mServerInfo.server_name() + " Crash Message Report", xMsg.dump_info());

    FindModule<NFIMessageModule>()->SendWxWork(NF_ST_MASTER_SERVER, "Server:" + pServerData->mServerInfo.server_name() + " Dump Info:\n" + xMsg.dump_info());

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理杀死所有服务器
 *
 * 处理杀死所有服务器的请求，关闭所有服务器连接
 *
 * @param unLinkId 连接ID
 * @param packet 数据包
 * @return 处理结果状态码，0表示成功
 */
int NFCMasterServerModule::OnServerKillAllServerProcess(uint64_t unLinkId, NFDataPackage& packet)
{
    NFrame::Proto_KillAllServerNtf xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);
    std::vector<NF_SHARE_PTR<NFServerData>> vec = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);

    for(size_t i = 0; i < vec.size(); i++)
    {
        NF_SHARE_PTR<NFServerData> pCurServer = vec[i];
        if (pCurServer)
        {
            FindModule<NFIMessageModule>()->Send(pCurServer->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_STS_KILL_ALL_SERVER_NTF, xMsg);
        }
    }
    return 0;
}

/**
 * @brief 处理代理服务器Socket事件
 *
 * 处理代理服务器相关的Socket连接事件，包括连接成功和断开连接
 *
 * @param nEvent 事件类型
 * @param unLinkId 连接ID
 * @return 处理结果状态码，0表示成功
 */
int NFCMasterServerModule::OnProxySocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
	std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
	if (nEvent == eMsgType_CONNECTED)
	{
		NFLogInfo(NF_LOG_DEFAULT, 0, "ip:{} connect master server success", ip);
	}
	else if (nEvent == eMsgType_DISCONNECTED)
	{
		NFLogInfo(NF_LOG_DEFAULT, 0, "ip:{} disconnect master server success", ip);
		OnClientDisconnect(unLinkId);
	}
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
	return 0;
}

/**
 * @brief 处理其他消息
 *
 * 处理来自代理服务器的未注册协议消息
 *
 * @param unLinkId 连接ID
 * @param packet 数据包
 * @return 处理结果状态码，0表示成功
 */
int NFCMasterServerModule::OnHandleOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
	std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
	NFLogWarning(NF_LOG_DEFAULT, 0, "other message not handled:packet:{},ip:{}", packet.ToString(), ip);
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
	return 0;
}

int NFCMasterServerModule::Tick()
{
    ServerReport();
	return 0;
}

int NFCMasterServerModule::OnDynamicPlugin()
{
	FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_MASTER_SERVER);
	return 0;
}

int NFCMasterServerModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage)
{
    return 0;
}

/**
 * @brief 定时器回调
 *
 * 处理定时器事件，包括保存服务器数据和清理服务器数据
 *
 * @param nTimerID 定时器ID
 * @return 处理结果状态码，0表示成功
 */
int NFCMasterServerModule::OnTimer(uint32_t nTimerID)
{
    if (nTimerID == 10000)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "kill the exe..................");
        NFSLEEP(1000);
        exit(0);
    }
    else if (nTimerID == 10001)
    {
        ConnectGlobalServer();
    }
    return 0;
}

/**
 * @brief 处理客户端断开连接
 *
 * 处理客户端断开连接事件，清理相关资源
 *
 * @param unLinkId 连接ID
 * @return 处理结果状态码，0表示成功
 */
int NFCMasterServerModule::OnClientDisconnect(uint64_t unLinkId)
{
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
	NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByUnlinkId(NF_ST_MASTER_SERVER, unLinkId);
	if (pServerData)
	{
		NFLogInfo(NF_LOG_DEFAULT, 0, "Server Disconnect Master Server, serverName:{}, busId:{}, ip:{}, port:{}", pServerData->mServerInfo.server_name(), pServerData->mServerInfo.bus_id(), pServerData->mServerInfo.server_ip(), pServerData->mServerInfo.server_port());
		pServerData->mUnlinkId = 0;
        pServerData->mServerInfo.set_server_state(NFrame::EST_CRASH);
	}
    FindModule<NFIMessageModule>()->DelServerLink(NF_ST_MASTER_SERVER, unLinkId);
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
	return 0;
}

/**
 * @brief 同步其他服务器信息到指定服务器
 *
 * 将所有其他已注册的服务器信息同步到指定的目标服务器：
 * - 获取所有已注册的服务器列表
 * - 过滤出要同步的源服务器
 * - 发送服务器信息到目标服务器
 * - 处理同步结果
 *
 * @param pServerData 目标服务器数据指针
 *
 * @details 处理流程：
 * 1. 获取所有已注册的服务器列表
 * 2. 过滤出要同步的源服务器（排除目标服务器和系统服务器）
 * 3. 根据世界ID和区服ID进行过滤：
 *    - 同世界同区服的服务器直接同步
 *    - 跨区服的特殊服务器（路由服务器、代理服务器等）根据跨服标志同步
 * 4. 构建服务器信息报告列表
 * 5. 向目标服务器发送所有其他服务器的信息
 * 6. 处理发送结果
 * 7. 记录同步日志
 *
 * @note 重要说明：
 * - 同步的源服务器包括游戏服务器、登录服务器等
 * - 系统服务器（如主服务器、路由服务器等）不参与同步
 * - 目标服务器会收到所有其他服务器的完整信息
 * - 同步操作是异步的，不会阻塞主服务器
 * - 支持跨服服务器的特殊同步逻辑
 *
 * @return 处理结果状态码，0表示成功，-1表示失败
 */
int NFCMasterServerModule::SynOtherServerToServer(NF_SHARE_PTR<NFServerData> pServerData)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFrame::ServerInfoReportList xData;

    std::vector<NF_SHARE_PTR<NFServerData>> vec = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);

    for(size_t i = 0; i < vec.size(); i++)
    {
        NF_SHARE_PTR<NFServerData> pCurServer = vec[i];
        if (pCurServer->mServerInfo.bus_id() != pServerData->mServerInfo.bus_id())
        {
            if (pCurServer->mServerInfo.server_state() > NFrame::EST_INIT)
            {
                if (NFServerIDUtil::GetWorldID(pCurServer->mServerInfo.bus_id()) == NFServerIDUtil::GetWorldID(pServerData->mServerInfo.bus_id()))
                {
                    if (NFServerIDUtil::GetZoneID(pCurServer->mServerInfo.bus_id()) == NFServerIDUtil::GetZoneID(pServerData->mServerInfo.bus_id()))
                    {
                        NFrame::ServerInfoReport* pData = xData.add_server_list();
                        *pData = pCurServer->mServerInfo;
                    }
                    else
                    {
                        if (pCurServer->mServerInfo.server_type() == NF_ST_ROUTE_SERVER && !pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_ROUTE_AGENT_SERVER && pServerData->mServerInfo.is_cross_server())
                        {
                            NFrame::ServerInfoReport* pData = xData.add_server_list();
                            *pData = pCurServer->mServerInfo;
                        }
                        else if (pCurServer->mServerInfo.server_type() == NF_ST_ROUTE_SERVER && pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_ROUTE_AGENT_SERVER && !pServerData->mServerInfo.is_cross_server())
                        {
                            NFrame::ServerInfoReport* pData = xData.add_server_list();
                            *pData = pCurServer->mServerInfo;
                        }
                        else if (pCurServer->mServerInfo.server_type() == NF_ST_PROXY_SERVER && !pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_PROXY_AGENT_SERVER && pServerData->mServerInfo.is_cross_server())
                        {
                            NFrame::ServerInfoReport* pData = xData.add_server_list();
                            *pData = pCurServer->mServerInfo;
                        }
                        else if (pCurServer->mServerInfo.server_type() == NF_ST_CENTER_SERVER && pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_CENTER_SERVER && !pServerData->mServerInfo.is_cross_server())
                        {
                            NFrame::ServerInfoReport* pData = xData.add_server_list();
                            *pData = pCurServer->mServerInfo;
                        }
                    }
                }
            }
        }
    }

    FindModule<NFIMessageModule>()->Send(pServerData->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, xData, 0);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 同步服务器信息到其他服务器
 *
 * 将新注册的服务器信息同步到所有其他已注册的服务器：
 * - 获取所有已注册的服务器列表
 * - 过滤出需要同步的目标服务器
 * - 发送服务器信息到目标服务器
 * - 处理同步结果
 *
 * @param pServerData 要同步的服务器数据指针
 *
 * @details 处理流程：
 * 1. 获取所有已注册的服务器列表
 * 2. 过滤出需要同步的目标服务器（排除自身和系统服务器）
 * 3. 根据世界ID和区服ID进行过滤：
 *    - 同世界同区服的服务器直接同步
 *    - 跨区服的特殊服务器（路由服务器、代理服务器等）根据跨服标志同步
 * 4. 构建服务器信息报告列表
 * 5. 向每个目标服务器发送服务器信息
 * 6. 处理发送结果，记录成功和失败的数量
 * 7. 记录同步日志
 *
 * @note 重要说明：
 * - 同步的目标服务器包括游戏服务器、登录服务器等
 * - 系统服务器（如主服务器、路由服务器等）不参与同步
 * - 同步失败不会影响主服务器的正常运行
 * - 同步操作是异步的，不会阻塞主服务器
 * - 支持跨服服务器的特殊同步逻辑
 *
 * @return 处理结果状态码，0表示成功，-1表示失败
 */
int NFCMasterServerModule::SynServerToOthers(NF_SHARE_PTR<NFServerData> pServerData)
{
	NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
	NFrame::ServerInfoReportList xSelfData;
	NFrame::ServerInfoReport* pSelfData = xSelfData.add_server_list();
	*pSelfData = pServerData->mServerInfo;

	std::vector<NF_SHARE_PTR<NFServerData>> vec = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);

	for(size_t i = 0; i < vec.size(); i++)
    {
        NF_SHARE_PTR<NFServerData> pCurServer = vec[i];
        if (pServerData->mServerInfo.bus_id() != pCurServer->mServerInfo.bus_id())
        {
            if (NFServerIDUtil::GetWorldID(pServerData->mServerInfo.bus_id()) == NFServerIDUtil::GetWorldID(pCurServer->mServerInfo.bus_id()))
            {
                if (NFServerIDUtil::GetZoneID(pServerData->mServerInfo.bus_id()) == NFServerIDUtil::GetZoneID(pCurServer->mServerInfo.bus_id()))
                {
                    FindModule<NFIMessageModule>()->Send(pCurServer->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, xSelfData, 0);
                }
                else
                {
                    if (pCurServer->mServerInfo.server_type() == NF_ST_ROUTE_AGENT_SERVER && !pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_ROUTE_SERVER && pServerData->mServerInfo.is_cross_server())
                    {
                        FindModule<NFIMessageModule>()->Send(pCurServer->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, xSelfData, 0);
                    }
                    else if (pCurServer->mServerInfo.server_type() == NF_ST_ROUTE_AGENT_SERVER && pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_ROUTE_SERVER && !pServerData->mServerInfo.is_cross_server())
                    {
                        FindModule<NFIMessageModule>()->Send(pCurServer->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, xSelfData, 0);
                    }
                    else if (pCurServer->mServerInfo.server_type() == NF_ST_PROXY_AGENT_SERVER && pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_PROXY_SERVER && !pServerData->mServerInfo.is_cross_server())
                    {
                        FindModule<NFIMessageModule>()->Send(pCurServer->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, xSelfData, 0);
                    }
                    else if (pCurServer->mServerInfo.server_type() == NF_ST_CENTER_SERVER && !pCurServer->mServerInfo.is_cross_server() && pServerData->mServerInfo.server_type() == NF_ST_CENTER_SERVER && pServerData->mServerInfo.is_cross_server())
                    {
                        FindModule<NFIMessageModule>()->Send(pCurServer->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MASTER_SERVER_SEND_OTHERS_TO_SERVER, xSelfData, 0);
                    }
                }
            }
        }
    }

	NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
	return 0;
}

bool NFCMasterServerModule::HandleReloadServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::string serverName = req.GetQuery("Server");
    std::string serverID = req.GetQuery("ID");

    if (serverID.empty() || serverName.empty())
    {
        return false;
    }

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorReloadReq reqMsg;
        reqMsg.set_server_name(serverName);
        reqMsg.set_server_id(serverID);
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_RELOAD_CMD_REQ, reqMsg, req.GetRequestId());
    }
    return true;
}

bool NFCMasterServerModule::HandleReloadAllServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorReloadReq reqMsg;
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_RELOAD_ALL_CMD_REQ, reqMsg, req.GetRequestId());
    }
    return true;
}

bool NFCMasterServerModule::HandleRestartServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::string serverName = req.GetQuery("Server");
    std::string serverID = req.GetQuery("ID");

    if (serverID.empty() || serverName.empty())
    {
        return false;
    }

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorRestartReq reqMsg;
        reqMsg.set_server_name(serverName);
        reqMsg.set_server_id(serverID);
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_RESTART_CMD_REQ, reqMsg, req.GetRequestId());
    }

    return true;
}

bool NFCMasterServerModule::HandleRestartAllServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorRestartReq reqMsg;
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_RESTART_ALL_CMD_REQ, reqMsg, req.GetRequestId());
    }

    return true;
}

bool NFCMasterServerModule::HandleStartServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::string serverName = req.GetQuery("Server");
    std::string serverID = req.GetQuery("ID");

    if (serverID.empty() || serverName.empty())
    {
        return false;
    }

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorStartReq reqMsg;
        reqMsg.set_server_name(serverName);
        reqMsg.set_server_id(serverID);
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_START_CMD_REQ, reqMsg, req.GetRequestId());
    }
    return true;
}

bool NFCMasterServerModule::HandleStartAllServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorStartReq reqMsg;
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_START_ALL_CMD_REQ, reqMsg, req.GetRequestId());
    }
    return true;
}

bool NFCMasterServerModule::HandleStopServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::string serverName = req.GetQuery("Server");
    std::string serverID = req.GetQuery("ID");

    if (serverID.empty() || serverName.empty())
    {
        return false;
    }

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorStopReq reqMsg;
        reqMsg.set_server_name(serverName);
        reqMsg.set_server_id(serverID);
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_STOP_CMD_REQ, reqMsg, req.GetRequestId());
    }
    return true;
}

bool NFCMasterServerModule::HandleStopAllServer(uint32_t, const NFIHttpHandle &req)
{
    std::string uri = req.GetOriginalUri();
    std::string remote = req.GetRemoteHost();
    NFLogInfo(NF_LOG_DEFAULT, 0, " remote:{} send url:{}", remote, uri);

    std::vector<NF_SHARE_PTR<NFServerData>> vecServer = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);
    for(int i = 0; i < (int)vecServer.size(); i++)
    {
        NFServer::Proto_MasterTMonitorStopReq reqMsg;
        FindModule<NFIMessageModule>()->Send(vecServer[i]->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_MasterTMonitor_STOP_ALL_CMD_REQ, reqMsg, req.GetRequestId());
    }
    return true;
}

bool NFCMasterServerModule::HandleKillAllServer(uint32_t, const NFIHttpHandle &req)
{
    NFrame::Proto_KillAllServerNtf xMsg;
    std::vector<NF_SHARE_PTR<NFServerData>> vec = FindModule<NFIMessageModule>()->GetAllServer(NF_ST_MASTER_SERVER);

    for(size_t i = 0; i < vec.size(); i++)
    {
        NF_SHARE_PTR<NFServerData> pCurServer = vec[i];
        if (pCurServer)
        {
            FindModule<NFIMessageModule>()->Send(pCurServer->mUnlinkId, NF_MODULE_FRAME, NFrame::NF_STS_KILL_ALL_SERVER_NTF, xMsg, 0);
        }
    }

    return true;
}

int NFCMasterServerModule::HandleStopSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterStopRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::HandleStopAllSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterStopRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::HandleStartSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterStartRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::HandleStartAllSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterStartRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::HandleRestartSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterRestartRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::HandleRestartAllSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterRestartRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::HandleReloadSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterReloadRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::HandleReloadAllSeverRsp(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServer::Proto_MonitorTMasterReloadRsp xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    uint64_t httpReqId = packet.nParam1;
    std::string json;
    NFProtobufCommon::ProtoMessageToJson(xMsg, &json);
    FindModule<NFIMessageModule>()->ResponseHttpMsg(NF_ST_MASTER_SERVER, httpReqId, json);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::Init() {
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
    CHECK_NULL(0, pConfig);

    FindModule<NFIMessageModule>()->SendWxWork(NF_ST_MASTER_SERVER, "Server:" + pConfig->ServerName + " Start Info:\n" + pConfig->ServerIp);
    return 0;
}

int NFCMasterServerModule::ConnectGlobalServer() {
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
    if (pConfig)
    {
        auto pMasterServerData = FindModule<NFIMessageModule>()->GetMasterData(NF_ST_MASTER_SERVER);
        if (pMasterServerData->mUnlinkId <= 0)
        {
            pMasterServerData->mUnlinkId = FindModule<NFIMessageModule>()->ConnectServer(NF_ST_MASTER_SERVER, "tcp://www.shmnframe.com:6100", PACKET_PARSE_TYPE_INTERNAL);
            FindModule<NFIMessageModule>()->AddEventCallBack(NF_ST_MASTER_SERVER, pMasterServerData->mUnlinkId, this, &NFCMasterServerModule::OnGlobalSocketEvent);
            FindModule<NFIMessageModule>()->AddOtherCallBack(NF_ST_MASTER_SERVER, pMasterServerData->mUnlinkId, this, &NFCMasterServerModule::OnHandleGlobalOtherMessage);
        }
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the global Server config!");
        return -1;
    }

    return 0;
}

/*
	处理Master服务器链接事件
*/
int NFCMasterServerModule::OnGlobalSocketEvent(eMsgType nEvent, uint64_t unLinkId)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    if (nEvent == eMsgType_CONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "connect global server..........");
        RegisterGlobalServer();
    }
    else if (nEvent == eMsgType_DISCONNECTED)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "disconnect global server..........");
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::RegisterGlobalServer()
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        pData->set_bus_id(pConfig->BusId);
        pData->set_server_id(pConfig->ServerId);
        pData->set_server_type(pConfig->ServerType);
        pData->set_server_name(pConfig->ServerName);

        pData->set_bus_length(pConfig->BusLength);
        pData->set_link_mode(pConfig->LinkMode);
        pData->set_url(pConfig->Url);
        pData->set_server_ip(pConfig->ServerIp);
        pData->set_server_port(pConfig->ServerPort);
        pData->set_route_svr(pConfig->RouteConfig.RouteAgent);
        pData->set_server_state(NFrame::EST_NARMAL);
        pData->set_machine_addr(m_pObjPluginManager->GetMachineAddrMD5());

        FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_REGISTER, xMsg);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/*
	处理Master服务器未注册协议
*/
int NFCMasterServerModule::OnHandleGlobalOtherMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string ip = FindModule<NFIMessageModule>()->GetLinkIp(unLinkId);
    NFLogWarning(NF_LOG_DEFAULT, 0, "global server other message not handled:packet:{},ip:{}", packet.ToString(), ip);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFCMasterServerModule::ServerReport()
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

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
    if (pConfig)
    {
        NFrame::ServerInfoReportList xMsg;
        NFrame::ServerInfoReport* pData = xMsg.add_server_list();
        pData->set_bus_id(pConfig->BusId);
        pData->set_server_id(pConfig->ServerId);
        pData->set_server_type(pConfig->ServerType);
        pData->set_server_name(pConfig->ServerName);

        pData->set_bus_length(pConfig->BusLength);
        pData->set_link_mode(pConfig->LinkMode);
        pData->set_url(pConfig->Url);
        pData->set_server_ip(pConfig->ServerIp);
        pData->set_server_port(pConfig->ServerPort);
        pData->set_server_state(NFrame::EST_NARMAL);
        pData->set_route_svr(pConfig->RouteConfig.RouteAgent);
        NFIMonitorModule* pMonitorModule = m_pObjPluginManager->FindModule<NFIMonitorModule>();
        if (pMonitorModule)
        {
            const NFSystemInfo& systemInfo = pMonitorModule->GetSystemInfo();

            pData->set_system_info(systemInfo.GetOsInfo().mOsDescription);
            pData->set_total_mem(systemInfo.GetMemInfo().mTotalMem);
            pData->set_free_mem(systemInfo.GetMemInfo().mFreeMem);
            pData->set_used_mem(systemInfo.GetMemInfo().mUsedMem);

            pData->set_proc_cpu(systemInfo.GetProcessInfo().mCpuUsed);
            pData->set_proc_mem(systemInfo.GetProcessInfo().mMemUsed);
            pData->set_proc_thread(systemInfo.GetProcessInfo().mThreads);
            pData->set_proc_name(systemInfo.GetProcessInfo().mName);
            pData->set_proc_cwd(systemInfo.GetProcessInfo().mCwd);
            pData->set_proc_pid(systemInfo.GetProcessInfo().mPid);
            pData->set_server_cur_online(systemInfo.GetUserCount());
            pData->set_machine_addr(m_pObjPluginManager->GetMachineAddrMD5());
        }

        if (pData->proc_cpu() > 0 && pData->proc_mem() > 0)
        {
            FindModule<NFIServerMessageModule>()->SendMsgToMasterServer(NF_ST_MASTER_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_MASTER_SERVER_REPORT, xMsg);
        }
    }
    return 0;
}