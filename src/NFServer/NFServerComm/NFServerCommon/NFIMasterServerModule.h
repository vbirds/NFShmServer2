// -------------------------------------------------------------------------
//    @FileName         :    NFIGameServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerCommon
//    @Desc             :    主服务器模块接口头文件，提供主服务器的通用功能接口。
//                          该文件定义了主服务器模块接口类，包括主服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供主服务器的通用功能接口、支持主服务器连接管理、
//                          支持消息处理和路由分发、提供网络事件处理。
//                          主服务器模块接口是NFShmXFrame框架的主服务器组件，负责：
//                          - 主服务器的通用功能接口
//                          - 主服务器连接管理和状态维护
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
 * @brief 主服务器模块接口类
 * 
 * 该类是主服务器的通用功能接口，提供了：
 * - 主服务器连接管理和状态维护
 * - 消息处理和路由分发
 * - 服务器注册和发现
 * - 网络事件处理和回调
 * 
 * 主要功能：
 * - 提供主服务器的通用功能接口
 * - 支持主服务器连接管理和状态维护
 * - 支持消息处理和路由分发
 * - 提供服务器注册和发现
 * - 支持网络事件处理和回调
 * - 支持跨服务器通信
 * 
 * 使用方式：
 * @code
 * class MyMasterServerModule : public NFIMasterServerModule {
 * public:
 *     virtual int OnExecute() override;
 *     virtual int OnTimer() override;
 * };
 * @endcode
 */
class NFIMasterServerModule : public NFIDynamicModule
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化主服务器模块
	 * 
	 * @param p 插件管理器指针
	 */
	NFIMasterServerModule(NFIPluginManager* p) :NFIDynamicModule(p)
	{

	}

	/**
	 * @brief 析构函数
	 */
	virtual ~NFIMasterServerModule()
	{

	}
};

