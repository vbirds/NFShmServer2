// -------------------------------------------------------------------------
//    @FileName         :    NFServerIDUtil.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerIDUtil
//
// -------------------------------------------------------------------------

/**
 * @file NFServerIDUtil.h
 * @brief 服务器ID工具类
 * 
 * 此文件提供了服务器ID的编码、解码和地址解析功能。
 * 服务器ID采用分层结构：世界服ID.分区ID.服务器类型.实例ID
 * 支持32位整数和点分十进制字符串两种表示方式。
 */

#pragma once

#include "NFPlatform.h"

/** @brief 公共分区位数基础值 */
#define PUB_ZONE_BITS_BASE 1000
/** @brief 区域数字基础值 */
#define AREA_DIGIT_BASE 10000

// 服务器ID位数定义
/** @brief 世界服ID位数：4位，支持0-15 */
#define WORLD_ID_BITS 4
/** @brief 分区ID位数：12位，支持0-4095 */
#define ZONE_ID_BITS 12
/** @brief 服务器类型位数：8位，支持0-255 */
#define SERVER_TYPE_BITS 8
/** @brief 实例ID位数：8位，支持0-255（修复了原来的拼写错误INST_ID_BIS） */
#define INST_ID_BITS 8

/**
 * @brief 通道地址结构体
 * 
 * 用于表示网络通信的地址信息，支持多种协议（TCP、UDP、BUS等）。
 * 可以解析完整的URL格式地址，如：tcp://127.0.0.1:8123
 */
struct NFChannelAddress
{
	/**
	 * @brief 默认构造函数
	 * 初始化端口为0
	 */
	NFChannelAddress()
	{
		mPort = 0;
	}

	/** @brief 主机完整地址，比如：tcp://127.0.0.1:8123 udp://127.0.0.1:1000 或 bus://127.0.0.1 */
	std::string mAddress;
	/** @brief 协议名称，比如：tcp 或 bus */
	std::string mScheme;
	/** @brief 主机地址，比如：127.0.0.1 */
	std::string mHost;
	/** @brief 端口（仅网络连接有效） */
	int         mPort;
};

/**
 * @brief BUS通信地址模板分析的相关接口
 * 
 * Bus系统进程通信地址有两种表示方法：
 * 1. 内部表示：32位无符号整数
 * 2. 外部显示或配置表示：十进制点分表示的字符串 xx.xx.xx.xxx
 * 
 * 服务器ID结构：[世界服ID(4位)][分区ID(12位)][服务器类型(8位)][实例ID(8位)]
 * 
 * 使用方法：
 * @code
 * // 构造服务器ID
 * uint32_t busId = NFServerIDUtil::MakeProcID(15, 50, 5, 1); // 15.50.5.1
 * 
 * // 解析服务器ID
 * uint32_t worldId = NFServerIDUtil::GetWorldID(busId);      // 15
 * uint32_t zoneId = NFServerIDUtil::GetZoneID(busId);        // 50
 * uint32_t serverType = NFServerIDUtil::GetServerType(busId); // 5
 * uint32_t instId = NFServerIDUtil::GetInstID(busId);        // 1
 * @endcode
 */
class NFServerIDUtil
{
public:
	/**
	 * @brief 从BusID获取世界服ID
	 * 
	 * 通过busid(15.50.5.1)获得世界服ID，15是世界服ID，50是分区ID，5是服务器类型，1是索引
	 * 
	 * @param busId 32位的BusID
	 * @return uint32_t 世界服ID（0-15）
	 */
	static uint32_t GetWorldID(uint32_t busId);

	/**
	 * @brief 从BusID获取分区ID
	 * 
	 * 通过busid(15.50.5.1)获得分区ID，15是世界服ID，50是分区ID，5是服务器类型，1是索引
	 * 
	 * @param busId 32位的BusID
	 * @return uint32_t 分区ID（0-4095）
	 */
	static uint32_t GetZoneID(uint32_t busId);

	/**
	 * @brief 根据区域ID获取对应的分区ID
	 * 
	 * 此函数通过区域ID查找并返回相应的分区ID。区域ID和分区ID之间的映射关系
	 * 基于特定的规则：区域ID = 世界服ID * 10000 + 分区ID
	 * 
	 * @param iZoneAreaID 输入的区域ID
	 * @return int 返回查找到的分区ID。如果找不到对应的分区ID，可能返回特定的错误码或默认值
	 */
	static int GetZoneIDFromZoneAreaID(int iZoneAreaID);

	/**
	 * @brief 从BusID获取服务器类型
	 * 
	 * 通过busid(15.50.5.1)获得服务器类型，15是世界服ID，50是分区ID，5是服务器类型，1是索引
	 * 
	 * @param busId 32位的BusID
	 * @return uint32_t 服务器类型（0-255）
	 */
	static uint32_t GetServerType(uint32_t busId);

	/**
	 * @brief 从BusID获取实例ID
	 * 
	 * 通过busid(15.50.5.1)获得实例ID，15是世界服ID，50是分区ID，5是服务器类型，1是索引
	 * 
	 * @param busId 32位的BusID
	 * @return uint32_t 实例ID（0-255）
	 */
	static uint32_t GetInstID(uint32_t busId);

	/**
	 * @brief 从BusID获取区域服务器ID
	 * 
	 * 通过busid(15.50.5.1)计算区域服务器ID
	 * 区域服务器ID = 世界服ID * 10000 + 分区ID
	 * 
	 * @param busId 32位的BusID
	 * @return uint32_t 区域服务器ID
	 */
	static uint32_t GetZoneAreaID(uint32_t busId);

	/**
	 * @brief 构造进程ID（BusID）
	 * 
	 * 根据各个组件构造32位的进程ID，格式为：世界服ID.分区ID.服务器类型.实例ID
	 * 
	 * @param world 世界服ID，占用4位，范围0-15
	 * @param zone 分区ID，占用12位，范围0-4095
	 * @param servertype 服务器类型，占用8位，范围0-255
	 * @param inst 实例ID，占用8位，范围0-255
	 * @return uint32_t 构造的32位进程ID
	 */
	static uint32_t MakeProcID(int world, int zone, int serverType, int inst);
	
	/**
	 * @brief 构造进程ID的内部实现
	 * 
	 * MakeProcID的具体实现函数
	 * 
	 * @param world 世界服ID
	 * @param zone 分区ID
	 * @param serverType 服务器类型
	 * @param inst 实例ID
	 * @return uint32_t 构造的32位进程ID
	 */
	static uint32_t MakeProcIDImpl(int world, int zone, int serverType, int inst);

	/**
	 * @brief 从BusName字符串获取BusID
	 * 
	 * 将点分十进制字符串（如"15.50.5.1"）转换为32位整数表示的BusID
	 * 
	 * @param busname 字符串格式的BusID，如"15.50.5.1"
	 * @return uint32_t 32位数字表示的BusID
	 */
	static uint32_t GetBusID(const std::string& busname);

	/**
	 * @brief 从BusName字符串获取共享内存对象键值
	 * 
	 * 根据BusName字符串生成共享内存对象的键值
	 * 
	 * @param busname 字符串格式的BusID，如"15.50.5.1"
	 * @return uint32_t 32位数字表示的共享内存对象键值
	 */
	static uint32_t GetShmObjKey(const std::string& busname);

	/**
	 * @brief 从BusID获取BusName字符串
	 * 
	 * 将32位整数BusID转换为点分十进制字符串格式
	 * 
	 * @param busId 32位的BusID
	 * @return std::string 点分十进制格式的BusName字符串
	 */
	static std::string GetBusNameFromBusID(uint32_t busId);
	
	/**
	 * @brief 从BusID字符串获取BusName字符串
	 * 
	 * 字符串格式的BusID转换（可能进行格式验证和标准化）
	 * 
	 * @param busId 字符串格式的BusID
	 * @return std::string 标准化的BusName字符串
	 */
	static std::string GetBusNameFromBusID(const std::string& busId);

	/**
	 * @brief 解析地址字符串
	 * 
	 * 将地址字符串解析为NFChannelAddress结构，支持多种协议格式：
	 * - tcp://127.0.0.1:8123
	 * - udp://127.0.0.1:1000
	 * - bus://127.0.0.1
	 * 
	 * @param addrStr 地址字符串
	 * @param addr [out] 解析后的地址信息
	 * @return bool 解析成功返回true，失败返回false
	 */
	static bool MakeAddress(const std::string& addrStr, NFChannelAddress& addr);

	/**
	 * @brief 构造地址信息
	 * 
	 * 根据协议、主机和端口信息构造完整的地址信息
	 * 
	 * @param scheme 协议名称（如tcp、udp、bus）
	 * @param host 主机地址
	 * @param port 端口号
	 * @param addr [out] 构造的地址信息
	 */
	static void MakeAddress(const std::string& scheme, const std::string& host, int port, NFChannelAddress& addr);
};
