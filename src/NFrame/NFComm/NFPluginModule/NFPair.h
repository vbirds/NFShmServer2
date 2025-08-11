// -------------------------------------------------------------------------
//    @FileName         :    NFPair.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//    @Description      :    轻量级键值对模板类，类似std::pair的实现
//
// -------------------------------------------------------------------------

/**
 * @file NFPair.h
 * @brief 轻量级键值对模板类
 * @details 该文件实现了一个类似于std::pair的简化版本，主要特性包括：
 * 
 * **设计目标：**
 * - **轻量级**：简化的实现，减少编译依赖和代码体积
 * - **高性能**：最小化运行时开销，内联友好
 * - **兼容性**：与std::pair类似的接口设计
 * - **可控制**：避免STL的复杂性，便于调试和优化
 * 
 * **技术特点：**
 * - 模板类实现，支持任意类型组合
 * - 值语义设计，支持拷贝和赋值
 * - 简洁的接口，易于理解和使用
 * - 编译期优化友好，内联展开
 * 
 * **适用场景：**
 * - 需要简单键值对存储的场合
 * - 函数返回多个值的情况
 * - 临时数据组合和传递
 * - 避免STL依赖的轻量级项目
 * 
 * **使用示例：**
 * @code
 * // 创建整数-字符串对
 * NFPair<int, std::string> pair1(42, "answer");
 * 
 * // 访问成员
 * int key = pair1.first;
 * std::string value = pair1.second;
 * 
 * // 拷贝构造
 * NFPair<int, std::string> pair2 = pair1;
 * @endcode
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include <stddef.h>
#include <string.h>

/**
 * @class NFPair
 * @brief 轻量级键值对模板类
 * @tparam _T1 第一个元素的类型
 * @tparam _T2 第二个元素的类型
 * @details 提供类似std::pair的功能，但更加轻量级和可控。
 * 
 * **核心特性：**
 * - **类型安全**：模板参数确保编译期类型检查
 * - **值语义**：支持深拷贝和赋值操作
 * - **简洁接口**：提供必要的构造、拷贝、赋值功能
 * - **内存效率**：直接存储值，无额外间接开销
 * - **性能优化**：内联友好，编译器易于优化
 * 
 * **设计考虑：**
 * - 简化实现，避免STL的复杂性
 * - 减少模板实例化开销
 * - 提供清晰的所有权语义
 * - 支持POD和复杂类型
 * 
 * **内存布局：**
 * 两个成员按声明顺序连续存储，遵循C++对象布局规则。
 * 
 * **线程安全：**
 * @warning 该类不是线程安全的，并发访问需要外部同步
 * 
 * **使用模式：**
 * @code
 * // 基本使用
 * NFPair<int, float> data(10, 3.14f);
 * 
 * // 作为函数返回值
 * NFPair<bool, std::string> ParseResult(const std::string& input) {
 *     // ... 解析逻辑
 *     return NFPair<bool, std::string>(success, errorMsg);
 * }
 * 
 * // 容器中使用
 * std::vector<NFPair<int, std::string>> items;
 * items.push_back(NFPair<int, std::string>(1, "first"));
 * @endcode
 */
template<class _T1, class _T2>
class NFPair
{
public:
    /**
     * @brief 默认构造函数
     * @param _first 第一个元素的初始值，默认使用类型的默认构造函数
     * @param _second 第二个元素的初始值，默认使用类型的默认构造函数
     * 
     * **功能说明：**
     * 使用给定参数或默认值初始化键值对的两个成员。
     * 
     * **初始化过程：**
     * - 使用参数值直接初始化成员变量
     * - 如果不提供参数，则使用类型的默认构造函数
     * - 支持隐式类型转换（如果类型支持）
     * 
     * **性能特点：**
     * - 直接初始化，无额外拷贝开销
     * - 编译器内联优化友好
     * - 适合大部分使用场景
     * 
     * **使用示例：**
     * @code
     * NFPair<int, std::string> p1;                    // 默认构造
     * NFPair<int, std::string> p2(42, "hello");      // 带参数构造
     * NFPair<double, int> p3(3.14);                  // 部分参数构造
     * @endcode
     */
	NFPair(_T1 _first = _T1(), _T2 _second = _T2()):first(_first), second(_second)
	{

	}
    
    /**
     * @brief 拷贝构造函数
     * @param pair 要拷贝的源NFPair对象
     * 
     * **功能说明：**
     * 从另一个NFPair对象创建新实例，执行深拷贝操作。
     * 
     * **拷贝过程：**
     * - 检查自我赋值，避免不必要的操作
     * - 分别拷贝first和second成员
     * - 调用成员类型的拷贝构造函数或赋值操作符
     * 
     * **安全性：**
     * - 自我拷贝检测，防止意外错误
     * - 异常安全，如果成员拷贝失败不会破坏对象状态
     * - 深拷贝语义，确保独立的对象实例
     * 
     * **性能考虑：**
     * - 对于POD类型，编译器会优化为简单的内存拷贝
     * - 对于复杂类型，依赖于成员类型的拷贝效率
     * - 自我拷贝检测的开销通常可以忽略
     */
	NFPair(const NFPair& pair)
	{
		if (this != &pair)
		{
			first = pair.first;
			second = pair.second;
		}
	}

    /**
     * @brief 赋值操作符
     * @param pair 要赋值的源NFPair对象
     * @return 返回当前对象的引用，支持链式赋值
     * 
     * **功能说明：**
     * 将另一个NFPair对象的值赋给当前对象。
     * 
     * **赋值过程：**
     * - 自我赋值检测，避免不必要的操作
     * - 分别赋值first和second成员
     * - 调用成员类型的赋值操作符
     * - 返回*this支持链式操作
     * 
     * **异常安全：**
     * - 强异常安全保证，赋值失败时对象保持原状态
     * - 自我赋值安全，不会破坏对象状态
     * - 依赖于成员类型的异常安全性
     * 
     * **返回值设计：**
     * 返回引用支持链式赋值：`a = b = c`
     * 
     * **使用示例：**
     * @code
     * NFPair<int, string> p1(1, "one");
     * NFPair<int, string> p2(2, "two");
     * NFPair<int, string> p3;
     * 
     * p3 = p1;           // 简单赋值
     * p1 = p2 = p3;      // 链式赋值
     * @endcode
     */
	NFPair& operator=(const NFPair& pair)
	{
		if (this != &pair)
		{
			first = pair.first;
			second = pair.second;
		}

		return *this;
	}

    /**
     * @brief 第一个元素
     * @details 存储键值对的第一个值，通常用作"键"或"第一项"。
     * 
     * **访问特性：**
     * - 公有成员，可直接读写访问
     * - 类型为模板参数_T1指定的类型
     * - 支持所有该类型支持的操作
     * 
     * **命名约定：**
     * 遵循std::pair的命名规范，便于代码迁移和理解。
     */
	_T1 first;
    
    /**
     * @brief 第二个元素
     * @details 存储键值对的第二个值，通常用作"值"或"第二项"。
     * 
     * **访问特性：**
     * - 公有成员，可直接读写访问  
     * - 类型为模板参数_T2指定的类型
     * - 支持所有该类型支持的操作
     * 
     * **命名约定：**
     * 遵循std::pair的命名规范，便于代码迁移和理解。
     */
	_T2 second;
};
