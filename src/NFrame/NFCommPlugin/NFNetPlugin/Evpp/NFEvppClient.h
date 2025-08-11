// -------------------------------------------------------------------------
//    @FileName         :    NFEvppClient.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    基于Evpp库实现的网络客户端类
//
// -------------------------------------------------------------------------
#pragma once

#include <evpp/event_loop_thread.h>

#include "NFIConnection.h"

class NFEvppObject;

/**
 * @file NFEvppClient.h
 * @brief Evpp客户端头文件
 * 
 * 该文件定义了基于evpp库的TCP客户端，包括：
 * - Evpp客户端类定义
 * - 客户端初始化和生命周期管理
 * - 客户端连接管理
 * - 异步消息处理
 * - 事件循环管理
 * 
 * 主要特性：
 * - 基于libevent的高性能事件驱动
 * - 支持异步非阻塞I/O
 * - 自动连接管理和重连机制
 * - 线程安全的消息处理
 * - 可配置的连接参数
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @class NFEvppClient
 * @brief 基于evpp库的网络客户端实现类
 * 
 * 该类继承自NFIConnection，基于libevent的evpp库实现高性能网络客户端功能
 * 主要用于TCP连接的客户端通信
 * 
 * 主要功能：
 * - TCP客户端连接管理
 * - 异步消息发送和接收
 * - 连接状态监控和重连
 * - 基于事件循环的高性能网络处理
 * - 支持多线程事件处理
 * 
 * 特性：
 * - 基于libevent的高性能事件驱动
 * - 支持异步非阻塞I/O
 * - 自动连接管理和重连机制
 * - 线程安全的消息处理
 * 
 * 使用方式：
 * - 创建客户端实例并初始化
 * - 连接到指定的服务器
 * - 发送和接收异步消息
 * - 监控连接状态
 * - 处理网络事件
 */
class NFEvppClient final : public NFIConnection
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
	NFEvppClient(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag):NFIConnection(p, serverType, flag)
	{
		m_eventLoop = nullptr;
		m_tcpClient = nullptr;
	}

    /**
     * @brief 初始化Evpp客户端
     * 
     * 创建TCP客户端连接，包括：
     * - 创建事件循环（如果未提供）
     * - 创建TCP客户端对象
     * - 设置连接和消息回调
     * - 配置自动重连参数
     * - 启动连接
     * 
     * @param loop 事件循环指针，如果为nullptr则创建新的事件循环
     * @return true 初始化成功，false 初始化失败
     */
	bool Init(evpp::EventLoop* loop);

    /**
     * @brief 关闭Evpp客户端
     * 
     * 断开TCP连接并停止事件循环
     * 
     * @return 关闭结果，0表示成功
     */
	int Shut() override;

    /**
     * @brief 释放Evpp客户端资源
     * 
     * 清理TCP客户端和事件循环对象
     * 
     * @return 释放结果，0表示成功
     */
	int Finalize() override;

private:
    /**
     * @brief 事件循环线程
     * 
     * 用于处理网络事件的独立线程
     */
    std::unique_ptr<evpp::EventLoopThread> m_eventLoop;

    /**
     * @brief TCP客户端对象
     * 
     * 管理TCP连接和消息处理
     */
    std::shared_ptr<evpp::TCPClient> m_tcpClient;
};
