// -------------------------------------------------------------------------
//    @FileName         :    NFProxyServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFProxyServerModule
//    @Desc             :    NFShmXFrame代理服务器模块接口定义
//                          提供客户端连接代理和消息转发功能
//                          支持客户端连接管理、消息路由和负载均衡
// -------------------------------------------------------------------------

#pragma once
#include "NFServerComm/NFServerCommon/NFIProxyServerModule.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include <NFComm/NFPluginModule/NFEventObj.h>
#include "NFComm/NFCore/NFCommMap.hpp"

/**
 * @brief 代理服务器模块实现类
 *
 * NFCProxyServerModule是NFIProxyServerModule接口的具体实现，
 * 负责处理客户端连接代理和消息转发功能。
 *
 * 该模块提供以下主要功能：
 * - 客户端连接管理和代理
 * - 消息路由和转发
 * - 负载均衡和连接分发
 * - 代理服务器间通信协调
 * - 代理代理服务器注册管理
 * - 客户端消息到服务器映射
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCProxyServerModule : public NFIProxyServerModule
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化代理服务器模块，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFCProxyServerModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 *
	 * 清理代理服务器模块资源
	 */
	virtual ~NFCProxyServerModule();

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
	 * 初始化代理服务器模块，包括连接管理等
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
     * @brief 获取客户端消息对应的服务器
     *
     * 根据消息ID获取对应的服务器类型
     *
     * @param msgId 消息ID
     * @return 服务器类型
     */
    virtual uint32_t GetClientMsgServer(uint32_t msgId) override;

public:
    /**
     * @brief 处理来自NFProxyAgentServer服务器的注册
     *
     * 处理来自代理代理服务器的注册请求，可能是代理代理服务器本身的注册，
     * 也可能是代理代理服务器转发的继承NFWorkServerModule的业务服务器的注册
     *
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
    int OnHandleServerRegisterFromProxyAgentServer(uint64_t unLinkId, NFDataPackage& packet);

    /**
     * @brief 处理NFProxyAgentServer转发的继承NFWorkServerModule的业务服务器的注册
     *
     * 处理代理代理服务器转发的业务服务器注册请求
     *
     * @param xData 服务器信息报告
     * @param unLinkId 连接ID
     * @return 处理结果状态码
     */
    int OnHandleOtherWorkServerRegister(const NFrame::ServerInfoReport& xData, uint64_t unLinkId);

    /**
     * @brief 处理NFProxyAgentServer服务器的注册
     *
     * 处理代理代理服务器本身的注册请求
     *
     * @param xData 服务器信息报告
     * @param unLinkId 连接ID
     * @return 处理结果状态码
     */
    int OnHandleProxyAgentServerRegister(const NFrame::ServerInfoReport& xData, uint64_t unLinkId);

    /**
     * @brief 将自己注册到NFProxyAgentServer
     *
     * 向代理代理服务器注册当前服务器，然后由代理代理服务器转发到其他NFWorkServer业务服务器
     *
     * @param unLinkId 连接ID
     * @return 注册结果状态码
     */
    int RegisterProxyAgentServer(uint64_t unLinkId);

    ////////////////////////////////test send msg/////////////////////////////////////////////////
    /**
     * @brief 测试发送代理消息到其他服务器
     *
     * 测试向其他服务器发送代理消息的功能
     *
     * @param dstBusId 目标服务器总线ID
     * @return 发送结果状态码
     */
    int TestSendProxyMsgToOtherServer(uint64_t dstBusId);

    /**
     * @brief 处理测试其他服务器发送的消息
     *
     * 处理来自其他服务器的测试消息
     *
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
    int OnHandleTestOtherSendMsg(uint64_t unLinkId, NFDataPackage& packet);

public:
    std::vector<uint32_t> m_clientMsgToServerMap; ///< 客户端消息到服务器映射表
};
