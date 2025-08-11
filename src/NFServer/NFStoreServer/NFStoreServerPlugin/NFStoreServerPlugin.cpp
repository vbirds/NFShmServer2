// -------------------------------------------------------------------------
//    @FileName         :    NFStoreServerPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFStoreServerPlugin
//    @Desc             :    NFShmXFrame存储服务器插件实现
//                          实现存储服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持数据库操作和RPC服务
//
// -------------------------------------------------------------------------

#include "NFStoreServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFCStoreServerModule.h"

#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 * 
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建存储服务器插件实例
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFStoreServerPlugin)

};

/**
 * @brief 动态插件停止函数
 * 
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁存储服务器插件实例
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFStoreServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 * 
 * @return 插件版本号，当前版本为0
 */
int NFStoreServerPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 * 
 * @return 插件名称字符串
 */
std::string NFStoreServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFStoreServerPlugin);
}

/**
 * @brief 安装插件
 * 
 * 注册存储服务器模块到插件管理器，添加服务器类型
 */
void NFStoreServerPlugin::Install()
{
	NFGlobalSystem::Instance()->AddServerType(NF_ST_STORE_SERVER);
	REGISTER_MODULE(m_pObjPluginManager, NFIStoreServerModule, NFCStoreServerModule);
}

/**
 * @brief 卸载插件
 * 
 * 从插件管理器中注销存储服务器模块
 */
void NFStoreServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFIStoreServerModule, NFCStoreServerModule);
}
