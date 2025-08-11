// -------------------------------------------------------------------------
//    @FileName         :    NFShmPlugin.h
//    @Author           :    gaoyi
//    @Date             :   2022-09-18
//    @Module           :    NFShmPlugin
//    @Desc             :    共享内存插件头文件，提供共享内存管理功能。
//                          该文件定义了共享内存插件的核心类，提供共享内存插件的生命周期管理、
//                          共享内存对象的注册和初始化、插件版本和名称管理、动态加载支持。
//                          主要功能包括插件安装和卸载、共享内存对象注册、插件版本控制、
//                          动态加载配置
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 共享内存插件类
 * 
 * 负责管理共享内存插件的生命周期和功能
 * 继承自NFIPlugin，提供共享内存相关的核心功能
 */
class NFShmPlugin final : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化共享内存插件
	 * 
	 * @param p 插件管理器指针
	 */
	explicit NFShmPlugin(NFIPluginManager* p): NFIPlugin(p)
	{
	}

	/**
	 * @brief 析构函数
	 * 
	 * 清理插件资源
	 */
	~NFShmPlugin() override
	{
	}

	/**
	 * @brief 获取插件版本
	 * 
	 * @return 插件版本号
	 */
	int GetPluginVersion() override;

	/**
	 * @brief 获取插件名称
	 * 
	 * @return 插件名称字符串
	 */
	std::string GetPluginName() override;

	/**
	 * @brief 安装插件
	 * 
	 * 注册插件到插件管理器
	 */
	void Install() override;

	/**
	 * @brief 卸载插件
	 * 
	 * 从插件管理器中移除插件
	 */
	void Uninstall() override;

	/**
	 * @brief 是否支持动态加载
	 * 
	 * @return true 支持动态加载，false 不支持
	 */
	bool IsDynamicLoad() override;

	/**
	 * @brief 初始化共享内存对象注册
	 * 
	 * 注册所有共享内存对象类型
	 * 
	 * @return true 注册成功，false 注册失败
	 */
	bool InitShmObjectRegister() override;
};

