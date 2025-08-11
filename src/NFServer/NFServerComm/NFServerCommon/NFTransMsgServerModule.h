// -------------------------------------------------------------------------
//    @FileName         :    NFTransMsgServerModule.h
//    @Author           :    gaoyi
//    @Date             :    22-10-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFTransMsgServerModule
//    @Desc             :    事务消息服务器模块头文件，提供事务消息服务器的通用功能。
//                          该文件定义了事务消息服务器模块类，包括服务器连接管理、
//                          消息处理接口、事务处理功能、网络事件处理。
//                          主要功能包括提供事务消息服务器的通用功能、支持多种服务器类型、
//                          支持事务处理和状态管理、提供网络事件处理。
//                          事务消息服务器模块是NFShmXFrame框架的事务处理组件，负责：
//                          - 事务消息服务器的通用功能实现
//                          - 服务器连接管理和状态维护
//                          - 事务处理和状态管理
//                          - 消息处理和路由分发
//                          - 网络事件处理和回调
//                          - 跨服务器事务通信支持
//
// -------------------------------------------------------------------------

#include "NFComm/NFPluginModule/NFIDynamicModule.h"

/**
 * @brief 事务消息服务器模块类，提供事务消息服务器的通用功能
 * 
 * 该类是事务消息服务器（如NFProxyServer、NFProxyAgentServer、NFRouteAgentServer等）的基础模块，
 * 实现了连接NFMasterServer等功能。
 * 
 * 主要功能包括：
 * - 服务器连接管理（主服务器、代理服务器、路由服务器）
 * - 事务处理和状态管理
 * - 消息处理和路由分发
 * - 网络事件处理和回调
 * - 跨服务器事务通信支持
 * 
 * 支持的服务器类型：
 * - Proxy Server (代理服务器)
 * - Proxy Agent Server (代理代理服务器)
 * - Route Agent Server (路由代理服务器)
 * - Master Server (主服务器)
 * 
 * 使用方式：
 * @code
 * class MyTransMsgServerModule : public NFTransMsgServerModule {
 * public:
 *     MyTransMsgServerModule(NFIPluginManager* p, NF_SERVER_TYPE serverType) 
 *         : NFTransMsgServerModule(p, serverType) {}
 *     
 *     // 重写消息处理函数
 *     virtual int OnHandleServerOtherMessage(uint64_t unLinkId, NFDataPackage& packet) override;
 * };
 * @endcode
 */
class NFTransMsgServerModule : public NFIDynamicModule
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化事务消息服务器模块，设置默认连接配置：
     * - 连接主服务器
     * 
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     */
    NFTransMsgServerModule(NFIPluginManager *p, NF_SERVER_TYPE serverType) : NFIDynamicModule(p), m_serverType(serverType)
    {
        m_connectMasterServer = true;
    }

    /**
     * @brief 析构函数
     */
    virtual ~NFTransMsgServerModule()
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
     * @brief 绑定服务器
     * 
     * 绑定事务消息服务器到指定服务器类型，并注册消息回调、事件订阅、定时器等。
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
     * 
     * 当服务器断开连接时，更新服务器状态并清理链接。
     * 
     * @param unLinkId 连接ID
     * @return 处理结果
     */
    virtual int OnHandleServerDisconnect(uint64_t unLinkId);
    //////////////////////////////////////////////////////////Server服务器//////////////////////////////////////////////////////////////////
public:
    //////////////////////////////////////////////////////////NFMasterServer服务器//////////////////////////////////////////////////////////////////
    /**
     * @brief 连接Master服务器
     * 
     * 尝试连接主服务器，并注册相关回调。
     * 
     * @param xData 服务器信息
     * @return 连接结果
     */
    virtual int ConnectMasterServer(const NFrame::ServerInfoReport &xData);

    /**
     * @brief 注册Master服务器
     * 
     * 如果当前连接主服务器，则向主服务器发送注册请求。
     * 
     * @param serverState 服务器状态
     * @return 注册结果
     */
    virtual int RegisterMasterServer(uint32_t serverState);

    /**
     * @brief 处理Master服务器连接事件
     * 
     * 当Master服务器连接或断开时，更新相关状态并注册回调。
     * 
     * @param nEvent 事件类型
     * @param unLinkId 连接ID
     * @return 处理结果
     */
    virtual int OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId);

    /**
     * @brief 处理Master服务器未注册消息
     * 
     * 处理Master服务器发送的未注册协议消息。
     * 
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果
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
    bool IsConnectMasterServer() const;

    void SetConnectMasterServer(bool connectMasterServer);
private:
    NF_SERVER_TYPE m_serverType;
    bool m_connectMasterServer;
};

