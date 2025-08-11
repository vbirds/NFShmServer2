// -------------------------------------------------------------------------
//    @FileName         :    NFSnsServerPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFSnsServerPlugin
//    @Desc             :    NFShmXFrame社交网络服务器插件实现
//                          实现社交网络服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持社交功能和用户互动
// -------------------------------------------------------------------------

#include "NFSnsServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFSnsServerModule.h"


#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建社交网络服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFSnsServerPlugin)

};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁社交网络服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFSnsServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFSnsServerPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFSnsServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFSnsServerPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册社交网络服务器模块到插件管理器，添加服务器类型
 */
void NFSnsServerPlugin::Install()
{
    NFGlobalSystem::Instance()->AddServerType(NF_ST_SNS_SERVER);
	REGISTER_MODULE(m_pObjPluginManager, NFISnsServerModule, NFCSnsServerModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销社交网络服务器模块
 */
void NFSnsServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFISnsServerModule, NFCSnsServerModule);
}

/**
 * @brief 初始化共享内存对象注册
 *
 * 注册社交网络服务器相关的共享内存对象
 *
 * @return 注册结果，true表示成功
 */
bool NFSnsServerPlugin::InitShmObjectRegister()
{
    //NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_SNS_SERVER);
    //NF_ASSERT(pConfig);

    //uint32_t maxOnlinePlayerNum = pConfig->mMaxOnlinePlayerNum;

    return true;
}