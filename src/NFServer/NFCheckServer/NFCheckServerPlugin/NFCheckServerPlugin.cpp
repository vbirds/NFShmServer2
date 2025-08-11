// -------------------------------------------------------------------------
//    @FileName         :    NFCheckServerPlugin.cpp
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFCheckServerPlugin
//    @Desc             :    NFShmXFrame检查服务器插件实现
//                          实现检查服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持系统健康检查和监控
// -------------------------------------------------------------------------

#include "NFCheckServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFCheckServerModule.h"


#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建检查服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFCheckServerPlugin)
};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁检查服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFCheckServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFCheckServerPlugin::GetPluginVersion()
{
    return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFCheckServerPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFCheckServerPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册检查服务器模块到插件管理器，添加服务器类型
 */
void NFCheckServerPlugin::Install()
{
    NFGlobalSystem::Instance()->AddServerType(NF_ST_CHECK_SERVER);
    REGISTER_MODULE(m_pObjPluginManager, NFICheckServerModule, NFCheckServerModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销检查服务器模块
 */
void NFCheckServerPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFICheckServerModule, NFCheckServerModule);
}

/**
 * @brief 初始化共享内存对象注册
 *
 * 注册检查服务器相关的共享内存对象
 *
 * @return 注册结果，true表示成功
 */
bool NFCheckServerPlugin::InitShmObjectRegister()
{
    return true;
}
