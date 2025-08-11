// -------------------------------------------------------------------------
//    @FileName         :    NFCRC16.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFCRC16.h
 * @brief CRC16循环冗余校验算法实现
 * 
 * 此文件提供了CRC16（16位循环冗余校验）算法的实现，使用CRC-16/CCITT标准。
 * CRC16算法广泛应用于数据通信、存储设备等领域的错误检测。
 */

#pragma once

#include "NFPlatform.h"

/**
 * @brief CRC16循环冗余校验类（CRC-16/CCITT标准）
 * 
 * NFCRC16实现了CRC-16/CCITT算法，这是一种广泛使用的16位循环冗余校验算法。
 * 主要用于检测数据传输或存储过程中的错误。
 * 
 * 主要特性：
 * - CCITT标准：符合CRC-16/CCITT标准（多项式0x1021）
 * - 高效实现：优化的校验算法，适合实时处理
 * - 静态接口：无需实例化，直接调用静态方法
 * - 多重载：支持字节数组和字符串的校验
 * - 线程安全：静态方法，无状态设计
 * 
 * CRC16/CCITT特点：
 * - 多项式：0x1021（x^16 + x^12 + x^5 + 1）
 * - 初始值：0xFFFF
 * - 输出长度：16位校验值（2字节）
 * - 检测能力：能检测所有单比特错误、双比特错误和绝大多数多比特错误
 * - 计算复杂度：O(n)，其中n为数据长度
 * 
 * 适用场景：
 * - 串口通信数据校验
 * - 短消息完整性检查
 * - 嵌入式设备数据校验
 * - 简单协议的错误检测
 * - 低开销的数据完整性验证
 * 
 * 与CRC32的比较：
 * - CRC16：16位校验值，计算更快，校验能力较弱
 * - CRC32：32位校验值，计算稍慢，校验能力更强
 * - 选择建议：短数据或对性能要求高的场景使用CRC16
 * 
 * 使用方法：
 * @code
 * // 计算字节数组的CRC16
 * const char* data = "Hello";
 * uint16_t crc = NFCRC16::Sum(data, strlen(data));
 * std::cout << "CRC16: 0x" << std::hex << crc << std::endl;
 * 
 * // 计算字符串的CRC16
 * std::string message = "测试数据";
 * uint16_t crc2 = NFCRC16::Sum(message);
 * 
 * // 数据完整性验证
 * const char* original = "重要数据";
 * uint16_t original_crc = NFCRC16::Sum(original, strlen(original));
 * 
 * // ... 数据传输 ...
 * 
 * const char* received = "重要数据";  // 接收到的数据
 * uint16_t received_crc = NFCRC16::Sum(received, strlen(received));
 * 
 * if (original_crc == received_crc) {
 *     std::cout << "数据完整" << std::endl;
 * } else {
 *     std::cout << "数据可能损坏" << std::endl;
 * }
 * 
 * // 在协议中使用CRC16
 * struct Message {
 *     char data[100];
 *     uint16_t crc;
 * };
 * 
 * Message msg;
 * strcpy(msg.data, "Hello World");
 * msg.crc = NFCRC16::Sum(msg.data, strlen(msg.data));
 * @endcode
 * 
 * @note 这是一个纯静态类，不需要实例化
 * @note CRC16值不是密码学安全的，不能用于安全认证
 * @note 相同的数据总是产生相同的CRC16值
 * @note CRC16/CCITT与其他CRC16变种（如CRC16/IBM）不兼容
 */
class _NFExport NFCRC16
{
public:
	/**
	 * @brief 计算数据的CRC16校验值
	 * 
	 * 对指定的字节数据计算CRC16/CCITT校验值。
	 * 
	 * @param data 源数据缓冲区指针，不能为nullptr（当len>0时）
	 * @param len 数据长度（字节数），0表示空数据
	 * @return uint16_t CRC16校验值（0x0000-0xFFFF）
	 * 
	 * @note 使用CRC-16/CCITT算法（多项式0x1021）
	 * @note 如果数据长度大于实际缓冲区大小，结果未定义
	 * @note 空数据（len=0）的CRC16值由算法初始值决定
	 * @note 算法使用标准的CCITT参数配置
	 */
	static uint16_t Sum(const void* data, size_t len);

	/**
	 * @brief 计算字符串的CRC16校验值
	 * 
	 * 对std::string对象计算CRC16/CCITT校验值的便捷方法。
	 * 
	 * @param s 要计算校验值的字符串
	 * @return uint16_t CRC16校验值（0x0000-0xFFFF）
	 * 
	 * @note 计算整个字符串内容的校验值，包括嵌入的'\0'字符
	 * @note 空字符串的CRC16值由算法初始值决定
	 * @note 字符串编码不影响计算结果，按字节计算
	 */
	static uint16_t Sum(const std::string& s)
	{
		return Sum(s.data(), s.size());
	}
};

