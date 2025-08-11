// -------------------------------------------------------------------------
//    @FileName         :    NFKernelPlugin.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFKernelPlugin
//    @Description      :    内核插件实现文件，负责系统核心模块的注册和管理
//
// -------------------------------------------------------------------------

#include "NFKernelPlugin.h"
#include "NFCKernelModule.h"
#include "NFCTimerModule.h"
#include "NFCEventModule.h"
#include "NFCLogModule.h"
#include "NFCConfigModule.h"
#include "NFConsoleModule.h"
#include "NFMonitorModule.h"
#include "NFCMessageModule.h"
#include "NFCCoroutineModule.h"
#include "NFCTaskModule.h"

//
//
#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态加载插件启动函数
 * @param pm 插件管理器指针
 * 
 * 当插件以动态库形式加载时，系统会调用此函数来创建插件实例。
 * 该函数会创建NFKernelPlugin实例并注册到插件管理器中。
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
	CREATE_PLUGIN(pm, NFKernelPlugin)
};

/**
 * @brief 动态加载插件停止函数
 * @param pm 插件管理器指针
 * 
 * 当插件以动态库形式卸载时，系统会调用此函数来销毁插件实例。
 * 该函数会销毁NFKernelPlugin实例并从插件管理器中移除。
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
	DESTROY_PLUGIN(pm, NFKernelPlugin)
};

#endif

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 * @return 返回插件版本号，当前为0
 * 
 * 用于插件版本控制和兼容性检查。
 */
int NFKernelPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 * @return 返回插件名称字符串"NFKernelPlugin"
 * 
 * 使用宏GET_CLASS_NAME自动获取类名作为插件名称。
 */
std::string NFKernelPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFKernelPlugin);
}

/**
 * @brief 判断是否为动态加载插件
 * @return 返回false，表示此插件不支持动态加载
 * 
 * 内核插件作为框架的基础组件，通常在系统启动时静态加载，
 * 不支持运行时的动态加载和卸载。
 */
bool NFKernelPlugin::IsDynamicLoad()
{
	return false;
}

/**
 * @brief 安装插件，注册所有核心模块
 * 
 * 按照特定顺序注册框架的核心模块，确保模块依赖关系正确建立：
 * 1. 日志模块(Log) - 最先启动，为其他模块提供日志功能
 * 2. 配置模块(Config) - 加载系统配置
 * 3. 定时器模块(Timer) - 提供定时功能，注册为Tick模块
 * 4. 事件模块(Event) - 事件驱动机制
 * 5. 内核模块(Kernel) - 核心对象管理
 * 6. 监控模块(Monitor) - 系统监控
 * 7. 控制台模块(Console) - 命令行交互
 * 8. 消息模块(Message) - 网络消息处理
 * 9. 协程模块(Coroutine) - 异步编程支持，注册为Tick模块
 * 10. 任务模块(Task) - 任务调度管理，注册为Tick模块
 * 
 * @note 日志系统第一个启动，然后是配置系统
 * @note 部分模块注册为Tick模块，会在每帧进行更新
 */
void NFKernelPlugin::Install()
{
	REGISTER_MODULE(m_pObjPluginManager, NFILogModule, NFCLogModule);
	REGISTER_MODULE(m_pObjPluginManager, NFIConfigModule, NFCConfigModule);
	REGISTER_MODULE_TICK(m_pObjPluginManager, NFITimerModule, NFCTimerModule);
	REGISTER_MODULE(m_pObjPluginManager, NFIEventModule, NFCEventModule);
	REGISTER_MODULE(m_pObjPluginManager, NFIKernelModule, NFCKernelModule);
	REGISTER_MODULE(m_pObjPluginManager, NFIMonitorModule, NFCMonitorModule);
	REGISTER_MODULE(m_pObjPluginManager, NFIConsoleModule, NFCConsoleModule);
	REGISTER_MODULE(m_pObjPluginManager, NFIMessageModule, NFCMessageModule);
    REGISTER_MODULE_TICK(m_pObjPluginManager, NFICoroutineModule, NFCCoroutineModule);
	REGISTER_MODULE_TICK(m_pObjPluginManager, NFITaskModule, NFCTaskModule);

	/*
		log 系统第一个启动，然后是配置
	*/
	FindModule<NFILogModule>()->InitLogSystem();
	/*
		加载服务器配置
	*/
	FindModule<NFIConfigModule>()->LoadFrameConfig();
}

/**
 * @brief 卸载插件，注销所有核心模块
 * 
 * 按照与安装相反的顺序注销所有模块，确保模块依赖关系得到正确处理：
 * 1. 协程模块(Coroutine) - 最先卸载高级功能
 * 2. 任务模块(Task)
 * 3. 监控模块(Monitor)
 * 4. 控制台模块(Console)
 * 5. 内核模块(Kernel)
 * 6. 消息模块(Message)
 * 7. 事件模块(Event)
 * 8. 定时器模块(Timer)
 * 9. 日志模块(Log)
 * 10. 配置模块(Config) - 最后卸载基础模块
 * 
 * @note 卸载顺序与安装顺序相反，确保依赖关系正确
 */
void NFKernelPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFICoroutineModule, NFCCoroutineModule);
	UNREGISTER_MODULE(m_pObjPluginManager, NFITaskModule, NFCTaskModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIMonitorModule, NFCMonitorModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIConsoleModule, NFCConsoleModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIKernelModule, NFCKernelModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIMessageModule, NFCMessageModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIEventModule, NFCEventModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFITimerModule, NFCTimerModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFILogModule, NFCLogModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIConfigModule, NFCConfigModule);
}

/**
 * @brief 初始化共享内存对象注册
 * @return 返回true表示初始化成功
 * 
 * 在共享内存模式下，注册框架需要的基础对象类型。
 * 当前实现为空，直接返回成功。
 * 
 * @note 如果需要在共享内存中注册特定对象类型，可以在此方法中实现
 */
bool NFKernelPlugin::InitShmObjectRegister()
{
	return true;
}
