// -------------------------------------------------------------------------
//    @FileName         :    NFProxyClientPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFProxyClientPlugin
//    @Desc             :    NFShmXFrame代理客户端插件实现
//                          实现代理客户端插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持客户端连接和消息处理
// -------------------------------------------------------------------------

#include "NFProxyClientPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFProxyClientModule.h"

#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建代理客户端插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFProxyClientPlugin)

};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁代理客户端插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFProxyClientPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFProxyClientPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFProxyClientPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFProxyClientPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册代理客户端模块到插件管理器
 */
void NFProxyClientPlugin::Install()
{
	REGISTER_MODULE(m_pObjPluginManager, NFIProxyClientModule, NFCProxyClientModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销代理客户端模块
 */
void NFProxyClientPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFIProxyClientModule, NFCProxyClientModule);
}
