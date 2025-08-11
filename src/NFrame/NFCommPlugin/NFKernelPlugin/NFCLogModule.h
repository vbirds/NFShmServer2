// -------------------------------------------------------------------------
//    @FileName         :    NFCLogModule.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFCLogModule
//    @Desc             :    日志模块头文件，提供统一的日志记录和管理系统。
//                          该文件定义了NFShmXFrame框架的日志模块，基于spdlog库实现高性能日志系统，
//                          包括多级别日志、多格式输出、异步日志、日志分类、日志过滤等功能。
//                          主要功能包括线程安全的日志操作、自动日志轮转和归档、格式化输出支持、
//                          源码位置记录、配置热重载支持、零拷贝日志数据传递
//    @Description      :    日志模块头文件，提供统一的日志记录和管理系统
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFPluginModule/NFILogModule.h"
#include "NFComm/NFCore/NFPlatform.h"

#include "common/spdlog/spdlog.h"
#include "common/spdlog/fmt/fmt.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @class NFCLogModule
 * @brief 日志模块实现类
 *
 * NFCLogModule基于spdlog库实现的高性能日志系统，提供了完整的日志解决方案：
 * 
 * 日志功能特性：
 * - 多级别日志：DEBUG、INFO、WARN、ERROR、FATAL等
 * - 多格式输出：控制台、文件、网络等多种输出方式
 * - 异步日志：支持高性能的异步日志写入
 * - 日志分类：支持按模块、功能、玩家等维度分类记录
 * - 日志过滤：可配置的日志级别和ID过滤机制
 * 
 * 高级特性：
 * - 线程安全的日志操作
 * - 自动日志轮转和归档
 * - 格式化输出支持（基于fmt库）
 * - 源码位置记录（文件名、行号、函数名）
 * - 进程ID和线程ID记录
 * - 配置热重载支持
 * 
 * 日志分类系统：
 * - 默认日志：系统通用日志
 * - 行为日志：玩家行为和业务逻辑日志
 * - 自定义日志：按需创建的专用日志记录器
 * - 模块日志：按模块ID分类的日志
 * 
 * 性能优化：
 * - 零拷贝日志数据传递
 * - 内存池管理日志缓冲区
 * - 批量日志写入优化
 * - 异步日志线程池
 * 
 * @note 该模块是框架启动的第一个模块，为其他模块提供日志服务
 * @note 基于spdlog高性能日志库实现
 */
class NFCLogModule : public NFILogModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFCLogModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 */
	virtual ~NFCLogModule();

	/**
	 * @brief 关闭日志模块
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Shut() override;
	
	/**
	 * @brief 配置重载处理
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 重新加载日志配置，更新日志级别、输出格式等设置。
	 */
	virtual int OnReloadConfig() override;

	/**
	 * @brief 初始化日志系统
	 * 
	 * 创建默认的日志记录器，设置日志格式和输出目标。
	 * 这是框架启动时调用的第一个初始化函数。
	 */
	virtual void InitLogSystem() override;

	/**
	 * @brief 输出默认日志（带源码位置信息）
	 * @param log_level 日志级别
	 * @param loc 源码位置信息（文件名、行号、函数名）
	 * @param logId 日志选项ID，可配置是否输出
	 * @param guid 玩家或对象ID，用于特定对象的日志过滤
	 * @param log 日志内容
	 * 
	 * 记录包含完整上下文信息的日志。
	 */
	virtual void LogDefault(NF_LOG_LEVEL log_level, const NFSourceLoc& loc, uint32_t logId, uint64_t guid, const std::string& log);

	/**
	 * @brief 输出默认日志（带模块信息）
	 * @param log_level 日志级别
	 * @param loc 源码位置信息
	 * @param logId 日志选项ID
	 * @param guid 玩家或对象ID
	 * @param module 模块ID，用于标识日志来源模块
	 * @param log 日志内容
	 * 
	 * 记录包含模块信息的日志，便于按模块查看和分析。
	 */
	virtual void LogDefault(NF_LOG_LEVEL log_level, const NFSourceLoc& loc, uint32_t logId, uint64_t guid, uint32_t module, const std::string& log);

	/**
	 * @brief 输出行为日志
	 * @param log_level 日志级别
	 * @param logId 日志选项ID
	 * @param log 日志内容
	 * 
	 * 专门用于记录玩家行为、业务逻辑等重要操作的日志。
	 * 通常用于数据分析和问题追踪。
	 */
	virtual void LogBehaviour(NF_LOG_LEVEL log_level, uint32_t logId, const std::string& log);

	/**
	 * @brief 检查指定日志ID是否启用
	 * @param log_level 日志级别
	 * @param logId 日志选项ID
	 * @return 返回true表示启用，false表示禁用
	 * 
	 * 在输出日志前检查是否需要记录该类型的日志，
	 * 可以避免不必要的字符串格式化操作。
	 */
	virtual bool IsLogIdEnable(NF_LOG_LEVEL log_level, uint32_t logId);

	/**
	 * @brief 输出默认日志（简化版本）
	 * @param log_level 日志级别
	 * @param logId 日志选项ID
	 * @param guid 玩家或对象ID
	 * @param log 日志内容
	 * 
	 * 不包含源码位置信息的简化日志输出。
	 */
	virtual void LogDefault(NF_LOG_LEVEL log_level, uint32_t logId, uint64_t guid, const std::string& log);

	/**
	 * @brief 设置默认日志配置
	 * 
	 * 使用系统默认的日志配置参数，包括日志级别、输出格式、
	 * 文件轮转规则等。
	 */
	virtual void SetDefaultLogConfig();

	/**
	 * @brief 创建自定义日志记录器
	 * @param logId 日志记录器的唯一ID
	 * @param logName 日志文件名
	 * @param async 是否使用异步模式
	 * @param level 日志级别，默认为debug级别
	 * @return 返回日志记录器的智能指针
	 * 
	 * 创建独立的日志记录器，用于特定模块或功能的专用日志。
	 */
	std::shared_ptr<spdlog::logger> CreateLogger(uint32_t logId, const std::string& logName, bool async, uint32_t level = spdlog::level::level_enum::debug);

	/**
	 * @brief 根据配置信息创建日志记录器
	 * @param logInfo 日志配置信息对象
	 * @return 返回日志记录器的智能指针
	 * 
	 * 基于详细的配置信息创建日志记录器。
	 */
	std::shared_ptr<spdlog::logger> CreateLogger(const LogInfoConfig& logInfo);

protected:
	/**
	 * @brief 创建默认系统日志记录器
	 * 
	 * 初始化系统的主要日志记录器，设置基本的输出格式和规则。
	 */
	void CreateDefaultLogger();

private:
	/**
	 * @brief 默认日志记录器
	 * 
	 * 系统的主要日志记录器，用于记录框架和应用的通用日志。
	 */
	std::shared_ptr<spdlog::logger> m_defaultLogger;
	
	/**
	 * @brief 日志记录器列表
	 * 
	 * 存储所有创建的日志记录器，便于统一管理和清理。
	 */
	std::vector<std::shared_ptr<spdlog::logger>> m_loggerMap;
	
	/**
	 * @brief 日志配置信息列表
	 * 
	 * 存储各个日志记录器的配置信息。
	 */
	std::vector<LogInfoConfig> m_logInfoConfig;
	
	/**
	 * @brief 异步日志线程池
	 * 
	 * 用于异步日志写入的线程池，提高日志系统的性能。
	 */
	NF_SHARE_PTR<spdlog::details::thread_pool> m_logThreadPool;
	
	/**
	 * @brief 进程ID字符串
	 * 
	 * 当前进程的ID，用于在多进程环境中区分日志来源。
	 */
	std::string m_pid;
};


