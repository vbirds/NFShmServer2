// -------------------------------------------------------------------------
//    @FileName         :    NFSnsServerPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFSnsServerPlugin
//    @Desc             :    NFShmXFrame社交网络服务器插件接口定义
//                          提供社交网络服务器功能的插件接口，负责社交功能和用户互动
//                          支持好友系统、聊天、动态分享等社交功能
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 社交网络服务器插件类
 *
 * NFSnsServerPlugin是NFShmXFrame框架的社交网络服务器插件，
 * 负责管理社交网络相关的功能和业务逻辑。
 *
 * 该插件提供以下主要功能：
 * - 好友系统管理
 * - 聊天和消息处理
 * - 动态分享和互动
 * - 社交数据存储
 * - 用户关系管理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFSnsServerPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化社交网络服务器插件，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFSnsServerPlugin(NFIPluginManager* p):NFIPlugin(p)
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
	 * 注册社交网络服务器模块到插件管理器
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 *
	 * 从插件管理器中注销社交网络服务器模块
	 */
	virtual void Uninstall() override;

	/**
	 * @brief 初始化共享内存对象注册
	 *
	 * 注册社交网络服务器相关的共享内存对象
	 *
	 * @return 注册结果，true表示成功
	 */
	virtual bool InitShmObjectRegister() override;
};

