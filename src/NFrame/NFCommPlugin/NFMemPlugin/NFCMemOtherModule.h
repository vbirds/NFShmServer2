// -------------------------------------------------------------------------
//    @FileName         :    NFCMemOtherModule.h
//    @Author           :    gaoyi
//    @Date             :    23-9-8
//    @Email			:    445267987@qq.com
//    @Module           :    NFCMemOtherModule
//    @Desc             :    内存辅助模块头文件，提供内存相关的辅助功能和动态模块支持。
//                          该文件定义了NFShmXFrame框架的内存辅助模块，负责内存相关的辅助功能、
//                          事件处理机制、服务器状态检查、动态模块生命周期管理等。
//                          主要功能包括模块初始化和清理、事件执行处理、服务器停止检查、
//                          服务器停止处理、动态模块支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFIDynamicModule.h"

/**
 * @brief 内存辅助模块类，提供内存相关的辅助功能和动态模块支持
 * 
 * 该类继承自NFIDynamicModule，负责内存相关的辅助功能、
 * 事件处理机制、服务器状态检查、动态模块生命周期管理等。
 * 提供模块初始化和清理、事件执行处理、服务器停止检查、
 * 服务器停止处理、动态模块支持等功能。
 */
class NFCMemOtherModule final : public NFIDynamicModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    explicit NFCMemOtherModule(NFIPluginManager* p);

    /**
     * @brief 析构函数
     */
    ~NFCMemOtherModule() override;

public:
    /**
     * @brief 模块唤醒函数
     * @return 执行结果
     */
    int Awake() override;

    /**
     * @brief 事件执行处理函数
     * @param serverType 服务器类型
     * @param eventId 事件ID
     * @param srcType 事件源类型
     * @param srcId 事件源ID
     * @param pMessage 事件消息
     * @return 执行结果
     */
    int OnExecute(uint32_t serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message* pMessage) override;

    /**
     * @brief 检查服务器停止状态
     * @return 检查结果
     */
    int CheckStopServer() override;

    /**
     * @brief 停止服务器
     * @return 停止结果
     */
    int StopServer() override;
};
