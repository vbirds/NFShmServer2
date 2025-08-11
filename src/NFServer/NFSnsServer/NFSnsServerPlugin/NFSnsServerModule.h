// -------------------------------------------------------------------------
//    @FileName         :    NFCSnsServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCSnsServerModule
//    @Desc             :    NFShmXFrame社交网络服务器模块接口定义
//                          提供社交功能和用户互动功能
//                          支持好友系统、聊天、动态分享等社交功能
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFCore/NFCommMap.hpp"
#include "NFServerComm/NFServerCommon/NFISnsServerModule.h"

/**
 * @brief 社交网络服务器模块实现类
 *
 * NFCSnsServerModule是NFISnsServerModule接口的具体实现，
 * 负责处理社交网络相关的功能和业务逻辑。
 *
 * 该模块提供以下主要功能：
 * - 好友系统管理
 * - 聊天和消息处理
 * - 动态分享和互动
 * - 社交数据存储
 * - 用户关系管理
 * - 社交事件处理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCSnsServerModule : public NFISnsServerModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化社交网络服务器模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCSnsServerModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     *
     * 清理社交网络服务器模块资源
     */
    virtual ~NFCSnsServerModule();

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
     * 初始化社交网络服务器模块，包括连接管理等
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
