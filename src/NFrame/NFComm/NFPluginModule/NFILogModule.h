// -------------------------------------------------------------------------
//    @FileName         :    NFILogModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFILogModule
//    @Description      :    日志模块接口定义，提供完整的日志管理和输出功能
//                           支持多级别日志、格式化输出、条件过滤和Lua集成
//
// -------------------------------------------------------------------------

#ifndef NFI_LOG_MODULE_H
#define NFI_LOG_MODULE_H

#include "NFIModule.h"
#include "common/spdlog/fmt/fmt.h"
#include "google/protobuf/message.h"
#include "NFComm/NFKernelMessage/FrameEnum.pb.h"
#include "NFComm/NFKernelMessage/FrameEnum.nanopb.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"

/// @brief 在编译时获取__FILE__的基本名称（如果可能的话）
#if FMT_HAS_FEATURE(__builtin_strrchr)
#define NFLOG_STRRCHR(str, sep) __builtin_strrchr(str, sep)
#else
#define NFLOG_STRRCHR(str, sep) strrchr(str, sep)
#endif //__builtin_strrchr not defined

/// @brief 根据平台获取文件基本名称的宏定义
#ifdef _WIN32
/// @brief Windows平台下获取文件基本名称（去除路径前缀）
#define NFLOG_FILE_BASENAME(file) NFLOG_STRRCHR("\\" file, '\\') + 1
#else
/// @brief Linux/Unix平台下获取文件基本名称（去除路径前缀）
#define NFLOG_FILE_BASENAME(file) NFLOG_STRRCHR("/" file, '/') + 1
#endif

/**
 * @brief 日志源码位置信息结构体
 * 
 * 用于记录日志产生的源码位置信息，包括文件名、行号和函数名。
 * 这些信息有助于快速定位日志的来源，便于调试和问题排查。
 */
struct NFSourceLoc
{
	/**
	 * @brief 默认构造函数
	 * 
	 * 初始化所有字段为默认值。
	 */
	NFSourceLoc()
		: filename{""}
		, line{0}
		, funcname{""}
	{
	}
	
	/**
	 * @brief 带参数的构造函数
	 * @param filename 文件名
	 * @param line 行号
	 * @param funcname 函数名
	 */
	NFSourceLoc(const char *filename, int line, const char *funcname)
		: filename{filename}
		, line{static_cast<uint32_t>(line)}
		, funcname{funcname}
	{
	}

	/**
	 * @brief 检查位置信息是否为空
	 * @return 如果行号为0则返回true，否则返回false
	 */
	 bool empty() const
	{
		return line == 0;
	}
	
	const char *filename;  ///< 文件名
	uint32_t line;         ///< 行号
	const char *funcname;  ///< 函数名
};


class LogInfoConfig;

/**
 * @brief 日志模块接口类
 * 
 * NFILogModule 提供了完整的日志管理功能，支持：
 * 
 * 1. 多级别日志输出：
 *    - DEBUG、INFO、WARN、ERROR等不同级别
 *    - 支持日志级别过滤和控制
 * 
 * 2. 格式化日志：
 *    - 支持fmt库的现代C++格式化语法
 *    - 模板化的日志输出接口
 * 
 * 3. 条件日志：
 *    - 基于logId的条件输出
 *    - 基于用户ID的个性化日志
 * 
 * 4. 特殊日志类型：
 *    - 行为日志（Behaviour Log）
 *    - Lua脚本日志集成
 * 
 * 5. 源码位置跟踪：
 *    - 自动记录日志产生的文件、行号、函数
 *    - 便于调试和问题定位
 * 
 * 日志系统特性：
 * - 高性能异步日志输出
 * - 灵活的配置管理
 * - 模块化的日志分类
 * - 运行时动态控制
 */
class NFILogModule
	: public NFIModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFILogModule(NFIPluginManager* p) :NFIModule(p)
	{

	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构，清理日志系统资源。
	 */
	virtual ~NFILogModule()
	{

	}
	
	/**
	 * @brief 初始化日志系统
	 * 
	 * 初始化日志输出系统，包括日志文件创建、格式设置、
	 * 输出级别配置等基础工作。
	 */
	virtual void InitLogSystem() = 0;

	/**
	 * @brief 模板化的格式化日志输出接口
	 * @tparam ARGS 可变参数类型
	 * @param log_level 日志级别
	 * @param loc 源码位置信息
	 * @param logId 日志选项ID，可配置是否输出
	 * @param guid 用户ID，用于个性化日志过滤
	 * @param my_fmt 格式化字符串
	 * @param args 格式化参数
	 * 
	 * 使用现代C++格式化语法的日志输出接口，支持类型安全的参数传递。
	 * 
	 * 使用示例：
	 * @code
	 * Log(NLL_INFO_NORMAL, loc, 1001, playerId, "Player {} login from {}", playerName, ipAddress);
	 * @endcode
	 */
	template<typename... ARGS>
	void Log(NF_LOG_LEVEL log_level, const NFSourceLoc& loc, uint32_t logId, uint64_t guid, const char* my_fmt, const ARGS& ... args)
	{
		std::string str = fmt::format(my_fmt, args...);
		LogDefault(log_level, loc, logId, guid, str);
	}

	/**
	 * @brief 输出默认日志（带源码位置信息）
	 * @param log_level 日志级别
	 * @param loc 源码位置信息
	 * @param logId 日志选项ID，可配置是否输出
	 * @param guid 用户ID，某些情况下只输出特定用户的日志
	 * @param log 日志内容
	 * 
	 * 输出包含完整源码位置信息的日志，便于调试和问题定位。
	 */
	virtual void LogDefault(NF_LOG_LEVEL log_level, const NFSourceLoc& loc, uint32_t logId, uint64_t guid, const std::string& log) = 0;

	/**
	 * @brief 输出默认日志（带模块信息）
	 * @param log_level 日志级别
	 * @param loc 源码位置信息
	 * @param logId 日志选项ID
	 * @param guid 用户ID
	 * @param module 模块ID
	 * @param log 日志内容
	 * 
	 * 输出包含模块信息的日志，用于模块化的日志管理。
	 */
	virtual void LogDefault(NF_LOG_LEVEL log_level, const NFSourceLoc& loc, uint32_t logId, uint64_t guid, uint32_t module, const std::string& log) = 0;

	/**
	 * @brief 输出行为日志
	 * @param log_level 日志级别
	 * @param logId 日志选项ID
	 * @param log 日志内容
	 * 
	 * 专门用于记录用户行为的日志输出，通常用于数据分析和统计。
	 */
	virtual void LogBehaviour(NF_LOG_LEVEL log_level, uint32_t logId, const std::string& log) = 0;

	/**
	 * @brief 输出默认日志（简化版）
	 * @param log_level 日志级别
	 * @param logId 日志选项ID
	 * @param guid 用户ID
	 * @param log 日志内容
	 * 
	 * 不包含源码位置信息的简化日志输出接口。
	 */
	virtual void LogDefault(NF_LOG_LEVEL log_level, uint32_t logId, uint64_t guid, const std::string& log) = 0;

	/**
	 * @brief 设置默认日志配置
	 * 
	 * 应用默认的日志配置参数，包括输出级别、格式、文件路径等。
	 */
	virtual void SetDefaultLogConfig() = 0;

	/**
	 * @brief 检查指定日志ID在指定级别下是否启用
	 * @param log_level 日志级别
	 * @param logId 日志选项ID
	 * @return 如果启用返回true，否则返回false
	 * 
	 * 用于在输出日志前检查是否需要输出，避免不必要的字符串格式化开销。
	 */
	virtual bool IsLogIdEnable(NF_LOG_LEVEL log_level, uint32_t logId) = 0;

	/**
	 * @brief Lua系统的DEBUG级别日志输出
	 * @param logId 日志选项ID
	 * @param guid 用户ID
	 * @param str 日志内容
	 * 
	 * 专门为Lua脚本系统提供的DEBUG级别日志输出接口，
	 * 会自动添加[Lua]标识前缀。
	 */
	virtual void LuaDebug(uint32_t logId, uint64_t guid, const std::string& str)
	{
		std::string strInfo = fmt::format("[Lua] | {}", str);
		LogDefault(NLL_DEBUG_NORMAL, logId, guid, strInfo);
	}

	/**
	 * @brief Lua系统的INFO级别日志输出
	 * @param logId 日志选项ID
	 * @param guid 用户ID
	 * @param str 日志内容
	 * 
	 * 专门为Lua脚本系统提供的INFO级别日志输出接口，
	 * 会自动添加[Lua]标识前缀。
	 */
	virtual void LuaInfo(uint32_t logId, uint64_t guid, const std::string& str)
	{
		std::string strInfo = fmt::format("[Lua] | {}", str);
		LogDefault(NLL_INFO_NORMAL, logId, guid, strInfo);
	}

	/**
	 * @brief Lua系统的WARN级别日志输出
	 * @param logId 日志选项ID
	 * @param guid 用户ID
	 * @param str 日志内容
	 * 
	 * 专门为Lua脚本系统提供的WARN级别日志输出接口，
	 * 会自动添加[Lua]标识前缀。
	 */
	virtual void LuaWarn(uint32_t logId, uint64_t guid, const std::string& str)
	{
		std::string strInfo = fmt::format("[Lua] | {}", str);
		LogDefault(NLL_WARING_NORMAL, logId, guid, strInfo);
	}

	/**
	 * @brief Lua系统的ERROR级别日志输出
	 * @param logId 日志选项ID
	 * @param guid 用户ID
	 * @param str 日志内容
	 * 
	 * 专门为Lua脚本系统提供的ERROR级别日志输出接口，
	 * 会自动添加[Lua]标识前缀。
	 */
	virtual void LuaError(uint32_t logId, uint64_t guid, const std::string& str)
	{
		std::string strInfo = fmt::format("[Lua] | {}", str);
		LogDefault(NLL_ERROR_NORMAL, logId, guid, strInfo);
	}
};

#endif

