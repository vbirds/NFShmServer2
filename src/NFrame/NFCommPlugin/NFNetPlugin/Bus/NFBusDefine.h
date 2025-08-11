// -------------------------------------------------------------------------
//    @FileName         :    NFBusDefine.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFBusPlugin
//    @Desc             :    Bus通信协议相关的常量定义和类型定义。
//                          该文件包含了Bus进程间通信系统的所有基础定义，包括数据对齐类型定义、
//                          连接限制常量、共享内存相关常量、消息处理相关常量
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"

/**
 * @typedef NFDataAlignType
 * @brief 数据对齐类型，用于内存对齐操作
 * 
 * 使用uint64_t类型确保在64位系统上的最佳性能
 */
typedef uint64_t NFDataAlignType;

/** 
 * @def NFBUS_MACRO_SRC_BUS_LIMIT
 * @brief Bus连接方最大数目限制
 * 
 * 限制单个Bus服务器最多可以连接的客户端数量为512个
 */
#define NFBUS_MACRO_SRC_BUS_LIMIT 512

/** 
 * @def NFBUS_MACRO_HUGETLB_SIZE
 * @brief 大页内存块大小 (4MB)
 * 
 * 用于共享内存分配的大页内存块大小，提高内存访问效率
 */
#define NFBUS_MACRO_HUGETLB_SIZE 4194304

/** 
 * @def NFBUS_MACRO_MSG_LIMIT
 * @brief 单个消息的最大长度限制 (64KB)
 * 
 * 限制单个Bus消息的最大长度，防止内存溢出
 */
#define NFBUS_MACRO_MSG_LIMIT 65536

/** 
 * @def NFBUS_MACRO_CONNECTION_CONFIRM_TIMEOUT
 * @brief 连接确认超时时间 (30秒)
 * 
 * 客户端连接服务器时的超时时间，超过此时间未收到确认则连接失败
 */
#define NFBUS_MACRO_CONNECTION_CONFIRM_TIMEOUT 30

/** 
 * @def NFBUS_MACRO_BUSID_TYPE
 * @brief Bus ID 类型定义
 * 
 * 使用uint64_t类型作为Bus连接的唯一标识符
 */
#define NFBUS_MACRO_BUSID_TYPE uint64_t

/** 
 * @def NFBUS_MACRO_DATA_NODE_SIZE
 * @brief 数据节点大小 (128字节)
 * 
 * 共享内存中每个数据节点的固定大小
 */
#define NFBUS_MACRO_DATA_NODE_SIZE 128

/** 
 * @def NFBUS_MACRO_DATA_ALIGN_SIZE
 * @brief 数据对齐大小 (16字节)
 * 
 * 内存对齐的基本单位，确保数据访问效率
 */
#define NFBUS_MACRO_DATA_ALIGN_SIZE 16

/** 
 * @def NFBUS_MACRO_DATA_ALIGN_TYPE
 * @brief 数据对齐类型
 * 
 * 用于内存对齐操作的数据类型定义
 */
#define NFBUS_MACRO_DATA_ALIGN_TYPE uint64_t

/** 
 * @def NFBUS_MACRO_DATA_MAX_PROTECT_SIZE
 * @brief 数据保护区最大大小 (16KB)
 * 
 * 共享内存中数据保护区的最大大小限制
 */
#define NFBUS_MACRO_DATA_MAX_PROTECT_SIZE 16384

/** 
 * @def SHM_CHANNEL_NAME
 * @brief 共享内存通道名称
 * 
 * 用于标识Bus共享内存通道的字符串标识符
 */
#define SHM_CHANNEL_NAME "NFBUSMEM"

/** 
 * @def SHM_CHANNEL_VERSION
 * @brief 共享内存通道版本号
 * 
 * 当前Bus共享内存协议的版本号，用于协议兼容性检查
 */
#define SHM_CHANNEL_VERSION 2
