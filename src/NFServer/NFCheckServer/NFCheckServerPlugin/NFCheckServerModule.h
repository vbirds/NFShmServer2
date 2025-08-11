// -------------------------------------------------------------------------
//    @FileName         :    NFCheckServerModule.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFCheckServerModule
//    @Desc             :    NFShmXFrame检查服务器模块接口定义
//                          提供系统健康检查和监控功能
//                          支持系统状态检查、性能监控和故障诊断
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFServerComm/NFServerCommon/NFICheckServerModule.h"

/**
 * @brief 检查服务器模块实现类
 *
 * NFCheckServerModule是NFICheckServerModule接口的具体实现，
 * 负责系统健康检查和监控相关的功能和业务逻辑。
 *
 * 该模块提供以下主要功能：
 * - 系统健康状态检查
 * - 性能监控和统计
 * - 故障诊断和报告
 * - 系统资源监控
 * - 服务可用性检查
 * - 检查事件处理
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFCheckServerModule : public NFICheckServerModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化检查服务器模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCheckServerModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     *
     * 清理检查服务器模块资源
     */
    virtual ~NFCheckServerModule();

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
     * 初始化检查服务器模块，包括连接管理等
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
     * @brief 处理来自服务器的信息
     *
     * 处理来自其他服务器的消息请求
     *
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
    virtual int OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet) override;
};