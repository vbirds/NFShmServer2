// -------------------------------------------------------------------------
//    @FileName         :    NFProxyClientModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFProxyClientModule
//    @Desc             :    NFShmXFrame代理客户端模块接口定义
//                          提供客户端连接和消息处理功能
//                          支持客户端连接管理、消息路由和代理通信
// -------------------------------------------------------------------------

#pragma once

#include "NFServerComm/NFServerCommon/NFIProxyClientModule.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFCore/NFCommMap.hpp"

/**
 * @brief 代理客户端模块实现类
 *
 * NFCProxyClientModule是NFIProxyClientModule接口的具体实现，
 * 负责处理客户端连接和消息处理功能。
 *
 * 该模块提供以下主要功能：
 * - 客户端连接管理
 * - 消息路由和处理
 * - 代理通信支持
 * - 客户端状态管理
 * - 外部客户端监听
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCProxyClientModule : public NFIProxyClientModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化代理客户端模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCProxyClientModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     *
     * 清理代理客户端模块资源
     */
    virtual ~NFCProxyClientModule();

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
     * 初始化代理客户端模块，包括连接管理等
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

private:
    uint64_t m_proxyClientLinkId; ///< 对外部客户端监听唯一ID
};
