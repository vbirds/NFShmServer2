// -------------------------------------------------------------------------
//    @FileName         :    NFKernelPlugin.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFKernelPlugin
//    @Desc             :    内核插件主头文件，负责系统核心模块的注册和管理。
//                          该文件定义了NFShmXFrame框架的核心插件类，提供系统基础模块的管理功能，
//                          包括内核模块、配置模块、消息模块、定时器模块、事件模块、日志模块、
//                          协程模块、任务模块、控制台模块、监控模块等。
//                          主要功能包括插件生命周期管理、模块注册和卸载、系统基础服务提供、
//                          动态加载支持、版本兼容性管理
//    @Description      :    内核插件主头文件，负责系统核心模块的注册和管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

//////////////////////////////////////////////////////////////////////////

/**
 * @class NFKernelPlugin
 * @brief 内核插件类
 *
 * NFKernelPlugin是NFShmXFrame框架的核心插件，负责管理和注册系统的基础模块。
 * 该插件是整个框架的基础，提供了以下核心功能模块：
 * 
 * 核心模块：
 * - NFCKernelModule：内核模块，提供基础的对象管理和生命周期控制
 * - NFCConfigModule：配置模块，负责系统配置的加载和管理
 * - NFCMessageModule：消息模块，处理网络消息的收发和路由
 * 
 * 系统模块：
 * - NFCTimerModule：定时器模块，提供定时任务和延时执行功能
 * - NFCEventModule：事件模块，实现事件驱动的消息传递机制
 * - NFCLogModule：日志模块，统一的日志记录和管理系统
 * 
 * 高级模块：
 * - NFCCoroutineModule：协程模块，提供异步编程和协程调度功能
 * - NFCTaskModule：任务模块，管理异步任务的执行和调度
 * 
 * 工具模块：
 * - NFConsoleModule：控制台模块，提供命令行交互功能
 * - NFMonitorModule：监控模块，系统性能和状态监控
 * 
 * @note 该插件是框架的核心依赖，其他插件通常都依赖于此插件提供的基础功能
 * @note 支持动态加载和共享内存对象注册
 */
class NFKernelPlugin : public NFIPlugin
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针，用于管理插件的生命周期和依赖关系
	 * 
	 * 初始化内核插件实例，建立与插件管理器的关联关系。
	 */
	explicit NFKernelPlugin(NFIPluginManager* p):NFIPlugin(p)
	{

	}

	/**
	 * @brief 析构函数
	 * 
	 * 释放插件占用的资源，确保所有注册的模块都被正确卸载。
	 */
	virtual ~NFKernelPlugin()
	{
	}

	/**
	 * @brief 获取插件版本号
	 * @return 返回插件的版本号
	 * 
	 * 用于版本兼容性检查和插件管理。
	 */
	virtual int GetPluginVersion() override;

	/**
	 * @brief 获取插件名称
	 * @return 返回插件的名称字符串
	 * 
	 * 返回"NFKernelPlugin"，用于插件识别和日志记录。
	 */
	virtual std::string GetPluginName() override;

	/**
	 * @brief 安装插件
	 * 
	 * 注册所有内核相关的模块到插件管理器中，包括：
	 * - 基础核心模块（Kernel、Config、Message）
	 * - 系统功能模块（Timer、Event、Log）
	 * - 高级功能模块（Coroutine、Task）
	 * - 工具模块（Console、Monitor）
	 * 
	 * @note 模块注册顺序会影响模块间的依赖关系
	 */
	virtual void Install() override;

	/**
	 * @brief 卸载插件
	 * 
	 * 从插件管理器中注销所有内核相关的模块，按照与安装相反的顺序进行卸载，
	 * 确保模块依赖关系得到正确处理。
	 */
	virtual void Uninstall() override;

	/**
	 * @brief 判断是否为动态加载插件
	 * @return 返回false，表示此插件不支持动态加载
	 * 
	 * 内核插件作为框架的基础组件，通常在系统启动时静态加载，
	 * 不支持运行时的动态加载和卸载，以确保系统稳定性。
	 */
	virtual bool IsDynamicLoad() override;

	/**
	 * @brief 初始化共享内存对象注册
	 * @return 返回true表示初始化成功，false表示失败
	 * 
	 * 在共享内存环境中注册必要的对象类型和数据结构，
	 * 确保跨进程的对象访问和数据共享功能正常工作。
	 * 
	 * @note 此方法仅在使用共享内存模式时被调用
	 */
	virtual bool InitShmObjectRegister() override;
};

