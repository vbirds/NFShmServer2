// -------------------------------------------------------------------------
//    @FileName         :    NFMacros.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// ------------------------------------------------------------------------

/**
 * @file NFMacros.h
 * @brief 通用宏定义集合
 * 
 * 此文件定义了框架中常用的宏，包括禁用构造函数的宏、数组大小计算宏、
 * 分支预测优化宏等。这些宏提供了代码编写的便利性和性能优化支持。
 */

#pragma once

/**
 * @name 构造函数控制宏
 * @brief 用于禁用特定构造函数和赋值操作符的宏
 * @{
 */

/**
 * @brief 禁用拷贝构造函数和赋值操作符
 * 
 * 在类的private或public区域使用此宏，可以禁用类的拷贝构造函数
 * 和赋值操作符，防止对象被意外拷贝。
 * 
 * 使用方法：
 * @code
 * class MyClass {
 * public:
 *     MyClass() = default;
 * private:
 *     NF_DISALLOW_EVIL_CONSTRUCTORS(MyClass);
 * };
 * @endcode
 * 
 * @param TypeName 类名
 */
#define NF_DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
  TypeName(const TypeName&);                           \
  void operator=(const TypeName&)

/**
 * @brief 禁用所有隐式构造函数
 * 
 * 在类的private或public区域使用此宏，可以禁用类的默认构造函数、
 * 拷贝构造函数和赋值操作符。通常用于工具类或静态类。
 * 
 * 使用方法：
 * @code
 * class UtilityClass {
 * public:
 *     static void DoSomething();
 * private:
 *     NF_DISALLOW_IMPLICIT_CONSTRUCTORS(UtilityClass);
 * };
 * @endcode
 * 
 * @param TypeName 类名
 */
#define NF_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();                                           \
  TypeName(const TypeName&);                            \
  void operator=(const TypeName&)

/** @} */

/**
 * @name 数组和容器工具宏
 * @brief 用于数组和容器操作的工具宏
 * @{
 */

/**
 * @brief 计算静态数组的元素个数
 * 
 * 编译时计算静态数组的元素个数，比strlen等运行时函数更高效。
 * 只能用于静态数组，不能用于指针。
 * 
 * 使用方法：
 * @code
 * int arr[] = {1, 2, 3, 4, 5};
 * size_t count = NF_ARRAYSIZE(arr);  // count = 5
 * 
 * const char* names[] = {"Alice", "Bob", "Charlie"};
 * size_t nameCount = NF_ARRAYSIZE(names);  // nameCount = 3
 * @endcode
 * 
 * @param Array 静态数组名
 * @return 数组元素个数
 * 
 * @warning 不要对指针使用此宏，会得到错误的结果
 * @note 这是一个编译时常量表达式
 */
#define NF_ARRAYSIZE(Array) (sizeof(Array)/sizeof(Array[0]))

/** @} */

/**
 * @name 分支预测优化宏
 * @brief 用于优化分支预测的宏定义
 * 
 * 这些宏帮助编译器优化分支预测，提高CPU执行效率。
 * 通过告诉编译器某些条件的发生概率，编译器可以生成更优化的代码。
 * 
 * 原理：
 * - likely: 告诉编译器条件很可能为真，优化为快速路径
 * - unlikely: 告诉编译器条件很可能为假，优化分支布局
 * 
 * 使用场景：
 * - 错误检查（通常unlikely）
 * - 缓存命中检查（通常likely）
 * - 循环条件（根据实际情况）
 * 
 * 使用方法：
 * @code
 * // 错误处理，通常不会发生
 * if (unlikely(ptr == nullptr)) {
 *     HandleError();
 *     return;
 * }
 * 
 * // 正常情况，通常会发生
 * if (likely(cache.HasData())) {
 *     return cache.GetData();
 * }
 * 
 * // 循环条件，通常为真
 * while (likely(HasMoreWork())) {
 *     DoWork();
 * }
 * @endcode
 * 
 * @{
 */
#if defined(__GNUC__)
/**
 * @brief 标记条件很可能为真（GCC版本）
 * 
 * 告诉GCC编译器该条件在大多数情况下为真，编译器会据此优化代码布局。
 * 
 * @param x 要判断的条件表达式
 * @return 条件表达式的值，附带概率提示信息
 */
#define likely(x) __builtin_expect ((x), 1)

/**
 * @brief 标记条件很可能为假（GCC版本）
 * 
 * 告诉GCC编译器该条件在大多数情况下为假，编译器会据此优化代码布局。
 * 
 * @param x 要判断的条件表达式
 * @return 条件表达式的值，附带概率提示信息
 */
#define unlikely(x) __builtin_expect ((x), 0)
#else
/**
 * @brief 标记条件很可能为真（通用版本）
 * 
 * 对于不支持__builtin_expect的编译器，直接返回原表达式。
 * 
 * @param x 要判断的条件表达式
 * @return 条件表达式的值
 */
#define likely(x) (x)

/**
 * @brief 标记条件很可能为假（通用版本）
 * 
 * 对于不支持__builtin_expect的编译器，直接返回原表达式。
 * 
 * @param x 要判断的条件表达式
 * @return 条件表达式的值
 */
#define unlikely(x) (x)

/**
 * @brief __builtin_expect的通用版本占位符
 * 
 * 对于不支持__builtin_expect的编译器，定义为直接返回表达式。
 * 
 * @param expr 表达式
 * @param val 期望值（在通用版本中被忽略）
 * @return 表达式的值
 */
#define __builtin_expect(expr, val)   (expr)
#endif

/** @} */
