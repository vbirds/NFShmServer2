// -------------------------------------------------------------------------
//    @FileName         :    NFStoreServerPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFStoreServerPlugin
//    @Desc             :    NFShmXFrame存储服务器插件接口定义
//                          提供存储服务器功能的插件接口，负责数据库操作和存储管理
//                          支持数据库查询、插入、修改、删除等操作，提供RPC服务
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 存储服务器插件类
 * 
 * NFStoreServerPlugin是NFShmXFrame框架的存储服务器插件，
 * 负责管理数据库操作和存储服务功能。
 * 
 * 该插件提供以下主要功能：
 * - 数据库查询、插入、修改、删除操作
 * - RPC服务接口，支持远程数据库操作
 * - 数据库连接管理和事务处理
 * - 缓存机制支持
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFStoreServerPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化存储服务器插件，设置插件管理器
	 * 
	 * @param p 插件管理器指针
	 */
	explicit NFStoreServerPlugin(NFIPluginManager* p):NFIPlugin(p)
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
	 * 注册存储服务器模块到插件管理器
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 * 
	 * 从插件管理器中注销存储服务器模块
	 */
	virtual void Uninstall() override;
};

