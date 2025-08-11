//------------------------------------------------------------------------ -
//    @FileName         :    NFNetPlugin.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFNetPlugin
//    @Desc             :    网络插件头文件，提供网络通信基础功能。
//                          该文件定义了网络插件类，包括网络插件类定义、
//                          插件生命周期管理接口、模块注册和注销功能。
//                          主要功能包括提供网络通信基础功能、管理网络模块的生命周期、
//                          支持动态加载和卸载、提供插件版本管理。
//                          网络插件是NFShmXFrame框架的核心组件之一，负责：
//                          - TCP/UDP网络通信
//                          - HTTP/HTTPS协议支持
//                          - 数据包解析和处理
//                          - 网络连接池管理
//                          - 网络事件分发
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 网络插件类，负责网络模块的初始化和管理
 *
 * 该插件提供了基础的网络通信功能，包括：
 * - 网络连接管理
 * - 数据包处理
 * - 消息收发
 * - 网络事件处理
 * - TCP/UDP协议支持
 * - HTTP/HTTPS协议支持
 * - 网络连接池管理
 * - 跨服务器通信
 * 
 * 使用方式：
 * @code
 * // 插件会自动注册网络模块
 * // 无需手动调用，由插件管理器管理
 * @endcode
 */
class NFNetPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFNetPlugin(NFIPluginManager* p):NFIPlugin(p)
	{

	}

	/**
	 * @brief 获取插件版本号
	 * @return 插件版本号
	 */
	int GetPluginVersion() override;

	/**
	 * @brief 获取插件名称
	 * @return 插件名称字符串
	 */
	std::string GetPluginName() override;

	/**
	 * @brief 安装插件，注册插件模块
	 * 
	 * 在插件安装时调用，负责：
	 * - 注册网络模块到插件管理器
	 * - 启用心跳处理功能
	 * - 初始化网络相关资源
	 */
	void Install() override;

	/**
	 * @brief 卸载插件，清理插件资源
	 * 
	 * 在插件卸载时调用，负责：
	 * - 从插件管理器中注销网络模块
	 * - 清理网络相关资源
	 * - 停止所有网络服务
	 */
	void Uninstall() override;

	/**
	 * @brief 检查插件是否支持动态加载
	 * @return true 支持动态加载，false 不支持
	 */
	bool IsDynamicLoad() override;
};
