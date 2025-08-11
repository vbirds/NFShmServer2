// -------------------------------------------------------------------------
//    @FileName         :    NFCRouteServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCRouteServerModule
//    @Desc             :    NFShmXFrame路由服务器模块接口定义
//                          提供消息路由和负载均衡功能
//                          支持跨服通信、路由分发和连接管理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFCore/NFCommMap.hpp"
#include "NFServerComm/NFServerCommon/NFIRouteServerModule.h"

class NFIMessageModule;

/**
 * @brief 路由服务器模块实现类
 *
 * NFCRouteServerModule是NFIRouteServerModule接口的具体实现，
 * 负责处理消息路由和负载均衡功能。
 *
 * 该模块提供以下主要功能：
 * - 消息路由和分发
 * - 跨服通信支持
 * - 负载均衡和连接管理
 * - 路由策略配置
 * - 服务器注册和发现
 * - Master服务器连接管理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCRouteServerModule : public NFIRouteServerModule
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化路由服务器模块，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFCRouteServerModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 *
	 * 清理路由服务器模块资源
	 */
	virtual ~NFCRouteServerModule();

	/**
	 * @brief 模块唤醒
	 *
	 * 在模块初始化完成后调用，进行必要的初始化工作
	 *
	 * @return 初始化结果状态码
	 */
	virtual int Awake() override;

	/**
	 * @brief 模块初始化
	 *
	 * 初始化路由服务器模块，包括连接管理等
	 *
	 * @return 初始化结果状态码
	 */
	virtual int Init() override;

	/**
	 * @brief 模块定时更新
	 *
	 * 每帧调用，处理模块的定时任务
	 *
	 * @return 处理结果状态码
	 */
	virtual int Tick() override;

	/**
	 * @brief 动态插件处理
	 *
	 * 处理动态插件的加载和卸载
	 *
	 * @return 处理结果状态码
	 */
	virtual int OnDynamicPlugin() override;

    /**
     * @brief 定时器回调
     *
     * 处理定时器事件
     *
     * @param nTimerID 定时器ID
     * @return 处理结果状态码
     */
    virtual int OnTimer(uint32_t nTimerID) override;

    /**
     * @brief 执行事件
     *
     * 处理服务器事件执行
     *
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 源类型
     * @param nSrcID 源ID
     * @param pMessage 消息数据
     * @return 处理结果状态码
     */
    virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage);

	/**
	 * @brief 处理路由Socket事件
	 *
	 * 处理路由相关的Socket连接事件
	 *
	 * @param nEvent 事件类型
	 * @param unLinkId 连接ID
	 * @return 处理结果状态码
	 */
	int OnRouteSocketEvent(eMsgType nEvent, uint64_t unLinkId);

    /**
     * @brief 处理其他消息
     *
     * 处理来自其他服务器的消息请求
     *
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
    int OnHandleOtherMessage(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理服务器断开连接
	 *
	 * 处理服务器断开连接事件
	 *
	 * @param unLinkId 连接ID
	 * @return 处理结果状态码
	 */
	int OnHandleServerDisconnect(uint64_t unLinkId);

	/**
	 * @brief 处理服务器注册
	 *
	 * 处理服务器注册请求
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
	int OnServerRegisterProcess(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理服务器报告
	 *
	 * 处理服务器状态报告
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
	int OnHandleServerReport(uint64_t unLinkId, NFDataPackage& packet);

    /**
     * @brief 处理服务器注册路由代理
     *
     * 处理服务器向路由代理注册的请求
     *
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
    int OnHandleServerRegisterRouteAgent(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 注册到Master服务器
	 *
	 * 向Master服务器注册当前服务器信息
	 *
	 * @param serverState 服务器状态
	 * @return 注册结果状态码
	 */
	int RegisterMasterServer(uint32_t serverState);

	/**
	 * @brief 服务器报告
	 *
	 * 向Master服务器报告当前服务器状态
	 *
	 * @return 报告结果状态码
	 */
	int ServerReport();

	/**
	 * @brief 处理路由代理注册
	 *
	 * 处理路由代理服务器的注册请求
	 *
	 * @param xData 服务器信息报告
	 * @param unlinkId 连接ID
	 * @return 处理结果状态码
	 */
	int OnHandleRouteAgentRegister(const NFrame::ServerInfoReport& xData, uint64_t unlinkId);

	/**
	 * @brief 连接Master服务器
	 *
	 * 连接到Master服务器
	 *
	 * @param xData 服务器信息报告
	 * @return 连接结果状态码
	 */
    int ConnectMasterServer(const NFrame::ServerInfoReport& xData);

	/**
	 * @brief 处理Master服务器Socket事件
	 *
	 * 处理Master服务器相关的Socket连接事件
	 *
	 * @param nEvent 事件类型
	 * @param unLinkId 连接ID
	 * @return 处理结果状态码
	 */
	int OnMasterSocketEvent(eMsgType nEvent, uint64_t unLinkId);

	/**
	 * @brief 处理Master服务器其他消息
	 *
	 * 处理来自Master服务器的其他消息
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
	int OnHandleMasterOtherMessage(uint64_t unLinkId, NFDataPackage& packet);
};
