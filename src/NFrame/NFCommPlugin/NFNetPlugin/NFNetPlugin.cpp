//------------------------------------------------------------------------ -
//    @FileName         :    NFNetPlugin.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFNetPlugin
//    @Desc             :    网络插件实现，负责网络模块的生命周期管理
//
// -------------------------------------------------------------------------

#include "NFNetPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFCNetModule.h"

/**
 * @file NFNetPlugin.cpp
 * @brief 网络插件实现文件
 * 
 * 该文件实现了网络插件的核心功能，包括：
 * - 插件生命周期管理
 * - 动态库导出函数
 * - 模块注册和注销
 * - 插件版本管理
 * 
 * 主要功能：
 * - 提供网络通信基础功能
 * - 管理网络模块的生命周期
 * - 支持动态加载和卸载
 * - 提供插件版本管理
 * 
 * @author LvSheng.Huang
 * @date 2022-09-18
 * @version 1.0
 */

#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态库启动插件入口函数
 * 
 * 当动态库被加载时调用，负责：
 * - 创建网络插件实例
 * - 注册到插件管理器
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
	CREATE_PLUGIN(pm, NFNetPlugin)
};

/**
 * @brief 动态库停止插件入口函数
 * 
 * 当动态库被卸载时调用，负责：
 * - 销毁网络插件实例
 * - 从插件管理器中注销
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
	DESTROY_PLUGIN(pm, NFNetPlugin)
};

#endif

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 * 
 * 返回网络插件的版本号，用于版本管理和兼容性检查
 * 
 * @return 插件版本号，当前版本为0
 */
int NFNetPlugin::GetPluginVersion()
{
	// 返回插件版本号，当前版本为0
	return 0;
}

/**
 * @brief 获取插件名称
 * 
 * 返回网络插件的类名，用于插件识别和管理
 * 
 * @return 插件类名字符串
 */
std::string NFNetPlugin::GetPluginName()
{
	// 返回插件类名
	return GET_CLASS_NAME(NFNetPlugin);
}

/**
 * @brief 检查插件是否支持动态加载
 * 
 * 该插件不支持动态加载，必须静态链接
 * 
 * @return false 不支持动态加载
 */
bool NFNetPlugin::IsDynamicLoad()
{
	// 该插件不支持动态加载
	return false;
}

/**
 * @brief 安装插件，注册插件模块
 * 
 * 在插件安装时调用，负责：
 * - 注册网络模块到插件管理器
 * - 启用心跳处理功能
 * - 初始化网络相关资源
 */
void NFNetPlugin::Install()
{
	// 注册网络模块到插件管理器，启用心跳处理
	REGISTER_MODULE_TICK(m_pObjPluginManager, NFINetModule, NFCNetModule);
}

/**
 * @brief 卸载插件，清理插件资源
 * 
 * 在插件卸载时调用，负责：
 * - 从插件管理器中注销网络模块
 * - 清理网络相关资源
 * - 停止所有网络服务
 */
void NFNetPlugin::Uninstall()
{
	// 从插件管理器中注销网络模块
	UNREGISTER_MODULE(m_pObjPluginManager, NFINetModule, NFCNetModule);
}
