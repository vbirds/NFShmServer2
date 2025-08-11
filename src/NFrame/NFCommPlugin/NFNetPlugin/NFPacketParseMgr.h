// -------------------------------------------------------------------------
//    @FileName         :    NFPacketParseMgr.h
//    @Author           :    gaoyi
//    @Date             :    24-8-1
//    @Email            :    445267987@qq.com
//    @Module           :    NFPacketParseMgr
//    @Desc             :    数据包解析管理器头文件，统一管理不同类型的数据包解析器。
//                          该文件定义了数据包解析管理器，包括数据包解析管理器类定义、
//                          编解码接口函数、压缩解压接口函数、解析器管理接口。
//                          主要功能包括统一管理不同类型的数据包解析器、提供统一的编解码接口、
//                          支持数据压缩和解压、支持解析器的动态注册和管理。
//                          数据包解析管理器是NFShmXFrame框架网络通信的核心组件，负责：
//                          - 统一管理不同类型的数据包解析器
//                          - 提供统一的编解码接口
//                          - 支持数据压缩和解压功能
//                          - 支持解析器的动态注册和管理
//                          - 支持多种数据包格式处理
//                          - 提供高性能的数据包处理能力
//
// -------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <vector>
#include "NFComm/NFCore/NFBuffer.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
class NFIPacketParse;

/**
 * @brief 数据包解析管理器类
 * 
 * 该类负责管理和协调不同类型的数据包解析器，提供统一的编解码接口
 * 支持多种数据包格式的处理，包括：
 * - 内部通信协议解析
 * - 外部协议解析
 * - 自定义协议解析
 * - 二进制协议解析
 * - 文本协议解析
 * 
 * 主要功能：
 * - 根据包类型选择合适的解析器
 * - 统一的编解码接口
 * - 解析器的注册和管理
 * - 数据压缩和解压
 * - 高性能数据处理
 * 
 * 使用方式：
 * @code
 * // 解码数据包
 * char* outData;
 * uint32_t outLen, allLen;
 * NFDataPackage package;
 * int result = NFPacketParseMgr::DeCode(packetType, inputData, inputLen, 
 *                                       outData, outLen, allLen, package);
 * 
 * // 编码数据包
 * NFBuffer buffer;
 * int encodedLen = NFPacketParseMgr::EnCode(packetType, package, data, len, buffer);
 * @endcode
 */
class NFPacketParseMgr
{
public:
	/////////////////////////////////////////////////////////////
	/**
	 * @brief 解码函数
	 *
	 * 根据指定的包类型和输入的数据，解码出原始信息
	 *
	 * @param packetType 包类型，用于区分不同的数据包格式
	 * @param strData 输入的数据字符串
	 * @param unLen 输入数据的长度
	 * @param outData 解码后的数据指针，通过引用返回
	 * @param outLen 解码后的数据长度，通过引用返回
	 * @param allLen 总的数据长度，包括解码前和解码后的，通过引用返回
	 * @param recvPackage 解码后的数据包对象，包含解码结果和包类型等信息
	 * @return 返回状态码，表示解码是否成功
	 */
	static int DeCode(uint32_t packetType, const char* strData, uint32_t unLen, char*& outData, uint32_t& outLen, uint32_t& allLen, NFDataPackage& recvPackage);

	/**
	 * @brief 编码函数
	 *
	 * 根据指定的包类型和输入的数据包对象，编码成特定格式的数据
	 *
	 * @param packetType 包类型，用于区分不同的数据包格式
	 * @param recvPackage 数据包对象，包含需要编码的数据和包类型等信息
	 * @param strData 额外输入的数据字符串，用于编码
	 * @param unLen 额外输入数据的长度
	 * @param buffer 编码后的数据缓冲区对象
	 * @param nSendBusLinkId 发送总线链接ID，用于标识数据发送的目标，默认为0
	 * @return 返回状态码，表示编码是否成功
	 */
	static int EnCode(uint32_t packetType, const NFDataPackage& recvPackage, const char* strData, uint32_t unLen, NFBuffer& buffer, uint64_t nSendBusLinkId = 0);

	/**
	 * @brief 压缩函数
	 *
	 * 使用 lzf 算法对输入的数据进行压缩
	 *
	 * @param packetType 包类型，用于区分不同的数据包格式
	 * @param inBuffer 输入的数据缓冲区指针
	 * @param inLen 输入数据的长度
	 * @param outBuffer 压缩后的数据输出缓冲区指针
	 * @param outSize 压缩后的数据输出缓冲区大小
	 * @return 返回压缩后的数据长度，如果压缩失败则返回负值
	 */
	static int Compress(uint32_t packetType, const char* inBuffer, int inLen, void* outBuffer, unsigned int outSize);

	/**
	 * @brief 解压缩函数
	 *
	 * 使用 lzf 算法对输入的压缩数据进行解压缩
	 *
	 * @param packetType 包类型，用于区分不同的数据包格式
	 * @param inBuffer 输入的压缩数据缓冲区指针
	 * @param inLen 输入的压缩数据长度
	 * @param outBuffer 解压缩后的数据输出缓冲区指针
	 * @param outSize 解压缩后的数据输出缓冲区大小
	 * @return 返回解压缩后的数据长度，如果解压缩失败则返回负值
	 */
	static int Decompress(uint32_t packetType, const char* inBuffer, int inLen, void* outBuffer, int outSize);

	////////////////////////////////////////////////////////////
	/**
	 * @brief 创建一个数据包解析器对象
	 *
	 * @param parseType 解析器类型，默认为0。可能用于区分不同的解析规则或格式
	 * @return 返回创建的数据包解析器对象指针
	 *
	 * 此函数用于根据解析类型动态生成一个数据包解析器对象，供后续的数据解析使用
	 */
	static NFIPacketParse* CreatePacketParse(int parseType = 0);

	/**
	 * @brief 删除一个不再需要的数据包解析器对象
	 *
	 * @param pPacketParse 指向要删除的数据包解析器对象的指针
	 *
	 * 此函数用于释放内存中指定的数据包解析器对象，以防止内存泄漏
	 */
	static void DeletePacketParse(const NFIPacketParse* pPacketParse);

	/**
	 * @brief 释放所有数据包解析器资源
	 *
	 * 此函数可能用于在程序结束或重新初始化解析器之前，释放所有已分配的解析器资源
	 */
	static void ReleasePacketParse();

	/**
	 * @brief 重置数据包解析器的配置
	 *
	 * @param parseType 解析器类型，用于指定新的解析规则或格式
	 * @param pPacketParse 指向需要重置的数据包解析器对象
	 * @return 返回重置结果，成功或失败的标识
	 *
	 * 此函数允许在不删除解析器对象的情况下，更新解析器的配置，以适应新的解析需求
	 */
	static int ResetPacketParse(uint32_t parseType, NFIPacketParse* pPacketParse);

public:
	/**
	 * @brief 数据包解析器数组
	 * 
	 * 存储所有已注册的数据包解析器，按解析类型索引
	 */
	static std::vector<NFIPacketParse*> m_pPacketParse;
};
