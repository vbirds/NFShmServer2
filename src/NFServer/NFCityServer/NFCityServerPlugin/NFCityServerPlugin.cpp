// -------------------------------------------------------------------------
//    @FileName         :    NFCityServerPlugin.cpp
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFCityServerPlugin
//    @Desc             :    NFShmXFrame城市服务器插件实现
//                          实现城市服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持城市管理和区域服务
// -------------------------------------------------------------------------

#include "NFCityServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFCityServerModule.h"


#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建城市服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFCityServerPlugin)
};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁城市服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFCityServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFCityServerPlugin::GetPluginVersion()
{
    return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFCityServerPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFCityServerPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册城市服务器模块到插件管理器，添加服务器类型
 */
void NFCityServerPlugin::Install()
{
    NFGlobalSystem::Instance()->AddServerType(NF_ST_CITY_SERVER);
    REGISTER_MODULE(m_pObjPluginManager, NFICityServerModule, NFCityServerModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销城市服务器模块
 */
void NFCityServerPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFICityServerModule, NFCityServerModule);
}

/**
 * @brief 初始化共享内存对象注册
 *
 * 注册城市服务器相关的共享内存对象
 *
 * @return 注册结果，true表示成功
 */
bool NFCityServerPlugin::InitShmObjectRegister()
{
    return true;
}