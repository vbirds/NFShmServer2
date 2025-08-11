// -------------------------------------------------------------------------
//    @FileName         :    InternalPacketParse.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//
// -------------------------------------------------------------------------
#include "InternalPacketParse.h"
#include "NFComm/NFPluginModule/NFNetDefine.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"

/**
 * @file InternalPacketParse.cpp
 * @brief 内部数据包解析器实现文件
 * 
 * 该文件实现了内部通信协议的数据包解析功能，包括：
 * - 内部消息结构体定义和实现
 * - 数据包解码功能实现
 * - 数据包编码功能实现
 * - 模块ID和命令ID的处理
 * 
 * 主要功能：
 * - 解析内部通信协议的数据包
 * - 支持模块ID和命令ID的编码
 * - 支持压缩标志位处理
 * - 提供统一的编解码接口
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

#pragma pack(push)
#pragma pack(1)

/**
 * @struct InternalMsg
 * @brief 内部消息结构体
 * 
 * 定义了内部通信协议的消息格式，包括：
 * - 高16位：1位压缩标志 + 15位模块号
 * - 低16位：16位命令号
 * - 消息长度、参数、源ID、目标ID等信息
 * 
 * 数据包格式：
 * - m_cmdAndFlag: 命令和标志位（32位）
 * - m_length: 消息长度（32位）
 * - m_param1, m_param2: 参数（64位）
 * - m_srcId, m_dstId: 源ID和目标ID（64位）
 * - m_sendBusLinkId: 发送总线连接ID（64位）
 * - m_errCode: 错误代码（32位）
 */
struct InternalMsg
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化所有成员变量为0
	 */
	InternalMsg() : m_cmdAndFlag(0), m_length(0), m_param1(0), m_param2(0), m_srcId(0), m_dstId(0), m_sendBusLinkId(0), m_errCode(0)
	{
	}

	/**
	 * @brief 设置消息长度
	 * @param len 消息长度
	 */
	void SetLength(uint32_t len)
	{
		m_length = len;
	}

	/**
	 * @brief 获取消息长度
	 * @return 消息长度
	 */
	uint32_t GetLength() const
	{
		return m_length;
	}

	/**
	 * @brief 设置模块ID
	 * 
	 * 将模块ID设置到高16位，保留压缩标志位
	 * @param module 模块ID
	 */
	void SetModule(uint16_t module)
	{
		uint16_t hiWord = MMO_HIWORD(m_cmdAndFlag);
		hiWord = module | (hiWord & (1 << s_compressBitPos));
		m_cmdAndFlag = MMO_MAKELONG(MMO_LOWORD(m_cmdAndFlag), hiWord);
	}

	/**
	 * @brief 设置命令ID
	 * 
	 * 将命令ID设置到低16位
	 * @param cmd 命令ID
	 */
	void SetCmd(uint16_t cmd)
	{
		m_cmdAndFlag = MMO_MAKELONG(cmd, MMO_HIWORD(m_cmdAndFlag));
	}

	/**
	 * @brief 获取模块ID
	 * 
	 * 从高16位提取模块ID，忽略压缩标志位
	 * @return 模块ID
	 */
	uint16_t GetModule() const
	{
		uint16_t hiWord = MMO_HIWORD(m_cmdAndFlag);
		return (hiWord & (~(1 << s_compressBitPos)));
	}

	/**
	 * @brief 获取命令ID
	 * 
	 * 从低16位提取命令ID
	 * @return 命令ID
	 */
	uint16_t GetCmd() const
	{
		return MMO_LOWORD(m_cmdAndFlag);
	}

	/**
	 * @brief 设置压缩标志
	 * 
	 * 在高16位设置压缩标志位
	 */
	void SetCompress()
	{
		uint16_t hiWord = MMO_HIWORD(m_cmdAndFlag);
		hiWord |= (1 << s_compressBitPos);
		m_cmdAndFlag = MMO_MAKELONG(MMO_LOWORD(m_cmdAndFlag), hiWord);
	}

	/**
	 * @brief 清除压缩标志
	 * 
	 * 在高16位清除压缩标志位
	 */
	void ClearCompress()
	{
		uint16_t hiWord = MMO_HIWORD(m_cmdAndFlag);
		hiWord &= ~(1 << s_compressBitPos);
		m_cmdAndFlag = MMO_MAKELONG(MMO_LOWORD(m_cmdAndFlag), hiWord);
	}

	/**
	 * @brief 检查是否压缩
	 * 
	 * 检查高16位的压缩标志位是否设置
	 * @return true 已压缩，false 未压缩
	 */
	bool IsCompressed() const
	{
		uint16_t hiWord = MMO_HIWORD(m_cmdAndFlag);
		return 0 != (hiWord & (1 << s_compressBitPos));
	}

	uint32_t m_cmdAndFlag; ///< 命令和标志位（高16位：1位压缩 + 15位模块号，低16位：16位命令号）
	uint32_t m_length; ///< 消息长度
	uint64_t m_param1; ///< 参数1
	uint64_t m_param2; ///< 参数2
	uint64_t m_srcId; ///< 源ID
	uint64_t m_dstId; ///< 目标ID
	uint64_t m_sendBusLinkId; ///< 发送总线连接ID（总线消息需要）
	int32_t m_errCode; ///< 错误代码（主要是路由错误）
};

#pragma pack(pop)

/**
 * @brief 内部数据包解析器构造函数
 */
InternalPacketParse::InternalPacketParse()
{
}

/**
 * @brief 数据包解码实现
 * 
 * 将网络数据解码为内部消息格式，包括：
 * - 验证输入参数的有效性
 * - 解析数据包头信息
 * - 提取模块ID和命令ID
 * - 验证数据包完整性
 * - 填充NFDataPackage结构
 * 
 * @param strData 输入数据指针
 * @param unLen 输入数据长度
 * @param outData 输出数据指针（引用返回）
 * @param outLen 输出数据长度（引用返回）
 * @param allLen 总数据长度（引用返回）
 * @param recvPackage 接收到的数据包（引用返回）
 * @return 解码结果，0表示成功，1表示数据不完整，-1表示错误
 */
int InternalPacketParse::DeCodeImpl(const char* strData, uint32_t unLen, char*& outData, uint32_t& outLen, uint32_t& allLen, NFDataPackage& recvPackage)
{
	if (strData == nullptr || unLen == 0) return 1;

	InternalMsg* packHead = nullptr;

	if (unLen < static_cast<uint32_t>(sizeof(InternalMsg)))
	{
		return 1;
	}

	packHead = (InternalMsg*)strData; //-V519
	uint32_t msgSize = packHead->m_length;
	uint32_t moduleId = packHead->GetModule();
	uint32_t tmpMsgId = packHead->GetCmd();

	if (sizeof(InternalMsg) + msgSize >= MAX_RECV_BUFFER_SIZE) //-V560
	{
		NFLogError(NF_LOG_DEFAULT, 0, "net server parse data failed, msgSize:{}, moduleId:{} msgId:{}", msgSize, moduleId, tmpMsgId);
		return -1;
	}

	if (sizeof(InternalMsg) + msgSize > unLen)
	{
		return 1;
	}

	outData = const_cast<char*>(strData + sizeof(InternalMsg));
	outLen = msgSize;
	recvPackage.mModuleId = moduleId;
	recvPackage.nMsgId = tmpMsgId;
	recvPackage.nParam1 = packHead->m_param1;
	recvPackage.nParam2 = packHead->m_param2;
	recvPackage.nSrcId = packHead->m_srcId;
	recvPackage.nDstId = packHead->m_dstId;
	recvPackage.nErrCode = packHead->m_errCode;
	recvPackage.nSendBusLinkId = packHead->m_sendBusLinkId;
	allLen = sizeof(InternalMsg) + msgSize;
	return 0;
}

/**
 * @brief 数据包编码实现
 * 
 * 将内部消息格式编码为网络数据，包括：
 * - 构建数据包头信息
 * - 设置模块ID和命令ID
 * - 添加消息数据
 * - 写入输出缓冲区
 * 
 * @param recvPackage 要编码的数据包
 * @param strData 额外数据指针
 * @param unLen 额外数据长度
 * @param buffer 输出缓冲区
 * @param nSendBusLinkId 发送总线连接ID，默认为0
 * @return 编码后的数据长度
 */
int InternalPacketParse::EnCodeImpl(const NFDataPackage& recvPackage, const char* strData, uint32_t unLen, NFBuffer& buffer, uint64_t nSendBusLinkId)
{
	InternalMsg packHead;
	packHead.SetModule(recvPackage.mModuleId);
	packHead.SetCmd(recvPackage.nMsgId);
	packHead.SetLength(unLen);
	packHead.m_param1 = recvPackage.nParam1;
	packHead.m_param2 = recvPackage.nParam2;
	packHead.m_srcId = recvPackage.nSrcId;
	packHead.m_dstId = recvPackage.nDstId;
	packHead.m_sendBusLinkId = nSendBusLinkId;
	packHead.m_errCode = recvPackage.nErrCode;

	buffer.PushData(&packHead, sizeof(InternalMsg));
	buffer.PushData(strData, unLen);

	return packHead.m_length;
}
