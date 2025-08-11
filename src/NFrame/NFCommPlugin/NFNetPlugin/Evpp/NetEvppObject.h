// -------------------------------------------------------------------------
//    @FileName         :    NetEvppObject.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    Evpp网络对象类，代表一个TCP连接
//
// -------------------------------------------------------------------------
#pragma once

#include <cstdint>

#include "NFComm/NFPluginModule/NFNetDefine.h"
#include "evpp/tcp_conn.h"
#include "evpp/event_loop.h"

class NFEvppNetMessage;
class NFEvppClient;

/**
 * @file NetEvppObject.h
 * @brief Evpp网络对象头文件
 * 
 * 该文件定义了基于evpp库的TCP连接对象，包括：
 * - Evpp网络对象类定义
 * - 连接信息管理
 * - 连接状态管理
 * - 数据包解析类型设置
 * - 心跳时间管理
 * 
 * 主要特性：
 * - 支持客户端和服务器端连接
 * - 可配置的数据包解析方式
 * - 连接状态监控
 * - 自动心跳检测
 * - 线程安全的连接管理
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @class NetEvppObject
 * @brief Evpp网络对象类，代表一个TCP连接
 * 
 * 该类封装了基于evpp库的TCP连接对象，提供以下功能：
 * - 连接信息管理（IP、端口、连接ID等）
 * - 连接状态管理
 * - 数据包解析类型设置
 * - 心跳时间管理
 * - 安全连接支持
 * 
 * 主要特性：
 * - 支持客户端和服务器端连接
 * - 可配置的数据包解析方式
 * - 连接状态监控
 * - 自动心跳检测
 * - 线程安全的连接管理
 * 
 * 使用方式：
 * - 创建NetEvppObject对象管理TCP连接
 * - 设置连接参数和状态
 * - 管理连接生命周期
 * - 处理连接事件和数据传输
 */
class NetEvppObject final
{
public:
    friend NFEvppNetMessage;
    friend NFEvppClient;

    /**
     * @brief 构造函数
     * @param conn TCP连接指针
     */
    explicit NetEvppObject(const evpp::TCPConnPtr &conn);

    /**
     * @brief 析构函数
     */
    ~NetEvppObject();

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
     * @brief 设置TCP连接指针
     * @param conn TCP连接指针
     */
    void SetConnPtr(const evpp::TCPConnPtr& conn) { m_connPtr = conn; }

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
     * @brief 来自EVPP的TCP连接代理
     * 
     * 封装了evpp库的TCP连接对象
     */
    evpp::TCPConnPtr m_connPtr;

    /**
     * @brief 心跳包更新时间
     * 
     * 记录最后一次心跳包的时间戳
     */
    uint64_t m_lastHeartBeatTime;

    /**
     * @brief 安全连接标志
     * 
     * 标识是否为SSL/TLS安全连接
     */
    bool m_security;
};
