// -------------------------------------------------------------------------
//    @FileName         :    NFCBusServer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCBusServer
//    @Desc             :    Bus服务器实现类，负责进程间通信的服务器端。
//                          该文件定义了Bus进程间通信的服务器端实现，包括Bus服务器类定义、
//                          服务器端连接管理、消息路由和处理、共享内存通信管理、多客户端支持。
//                          主要特性包括基于共享内存的高性能通信、支持多客户端并发访问、
//                          消息队列管理、连接状态监控、自动资源清理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIBusConnection.h"
#include "../NFINetMessage.h"
#include "NFComm/NFPluginModule/NFNetDefine.h"

/**
 * @class NFCBusServer
 * @brief Bus服务器类，实现进程间通信的服务器端功能
 * 
 * 该类继承自NFIBusConnection，提供以下功能：
 * - 监听客户端连接请求
 * - 管理多个客户端连接
 * - 处理进程间消息路由
 * - 维护共享内存通信通道
 * - 消息广播和转发
 * 
 * 主要特性：
 * - 基于共享内存的高性能通信
 * - 支持多客户端并发访问
 * - 消息队列管理
 * - 连接状态监控
 * - 自动资源清理
 * 
 * 使用方式：
 * - 创建服务器实例并初始化
 * - 绑定服务器端口和配置
 * - 处理客户端连接和消息
 * - 管理共享内存资源
 */
class NFCBusServer final : public NFIBusConnection
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
    explicit NFCBusServer(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag);

    /**
     * @brief 析构函数，清理服务器资源
     */
    ~NFCBusServer() override;

    /**
     * @brief 定时器更新函数
     * @return 操作结果
     */
    int Tick() override;

    /**
     * @brief 初始化服务器
     * @return 操作结果
     */
    int Init() override;

    /**
     * @brief 关闭服务器
     * @return 操作结果
     */
    int Shut() override;

    /**
     * @brief 最终化服务器
     * @return 操作结果
     */
    int Finalize() override;

public:
    /**
     * @brief 绑定服务器
     * 
     * 初始化服务器并绑定到指定的消息标志
     * 
     * @param flag 消息标志，包含服务器配置信息
     * @return 绑定结果，成功返回服务器ID，失败返回0
     */
    uint64_t BindServer(const NFMessageFlag& flag);

    /**
     * @brief 主线程处理消息队列
     * 
     * 在主线程中处理接收到的消息队列
     * 包括消息解析、路由和分发
     */
    void ProcessMsgLogicThread();

    /**
     * @brief 发送原始数据
     * @param packet 数据包
     * @param msg 消息数据
     * @param nLen 数据长度
     * @return 发送结果
     */
    bool Send(NFDataPackage& packet, const char* msg, uint32_t nLen) override;

    /**
     * @brief 发送Protobuf消息
     * @param packet 数据包
     * @param xData Protobuf消息对象
     * @return 发送结果
     */
    bool Send(NFDataPackage& packet, const google::protobuf::Message& xData) override;

private:
    /** @brief 服务器每一帧处理的消息数 */
    uint32_t m_handleMsgNumPerFrame;
    /** @brief 是否完成连接初始化 */
    bool m_isFinishConnect;
};
