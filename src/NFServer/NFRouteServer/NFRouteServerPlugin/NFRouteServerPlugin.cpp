// -------------------------------------------------------------------------
//    @FileName         :    NFRouteServerPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFRouteServerPlugin
//    @Desc             :    NFShmXFrame路由服务器插件实现
//                          实现路由服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持消息路由和负载均衡
// -------------------------------------------------------------------------

#include "NFRouteServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFCRouteServerModule.h"

#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建路由服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFRouteServerPlugin)

};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁路由服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFRouteServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFRouteServerPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFRouteServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFRouteServerPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册路由服务器模块到插件管理器，添加服务器类型
 */
void NFRouteServerPlugin::Install()
{
	NFGlobalSystem::Instance()->AddServerType(NF_ST_ROUTE_SERVER);
	REGISTER_MODULE_TICK(m_pObjPluginManager, NFIRouteServerModule, NFCRouteServerModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销路由服务器模块
 */
void NFRouteServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFIRouteServerModule, NFCRouteServerModule);
}
