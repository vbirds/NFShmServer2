// -------------------------------------------------------------------------
//    @FileName         :    NFMemPlugin.h
//    @Author           :    gaoyi
//    @Date             :   2022-09-18
//    @Module           :    NFMemPlugin
//    @Desc             :    内存插件主头文件，负责内存相关模块的注册和管理。
//                          该文件定义了NFShmXFrame框架的内存插件类，提供内存管理相关模块的生命周期管理、
//                          插件注册与卸载、动态加载支持、共享内存对象注册等功能。
//                          主要功能包括插件生命周期管理、内存管理模块注册、动态加载支持、
//                          共享内存对象注册与初始化
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 内存插件类，负责内存相关模块的注册和管理
 * 
 * 该类继承自NFIPlugin，是NFShmXFrame框架的内存管理插件，
 * 负责注册和管理内存相关的模块，包括内存管理模块、内存辅助模块等。
 * 提供插件的生命周期管理、模块注册与卸载、共享内存对象注册等功能。
 */
class NFMemPlugin final : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFMemPlugin(NFIPluginManager* p): NFIPlugin(p)
	{
	}

	/**
	 * @brief 析构函数
	 */
	~NFMemPlugin() override
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
	 * @brief 安装插件，注册相关模块
	 * 
	 * 在插件安装时调用，负责注册内存管理相关的模块到插件管理器中
	 */
	void Install() override;

	/**
	 * @brief 卸载插件，注销相关模块
	 * 
	 * 在插件卸载时调用，负责从插件管理器中注销内存管理相关的模块
	 */
	void Uninstall() override;

	/**
	 * @brief 判断是否为动态加载插件
	 * @return 是否为动态加载
	 */
	bool IsDynamicLoad() override;

	/**
	 * @brief 初始化共享内存对象注册
	 * 
	 * 负责注册各种共享内存对象类型，包括全局ID管理器、定时器管理器、
	 * 事件管理器、事务管理器等，并设置相应的对象数量限制
	 * @return 初始化是否成功
	 */
	bool InitShmObjectRegister() override;
};

