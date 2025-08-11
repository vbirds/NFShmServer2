// -------------------------------------------------------------------------
//    @FileName         :    NFLoginServerPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFLoginServerPlugin
//    @Desc             :    NFShmXFrame登录服务器插件接口定义
//                          提供登录服务器功能的插件接口，负责用户认证和登录管理
//                          支持用户登录验证、账号管理和会话管理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 登录服务器插件类
 *
 * NFLoginServerPlugin是NFShmXFrame框架的登录服务器插件，
 * 负责管理用户认证和登录相关的功能和业务逻辑。
 *
 * 该插件提供以下主要功能：
 * - 用户登录验证
 * - 账号管理和认证
 * - 会话管理和维护
 * - 登录状态跟踪
 * - 安全验证机制
 * - 共享内存对象管理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFLoginServerPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化登录服务器插件，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFLoginServerPlugin(NFIPluginManager* p): NFIPlugin(p)
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
	 * 注册登录服务器模块到插件管理器
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 *
	 * 从插件管理器中注销登录服务器模块
	 */
	virtual void Uninstall() override;

	/**
	 * @brief 初始化共享内存对象注册
	 *
	 * 注册登录服务器相关的共享内存对象，包括：
	 * - 登录会话对象
	 * - 用户认证对象
	 * - 账号管理对象
	 *
	 * @return 注册结果，true表示成功
	 */
	virtual bool InitShmObjectRegister() override;
};

