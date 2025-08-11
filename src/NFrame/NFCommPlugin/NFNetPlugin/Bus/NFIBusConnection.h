// -------------------------------------------------------------------------
//    @FileName         :    NFIBusConnection.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIBusConnection
//    @Desc             :    Bus连接接口类，定义进程间通信的基础连接功能。
//                          该文件定义了Bus进程间通信的核心接口类，包括Bus连接接口类定义、
//                          消息信息结构体、共享内存管理功能、消息发送和接收接口、连接状态管理。
//                          主要特性包括基于共享内存的高性能通信、支持多种消息类型、
//                          线程安全的数据传输、自动连接管理、可扩展的消息处理机制
//
// -------------------------------------------------------------------------

#pragma once

#include <map>
#include "NFBusDefine.h"
#include "NFBusShm.h"
#include "../NFINetMessage.h"
#include "NFComm/NFCore/NFBuffer.h"
#include "NFComm/NFCore/NFSpinLock.h"
#include "NFComm/NFPluginModule/NFIBusModule.h"
#include "NFComm/NFPluginModule/NFNetDefine.h"

struct MsgFromBusInfo;
typedef std::function<void(eMsgType type, uint64_t connectLinkId, uint64_t objectLinkId, NFDataPackage& package)> BusMsgPeerCallback;

/**
 * @struct MsgFromBusInfo
 * @brief Bus消息信息结构体
 * 
 * 用于存储从Bus接收到的消息信息，包含消息类型和数据包
 * 提供消息的完整上下文信息，便于消息处理和路由
 */
struct MsgFromBusInfo
{
    /**
     * @brief 构造函数，初始化消息类型为无效类型
     */
    MsgFromBusInfo()
    {
        m_type = eMsgType_Num;
    }

    /**
     * @brief 清空消息信息
     * 
     * 重置消息类型为无效类型，清空数据包内容
     */
    void Clear()
    {
        m_type = eMsgType_Num;
        m_packet.Clear();
    }

    eMsgType m_type;              ///< 消息类型
    NFDataPackage m_packet;       ///< 数据包
};

/**
 * @class NFIBusConnection
 * @brief Bus连接接口类，提供进程间通信的基础功能
 * 
 * 该类是Bus通信系统的核心接口，定义了进程间通信的基础功能：
 * - 共享内存管理
 * - 消息发送和接收
 * - 连接状态管理
 * - 心跳机制
 * - 数据包处理
 * 
 * 主要特性：
 * - 基于共享内存的高性能通信
 * - 支持多种消息类型
 * - 线程安全的数据传输
 * - 自动连接管理
 * - 可扩展的消息处理机制
 * 
 * 使用方式：
 * - 继承该类实现具体的Bus连接功能
 * - 重写虚函数实现自定义的消息处理逻辑
 * - 使用共享内存进行高效的数据传输
 */
class NFIBusConnection : public NFIModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     * @param flag 消息标志
     */
    NFIBusConnection(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag): NFIModule(p), m_connectionType(0), m_unLinkId(0), m_serverType(serverType), m_flag(flag)
    {
        m_pShmRecord = nullptr;
        m_nLastActionChannelPtr = nullptr;
        m_buffer.AssureSpace(MAX_SEND_BUFFER_SIZE);
        m_connectBuffer.AssureSpace(MAX_SEND_BUFFER_SIZE);
        m_lastHeartBeatTime = 0;
    }

    /**
     * @brief 析构函数，清理共享内存资源
     */
    ~NFIBusConnection() override
    {
        if (m_pShmRecord)
        {
            NF_SAFE_DELETE(m_pShmRecord);
        }
    }

public:
    /**
     * @brief 获取绑定标志
     * 
     * 返回当前连接的绑定标志信息
     * 
     * @return 绑定标志的常量引用
     */
    const NFMessageFlag& GetBindFlag() const;

    /**
     * @brief 发送Bus连接消息
     * 
     * 向指定的Bus发送连接请求消息
     * 
     * @param busId Bus ID
     * @param busLength Bus长度
     * @return 操作结果
     */
    int SendBusConnectMsg(uint64_t busId, uint64_t busLength);
    
    /**
     * @brief 发送Bus连接响应消息
     * @param busId Bus ID
     * @param busLength Bus长度
     * @return 操作结果
     */
    int SendBusConnectRspMsg(uint64_t busId, uint64_t busLength);
    
    /**
     * @brief 发送Bus心跳消息
     * @param busId Bus ID
     * @param busLength Bus长度
     * @return 操作结果
     */
    int SendBusHeartBeatMsg(uint64_t busId, uint64_t busLength);
    
    /**
     * @brief 发送Bus心跳响应消息
     * @param busId Bus ID
     * @param busLength Bus长度
     * @return 操作结果
     */
    int SendBusHeartBeatRspMsg(uint64_t busId, uint64_t busLength);

    /**
     * @brief 设置最后心跳时间
     * @param updateTime 更新时间
     */
    virtual void SetLastHeartBeatTime(uint64_t updateTime) { m_lastHeartBeatTime = updateTime; }

    /**
     * @brief 获取最后心跳时间
     * @return 最后心跳时间
     */
    virtual uint64_t GetLastHeartBeatTime() const { return m_lastHeartBeatTime; }

    /**
     * @brief 获取连接IP地址
     * @return IP地址字符串
     */
    virtual std::string GetLinkIp();

    /**
     * @brief 关闭连接ID
     */
    virtual void CloseLinkId();

    /**
     * @brief 检查是否已连接
     * @return 连接状态
     */
    virtual bool IsConnected() { return true; }

    /**
     * @brief 设置连接状态
     * @param connected 连接状态
     */
    virtual void SetConnected(bool connected)
    {
        // 基类默认实现
    }

    /**
     * @brief 通过key找到相应的共享内存通道
     * 
     * 根据共享内存键值获取对应的共享内存通道
     * 
     * @return 共享内存通道指针，如果未找到返回nullptr
     */
    virtual NFShmChannel* GetShmChannel();

    /**
     * @brief 通过key找到相应的共享内存记录
     * 
     * 根据共享内存键值获取对应的共享内存记录
     * 
     * @return 共享内存记录指针，如果未找到返回nullptr
     */
    virtual NFShmRecordType* GetShmRecord();

    /**
     * @brief 关闭一块共享内存
     * @return 操作结果
     */
    int CloseShmBuffer();

    /**
     * @brief 打开共享内存缓冲区
     * @param shmKey 共享内存键值
     * @param len 长度
     * @param data 数据指针
     * @param realSize 实际大小
     * @param create 是否创建
     * @return 操作结果
     */
    int OpenShmBuffer(key_t shmKey, size_t len, void** data, size_t* realSize, bool create);

    /**
     * @brief 附加共享内存
     * @param shmKey 共享内存键值
     * @param len 长度
     * @return 操作结果
     */
    virtual int AttachShm(key_t shmKey, size_t len);

    /**
     * @brief 检查附加的共享内存
     * @param buffer 缓冲区指针
     * @param len 长度
     * @return 操作结果
     */
    virtual int AttachShmCheck(void* buffer, size_t len);

    /**
     * @brief 初始化共享内存
     * @param shmKey 共享内存键值
     * @param len 长度
     * @return 操作结果
     */
    virtual int InitShm(key_t shmKey, size_t len);

    /**
     * @brief 初始化共享内存缓冲区
     * @param buffer 缓冲区指针
     * @param len 长度
     * @return 操作结果
     */
    virtual int InitShmBuffer(void* buffer, size_t len);

    /**
     * @brief 关闭共享内存
     * @return 操作结果
     */
    virtual int CloseShm();

    /**
     * @brief 设置消息对等回调函数
     * @param cb 回调函数
     */
    virtual void SetMsgPeerCallback(const BusMsgPeerCallback& cb);

    /**
     * @brief 发送数据
     * 
     * 发送包含数据头的消息数据
     *
     * @param packet 数据包
     * @param msg 发送的数据，这里的数据已经包含了数据头
     * @param nLen 数据的大小
     * @return 发送结果
     */
    virtual bool Send(NFDataPackage& packet, const char* msg, uint32_t nLen) = 0;

    /**
     * @brief 发送Protobuf消息
     * 
     * 发送Protobuf格式的消息
     * 
     * @param packet 数据包
     * @param xData Protobuf消息对象
     * @return 发送结果
     */
    virtual bool Send(NFDataPackage& packet, const google::protobuf::Message& xData) = 0;

protected:
    /**
     * @brief 共享内存发送数据
     * @param channel 通道指针
     * @param buf 数据缓冲区
     * @param len 数据长度
     * @return 操作结果
     */
    int ShmSend(NFShmChannel* channel, const void* buf, size_t len);

    /**
     * @brief 共享内存接收数据
     * @param channel 通道指针
     * @param buf 数据缓冲区
     * @param len 数据长度
     * @param recvSize 接收大小
     * @return 操作结果
     */
    int ShmRecv(NFShmChannel* channel, void* buf, size_t len, size_t* recvSize);

    /**
     * @brief 共享内存实际发送数据
     * @param channel 通道指针
     * @param buf 数据缓冲区
     * @param len 数据长度
     * @return 操作结果
     */
    int ShmRealSend(NFShmChannel* channel, const void* buf, size_t len);

protected:
    /**
     * @brief 获取最后操作信息
     * 
     * 返回最后操作的通道信息，用于调试
     * 
     * @return 包含开始和结束索引的pair
     */
    std::pair<size_t, size_t> LastAction();

    /**
     * @brief 显示共享内存通道信息
     * @param channel 通道指针
     * @param out 输出流
     * @param needNodeStatus 是否需要节点状态
     * @param needNodeData 是否需要节点数据
     */
    void ShowShmChannel(NFShmChannel* channel, std::ostream& out, bool needNodeStatus, size_t needNodeData);

    /**
     * @brief 获取共享内存统计信息
     * @param channel 通道指针
     * @param out 输出统计信息
     */
    void GetShmStats(NFShmChannel* channel, NFShmStatsBlockError& out);

protected:
    /**
     * @brief 设置写超时时间
     * @param channel 通道指针
     * @param ms 超时时间（毫秒）
     * @return 操作结果
     */
    static int SetWriteTimeout(NFShmChannel* channel, uint64_t ms);

    /**
     * @brief 获取写超时时间
     * @param channel 通道指针
     * @return 超时时间（毫秒）
     */
    static uint64_t GetWriteTimeout(NFShmChannel* channel);

    /**
     * @brief 设置写重试次数
     * @param channel 通道指针
     * @param times 重试次数
     * @return 操作结果
     */
    static int SetWriteRetryTimes(NFShmChannel* channel, size_t times);

    /**
     * @brief 获取写重试次数
     * @param channel 通道指针
     * @return 重试次数
     */
    static size_t GetWriteRetryTimes(NFShmChannel* channel);

    /**
     * @brief 复制配置信息
     * @param dst 目标配置
     * @param src 源配置
     */
    static void CopyConf(NFShmConf& dst, const NFShmConf& src);

    /**
     * @brief 检查标志位
     * @param flag 标志
     * @param checked 检查的标志
     * @return 是否设置
     */
    static bool CheckFlag(uint32_t flag, NFShmFlag checked) { return !!(flag & checked); }

    /**
     * @brief 设置标志位
     * @param flag 标志
     * @param checked 要设置的标志
     * @return 设置后的标志
     */
    static uint32_t SetFlag(uint32_t flag, NFShmFlag checked) { return flag | checked; }

    /**
     * @brief 创建默认配置
     * @param channel 通道指针
     */
    static void CreateDefaultConf(NFShmChannel* channel);

    /**
     * @brief 获取节点头
     * @param channel 通道指针
     * @param index 索引
     * @param data 数据指针
     * @param dataLen 数据长度
     * @return 节点头指针
     */
    static volatile NFShmNodeHead* GetNodeHead(NFShmChannel* channel, size_t index, void** data, size_t* dataLen);

    /**
     * @brief 获取块头
     * @param channel 通道指针
     * @param index 索引
     * @param data 数据指针
     * @param dataLen 数据长度
     * @return 块头指针
     */
    static NFShmBlockHead* GetBlockHead(NFShmChannel* channel, size_t index, void** data, size_t* dataLen);

    /**
     * @brief 获取下一个索引
     * @param channel 通道指针
     * @param index 当前索引
     * @param offset 偏移量
     * @return 下一个索引
     */
    static size_t GetNextIndex(NFShmChannel* channel, size_t index, size_t offset);

    /**
     * @brief 获取可用节点数量
     * @param channel 通道指针
     * @param readCur 读当前位置
     * @param writeCur 写当前位置
     * @return 可用节点数量
     */
    static size_t GetAvailableNodeCount(NFShmChannel* channel, size_t readCur, size_t writeCur);

    /**
     * @brief 获取节点范围数量
     * @param channel 通道指针
     * @param beginCur 开始位置
     * @param endCur 结束位置
     * @return 节点范围数量
     */
    static size_t GetNodeRangeCount(NFShmChannel* channel, size_t beginCur, size_t endCur);

    /**
     * @brief 获取操作序列号
     * @param channel 通道指针
     * @return 操作序列号
     */
    static uint32_t FetchOperationSeq(NFShmChannel* channel);

    /**
     * @brief 计算节点数量
     * @param channel 通道指针
     * @param len 长度
     * @return 节点数量
     */
    static size_t CalcNodeNum(NFShmChannel* channel, size_t len);

    /**
     * @brief 快速校验
     * @param src 源数据
     * @param len 长度
     * @return 校验值
     */
    static NFDataAlignType FastCheck(const void* src, size_t len);

    // 对齐单位的大小必须是2的N次方
    static_assert(0 == (sizeof(NFDataAlignType) & (sizeof(NFDataAlignType) - 1)), "data align size must be 2^N");
    // 节点大小必须是2的N次
    static_assert(0 == ((NFShmBlock::NODE_DATA_SIZE - 1) & NFShmBlock::NODE_DATA_SIZE), "node size must be 2^N");
    // 节点大小必须是对齐单位的2的N次方倍
    static_assert(0 == (NFShmBlock::NODE_DATA_SIZE & (NFShmBlock::NODE_DATA_SIZE - sizeof(NFDataAlignType))),
                  "node size must be [data align size] * 2^N");
public:
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
     * @return 解析类型
     */
    virtual uint32_t GetPacketParseType() const { return m_flag.mPacketParseType; }

    /**
     * @brief 检查是否安全
     * @return 安全状态
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
     * @brief 检查是否为活跃连接
     * @return 活跃状态
     */
    virtual bool IsActivityConnect() const { return m_flag.bActivityConnect; }

protected:
    NFShmRecordType* m_pShmRecord = nullptr;     ///< 共享内存记录指针
    NFBuffer m_buffer;                            ///< 缓冲区
    NFBuffer m_connectBuffer;                     ///< 连接缓冲区
    NFMessageFlag m_bindFlag;                     ///< 绑定标志

    /**
     * @brief 消息对等回调函数
     */
    BusMsgPeerCallback m_busMsgPeerCb;

    /**
    * @brief 心跳包更新时间
    */
    uint64_t m_lastHeartBeatTime;

    /**
     * @brief 调试辅助信息
     * @note 不再使用tls记录调试信息，以防跨线程dump拿不到数据
     */
    size_t m_nLastActionChannelEndNodeIndex = 0;   ///< 最后操作通道结束节点索引
    size_t m_nLastActionChannelBeginNodeIndex = 0; ///< 最后操作通道开始节点索引
    NFShmChannel* m_nLastActionChannelPtr = nullptr; ///< 最后操作通道指针

protected:
    uint32_t m_connectionType;                    ///< 连接类型
    uint64_t m_unLinkId;                          ///< 连接ID
    NF_SERVER_TYPE m_serverType;                  ///< 服务器类型
    NFMessageFlag m_flag;                         ///< 消息标志
};
