// -------------------------------------------------------------------------
//    @FileName         :    NFSHA1.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFSHA1.h
 * @brief SHA系列哈希算法实现
 * 
 * 此文件提供了SHA-1和SHA-256等安全哈希算法的实现。
 * SHA（Secure Hash Algorithm）是一系列密码散列函数，广泛用于
 * 数字签名、消息认证码、密钥派生等安全场景。
 * 
 * 基于Steve Reid的公共域SHA-1实现，100%公共域。
 */

#pragma once

#include <string>
#include <vector>
#include "NFException.hpp"


/////////////////////////////////////////////////
/**
* @file tc_sha.h
* @brief sha各种hash算法.
*
*/
/////////////////////////////////////////////////


/**
SHA-1   in   C
By   Steve   Reid   <steve@edmweb.com>
100%   Public   Domain
Test   Vectors   (from   FIPS   PUB   180-1)
"abc "
A9993E36   4706816A   BA3E2571   7850C26C   9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq "
84983E44   1C3BD26E   BAAE4AA1   F95129E5   E54670F1
A   million   repetitions   of   "a "
34AA973C   D4C4DAA4   F61EEB2B   DBAD2731   6534016F
*/

/**
 * @brief SHA算法异常类
 * 
 * 当SHA算法执行过程中发生错误时抛出此异常，
 * 通常是文件读取错误或内存分配失败等情况。
 */
struct NF_SHA_Exception : public NFException
{
	/**
	 * @brief 构造函数
	 * @param buffer 错误描述信息
	 * @param err 错误代码
	 */
	NF_SHA_Exception(const std::string &buffer, int err) : NFException(buffer, err) {};
	
	/**
	 * @brief 析构函数
	 */
	~NF_SHA_Exception() throw() {};
};

/**
 * @brief SHA系列哈希算法工具类
 * 
 * 该类提供了SHA-1和SHA-256等安全哈希算法的实现。
 * 支持对字符串、二进制数据和文件进行哈希计算。
 * 
 * 算法特点：
 * - SHA-1：产生160位（20字节）的哈希值，已不推荐用于安全场景
 * - SHA-256：产生256位（32字节）的哈希值，目前广泛使用的安全算法
 * 
 * 使用方法：
 * @code
 * std::string data = "Hello World";
 * std::string sha1Hash = NFSHA1::sha1str(data);      // SHA-1字符串哈希
 * std::string sha256Hash = NFSHA1::sha256str(data);  // SHA-256字符串哈希
 * std::string fileHash = NFSHA1::sha1file("test.txt"); // 文件SHA-1哈希
 * @endcode
 */
class _NFExport NFSHA1
{
public:
	/**
	 * @brief SHA-1哈希算法（二进制输出）
	 * 
	 * 对输入的二进制数据进行SHA-1哈希计算，返回20字节的二进制结果。
	 * SHA-1产生160位的哈希值，具有良好的雪崩效应。
	 * 
	 * @param buffer 输入数据缓冲区
	 * @param length 输入数据长度
	 * @return std::vector<char> 哈希结果（20个字节）
	 */
	static std::vector<char> sha1bin(const char *buffer, size_t length);

	/**
	 * @brief SHA-1哈希算法（十六进制字符串输出）
	 * 
	 * 对输入的二进制数据进行SHA-1哈希计算，返回40字符的十六进制字符串。
	 * 
	 * @param buffer 输入数据缓冲区
	 * @param length 输入数据长度
	 * @return std::string 十六进制哈希字符串（40个字符）
	 */
	static std::string sha1str(const char *buffer, size_t length);
	
	/**
	 * @brief SHA-1哈希算法（字符串版本）
	 * 
	 * 对输入字符串进行SHA-1哈希计算，返回40字符的十六进制字符串。
	 * 
	 * @param buffer 输入字符串
	 * @return std::string 十六进制哈希字符串（40个字符）
	 */
    static std::string sha1str(const std::string& buffer);

	/**
	 * @brief 对文件计算SHA-1哈希
	 * 
	 * 读取文件内容并计算其SHA-1哈希值，适用于文件完整性校验。
	 * 
	 * @param fileName 文件名路径
	 * @return std::string 十六进制哈希字符串（40个字符）
	 * @throws NF_SHA_Exception 文件读取错误时抛出异常
	 */
	static std::string sha1file(const std::string &fileName);

	/**
	 * @brief SHA-256哈希算法（二进制输出）
	 * 
	 * 对输入的二进制数据进行SHA-256哈希计算，返回32字节的二进制结果。
	 * SHA-256是目前推荐使用的安全哈希算法。
	 * 
	 * @param buffer 输入数据缓冲区
	 * @param length 输入数据长度
	 * @return std::vector<char> 哈希结果（32个字节）
	 */
	static std::vector<char> sha256bin(const char *buffer, size_t length);

	/**
	 * @brief SHA-256哈希算法（十六进制字符串输出）
	 * 
	 * 对输入的二进制数据进行SHA-256哈希计算，返回64字符的十六进制字符串。
	 * 
	 * @param buffer 输入数据缓冲区
	 * @param length 输入数据长度
	 * @return std::string 十六进制哈希字符串（64个字符）
	 */
	static std::string sha256str(const char *buffer, size_t length);

	/**
	* @brief 对文件计算sha256 hash算法.
	*
	* @param fileName  文件名
	* @throw TC_SHA_Exception, 文件读取错误会抛出异常
	* @return string   加密后的内容(32*2个字符)
	*/
	static std::string sha256file(const std::string &fileName);

	/**
	* @brief sha1 hash算法.
	*
	* @param sIn     输入buffer
	* @param iInLen  输入buffer长度
	* @return vector<char> hash的内容(48个字节)
	*/
	static std::vector<char> sha384bin(const char *buffer, size_t length);

	/**
	* @brief sha1 hash算法.
	*
	* @param sIn     输入buffer
	* @param iInLen  输入buffer长度
	* @return        string 加密后的内容(48*2个字符)
	*/
	static std::string sha384str(const char *buffer, size_t length);

	/**
	* @brief 对文件计算sha384 hash算法.
	*
	* @param fileName  文件名
	* @throw TC_SHA_Exception, 文件读取错误会抛出异常
	* @return string   加密后的内容(48*2个字符)
	*/
	static std::string sha384file(const std::string &fileName);

	/**
	* @brief sha1 hash算法.
	*
	* @param sIn     输入buffer
	* @param iInLen  输入buffer长度
	* @return vector<char> hash的内容(64个字节)
	*/
	static std::vector<char> sha512bin(const char *buffer, size_t length);

	/**
	* @brief sha1 hash算法.
	*
	* @param sIn     输入buffer
	* @param iInLen  输入buffer长度
	* @return        string 加密后的内容(64*2个字符)
	*/
	static std::string sha512str(const char *buffer, size_t length);

	/**
	* @brief 对文件计算sha512 hash算法.
	*
	* @param fileName  文件名
	* @throw TC_SHA_Exception, 文件读取错误会抛出异常
	* @return string   加密后的内容(64*2个字符)
	*/
	static std::string sha512file(const std::string &fileName);
};
