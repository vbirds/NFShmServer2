// -------------------------------------------------------------------------
//    @FileName         :    NFMemPlugin.cpp
//    @Author           :    gaoyi
//    @Date             :   2022-09-18
//    @Module           :    NFMemPlugin
//    @Desc             :    内存插件实现文件，负责内存相关模块的注册和管理。
//                          该文件实现了NFShmXFrame框架的内存插件类，提供内存管理相关模块的生命周期管理、
//                          插件注册与卸载、动态加载支持、共享内存对象注册等功能。
//                          主要功能包括插件生命周期管理、内存管理模块注册、动态加载支持、
//                          共享内存对象注册与初始化
//
// -------------------------------------------------------------------------

#include "NFMemPlugin.h"
#include "NFCMemMngModule.h"
#include "NFCMemOtherModule.h"

#include "NFMemGlobalId.h"
#include "NFMemTimer.h"
#include "NFMemTimerMng.h"
#include "NFMemTransMng.h"
#include "NFComm/NFObjCommon/NFTransBase.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFMemSubscribeInfo.h"
#include "NFMemEventMgr.h"

//
//
#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
	CREATE_PLUGIN(pm, NFMemPlugin)
};

/**
 * @brief 动态插件停止函数
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
	DESTROY_PLUGIN(pm, NFMemPlugin)
};

#endif

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 * @return 插件版本号
 */
int NFMemPlugin::GetPluginVersion()
{
    return 0;
}

/**
 * @brief 获取插件名称
 * @return 插件名称字符串
 */
std::string NFMemPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFMemPlugin);
}

/**
 * @brief 判断是否为动态加载插件
 * @return 是否为动态加载
 */
bool NFMemPlugin::IsDynamicLoad()
{
    return false;
}

/**
 * @brief 安装插件，注册相关模块
 * 
 * 在插件安装时调用，负责注册内存管理相关的模块到插件管理器中
 */
void NFMemPlugin::Install()
{
    REGISTER_MODULE_TICK(m_pObjPluginManager, NFIMemMngModule, NFCMemMngModule);
    REGISTER_MODULE(m_pObjPluginManager, NFCMemOtherModule, NFCMemOtherModule);
}

/**
 * @brief 卸载插件，注销相关模块
 * 
 * 在插件卸载时调用，负责从插件管理器中注销内存管理相关的模块
 */
void NFMemPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFIMemMngModule, NFCMemMngModule);
    UNREGISTER_MODULE(m_pObjPluginManager, NFCMemOtherModule, NFCMemOtherModule);
}

/**
 * @brief 初始化共享内存对象注册
 * 
 * 负责注册各种共享内存对象类型，包括全局ID管理器、定时器管理器、
 * 事件管理器、事务管理器等，并设置相应的对象数量限制
 * @return 初始化是否成功
 */
bool NFMemPlugin::InitShmObjectRegister()
{
    // 设置默认最大在线玩家数量
    uint32_t maxOnlinePlayerNum = 100;
    if (!m_pObjPluginManager->IsLoadAllServer())
    {
        // 获取应用配置，设置最大在线玩家数量
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
        NF_ASSERT(pConfig);

        maxOnlinePlayerNum = pConfig->MaxOnlinePlayerNum;
    }

    // 计算最大定时器数量：基础数量 + 玩家数量 * 10
    uint32_t maxShmtimer = ALL_TIMER_COUNT / 10 + maxOnlinePlayerNum * 10;
    if (maxShmtimer >= ALL_TIMER_COUNT)
    {
        maxShmtimer = ALL_TIMER_COUNT;
    }

    // 计算最大事件数量：基础数量 + 玩家数量 * 10
    uint32_t maxShmEvent = NF_SHM_EVENT_KEY_MAX_NUM / 10 + maxOnlinePlayerNum * 10;
    if (maxShmEvent >= NF_SHM_EVENT_KEY_MAX_NUM)
    {
        maxShmEvent = NF_SHM_EVENT_KEY_MAX_NUM;
    }

    // 注册各种共享内存对象
    REGISTER_SHM_OBJ(NFObject, 0);                    // 基础对象
    REGISTER_SINGLETON_SHM_OBJ(NFMemGlobalId);        // 全局ID管理器（单例）
    REGISTER_SHM_OBJ(NFMemTimer, maxShmtimer);        // 定时器对象
    REGISTER_SINGLETON_SHM_OBJ(NFMemTimerMng);        // 定时器管理器（单例）
    REGISTER_SHM_OBJ(NFMemSubscribeInfo, maxShmEvent); // 订阅信息对象
    REGISTER_SINGLETON_SHM_OBJ(NFMemEventMgr);        // 事件管理器（单例）
    REGISTER_SINGLETON_SHM_OBJ(NFMemTransMng);        // 事务管理器（单例）
    REGISTER_SHM_OBJ(NFTransBase, 0);                 // 事务基础对象
    return true;
}
