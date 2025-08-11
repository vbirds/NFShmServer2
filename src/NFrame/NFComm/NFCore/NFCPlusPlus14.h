// -------------------------------------------------------------------------
//    @FileName         :    NFCPlusPlus14.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFCPlusPlus14.h
 * @brief C++14特性兼容性支持
 * 
 * 此文件为C++11环境提供C++14标准库特性的兼容性实现。
 * 当编译器只支持C++11时，提供关键的C++14特性，如make_unique、
 * index_sequence、type traits别名等。这样可以在C++11环境中
 * 使用现代C++的便利特性。
 */

#pragma once

#include <type_traits>
#include <memory>
#include <tuple>

/**
 * @brief C++11环境下的C++14特性兼容性实现
 * 
 * 只有当编译器版本为C++11时，才会提供这些兼容性实现。
 * 对于C++14及以上版本，直接使用标准库的实现。
 */
#if __cplusplus == 201103L

namespace std {
	/**
	 * @brief unique_ptr类型选择器
	 * 
	 * 用于make_unique的类型推导，支持单个对象、未知边界数组和已知边界数组。
	 * 
	 * @tparam T 要创建的对象类型
	 */
	template<class T>
	struct unique_if {
		/** @brief 单个对象的unique_ptr类型 */
		typedef unique_ptr<T> single_object;
	};

	/**
	 * @brief unique_ptr类型选择器（未知边界数组特化）
	 * 
	 * 针对T[]类型的特化版本。
	 * 
	 * @tparam T 数组元素类型
	 */
	template<class T>
	struct unique_if<T[]> {
		/** @brief 未知边界数组的unique_ptr类型 */
		typedef unique_ptr<T[]> unknown_bound;
	};

	/**
	 * @brief unique_ptr类型选择器（已知边界数组特化）
	 * 
	 * 针对T[N]类型的特化版本，禁止创建已知大小的数组。
	 * 
	 * @tparam T 数组元素类型
	 * @tparam N 数组大小
	 */
	template<class T, size_t N>
	struct unique_if<T[N]> {
		/** @brief 已知边界数组类型，设为void以禁止使用 */
		typedef void known_bound;
	};

	/**
	 * @brief 创建unique_ptr（单个对象版本）
	 * 
	 * 创建指向单个对象的unique_ptr，类似于C++14的std::make_unique。
	 * 
	 * @tparam T 要创建的对象类型
	 * @tparam Args 构造函数参数类型包
	 * @param args 传递给构造函数的参数
	 * @return typename unique_if<T>::single_object 指向新创建对象的unique_ptr
	 * 
	 * 使用示例：
	 * @code
	 * auto ptr = std::make_unique<MyClass>(arg1, arg2);
	 * @endcode
	 */
	template<class T, class... Args>
	typename unique_if<T>::single_object make_unique(Args&&... args) {
		return unique_ptr<T>(new T(forward<Args>(args)...));
	}

	/**
	 * @brief 创建unique_ptr（未知边界数组版本）
	 * 
	 * 创建指向动态数组的unique_ptr。
	 * 
	 * @tparam T 数组类型（如int[]）
	 * @param n 数组大小
	 * @return typename unique_if<T>::unknown_bound 指向新创建数组的unique_ptr
	 * 
	 * 使用示例：
	 * @code
	 * auto arr = std::make_unique<int[]>(10);
	 * @endcode
	 */
	template<class T>
	typename unique_if<T>::unknown_bound make_unique(size_t n) {
		typedef typename remove_extent<T>::type U;
		return unique_ptr<T>(new U[n]());
	}

	/**
	 * @brief 禁止创建已知边界数组的unique_ptr
	 * 
	 * 这个版本被删除，防止创建如int[10]这样的固定大小数组。
	 * 
	 * @tparam T 数组类型
	 * @tparam Args 参数类型包
	 */
	template<class T, class... Args>
	typename unique_if<T>::known_bound make_unique(Args&&...) = delete;

	/**
	 * @brief 编译时整数序列
	 * 
	 * 表示一个编译时整数序列，用于模板元编程。
	 * 这是C++14 std::index_sequence的兼容实现。
	 * 
	 * @tparam Ints 整数序列
	 */
	template<size_t... Ints>
	struct index_sequence {
		/** @brief 类型别名 */
		using type = index_sequence;
		/** @brief 值类型 */
		using value_type = size_t;
		/** @brief 获取序列长度 */
		static constexpr std::size_t size() noexcept { return sizeof...(Ints); }
	};

	// --------------------------------------------------------------

	/**
	 * @brief 合并并重编号两个索引序列
	 * 
	 * 内部辅助结构，用于合并两个index_sequence。
	 * 
	 * @tparam Sequence1 第一个序列类型
	 * @tparam Sequence2 第二个序列类型
	 */
	template<class Sequence1, class Sequence2>
	struct _merge_and_renumber;

	/**
	 * @brief 合并并重编号的特化版本
	 * 
	 * 将两个索引序列合并，第二个序列的索引会加上第一个序列的长度。
	 * 
	 * @tparam I1 第一个序列的索引包
	 * @tparam I2 第二个序列的索引包
	 */
	template<size_t... I1, size_t... I2>
	struct _merge_and_renumber<index_sequence<I1...>, index_sequence<I2...>>
		: index_sequence<I1..., (sizeof...(I1) + I2)...> {};

	// --------------------------------------------------------------

	/**
	 * @brief 创建0到N-1的索引序列
	 * 
	 * 递归地创建从0到N-1的索引序列，这是C++14 std::make_index_sequence的兼容实现。
	 * 
	 * @tparam N 序列长度
	 */
	template<size_t N>
	struct make_index_sequence : _merge_and_renumber<typename make_index_sequence<N / 2>::type,
		typename make_index_sequence<N - N / 2>::type> {};

	/**
	 * @brief 递归终止条件（N=0）
	 */
	template<>
	struct make_index_sequence<0> : index_sequence<> {};
	
	/**
	 * @brief 递归终止条件（N=1）
	 */
	template<>
	struct make_index_sequence<1> : index_sequence<0> {};

	/**
	 * @brief 为类型包创建索引序列
	 * 
	 * 根据类型包的大小创建对应的索引序列。
	 * 
	 * @tparam T 类型参数包
	 */
	template<typename... T>
	using index_sequence_for = make_index_sequence<sizeof...(T)>;

	/**
	 * @brief enable_if的类型别名
	 * 
	 * C++14风格的类型别名，简化enable_if的使用。
	 * 
	 * @tparam B 布尔条件
	 * @tparam T 类型
	 */
	template<bool B, class T = void>
	using enable_if_t = typename enable_if<B, T>::type;

	/**
	 * @brief remove_const的类型别名
	 * 
	 * @tparam T 要移除const的类型
	 */
	template<typename T>
	using remove_const_t = typename remove_const<T>::type;

	/**
	 * @brief remove_reference的类型别名
	 * 
	 * @tparam T 要移除引用的类型
	 */
	template<typename T>
	using remove_reference_t = typename remove_reference<T>::type;

	/**
	 * @brief tuple_element的类型别名
	 * 
	 * @tparam I 索引
	 * @tparam T tuple类型
	 */
	template<int I, typename T>
	using tuple_element_t = typename tuple_element<I, T>::type;

	/**
	 * @brief decay的类型别名
	 * 
	 * @tparam T 要decay的类型
	 */
	template<typename T>
	using decay_t = typename decay<T>::type;

	/**
	 * @brief apply辅助函数
	 * 
	 * 将tuple中的元素作为参数展开调用函数。
	 * 
	 * @tparam F 函数类型
	 * @tparam Tuple tuple类型
	 * @tparam Idx 索引序列
	 * @param f 要调用的函数
	 * @param tp tuple参数
	 * @param 索引序列（编译时生成）
	 * @return 函数调用的返回值
	 */
	template<typename F, typename Tuple, size_t... Idx>
	auto apply_helper(F&& f, Tuple&& tp, std::index_sequence<Idx...>)
		-> decltype(std::forward<F>(f)(std::get<Idx>(std::forward<Tuple>(tp))...)) {
		return std::forward<F>(f)(std::get<Idx>(std::forward<Tuple>(tp))...);
	}

	/**
	 * @brief 将tuple元素作为参数应用到函数
	 * 
	 * C++14 std::apply的兼容实现，将tuple中的元素展开作为函数参数。
	 * 
	 * @tparam F 函数类型
	 * @tparam Tuple tuple类型
	 * @param f 要调用的函数
	 * @param tp 包含参数的tuple
	 * @return 函数调用的返回值
	 * 
	 * 使用示例：
	 * @code
	 * auto args = std::make_tuple(1, 2.0, "hello");
	 * auto result = std::apply(myFunction, args);
	 * @endcode
	 */
	template<typename F, typename Tuple>
	auto apply(F&& f, Tuple&& tp)
		-> decltype(apply_helper(std::forward<F>(f), std::forward<Tuple>(tp),
			std::make_index_sequence<std::tuple_size<decay_t<Tuple>>::value>{})) {
		return apply_helper(std::forward<F>(f), std::forward<Tuple>(tp),
			std::make_index_sequence<std::tuple_size<decay_t<Tuple>>::value>{});
	}

	/**
	 * @brief 调用函数
	 * 
	 * C++17 std::invoke的简化版本，直接调用函数。
	 * 
	 * @tparam F 函数类型
	 * @tparam Args 参数类型包
	 * @param f 要调用的函数
	 * @param args 函数参数
	 * @return 函数调用的返回值
	 */
	template<typename F, typename... Args>
	auto invoke(F&& f, Args&&... args) -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) {
		return std::forward<F>(f)(std::forward<Args>(args)...);
	}

}  // namespace std

#endif

