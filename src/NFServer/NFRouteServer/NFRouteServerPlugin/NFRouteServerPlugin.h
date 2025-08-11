// -------------------------------------------------------------------------
//    @FileName         :    NFRouteServerPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFRouteServerPlugin
//    @Desc             :    NFShmXFrame路由服务器插件接口定义
//                          提供路由服务器功能的插件接口，负责消息路由和负载均衡
//                          支持跨服通信、路由分发和连接管理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 路由服务器插件类
 *
 * NFRouteServerPlugin是NFShmXFrame框架的路由服务器插件，
 * 负责管理消息路由和负载均衡功能。
 *
 * 该插件提供以下主要功能：
 * - 消息路由和分发
 * - 跨服通信支持
 * - 负载均衡和连接管理
 * - 路由策略配置
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFRouteServerPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化路由服务器插件，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFRouteServerPlugin(NFIPluginManager* p):NFIPlugin(p)
	{
	}

	/**
	 * @brief 获取插件版本号
	 *
	 * @return 插件版本号
	 */
	virtual int GetPluginVersion() override;

	/**
	 * @brief 获取插件名称
	 *
	 * @return 插件名称字符串
	 */
	virtual std::string GetPluginName() override;

	/**
	 * @brief 安装插件
	 *
	 * 注册路由服务器模块到插件管理器
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 *
	 * 从插件管理器中注销路由服务器模块
	 */
	virtual void Uninstall() override;
};

