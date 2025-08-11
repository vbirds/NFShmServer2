// -------------------------------------------------------------------------
//    @FileName         :    NFLikely.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFLikely.h
 * @brief 分支预测优化宏定义
 * 
 * 此文件定义了用于分支预测优化的宏，帮助编译器生成更优化的代码。
 * 通过向编译器提供分支概率信息，可以优化CPU分支预测和指令缓存。
 * 
 * 这些宏在性能关键的代码路径中特别有用，可以减少分支预测失败的开销。
 */

#pragma once

/**
 * @name 编译器内建支持检测
 * @brief 检测编译器是否支持__builtin_expect
 * @{
 */
#if __GNUC__
/** @brief GCC编译器的内建期望函数，用于分支预测优化 */
#define NF_DETAIL_BUILTIN_EXPECT(b, t) (__builtin_expect(b, t))
#else
/** @brief 其他编译器的占位符定义，不进行优化 */
#define NF_DETAIL_BUILTIN_EXPECT(b, t) b
#endif
/** @} */

/**
 * @name 可能性注解宏
 * @brief 用于标记代码分支的可能性，帮助编译器优化
 * 
 * 当开发者比编译器更了解分支条件发生的概率时，这些宏非常有用。
 * 也适用于开发者比编译器更了解哪些代码路径是快速路径、哪些是慢速路径的情况，
 * 可以强制编译器为快速路径进行优化，即使它不是绝对可能的情况。
 * 
 * 使用场景：
 * - 错误处理分支（通常不太可能执行）
 * - 缓存命中/未命中的情况
 * - 网络数据包的正常/异常处理
 * - 内存分配成功/失败的情况
 * 
 * 使用方法：
 * @code
 * // 示例1：错误处理（不太可能发生）
 * if (NF_UNLIKELY(ptr == nullptr)) {
 *     // 错误处理代码
 *     HandleError();
 *     return;
 * }
 * 
 * // 示例2：正常情况（很可能发生）
 * if (NF_LIKELY(cache_hit)) {
 *     // 快速路径：从缓存获取数据
 *     return cached_data;
 * } else {
 *     // 慢速路径：重新计算或加载数据
 *     return ComputeData();
 * }
 * 
 * // 示例3：循环中的条件判断
 * while (NF_LIKELY(hasMoreData())) {
 *     processData();
 * }
 * @endcode
 * 
 * @{
 */

/**
 * @brief 标记条件很可能为真
 * 
 * 用于告诉编译器某个条件很可能为真，编译器会据此优化代码布局，
 * 将可能执行的代码路径放在更容易被CPU预取的位置。
 * 
 * @param x 要评估的条件表达式
 * @return 原始条件表达式的值，但带有可能性提示
 */
#define NF_LIKELY(x) NF_DETAIL_BUILTIN_EXPECT((x), 1)

/**
 * @brief 标记条件很可能为假
 * 
 * 用于告诉编译器某个条件很可能为假，编译器会据此优化代码布局，
 * 将不太可能执行的代码路径（如错误处理）放在次要位置。
 * 
 * @param x 要评估的条件表达式
 * @return 原始条件表达式的值，但带有可能性提示
 */
#define NF_UNLIKELY(x) NF_DETAIL_BUILTIN_EXPECT((x), 0)

/** @} */

/**
 * @name 非命名空间化的注解宏
 * @brief 提供不带NF前缀的全局宏定义
 * 
 * 这些宏提供了更简短的语法，但可能与其他库的宏定义冲突。
 * 使用前会先取消可能存在的定义以避免冲突。
 * 
 * @{
 */

#undef LIKELY
#undef UNLIKELY

#if defined(__GNUC__)
/**
 * @brief 全局LIKELY宏（GCC编译器）
 * 
 * 为GCC编译器提供的全局LIKELY宏，功能与NF_LIKELY相同
 * 
 * @param x 要评估的条件表达式
 * @return 带有可能性提示的条件表达式值
 */
#define LIKELY(x)   (__builtin_expect((x), 1))

/**
 * @brief 全局UNLIKELY宏（GCC编译器）
 * 
 * 为GCC编译器提供的全局UNLIKELY宏，功能与NF_UNLIKELY相同
 * 
 * @param x 要评估的条件表达式
 * @return 带有可能性提示的条件表达式值
 */
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
/**
 * @brief 全局LIKELY宏（其他编译器）
 * 
 * 对于不支持__builtin_expect的编译器，直接返回原表达式
 * 
 * @param x 要评估的条件表达式
 * @return 原始条件表达式的值
 */
#define LIKELY(x)   (x)

/**
 * @brief 全局UNLIKELY宏（其他编译器）
 * 
 * 对于不支持__builtin_expect的编译器，直接返回原表达式
 * 
 * @param x 要评估的条件表达式
 * @return 原始条件表达式的值
 */
#define UNLIKELY(x) (x)
#endif

/** @} */

