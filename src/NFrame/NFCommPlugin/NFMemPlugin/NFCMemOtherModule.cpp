// -------------------------------------------------------------------------
//    @FileName         :    NFCMemOtherModule.cpp
//    @Author           :    gaoyi
//    @Date             :    23-9-8
//    @Email			:    445267987@qq.com
//    @Module           :    NFCMemOtherModule
//    @Desc             :    内存辅助模块实现文件，提供内存相关的辅助功能和动态模块支持。
//                          该文件实现了NFShmXFrame框架的内存辅助模块，负责内存相关的辅助功能、
//                          事件处理机制、服务器状态检查、动态模块生命周期管理等。
//                          主要功能包括模块初始化和清理、事件执行处理、服务器停止检查、
//                          服务器停止处理、动态模块支持
//
// -------------------------------------------------------------------------

#include "NFCMemOtherModule.h"

#include <NFComm/NFCore/NFServerTime.h>

#include "NFMemTransMng.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"

/**
 * @brief 构造函数
 * @param p 插件管理器指针
 */
NFCMemOtherModule::NFCMemOtherModule(NFIPluginManager* p): NFIDynamicModule(p)
{
}

/**
 * @brief 析构函数
 */
NFCMemOtherModule::~NFCMemOtherModule()
{
}

/**
 * @brief 模块唤醒函数
 * 
 * 该函数在模块唤醒时被调用，负责模块的初始化工作。
 * 目前该函数为空实现，可以根据需要添加初始化逻辑。
 * 
 * @return 初始化结果，0表示成功
 */
int NFCMemOtherModule::Awake()
{
    return 0;
}

/**
 * @brief 事件执行函数
 * 
 * 该函数用于处理动态模块接收到的事件。
 * 目前该函数为空实现，可以根据需要添加事件处理逻辑。
 * 
 * @param serverType 服务器类型
 * @param eventId 事件ID
 * @param srcType 源类型
 * @param srcId 源ID
 * @param pMessage 事件消息
 * @return 处理结果，0表示成功
 */
int NFCMemOtherModule::OnExecute(uint32_t serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message* pMessage)
{
    return 0;
}

/**
 * @brief 检查服务器停止状态
 * 
 * 该函数用于检查服务器是否需要停止。
 * 目前该函数为空实现，可以根据需要添加服务器状态检查逻辑。
 * 
 * @return 检查结果，0表示成功
 */
int NFCMemOtherModule::CheckStopServer()
{
    return 0;
}

/**
 * @brief 停止服务器
 * 
 * 该函数用于执行服务器停止操作。
 * 目前该函数为空实现，可以根据需要添加服务器停止逻辑。
 * 
 * @return 停止结果，0表示成功
 */
int NFCMemOtherModule::StopServer()
{
    return 0;
}


