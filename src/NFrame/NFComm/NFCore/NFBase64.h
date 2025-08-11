// -------------------------------------------------------------------------
//    @FileName         :    NFBase64.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFBase64.h
 * @brief Base64编解码工具类
 * 
 * 此文件提供了标准的Base64编码和解码功能，支持字符串和二进制数据的转换。
 * Base64是一种用64个字符来表示任意二进制数据的方法，常用于网络传输和数据存储。
 */

#pragma once

#include <string>

#include "NFPlatform.h"

/////////////////////////////////////////////////
/**
* @file tc_base64.h
* @brief  base64编解码类.
*/

/////////////////////////////////////////////////

/**
 * @brief Base64编解码工具类
 * 
 * 该类提供标准的Base64编码和解码功能，支持以下特性：
 * - 字符串和二进制数据的编解码
 * - 可选的换行符支持（符合RFC标准）
 * - 高效的内存操作
 * 
 * Base64编码将3个8位字节转换为4个6位字符，使用64个可打印字符表示。
 * 当原始数据长度不是3的倍数时，会使用'='字符进行填充。
 * 
 * 使用方法：
 * @code
 * std::string data = "Hello World";
 * std::string encoded = NFBase64::Encode(data);
 * std::string decoded = NFBase64::Decode(encoded);
 * @endcode
 */
class _NFExport NFBase64
{
public:
	/**
	* @brief  对字符串进行base64编码.
	*
	* @param data         需要编码的数据
	* @param bChangeLine  是否需要在最终编码数据加入换行符 ，
	*                     (RFC中建议每76个字符后加入回车换行，默认为不添加换行
	* @return string      编码后的数据
	*/
	static std::string Encode(const std::string &data, bool bChangeLine = false);

	/**
	* @brief  对字符串进行base64解码.
	*
	* @param data     需要解码的数据
	* @return string  解码后的数据
	*/
	static std::string Decode(const std::string &data);

	/**
	* @brief  对字符串进行base64编码 .
	*
	* @param pSrc        需要编码的数据
	* @param nSrcLen     需要编码的数据长度
	* @param pDst        编码后的数据
	* @param bChangeLine 是否需要在最终编码数据加入换行符，
	*                    RFC中建议每76个字符后加入回车换行，默认为不添加换行
	* @return            编码后的字符串的长度
	*/
	static int Encode(const unsigned char* pSrc, int nSrcLen, char* pDst, bool bChangeLine = false);

	/**
	* @brief  对字符串进行base64解码.
	*
	* @param pSrc    需要解码的数据
	* @param nSrcLe  需要解码的数据长度
	* @param pDst   解码后的数据
	* @return       解码后的字符串的长度
	*/
	static int Decode(const char* pSrc, int nSrcLen, unsigned char* pDst);

protected:

	/**
	* @brief base64编码表
	* 包含64个用于编码的字符：A-Z, a-z, 0-9, +, /
	*/
	static const char EnBase64Tab[];
	
	/**
	* @brief base64解码表
	* 用于快速解码Base64字符的查找表
	*/
	static const char DeBase64Tab[];
};

