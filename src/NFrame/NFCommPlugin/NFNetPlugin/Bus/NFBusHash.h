// -------------------------------------------------------------------------
//    @FileName         :    NFBusHash.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFBusPlugin
//    @Desc             :    Bus哈希算法工具，提供32位和64位哈希计算功能。
//                          该文件提供了Bus通信系统中使用的哈希算法工具，包括32位和64位哈希计算、
//                          基于MurmurHash3的高性能哈希算法、模板化的哈希因子结构体。
//                          主要特性包括高性能的哈希计算、支持不同位数的哈希算法、
//                          线程安全的哈希计算、适用于数据完整性校验
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFHash.hpp"

/**
 * @struct HashFactor
 * @brief 哈希因子模板结构体
 * 
 * 该模板结构体提供了通用的哈希计算功能，支持32位和64位哈希算法
 * 使用MurmurHash3算法进行高性能哈希计算
 * 
 * @tparam IsHash64 是否使用64位哈希，true为64位，false为32位
 */
template <bool IsHash64>
struct HashFactor;

/**
 * @struct HashFactor<false>
 * @brief 32位哈希因子特化
 * 
 * 提供32位MurmurHash3哈希计算功能
 * 适用于大多数数据完整性校验场景
 * 
 * 性能特点：
 * - 计算速度快
 * - 内存占用小
 * - 碰撞率低
 */
template <>
struct HashFactor<false>
{
    /**
     * @brief 计算32位哈希值
     * 
     * 使用MurmurHash3_x86_32算法计算哈希值
     * 
     * @param seed 哈希种子，用于增加哈希的随机性
     * @param s 要哈希的数据指针
     * @param l 数据长度
     * @return 32位哈希值
     * 
     * @note 该函数使用MurmurHash3算法，性能优于CRC32算法
     * @see NFHash::murmur_hash3_x86_32
     */
	static uint32_t Hash(uint32_t seed, const void* s, size_t l)
	{
		return NFHash::murmur_hash3_x86_32(s, static_cast<int>(l), seed);
		// CRC算法性能太差，包长度也不太可能超过2GB
		// return atbus::detail::crc32(crc, static_cast<const unsigned char *>(s), l);
	}
};

/**
 * @struct HashFactor<true>
 * @brief 64位哈希因子特化
 * 
 * 提供64位MurmurHash3哈希计算功能
 * 适用于需要更高精度哈希的场景
 * 
 * 性能特点：
 * - 更高的哈希精度
 * - 更低的碰撞率
 * - 适用于大数据量场景
 */
template <>
struct HashFactor<true>
{
    /**
     * @brief 计算64位哈希值
     * 
     * 使用MurmurHash3_x86_32算法计算哈希值，返回64位结果
     * 
     * @param seed 哈希种子，用于增加哈希的随机性
     * @param s 要哈希的数据指针
     * @param l 数据长度
     * @return 64位哈希值
     * 
     * @note 该函数使用MurmurHash3算法，性能优于CRC64算法
     * @see NFHash::murmur_hash3_x86_32
     */
	static uint64_t Hash(uint64_t seed, const void* s, size_t l)
	{
		return NFHash::murmur_hash3_x86_32(s, static_cast<int>(l), static_cast<uint32_t>(seed));
		// CRC算法性能太差，包长度也不太可能超过2GB
		// return atbus::detail::crc64(crc, static_cast<const unsigned char *>(s), l);
	}
};
