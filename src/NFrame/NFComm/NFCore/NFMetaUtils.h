// -------------------------------------------------------------------------
//    @FileName         :    NFMetaUtils.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFMetaUtils.h
 * @brief 模板元编程工具库
 * 
 * 此文件提供了模板元编程相关的工具函数和类型萃取器。
 * 包含函数类型萃取、tuple操作、函数特征分析等现代C++模板技术。
 * 主要用于反射、序列化、函数调用包装等高级功能的实现。
 */

#pragma once

#include "NFCPlusPlus14.h"
#include <functional>

/**
 * @brief 对tuple中的每个元素应用函数
 * 
 * 使用index_sequence展开tuple，对每个元素应用给定的函数。
 * 这是一个编译时展开的操作，性能很高。
 * 
 * @tparam Args tuple中元素的类型包
 * @tparam Func 要应用的函数类型
 * @tparam Idx 索引序列
 * @param t 要操作的tuple
 * @param f 要应用的函数
 * @param 索引序列（编译时生成）
 * 
 * @note 这是内部实现函数，通常不直接调用
 */
template <typename... Args, typename Func, std::size_t... Idx>
void for_each(const std::tuple<Args...>& t, Func&& f, std::index_sequence<Idx...>) {
	(void)std::initializer_list<int> { (f(std::get<Idx>(t)), void(), 0)...};
}

/**
 * @brief 对tuple中的每个元素应用带索引的函数
 * 
 * 与for_each类似，但同时传递元素的索引给函数。
 * 
 * @tparam Args tuple中元素的类型包
 * @tparam Func 要应用的函数类型
 * @tparam Idx 索引序列
 * @param t 要操作的tuple
 * @param f 要应用的函数（接受元素和索引两个参数）
 * @param 索引序列（编译时生成）
 * 
 * @note 函数f需要接受两个参数：元素值和std::integral_constant<size_t, N>类型的索引
 */
template <typename... Args, typename Func, std::size_t... Idx>
void for_each_i(const std::tuple<Args...>& t, Func&& f, std::index_sequence<Idx...>) {
	(void)std::initializer_list<int> { (f(std::get<Idx>(t), std::integral_constant<size_t, Idx>{}), void(), 0)...};
}

/**
 * @brief 函数特征萃取器基础模板
 * 
 * 用于提取函数类型的特征信息，包括参数类型、返回类型、参数个数等。
 * 这是所有函数特征萃取的基础模板。
 * 
 * @tparam T 要分析的函数类型
 */
template<typename T>
struct function_traits;

/**
 * @brief 函数特征萃取器（普通函数特化）
 * 
 * 针对普通函数类型的特化版本，提取函数的各种类型信息。
 * 
 * @tparam Ret 函数返回类型
 * @tparam Arg 第一个参数类型
 * @tparam Args 其余参数类型包
 */
template<typename Ret, typename Arg, typename... Args>
struct function_traits<Ret(Arg, Args...)>
{
public:
	/** @brief 函数参数个数 */
	enum { arity = sizeof...(Args) + 1 };
	/** @brief 函数类型定义 */
	typedef Ret function_type(Arg, Args...);
	/** @brief 返回值类型 */
	typedef Ret return_type;
	/** @brief STL function包装器类型 */
	using stl_function_type = std::function<function_type>;
	/** @brief 函数指针类型 */
	typedef Ret(*pointer)(Arg, Args...);

	/** @brief 参数类型的tuple */
	typedef std::tuple<Arg, Args...> tuple_type;
	/** @brief 去除const和引用的参数类型tuple */
	typedef std::tuple<std::remove_const_t<std::remove_reference_t<Arg>>, std::remove_const_t<std::remove_reference_t<Args>>...> bare_tuple_type;
	/** @brief 带字符串名称的参数类型tuple */
	using args_tuple = std::tuple<std::string, Arg, std::remove_const_t<std::remove_reference_t<Args>>...>;
	/** @brief 带字符串名称的参数类型tuple（从第二个参数开始） */
	using args_tuple_2nd = std::tuple<std::string, std::remove_const_t<std::remove_reference_t<Args>>...>;
};

/**
 * @brief 函数特征萃取器（无参数函数特化）
 * 
 * 针对无参数函数的特化版本。
 * 
 * @tparam Ret 函数返回类型
 */
template<typename Ret>
struct function_traits<Ret()> {
public:
	/** @brief 函数参数个数（0） */
	enum { arity = 0 };
	/** @brief 函数类型定义 */
	typedef Ret function_type();
	/** @brief 返回值类型 */
	typedef Ret return_type;
	/** @brief STL function包装器类型 */
	using stl_function_type = std::function<function_type>;
	/** @brief 函数指针类型 */
	typedef Ret(*pointer)();

	/** @brief 空的参数类型tuple */
	typedef std::tuple<> tuple_type;
	/** @brief 空的参数类型tuple */
	typedef std::tuple<> bare_tuple_type;
	/** @brief 仅包含字符串名称的tuple */
	using args_tuple = std::tuple<std::string>;
	/** @brief 仅包含字符串名称的tuple */
	using args_tuple_2nd = std::tuple<std::string>;
};

/**
 * @brief 函数特征萃取器（函数指针特化）
 * 
 * 针对函数指针类型的特化版本，继承普通函数的特征。
 * 
 * @tparam Ret 函数返回类型
 * @tparam Args 函数参数类型包
 */
template<typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

/**
 * @brief 函数特征萃取器（std::function特化）
 * 
 * 针对std::function包装器的特化版本。
 * 
 * @tparam Ret 函数返回类型
 * @tparam Args 函数参数类型包
 */
template <typename Ret, typename... Args>
struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)> {};

/**
 * @brief 函数特征萃取器（成员函数特化）
 * 
 * 针对类成员函数指针的特化版本。
 * 
 * @tparam ReturnType 成员函数返回类型
 * @tparam ClassType 成员函数所属的类类型
 * @tparam Args 成员函数参数类型包
 */
template <typename ReturnType, typename ClassType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...)> : function_traits<ReturnType(Args...)> {};

/**
 * @brief 函数特征萃取器（const成员函数特化）
 * 
 * 针对const类成员函数指针的特化版本。
 * 
 * @tparam ReturnType 成员函数返回类型
 * @tparam ClassType 成员函数所属的类类型
 * @tparam Args 成员函数参数类型包
 */
template <typename ReturnType, typename ClassType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> : function_traits<ReturnType(Args...)> {};

/**
 * @brief 函数特征萃取器（可调用对象特化）
 * 
 * 针对有operator()的可调用对象的特化版本。
 * 通过SFINAE技术自动检测operator()的签名。
 * 
 * @tparam Callable 可调用对象类型
 */
template<typename Callable>
struct function_traits : function_traits<decltype(&Callable::operator())> {};

/**
 * @brief 移除const引用类型别名
 * 
 * 便捷的类型别名，用于移除类型的const和引用修饰符。
 * 
 * @tparam T 要处理的类型
 */
template<typename T>
using remove_const_reference_t = std::remove_const_t<std::remove_reference_t<T>>;

/**
 * @brief 从索引序列创建tuple（内部实现）
 * 
 * 根据编译时索引序列创建对应的tuple。
 * 
 * @tparam Is 索引序列
 * @param 索引序列参数
 * @return 包含索引的tuple
 */
template<size_t... Is>
auto make_tuple_from_sequence(std::index_sequence<Is...>)->decltype(std::make_tuple(Is...)) {
	std::make_tuple(Is...);
}

/**
 * @brief 从索引序列创建tuple
 * 
 * 根据给定的数量N创建包含0到N-1索引的tuple。
 * 
 * @tparam N 索引数量
 * @return 包含索引序列的tuple
 */
template<size_t N>
constexpr auto make_tuple_from_sequence()->decltype(make_tuple_from_sequence(std::make_index_sequence<N>{})) {
	return make_tuple_from_sequence(std::make_index_sequence<N>{});
}

/**
 * @brief 内部实现命名空间
 * 
 * 包含tuple_switch的具体实现细节。
 */
namespace detail {
	/**
	 * @brief tuple运行时切换实现
	 * 
	 * 根据运行时索引调用相应的函数。
	 * 
	 * @tparam Tuple tuple类型
	 * @tparam F 函数类型
	 * @tparam Is 索引序列
	 * @param i 运行时索引
	 * @param t tuple对象
	 * @param f 要调用的函数
	 * @param 编译时索引序列
	 */
	template <class Tuple, class F, std::size_t...Is>
	void tuple_switch(const std::size_t i, Tuple&& t, F&& f, std::index_sequence<Is...>) {
		(void)std::initializer_list<int> {
			(i == Is && (
				(void)std::forward<F>(f)(std::integral_constant<size_t, Is>{}), 0))...
		};
	}
} // namespace detail

/**
 * @brief tuple运行时切换
 * 
 * 根据运行时索引值调用相应的函数。这是一个非常有用的工具，
 * 可以将运行时的整数值转换为编译时的类型信息。
 * 
 * @tparam Tuple tuple类型
 * @tparam F 函数类型
 * @param i 运行时索引值
 * @param t tuple对象
 * @param f 要调用的函数，接受std::integral_constant<size_t, N>参数
 * 
 * 使用示例：
 * @code
 * auto t = std::make_tuple(1, 2.0, "hello");
 * int index = 1;
 * tuple_switch(index, t, [](auto idx) {
 *     constexpr size_t I = decltype(idx)::value;
 *     std::cout << "Element at index " << I << std::endl;
 * });
 * @endcode
 */
template <class Tuple, class F>
inline void tuple_switch(const std::size_t i, Tuple&& t, F&& f) {
	constexpr auto N =
		std::tuple_size<std::remove_reference_t<Tuple>>::value;

	detail::tuple_switch(i, std::forward<Tuple>(t), std::forward<F>(f),
		std::make_index_sequence<N>{});
}

/**
 * @brief 获取第N个类型
 * 
 * 从参数包中获取第N个类型的类型别名。
 * 
 * @tparam N 索引（0开始）
 * @tparam Args 类型参数包
 */
template<int N, typename... Args>
using nth_type_of = std::tuple_element_t<N, std::tuple<Args...>>;

/**
 * @brief 获取最后一个类型
 * 
 * 从参数包中获取最后一个类型的类型别名。
 * 
 * @tparam Args 类型参数包
 */
template<typename... Args>
using last_type_of = nth_type_of<sizeof...(Args) - 1, Args...>;

