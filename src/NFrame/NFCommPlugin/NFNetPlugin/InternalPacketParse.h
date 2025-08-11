// -------------------------------------------------------------------------
//    @FileName         :    InternalPacketParse.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    内部数据包解析器头文件，定义内部通信协议的数据包解析器。
//                          该文件定义了内部通信协议的数据包解析器，包括内部数据包解析器类定义、
//                          内部消息结构体定义、数据包编解码接口。
//                          主要功能包括解析内部通信协议的数据包、支持模块ID和命令ID的编码、
//                          支持压缩标志位处理、提供统一的编解码接口。
//                          内部数据包解析器是NFShmXFrame框架网络通信的核心组件，负责：
//                          - 内部通信协议的数据包解析
//                          - 模块ID和命令ID的处理
//                          - 压缩标志位的处理
//                          - 数据包完整性验证
//                          - 高性能的数据包编解码
//                          - 统一的编解码接口
//
// -------------------------------------------------------------------------
#pragma once

#include <NFComm/NFPluginModule/NFIPacketParse.h>

/**
 * @brief 内部数据包解析器类
 * 
 * 该类实现了内部通信协议的数据包解析功能，包括：
 * - 数据包解码：从网络数据中提取消息内容
 * - 数据包编码：将消息内容打包成网络数据
 * - 支持模块ID和命令ID的处理
 * - 支持压缩标志位的处理
 * - 数据包完整性验证
 * - 高性能数据处理
 * 
 * 使用方式：
 * @code
 * InternalPacketParse parser;
 * NFDataPackage package;
 * char* outData;
 * uint32_t outLen, allLen;
 * int result = parser.DeCodeImpl(inputData, inputLen, outData, outLen, allLen, package);
 * @endcode
 */
class InternalPacketParse : public NFIPacketParse
{
public:
	/**
	 * @brief 构造函数
	 */
	InternalPacketParse();
	
	/**
	 * @brief 数据包解码实现
	 * 
	 * 将网络数据解码为内部消息格式，包括：
	 * - 解析数据包头信息
	 * - 提取模块ID和命令ID
	 * - 验证数据包完整性
	 * - 填充NFDataPackage结构
	 * - 处理压缩标志位
	 * 
	 * @param strData 输入数据指针
	 * @param unLen 输入数据长度
	 * @param outData 输出数据指针（引用返回）
	 * @param outLen 输出数据长度（引用返回）
	 * @param allLen 总数据长度（引用返回）
	 * @param recvPackage 接收到的数据包（引用返回）
	 * @return 解码结果，0表示成功，1表示数据不完整，-1表示错误
	 */
	int DeCodeImpl(const char* strData, uint32_t unLen, char*& outData, uint32_t& outLen, uint32_t& allLen, NFDataPackage& recvPackage) override;
	
	/**
	 * @brief 数据包编码实现
	 * 
	 * 将内部消息格式编码为网络数据，包括：
	 * - 构建数据包头信息
	 * - 设置模块ID和命令ID
	 * - 添加消息数据
	 * - 写入输出缓冲区
	 * - 处理压缩标志位
	 * 
	 * @param recvPackage 要编码的数据包
	 * @param strData 额外数据指针
	 * @param unLen 额外数据长度
	 * @param buffer 输出缓冲区
	 * @param nSendBusLinkId 发送总线连接ID，默认为0
	 * @return 编码后的数据长度
	 */
	int EnCodeImpl(const NFDataPackage& recvPackage, const char* strData, uint32_t unLen, NFBuffer& buffer, uint64_t nSendBusLinkId = 0) override;
};
