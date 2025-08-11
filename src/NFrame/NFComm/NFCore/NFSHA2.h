// -------------------------------------------------------------------------
//    @FileName         :    NFSHA2.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFSHA2.h
 * @brief SHA1算法实现类（SHA-2系列的SHA1实现）
 * 
 * 此文件提供了SHA1算法的完整实现，基于Dominik Reichl的公共域实现。
 * 支持增量更新、文件哈希计算等功能。这是一个经过优化的SHA1实现，
 * 支持不同的字节序和安全选项配置。
 */

/*
100% free public domain implementation of the SHA-1 algorithm
by Dominik Reichl <dominik.reichl@t-online.de>
Web: http://www.dominik-reichl.de/

Version 1.6 - 2005-02-07 (thanks to Howard Kapustein for patches)
- You can set the endianness in your files, no need to modify the
  header file of the CSHA1 class any more
- Aligned data support
- Made support/compilation of the utility functions (ReportHash
  and HashFile) optional (useful, if bytes count, for example in
  embedded environments)

Version 1.5 - 2005-01-01
- 64-bit compiler compatibility added
- Made variable wiping optional (define SHA1_WIPE_VARIABLES)
- Removed unnecessary variable initializations
- ROL32 improvement for the Microsoft compiler (using _rotl)

======== Test Vectors (from FIPS PUB 180-1) ========

NFPebbleSha1("abc") =
A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D

NFPebbleSha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") =
84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1

NFPebbleSha1(A million repetitions of "a") =
34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#ifndef ___SHA1_HDR___
#define ___SHA1_HDR___

#include "NFPlatform.h"

#if !defined(SHA1_UTILITY_FUNCTIONS) && !defined(SHA1_NO_UTILITY_FUNCTIONS)
#define SHA1_UTILITY_FUNCTIONS
#endif

#include <memory.h> // Needed for memset and memcpy
#include <string>

#ifdef SHA1_UTILITY_FUNCTIONS
#include <stdio.h>  // Needed for file access and sprintf
#include <string.h> // Needed for strcat and strcpy
#endif

#ifdef _MSC_VER
#include <stdlib.h>
#endif

// You can define the endian mode in your files, without modifying the NFPebbleSha1
// source files. Just #define SHA1_LITTLE_ENDIAN or #define SHA1_BIG_ENDIAN
// in your files, before including the NFPebbleSha1.h header file. If you don't
// define anything, the class defaults to little endian.

#if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
#define SHA1_LITTLE_ENDIAN
#endif

// Same here. If you want variable wiping, #define SHA1_WIPE_VARIABLES, if
// not, #define SHA1_NO_WIPE_VARIABLES. If you don't define anything, it
// defaults to wiping.

#if !defined(SHA1_WIPE_VARIABLES) && !defined(SHA1_NO_WIPE_VARIABLES)
#define SHA1_WIPE_VARIABLES
#endif

/////////////////////////////////////////////////////////////////////////////
// Define 8- and 32-bit variables

#ifndef UINT_32

#ifdef _MSC_VER

#define UINT_8  unsigned __int8
#define UINT_32 unsigned __int32

#else

#define UINT_8 unsigned char

#if (ULONG_MAX == 0xFFFFFFFF)
#define UINT_32 unsigned long
#else
#define UINT_32 unsigned int
#endif

#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Declare NFPebbleSha1 workspace

/**
 * @brief SHA1工作区块联合体
 * 
 * 用于SHA1算法计算过程中的数据对齐和访问优化。
 * 可以同时按字节和32位字方式访问同一块内存。
 */
typedef union
{
	/** @brief 按字节访问的64字节数组 */
	UINT_8  c[64];
	/** @brief 按32位字访问的16个元素数组 */
	UINT_32 l[16];
} SHA1_WORKSPACE_BLOCK;

/**
 * @brief SHA1算法实现类
 * 
 * NFSHA2提供了完整的SHA1算法实现，支持增量更新和多种输出格式。
 * 这是一个经过优化的实现，支持大文件处理和内存敏感的环境。
 * 
 * 主要特性：
 * - 标准实现：符合FIPS PUB 180-1标准
 * - 增量更新：支持分块处理大数据
 * - 文件哈希：直接计算文件的SHA1值
 * - 字节序支持：可配置大端或小端字节序
 * - 安全选项：可选的变量清除功能
 * - 64位兼容：支持64位编译器
 * 
 * 算法特点：
 * - 输出长度：160位（20字节）
 * - 块大小：512位（64字节）
 * - 轮次数：80轮
 * - 安全性：已知存在碰撞攻击，不建议用于安全应用
 * 
 * 适用场景：
 * - 文件完整性校验
 * - 数据指纹计算
 * - 兼容性要求SHA1的系统
 * - 非安全敏感的哈希计算
 * - 大文件的哈希处理
 * 
 * 使用方法：
 * @code
 * // 字符串哈希
 * std::string data = "Hello World";
 * std::string hash = NFSHA2::encode(data);
 * std::cout << "SHA1: " << hash << std::endl;
 * 
 * // 增量更新
 * NFSHA2 sha1;
 * sha1.Reset();
 * 
 * std::string part1 = "Hello ";
 * std::string part2 = "World";
 * sha1.Update((UINT_8*)part1.c_str(), part1.length());
 * sha1.Update((UINT_8*)part2.c_str(), part2.length());
 * 
 * sha1.Final();
 * 
 * // 获取结果
 * UINT_8 digest[20];
 * sha1.GetHash(digest);
 * 
 * // 文件哈希
 * NFSHA2 file_sha1;
 * if (file_sha1.HashFile("test.txt")) {
 *     char report[256];
 *     file_sha1.ReportHash(report, NFSHA2::REPORT_HEX);
 *     std::cout << "文件SHA1: " << report << std::endl;
 * }
 * @endcode
 * 
 * @note 此实现基于Dominik Reichl的公共域代码
 * @note SHA1已不推荐用于安全应用，新项目建议使用SHA-256
 * @note 支持通过宏定义配置字节序和安全选项
 */
class _NFExport NFSHA2
{
public:
#ifdef SHA1_UTILITY_FUNCTIONS
	/**
	 * @brief 哈希报告格式枚举
	 * 
	 * 定义ReportHash方法的输出格式选项。
	 */
	enum
	{
		/** @brief 十六进制格式输出 */
		REPORT_HEX = 0,
		/** @brief 数字格式输出 */
		REPORT_DIGIT = 1
	};
#endif

	/**
	 * @brief 静态编码方法
	 * 
	 * 对字符串计算SHA1哈希值并返回十六进制字符串。
	 * 这是一个便捷的静态方法，适合简单的哈希计算。
	 * 
	 * @param str 要计算哈希的字符串
	 * @return std::string 40字符的十六进制SHA1值
	 * 
	 * @note 返回的字符串长度总是40个字符
	 * @note 内部会创建临时对象进行计算
	 */
	static std::string encode(const std::string& str);

	/**
	 * @brief 构造函数
	 * 
	 * 创建SHA1计算器并初始化为默认状态。
	 */
	NFSHA2();
	
	/**
	 * @brief 析构函数
	 * 
	 * 清理资源，根据配置可能会清除敏感数据。
	 */
	~NFSHA2();

	/** @brief SHA1算法的5个32位状态寄存器 */
	UINT_32 m_state[5];
	/** @brief 消息长度计数器（低32位和高32位） */
	UINT_32 m_count[2];
	/** @brief 保留字段1 */
	UINT_32 __reserved1[1];
	/** @brief 64字节消息缓冲区 */
	UINT_8  m_buffer[64];
	/** @brief 20字节SHA1摘要输出 */
	UINT_8  m_digest[20];
	/** @brief 保留字段2 */
	UINT_32 __reserved2[3];

	/**
	 * @brief 重置SHA1状态
	 * 
	 * 将SHA1计算器重置为初始状态，准备进行新的哈希计算。
	 * 
	 * @note 调用此方法后可以开始新的哈希计算
	 * @note 会清除之前的所有计算状态
	 */
	void Reset();

	/**
	 * @brief 更新SHA1计算
	 * 
	 * 向SHA1计算中添加新的数据块。支持多次调用以处理大量数据。
	 * 
	 * @param data 要添加的数据指针
	 * @param len 数据长度（字节数）
	 * 
	 * @note 可以多次调用以增量更新
	 * @note 数据会在内部缓冲，直到凑够64字节块再处理
	 * @note 支持任意长度的数据输入
	 */
	void Update(UINT_8 *data, UINT_32 len);
	
#ifdef SHA1_UTILITY_FUNCTIONS
	/**
	 * @brief 计算文件的SHA1哈希值
	 * 
	 * 直接读取文件内容并计算其SHA1哈希值。
	 * 适合处理大文件，内部会分块读取避免内存占用过大。
	 * 
	 * @param szFileName 文件路径，支持相对路径和绝对路径
	 * @return bool 计算成功返回true，文件不存在或读取失败返回false
	 * 
	 * @note 计算完成后需要调用Final()方法
	 * @note 大文件会分块处理，内存占用较少
	 * @note 文件路径应该是有效的可读文件
	 */
	bool HashFile(char *szFileName);
#endif

	/**
	 * @brief 完成SHA1计算
	 * 
	 * 完成SHA1哈希计算，处理最后的填充和长度信息。
	 * 调用后m_digest数组将包含最终的20字节SHA1值。
	 * 
	 * @note 必须在所有Update()调用完成后调用
	 * @note 调用后不能再次Update()，需要Reset()重新开始
	 * @note 结果存储在m_digest成员变量中
	 */
	void Final();

#ifdef SHA1_UTILITY_FUNCTIONS
	/**
	 * @brief 生成格式化的哈希报告
	 * 
	 * 将计算完成的SHA1值格式化为可读字符串。
	 * 
	 * @param szReport [out] 输出缓冲区，必须足够大
	 * @param uReportType 报告格式，REPORT_HEX或REPORT_DIGIT
	 * 
	 * @note 必须在Final()调用后使用
	 * @note REPORT_HEX格式输出40字符的十六进制字符串
	 * @note 输出缓冲区应至少分配41字节（包括终止符）
	 */
	void ReportHash(char *szReport, unsigned char uReportType = REPORT_HEX);
#endif

	/**
	 * @brief 获取原始哈希值
	 * 
	 * 将计算完成的20字节SHA1值拷贝到指定缓冲区。
	 * 
	 * @param puDest [out] 目标缓冲区，必须至少20字节
	 * 
	 * @note 必须在Final()调用后使用
	 * @note 输出为20字节的原始二进制数据
	 * @note 调用者负责确保目标缓冲区有足够空间
	 */
	void GetHash(UINT_8 *puDest);

private:
	/**
	 * @brief SHA1核心变换函数
	 * 
	 * 执行SHA1算法的核心变换操作，处理单个64字节块。
	 * 
	 * @param state SHA1状态数组（5个32位值）
	 * @param buffer 64字节数据块
	 * 
	 * @note 这是内部方法，实现SHA1算法的核心逻辑
	 * @note 每个64字节块都会调用一次此方法
	 */
	void Transform(UINT_32 *state, UINT_8 *buffer);

	/** @brief 64字节工作空间，用于内部计算 */
	UINT_8 m_workspace[64];
	/** @brief 指向工作空间的结构化指针，便于按不同方式访问 */
	SHA1_WORKSPACE_BLOCK *m_block;
};

#endif
