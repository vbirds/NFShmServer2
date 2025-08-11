// -------------------------------------------------------------------------
//    @FileName         :    NetEvppObject.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    Evpp网络对象实现文件，提供TCP连接对象的生命周期管理
//
// -------------------------------------------------------------------------

#include "NetEvppObject.h"
#include "NFComm/NFCore/NFPlatform.h"

/**
 * @file NetEvppObject.cpp
 * @brief Evpp网络对象实现文件
 * 
 * 该文件实现了基于evpp库的TCP连接对象，包括：
 * - 网络对象的创建和销毁
 * - 连接信息的管理和查询
 * - 连接状态的检查和设置
 * - 连接生命周期管理
 * - 心跳时间管理
 * 
 * 主要功能：
 * - 封装evpp TCP连接对象
 * - 管理连接状态和参数
 * - 提供连接信息查询接口
 * - 支持连接关闭和清理
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/** @brief WebSocket握手密钥常量 */
#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/**
 * @brief Evpp网络对象构造函数
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
 * @param conn TCP连接指针
 */
NetEvppObject::NetEvppObject(const evpp::TCPConnPtr& conn) : m_usLinkId(0), m_needRemove(false), m_connPtr(conn)
{
	m_isServer = true;
	m_packetParseType = 0;
	m_lastHeartBeatTime = NFGetTime();
	m_port = 0;
	m_security = false;
}

/**
 * @brief Evpp网络对象析构函数
 * 
 * 清理网络对象资源
 */
NetEvppObject::~NetEvppObject()
{
}

/**
 * @brief 获取连接IP地址
 * 
 * 返回当前连接对象的IP地址字符串
 * 
 * @return IP地址字符串
 */
std::string NetEvppObject::GetStrIp() const
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
uint32_t NetEvppObject::GetPort() const
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
void NetEvppObject::SetStrIp(const std::string& val)
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
void NetEvppObject::SetPort(uint32_t port)
{
    m_port = port;
}

/**
 * @brief 设置是否为服务器端
 * 
 * 设置当前连接对象的服务器端标志
 * 
 * @param b 服务器端标志
 */
void NetEvppObject::SetIsServer(bool b)
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
bool NetEvppObject::IsServer() const
{
    return m_isServer;
}

/**
 * @brief 获取连接唯一ID
 * 
 * 返回当前连接对象的唯一标识符
 * 
 * @return 连接ID
 */
uint64_t NetEvppObject::GetLinkId() const
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
void NetEvppObject::SetLinkId(uint64_t linkId)
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
bool NetEvppObject::GetNeedRemove() const
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
void NetEvppObject::SetNeedRemove(bool val)
{
	m_needRemove = val;
}

/**
 * @brief 关闭对象，禁止对象的读写功能
 * 
 * 如果连接已建立或正在连接中，则关闭连接
 * 该方法会调用evpp库的关闭连接函数
 */
void NetEvppObject::CloseObject() const
{
	if (m_connPtr)
	{
		if (m_connPtr->IsConnected() || m_connPtr->IsConnecting())
		{
			m_connPtr->Close();
		}
	}
}
