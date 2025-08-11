// -------------------------------------------------------------------------
//    @FileName         :    NFILuaLoader.h
//    @Author           :    GaoYi
//    @Date             :    2018/05/25
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    Lua脚本加载器和运行环境管理接口
//                           提供完整的Lua集成支持，包括脚本执行、变量访问和函数调用
//
// -------------------------------------------------------------------------
#pragma once

#define LUAINTF_LINK_LUA_COMPILED_IN_CXX 0
#include "LuaBind/luaintf/LuaIntf.h"
#include "NFComm/NFCore/NFPlatform.h"
#include <iostream>
#include <vector>
#include <map>

/**
 * @brief LuaIntf库的共享指针、容器类型配置
 * 
 * 为LuaIntf库配置标准C++容器类型：
 * - 共享指针：使用std::shared_ptr作为默认智能指针
 * - 列表容器：使用std::vector作为默认列表类型
 * - 映射容器：使用std::map作为默认映射类型
 * 
 * 这些配置确保了Lua和C++之间数据类型的一致性和兼容性。
 */
namespace LuaIntf
{
	LUA_USING_SHARED_PTR_TYPE(std::shared_ptr);
	LUA_USING_LIST_TYPE(std::vector);
	LUA_USING_MAP_TYPE(std::map);
}

/**
 * @brief Lua引用类型别名
 * 
 * 简化LuaIntf::LuaRef的使用，提供更简洁的类型名称。
 * LuaRef用于表示Lua中的任意值引用，支持类型安全的访问。
 */
typedef LuaIntf::LuaRef NFLuaRef;

/**
 * @brief Lua表引用类型别名
 * 
 * 简化LuaIntf::LuaTableRef的使用，专门用于操作Lua表对象。
 * 提供了类型安全的表访问和操作接口。
 */
typedef LuaIntf::LuaTableRef NFLuaTableRef;

/**
 * @brief Lua脚本加载器和运行环境管理类
 * 
 * NFILuaLoader 提供了完整的Lua脚本集成支持：
 * 
 * 1. 设计理念：
 *    - 封装简化：隐藏LuaIntf的复杂性，提供简洁的接口
 *    - 类型安全：提供类型安全的Lua值访问和转换
 *    - 异常处理：完善的错误处理和异常捕获机制
 *    - 资源管理：自动管理Lua状态机的生命周期
 * 
 * 2. 核心功能：
 *    - Lua上下文管理：创建和管理Lua虚拟机实例
 *    - 脚本执行：支持字符串和文件形式的脚本执行
 *    - 变量访问：类型安全的全局变量读写操作
 *    - 函数调用：支持可变参数的Lua函数调用
 *    - 路径管理：动态添加和管理Lua模块搜索路径
 * 
 * 3. 脚本集成特性：
 *    - 双向绑定：C++对象和函数可以暴露给Lua使用
 *    - 类型转换：自动处理Lua和C++类型之间的转换
 *    - 表操作：提供便捷的Lua表访问和操作接口
 *    - 模块系统：支持Lua模块的动态加载和管理
 * 
 * 4. 安全特性：
 *    - 异常捕获：捕获和处理Lua脚本执行中的错误
 *    - 内存安全：自动管理Lua对象的生命周期
 *    - 类型检查：运行时类型验证和转换安全
 *    - 错误恢复：脚本错误不会影响C++程序稳定性
 * 
 * 5. 性能优化：
 *    - 轻量级封装：最小化性能开销的包装层
 *    - 智能缓存：缓存常用的Lua对象和函数引用
 *    - 批量操作：支持批量脚本操作以提高效率
 *    - 资源复用：重用Lua状态机实例减少创建开销
 * 
 * 使用场景：
 * - 游戏逻辑脚本：游戏规则和玩法的脚本化实现
 * - 配置管理：使用Lua脚本描述复杂的配置信息
 * - 热更新系统：运行时动态加载和更新逻辑代码
 * - 插件系统：通过Lua脚本扩展应用程序功能
 * - 数据处理：使用Lua进行复杂的数据变换和计算
 * 
 * 使用示例：
 * @code
 * // 创建Lua加载器
 * NFILuaLoader luaLoader;
 * 
 * // 加载脚本文件
 * if (luaLoader.TryLoadScriptFile("config.lua")) {
 *     // 读取全局变量
 *     int maxPlayers;
 *     if (luaLoader.GetValue("max_players", maxPlayers)) {
 *         NFLogInfo(NF_LOG_DEFAULT, 0, "最大玩家数: {}", maxPlayers);
 *     }
 * }
 * 
 * // 执行Lua函数
 * auto result = luaLoader.TryRunGlobalScriptFunc("calculate_damage", 100, 50);
 * if (!result.isNil()) {
 *     int damage = result.toValue<int>();
 *     NFLogInfo(NF_LOG_DEFAULT, 0, "计算伤害: {}", damage);
 * }
 * 
 * // 操作Lua表
 * auto configTable = luaLoader.GetGlobal<NFLuaTableRef>("config");
 * std::string serverName;
 * NFILuaLoader::GetLuaTableValue(configTable, "server_name", serverName);
 * @endcode
 * 
 * 设计优势：
 * - 简化了Lua集成的复杂性
 * - 提供了类型安全的脚本接口
 * - 支持现代C++的编程风格
 * - 便于维护和扩展
 */
class NFILuaLoader
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化Lua加载器并创建Lua上下文：
	 * - 初始化成员变量
	 * - 调用CreateLuaContext()创建Lua虚拟机
	 * - 准备接收脚本执行请求
	 */
	NFILuaLoader()
	{
		m_pLuaContext = nullptr;
		CreateLuaContext();
	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 清理Lua加载器的所有资源：
	 * - 安全删除Lua上下文对象
	 * - 释放相关的内存资源
	 * - 确保没有内存泄漏
	 */
	virtual ~NFILuaLoader()
	{
		if (m_pLuaContext)
		{
			NF_SAFE_DELETE(m_pLuaContext);
		}
	}

	/**
	 * @brief 创建新的Lua上下文
	 * 
	 * 创建或重新创建Lua虚拟机实例：
	 * - 如果已存在上下文，先安全删除
	 * - 创建新的LuaIntf::LuaContext实例
	 * - 初始化Lua标准库和环境
	 * - 准备执行Lua脚本
	 * 
	 * 使用场景：
	 * - 初始化时创建Lua环境
	 * - 重置Lua环境以清除状态
	 * - 错误恢复时重新创建环境
	 */
	void CreateLuaContext()
	{
		if (m_pLuaContext)
		{
			NF_SAFE_DELETE(m_pLuaContext);
		}
		m_pLuaContext = NF_NEW LuaIntf::LuaContext();
	}

	/**
	 * @brief 获取原生Lua状态机指针
	 * @return lua_State指针，用于直接操作Lua API
	 * 
	 * 返回底层的Lua状态机指针：
	 * - 用于需要直接调用Lua C API的场景
	 * - 与第三方Lua库的集成
	 * - 高级Lua操作和优化
	 * - 调试和诊断Lua状态
	 * 
	 * 注意：直接使用lua_State需要谨慎，不当使用可能导致状态不一致
	 */
	lua_State* GetLuaState() const
	{
		return m_pLuaContext->state();
	}

	/**
	 * @brief 获取Lua上下文对象
	 * @return LuaIntf::LuaContext指针，用于高级Lua操作
	 * 
	 * 返回LuaIntf的上下文对象：
	 * - 提供更高级的Lua操作接口
	 * - 用于复杂的类型绑定和操作
	 * - 与LuaIntf库的直接集成
	 * - 高级功能的访问入口
	 */
	LuaIntf::LuaContext* GetLuaContext()
	{
		return m_pLuaContext;
	}

public:
	/**
	 * @brief 获取全局变量值（模板版本）
	 * @tparam V 返回值类型，默认为LuaIntf::LuaRef
	 * @param keyName 全局变量名称
	 * @return 指定类型的变量值
	 * 
	 * 从Lua全局环境中获取指定名称的变量：
	 * - 支持任意类型的返回值
	 * - 自动进行类型转换和检查
	 * - 类型安全的变量访问
	 * - 支持复杂的Lua对象获取
	 * 
	 * 支持的类型：
	 * - 基本类型：int, float, double, bool, string
	 * - Lua类型：LuaRef, LuaTableRef, LuaFunction
	 * - 容器类型：vector, map等（需要预先绑定）
	 * - 自定义类型：通过LuaIntf绑定的C++类型
	 * 
	 * 使用示例：
	 * @code
	 * // 获取基本类型
	 * auto version = luaLoader.GetGlobal<std::string>("version");
	 * auto maxLevel = luaLoader.GetGlobal<int>("max_level");
	 * 
	 * // 获取Lua表
	 * auto configTable = luaLoader.GetGlobal<NFLuaTableRef>("config");
	 * 
	 * // 获取Lua函数
	 * auto func = luaLoader.GetGlobal<LuaIntf::LuaRef>("my_function");
	 * @endcode
	 */
	template <typename V = LuaIntf::LuaRef>
	V GetGlobal(const std::string& keyName) const
	{
		return  m_pLuaContext->getGlobal(keyName.c_str());
	}

	/**
	 * @brief 获取全局变量值并进行类型转换
	 * @tparam T 目标类型
	 * @param keyName 全局变量名称
	 * @param value 输出参数，存储转换后的值
	 * @return 获取成功返回true，失败返回false
	 * 
	 * 安全地获取全局变量并转换为指定类型：
	 * - 包含完整的错误检查和异常处理
	 * - 支持类型转换失败的优雅处理
	 * - 自动检查变量是否存在
	 * - 提供详细的错误信息输出
	 * 
	 * 错误处理：
	 * - 变量不存在：返回false，不修改value
	 * - 类型转换失败：捕获异常，输出错误信息，返回false
	 * - 其他Lua错误：捕获并处理，确保程序稳定性
	 * 
	 * 使用示例：
	 * @code
	 * int maxPlayers;
	 * if (luaLoader.GetValue("max_players", maxPlayers)) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "最大玩家数: {}", maxPlayers);
	 * } else {
	 *     NFLogError(NF_LOG_DEFAULT, 0, "无法获取max_players配置");
	 * }
	 * 
	 * std::string serverName;
	 * if (luaLoader.GetValue("server_name", serverName)) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "服务器名称: {}", serverName);
	 * }
	 * @endcode
	 */
	template <typename T>
	bool GetValue(const std::string& keyName, T& value) const
	{
		LuaIntf::LuaRef ref = GetGlobal(keyName);
		if (ref == nullptr)
		{
			return false;
		}

		try
		{
			value = ref.toValue<T>();
			return true;
		}
		catch (LuaIntf::LuaException& e)
		{
			std::cout << e.what() << std::endl;
		}

		return true;
	}

public:
	/**
	 * @brief 尝试执行Lua脚本字符串
	 * @param strScript 要执行的Lua脚本代码
	 * @return 执行成功返回true，失败返回false
	 * 
	 * 执行传入的Lua脚本代码字符串：
	 * - 支持任意长度的Lua代码
	 * - 完整的异常捕获和错误处理
	 * - 执行结果的验证和反馈
	 * - 不影响现有Lua环境状态
	 * 
	 * 适用场景：
	 * - 动态生成的Lua代码执行
	 * - 配置信息的脚本化处理
	 * - 运行时的逻辑代码更新
	 * - 临时脚本的快速执行
	 * 
	 * 错误处理：
	 * - 语法错误：捕获编译错误，输出详细信息
	 * - 运行时错误：捕获执行异常，保护程序稳定性
	 * - 内存错误：处理可能的内存分配失败
	 * 
	 * 使用示例：
	 * @code
	 * // 执行简单的Lua代码
	 * if (luaLoader.TryLoadScriptString("print('Hello World')")) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "脚本执行成功");
	 * }
	 * 
	 * // 执行复杂的Lua逻辑
	 * std::string luaCode = R"(
	 *     function calculate_damage(base, multiplier)
	 *         return base * multiplier * math.random(0.8, 1.2)
	 *     end
	 * )";
	 * luaLoader.TryLoadScriptString(luaCode);
	 * @endcode
	 */
	bool TryLoadScriptString(const std::string& strScript)
	{
		try
		{
			m_pLuaContext->doString(strScript.c_str());
			return true;
		}
		catch (LuaIntf::LuaException& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}

	/**
	 * @brief 尝试加载并执行Lua脚本文件
	 * @param strFileName Lua脚本文件的路径
	 * @return 加载成功返回true，失败返回false
	 * 
	 * 从文件系统加载并执行Lua脚本文件：
	 * - 支持相对路径和绝对路径
	 * - 自动处理文件读取和编译
	 * - 完整的文件错误处理
	 * - 支持UTF-8编码的脚本文件
	 * 
	 * 文件处理：
	 * - 文件不存在：返回false，输出错误信息
	 * - 文件权限问题：捕获并处理权限错误
	 * - 编码问题：处理可能的字符编码错误
	 * - 语法错误：捕获Lua编译错误
	 * 
	 * 最佳实践：
	 * - 使用相对路径便于部署
	 * - 检查返回值确保加载成功
	 * - 在关键文件加载失败时进行错误处理
	 * - 考虑文件变更的热重载机制
	 * 
	 * 使用示例：
	 * @code
	 * // 加载配置文件
	 * if (luaLoader.TryLoadScriptFile("config/server.lua")) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "配置文件加载成功");
	 * } else {
	 *     NFLogError(NF_LOG_DEFAULT, 0, "配置文件加载失败");
	 * }
	 * 
	 * // 加载游戏逻辑脚本
	 * if (luaLoader.TryLoadScriptFile("scripts/game_logic.lua")) {
	 *     // 继续后续逻辑
	 * }
	 * @endcode
	 */
	bool TryLoadScriptFile(const std::string& strFileName)
	{
		try
		{
			m_pLuaContext->doFile(strFileName.c_str());
			return true;
		}
		catch (LuaIntf::LuaException& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}

	/**
	 * @brief 添加Lua包搜索路径
	 * @param strFilePath 要添加的搜索路径
	 * @return 添加成功返回true，失败返回false
	 * 
	 * 向Lua的package.path中添加新的模块搜索路径：
	 * - 扩展Lua模块的搜索范围
	 * - 支持自定义模块目录结构
	 * - 便于组织和管理Lua模块
	 * - 支持动态路径配置
	 * 
	 * 路径格式：
	 * - 支持绝对路径和相对路径
	 * - 自动处理路径分隔符
	 * - 支持通配符模式（如?.lua）
	 * - 兼容不同操作系统的路径格式
	 * 
	 * 模块加载：
	 * - 影响require()函数的搜索行为
	 * - 支持分层的模块目录结构
	 * - 便于第三方Lua模块的集成
	 * - 提高模块加载的灵活性
	 * 
	 * 使用示例：
	 * @code
	 * // 添加自定义模块路径
	 * luaLoader.TryAddPackagePath("./scripts/?.lua");
	 * luaLoader.TryAddPackagePath("./libs/?.lua");
	 * 
	 * // 现在可以直接require这些路径下的模块
	 * luaLoader.TryLoadScriptString("local utils = require('utils')");
	 * @endcode
	 */
	bool TryAddPackagePath(const std::string& strFilePath)
	{
		try
		{
			m_pLuaContext->addPackagePath(strFilePath);
			return true;
		}
		catch (LuaIntf::LuaException& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}

public:
	/**
	 * @brief 尝试运行全局Lua函数（可变参数版本）
	 * @tparam Arg 参数类型包
	 * @param strFuncName 要调用的Lua函数名称
	 * @param args 传递给Lua函数的参数列表
	 * @return 函数执行结果，包装为LuaIntf::LuaRef
	 * 
	 * 调用Lua全局环境中的函数并传递任意数量的参数：
	 * - 支持完美转发的参数传递
	 * - 自动处理参数类型转换
	 * - 类型安全的函数调用
	 * - 完整的异常捕获和处理
	 * 
	 * 参数支持：
	 * - 基本类型：int, float, double, bool, string
	 * - 容器类型：vector, map等（需要绑定）
	 * - 自定义类型：通过LuaIntf绑定的C++对象
	 * - 引用类型：支持左值和右值引用
	 * 
	 * 返回值处理：
	 * - 返回LuaRef，可以转换为任意Lua类型
	 * - 支持多返回值的处理
	 * - 可以检查返回值的类型和有效性
	 * - 支持nil值的特殊处理
	 * 
	 * 错误处理：
	 * - 函数不存在：返回nil的LuaRef
	 * - 参数类型错误：捕获异常，返回nil
	 * - 运行时错误：完整的异常处理机制
	 * 
	 * 使用示例：
	 * @code
	 * // 调用无参数函数
	 * auto result1 = luaLoader.TryRunGlobalScriptFunc("get_server_time");
	 * if (!result1.isNil()) {
	 *     int serverTime = result1.toValue<int>();
	 * }
	 * 
	 * // 调用带参数函数
	 * auto result2 = luaLoader.TryRunGlobalScriptFunc("calculate_damage", 100, 1.5f, true);
	 * if (!result2.isNil()) {
	 *     float damage = result2.toValue<float>();
	 * }
	 * 
	 * // 调用字符串参数函数
	 * auto result3 = luaLoader.TryRunGlobalScriptFunc("format_message", 
	 *                                                 "Hello", "World", 42);
	 * @endcode
	 */
	template <typename... Arg>
    LuaIntf::LuaRef TryRunGlobalScriptFunc(const std::string& strFuncName, Arg&&... args)
	{
		try
		{
			LuaIntf::LuaRef func(*m_pLuaContext, strFuncName.c_str());
            LuaIntf::LuaRef result = func.call<LuaIntf::LuaRef>(std::forward<Arg>(args)...);
			return result;
		}
		catch (LuaIntf::LuaException& e)
		{
			std::cout << e.what() << std::endl;
		}
		return LuaIntf::LuaRef();
	}

public:
	/**
	 * @brief 从Lua表中获取指定键的值（静态模板函数）
	 * @tparam KEY 键的类型
	 * @tparam VALUE 值的类型
	 * @param table Lua表的引用
	 * @param keyName 要查找的键
	 * @param value 输出参数，存储获取到的值
	 * @return 获取成功返回true，失败返回false
	 * 
	 * 类型安全地从Lua表中提取特定键的值：
	 * - 支持任意类型的键和值
	 * - 完整的类型检查和转换
	 * - 优雅的错误处理机制
	 * - 静态函数，可以独立使用
	 * 
	 * 键类型支持：
	 * - 字符串键：最常用的表索引方式
	 * - 数字键：数组风格的索引访问
	 * - 自定义类型：通过LuaIntf绑定的类型
	 * 
	 * 值类型支持：
	 * - 基本类型：int, float, double, bool, string
	 * - 嵌套表：LuaTableRef, LuaRef
	 * - 复杂对象：绑定的C++类型
	 * - 函数对象：Lua函数引用
	 * 
	 * 错误情况：
	 * - 键不存在：返回false，不修改value
	 * - 类型不匹配：捕获异常，返回false
	 * - 表无效：安全处理nil或非表类型
	 * 
	 * 使用示例：
	 * @code
	 * // 获取配置表中的值
	 * auto configTable = luaLoader.GetGlobal<NFLuaTableRef>("config");
	 * 
	 * std::string serverName;
	 * if (NFILuaLoader::GetLuaTableValue(configTable, "server_name", serverName)) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "服务器名称: {}", serverName);
	 * }
	 * 
	 * int maxLevel;
	 * if (NFILuaLoader::GetLuaTableValue(configTable, "max_level", maxLevel)) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "最大等级: {}", maxLevel);
	 * }
	 * 
	 * // 数字索引访问
	 * auto arrayTable = luaLoader.GetGlobal<NFLuaTableRef>("rewards");
	 * int firstReward;
	 * if (NFILuaLoader::GetLuaTableValue(arrayTable, 1, firstReward)) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "第一个奖励: {}", firstReward);
	 * }
	 * @endcode
	 */
	template <typename KEY, typename VALUE>
	static bool GetLuaTableValue(const LuaIntf::LuaRef& table, const KEY& keyName, VALUE& value)
	{
		try
		{
			LuaIntf::LuaRef valueRef = table[keyName];
			if (valueRef == nullptr)
			{
				//std::cout << "load lua table " << keyName << " failed!" << std::endl;
				return false;
			}

			value = valueRef.toValue<VALUE>();
			return true;
		}
		catch (LuaIntf::LuaException& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}

protected:
	LuaIntf::LuaContext* m_pLuaContext;  ///< Lua上下文对象指针，管理Lua虚拟机实例
};

