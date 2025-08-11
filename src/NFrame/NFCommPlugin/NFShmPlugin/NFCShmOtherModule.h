// -------------------------------------------------------------------------
//    @FileName         :    NFCShmOtherModule.h
//    @Author           :    gaoyi
//    @Date             :    23-9-8
//    @Email			:    445267987@qq.com
//    @Module           :    NFCShmOtherModule
//    @Desc             :    共享内存其他模块头文件，提供共享内存相关的辅助功能。
//                          该文件定义了共享内存其他模块类，提供共享内存相关的辅助功能、
//                          事件处理机制、服务器状态管理、动态模块生命周期管理。
//                          主要功能包括模块初始化和清理、事件执行处理、服务器停止检查、
//                          服务器停止处理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFIDynamicModule.h"

/**
 * @brief 共享内存其他模块类
 * 
 * 提供共享内存相关的辅助功能和事件处理
 * 继承自NFIDynamicModule，实现动态模块接口
 */
class NFCShmOtherModule final : public NFIDynamicModule
{
public:
    /**
     * @brief 构造函数
     * 
     * @param p 插件管理器指针
     */
    explicit NFCShmOtherModule(NFIPluginManager* p);

    /**
     * @brief 析构函数
     * 
     * 清理模块资源
     */
    ~NFCShmOtherModule() override;

public:
    /**
     * @brief 唤醒函数
     * 
     * 模块唤醒时的处理
     * 
     * @return 处理结果
     */
    int Awake() override;

    /**
     * @brief 事件执行处理
     * 
     * 处理指定的事件
     * 
     * @param serverType 服务器类型
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @param pMessage 消息指针
     * @return 处理结果
     */
    int OnExecute(uint32_t serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message* pMessage) override;

    /**
     * @brief 检查停止服务器
     * 
     * 检查是否可以停止服务器
     * 
     * @return 检查结果
     */
    int CheckStopServer() override;

    /**
     * @brief 停止服务器
     * 
     * 执行服务器停止操作
     * 
     * @return 停止结果
     */
    int StopServer() override;
};
