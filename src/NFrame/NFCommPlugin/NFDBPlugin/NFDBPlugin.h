// -------------------------------------------------------------------------
//    @FileName         :    NFDBPlugin.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFDBPlugin
//    @Desc             :    数据库插件主头文件，负责数据库相关模块的注册和管理。
//                          该文件定义了NFShmXFrame框架的数据库插件类，提供数据库相关模块的管理功能，
//                          包括MySQL模块、NoSQL模块、异步数据库模块、Redis模块等。
//                          主要功能包括注册MySQL同步和异步操作模块、注册Redis和NoSQL操作模块、
//                          管理数据库连接和驱动、提供统一的数据库接口、插件生命周期管理
//    @Description      :    数据库插件主头文件，负责数据库相关模块的注册和管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

//////////////////////////////////////////////////////////////////////////

/**
 * @class NFDBPlugin
 * @brief 数据库插件类
 * 
 * 负责管理数据库相关的所有模块，包括MySQL模块、NoSQL模块、异步数据库模块等。
 * 该插件是数据库功能的入口点，负责各个数据库模块的注册、初始化和销毁。
 * 
 * 主要功能：
 * - 注册MySQL同步/异步操作模块
 * - 注册Redis/NoSQL操作模块  
 * - 管理数据库连接和驱动
 * - 提供统一的数据库接口
 */
class NFDBPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针，用于管理插件的生命周期
	 */
	explicit NFDBPlugin(NFIPluginManager* p):NFIPlugin(p)
	{

	}

	/**
	 * @brief 析构函数
	 * 释放插件占用的资源
	 */
	virtual ~NFDBPlugin()
	{
	}

	/**
	 * @brief 获取插件版本号
	 * @return 返回插件的版本号
	 */
	virtual int GetPluginVersion() override;

	/**
	 * @brief 获取插件名称
	 * @return 返回插件的名称字符串
	 */
	virtual std::string GetPluginName() override;

	/**
	 * @brief 安装插件
	 * 注册所有数据库相关的模块到插件管理器中
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 * 从插件管理器中注销所有数据库相关的模块
	 */
	virtual void Uninstall() override;

	/**
	 * @brief 判断是否为动态加载插件
	 * @return 返回false，表示此插件不支持动态加载
	 */
	virtual bool IsDynamicLoad() override;
};

