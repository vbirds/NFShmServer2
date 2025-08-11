// -------------------------------------------------------------------------
//    @FileName         :    NFDescStorePlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDescStorePlugin
//    @Desc             :    NFShmXFrame描述存储插件实现
//                          实现描述存储插件的核心功能，包括插件生命周期管理
//                          提供动态加载接口，支持配置表数据的存储和访问
//
// -------------------------------------------------------------------------

#include "NFDescStorePlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFServerComm/NFServerCommon/NFBaseDBObj.h"
#include "NFServerComm/NFServerCommon/NFDBObjTrans.h"
#include "NFServerComm/NFServerCommon/NFDBObjMgr.h"
#include "NFCDescStoreModule.h"
#include "NFServerComm/NFServerCommon/NFServerSyncDataObj.h"
#include "NFServerComm/NFServerCommon/NFServerSyncDataObjMgr.h"
#include "NFServerComm/NFServerCommon/NFServerSyncDataTrans.h"


#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 * 
 * 当插件以动态库形式加载时，此函数会被调用
 * 用于创建描述存储插件实例
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFDescStorePlugin)

};

/**
 * @brief 动态插件停止函数
 * 
 * 当插件以动态库形式卸载时，此函数会被调用
 * 用于销毁描述存储插件实例
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFDescStorePlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 * 
 * @return 插件版本号，当前版本为0
 */
int NFDescStorePlugin::GetPluginVersion()
{
    return 0;
}

/**
 * @brief 获取插件名称
 * 
 * @return 插件名称字符串
 */
std::string NFDescStorePlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFDescStorePlugin);
}

/**
 * @brief 安装插件
 * 
 * 注册描述存储模块到插件管理器，启用定时器功能
 */
void NFDescStorePlugin::Install()
{
    REGISTER_MODULE_TICK(m_pObjPluginManager, NFIDescStoreModule, NFCDescStoreModule);
}

/**
 * @brief 卸载插件
 * 
 * 从插件管理器中注销描述存储模块
 */
void NFDescStorePlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFIDescStoreModule, NFCDescStoreModule);
}

/**
 * @brief 初始化共享内存对象注册
 * 
 * 注册描述存储相关的共享内存对象，包括：
 * - 基础数据库对象
 * - 数据库事务对象
 * - 数据库对象管理器（单例）
 * - 服务器同步数据对象
 * - 服务器同步数据事务
 * - 服务器同步数据对象管理器（单例）
 * 
 * @return 注册是否成功
 */
bool NFDescStorePlugin::InitShmObjectRegister()
{
    REGISTER_SHM_OBJ(NFBaseDBObj, 0);
    REGISTER_SHM_OBJ(NFDBObjTrans, 100);
    REGISTER_SINGLETON_SHM_OBJ(NFDBObjMgr);
    REGISTER_SHM_OBJ(NFServerSyncDataObj, 0);
    REGISTER_SHM_OBJ(NFServerSyncDataTrans, 100);
    REGISTER_SINGLETON_SHM_OBJ(NFServerSyncDataObjMgr);
    return true;
}
