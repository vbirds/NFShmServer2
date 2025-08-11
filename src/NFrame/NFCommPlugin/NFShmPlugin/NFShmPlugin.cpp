// -------------------------------------------------------------------------
//    @FileName         :    NFShmPlugin.cpp
//    @Author           :    gaoyi
//    @Date             :   2022-09-18
//    @Module           :    NFShmPlugin
//    @Desc             :    共享内存插件实现文件，提供共享内存管理功能。
//                          该文件实现了共享内存插件的核心功能，包括插件生命周期管理、
//                          共享内存对象注册、模块注册和卸载、动态加载支持。
//                          主要功能包括插件安装和卸载、共享内存对象类型注册、
//                          配置参数读取、模块依赖管理
//
// -------------------------------------------------------------------------

#include "NFShmPlugin.h"
#include "NFCShmMngModule.h"
#include "NFCShmOtherModule.h"

#include "NFShmGlobalId.h"
#include "NFShmTimer.h"
#include "NFShmTimerMng.h"
#include "NFShmTransMng.h"
#include "NFComm/NFObjCommon/NFTransBase.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFShmSubscribeInfo.h"
#include "NFShmEventMgr.h"

//
//
#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 * 
 * 创建共享内存插件实例
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
	CREATE_PLUGIN(pm, NFShmPlugin)
};

/**
 * @brief 动态插件停止函数
 * 
 * 销毁共享内存插件实例
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
	DESTROY_PLUGIN(pm, NFShmPlugin)
};

#endif

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本
 * 
 * @return 插件版本号，当前为0
 */
int NFShmPlugin::GetPluginVersion()
{
	return 0;
}

/**
 * @brief 获取插件名称
 * 
 * @return 插件名称字符串
 */
std::string NFShmPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFShmPlugin);
}

/**
 * @brief 是否支持动态加载
 * 
 * @return false，当前不支持动态加载
 */
bool NFShmPlugin::IsDynamicLoad()
{
	return false;
}

/**
 * @brief 安装插件
 * 
 * 注册共享内存管理模块和其他模块到插件管理器
 */
void NFShmPlugin::Install()
{
    REGISTER_MODULE_TICK(m_pObjPluginManager, NFIMemMngModule, NFCShmMngModule);
    REGISTER_MODULE(m_pObjPluginManager, NFCShmOtherModule, NFCShmOtherModule);
}

/**
 * @brief 卸载插件
 * 
 * 从插件管理器中注销共享内存管理模块和其他模块
 */
void NFShmPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFIMemMngModule, NFCShmMngModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFCShmOtherModule, NFCShmOtherModule);
}

/**
 * @brief 初始化共享内存对象注册
 * 
 * 根据配置参数注册所有共享内存对象类型，包括：
 * - 基础对象类型
 * - 全局ID管理器
 * - 定时器相关对象
 * - 事件管理器
 * - 事务管理器
 * 
 * @return true 注册成功，false 注册失败
 */
bool NFShmPlugin::InitShmObjectRegister()
{
    // 获取最大在线玩家数量
    uint32_t maxOnlinePlayerNum = 100;
    if (!m_pObjPluginManager->IsLoadAllServer())
    {
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
        NF_ASSERT(pConfig);

        maxOnlinePlayerNum = pConfig->MaxOnlinePlayerNum;
    }

    // 计算最大定时器数量
    uint32_t maxShmtimer = ALL_TIMER_COUNT/10 + maxOnlinePlayerNum*10;
    if (maxShmtimer >= ALL_TIMER_COUNT)
    {
        maxShmtimer = ALL_TIMER_COUNT;
    }

    // 计算最大事件数量
    uint32_t maxShmEvent = NF_SHM_EVENT_KEY_MAX_NUM/10 + maxOnlinePlayerNum*10;
    if (maxShmEvent >= NF_SHM_EVENT_KEY_MAX_NUM)
    {
        maxShmEvent = NF_SHM_EVENT_KEY_MAX_NUM;
    }

    // 注册共享内存对象类型
    REGISTER_SHM_OBJ(NFObject, 0);
    REGISTER_SINGLETON_SHM_OBJ(NFShmGlobalId);
	REGISTER_SHM_OBJ(NFShmTimer, maxShmtimer);
    REGISTER_SINGLETON_SHM_OBJ(NFShmTimerMng);
    REGISTER_SHM_OBJ(NFShmSubscribeInfo, maxShmEvent);
    REGISTER_SINGLETON_SHM_OBJ(NFShmEventMgr);
    REGISTER_SINGLETON_SHM_OBJ(NFShmTransMng);
    REGISTER_SHM_OBJ(NFTransBase, 0);
	return true;
}
