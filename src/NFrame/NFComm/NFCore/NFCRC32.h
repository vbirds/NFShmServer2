// -------------------------------------------------------------------------
//    @FileName         :    NFCRC32.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFCRC32.h
 * @brief CRC32循环冗余校验算法实现
 * 
 * 此文件提供了CRC32（32位循环冗余校验）算法的实现，用于数据完整性验证。
 * CRC32是一种广泛使用的错误检测算法，常用于网络传输、文件校验等场景。
 */

#pragma once

#include "NFPlatform.h"

/**
 * @brief CRC32循环冗余校验类
 * 
 * NFCRC32提供了CRC32算法的静态实现，用于计算数据的32位循环冗余校验值。
 * CRC32算法能够检测数据传输或存储过程中的错误，广泛应用于数据完整性验证。
 * 
 * 主要特性：
 * - 标准CRC32算法：符合IEEE 802.3标准
 * - 高效实现：使用查表法提高计算速度
 * - 静态接口：无需实例化，直接调用静态方法
 * - 多重载：支持字节数组和字符串的校验
 * - 线程安全：静态方法，无状态设计
 * 
 * CRC32特点：
 * - 检测能力：能检测所有单比特错误、双比特错误和奇数个比特错误
 * - 多项式：使用标准CRC32多项式 0xEDB88320
 * - 输出长度：32位校验值（4字节）
 * - 计算复杂度：O(n)，其中n为数据长度
 * 
 * 适用场景：
 * - 网络数据包校验
 * - 文件完整性检查
 * - 数据传输错误检测
 * - 存储设备数据校验
 * - 协议栈实现
 * 
 * 使用方法：
 * @code
 * // 计算字节数组的CRC32
 * const char* data = "Hello, World!";
 * uint32_t crc = NFCRC32::Sum(data, strlen(data));
 * std::cout << "CRC32: 0x" << std::hex << crc << std::endl;
 * 
 * // 计算字符串的CRC32
 * std::string str = "测试数据";
 * uint32_t crc2 = NFCRC32::Sum(str);
 * 
 * // 数据完整性验证
 * const char* original = "重要数据";
 * uint32_t original_crc = NFCRC32::Sum(original, strlen(original));
 * 
 * // ... 数据传输 ...
 * 
 * const char* received = "重要数据";  // 接收到的数据
 * uint32_t received_crc = NFCRC32::Sum(received, strlen(received));
 * 
 * if (original_crc == received_crc) {
 *     std::cout << "数据完整" << std::endl;
 * } else {
 *     std::cout << "数据损坏" << std::endl;
 * }
 * @endcode
 * 
 * @note 这是一个纯静态类，不需要实例化
 * @note CRC32值不是密码学安全的，不能用于安全认证
 * @note 相同的数据总是产生相同的CRC32值
 */
class _NFExport NFCRC32
{
public:
	/**
	 * @brief 计算数据的CRC32校验值
	 * 
	 * 对指定的字节数据计算CRC32校验值。使用标准的IEEE 802.3 CRC32算法。
	 * 
	 * @param d 源数据缓冲区指针，不能为nullptr（当len>0时）
	 * @param len 数据长度（字节数），0表示空数据
	 * @return uint32_t CRC32校验值
	 * 
	 * @note 如果数据长度大于实际缓冲区大小，结果未定义
	 * @note 空数据（len=0）的CRC32值为0
	 * @note 算法使用小端字节序
	 */
	static uint32_t Sum(const void* d, size_t len);

	/**
	 * @brief 计算字符串的CRC32校验值
	 * 
	 * 对std::string对象计算CRC32校验值的便捷方法。
	 * 
	 * @param s 要计算校验值的字符串
	 * @return uint32_t CRC32校验值
	 * 
	 * @note 计算整个字符串内容的校验值，包括嵌入的'\0'字符
	 * @note 空字符串的CRC32值为0
	 */
	static uint32_t Sum(const string& s)
	{
		return Sum(s.data(), s.size());
	}

private:
	/**
	 * @brief 初始化CRC32查找表
	 * 
	 * 初始化256个元素的CRC32查找表，用于加速CRC计算。
	 * 查找表基于CRC32标准多项式生成。
	 * 
	 * @param table 指向256个uint32_t元素的数组
	 * 
	 * @note 这是内部方法，用户不应直接调用
	 * @note 查找表只需初始化一次，后续计算可重复使用
	 */
	static void InitTable(uint32_t* table);
	
	/**
	 * @brief 位反射函数
	 * 
	 * 对指定位数的数值进行位反射操作，用于CRC计算的预处理。
	 * 
	 * @param ref 要反射的数值
	 * @param ch 反射的位数
	 * @return uint32_t 反射后的数值
	 * 
	 * @note 这是内部方法，用于CRC算法的实现细节
	 * @note 位反射是CRC算法的标准操作之一
	 */
	static uint32_t Reflect(uint32_t ref, char ch);
};

