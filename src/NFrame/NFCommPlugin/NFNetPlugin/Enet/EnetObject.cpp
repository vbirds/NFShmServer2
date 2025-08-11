// -------------------------------------------------------------------------
//    @FileName         :    EnetObject.cpp
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    EnetObject
//    @Desc             :    Enet网络对象实现文件，提供UDP可靠传输连接对象的生命周期管理
//
// -------------------------------------------------------------------------

#include "EnetObject.h"

/**
 * @file EnetObject.cpp
 * @brief Enet网络对象实现文件
 * 
 * 该文件实现了基于ENet库的UDP连接对象，包括：
 * - 网络对象的创建和销毁
 * - 连接信息的管理和查询
 * - 连接状态的检查和设置
 * - 连接生命周期管理
 * - 心跳时间管理
 * 
 * 主要功能：
 * - 封装ENet连接对象
 * - 管理连接状态和参数
 * - 提供连接信息查询接口
 * - 支持连接关闭和清理
 * 
 * @author gaoyi
 * @date 2025-03-13
 * @version 1.0
 */

/**
 * @brief Enet网络对象构造函数
 * 
 * 初始化网络对象，设置默认值：
 * - 连接ID为0
 * - 不需要移除标志为false
 * - 设置为服务器端
 * - 数据包解析类型为0
 * - 记录当前时间作为心跳时间
 * - 端口为0
 * - 非安全连接
 * 
 * @param pConn ENet连接对象指针
 */
EnetObject::EnetObject(ENetPeer* pConn) : m_usLinkId(0), m_needRemove(false), m_connPtr(pConn)
{
    m_isServer = true;
    m_packetParseType = 0;
    m_lastHeartBeatTime = NFGetTime();
    m_port = 0;
    m_security = false;
}

/**
 * @brief Enet网络对象析构函数
 * 
 * 清理网络对象资源
 */
EnetObject::~EnetObject()
{
}

/**
 * @brief 获取连接IP地址
 * 
 * 返回当前连接对象的IP地址字符串
 * 
 * @return IP地址字符串
 */
std::string EnetObject::GetStrIp() const
{
    return m_strIp;
}

/**
 * @brief 获取连接端口号
 * 
 * 返回当前连接对象的端口号
 * 
 * @return 端口号
 */
uint32_t EnetObject::GetPort() const
{
    return m_port;
}

/**
 * @brief 设置连接IP地址
 * 
 * 设置当前连接对象的IP地址
 * 
 * @param val IP地址字符串
 */
void EnetObject::SetStrIp(const std::string& val)
{
    m_strIp = val;
}

/**
 * @brief 设置连接端口号
 * 
 * 设置当前连接对象的端口号
 * 
 * @param port 端口号
 */
void EnetObject::SetPort(uint32_t port)
{
    m_port = port;
}

/**
 * @brief 获取连接唯一ID
 * 
 * 返回当前连接对象的唯一标识符
 * 
 * @return 连接ID
 */
uint64_t EnetObject::GetLinkId() const
{
    return m_usLinkId;
}

/**
 * @brief 设置连接唯一ID
 * 
 * 设置当前连接对象的唯一标识符
 * 
 * @param linkId 连接ID
 */
void EnetObject::SetLinkId(uint64_t linkId)
{
    m_usLinkId = linkId;
}

/**
 * @brief 检查是否需要移除
 * 
 * 检查当前连接对象是否需要从连接列表中移除
 * 
 * @return true 需要移除，false 不需要移除
 */
bool EnetObject::GetNeedRemove() const
{
    return m_needRemove;
}

/**
 * @brief 设置是否需要移除
 * 
 * 设置当前连接对象是否需要从连接列表中移除
 * 
 * @param val 移除标志
 */
void EnetObject::SetNeedRemove(bool val)
{
    m_needRemove = val;
}

/**
 * @brief 关闭对象，禁止对象的读写功能
 * 
 * 如果连接已断开，则发送断开连接请求
 * 该方法会调用ENet库的断开连接函数
 */
void EnetObject::CloseObject() const
{
    if (m_connPtr && IsDisConnect())
    {
        enet_peer_disconnect(m_connPtr, 0);
    }
}

/**
 * @brief 设置是否为服务器端
 * 
 * 设置当前连接对象的服务器端标志
 * 
 * @param b 服务器端标志
 */
void EnetObject::SetIsServer(bool b)
{
    m_isServer = b;
}

/**
 * @brief 检查是否为服务器端
 * 
 * 检查当前连接对象是否为服务器端
 * 
 * @return true 服务器端，false 客户端
 */
bool EnetObject::IsServer() const
{
    return m_isServer;
}

/**
 * @brief 检查连接是否已建立
 * 
 * 检查当前连接对象是否已建立连接
 * 
 * @return true 已连接，false 未连接
 */
bool EnetObject::IsConnect() const
{
    return !IsDisConnect();
}

/**
 * @brief 检查连接是否已断开
 * 
 * 检查ENet连接状态，判断是否处于断开状态
 * 检查的状态包括：正在断开、已断开、确认断开、僵尸状态
 * 
 * @return true 已断开，false 未断开
 */
bool EnetObject::IsDisConnect() const
{
    if (!m_connPtr) return true;
    if (m_connPtr->state == ENET_PEER_STATE_DISCONNECTING ||
        m_connPtr->state == ENET_PEER_STATE_DISCONNECTED ||
        m_connPtr->state == ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT ||
        m_connPtr->state == ENET_PEER_STATE_ZOMBIE)
    {
        return true;
    }
    return false;
}
