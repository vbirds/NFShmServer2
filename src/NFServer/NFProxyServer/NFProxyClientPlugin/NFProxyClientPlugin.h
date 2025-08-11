// -------------------------------------------------------------------------
//    @FileName         :    NFProxyClientPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFProxyClientPlugin
//    @Desc             :    NFShmXFrame代理客户端插件接口定义
//                          提供代理客户端功能的插件接口，负责客户端连接和消息处理
//                          支持客户端连接管理、消息路由和代理通信
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 代理客户端插件类
 *
 * NFProxyClientPlugin是NFShmXFrame框架的代理客户端插件，
 * 负责管理客户端连接和消息处理功能。
 *
 * 该插件提供以下主要功能：
 * - 客户端连接管理
 * - 消息路由和处理
 * - 代理通信支持
 * - 客户端状态管理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFProxyClientPlugin: public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化代理客户端插件，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFProxyClientPlugin(NFIPluginManager* p):NFIPlugin(p)
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
	 * 注册代理客户端模块到插件管理器
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 *
	 * 从插件管理器中注销代理客户端模块
	 */
	virtual void Uninstall() override;
};
