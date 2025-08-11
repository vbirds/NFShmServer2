// -------------------------------------------------------------------------
//    @FileName         :    NFMD5.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFMD5.h
 * @brief MD5哈希算法实现
 * 
 * 此文件提供了MD5消息摘要算法的实现，MD5是一种广泛使用的密码散列函数，
 * 能够产生出一个128位（16字节）的散列值。本实现修改并完善自MD5的C版本。
 */

#pragma once

#include <string>
#include "NFPlatform.h"

/////////////////////////////////////////////////
/**
* @file tc_md5.h
* @brief md5类(修改并完善至md5的c版本)
*
*/
/////////////////////////////////////////////////

/**
 * @brief MD5散列算法工具类
 * 
 * 该类提供MD5的散列算法，通过静态函数提供。
 * 提供两种输出方式：字符串(32个字符的十六进制)或二进制(16个字节)
 * 
 * MD5算法特点：
 * - 输入任意长度的消息，输出128位（16字节）摘要
 * - 具有雪崩效应，输入的微小变化会导致输出的巨大变化
 * - 单向函数，从摘要推导出原始消息在计算上不可行
 * 
 * 使用方法：
 * @code
 * std::string text = "Hello World";
 * std::string hexHash = NFMD5::md5str(text);     // 32字符十六进制
 * std::string binHash = NFMD5::md5bin(text);     // 16字节二进制
 * std::string fileHash = NFMD5::md5file("test.txt"); // 文件MD5
 * @endcode
 */

/**
 * @brief 从字节数组中以小端序方式获取32位无符号长整数
 * @param n 输出的32位无符号长整数
 * @param b 字节数组
 * @param i 起始索引
 */
#ifndef GET_ULONG_LE
#define GET_ULONG_LE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ]       )        \
        | ( (unsigned long) (b)[(i) + 1] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 2] << 16 )        \
        | ( (unsigned long) (b)[(i) + 3] << 24 );       \
}
#endif

/**
 * @brief 将32位无符号长整数以小端序方式写入字节数组
 * @param n 要写入的32位无符号长整数
 * @param b 目标字节数组
 * @param i 起始索引
 */
#ifndef PUT_ULONG_LE
#define PUT_ULONG_LE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n)       );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 3] = (unsigned char) ( (n) >> 24 );       \
}
#endif

class _NFExport NFMD5
{
	/** @brief 指向unsigned char的指针类型 */
	typedef unsigned char *POINTER;
	/** @brief 16位无符号整数类型 */
	typedef unsigned short int UINT2;
	/** @brief 32位无符号整数类型 */
	typedef uint32_t UINT4;

	/**
	 * @brief MD5算法的上下文结构体
	 * 
	 * 该结构体包含了MD5算法运行过程中需要维护的状态信息：
	 * - state: 4个32位的状态变量（A、B、C、D）
	 * - count: 已处理的位数计数器（64位，低位在前）
	 * - buffer: 64字节的输入缓冲区
	 */
	typedef struct
	{
		/**
		* @brief MD5算法的状态变量 (ABCD)
		* 包含算法的4个32位状态寄存器
		*/
		UINT4 state[4];

		/**
		* @brief 已处理的位数计数器，模2^64（低位在前）
		* count[0]存储低32位，count[1]存储高32位
		*/
		UINT4 count[2];

		/**
		* @brief 64字节的输入缓冲区
		* 用于缓存不足64字节的输入数据
		*/
		unsigned char buffer[64];
	}MD5_CTX;

public:
	/**
	* @brief 对字符串进行md5处理,返回16字节二进制数据.
	*
	* @param sString  输入字符串
	* @return string 输出,16个字节的二进制数据
	*/
	static std::string md5bin(const std::string &sString);

	/**
	* @brief 对字符串进行md5处理 ，
	*        将md5的二进制数据转换成HEX的32个字节的字符串
	* @param sString  输入字符串
	* @return string 输出,32个字符的十六进制字符串
	*/
	static std::string md5str(const std::string &sString);

	/**
	* @brief 对文件进行md5处理.
	*
	* @param fileName 要处理的文件名
	* @return string  处理后的32字符十六进制字符串
	*/
	static std::string md5file(const std::string& fileName);

protected:

	static std::string bin2str(const void *buf, size_t len, const std::string &sSep);

	/**
	* @brief MD5 init.
	*
	* @param context 上下文信息
	* @return
	*/
	static void md5init(MD5_CTX *context);

	/**
	* @brief MD5 block update operation，Continues an MD5
	* message-digest operation, processing another message block,
	* and updating the context
	* @param context  上下文信息
	* @param input    输入
	* @param inputLen 输入长度
	* @return
	*/
	static void md5update(MD5_CTX *context, unsigned char *input, unsigned int inputLen);

	/**
	* @brief  MD5 finalization，Ends an MD5 message-digest
	* operation, writing the message digest and zeroizing the
	* context
	* @param digest   摘要
	* @param context 上下文信息
	*/
	static void md5final(unsigned char digest[16], MD5_CTX *context);

	/**
	* @brief  MD5 basic transformation，Transforms state based on
	*         block
	* @param state 状态
	* @param block : ...
	*/
	static void md5_process(MD5_CTX *ctx, const unsigned char data[64]);

	/**
	* @brief  Encodes input (UINT4) into output (unsigned
	*         char)，Assumes len is a multiple of 4
	* @param output 输出
	* @param input  输入
	* @param len    输入长度
	*/
	static void encode(unsigned char *output, UINT4 *input, unsigned int len);

	/**
	* @brief Decodes input (unsigned char) into output (UINT4)，
	* Assumes len is a multiple of 4
	* @param output 输出
	* @param input  输入
	* @param len    输入长度
	*/
	static void decode(UINT4 *output, unsigned char *input, unsigned int len);

	/**
	* @brief replace "for loop" with standard memcpy if possible.
	*
	* @param output  输出
	* @param input   输入
	* @param len     输入长度
	*/
	static void md5_memcpy(POINTER output, POINTER input, unsigned int len);

	/**
	* @brief replace "for loop" with standard memset if possible.
	*
	* @param output 输出
	* @param value  值
	* @param len    输入长度
	*/
	static void md5_memset(POINTER output, int value, unsigned int len);

	/**
	* 填充值
	*/
	static unsigned char PADDING[64];
};
