// -------------------------------------------------------------------------
//    @FileName         :    NFProxyServerPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFProxyServerPlugin
//    @Desc             :    NFShmXFrame代理服务器插件接口定义
//                          提供代理服务器功能的插件接口，负责客户端连接代理和消息转发
//                          支持客户端连接管理、消息路由和负载均衡
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 代理服务器插件类
 *
 * NFProxyServerPlugin是NFShmXFrame框架的代理服务器插件，
 * 负责管理客户端连接代理和消息转发功能。
 *
 * 该插件提供以下主要功能：
 * - 客户端连接管理和代理
 * - 消息路由和转发
 * - 负载均衡和连接分发
 * - 代理服务器间通信协调
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFProxyServerPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化代理服务器插件，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFProxyServerPlugin(NFIPluginManager* p):NFIPlugin(p)
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
	 * 注册代理服务器模块到插件管理器
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 *
	 * 从插件管理器中注销代理服务器模块
	 */
	virtual void Uninstall() override;
};
