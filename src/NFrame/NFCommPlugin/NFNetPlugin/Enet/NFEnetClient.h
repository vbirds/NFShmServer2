// -------------------------------------------------------------------------
//    @FileName         :    NFEnetClient.h
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFEnetClient
//    @Desc             :    基于ENet库实现的UDP可靠传输客户端类。
//                          该文件定义了基于ENet库的UDP可靠传输客户端，包括Enet客户端类定义、
//                          客户端初始化和生命周期管理、客户端连接管理、数据包的有序传输、连接状态监控。
//                          主要特性包括低延迟网络通信、内置数据包重传机制、支持数据包分片和重组、
//                          适合实时游戏网络通信、自动重连和流量控制
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIEnetConnection.h"

/**
 * @class NFEnetClient
 * @brief 基于ENet库的UDP可靠传输客户端实现类
 * 
 * 该类继承自NFIEnetConnection，基于ENet库实现UDP协议的可靠传输客户端功能
 * ENet是一个轻量级的网络库，专门为游戏设计，提供UDP上的可靠有序数据传输
 * 
 * 主要功能：
 * - UDP协议的可靠传输
 * - 客户端连接管理
 * - 数据包的有序传输
 * - 连接状态监控
 * - 自动重传和流量控制
 * 
 * 特性：
 * - 低延迟网络通信
 * - 内置数据包重传机制
 * - 支持数据包分片和重组
 * - 适合实时游戏网络通信
 * - 自动重连机制
 * 
 * 使用方式：
 * - 创建客户端实例并初始化
 * - 连接到指定的服务器
 * - 发送和接收数据包
 * - 监控连接状态
 * - 处理网络事件
 */
class NFEnetClient final : public NFIEnetConnection
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
    NFEnetClient(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag): NFIEnetConnection(p, serverType, flag), m_pHost(nullptr)
    {
    }

    /**
     * @brief 初始化客户端
     * 
     * 初始化ENet客户端，包括：
     * - 创建ENet主机对象
     * - 设置连接参数
     * - 准备连接服务器
     * 
     * @return 初始化结果，0表示成功
     */
    int Init() override;

    /**
     * @brief 客户端心跳处理
     * 
     * 处理客户端的定时任务，包括：
     * - 处理网络事件
     * - 管理连接状态
     * - 处理数据包传输
     * - 心跳检测
     * 
     * @return 处理结果，0表示成功
     */
    int Tick() override;

    /**
     * @brief 关闭客户端
     * 
     * 关闭客户端连接并清理资源
     * 
     * @return 关闭结果，0表示成功
     */
    int Shut() override;

    /**
     * @brief 释放客户端资源
     * 
     * 释放所有客户端占用的资源
     * 
     * @return 释放结果，0表示成功
     */
    int Finalize() override;

private:
    /**
     * @brief ENet主机对象指针
     * 
     * 用于管理UDP客户端连接和事件处理
     */
    ENetHost* m_pHost;
};
