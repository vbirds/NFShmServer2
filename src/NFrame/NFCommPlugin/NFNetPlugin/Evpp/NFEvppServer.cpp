// -------------------------------------------------------------------------
//    @FileName         :    NFEvppServer.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    基于Evpp库的TCP服务器实现文件
//
// -------------------------------------------------------------------------

#include "NFEvppServer.h"

#include <evpp/tcp_server.h>

/**
 * @file NFEvppServer.cpp
 * @brief Evpp服务器实现文件
 * 
 * 该文件实现了基于evpp库的TCP服务器，包括：
 * - 服务器的初始化和销毁
 * - 网络事件的处理
 * - 连接管理
 * - 数据包处理
 * - 错误处理和日志记录
 * 
 * 主要功能：
 * - 创建和管理evpp TCP服务器
 * - 监听客户端连接
 * - 处理连接和断开事件
 * - 处理数据接收事件
 * - 提供回调机制
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief 初始化Evpp服务器
 * 
 * 创建TCP服务器并开始监听，包括：
 * - 创建事件循环线程
 * - 设置监听地址和端口
 * - 创建TCP服务器对象
 * - 设置连接和消息回调
 * - 启动服务器监听
 * 
 * @return 初始化结果，0表示成功，-1表示失败
 */
int NFEvppServer::Init()
{
    // 创建事件循环线程
    m_eventLoop.reset(NF_NEW evpp::EventLoopThread());
    m_eventLoop->set_name(GetServerName(m_serverType));
    m_eventLoop->Start(true);

    // 设置监听地址
    std::string listenAddr;
    if (m_flag.mStrIp == "127.0.0.1")
    {
        listenAddr = NF_FORMAT("{}:{}", m_flag.mStrIp, m_flag.nPort);
    }
    else
    {
        listenAddr = NF_FORMAT("0.0.0.0:{}", m_flag.nPort);
    }

    // 创建TCP服务器
    m_tcpServer.reset(NF_NEW evpp::TCPServer(m_eventLoop->loop(), listenAddr, GetServerName(m_serverType), m_flag.nNetThreadNum, m_flag.mMaxConnectNum));
    if (!m_tcpServer)
    {
        return -1;
    }

    // 设置连接类型
    m_connectionType = NF_CONNECTION_TYPE_TCP_SERVER;

    // 设置连接回调（在事件循环线程中运行）
    m_tcpServer->SetConnectionCallback(m_connCallback);

    // 设置消息回调（在事件循环线程中运行）
    m_tcpServer->SetMessageCallback(m_messageCallback);

    // 初始化并启动服务器
    if (m_tcpServer->Init())
    {
        if (m_tcpServer->Start())
        {
            return 0;
        }
    }
    return -1;
}

/**
 * @brief 关闭Evpp服务器
 * 
 * 优雅地关闭TCP服务器，包括：
 * - 延迟停止服务器
 * - 等待服务器完全停止
 * - 停止事件循环
 * 
 * @return 关闭结果，0表示成功
 */
int NFEvppServer::Shut()
{
    // 延迟100秒后停止服务器
    m_eventLoop->loop()->RunAfter(100.0, [this]
    {
        m_tcpServer->Stop();
    });

    // 等待服务器完全停止
    while (!m_tcpServer->IsStopped())
    {
        NFSLEEP(1);
    }

    // 停止事件循环
    m_eventLoop->Stop(true);

    return 0;
}

/**
 * @brief 释放Evpp服务器资源
 * 
 * 清理TCP服务器和事件循环对象
 * 
 * @return 释放结果，0表示成功
 */
int NFEvppServer::Finalize()
{
    // 释放事件循环
    m_eventLoop.reset();
    // 释放TCP服务器
    m_tcpServer.reset();
    return 0;
}

