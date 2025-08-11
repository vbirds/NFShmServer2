// -------------------------------------------------------------------------
//    @FileName         :    NFRandom.hpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFRandom.hpp
 * @brief 随机数生成工具类和函数定义
 * 
 * 此文件包含了各种随机数生成的工具类和函数，包括简单的线性同余生成器、
 * 标准库随机数生成器包装函数，以及各种便捷的随机数生成函数。
 */

#pragma once

#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <random>
#include <string>

#include "NFPlatform.h"

// Copy from leveldb project
// @see https://github.com/google/leveldb/blob/master/util/random.h
//
// A very simple random number generator.  Not especially good at
// generating truly random bits, but good enough for our needs in this
// package.

/**
 * @brief 简单的随机数生成器类
 * 
 * 基于线性同余法的简单随机数生成器，从LevelDB项目移植而来。
 * 虽然不是密码学安全的随机数生成器，但对于一般用途来说已经足够。
 * 
 * 使用线性同余公式: seed = (seed * A) % M
 * 其中 A = 16807, M = 2^31-1
 */
class _NFExport NFRandom
{
public:
	/**
	 * @brief 构造函数
	 * @param s 随机数种子，会被限制在有效范围内
	 */
	explicit NFRandom(uint32_t s) : seed_(s & 0x7fffffffu)
	{
	}

	/**
	 * @brief 生成下一个随机数
	 * @return uint32_t 生成的随机数
	 */
	uint32_t Next()
	{
		static const uint32_t M = 2147483647L; // 2^31-1
		static const uint64_t A = 16807; // bits 14, 8, 7, 5, 2, 1, 0

		// We are computing
		//       seed_ = (seed_ * A) % M,    where M = 2^31-1
		//
		// seed_ must not be zero or M, or else all subsequent computed values
		// will be zero or M respectively.  For all other values, seed_ will end
		// up cycling through every number in [1,M-1]
		uint64_t product = seed_ * A;

		// Compute (product % M) using the fact that ((x << 31) % M) == x.
		seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
		// The first reduction may overflow by 1 bit, so we may need to
		// repeat.  mod == M is not possible; using > allows the faster
		// sign-bit-based test.
		if (seed_ > M)
		{
			seed_ -= M;
		}

		return seed_;
	}

	/**
	 * @brief 生成均匀分布的随机数
	 * @param n 上限值（不包含）
	 * @return uint32_t 返回[0, n-1]范围内的随机数
	 * @note 要求 n > 0
	 */
	uint32_t Uniform(int n)
	{
		assert(n > 0);
		return Next() % n;
	}

	/**
	 * @brief 以1/n的概率返回true
	 * @param n 概率分母
	 * @return bool 大约以1/n的概率返回true，否则返回false
	 * @note 要求 n > 0
	 */
	bool Onein(int n)
	{
		assert(n > 0);
		return (Next() % n) == 0;
	}

	/**
	 * @brief 生成偏向较小数的随机数
	 * 
	 * 先从[0, max_log]范围内均匀选择一个base，然后返回base个随机比特。
	 * 效果是在[0, 2^max_log-1]范围内生成一个数，但偏向于较小的数。
	 * 
	 * @param max_log 最大的对数值
	 * @return uint32_t 生成的偏斜随机数
	 */
	uint32_t Skewed(int max_log)
	{
		return Uniform(1 << Uniform(max_log + 1));
	}

private:
	/** @brief 随机数种子 */
	uint32_t seed_;
};

/**
 * @brief 设置随机数种子
 * 使用当前时间初始化全局随机数种子
 */
_NFExport void NFRandomSeed();

/**
 * @brief 生成随机整数
 * @return int32_t 生成的随机整数
 */
_NFExport int32_t NFRandomAInt();

/**
 * @brief 生成随机无符号整数
 * @return uint32_t 生成的随机无符号整数
 */
_NFExport uint32_t NFRandomAUInt();

/**
 * @brief 生成指定范围内的随机整数
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return int32_t 生成的随机整数
 */
_NFExport int32_t NFRandomInt(int32_t min, int32_t max);

/**
 * @brief 生成指定范围内的随机无符号整数
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return uint32_t 生成的随机无符号整数
 */
_NFExport uint32_t NFRandomUInt(uint32_t min, uint32_t max);

/**
 * @brief 生成指定范围内的随机浮点数
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return float 生成的随机浮点数
 */
_NFExport float NFRandomFloat(float min, float max);

/**
 * @brief 生成指定范围内的随机双精度浮点数
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return double 生成的随机双精度浮点数
 */
_NFExport double NFRandomDouble(double min, double max);

/**
 * @brief 生成随机字符串
 * @param nChar 字符串长度
 * @return std::string 生成的随机字符串
 */
_NFExport std::string NFRandomString(size_t nChar);

/**
 * @brief 生成随机英文字符串
 * @param nChar 字符串长度
 * @return std::string 生成的随机英文字符串
 */
_NFExport std::string NFRandomEnglish(size_t nChar);;

/**
 * @brief 生成[min, max)范围内的随机整数（模板函数）
 * @tparam T 整数类型
 * @param min 最小值（包含）
 * @param max 最大值（不包含）
 * @return T 生成的随机数
 */
template <typename T>
T NFRandInt(T min, T max)
{
	static std::default_random_engine generator(static_cast<uint32_t>(time(nullptr)));
	std::uniform_int_distribution<T> distribution(min, max - 1);
	return distribution(generator);
}

/**
 * @brief 生成[min, max]范围内的随机实数（模板函数）
 * @tparam T 实数类型（float、double等）
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return T 生成的随机实数
 */
template <typename T>
T NFRandReal(T min, T max)
{
	static std::default_random_engine generator(time(nullptr));
	std::uniform_real_distribution<T> distribution(min, max);
	return distribution(generator);
}

/**
 * @brief 生成指定范围内的随机整数
 * @param lowerbound 下界（包含）
 * @param upperbound 上界（包含）
 * @return int 生成的随机整数
 * @note 要求参数为正数或0
 */
inline int RangeRand(int lowerbound, int upperbound)
{
	return lowerbound + (int)((upperbound - lowerbound + 1) * (rand() / (RAND_MAX + 1.0)));
}

/**
 * @brief 生成指定范围内的随机64位无符号整数
 * @param ullLowerBound 下界（包含）
 * @param ullUpperBound 上界（包含）
 * @return uint64_t 生成的随机64位无符号整数
 */
inline uint64_t RangeRand64(uint64_t ullLowerBound, uint64_t ullUpperBound)
{
	return ullLowerBound + (uint64_t)((ullUpperBound - ullLowerBound + 1) * (rand() / (RAND_MAX + 1.0)));
}

/**
 * @brief 判断是否满足万分之几的概率
 * @param iChance 万分之几的概率值
 * @return bool 如果随机数满足概率则返回true
 */
inline bool IsSatifyRangeRand10K(int iChance)
{
	return RangeRand(1, 10000) <= iChance;
}

/**
 * @brief 判断是否满足百万分之几的概率
 * @param iChance 百万分之几的概率值
 * @return bool 如果随机数满足概率则返回true
 */
inline bool IsSatifyRangeRand1M(int iChance)
{
	return RangeRand(1, 1000000) <= iChance;
}

/**
 * @brief 生成1-100范围内的随机数
 * @return int 1-100范围内的随机整数
 */
inline int RandRandHundred()
{
	return RangeRand(1, 100);
}

/**
 * @brief 生成1-10000范围内的随机数
 * @return int 1-10000范围内的随机整数
 */
inline int RangeRand10K()
{
	return RangeRand(1, 10000);
}

/**
 * @brief 生成1-1000000范围内的随机数
 * @return int 1-1000000范围内的随机整数
 */
inline int RandRand1M()
{
	return RangeRand(1, 1000000);
}
