// -------------------------------------------------------------------------
//    @FileName         :    NFCBusModule.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFBusPlugin
//    @Desc             :    Bus消息管理类，负责Bus通信系统的消息路由和处理。
//                          该文件定义了Bus通信系统的消息管理类，包括Bus消息管理类定义、
//                          消息路由和处理、连接管理、心跳机制、消息回调处理。
//                          主要特性包括统一的消息管理接口、支持服务器和客户端模式、
//                          自动心跳检测、连接状态监控、消息路由分发
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIBusConnection.h"
#include "../NFINetMessage.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFPluginModule/NFNetDefine.h"

class NFCBusServer;
class NFCBusClient;

/**
 * @class NFCBusMessage
 * @brief Bus消息管理类，负责Bus通信系统的消息路由和处理
 * 
 * 该类实现了NFINetMessage接口，提供以下功能：
 * - 统一的消息管理接口
 * - 支持服务器和客户端模式
 * - 自动心跳检测
 * - 连接状态监控
 * - 消息路由分发
 * 
 * 主要特性：
 * - 基于共享内存的高性能通信
 * - 支持多种消息类型
 * - 线程安全的消息处理
 * - 自动连接管理
 * - 可扩展的消息处理机制
 * 
 * 使用方式：
 * - 创建消息管理实例
 * - 绑定服务器或连接客户端
 * - 处理消息路由和分发
 * - 管理连接状态和心跳
 */
class NFCBusMessage final : public NFINetMessage
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     */
	explicit NFCBusMessage(NFIPluginManager* p, NF_SERVER_TYPE serverType);

    /**
     * @brief 析构函数，清理消息管理资源
     */
	~NFCBusMessage() override;

    /**
     * @brief 准备就绪更新函数
     * @return 操作结果
     */
	int ReadyTick() override;

    /**
     * @brief 定时器更新函数
     * @return 操作结果
     */
	int Tick() override;

    /**
     * @brief 关闭消息管理
     * @return 操作结果
     */
	int Shut() override;

    /**
     * @brief 最终化消息管理
     * @return 操作结果
     */
	int Finalize() override;

public:
	/**
	 * @brief 绑定服务器
	 * 
	 * 初始化并绑定Bus服务器
	 * 
	 * @param flag 消息标志，包含服务器配置信息
	 * @return 绑定结果，成功返回服务器ID，失败返回0
	 */
	uint64_t BindServer(const NFMessageFlag& flag) override;

	/**
	 * @brief 连接服务器
	 * 
	 * 连接到指定的Bus服务器
	 * 
	 * @param flag 消息标志，包含服务器配置信息
	 * @return 连接结果，成功返回连接ID，失败返回0
	 */
	uint64_t ConnectServer(const NFMessageFlag& flag) override;

	/**
	 * @brief 发送原始数据
	 * 
	 * 向指定连接发送不包含数据头的原始数据
	 * 
	 * @param usLinkId 连接ID
	 * @param packet 数据包
	 * @param msg 发送的数据
	 * @param nLen 数据的大小
	 * @return 发送结果
	 */
	bool Send(uint64_t usLinkId, NFDataPackage& packet, const char* msg, uint32_t nLen) override;

    /**
     * @brief 发送Protobuf消息
     * @param usLinkId 连接ID
     * @param packet 数据包
     * @param xData Protobuf消息对象
     * @return 发送结果
     */
	bool Send(uint64_t usLinkId, NFDataPackage& packet, const google::protobuf::Message& xData) override;

	/**
	 * @brief 获得连接IP地址
	 * 
	 * 获取指定连接的IP地址信息
	 * 
	 * @param usLinkId 连接ID
	 * @return IP地址字符串
	 */
	std::string GetLinkIp(uint64_t usLinkId) override;

    /**
     * @brief 获取连接端口
     * @param usLinkId 连接ID
     * @return 端口号
     */
	uint32_t GetPort(uint64_t usLinkId) override;

	/**
	 * @brief 关闭连接
	 * 
	 * 关闭指定的连接
	 * 
	 * @param usLinkId 连接ID
	 */
	void CloseLinkId(uint64_t usLinkId) override;

    /**
     * @brief 处理对等消息
     * 
     * 处理来自其他Bus连接的消息
     * 
     * @param type 消息类型
     * @param serverLinkId 服务器连接ID
     * @param objectLinkId 对象连接ID
     * @param package 数据包
     */
	void OnHandleMsgPeer(eMsgType type, uint64_t serverLinkId, uint64_t objectLinkId, NFDataPackage& package);

    /**
     * @brief 恢复连接
     * @return 操作结果
     */
	int ResumeConnect() override;

    /**
     * @brief 定时器回调
     * @param nTimerId 定时器ID
     * @return 操作结果
     */
	int OnTimer(uint32_t nTimerId) override;

    /**
     * @brief 发送心跳消息
     * 
     * 向所有连接发送心跳消息，保持连接活跃
     */
	void SendHeartMsg();

    /**
     * @brief 检查服务器心跳
     * 
     * 检查所有服务器连接的心跳状态，清理超时连接
     */
	void CheckServerHeartBeat();

private:
    /** @brief Bus连接映射表，存储所有Bus连接 */
	NFCommMapEx<uint64_t, NFIBusConnection> m_busConnectMap;
    /** @brief 绑定连接指针 */
    NF_SHARE_PTR<NFIBusConnection> m_bindConnect;
};
