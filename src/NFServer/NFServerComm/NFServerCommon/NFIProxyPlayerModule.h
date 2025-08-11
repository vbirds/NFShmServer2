// -------------------------------------------------------------------------
//    @FileName         :    NFIProxyClientModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerCommon
//    @Desc             :    代理玩家模块接口头文件，提供代理玩家服务器的通用功能接口。
//                          该文件定义了代理玩家模块接口类，包括代理玩家服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供代理玩家服务器的通用功能接口、支持代理玩家服务器连接管理、
//                          支持消息处理和路由分发、提供网络事件处理。
//                          代理玩家模块接口是NFShmXFrame框架的代理玩家服务器组件，负责：
//                          - 代理玩家服务器的通用功能接口
//                          - 代理玩家服务器连接管理和状态维护
//                          - 消息处理和路由分发
//                          - 服务器注册和发现
//                          - 网络事件处理和回调
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFComm/NFPluginModule/NFIDynamicModule.h"
#include "NFServerComm/NFServerCommon/NFServerCommonDefine.h"

/**
 * @brief 代理玩家模块接口类
 * 
 * 该类是代理玩家服务器的通用功能接口，提供了：
 * - 代理玩家服务器连接管理和状态维护
 * - 消息处理和路由分发
 * - 服务器注册和发现
 * - 网络事件处理和回调
 * 
 * 主要功能：
 * - 提供代理玩家服务器的通用功能接口
 * - 支持代理玩家服务器连接管理和状态维护
 * - 支持消息处理和路由分发
 * - 提供服务器注册和发现
 * - 支持网络事件处理和回调
 * - 支持跨服务器通信
 * 
 * 使用方式：
 * @code
 * class MyProxyPlayerModule : public NFIProxyPlayerModule {
 * public:
 *     virtual int OnExecute() override;
 *     virtual int OnTimer() override;
 * };
 * @endcode
 */
class NFIProxyPlayerModule : public NFIDynamicModule
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化代理玩家模块
     * 
     * @param p 插件管理器指针
     */
    NFIProxyPlayerModule(NFIPluginManager* p) :NFIDynamicModule(p)
	{

	}

    /**
     * @brief 析构函数
     */
	virtual ~NFIProxyPlayerModule()
	{

	}
};
