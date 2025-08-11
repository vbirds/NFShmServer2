// -------------------------------------------------------------------------
//    @FileName         :    NFMasterServerPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMasterServerPlugin
//    @Desc             :    NFShmXFrame主服务器插件接口定义
//                          提供主服务器功能的插件接口，负责服务器管理和协调
//                          支持服务器注册、状态管理和负载均衡
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 主服务器插件类
 *
 * NFMasterServerPlugin是NFShmXFrame框架的主服务器插件，
 * 负责管理所有其他服务器的注册、状态监控和协调工作。
 *
 * 该插件提供以下主要功能：
 * - 服务器注册管理
 * - 服务器状态监控
 * - 负载均衡协调
 * - 服务器间通信路由
 * - 系统配置管理
 * - 服务器集群管理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFMasterServerPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化主服务器插件，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFMasterServerPlugin(NFIPluginManager* p):NFIPlugin(p)
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
	 * 注册主服务器模块到插件管理器
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 *
	 * 从插件管理器中注销主服务器模块
	 */
	virtual void Uninstall() override;
};

