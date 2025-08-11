// -------------------------------------------------------------------------
//    @FileName         :    NFPacketParseMgr.cpp
//    @Author           :    gaoyi
//    @Date             :    24-8-1
//    @Email            :    445267987@qq.com
//    @Module           :    NFPacketParseMgr
//    @Desc             :    数据包解析管理器实现，统一管理不同类型的数据包解析器
//
// -------------------------------------------------------------------------

#include "NFPacketParseMgr.h"

#include <NFComm/NFPluginModule/NFCheck.h>

#include "InternalPacketParse.h"
#include "NFComm/NFPluginModule/NFIPacketParse.h"

/**
 * @file NFPacketParseMgr.cpp
 * @brief 数据包解析管理器实现文件
 * 
 * 该文件实现了数据包解析管理器的核心功能，包括：
 * - 数据包解析器的创建和销毁
 * - 统一的编解码接口实现
 * - 压缩解压功能实现
 * - 解析器管理功能
 * 
 * 主要功能：
 * - 统一管理不同类型的数据包解析器
 * - 提供统一的编解码接口
 * - 支持数据压缩和解压
 * - 支持解析器的动态注册和管理
 * 
 * @author gaoyi
 * @date 24-8-1
 * @version 1.0
 */

/**
 * @brief 数据包解析器数组，预初始化5个内部解析器
 * 
 * 默认创建5个内部数据包解析器实例，用于处理不同类型的网络协议
 */
std::vector<NFIPacketParse*> NFPacketParseMgr::m_pPacketParse = {CreatePacketParse(PACKET_PARSE_TYPE_INTERNAL), CreatePacketParse(PACKET_PARSE_TYPE_INTERNAL), CreatePacketParse(PACKET_PARSE_TYPE_INTERNAL), CreatePacketParse(PACKET_PARSE_TYPE_INTERNAL), CreatePacketParse(PACKET_PARSE_TYPE_INTERNAL)};

/**
 * @brief 删除指定的数据包解析器
 * 
 * 安全删除解析器对象，避免内存泄漏
 * 
 * @param pPacketParse 要删除的解析器指针
 */
void NFPacketParseMgr::DeletePacketParse(const NFIPacketParse* pPacketParse)
{
	if (pPacketParse)
	{
		NF_SAFE_DELETE(pPacketParse);
	}
}

/**
 * @brief 释放所有数据包解析器
 * 
 * 清理所有已创建的解析器对象，通常在程序退出时调用
 */
void NFPacketParseMgr::ReleasePacketParse()
{
	for (size_t i = 0; i < m_pPacketParse.size(); i++)
	{
		DeletePacketParse(m_pPacketParse[i]);
	}
	m_pPacketParse.clear();
}

/**
 * @brief 重置指定类型的数据包解析器
 * 
 * 用新的解析器替换指定类型的解析器，支持动态切换解析策略
 * 
 * @param parseType 解析器类型
 * @param pPacketParse 新的解析器指针
 * @return 操作结果，0表示成功，-1表示失败
 */
int NFPacketParseMgr::ResetPacketParse(uint32_t parseType, NFIPacketParse* pPacketParse)
{
	CHECK_NULL(0, pPacketParse);
	CHECK_EXPR(parseType < m_pPacketParse.size(), -1, "parseType:{}", parseType);
	if (m_pPacketParse[parseType])
	{
		DeletePacketParse(m_pPacketParse[parseType]);
		m_pPacketParse[parseType] = nullptr;
	}
	CHECK_EXPR(m_pPacketParse[parseType] == NULL, -1, "parseType:{} exist", parseType);
	m_pPacketParse[parseType] = pPacketParse;
	return 0;
}

/**
 * @brief 创建指定类型的数据包解析器
 * 
 * 根据解析器类型创建相应的解析器实例
 * 
 * @param parseType 解析器类型
 * @return 创建的解析器指针
 */
NFIPacketParse* NFPacketParseMgr::CreatePacketParse(int parseType)
{
	if (parseType == PACKET_PARSE_TYPE_INTERNAL)
	{
		return NF_NEW InternalPacketParse();
	}
	else
	{
		// 目前只支持内部解析器，其他类型也返回内部解析器
		return NF_NEW InternalPacketParse();
	}
}

/**
 * @brief 解码数据包
 * 
 * 根据包类型选择合适的解析器进行解码
 * 
 * @param packetType 包类型
 * @param strData 输入数据
 * @param unLen 输入数据长度
 * @param outData 输出数据指针
 * @param outLen 输出数据长度
 * @param allLen 总数据长度
 * @param recvPackage 接收到的数据包
 * @return 解码结果，0表示成功，-1表示失败
 */
int NFPacketParseMgr::DeCode(uint32_t packetType, const char* strData, uint32_t unLen, char*& outData, uint32_t& outLen, uint32_t& allLen, NFDataPackage& recvPackage)
{
	CHECK_EXPR(packetType < m_pPacketParse.size(), -1, "packetType:{}", packetType);
	CHECK_NULL(0, m_pPacketParse[packetType]);
	return m_pPacketParse[packetType]->DeCodeImpl(strData, unLen, outData, outLen, allLen, recvPackage);
}

/**
 * @brief 编码数据包
 * 
 * 根据包类型选择合适的解析器进行编码
 * 
 * @param packetType 包类型
 * @param recvPackage 要编码的数据包
 * @param strData 额外数据
 * @param unLen 额外数据长度
 * @param buffer 输出缓冲区
 * @param nSendBusLinkId 发送总线连接ID
 * @return 编码结果，0表示成功，-1表示失败
 */
int NFPacketParseMgr::EnCode(uint32_t packetType, const NFDataPackage& recvPackage, const char* strData, uint32_t unLen, NFBuffer& buffer, uint64_t nSendBusLinkId)
{
	CHECK_EXPR(packetType < m_pPacketParse.size(), -1, "packetType:{}", packetType);
	CHECK_NULL(0, m_pPacketParse[packetType]);
	return m_pPacketParse[packetType]->EnCodeImpl(recvPackage, strData, unLen, buffer, nSendBusLinkId);
}

/**
 * @brief 压缩数据
 * 
 * 使用lzf算法压缩数据，减少网络传输量
 * 
 * @param packetType 包类型
 * @param inBuffer 输入缓冲区
 * @param inLen 输入数据长度
 * @param outBuffer 输出缓冲区
 * @param outSize 输出缓冲区大小
 * @return 压缩结果，0表示成功，-1表示失败
 */
int NFPacketParseMgr::Compress(uint32_t packetType, const char* inBuffer, int inLen, void* outBuffer, unsigned int outSize)
{
	CHECK_EXPR(packetType < m_pPacketParse.size(), -1, "packetType:{}", packetType);
	CHECK_NULL(0, m_pPacketParse[packetType]);
	return m_pPacketParse[packetType]->CompressImpl(inBuffer, inLen, outBuffer, outSize);
}

/**
 * @brief 解压数据
 * 
 * 使用lzf算法解压数据，恢复原始数据
 * 
 * @param packetType 包类型
 * @param inBuffer 输入缓冲区
 * @param inLen 输入数据长度
 * @param outBuffer 输出缓冲区
 * @param outSize 输出缓冲区大小
 * @return 解压结果，0表示成功，-1表示失败
 */
int NFPacketParseMgr::Decompress(uint32_t packetType, const char* inBuffer, int inLen, void* outBuffer, int outSize)
{
	CHECK_EXPR(packetType < m_pPacketParse.size(), -1, "packetType:{}", packetType);
	CHECK_NULL(0, m_pPacketParse[packetType]);
	return m_pPacketParse[packetType]->DecompressImpl(inBuffer, inLen, outBuffer, outSize);
}
