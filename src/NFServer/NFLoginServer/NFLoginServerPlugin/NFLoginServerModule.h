// -------------------------------------------------------------------------
//    @FileName         :    NFCLoginServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCLoginServerModule
//    @Desc             :    NFShmXFrame登录服务器模块接口定义
//                          提供用户认证和登录管理功能
//                          支持用户登录验证、账号管理和会话管理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFCore/NFCommMap.hpp"
#include "NFServerComm/NFServerCommon/NFILoginServerModule.h"

/**
 * @brief 登录服务器模块实现类
 *
 * NFCLoginServerModule是NFILoginServerModule接口的具体实现，
 * 负责处理用户认证和登录相关的功能和业务逻辑。
 *
 * 该模块提供以下主要功能：
 * - 用户登录验证
 * - 账号管理和认证
 * - 会话管理和维护
 * - 登录状态跟踪
 * - 安全验证机制
 * - 登录事件处理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCLoginServerModule : public NFILoginServerModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化登录服务器模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCLoginServerModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     *
     * 清理登录服务器模块资源
     */
    virtual ~NFCLoginServerModule();

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
     * 初始化登录服务器模块，包括连接管理等
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
