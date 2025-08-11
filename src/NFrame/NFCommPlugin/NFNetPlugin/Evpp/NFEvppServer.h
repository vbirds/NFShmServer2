// -------------------------------------------------------------------------
//    @FileName         :    NFEvppServer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    基于Evpp库的TCP服务器类
//
// -------------------------------------------------------------------------

#pragma once

#include <evpp/event_loop_thread.h>
#include <evpp/tcp_server.h>

#include "NFIConnection.h"

/**
 * @file NFEvppServer.h
 * @brief Evpp服务器头文件
 * 
 * 该文件定义了基于evpp库的TCP服务器，包括：
 * - Evpp服务器类定义
 * - 服务器初始化和生命周期管理
 * - 多客户端连接管理
 * - 异步消息处理
 * - 事件循环管理
 * 
 * 主要特性：
 * - 基于libevent的高性能事件驱动
 * - 支持异步非阻塞I/O
 * - 自动连接管理和资源清理
 * - 线程安全的消息处理
 * - 可配置的连接参数
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @class NFEvppServer
 * @brief 基于evpp库的TCP服务器实现类
 * 
 * 该类继承自NFIConnection，基于libevent的evpp库实现高性能TCP服务器功能
 * 主要用于处理多客户端TCP连接
 * 
 * 主要功能：
 * - TCP服务器监听和连接管理
 * - 多客户端并发连接处理
 * - 异步消息接收和发送
 * - 基于事件循环的高性能网络处理
 * - 支持多线程事件处理
 * 
 * 特性：
 * - 基于libevent的高性能事件驱动
 * - 支持异步非阻塞I/O
 * - 自动连接管理和资源清理
 * - 线程安全的消息处理
 * - 可配置的连接参数
 * 
 * 使用方式：
 * - 创建服务器实例并初始化
 * - 监听客户端连接请求
 * - 管理多个客户端连接
 * - 处理异步消息传输
 * - 监控连接状态和性能
 */
class NFEvppServer final : public NFIConnection
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
	NFEvppServer(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag):NFIConnection(p, serverType, flag)
	{
		m_eventLoop = nullptr;
		m_tcpServer = nullptr;
	}

    /**
     * @brief 初始化服务器
     * 
     * 初始化evpp服务器，包括：
     * - 创建事件循环线程
     * - 创建TCP服务器对象
     * - 设置连接和消息回调
     * - 启动服务器监听
     * 
     * @return 初始化结果，0表示成功
     */
	int Init() override;

    /**
     * @brief 关闭服务器
     * 
     * 关闭服务器并停止监听
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
     * @brief 事件循环线程
     * 
     * 用于处理网络事件的独立线程
     */
    std::unique_ptr<evpp::EventLoopThread> m_eventLoop;

    /**
     * @brief TCP服务器对象
     * 
     * 管理TCP连接和消息处理
     */
    std::unique_ptr<evpp::TCPServer> m_tcpServer;
};

