// -------------------------------------------------------------------------
//    @FileName         :    NFMatchServerPlugin.cpp
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFMatchServerPlugin
//    @Desc             :    NFShmXFrame匹配服务器插件实现
//                          实现匹配服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持玩家匹配和队列管理
// -------------------------------------------------------------------------

#include "NFMatchServerPlugin.h"

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFMatchServerModule.h"


#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建匹配服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFMatchServerPlugin)
};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁匹配服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFMatchServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFMatchServerPlugin::GetPluginVersion()
{
    return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFMatchServerPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFMatchServerPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册匹配服务器模块到插件管理器，添加服务器类型
 */
void NFMatchServerPlugin::Install()
{
    NFGlobalSystem::Instance()->AddServerType(NF_ST_MATCH_SERVER);
    REGISTER_MODULE(m_pObjPluginManager, NFIMatchServerModule, NFMatchServerModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销匹配服务器模块
 */
void NFMatchServerPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFIMatchServerModule, NFMatchServerModule);
}

/**
 * @brief 初始化共享内存对象注册
 *
 * 注册匹配服务器相关的共享内存对象
 *
 * @return 注册结果，true表示成功
 */
bool NFMatchServerPlugin::InitShmObjectRegister()
{
    return true;
}
