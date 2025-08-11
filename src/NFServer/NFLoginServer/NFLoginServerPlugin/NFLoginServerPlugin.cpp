// -------------------------------------------------------------------------
//    @FileName         :    NFLoginServerPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFLoginServerPlugin
//    @Desc             :    NFShmXFrame登录服务器插件实现
//                          实现登录服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持用户认证和登录管理
// -------------------------------------------------------------------------

#include "NFLoginServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFLoginServerModule.h"

#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建登录服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFLoginServerPlugin)

};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁登录服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFLoginServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFLoginServerPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFLoginServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFLoginServerPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册登录服务器模块到插件管理器，添加服务器类型
 */
void NFLoginServerPlugin::Install()
{
	NFGlobalSystem::Instance()->AddServerType(NF_ST_LOGIN_SERVER);
	REGISTER_MODULE(m_pObjPluginManager, NFILoginServerModule, NFCLoginServerModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销登录服务器模块
 */
void NFLoginServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFILoginServerModule, NFCLoginServerModule);
}

/**
 * @brief 初始化共享内存对象注册
 *
 * 注册登录服务器相关的共享内存对象
 *
 * @return 注册结果，true表示成功
 */
bool NFLoginServerPlugin::InitShmObjectRegister()
{
	return true;
}