// -------------------------------------------------------------------------
//    @FileName         :    NFCBusClient.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCBusClient
//    @Desc             :    Bus客户端实现类，负责进程间通信的客户端连接。
//                          该文件定义了Bus进程间通信的客户端实现，包括Bus客户端类定义、
//                          客户端连接管理、消息发送和接收、共享内存通信、连接状态管理。
//                          主要特性包括基于共享内存的高性能通信、支持消息队列处理、
//                          线程安全的数据传输、自动重连机制、连接状态监控
//
// -------------------------------------------------------------------------

#pragma once

#include <map>
#include "NFBusShm.h"
#include "NFIBusConnection.h"
#include "../NFINetMessage.h"
#include "NFComm/NFCore/NFBuffer.h"
#include "NFComm/NFCore/NFSpinLock.h"
#include "NFComm/NFPluginModule/NFIBusModule.h"
#include "NFComm/NFPluginModule/NFNetDefine.h"

/**
 * @class NFCBusClient
 * @brief Bus客户端类，实现进程间通信的客户端连接功能
 * 
 * 该类继承自NFIBusConnection，提供以下功能：
 * - 与Bus服务器建立连接
 * - 处理进程间消息传输
 * - 管理共享内存通信
 * - 消息发送和接收
 * - 连接状态管理
 * 
 * 主要特性：
 * - 基于共享内存的高性能通信
 * - 支持消息队列处理
 * - 线程安全的数据传输
 * - 自动重连机制
 * 
 * 使用方式：
 * - 创建客户端实例并初始化
 * - 连接到指定的Bus服务器
 * - 发送和接收消息
 * - 管理连接状态
 */
class NFCBusClient final : public NFIBusConnection
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     * @param bindFlag 绑定标志
     */
    explicit NFCBusClient(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag, const NFMessageFlag& bindFlag):NFIBusConnection(p, serverType, flag)
    {
        m_bindFlag = bindFlag;
        m_sendBuffer.AssureSpace(MAX_SEND_BUFFER_SIZE);
        m_isConnected = false;
    }

    /**
     * @brief 析构函数，清理客户端资源
     */
    ~NFCBusClient() override;

    /**
     * @brief 定时器更新函数
     * @return 操作结果
     */
    int Tick() override;

    /**
     * @brief 初始化客户端
     * @return 操作结果
     */
    int Init() override;

    /**
     * @brief 关闭客户端
     * @return 操作结果
     */
    int Shut() override;

    /**
     * @brief 最终化客户端
     * @return 操作结果
     */
    int Finalize() override;

    /**
     * @brief 检查是否已连接
     * @return 连接状态
     */
    bool IsConnected() override;

    /**
     * @brief 设置连接状态
     * @param connected 连接状态
     */
    void SetConnected(bool connected) override;

public:
    /**
     * @brief 连接服务器
     * 
     * 连接到指定的Bus服务器
     * 
     * @param flag 消息标志，包含服务器配置信息
     * @param bindFlag 绑定标志，包含客户端配置信息
     * @return 连接结果，成功返回连接ID，失败返回0
     */
    uint64_t ConnectServer(const NFMessageFlag& flag, const NFMessageFlag& bindFlag);

    /**
     * @brief 发送原始数据
     * 
     * 发送不包含数据头的原始数据
     * 
     * @param packet 数据包
     * @param msg 发送的数据
     * @param nLen 数据的大小
     * @return 发送结果
     */
    bool Send(NFDataPackage& packet, const char* msg, uint32_t nLen) override;

    /**
     * @brief 发送Protobuf消息
     * @param packet 数据包
     * @param xData Protobuf消息对象
     * @return 发送结果
     */
    bool Send(NFDataPackage& packet, const google::protobuf::Message& xData) override;

    /**
     * @brief 通过共享内存通道发送数据
     * @param pChannel 共享内存通道指针
     * @param packetParseType 数据包解析类型
     * @param packet 数据包
     * @param msg 消息数据
     * @param nLen 数据长度
     * @return 发送结果
     */
    bool Send(NFShmChannel* pChannel, int packetParseType, const NFDataPackage& packet, const char* msg, uint32_t nLen);

private:
    NFBuffer m_sendBuffer;        ///< 发送缓冲区
    bool m_isConnected;           ///< 连接状态
};
