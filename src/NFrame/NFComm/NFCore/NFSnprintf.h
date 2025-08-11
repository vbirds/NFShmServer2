// -------------------------------------------------------------------------
//    @FileName         :    NFSnprintf.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFSnprintf.h
 * @brief 安全格式化输出函数封装
 * 
 * 此文件提供了对标准C snprintf API的安全封装，确保输出缓冲区的安全性，
 * 并统一了不同平台下snprintf函数的行为差异。主要用于替代不安全的sprintf函数。
 */

#pragma once

#include "NFPlatform.h"

#include <stdarg.h>

/**
 * @brief 安全的格式化字符串输出函数
 * 
 * 这是对标准C snprintf API的安全封装，可以保证结果缓冲区中的最后一个字节为'\0'，
 * 并统一了不同平台下的返回值行为。
 * 
 * 主要特性：
 * - 缓冲区安全：确保输出缓冲区以'\0'结尾，防止缓冲区溢出
 * - 跨平台兼容：统一了Windows和Unix/Linux平台的行为差异
 * - 返回值明确：提供一致的返回值语义
 * - 编译器优化：支持GCC的格式字符串检查
 * 
 * 返回值行为：
 * - 当缓冲区长度足够时：返回实际拷贝的数据长度（不包括'\0'）
 * - 当缓冲区长度不够时：
 *   * Unix/Linux平台：返回实际需要的缓冲区长度（>= size）
 *   * Windows平台：返回-1
 * 
 * 适用场景：
 * - 字符串格式化输出
 * - 安全的缓冲区操作
 * - 跨平台的字符串处理
 * - 替代不安全的sprintf函数
 * 
 * 使用方法：
 * @code
 * char buffer[256];
 * int name_id = 123;
 * const char* name = "用户";
 * 
 * // 安全的格式化输出
 * int len = NFSafeSnprintf(buffer, sizeof(buffer), 
 *                          "用户ID: %d, 姓名: %s", name_id, name);
 * 
 * if (len >= 0 && len < sizeof(buffer)) {
 *     std::cout << "格式化成功: " << buffer << std::endl;
 * } else {
 *     std::cout << "缓冲区空间不足" << std::endl;
 * }
 * @endcode
 * 
 * @param buf 存储结果的缓冲区，不能为nullptr
 * @param size buf缓冲区长度，必须大于0
 * @param fmt 变参格式字符串，遵循printf格式规范
 * @param ... 变参列表，与格式字符串对应
 * @return int 格式化结果的长度或错误码
 *         - 成功时：返回实际格式化的字符数（不包括'\0'）
 *         - 缓冲区不足时：Unix/Linux返回需要的长度，Windows返回-1
 * 
 * @note 输出缓冲区总是以'\0'结尾，即使发生截断
 * @note 当size为0时，不进行任何操作，返回-1
 * @note 格式字符串必须有效，否则行为未定义
 */
#if defined(__GNUC__) && (__GNUC__ > 4 ||(__GNUC__ == 4 && __GNUC_MINOR__ >= 4))
int NFSafeSnprintf(char *buf, size_t size,
	const char *fmt, ...) __attribute__((format(printf, 3, 4)));
#else
int NFSafeSnprintf(char* buf, size_t size, const char* fmt, ...);
#endif

/**
 * @brief 安全的格式化字符串输出函数（va_list版本）
 * 
 * 这是NFSafeSnprintf的va_list变参版本，用于在已有va_list参数的情况下
 * 进行安全的格式化输出。常用于封装其他变参函数。
 * 
 * 主要特性：
 * - 与NFSafeSnprintf相同的安全保证
 * - 使用va_list参数，适合函数封装
 * - 统一的跨平台行为
 * - 缓冲区溢出保护
 * 
 * 适用场景：
 * - 封装日志函数
 * - 变参函数的内部实现
 * - 需要传递va_list的场景
 * - 多层变参函数调用
 * 
 * 使用方法：
 * @code
 * void MyLogFunction(const char* fmt, ...) {
 *     char buffer[1024];
 *     va_list args;
 *     va_start(args, fmt);
 *     
 *     int len = NFSafeVsnprintf(buffer, sizeof(buffer), fmt, args);
 *     
 *     va_end(args);
 *     
 *     if (len >= 0) {
 *         // 处理格式化后的字符串
 *         std::cout << buffer << std::endl;
 *     }
 * }
 * @endcode
 * 
 * @param buf 存储结果的缓冲区，不能为nullptr
 * @param size buf缓冲区长度，必须大于0
 * @param fmt 变参格式字符串，遵循printf格式规范
 * @param ap 变参列表，通过va_start获得
 * @return int 格式化结果的长度或错误码
 *         - 成功时：返回实际格式化的字符数（不包括'\0'）
 *         - 缓冲区不足时：Unix/Linux返回需要的长度，Windows返回-1
 * 
 * @note 调用前需要使用va_start初始化ap，调用后需要使用va_end清理
 * @note 输出缓冲区总是以'\0'结尾，即使发生截断
 * @note 不会修改ap的状态，调用者负责管理va_list生命周期
 */
int NFSafeVsnprintf(char* buf, size_t size, const char* fmt, va_list ap);

