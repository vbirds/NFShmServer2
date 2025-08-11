// -------------------------------------------------------------------------
//    @FileName         :    NFHmacSha.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFHmacSha.h
//
// -------------------------------------------------------------------------

/**
 * @file NFHmacSha.h
 * @brief HMAC-SHA1算法实现
 * 
 * 此文件提供了HMAC-SHA1（基于哈希的消息认证码）算法的实现。
 * HMAC-SHA1是一种密码学安全的消息认证算法，广泛应用于数字签名、
 * 身份验证和数据完整性校验。
 */

#pragma once

#include <string>

/** @brief 32位无符号整数类型定义 */
typedef unsigned long    __u32;
/** @brief 8位无符号字符类型定义 */
typedef unsigned char    __u8;

/**
 * @brief SHA1上下文结构体
 * 
 * 用于存储SHA1算法的计算状态，支持增量更新。
 */
typedef struct
{
    /** @brief SHA1算法的5个32位状态寄存器 */
    __u32 state[5];
    /** @brief 消息长度计数器（低32位和高32位） */
    __u32 count[2];
    /** @brief 512位消息块缓冲区 */
    __u8  buffer[64];
} SHA1_CTX;

#if defined(rol)
#undef rol
#endif

/** @brief 启用SHA1算法的安全模式 */
#define SHA1HANDSOFF

/**
 * @brief 循环左移宏
 * 
 * 对32位值进行指定位数的循环左移操作。
 * 
 * @param value 要移位的32位值
 * @param bits 左移的位数（0-31）
 * @return 移位后的结果
 */
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/**
 * @name SHA1算法内部宏定义
 * @brief SHA1算法实现中使用的内部宏
 * @{
 */

/** @brief 初始扩展宏（小端序版本） */
#ifdef __LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
/** @brief 初始扩展宏（大端序版本） */
#define blk0(i) block->l[i]
#endif

/** @brief 消息扩展宏 */
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/** @brief SHA1算法第一阶段操作（轮次0-19） */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
/** @brief SHA1算法第一阶段操作（轮次16-19） */
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
/** @brief SHA1算法第二阶段操作（轮次20-39） */
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
/** @brief SHA1算法第三阶段操作（轮次40-59） */
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
/** @brief SHA1算法第四阶段操作（轮次60-79） */
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/** @} */

/**
 * @name SHA1算法核心函数
 * @brief SHA1算法的底层实现函数
 * @{
 */

/**
 * @brief SHA1变换函数
 * 
 * 处理单个512位消息块的SHA1变换，这是SHA1算法的核心。
 * 
 * @param state SHA1状态数组（5个32位值）
 * @param buffer 512位消息块缓冲区
 * 
 * @note 这是内部函数，通常不直接调用
 */
void SHA1Transform(__u32 state[5], __u8 buffer[64]);

/**
 * @brief 初始化SHA1上下文
 * 
 * 设置SHA1计算的初始状态，准备开始哈希计算。
 * 
 * @param context SHA1上下文指针，将被初始化
 * 
 * @note 每次开始新的SHA1计算前必须调用此函数
 */
void SHA1Init(SHA1_CTX *context);

/**
 * @brief 更新SHA1上下文
 * 
 * 向SHA1计算中添加新的数据块，支持增量更新。
 * 
 * @param context SHA1上下文指针
 * @param data 要添加的数据指针
 * @param len 数据长度（字节数）
 * 
 * @note 可以多次调用以处理大量数据
 * @note 数据会在内部缓冲，直到凑够512位块再处理
 */
void SHA1Update(SHA1_CTX *context, unsigned char *data, __u32 len);

/**
 * @brief 完成SHA1计算
 * 
 * 完成SHA1哈希计算，输出最终的160位哈希值。
 * 
 * @param digest [out] 输出的20字节SHA1哈希值
 * @param context SHA1上下文指针
 * 
 * @note 调用后context不能再使用，需要重新初始化
 * @note 输出的digest是20字节的二进制数据
 */
void SHA1Final(unsigned char digest[20], SHA1_CTX *context);

/** @} */

/**
 * @brief HMAC-SHA1算法实现函数
 * 
 * 计算基于SHA1的HMAC（Hash-based Message Authentication Code）。
 * HMAC算法提供了密码学安全的消息认证功能。
 * 
 * 主要特性：
 * - 密码学安全：基于SHA1的安全哈希算法
 * - 密钥认证：使用密钥确保消息的真实性
 * - 抗篡改：任何消息或密钥的变化都会导致HMAC值变化
 * - 标准算法：符合RFC 2104标准
 * 
 * 算法流程：
 * 1. 如果密钥长度大于64字节，先对密钥做SHA1哈希
 * 2. 如果密钥长度小于64字节，用零填充到64字节
 * 3. 计算内部填充：key XOR 0x36（重复64次）
 * 4. 计算外部填充：key XOR 0x5C（重复64次）
 * 5. 计算SHA1(外部填充 + SHA1(内部填充 + 消息))
 * 
 * @param k 密钥指针，用于认证的秘密密钥
 * @param lk 密钥长度（字节数）
 * @param d 要认证的数据指针
 * @param ld 数据长度（字节数）
 * @param out [out] 输出缓冲区，至少t字节
 * @param t 输出长度（通常为20，即完整SHA1长度）
 * 
 * @note 输出长度不应超过20字节（SHA1哈希长度）
 * @note 密钥应该保密，不同的应用应使用不同的密钥
 * @note HMAC值可以用于验证消息的完整性和真实性
 */
void hmac_sha
        (
                char* k,    /* 秘钥 secret key */
                int lk,     /*  秘钥长度 length of the key in bytes */
                char* d,    /* 数据 data */
                int ld,     /*  数据长度 length of data in bytes */
                char* out,  /* 输出的字符串 output buffer, at least "t" bytes */
                int t
        );

/**
 * @brief HMAC-SHA1算法封装类
 * 
 * NFHmacSha提供了HMAC-SHA1算法的高级封装，提供更便于使用的接口。
 * 主要用于生成基于密钥的消息认证码。
 * 
 * 主要特性：
 * - 简化接口：提供字符串输入输出的便捷方法
 * - 十六进制输出：直接返回十六进制字符串格式的HMAC值
 * - 静态方法：无需实例化，直接调用
 * - 标准实现：基于RFC 2104标准
 * 
 * 适用场景：
 * - API签名验证
 * - 消息完整性校验
 * - 身份认证令牌生成
 * - 安全通信协议
 * - 数字签名应用
 * 
 * 使用方法：
 * @code
 * // 生成HMAC-SHA1签名
 * std::string key = "my_secret_key";
 * std::string message = "Hello, World!";
 * std::string hmac = NFHmacSha::sha1str(key, message);
 * std::cout << "HMAC-SHA1: " << hmac << std::endl;
 * 
 * // API签名示例
 * std::string api_key = "user_api_key";
 * std::string request_data = "GET /api/users?id=123";
 * std::string signature = NFHmacSha::sha1str(api_key, request_data);
 * 
 * // 验证签名
 * std::string received_signature = "...";  // 从请求中获取
 * if (signature == received_signature) {
 *     std::cout << "签名验证成功" << std::endl;
 * } else {
 *     std::cout << "签名验证失败" << std::endl;
 * }
 * @endcode
 * 
 * @note HMAC-SHA1虽然仍然安全，但对于新应用建议使用HMAC-SHA256
 * @note 密钥的安全性直接影响HMAC的安全性
 */
class NFHmacSha
{
public:
    /**
     * @brief 计算HMAC-SHA1并返回十六进制字符串
     * 
     * 使用指定的密钥对数据计算HMAC-SHA1值，并返回十六进制字符串表示。
     * 
     * @param key 用于HMAC计算的密钥字符串
     * @param data 要认证的数据字符串
     * @return std::string 40字符的十六进制HMAC-SHA1值（小写）
     * 
     * @note 返回的字符串长度总是40个字符（20字节×2）
     * @note 返回的十六进制字符串使用小写字母
     * @note 空密钥或空数据也会产生有效的HMAC值
     */
    static std::string sha1str(const std::string& key, const std::string& data);
};

