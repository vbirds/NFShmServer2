// -------------------------------------------------------------------------
//    @FileName         :    NFCKernelModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFCKernelModule
//    @Desc             :    内核模块头文件，提供UUID生成、加密散列等核心功能。
//                          该文件定义了NFShmXFrame框架的核心内核模块，提供系统基础服务功能，
//                          包括UUID生成服务、加密散列功能、系统服务管理。
//                          主要功能包括64位和53位UUID生成、MD5哈希计算、CRC校验和计算、
//                          Base64编码解码、定时器支持、事件处理、服务器进程管理
//    @Description      :    内核模块头文件，提供UUID生成、加密散列等核心功能
//
// -------------------------------------------------------------------------

#ifndef NFC_KERNEL_MODULE_H
#define NFC_KERNEL_MODULE_H

#include "NFComm/NFPluginModule/NFIKernelModule.h"

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <chrono>
#include <unordered_map>

/**
 * @brief UUID生成规则（64位版本）
 * 
 * 位分布：
 * 63-60: 4位世界ID (WorldID)
 * 59-48: 12位区服ID (ZoneID)  
 * 47-44: 4位检查序号 (CheckSeq)
 * 43-32: 12位序列号 (Sequence)
 * 31-0:  32位时间戳 (Time)
 */
#define    UNIQUE_ID_START_TIME			1560000000//Jun  8 21:20:00 2019
#define    WORLDID_MASK					0xF000000000000000
#define    ZONEID_MASK					0x0FFF000000000000
#define    CHECK_SEQ_MASK				0x0000F00000000000
#define    ONE_SECOND_SEQ_MASK			0x00000FFF00000000
#define    ADAPTIVE_TIME_MASK			0x00000000FFFFFFFF
#define    ONE_SECOND_SEQ_NUM			0xFFF

/**
 * @brief UUID生成规则（53位版本）
 * 
 * 由于客户端LUA中int64_t类型最大只能表示2的53次方，所以角色cid由原来的64位改成现在的53位表示
 * 
 * 位分布：
 * 63-60: 4位世界ID (WorldID)
 * 59-48: 12位区服ID (ZoneID)
 * 47-44: 4位检查序号 (CheckSeq，服务器重启16次)
 * 43-32: 12位序列号 (Sequence)
 * 31-0:  32位时间戳 (Time)
 */
#define    WORLDID_53_MASK        0x000F00000000000000
#define    ZONEID_53_MASK         0x0000FFF000000000
#define    CHECK_SEQ_53_MASK      0x0000000F00000000

/**
 * @class NFCKernelModule
 * @brief 内核模块实现类
 *
 * NFCKernelModule是NFShmXFrame框架的核心模块，提供了以下主要功能：
 * 
 * UUID生成服务：
 * - 生成全局唯一的64位ID，支持分布式环境
 * - 生成53位ID，兼容客户端Lua环境
 * - 生成32位UUID用于特殊场景
 * - 时间戳+序列号+区服ID+世界ID的组合方式确保唯一性
 * 
 * 加密散列功能：
 * - MD5哈希计算
 * - CRC32和CRC16校验和计算
 * - Base64编码和解码
 * 
 * 系统服务：
 * - 定时器支持
 * - 事件处理
 * - 服务器进程管理
 * 
 * @note UUID生成算法考虑了时间、区服、世界等多维度信息
 * @note 支持服务器重启后的序号连续性保证
 */
class NFCKernelModule : public NFIKernelModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFCKernelModule(NFIPluginManager* p);
	
	/**
	 * @brief 析构函数
	 */
	virtual ~NFCKernelModule();

	/**
	 * @brief 初始化模块
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Init() override;
	
	/**
	 * @brief 关闭模块
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Shut() override;
	
	/**
	 * @brief 最终化处理
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Finalize() override;

	/**
	 * @brief 关闭前处理
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int BeforeShut() override;

	/**
	 * @brief 模块定时更新
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Tick() override;

	/**
	 * @brief 执行事件处理
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 源类型
	 * @param nSrcID 源ID
	 * @param pMessage 消息指针
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage) override;

public:
	/**
	 * @brief 获取随机UUID
	 * @return 返回随机生成的UUID
	 * 
	 * 生成完全随机的UUID，不保证全局唯一性，
	 * 适用于临时标识或测试场景。
	 */
	virtual uint64_t GetUUID() override;
	
	/**
	 * @brief 计算字符串的MD5哈希值
	 * @param str 输入字符串
	 * @return 返回MD5哈希值的十六进制字符串
	 */
	virtual std::string GetMD5(const std::string& str) override;
	
	/**
	 * @brief 计算字符串的CRC32校验和
	 * @param s 输入字符串
	 * @return 返回CRC32校验和
	 */
	virtual uint32_t GetCRC32(const std::string& s) override;
	
	/**
	 * @brief 计算字符串的CRC16校验和
	 * @param s 输入字符串
	 * @return 返回CRC16校验和
	 */
	virtual uint16_t GetCRC16(const std::string& s) override;
	
	/**
	 * @brief Base64编码
	 * @param s 待编码的字符串
	 * @return 返回Base64编码后的字符串
	 */
	virtual std::string Base64Encode(const std::string& s) override;
	
	/**
	 * @brief Base64解码
	 * @param s 待解码的Base64字符串
	 * @return 返回解码后的原始字符串
	 */
	virtual std::string Base64Decode(const std::string& s) override;

	/**
	 * @brief 获取64位全局唯一ID
	 * @return 返回64位UUID
	 * 
	 * 生成全局唯一的64位ID，包含时间戳、序列号、区服ID、世界ID等信息，
	 * 确保在分布式环境中的唯一性。
	 */
	virtual uint64_t Get64UUID() override;
	
	/**
	 * @brief 获取53位全局唯一ID
	 * @return 返回53位UUID
	 * 
	 * 为了兼容客户端Lua环境的数值限制（2^53），生成53位的UUID。
	 * 主要用于角色ID等需要传递到客户端的标识符。
	 */
    virtual uint64_t Get53UUID() override;
    
	/**
	 * @brief 获取32位全局唯一ID
	 * @return 返回32位UUID
	 * 
	 * 生成32位的UUID，适用于对ID长度有特殊要求的场景。
	 */
	virtual uint64_t Get32UUID() override;

	/**
	 * @brief 定时器回调处理
	 * @param nTimerID 定时器ID
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int OnTimer(uint32_t nTimerID) override;

	/**
	 * @brief 处理杀死服务器进程的消息
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 返回0表示成功，非0表示失败
	 */
    int OnKillServerProcess(uint64_t unLinkId, NFDataPackage& packet);

protected:
	/**
	 * @brief 生成下一个64位UUID的序列号
	 * @param iNowSec 当前时间戳（秒）
	 * @return 返回新的序列号
	 */
    int Next(int iNowSec);
    
	/**
	 * @brief 生成下一个53位UUID的序列号
	 * @param iNowSec 当前时间戳（秒）
	 * @return 返回新的序列号
	 */
    int Next53(int iNowSec);
    
	/**
	 * @brief 更新检查序号
	 * @param szCheckSeqFile 检查序号文件路径
	 * @return 返回更新后的检查序号
	 * 
	 * 从文件中读取并更新检查序号，用于服务器重启后的序号连续性保证。
	 */
    uint8_t UpdateCheckSeq(const std::string& szCheckSeqFile);

private:
	/**
	 * @brief 唯一ID文件路径
	 * 用于存储UUID生成相关的持久化信息
	 */
	std::string szUniqIDFile;
	
	/**
	 * @brief UUID生成掩码
	 * 用于UUID位运算的掩码值
	 */
    uint64_t m_ullMask;
    
	/**
	 * @brief 检查序号
	 * 用于服务器重启检测的序号
	 */
    uint64_t m_ucCheckSeq;
    
	/**
	 * @brief 当前序列号
	 * UUID生成的递增序列号
	 */
    uint64_t m_ushSequence;
    
	/**
	 * @brief 适应时间
	 * UUID时间戳的适应性调整值
	 */
    uint64_t m_iAdaptiveTime;
    
	/**
	 * @brief 区服ID
	 * 当前服务器所属的区服标识
	 */
    uint64_t m_iZoneId;
    
	/**
	 * @brief 世界ID
	 * 当前服务器所属的世界标识
	 */
    uint64_t m_iWorldId;
    
	/**
	 * @brief 上次GUID生成的时间戳
	 * 用于检测时间回溯和序号重置
	 */
    uint64_t mLastGuidTimeStamp;
};

#endif

