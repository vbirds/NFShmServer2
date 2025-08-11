// -------------------------------------------------------------------------
//    @FileName         :    NFIConnection.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    Evpp连接接口类，定义基于evpp库的网络连接基础功能
//
// -------------------------------------------------------------------------
#pragma once

#include <evpp/tcp_conn.h>

#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFComm/NFPluginModule/NFNetDefine.h"

/**
 * @file NFIConnection.h
 * @brief Evpp连接接口头文件
 * 
 * 该文件定义了基于evpp库的网络连接接口，包括：
 * - Evpp连接接口类定义
 * - 连接事件回调函数类型
 * - 消息回调函数类型
 * - 连接状态管理接口
 * - 数据包处理接口
 * 
 * 主要特性：
 * - 基于libevent的高性能事件驱动
 * - 支持异步非阻塞I/O
 * - 可配置的连接参数
 * - 灵活的回调机制
 * - 支持SSL安全连接
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @class NFIConnection
 * @brief Evpp连接接口类，提供基于evpp库的网络连接基础功能
 * 
 * 该类是Evpp网络通信系统的基础接口，定义了基于libevent的网络连接功能：
 * - TCP连接管理
 * - 消息回调处理
 * - 连接状态管理
 * - 事件驱动处理
 * - 安全连接支持
 * 
 * 主要特性：
 * - 基于libevent的高性能事件驱动
 * - 支持异步非阻塞I/O
 * - 可配置的连接参数
 * - 灵活的回调机制
 * - 支持SSL安全连接
 * 
 * 使用方式：
 * - 继承该类实现具体的Evpp连接功能
 * - 设置连接和消息回调函数
 * - 管理连接状态和参数
 * - 处理TCP网络通信
 */
class NFIConnection : public NFIModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
	NFIConnection(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag): NFIModule(p), m_connectionType(0), m_unLinkId(0), m_serverType(serverType), m_flag(flag)
	{

	}

    /**
     * @brief 析构函数
     */
	~NFIConnection() override
	{

	}

    /**
     * @brief 设置连接回调函数
     * 
     * 设置用于处理连接事件的回调函数，包括连接建立、断开等事件
     * 
     * @param back 连接回调函数
     */
	virtual void SetConnCallback(const evpp::ConnectionCallback& back)
	{
		m_connCallback = back;
	}

    /**
     * @brief 设置消息回调函数
     * 
     * 设置用于处理接收到的消息数据的回调函数
     * 
     * @param back 消息回调函数
     */
	virtual void SetMessageCallback(const evpp::MessageCallback& back)
	{
		m_messageCallback = back;
	}

    /**
     * @brief 获取Bus ID
     * @return Bus ID
     */
	virtual uint64_t GetBusId() const { return m_flag.mBusId; }

    /**
     * @brief 获取Bus长度
     * @return Bus长度
     */
	virtual uint64_t GetBusLength() const { return m_flag.mBusLength; }

    /**
     * @brief 获取数据包解析类型
     * @return 数据包解析类型
     */
	virtual uint32_t GetPacketParseType() const { return m_flag.mPacketParseType; }

    /**
     * @brief 检查是否为安全连接
     * @return true 安全连接，false 非安全连接
     */
	virtual bool IsSecurity() const { return m_flag.mSecurity; }

    /**
     * @brief 获取连接类型
     * @return 连接类型
     */
	virtual uint32_t GetConnectionType() { return m_connectionType; }

    /**
     * @brief 设置连接类型
     * @param type 连接类型
     */
	virtual void SetConnectionType(uint32_t type) { m_connectionType = type; }

    /**
     * @brief 设置连接ID
     * @param id 连接ID
     */
	virtual void SetLinkId(uint64_t id) { m_unLinkId = id; }
	
    /**
     * @brief 获取连接ID
     * @return 连接ID
     */
	virtual uint64_t GetLinkId() const { return m_unLinkId; }

    /**
     * @brief 检查是否为主动连接
     * @return true 主动连接，false 被动连接
     */
	virtual bool IsActivityConnect() const { return m_flag.bActivityConnect; }

protected:
	evpp::ConnectionCallback m_connCallback;      ///< 连接回调函数

	evpp::MessageCallback m_messageCallback;      ///< 消息回调函数

	uint32_t m_connectionType;                   ///< 连接类型

	uint64_t m_unLinkId;                         ///< 连接ID

	NF_SERVER_TYPE m_serverType;                 ///< 服务器类型

	NFMessageFlag m_flag;                        ///< 消息标志
};
