// -------------------------------------------------------------------------
//    @FileName         :    NFCityServerModule.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFCityServerModule
//    @Desc             :    NFShmXFrame城市服务器模块接口定义
//                          提供城市管理和区域服务功能
//                          支持城市数据管理、区域划分和城市相关业务逻辑
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFServerComm/NFServerCommon/NFICityServerModule.h"

/**
 * @brief 城市服务器模块实现类
 *
 * NFCityServerModule是NFICityServerModule接口的具体实现，
 * 负责处理城市相关的功能和业务逻辑。
 *
 * 该模块提供以下主要功能：
 * - 城市数据管理和维护
 * - 区域划分和城市边界管理
 * - 城市相关业务逻辑处理
 * - 城市服务器间通信协调
 * - 城市事件处理和消息路由
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFCityServerModule : public NFICityServerModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化城市服务器模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCityServerModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     *
     * 清理城市服务器模块资源
     */
    virtual ~NFCityServerModule();

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
     * 初始化城市服务器模块，包括连接管理等
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