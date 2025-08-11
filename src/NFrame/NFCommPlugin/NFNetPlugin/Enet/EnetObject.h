// -------------------------------------------------------------------------
//    @FileName         :    EnetObject.h
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    EnetObject
//    @Desc             :    Enet网络对象类，代表一个UDP可靠传输连接。
//                          该文件定义了基于ENet库的UDP连接对象，包括Enet网络对象类定义、
//                          连接信息管理、连接状态管理、数据包解析类型设置、心跳时间管理。
//                          主要特性包括支持UDP可靠传输、可配置的数据包解析方式、
//                          连接状态监控、自动心跳检测、低延迟网络通信
//
// -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <enet/enet.h>
#include "NFComm/NFPluginModule/NFNetDefine.h"

class NFEnetMessage;

/**
 * @class EnetObject
 * @brief Enet网络对象类，代表一个UDP可靠传输连接
 * 
 * 该类封装了基于ENet库的UDP连接对象，提供以下功能：
 * - 连接信息管理（IP、端口、连接ID等）
 * - 连接状态管理
 * - 数据包解析类型设置
 * - 心跳时间管理
 * - 安全连接支持
 * 
 * 主要特性：
 * - 支持UDP可靠传输
 * - 可配置的数据包解析方式
 * - 连接状态监控
 * - 自动心跳检测
 * - 低延迟网络通信
 * 
 * 使用方式：
 * - 创建EnetObject对象管理UDP连接
 * - 设置连接参数和状态
 * - 管理连接生命周期
 * - 处理连接事件和数据传输
 */
class EnetObject
{
public:
    friend NFEnetMessage;

    /**
     * @brief 构造函数
     * @param pConn ENet连接对象指针
     */
    explicit EnetObject(ENetPeer* pConn);

    /**
     * @brief 析构函数
     */
    ~EnetObject();

    /**
     * @brief 获取连接IP地址
     * @return IP地址字符串
     */
    std::string GetStrIp() const;

    /**
     * @brief 获取连接端口号
     * @return 端口号
     */
    uint32_t GetPort() const;

    /**
     * @brief 设置连接IP地址
     * @param val IP地址字符串
     */
    void SetStrIp(const std::string& val);

    /**
     * @brief 设置连接端口号
     * @param port 端口号
     */
    void SetPort(uint32_t port);

    /**
     * @brief 获取连接唯一ID
     * @return 连接ID
     */
    uint64_t GetLinkId() const;

    /**
     * @brief 设置连接唯一ID
     * @param linkId 连接ID
     */
    void SetLinkId(uint64_t linkId);

    /**
     * @brief 检查是否需要移除
     * @return true 需要移除，false 不需要移除
     */
    bool GetNeedRemove() const;

    /**
     * @brief 设置是否需要移除
     * @param val 移除标志
     */
    void SetNeedRemove(bool val);

    /**
     * @brief 关闭对象，禁止对象的读写功能
     */
    void CloseObject() const;

    /**
     * @brief 设置是否为服务器端
     * @param b 服务器端标志
     */
    void SetIsServer(bool b);

    /**
     * @brief 检查是否为服务器端
     * @return true 服务器端，false 客户端
     */
    bool IsServer() const;

    /**
     * @brief 设置数据包解析类型
     * @param packetType 解析类型
     */
    void SetPacketParseType(uint32_t packetType) { m_packetParseType = packetType; }

    /**
     * @brief 设置ENet连接指针
     * @param pConn ENet连接对象指针
     */
    void SetConnPtr(ENetPeer* pConn) { m_connPtr = pConn; }

    /**
     * @brief 设置最后心跳时间
     * @param updateTime 更新时间
     */
    void SetLastHeartBeatTime(uint64_t updateTime) { m_lastHeartBeatTime = updateTime; }

    /**
     * @brief 获取最后心跳时间
     * @return 最后心跳时间
     */
    uint64_t GetLastHeartBeatTime() const { return m_lastHeartBeatTime; }

    /**
     * @brief 设置安全连接标志
     * @param security 安全连接标志
     */
    void SetSecurity(bool security) { m_security = security; }

    /**
     * @brief 检查是否为安全连接
     * @return true 安全连接，false 非安全连接
     */
    bool IsSecurity() const { return m_security; }

    /**
     * @brief 检查连接是否已断开
     * @return true 已断开，false 未断开
     */
    bool IsDisConnect() const;
    
    /**
     * @brief 检查连接是否已建立
     * @return true 已连接，false 未连接
     */
    bool IsConnect() const;

protected:
    /**
     * @brief 代表客户端连接的唯一ID
     */
    uint64_t m_usLinkId;

    /**
     * @brief 连接代表的对方的IP地址
     */
    std::string m_strIp;
    
    /**
     * @brief 连接端口号
     */
    uint32_t m_port;

    /**
     * @brief 是否需要删除标志
     * 
     * 当连接不再起作用时，将在下一次循环中被删除
     */
    bool m_needRemove;

    /**
     * @brief 是否为服务器端标志
     */
    bool m_isServer;

    /**
     * @brief 数据包解析类型
     * 
     * 用于指定消息的解码方式
     */
    uint32_t m_packetParseType;

    /**
     * @brief 来自ENet的连接对象指针
     * 
     * 封装了ENet库的连接对象
     */
    ENetPeer* m_connPtr;

    /**
     * @brief 心跳包更新时间
     * 
     * 记录最后一次心跳包的时间戳
     */
    uint64_t m_lastHeartBeatTime;

    /**
     * @brief 安全连接标志
     * 
     * 标识是否为安全连接
     */
    bool m_security;
};
