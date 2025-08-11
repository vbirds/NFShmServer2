// -------------------------------------------------------------------------
//    @FileName         :    NFEnetClient.cpp
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFEnetClient
//    @Desc             :    基于ENet库的UDP可靠传输客户端实现文件
//
// -------------------------------------------------------------------------

#include "NFEnetClient.h"
#include <NFComm/NFPluginModule/NFLogMgr.h>

/**
 * @file NFEnetClient.cpp
 * @brief Enet客户端实现文件
 * 
 * 该文件实现了基于ENet库的UDP可靠传输客户端，包括：
 * - 客户端的初始化和销毁
 * - 网络事件的处理
 * - 连接管理
 * - 数据包处理
 * - 错误处理和日志记录
 * 
 * 主要功能：
 * - 创建和管理ENet主机
 * - 连接到指定服务器
 * - 处理连接和断开事件
 * - 处理数据接收事件
 * - 提供回调机制
 * 
 * @author gaoyi
 * @date 2025-03-13
 * @version 1.0
 */

/**
 * @brief 初始化Enet客户端
 * 
 * 创建ENet主机并连接到服务器，包括：
 * - 确定本地IP地址
 * - 创建ENet主机
 * - 设置服务器地址
 * - 建立连接
 * 
 * @return 初始化结果，0表示成功，-1表示失败
 */
int NFEnetClient::Init()
{
    // 确定本地IP地址
    std::string ipAddr;
    if (m_flag.mStrIp == "127.0.0.1")
    {
        ipAddr = m_flag.mStrIp;
    }
    else
    {
        ipAddr = "0.0.0.0";
    }

    // 创建ENet主机（客户端模式）
    m_pHost = enet_host_create(NULL, 1, 1, 0, 0);
    if (nullptr == m_pHost)
    {
        LOG_ERR(0, -1, "enet_host_create, {}:{} failed", ipAddr, m_flag.nPort);
        return -1;
    }

    // 设置服务器地址
    ENetAddress srv_addr;
    if(enet_address_set_host(&srv_addr, ipAddr.c_str())){
        LOG_ERR(0, -1, "enet_address_set_host_ip {} fail", m_flag.mStrIp);
        return -1;
    }
    srv_addr.port = m_flag.nPort;

    // 连接到服务器
    auto pPeer = enet_host_connect(m_pHost, &srv_addr, 1, 0);
    if(pPeer == nullptr){
        LOG_ERR(0, -1, "enet_host_connect {}:{} fail", m_flag.mStrIp, m_flag.nPort);
        return -1;
    }

    // 设置连接类型
    m_connectionType = NF_CONNECTION_TYPE_TCP_CLIENT;

    return 0;
}

/**
 * @brief Enet客户端心跳处理
 * 
 * 处理ENet事件，包括：
 * - 连接事件处理
 * - 断开事件处理
 * - 数据接收处理
 * - 错误处理
 * 
 * @return 处理结果，0表示成功
 */
int NFEnetClient::Tick()
{
    if (m_pHost == nullptr) return 0;

    int ret = 0;
    ENetEvent event;
    
    // 处理ENet事件
    ret = enet_host_service(m_pHost, &event, 0);
    if (ret > 0)
    {
        if (event.type == ENET_EVENT_TYPE_CONNECT || event.type == ENET_EVENT_TYPE_DISCONNECT)
        {
            // 处理连接/断开事件
            if (m_connCallback)
            {
                m_connCallback(event.type, event.peer, m_unLinkId);
            }
        }
        else if (event.type == ENET_EVENT_TYPE_RECEIVE)
        {
            // 处理数据接收事件
            if (m_messageCallback)
            {
                m_messageCallback(event.peer, event.packet, m_unLinkId);
            }
            enet_packet_destroy(event.packet);
        }
    }
    else if (0 == ret)
    {
        return 0;
    }
    else
    {
        LOG_ERR(0, -1, "enet_host_service fail");
        return 0;
    }
    return 0;
}

/**
 * @brief 关闭Enet客户端
 * 
 * 关闭客户端连接
 * 
 * @return 关闭结果，0表示成功
 */
int NFEnetClient::Shut()
{
    return 0;
}

/**
 * @brief 释放Enet客户端资源
 * 
 * 销毁ENet主机对象
 * 
 * @return 释放结果，0表示成功
 */
int NFEnetClient::Finalize()
{
    if (m_pHost)
    {
        enet_host_destroy(m_pHost);
        m_pHost = nullptr;
    }
    return 0;
}