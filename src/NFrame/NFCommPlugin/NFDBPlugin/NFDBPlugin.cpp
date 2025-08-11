// -------------------------------------------------------------------------
//    @FileName         :    NFDBPlugin.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFDBPlugin
//    @Description      :    数据库插件实现文件
//
// -------------------------------------------------------------------------

#include "NFDBPlugin.h"
#include "NFCMysqlModule.h"
#include "NFCNosqlModule.h"
#include "NFCAsyDBModule.h"
#include "NFCAsyNosqlModule.h"
#include "NFCAsyMysqlModule.h"

//
//
#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态库启动函数
 * @param pm 插件管理器指针
 * 
 * 当插件以动态库形式加载时，系统会调用此函数来创建插件实例
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
	CREATE_PLUGIN(pm, NFDBPlugin)
};

/**
 * @brief 动态库停止函数
 * @param pm 插件管理器指针
 * 
 * 当插件以动态库形式卸载时，系统会调用此函数来销毁插件实例
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
	DESTROY_PLUGIN(pm, NFDBPlugin)
};

#endif

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 * @return 返回版本号0
 */
int NFDBPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 * @return 返回插件类名"NFDBPlugin"
 */
std::string NFDBPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFDBPlugin);
}

/**
 * @brief 判断是否为动态加载插件
 * @return 返回false，表示此插件不支持动态加载
 */
bool NFDBPlugin::IsDynamicLoad()
{
	return false;
}

/**
 * @brief 安装插件，注册所有数据库相关模块
 * 
 * 按照以下顺序注册模块：
 * 1. MySQL同步模块 - 提供同步的MySQL数据库操作
 * 2. NoSQL模块 - 提供Redis等NoSQL数据库操作，支持Tick
 * 3. 异步数据库模块 - 提供异步的数据库操作，支持Tick
 * 4. 异步MySQL模块 - 提供异步的MySQL操作，支持Tick
 * 5. 异步NoSQL模块 - 提供异步的NoSQL操作
 */
void NFDBPlugin::Install()
{
    REGISTER_MODULE(m_pObjPluginManager, NFIMysqlModule, NFCMysqlModule);
    REGISTER_MODULE_TICK(m_pObjPluginManager, NFINosqlModule, NFCNosqlModule);
    REGISTER_MODULE_TICK(m_pObjPluginManager, NFIAsyDbModule, NFCAsyDbModule);
    REGISTER_MODULE_TICK(m_pObjPluginManager, NFIAsyMysqlModule, NFCAsyMysqlModule);
    REGISTER_MODULE(m_pObjPluginManager, NFIAsyNosqlModule, NFCAsyNosqlModule);
}

/**
 * @brief 卸载插件，注销所有数据库相关模块
 * 
 * 按照与安装相反的顺序注销模块，确保依赖关系正确处理
 */
void NFDBPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFIAsyDbModule, NFCAsyDBModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIMysqlModule, NFCMysqlModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIAsyMysqlModule, NFCAsyMysqlModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFIAsyNosqlModule, NFCAsyNosqlModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFINosqlModule, NFCNoSqlModule);
}
