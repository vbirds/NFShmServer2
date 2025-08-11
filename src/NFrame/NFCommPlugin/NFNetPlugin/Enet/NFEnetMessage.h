// -------------------------------------------------------------------------
//    @FileName         :    NFEnetMessage.h
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFEnetNetMessage
//    @Desc             :    基于ENet库的网络消息处理类，提供UDP可靠传输连接管理和消息处理功能。
//                          该文件定义了基于ENet库的网络消息处理类，包括Enet网络消息处理类定义、
//                          UDP可靠传输连接管理、网络对象生命周期管理、消息接收和发送处理、
//                          连接事件回调处理、数据包解析和路由。
//                          主要特性包括基于ENet库的UDP可靠传输、支持连接状态监控、
//                          自动重传和流量控制、低延迟网络通信、适合实时游戏网络通信
//
// -------------------------------------------------------------------------

#pragma once
#include <queue>
#include <enet/enet.h>
#include <NFComm/NFPluginModule/NFObjectPool.hpp>

#include "../NFINetMessage.h"

class NFCodeQueue;
class EnetObject;
class NFIEnetConnection;

/**
 * @struct ENetPeerData
 * @brief ENet连接数据结构体
 * 
 * 用于存储ENet连接的相关信息，包括对象连接ID、数据包解析类型和安全标志
 */
struct ENetPeerData
{
    /**
     * @brief 默认构造函数
     * 
     * 初始化所有成员为默认值
     */
    ENetPeerData()
    {
        m_objectLinkId = 0;
        m_packetParseType = 0;
        m_security = false;
    }

    uint64_t m_objectLinkId;    ///< 对象连接ID
    uint32_t m_packetParseType; ///< 数据包解析类型
    bool m_security;            ///< 安全连接标志
};

/**
 * @class NFEnetMessage
 * @brief 基于ENet库的网络消息处理类
 * 
 * 该类继承自NFINetMessage，基于ENet库实现UDP可靠传输的网络消息处理功能
 * 主要用于处理UDP协议的可靠传输连接和消息路由
 * 
 * 主要功能：
 * - UDP可靠传输连接管理
 * - 网络对象生命周期管理
 * - 消息接收和发送处理
 * - 连接事件回调处理
 * - 数据包解析和路由
 * 
 * 特性：
 * - 基于ENet库的UDP可靠传输
 * - 支持连接状态监控
 * - 自动重传和流量控制
 * - 低延迟网络通信
 * - 适合实时游戏网络通信
 * 
 * 使用方式：
 * - 创建消息处理实例
 * - 绑定服务器或连接客户端
 * - 处理网络事件和消息
 * - 管理连接对象生命周期
 * - 发送和接收数据包
 */
class NFEnetMessage final : public NFINetMessage
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     */
    NFEnetMessage(NFIPluginManager* p, NF_SERVER_TYPE serverType);

    /**
     * @brief 析构函数
     */
    ~NFEnetMessage() override;

public:
    /**
     * @brief 绑定服务器
     * 
     * 建立ENet服务器绑定，监听客户端连接
     * 
     * @param flag 绑定标志
     * @return 绑定ID，0表示绑定失败
     */
    uint64_t BindServer(const NFMessageFlag& flag) override;

    /**
     * @brief 连接服务器
     * 
     * 建立与ENet服务器的连接
     * 
     * @param flag 连接标志
     * @return 连接ID，0表示连接失败
     */
    uint64_t ConnectServer(const NFMessageFlag& flag) override;

    /**
     * @brief 添加网络对象
     * 
     * 创建并添加新的网络连接对象
     * 
     * @param pConn ENet连接对象指针
     * @param parseType 数据包解析类型
     * @param bSecurity 安全连接标志
     * @return 网络对象指针
     */
    EnetObject* AddNetObject(ENetPeer* pConn, uint32_t parseType, bool bSecurity);

    /**
     * @brief 添加网络对象（指定连接ID）
     * 
     * 使用指定的连接ID创建网络连接对象
     * 
     * @param unLinkId 连接ID
     * @param pConn ENet连接对象指针
     * @param parseType 数据包解析类型
     * @param bSecurity 安全连接标志
     * @return 网络对象指针
     */
    EnetObject* AddNetObject(uint64_t unLinkId, ENetPeer* pConn, uint32_t parseType, bool bSecurity);

    /**
     * @brief 根据连接ID获取网络对象
     * 
     * @param linkId 连接ID
     * @return 网络对象指针，如果未找到则返回nullptr
     */
    EnetObject* GetNetObject(uint64_t linkId) const;

public:
    /**
     * @brief 连接事件回调
     * 
     * 处理连接建立、断开等事件
     * 
     * @param eventType 事件类型
     * @param pConn ENet连接对象指针
     * @param serverLinkId 服务器连接ID
     */
    void ConnectionCallback(ENetEventType eventType, ENetPeer* pConn, uint64_t serverLinkId);

    /**
     * @brief 消息接收回调
     * 
     * 处理接收到的数据包
     * 
     * @param pConn ENet连接对象指针
     * @param pPacket 数据包指针
     * @param serverLinkId 服务器连接ID
     */
    void MessageCallback(const ENetPeer* pConn, const ENetPacket* pPacket, uint64_t serverLinkId);

    /**
     * @brief 处理对等消息
     * 
     * 对解析出来的数据进行处理，包括接受数据处理、连接成功处理、断开连接处理
     * 
     * @param type 数据类型，主要是为了和多线程统一处理
     * @param serverLinkId 服务器连接ID
     * @param objectLinkId 对象连接ID
     * @param packet 数据包
     */
    void OnHandleMsgPeer(eMsgType type, uint64_t serverLinkId, uint64_t objectLinkId, NFDataPackage& packet);

    /**
     * @brief 处理代码队列
     * 
     * 处理默认的代码队列，通常用于处理全局或默认的代码队列
     */
    void ProcessCodeQueue();

    /**
     * @brief 处理指定的代码队列
     * 
     * 处理传入的特定代码队列，对队列中的代码进行处理
     * 
     * @param pRecvQueue 指向要处理的NFCodeQueue结构的指针
     */
    void ProcessCodeQueue(NFCodeQueue* pRecvQueue);

public:
    /**
     * @brief 关闭网络模块
     * 
     * 关闭所有连接和清理资源
     * 
     * @return 关闭结果，0表示成功
     */
    int Shut() override;

    /**
     * @brief 释放网络模块资源
     * 
     * 清理所有占用的资源
     * 
     * @return 释放结果，0表示成功
     */
    int Finalize() override;

    /**
     * @brief 服务器每帧执行
     * 
     * 处理网络事件、消息队列和连接管理
     * 
     * @return 处理结果，0表示成功
     */
    int Tick() override;

    /**
     * @brief 获得连接IP地址
     * 
     * 根据连接ID获取对应的IP地址
     * 
     * @param usLinkId 连接ID
     * @return IP地址字符串
     */
    std::string GetLinkIp(uint64_t usLinkId) override;

    /**
     * @brief 获取连接端口号
     * 
     * 根据连接ID获取对应的端口号
     * 
     * @param usLinkId 连接ID
     * @return 端口号
     */
    uint32_t GetPort(uint64_t usLinkId) override;

    /**
     * @brief 关闭指定连接
     * 
     * 关闭指定ID的连接并清理相关资源
     * 
     * @param usLinkId 连接ID
     */
    void CloseLinkId(uint64_t usLinkId) override;

    /**
     * @brief 获取空闲连接ID
     * 
     * 从空闲连接ID队列中获取一个可用的连接ID
     * 
     * @return 空闲连接ID
     */
    uint64_t GetFreeUnLinkId();

    /**
     * @brief 发送消息（原始数据）
     * 
     * 向指定连接发送原始数据消息
     * 
     * @param usLinkId 连接ID
     * @param packet 数据包
     * @param msg 消息数据
     * @param nLen 数据长度
     * @return true 发送成功，false 发送失败
     */
    bool Send(uint64_t usLinkId, NFDataPackage& packet, const char* msg, uint32_t nLen) override;

    /**
     * @brief 发送消息（Protobuf数据）
     * 
     * 向指定连接发送Protobuf格式的消息
     * 
     * @param usLinkId 连接ID
     * @param packet 数据包
     * @param xData Protobuf消息对象
     * @return true 发送成功，false 发送失败
     */
    bool Send(uint64_t usLinkId, NFDataPackage& packet, const google::protobuf::Message& xData) override;

    /**
     * @brief 发送消息到指定对象
     * 
     * 向指定的网络对象发送消息
     * 
     * @param pObject 网络对象指针
     * @param packet 数据包
     * @param msg 消息数据
     * @param nLen 数据长度
     * @return true 发送成功，false 发送失败
     */
    bool Send(const EnetObject* pObject, NFDataPackage& packet, const char* msg, uint32_t nLen);

    /**
     * @brief 定时器回调
     * 
     * 处理定时器事件，包括心跳检测等
     * 
     * @param timerId 定时器ID
     * @return 处理结果，0表示成功
     */
    int OnTimer(uint32_t timerId) override;

private:
    /**
     * @brief 存储所有连接的列表
     * 
     * 该列表用于管理所有活动的连接，允许对连接进行遍历、查找和操作
     */
    std::vector<NFIEnetConnection*> m_connectionList;

    /**
     * @brief 用于存储空闲链接ID的队列
     * 
     * 空闲链接ID可以用于分配给新的连接，避免频繁的内存分配
     */
    std::queue<uint64_t> m_freeLinks;

    /**
     * @brief 链接对象数组
     * 
     * 存储所有网络对象的数组，用于快速访问
     */
    std::vector<EnetObject*> m_netObjectArray;

    /**
     * @brief 网络对象映射表
     * 
     * 根据连接ID快速查找对应的网络对象
     */
    std::unordered_map<uint64_t, EnetObject*> m_netObjectMap;

    /**
     * @brief 网络对象池
     * 
     * 用于管理网络对象的创建和销毁，提高性能
     */
    NFObjectPool<EnetObject> m_netObjectPool;

    /**
     * @brief 需要删除的连接对象列表
     * 
     * 存储需要在下一次循环中删除的连接对象ID
     */
    std::vector<uint64_t> m_removeObject;

    /**
     * @brief 发送缓冲区
     * 
     * 用于临时存储待发送的数据
     */
    NFBuffer m_sendBuffer;

    /**
     * @brief 发送压缩缓冲区
     * 
     * 用于存储压缩后的发送数据
     */
    NFBuffer m_sendComBuffer;

    /**
     * @brief 接收缓冲区
     * 
     * 用于临时存储接收到的数据
     */
    NFBuffer m_recvBuffer;

    /**
     * @brief 接收代码列表缓冲区
     * 
     * 用于存储接收到的代码列表数据
     */
    NFBuffer m_recvCodeList;

    /**
     * @brief 服务器每一帧处理的消息数限制
     * 
     * 限制每帧处理的消息数量，防止过载
     */
    uint32_t m_handleMsgNumPerFrame;

    /**
     * @brief 服务器当前帧处理的消息数
     * 
     * 记录当前帧已经处理的消息数量
     */
    int32_t m_curHandleMsgNum;
};
