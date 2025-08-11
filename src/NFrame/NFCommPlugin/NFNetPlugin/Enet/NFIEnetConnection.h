// -------------------------------------------------------------------------
//    @FileName         :    NFIEnetConnection.h
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFIEnetConnection
//    @Desc             :    Enet连接接口类，定义基于ENet库的UDP可靠传输连接功能。
//                          该文件定义了基于ENet库的UDP可靠传输连接接口，包括Enet连接接口类定义、
//                          连接事件回调函数类型、消息回调函数类型、连接状态管理接口、数据包处理接口。
//                          主要特性包括基于ENet库的UDP可靠传输、内置数据包重传和流量控制、
//                          支持连接事件处理、低延迟网络通信、适合实时游戏网络通信
//
// -------------------------------------------------------------------------

#pragma once

#include <enet/enet.h>
#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFComm/NFPluginModule/NFNetDefine.h"

/**
 * @typedef ENET_CONNECT_CALLBACK
 * @brief Enet连接事件回调函数类型
 * 
 * 用于处理Enet连接事件的回调函数，包括连接建立、断开等事件
 * 
 * @param eventType 事件类型
 * @param pConn 连接对象指针
 * @param serverLinkId 服务器连接ID
 */
typedef std::function<void(ENetEventType eventType, ENetPeer* pConn, uint64_t serverLinkId)> ENET_CONNECT_CALLBACK;

/**
 * @typedef ENET_MESSAGE_CALLBACK
 * @brief Enet消息回调函数类型
 * 
 * 用于处理Enet接收到的消息数据的回调函数
 * 
 * @param pConn 连接对象指针
 * @param pPacket 数据包指针
 * @param serverLinkId 服务器连接ID
 */
typedef std::function<void(ENetPeer* pConn, ENetPacket* pPacket, uint64_t serverLinkId)> ENET_MESSAGE_CALLBACK;

/**
 * @class NFIEnetConnection
 * @brief Enet连接接口类，提供基于ENet库的UDP可靠传输连接功能
 * 
 * 该类是Enet网络通信系统的基础接口，定义了基于ENet库的UDP可靠传输功能：
 * - UDP可靠传输连接管理
 * - 消息回调处理
 * - 连接状态管理
 * - 数据包重传机制
 * - 流量控制
 * 
 * 主要特性：
 * - 基于ENet库的UDP可靠传输
 * - 内置数据包重传和流量控制
 * - 支持连接事件处理
 * - 低延迟网络通信
 * - 适合实时游戏网络通信
 * 
 * 使用方式：
 * - 继承该类实现具体的Enet连接功能
 * - 设置连接和消息回调函数
 * - 管理连接状态和参数
 * - 处理UDP可靠传输
 */
class NFIEnetConnection : public NFIModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
    NFIEnetConnection(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag): NFIModule(p), m_connectionType(0), m_unLinkId(0), m_serverType(serverType), m_flag(flag)
    {
    }

    /**
     * @brief 析构函数
     */
    ~NFIEnetConnection() override
    {
    }

    /**
     * @brief 设置连接回调函数
     * 
     * 设置用于处理连接事件的回调函数，包括连接建立、断开等事件
     * 
     * @param back 连接回调函数
     */
    virtual void SetConnCallback(const ENET_CONNECT_CALLBACK& back)
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
    virtual void SetMessageCallback(const ENET_MESSAGE_CALLBACK& back)
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
    ENET_CONNECT_CALLBACK m_connCallback;       ///< 连接回调函数

    ENET_MESSAGE_CALLBACK m_messageCallback;    ///< 消息回调函数

    uint32_t m_connectionType;                  ///< 连接类型

    uint64_t m_unLinkId;                        ///< 连接ID

    NF_SERVER_TYPE m_serverType;                ///< 服务器类型

    NFMessageFlag m_flag;                       ///< 消息标志
};
