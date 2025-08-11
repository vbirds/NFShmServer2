// -------------------------------------------------------------------------
//    @FileName         :    NFEnetServer.cpp
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFEnetServer
//    @Desc             :    基于ENet库的UDP可靠传输服务器实现文件
//
// -------------------------------------------------------------------------

#include "NFEnetServer.h"

#include <NFComm/NFPluginModule/NFLogMgr.h>
#include <NFCommPlugin/NFNetPlugin/Evpp/NFEvppNetMessage.h>

/**
 * @file NFEnetServer.cpp
 * @brief Enet服务器实现文件
 * 
 * 该文件实现了基于ENet库的UDP可靠传输服务器，包括：
 * - 服务器的初始化和销毁
 * - 网络事件的处理
 * - 连接管理
 * - 数据包处理
 * - 错误处理和日志记录
 * 
 * 主要功能：
 * - 创建和管理ENet主机
 * - 监听客户端连接
 * - 处理连接和断开事件
 * - 处理数据接收事件
 * - 提供回调机制
 * 
 * @author gaoyi
 * @date 2025-03-13
 * @version 1.0
 */

/**
 * @brief Enet服务器构造函数
 * 
 * 初始化Enet服务器，设置主机指针为nullptr
 * 
 * @param p 插件管理器指针
 * @param serverType 服务器类型
 * @param flag 消息标志
 */
NFEnetServer::NFEnetServer(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag): NFIEnetConnection(p, serverType, flag)
{
    m_pHost = nullptr;
}

/**
 * @brief Enet服务器析构函数
 * 
 * 清理服务器资源
 */
NFEnetServer::~NFEnetServer()
{
}

/**
 * @brief 初始化Enet服务器
 * 
 * 创建ENet主机并绑定到指定地址，包括：
 * - 确定监听IP地址
 * - 设置ENet地址
 * - 创建ENet主机
 * - 配置连接类型
 * 
 * @return 初始化结果，0表示成功，-1表示失败
 */
int NFEnetServer::Init()
{
    // 确定监听IP地址
    std::string ipAddr;
    if (m_flag.mStrIp == "127.0.0.1")
    {
        ipAddr = m_flag.mStrIp;
    }
    else
    {
        ipAddr = "0.0.0.0";
    }

    // 设置ENet地址
    ENetAddress address;
    if (enet_address_set_host(&address, ipAddr.c_str()))
    {
        LOG_ERR(0, -1, "enet_address_set_host_ip:{} failed", ipAddr);
        return -1;
    }

    // 设置端口并创建ENet主机
    address.port = m_flag.nPort;
    m_pHost = enet_host_create(&address, 1, 1, 0, 0);
    if (nullptr == m_pHost)
    {
        LOG_ERR(0, -1, "enet_host_create, {}:{} failed", address.host, address.port);
        return -1;
    }

    // 设置连接类型
    m_connectionType = NF_CONNECTION_TYPE_TCP_SERVER;

    return 0;
}

/**
 * @brief Enet服务器心跳处理
 * 
 * 处理ENet事件，包括：
 * - 连接事件处理
 * - 断开事件处理
 * - 数据接收处理
 * - 错误处理
 * 
 * @return 处理结果，0表示成功
 */
int NFEnetServer::Tick()
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
 * @brief 关闭Enet服务器
 * 
 * 关闭服务器连接
 * 
 * @return 关闭结果，0表示成功
 */
int NFEnetServer::Shut()
{
    return 0;
}

/**
 * @brief 释放Enet服务器资源
 * 
 * 销毁ENet主机对象
 * 
 * @return 释放结果，0表示成功
 */
int NFEnetServer::Finalize()
{
    if (m_pHost)
    {
        enet_host_destroy(m_pHost);
        m_pHost = nullptr;
    }
    return 0;
}
