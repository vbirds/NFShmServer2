// -------------------------------------------------------------------------
//    @FileName         :    NFEnetServer.h
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFEnetServer
//    @Desc             :    基于ENet库实现的UDP可靠传输服务器类。
//                          该文件定义了基于ENet库的UDP可靠传输服务器，包括Enet服务器类定义、
//                          服务器初始化和生命周期管理、多客户端连接管理、数据包的有序传输和路由、
//                          连接状态监控和管理。
//                          主要特性包括低延迟网络通信、内置数据包重传机制、支持数据包分片和重组、
//                          适合实时游戏网络通信、支持多客户端并发连接
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIEnetConnection.h"

/**
 * @class NFEnetServer
 * @brief 基于ENet库的UDP可靠传输服务器实现类
 * 
 * 该类继承自NFIEnetConnection，基于ENet库实现UDP协议的可靠传输服务器功能
 * ENet是一个轻量级的网络库，专门为游戏设计，提供UDP上的可靠有序数据传输
 * 
 * 主要功能：
 * - UDP协议的可靠传输服务
 * - 多客户端连接管理
 * - 数据包的有序传输和路由
 * - 连接状态监控和管理
 * - 自动重传和流量控制
 * 
 * 特性：
 * - 低延迟网络通信
 * - 内置数据包重传机制
 * - 支持数据包分片和重组
 * - 适合实时游戏网络通信
 * - 支持多客户端并发连接
 * 
 * 使用方式：
 * - 创建服务器实例并初始化
 * - 监听客户端连接请求
 * - 管理多个客户端连接
 * - 处理数据包传输和路由
 * - 监控连接状态和性能
 */
class NFEnetServer : public NFIEnetConnection
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
    NFEnetServer(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag);
    
    /**
     * @brief 析构函数
     */
    ~NFEnetServer() override;

public:
    /**
     * @brief 初始化服务器
     * 
     * 初始化ENet服务器，包括：
     * - 创建ENet主机对象
     * - 绑定服务器端口
     * - 设置连接回调
     * 
     * @return 初始化结果，0表示成功
     */
    int Init() override;

    /**
     * @brief 服务器心跳处理
     * 
     * 处理服务器的定时任务，包括：
     * - 处理网络事件
     * - 管理连接状态
     * - 处理数据包传输
     * 
     * @return 处理结果，0表示成功
     */
    int Tick() override;

    /**
     * @brief 关闭服务器
     * 
     * 关闭服务器并清理资源
     * 
     * @return 关闭结果，0表示成功
     */
    int Shut() override;

    /**
     * @brief 释放服务器资源
     * 
     * 释放所有服务器占用的资源
     * 
     * @return 释放结果，0表示成功
     */
    int Finalize() override;

private:
    /**
     * @brief ENet主机对象指针
     * 
     * 用于管理UDP服务器连接和事件处理
     */
    ENetHost* m_pHost;
};
