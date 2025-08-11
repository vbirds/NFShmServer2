// -------------------------------------------------------------------------
//    @FileName         :    NFNavMeshServerPlugin.cpp
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFNavMeshServerPlugin
//    @Desc             :    NFShmXFrame导航网格服务器插件实现
//                          实现导航网格服务器插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持路径查找和导航计算
// -------------------------------------------------------------------------

#include "NFNavMeshServerPlugin.h"

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFNavMeshServerModule.h"


#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 *
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建导航网格服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFNavMeshServerPlugin)
};

/**
 * @brief 动态插件停止函数
 *
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁导航网格服务器插件实例
 *
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFNavMeshServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 *
 * @return 插件版本号，当前版本为0
 */
int NFNavMeshServerPlugin::GetPluginVersion()
{
    return 0;
}

/**
 * @brief 获取插件名称
 *
 * @return 插件名称字符串
 */
std::string NFNavMeshServerPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFNavMeshServerPlugin);
}

/**
 * @brief 安装插件
 *
 * 注册导航网格服务器模块到插件管理器，添加服务器类型
 */
void NFNavMeshServerPlugin::Install()
{
    NFGlobalSystem::Instance()->AddServerType(NF_ST_NAVMESH_SERVER);
    REGISTER_MODULE(m_pObjPluginManager, NFINavMeshServerModule, NFNavMeshServerModule);
}

/**
 * @brief 卸载插件
 *
 * 从插件管理器中注销导航网格服务器模块
 */
void NFNavMeshServerPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFINavMeshServerModule, NFNavMeshServerModule);
}

/**
 * @brief 初始化共享内存对象注册
 *
 * 注册导航网格服务器相关的共享内存对象
 *
 * @return 注册结果，true表示成功
 */
bool NFNavMeshServerPlugin::InitShmObjectRegister()
{
    return true;
}
