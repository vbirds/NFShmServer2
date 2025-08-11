// -------------------------------------------------------------------------
//    @FileName         :    NFWorkServerModule.h
//    @Author           :    gaoyi
//    @Date             :    22-10-26
//    @Email			:    445267987@qq.com
//    @Module           :    NFWorkServerModule
//    @Desc             :    工作服务器模块头文件，提供业务服务器的通用功能。
//                          该文件定义了工作服务器模块类，包括服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供业务服务器的通用功能、支持多种服务器类型、
//                          支持服务器间通信、支持Lua脚本加载、提供网络事件处理。
//                          工作服务器模块是NFShmXFrame框架的核心业务组件，负责：
//                          - 业务服务器的通用功能实现
//                          - 服务器连接管理和状态维护
//                          - 消息处理和路由分发
//                          - 服务器注册和发现
//                          - Lua脚本加载和管理
//                          - 网络事件处理和回调
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIDynamicModule.h"
#include "NFServerBindRpcService.h"

/**
 * @brief 工作服务器模块类，提供业务服务器的通用功能
 * 
 * 该类是业务服务器（如NFLogicServer、NFWorldServer、NFSnsServer等）的基础模块，
 * 实现了连接NFMasterServer、NFProxyAgentServer、NFRouteAgentServer等功能。
 * 
 * 主要功能包括：
 * - 服务器连接管理（主服务器、代理服务器、路由服务器）
 * - 消息处理和路由分发
 * - 服务器注册和状态维护
 * - Lua脚本加载和管理
 * - 网络事件处理和回调
 * - 跨服务器通信支持
 * 
 * 支持的服务器类型：
 * - Logic Server (逻辑服务器)
 * - World Server (世界服务器)
 * - SNS Server (社交服务器)
 * - Game Server (游戏服务器)
 * - Center Server (中心服务器)
 * 
 * 使用方式：
 * @code
 * class MyWorkServerModule : public NFWorkServerModule {
 * public:
 *     MyWorkServerModule(NFIPluginManager* p, NF_SERVER_TYPE serverType) 
 *         : NFWorkServerModule(p, serverType) {}
 *     
 *     // 重写消息处理函数
 *     virtual int OnHandleServerOtherMessage(uint64_t unLinkId, NFDataPackage& packet) override;
 * };
 * @endcode
 */
class NFWorkServerModule : public NFIDynamicModule
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化工作服务器模块，设置默认连接配置：
     * - 连接主服务器
     * - 连接路由代理服务器
     * - 连接代理代理服务器
     * - 禁用存储服务器检查
     * - 禁用世界服务器检查
     * - 禁用中心服务器检查
     * 
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     */
    NFWorkServerModule(NFIPluginManager *p, NF_SERVER_TYPE serverType) : NFIDynamicModule(p), m_serverType(serverType)
    {
        SetConnectMasterServer(true);
        SetConnectRouteAgentServer(true);
        SetConnectProxyAgentServer(true);
        m_checkStoreServer = false;
        m_checkWorldServer = false;
        m_checkCenterServer = false;
    }

    /**
     * @brief 析构函数
     */
    virtual ~NFWorkServerModule()
    {

    }

    /**
     * @brief 处理定时器事件
     * 
     * 处理系统定时器事件，包括：
     * - 心跳检测
     * - 状态更新
     * - 定期任务执行
     * 
     * @param nTimerID 定时器ID
     * @return 处理结果
     */
    virtual int OnTimer(uint32_t nTimerID) override;

    /**
     * @brief 响应注册事件
     * 
     * 处理服务器注册相关的事件，包括：
     * - 服务器注册请求
     * - 注册状态更新
     * - 注册响应处理
     * 
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 源类型
     * @param nSrcID 源ID
     * @param pMessage 消息数据
     * @return 处理结果
     */
    virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage) override;

public:
    /**
     * @brief 获取服务器类型
     * @return 服务器类型
     */
    NF_SERVER_TYPE GetServerType() const;

    /**
     * @brief 设置服务器类型
     * @param serverType 服务器类型
     */
    void SetServerType(NF_SERVER_TYPE serverType);

    /**
     * @brief 检查是否连接主服务器
     * @return 是否连接主服务器
     */
    bool IsConnectMasterServer() const;

    /**
     * @brief 设置是否连接主服务器
     * @param connectMasterServer 是否连接主服务器
     */
    void SetConnectMasterServer(bool connectMasterServer);

    /**
     * @brief 检查是否连接路由代理服务器
     * @return 是否连接路由代理服务器
     */
    bool IsConnectRouteAgentServer() const;

    /**
     * @brief 设置是否连接路由代理服务器
     * @param connectRouteAgentServer 是否连接路由代理服务器
     */
    void SetConnectRouteAgentServer(bool connectRouteAgentServer);

    /**
     * @brief 检查是否连接代理代理服务器
     * @return 是否连接代理代理服务器
     */
    bool IsConnectProxyAgentServer() const;

    /**
     * @brief 设置是否连接代理代理服务器
     * @param connectProxyAgentServer 是否连接代理代理服务器
     */
    void SetConnectProxyAgentServer(bool connectProxyAgentServer);

    /**
     * @brief 检查是否检查存储服务器
     * @return 是否检查存储服务器
     */
    bool IsCheckStoreServer() const;

    /**
     * @brief 设置是否检查存储服务器
     * @param checkStoreServer 是否检查存储服务器
     */
    void SetCheckStoreServer(bool checkStoreServer);

    /**
     * @brief 检查是否检查世界服务器
     * @return 是否检查世界服务器
     */
    bool IsCheckWorldServer() const;

    /**
     * @brief 设置是否检查世界服务器
     * @param checkWorldServer 是否检查世界服务器
     */
    void SetCheckWorldServer(bool checkWorldServer);

    /**
     * @brief 检查是否检查中心服务器
     * @return 是否检查中心服务器
     */
    bool IsCheckCenterServer() const;

    /**
     * @brief 设置是否检查中心服务器
     * @param checkCenterServer 是否检查中心服务器
     */
    void SetCheckCenterServer(bool checkCenterServer);

public:
    /**
     * @brief 绑定服务器
     * 
     * 绑定当前服务器到网络端口，启动监听服务
     * 
     * @return 绑定结果
     */
    int BindServer();

    /**
     * @brief 连接主服务器
     * 
     * 建立与主服务器的连接，进行服务器注册
     * 
     * @return 连接结果
     */
    int ConnectMasterServer();

    /**
     * @brief 初始化加载Lua脚本
     * 
     * 加载和初始化Lua脚本文件，设置脚本环境
     * 
     * @return 初始化结果
     */
    int InitLoadLua();
public:
    //////////////////////////////////////////////////////////Server服务器//////////////////////////////////////////////////////////////////
    /**
     * @brief Server服务器连接事件处理
     * 
     * 处理服务器连接事件，可以是网络/bus的连接事件，包括：
     * - 连接建立事件
     * - 连接断开事件
     * - 连接错误事件
     * 
     * @param nEvent 事件类型
     * @param unLinkId 连接ID
     * @return 处理结果
     */
    virtual int OnServerSocketEvent(eMsgType nEvent, uint64_t unLinkId);

    /**
     * @brief 处理Server未注册的消息
     * 
     * 处理服务器未注册的消息，包括：
     * - 未知消息类型
     * - 未实现的消息处理
     * - 错误消息处理
     * 
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果
     */
    virtual int OnHandleServerOtherMessage(uint64_t unLinkId, NFDataPackage &packet);

    /**
     * @brief 处理Server服务器的连接掉线
     * @param unLinkId
     * @return
     */
    virtual int OnHandleServerDisconnect(uint64_t unLinkId);
    //////////////////////////////////////////////////////////Server服务器//////////////////////////////////////////////////////////////////
public:
    //////////////////////////////////////////////////////////NFMasterServer服务器//////////////////////////////////////////////////////////////////
    /**
     * @brief 链接Master服务器
     * @param xData
     * @return
     */
    virtual int ConnectMasterServer(const NFrame::ServerInfoReport &xData);

    /**
     * @brief 注册Master服务器
     * @param serverState
     * @return
     */
    virtual int RegisterMasterServer(uint32_t serverState);

    /**
     * @brief 处理Master服务器链接事件
     * @param nEvent
     * @param unLinkId
     * @return
     */
    virtual int OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId);

    /**
     * @brief 处理Master服务器未注册消息
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnHandleMasterOtherMessage(uint64_t unLinkId, NFDataPackage &packet);

    /**
     * @brief 接受来自MasterServer的其他服务器的报告
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnHandleServerReportFromMasterServer(uint64_t unLinkId, NFDataPackage &packet);

    /**
     * @brief 接受来自MasterServer的其他服务器的报告
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnHandleOtherServerReportFromMasterServer(const NFrame::ServerInfoReport &xData);

    /**
     * @brief 每隔一段时间向Master服务器发送自身信息
     * @return
     */
    virtual int ServerReportToMasterServer();
    //////////////////////////////////////////////////////////NFMasterServer服务器//////////////////////////////////////////////////////////////////
public:
    //////////////////////////////////////////////////////////NFProxyServer,NFProxyAgentServer 服务器//////////////////////////////////////////////////////////////////
    /**
     * @brief 收到NFProxyAgentServer服务器报告, 连接NFProxyAgentServer服务器
     * @param xData
     * @return
     */
    virtual int OnHandleProxyAgentServerReport(const NFrame::ServerInfoReport &xData);

    /**
     * @brief 连接NFProxyAgentServer服务器网络事件处理
     * @param nEvent
     * @param unLinkId
     * @return
     */
    virtual int OnProxyAgentServerSocketEvent(eMsgType nEvent, uint64_t unLinkId);

    /**
     * @brief 处理NFProxyAgentServer转发过来的未注册的协议, 协议来自客户端，以及NFPRoxyServer
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnHandleProxyAgentServerOtherMessage(uint64_t unLinkId, NFDataPackage &packet);

    /**
     * @brief 注册自身信息到NFProxyAgentServer, 通过NFProxyAgentServer转发, 最终注册到NFProxyServer
     * @param unLinkId
     * @return
     */
    virtual int RegisterProxyAgentServer(uint64_t unLinkId);

    /**
     * @brief 受来自其他服务器的注册，这里主要是NFProxyServer注册, 通过NFProxyAgentServer转发
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnServerRegisterProcessFromProxyServer(uint64_t unLinkId, NFDataPackage &packet);

    /**
     * @brief 处理来自NFProxyAgentServer转发的NFProxyServer注册
     * @param xData
     * @param unlinkId
     * @return
     */
    virtual int OnHandleProxyServerRegister(const NFrame::ServerInfoReport &xData, uint64_t unlinkId);

    //////////////////////////////////////////////////////////NFProxyServer,NFProxyAgentServer 服务器//////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////NFStoreServer服务器//////////////////////////////////////////////////////////////////
    virtual int OnHandleStoreServerReport(const NFrame::ServerInfoReport &xData);

    //////////////////////////////////////////////////////////NFStoreServer服务器//////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////NFWorldServer服务器//////////////////////////////////////////////////////////////////
    virtual int OnHandleWorldServerReport(const NFrame::ServerInfoReport &xData);
    //////////////////////////////////////////////////////////NFCenterServer服务器//////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////NFCenterServer服务器//////////////////////////////////////////////////////////////////
    virtual int OnHandleCenterServerReport(const NFrame::ServerInfoReport &xData);
    //////////////////////////////////////////////////////////NFWorldServer服务器//////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////NFRouteAgent服务器//////////////////////////////////////////////////////////////////
    /**
     * @brief 收到NFRouteAgentServer服务器报告, 连接NFRouteAgentServer服务器
     * @param xData
     * @return
     */
    virtual int OnHandleRouteAgentServerReport(const NFrame::ServerInfoReport &xData);

    /**
     * @brief 注册自身信息到NFRouteAgentServer
     * @param unLinkId
     * @return
     */
    virtual int RegisterRouteAgentServer(uint64_t unLinkId);

    /**
     * @brief NFRouteAgentServer注册返回
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnRegisterRouteAgentServerRspProcess(uint64_t unLinkId, NFDataPackage &packet);

    /**
     * @brief 连接NFRouteAgentServer服务器网络事件处理
     * @param nEvent
     * @param unLinkId
     * @return
     */
    virtual int OnRouteAgentServerSocketEvent(eMsgType nEvent, uint64_t unLinkId);

    /**
     * @brief NFRouteAgentServer服务器未处理协议,协议来自别的内网服务器， 由NFRouteAgentServer,NFRouteServer转发
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnHandleRouteAgentServerOtherMessage(uint64_t unLinkId, NFDataPackage &packet);
    //////////////////////////////////////////////////////////NFRouteAgent服务器//////////////////////////////////////////////////////////////////
protected:
    NF_SERVER_TYPE m_serverType;
    bool m_connectMasterServer;
    bool m_connectRouteAgentServer;
    bool m_connectProxyAgentServer;
    bool m_checkStoreServer;
    bool m_checkWorldServer;
    bool m_checkCenterServer;
};