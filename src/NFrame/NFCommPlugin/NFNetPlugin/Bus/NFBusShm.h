// -------------------------------------------------------------------------
//    @FileName         :    NFBusShm.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFBusPlugin
//    @Desc             :    Bus共享内存管理头文件，定义共享内存通信的数据结构和操作。
//                          该文件定义了Bus进程间通信的共享内存管理相关结构体和操作，包括共享内存统计信息结构、
//                          跨平台共享内存记录类型、共享内存通道配置和头结构、数据节点和块结构定义、内存对齐和大小计算工具。
//                          主要特性包括跨平台支持（Windows/Linux）、原子操作保证线程安全、
//                          内存对齐优化性能、统计信息收集、错误检测和恢复
//
// -------------------------------------------------------------------------

#pragma once

#include "NFBusDefine.h"
#include <atomic>

#if NF_PLATFORM == NF_PLATFORM_WIN
#include <Windows.h>
typedef long key_t;
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

/**
 * @struct NFShmStatsBlockError
 * @brief 共享内存统计信息块错误结构体
 * 
 * 用于记录共享内存操作过程中的各种错误统计信息
 * 帮助诊断和监控共享内存通信的健康状态
 */
struct NFShmStatsBlockError {
	/** @brief 写完后校验操作序号错误次数 */
	size_t m_nWriteCheckSequenceFailedCount;
	/** @brief 写操作内部重试次数 */
	size_t m_nWriteRetryCount;

	/** @brief 读到的错误数据节点数量 */
	size_t m_nReadBadNodeCount;
	/** @brief 读到的错误数据块数量 */
	size_t m_nReadBadBlockCount;
	/** @brief 读到的写超时保护数量 */
	size_t m_nReadWriteTimeoutCount;
	/** @brief 读到的数据块长度检查错误数量 */
	size_t m_nReadCheckBlockSizeFailedCount;
	/** @brief 读到的数据节点和长度检查错误数量 */
	size_t m_nReadCheckNodeSizeFailedCount;
	/** @brief 读到的数据hash值检查错误数量 */
	size_t m_nReadCheckHashFailedCount;
};

#if NF_PLATFORM == NF_PLATFORM_WIN
/**
 * @struct NFShmRecordType
 * @brief Windows平台共享内存记录类型
 * 
 * 在Windows平台上使用的共享内存记录结构体
 * 包含共享内存句柄、缓冲区指针等信息
 */
struct NFShmRecordType {
    /**
     * @brief 构造函数，初始化所有成员为0
     */
    NFShmRecordType()
    {
        memset(this, 0, sizeof(NFShmRecordType));
    }
	HANDLE m_nHandle;                    ///< 共享内存句柄
	LPCTSTR m_nBuffer;                   ///< 共享内存缓冲区指针
	size_t m_nSize;                      ///< 共享内存大小
	size_t m_nReferenceCount;            ///< 引用计数
    bool m_nOwner;                       ///< 是否为所有者
	uint64_t m_nBusId;                   ///< Bus ID
	uint64_t m_nBusLength;               ///< Bus长度
    uint64_t m_nUnLinkId;               ///< 连接ID
    uint32_t m_packetParseType;         ///< 解码消息类型
};
#else
/**
 * @struct NFShmRecordType
 * @brief Linux平台共享内存记录类型
 * 
 * 在Linux平台上使用的共享内存记录结构体
 * 包含共享内存文件描述符、路径等信息
 */
struct NFShmRecordType{
    /**
     * @brief 构造函数，初始化所有成员
     */
    NFShmRecordType()
    {
        m_nShmFd = 0;
        m_nShmId = 0;
        m_nBuffer = NULL;
        m_nSize = 0;
        m_nReferenceCount = 0;
        m_nOwner = false;
        m_nBusId = 0;
        m_nBusLength = 0;
        m_nUnLinkId = 0;
        m_packetParseType = 0;
    }
    int m_nShmFd;                       ///< 共享内存文件描述符
    std::string m_nShmPath;             ///< 共享内存路径
	int m_nShmId;                       ///< 共享内存ID
	void *m_nBuffer;                    ///< 共享内存缓冲区指针
	size_t m_nSize;                     ///< 共享内存大小
    size_t m_nReferenceCount;           ///< 引用计数
    bool m_nOwner;                      ///< 是否为所有者
	uint64_t m_nBusId;                  ///< Bus ID
    uint64_t m_nBusLength;              ///< Bus长度
	uint64_t m_nUnLinkId;               ///< 连接ID
    uint32_t m_packetParseType;         ///< 解码消息类型
};
#endif

#if NF_PLATFORM == NF_PLATFORM_WIN
__pragma(pack(push, 1))
#  define NFBUS_MACRO_PACK_ATTR
#else
#  define NFBUS_MACRO_PACK_ATTR
// #define NFBUS_MACRO_PACK_ATTR __attribute__((packed))
#endif

/**
 * @struct NFShmConf
 * @brief 共享内存配置结构体
 * 
 * 定义共享内存通道的配置参数，包括保护节点数量、内存大小、超时时间等
 */
struct NFShmConf {
	size_t m_nProtectNodeCount;         ///< 保护节点数量
	size_t m_nProtectMemorySize;        ///< 保护内存大小
	uint64_t m_nConfSendTimeoutMs;      ///< 发送超时时间（毫秒）

	size_t m_nWriteRetryTimes;          ///< 写操作重试次数
	// TODO 接收端校验号(用于保证只有一个接收者)
	volatile std::atomic<size_t> m_nAtomicRecverIdentify; ///< 原子接收者标识
} NFBUS_MACRO_PACK_ATTR;

/**
 * @struct NFShmChannel
 * @brief 共享内存通道结构体
 * 
 * 定义共享内存通信通道的完整结构，包括通道头、数据节点、原子操作等
 * 是Bus通信系统的核心数据结构
 */
struct NFShmChannel {
	char m_nNodeMagic[8];               ///< 魔术串，用于标识数据类型
    uint16_t m_channelVersion;          ///< 通道版本号
    uint16_t m_channelAlignSize;        ///< 通道对齐大小
    uint16_t m_channelHostSize;         ///< 通道主机大小

	// 数据节点
	size_t m_nNodeSize;                 ///< 节点大小
	size_t m_nNodeSizeBinPower;         ///< 节点大小二进制幂次（用于优化算法）
	size_t m_nNodeCount;                ///< 节点数量

	// [atomic_read_cur, atomic_write_cur) 内的数据块都是已使用的数据块
	// atomic_write_cur指向的数据块一定是空块，故而必然有一个node的空洞
	// c11的stdatomic.h在很多编译器不支持并且还有些潜规则(gcc 不能使用-fno-builtin 和 -march=xxx)，故而使用c++版本
	volatile std::atomic<size_t> m_nAtomicReadCur;   ///< 原子读当前位置
	volatile std::atomic<size_t> m_nAtomicWriteCur;  ///< 原子写当前位置

	// 第一次读到正在写入数据的时间
	uint64_t m_nFirstFailedWritingTime; ///< 第一次写入失败时间

	volatile std::atomic<uint32_t> m_nAtomicOperationSeq; ///< 操作序列号(用于保证只有一个接收者)

	// 配置
	NFShmConf m_nConf;                 ///< 配置信息
	size_t m_nAreaChannelOffset;        ///< 通道区域偏移
	size_t m_nAreaHeadOffset;           ///< 头部区域偏移
	size_t m_nAreaDataOffset;           ///< 数据区域偏移
	size_t m_nAreaEndOffset;            ///< 结束区域偏移

	// 统计信息
	size_t m_nWriteCheckSequenceFailedCount; ///< 写完后校验操作序号错误
	size_t m_nWriteRetryCount;               ///< 写操作内部重试次数

	size_t m_nReadBadNodeCount;              ///< 读到的错误数据节点数量
	size_t m_nReadBadBlockCount;             ///< 读到的错误数据块数量
	size_t m_nReadWriteTimeoutCount;         ///< 读到的写超时保护数量
	size_t m_nReadCheckBlockSizeFailedCount; ///< 读到的数据块长度检查错误数量
	size_t m_nReadCheckNodeSizeFailedCount;  ///< 读到的数据节点和长度检查错误数量
	size_t m_nReadCheckHashFailedCount;      ///< 读到的数据hash值检查错误数量
} NFBUS_MACRO_PACK_ATTR;

#if NF_PLATFORM == NF_PLATFORM_WIN
__pragma(pack(pop))
#endif

static_assert(std::is_standard_layout<NFShmChannel>::value, "shm_channel must be a standard layout");

/**
 * @struct NFShmAddr
 * @brief 共享内存地址结构体
 * 
 * 定义共享内存中的地址映射结构，包含目标连接ID和源连接信息数组
 */
struct NFShmAddr
{
	volatile std::atomic<uint64_t> m_dstLinkId;                    ///< 目标连接ID
	volatile std::atomic<uint64_t> m_srcLinkId[NFBUS_MACRO_SRC_BUS_LIMIT];      ///< 源连接ID数组
	volatile std::atomic<uint64_t> m_srcBusLength[NFBUS_MACRO_SRC_BUS_LIMIT];   ///< 源Bus长度数组
	volatile std::atomic<uint32_t> m_srcParseType[NFBUS_MACRO_SRC_BUS_LIMIT];   ///< 源解析类型数组
	volatile std::atomic<bool> m_bActiveConnect[NFBUS_MACRO_SRC_BUS_LIMIT];     ///< 活跃连接状态数组
};

/**
 * @struct NFShmChannelHead
 * @brief 共享内存通道头结构体
 * 
 * 定义共享内存通道的完整头部结构，包含连接通道、数据通道和地址信息
 * 对齐到12KB，为未来扩展预留空间
 */
typedef struct {
    NFShmChannel m_nConnectChannel;     ///< 连接通道
	NFShmChannel m_nShmChannel;         ///< 共享内存通道
	NFShmAddr m_nShmAddr;               ///< 共享内存地址
	char m_nAlign[12 * 1024 - sizeof(NFShmChannel) - sizeof(NFShmChannel) - sizeof(NFShmAddr)]; ///< 对齐填充，用于以后拓展
    //char m_nAlign[12 * 1024 - sizeof(NFShmChannel) - sizeof(NFShmAddr)]; // 对齐到12KB,用于以后拓展
} NFShmChannelHead;

/**
 * @struct NFShmNodeHead
 * @brief 数据节点头结构体
 * 
 * 定义共享内存中数据节点的头部信息
 * @note 暂时忽略伪共享造成的cache line失效问题。否则head的内存浪费太大了
 * @see https://en.wikipedia.org/wiki/False_sharing
 */
typedef struct {
	uint32_t m_nFlag;                   ///< 节点标志
	uint32_t m_nOperationSeq;           ///< 操作序列号
} NFShmNodeHead;

/**
 * @struct NFShmBlockHead
 * @brief 数据块头结构体
 * 
 * 定义共享内存中数据块的头部信息，包含缓冲区大小和快速校验值
 */
typedef struct {
	size_t m_nBufferSize;               ///< 缓冲区大小
    uint64_t m_nFastCheck;              ///< 快速校验值
} NFShmBlockHead;

/**
 * @enum NFShmFlag
 * @brief 共享内存标志枚举
 * 
 * 定义共享内存操作中使用的各种标志位
 */
typedef enum {
	NF_WRITEN = 0x00000001,            ///< 已写入标志
	MF_START_NODE = 0x00000002,        ///< 起始节点标志
} NFShmFlag;

/**
 * @struct NFShmBlock
 * @brief 内存通道常量结构体
 * 
 * 定义共享内存块的各种大小常量
 * @note 为了压缩内存占用空间，这里使用手动对齐，不直接用 #pragma pack(sizoef(long))
 */
struct NFShmBlock {
	/**
	 * @enum size_def
	 * @brief 大小定义枚举
	 * 
	 * 定义共享内存块中各种结构的大小常量
	 */
	enum size_def {
		CHANNEL_HEAD_SIZE = sizeof(NFShmChannelHead),    ///< 通道头大小
		BLOCK_HEAD_SIZE = ((sizeof(NFShmBlockHead) + NFBUS_MACRO_DATA_ALIGN_SIZE - 1) / NFBUS_MACRO_DATA_ALIGN_SIZE) * NFBUS_MACRO_DATA_ALIGN_SIZE, ///< 块头大小
		NODE_HEAD_SIZE = ((sizeof(NFShmNodeHead) + NFBUS_MACRO_DATA_ALIGN_SIZE - 1) / NFBUS_MACRO_DATA_ALIGN_SIZE) * NFBUS_MACRO_DATA_ALIGN_SIZE, ///< 节点头大小

		NODE_DATA_SIZE = NFBUS_MACRO_DATA_NODE_SIZE,     ///< 节点数据大小
		NODE_HEAD_DATA_SIZE = NODE_DATA_SIZE - BLOCK_HEAD_SIZE, ///< 节点头数据大小
		CONNECT_SHM_SIZE = (NODE_HEAD_SIZE + NODE_DATA_SIZE) * 100, ///< 连接共享内存大小
	};
};

/**
 * @struct NFShmBinPowerCheck
 * @brief 检测数字是2的几次幂的模板结构体
 * 
 * 用于编译时检查数字是否为2的幂次方
 * 
 * @tparam S 要检查的数字
 */
template <size_t S>
struct NFShmBinPowerCheck {
	static_assert(0 == (S & (S - 1)), "not 2^N"); ///< 必须是2的N次幂
	static_assert(S, "must not be 0");            ///< 必须大于0

	enum { value = NFShmBinPowerCheck<(S >> 1)>::value + 1 }; ///< 递归计算幂次
};

/**
 * @struct NFShmBinPowerCheck<1>
 * @brief 特化版本，当数字为1时返回0
 */
template <>
struct NFShmBinPowerCheck<1> {
	enum { value = 0 }; ///< 1是2的0次幂
};
