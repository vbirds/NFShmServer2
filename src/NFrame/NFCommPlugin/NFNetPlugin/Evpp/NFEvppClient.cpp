// -------------------------------------------------------------------------
//    @FileName         :    NFEvppClient.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    基于Evpp库的TCP客户端实现文件
//
// -------------------------------------------------------------------------

#include "NFEvppClient.h"

#include <evpp/tcp_client.h>

/**
 * @file NFEvppClient.cpp
 * @brief Evpp客户端实现文件
 * 
 * 该文件实现了基于evpp库的TCP客户端，包括：
 * - 客户端的初始化和销毁
 * - 网络事件的处理
 * - 连接管理
 * - 数据包处理
 * - 错误处理和日志记录
 * 
 * 主要功能：
 * - 创建和管理evpp TCP客户端
 * - 连接到指定服务器
 * - 处理连接和断开事件
 * - 处理数据接收事件
 * - 提供回调机制
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

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
bool NFEvppClient::Init(evpp::EventLoop* loop)
{
    if (!loop)
    {
        // 创建新的事件循环线程
        m_eventLoop.reset(NF_NEW evpp::EventLoopThread());
        m_eventLoop->set_name(GetServerName(m_serverType));
        m_eventLoop->Start(true);

        // 构建连接地址
        std::string strIpPort = NF_FORMAT("{}:{}", m_flag.mStrIp, m_flag.nPort);

        // 创建TCP客户端
        m_tcpClient.reset(NF_NEW evpp::TCPClient(m_eventLoop->loop(), strIpPort, "NFEvppClient"));
        if (!m_tcpClient)
        {
            return false;
        }
    }
    else
    {
        // 使用提供的事件循环
        std::string strIpPort = NF_FORMAT("{}:{}", m_flag.mStrIp, m_flag.nPort);

        m_tcpClient.reset(NF_NEW evpp::TCPClient(loop, strIpPort, "NFEvppClient"));
        if (!m_tcpClient)
        {
            return false;
        }
    }

    // 设置连接类型
    m_connectionType = NF_CONNECTION_TYPE_TCP_CLIENT;

    // 设置连接回调（在事件循环线程中运行）
    m_tcpClient->SetConnectionCallback(m_connCallback);

    // 设置消息回调（在事件循环线程中运行）
    m_tcpClient->SetMessageCallback(m_messageCallback);
    
    // 配置自动重连参数
    m_tcpClient->set_auto_reconnect(true);
    m_tcpClient->set_reconnect_interval(evpp::Duration(static_cast<double>(10)));
    m_tcpClient->set_connecting_timeout(evpp::Duration(static_cast<double>(10)));

    // 启动连接
    m_tcpClient->Connect();

    return true;
}

/**
 * @brief 关闭Evpp客户端
 * 
 * 断开TCP连接并停止事件循环
 * 
 * @return 关闭结果，0表示成功
 */
int NFEvppClient::Shut()
{
    // 断开TCP连接
    if (m_tcpClient)
    {
        m_tcpClient->Disconnect();
    }

    // 停止事件循环
    if (m_eventLoop)
    {
        m_eventLoop->Stop(true);
    }

    return 0;
}

/**
 * @brief 释放Evpp客户端资源
 * 
 * 清理TCP客户端和事件循环对象
 * 
 * @return 释放结果，0表示成功
 */
int NFEvppClient::Finalize()
{
    // 释放事件循环
    if (m_eventLoop)
    {
        m_eventLoop.reset();
    }

    // 释放TCP客户端
    if (m_tcpClient)
    {
        m_tcpClient.reset();
    }
    return 0;
}

