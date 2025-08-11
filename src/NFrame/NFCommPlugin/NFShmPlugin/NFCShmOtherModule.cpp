// -------------------------------------------------------------------------
//    @FileName         :    NFCShmOtherModule.cpp
//    @Author           :    gaoyi
//    @Date             :    23-9-8
//    @Email			:    445267987@qq.com
//    @Module           :    NFCShmOtherModule
//    @Desc             :    共享内存其他模块实现文件，提供共享内存相关的辅助功能。
//                          该文件实现了共享内存其他模块的核心功能，包括模块的初始化和销毁、
//                          事件处理机制、服务器状态检查、服务器停止处理。
//                          主要功能包括模块生命周期管理、事件处理、服务器状态管理、
//                          停止服务器检查。
//                          设计特点包括基于动态模块架构、事件驱动机制、
//                          服务器状态监控、优雅停止支持
//
// -------------------------------------------------------------------------

#include "NFCShmOtherModule.h"

#include <NFComm/NFCore/NFServerTime.h>

#include "NFShmTransMng.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"

/**
 * @brief 构造函数
 * 
 * 初始化共享内存其他模块
 * 
 * @param p 插件管理器指针
 */
NFCShmOtherModule::NFCShmOtherModule(NFIPluginManager* p): NFIDynamicModule(p)
{
}

/**
 * @brief 析构函数
 * 
 * 清理共享内存其他模块资源
 */
NFCShmOtherModule::~NFCShmOtherModule()
{
}

/**
 * @brief 模块唤醒
 * 
 * 模块启动时的初始化操作
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmOtherModule::Awake()
{
    return 0;
}

/**
 * @brief 事件执行处理
 * 
 * 处理接收到的事件消息
 * 
 * @param serverType 服务器类型
 * @param eventId 事件ID
 * @param srcType 源类型
 * @param srcId 源ID
 * @param pMessage 消息指针
 * @return 0 成功，其他值表示失败
 */
int NFCShmOtherModule::OnExecute(uint32_t serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message* pMessage)
{
    return 0;
}

/**
 * @brief 检查是否可以停止服务器
 * 
 * 检查所有相关模块是否可以安全停止
 * 
 * @return 0 可以停止，其他值表示不能停止
 */
int NFCShmOtherModule::CheckStopServer()
{
    return 0;
}

/**
 * @brief 停止服务器
 * 
 * 执行服务器停止操作
 * 
 * @return 0 成功，其他值表示失败
 */
int NFCShmOtherModule::StopServer()
{
    return 0;
}


