// -------------------------------------------------------------------------
//    @FileName         :    NFStat.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFStat.h
 * @brief 统计功能类定义
 * 
 * 此文件提供了系统性能和消息处理的统计功能，包括资源使用统计、
 * 消息处理统计等。主要用于系统监控和性能分析。
 */

#pragma once

#include "NFPlatform.h"

#include <unordered_map>
#include <stdint.h>

/**
 * @brief 资源类统计结果
 * 
 * 用于存储资源统计的计算结果，包括平均值、最大值和最小值。
 * 主要用于CPU使用率、内存占用率、网络流量等资源类指标的统计。
 * 
 * 使用方法：
 * @code
 * ResourceStatItem item;
 * // 通过NFStat类计算后获取结果
 * std::cout << "平均值: " << item._average_value << std::endl;
 * std::cout << "最大值: " << item._max_value << std::endl;
 * std::cout << "最小值: " << item._min_value << std::endl;
 * @endcode
 */
class ResourceStatItem
{
public:
	/** @brief 统计周期内的平均值 */
	float _average_value;
	/** @brief 统计周期内的最大值 */
	float _max_value;
	/** @brief 统计周期内的最小值 */
	float _min_value;

	/**
	 * @brief 默认构造函数
	 * 初始化所有统计值为0
	 */
	ResourceStatItem()
	{
		_average_value = 0;
		_max_value = 0;
		_min_value = 0;
	}
};

/**
 * @brief 消息类统计结果
 * 
 * 用于存储消息处理统计的计算结果，包括失败率、处理耗时等信息。
 * 主要用于网络消息、RPC调用、数据库操作等消息类处理的统计。
 * 
 * 特性：
 * - 失败率统计：计算消息处理失败的比例
 * - 错误码分布：记录各种错误码的出现次数
 * - 耗时统计：记录消息处理的平均、最大、最小耗时
 * 
 * 使用方法：
 * @code
 * MessageStatItem item;
 * // 通过NFStat类计算后获取结果
 * std::cout << "失败率: " << item._failure_rate << "%" << std::endl;
 * std::cout << "平均耗时: " << item._average_cost_ms << "ms" << std::endl;
 * 
 * // 查看各错误码的分布
 * for (auto& pair : item._result) {
 *     std::cout << "错误码 " << pair.first << ": " << pair.second << " 次" << std::endl;
 * }
 * @endcode
 */
class MessageStatItem
{
public:
	/** @brief 失败率（百分比，0-100） */
	float _failure_rate;
	/** @brief 各种结果码的统计次数（key: 结果码，value: 出现次数） */
	std::unordered_map<int32_t, uint32_t> _result;
	/** @brief 平均处理耗时（毫秒） */
	uint32_t _average_cost_ms;
	/** @brief 最大处理耗时（毫秒） */
	uint32_t _max_cost_ms;
	/** @brief 最小处理耗时（毫秒） */
	uint32_t _min_cost_ms;

	/**
	 * @brief 默认构造函数
	 * 初始化所有统计值，最小耗时设为最大值以便后续比较
	 */
	MessageStatItem()
	{
		_failure_rate = 0;
		_average_cost_ms = 0;
		_max_cost_ms = 0;
		_min_cost_ms = UINT32_MAX;
	}
};

/** @brief 资源统计结果类型定义（名称 -> 统计结果） */
typedef std::unordered_map<std::string, ResourceStatItem> ResourceStatResult;
/** @brief 消息统计结果类型定义（名称 -> 统计结果） */
typedef std::unordered_map<std::string, MessageStatItem> MessageStatResult;

/**
 * @brief 基础统计模块
 * 
 * NFStat提供数据记录和计算功能，数据的使用交给调用者。
 * 支持两种类型的统计：
 * 
 * 1. 资源类统计：
 *    - 适用于周期性采样的指标（如CPU使用率、内存占用）
 *    - 提供平均值、最大值、最小值的计算
 *    - 支持浮点数值的统计
 * 
 * 2. 消息类统计：
 *    - 适用于事件触发的操作（如API调用、消息处理）
 *    - 提供成功率、失败率、耗时分析
 *    - 支持错误码分布统计
 * 
 * 主要特性：
 * - 线程安全：所有公开接口都是线程安全的
 * - 实时计算：数据添加时实时计算统计结果
 * - 内存高效：使用临时数据结构优化内存使用
 * - 灵活查询：支持按名称查询和批量查询
 * 
 * 使用方法：
 * @code
 * NFStat stat;
 * 
 * // 资源统计示例
 * stat.AddResourceItem("CPU使用率", 45.2f);
 * stat.AddResourceItem("CPU使用率", 52.8f);
 * stat.AddResourceItem("内存使用率", 68.5f);
 * 
 * // 消息统计示例
 * stat.AddMessageItem("用户登录", 0, 120);    // 成功，耗时120ms
 * stat.AddMessageItem("用户登录", -1, 250);   // 失败，耗时250ms
 * stat.AddMessageItem("数据查询", 0, 85);     // 成功，耗时85ms
 * 
 * // 获取统计结果
 * auto* cpuStat = stat.GetResourceResultByName("CPU使用率");
 * auto* loginStat = stat.GetMessageResultByName("用户登录");
 * 
 * // 清理数据
 * stat.Clear();
 * @endcode
 */
class NFStat
{
public:
	/**
	 * @brief 构造函数
	 * 初始化消息计数器为0
	 */
	NFStat() : m_message_counts(0), m_failure_message_counts(0)
	{
	}

	/**
	 * @brief 析构函数
	 */
	~NFStat()
	{
	}

	/**
	 * @brief 清理已记录的数据
	 * 
	 * 清除所有已记录的统计数据，包括临时数据和结果数据。
	 * 调用后统计对象恢复到初始状态。
	 */
	void Clear();

	/**
	 * @brief 添加资源统计项
	 * 
	 * 资源类统计一般用于定时采样，周期输出统计结果（平均值、最大值、最小值等）。
	 * 适用于CPU使用率、内存占用率、磁盘使用率等连续性指标。
	 * 
	 * @param name 资源项名称，要求非空且唯一标识一类资源
	 * @param value 采样值，可以是百分比、绝对数值等
	 * @return int32_t 0表示成功，非0表示失败
	 * 
	 * @note 相同名称的多次调用会累积计算统计结果
	 * @note 建议资源名称使用描述性的字符串，如"CPU使用率"、"内存占用MB"
	 */
	int32_t AddResourceItem(const std::string& name, float value);

	/**
	 * @brief 添加消息统计
	 * 
	 * 用于统计消息处理的结果和耗时，支持成功率、失败率、响应时间等指标的计算。
	 * 适用于API调用、数据库操作、网络请求等事件驱动的操作。
	 * 
	 * @param name 消息标识，如消息名或操作名，要求非空且唯一标识一类操作
	 * @param result 消息处理结果，0表示成功，非0为错误码表示失败
	 * @param time_cost_ms 消息处理时延，单位毫秒
	 * @return int32_t 0表示成功，非0表示失败
	 * 
	 * @note 成功和失败的定义由result参数决定，0为成功，其他为失败
	 * @note 错误码会被统计到结果分布中，便于分析失败原因
	 */
	int32_t AddMessageItem(const std::string& name, int32_t result, int32_t time_cost_ms);

	/**
	 * @brief 添加消息统计项（仅计数）
	 * 
	 * 如果无此消息的统计则增加一个统计项，值为0。
	 * 主要用于消息计数，不关心处理结果和耗时的场景。
	 * 
	 * @param name 消息标识，如消息名，要求非空
	 * @return int32_t 0表示成功，非0表示失败
	 */
	int32_t AddMessageItem(const std::string& name);

	/**
	 * @brief 按名字获取资源型统计结果
	 * 
	 * 根据资源名称查询对应的统计结果，包括平均值、最大值、最小值。
	 * 
	 * @param name 资源名称，必须与AddResourceItem中使用的名称一致
	 * @return const ResourceStatItem* 成功返回统计结果指针，失败返回nullptr
	 * 
	 * @note 返回的指针指向内部数据，不要删除或修改
	 * @note 如果统计项不存在，返回nullptr
	 */
	const ResourceStatItem* GetResourceResultByName(const std::string& name);

	/**
	 * @brief 按名字获取消息型统计结果
	 * 
	 * 根据消息名称查询对应的统计结果，包括失败率、耗时分析、错误码分布等。
	 * 
	 * @param name 消息名称，必须与AddMessageItem中使用的名称一致
	 * @return const MessageStatItem* 成功返回统计结果指针，失败返回nullptr
	 * 
	 * @note 返回的指针指向内部数据，不要删除或修改
	 * @note 如果统计项不存在，返回nullptr
	 */
	const MessageStatItem* GetMessageResultByName(const std::string& name);

	/**
	 * @brief 获取所有资源型统计结果
	 * 
	 * 返回所有资源统计的结果映射，可用于批量处理或输出报告。
	 * 
	 * @return const ResourceStatResult* 统计结果映射指针，key为资源名称，value为统计结果
	 * 
	 * @note 返回的指针指向内部数据，不要删除或修改
	 */
	const ResourceStatResult* GetAllResourceResults();

	/**
	 * @brief 获取所有消息型统计结果
	 * 
	 * 返回所有消息统计的结果映射，可用于批量处理或输出报告。
	 * 
	 * @return const MessageStatResult* 统计结果映射指针，key为消息名称，value为统计结果
	 * 
	 * @note 返回的指针指向内部数据，不要删除或修改
	 */
	const MessageStatResult* GetAllMessageResults();

	/**
	 * @brief 获取所有消息总数
	 * 
	 * 返回从创建或上次Clear()以来处理的消息总数（包括成功和失败的）。
	 * 
	 * @return uint32_t 消息总数
	 */
	uint32_t GetAllMessageCounts() const;

	/**
	 * @brief 获取所有失败的消息数
	 * 
	 * 返回从创建或上次Clear()以来处理失败的消息总数（result != 0的消息）。
	 * 
	 * @return uint32_t 失败消息总数
	 */
	uint32_t GetAllFailureMessageCounts() const;

private:
	/**
	 * @brief 资源类统计中的临时数据
	 * 
	 * 用于在统计过程中临时存储数据，便于最终计算平均值、最大值、最小值。
	 */
	class ResourceStatTempData
	{
	public:
		/** @brief 采样次数 */
		uint32_t _count;
		/** @brief 累计值，用于计算平均值 */
		float _total_value;
		/** @brief 指向最终结果的指针 */
		ResourceStatItem* _result;

		/**
		 * @brief 构造函数，初始化临时数据
		 */
		ResourceStatTempData()
		{
			_count = 0;
			_total_value = 0;
			_result = nullptr;
		}
	};

	/**
	 * @brief 消息类统计中的临时数据
	 * 
	 * 用于在统计过程中临时存储数据，便于最终计算失败率、平均耗时等。
	 */
	class MessageStatTempData
	{
	public:
		/** @brief 消息总数 */
		int64_t _total_count;
		/** @brief 累计耗时，用于计算平均耗时 */
		int64_t _total_cost_ms;
		/** @brief 失败次数，用于计算失败率 */
		float _failure_count;
		/** @brief 指向最终结果的指针 */
		MessageStatItem* _result;

		/**
		 * @brief 构造函数，初始化临时数据
		 */
		MessageStatTempData()
		{
			_total_count = 0;
			_failure_count = 0;
			_total_cost_ms = 0;
			_result = nullptr;
		}
	};

	/**
	 * @brief 计算资源统计结果
	 * 
	 * 根据临时数据计算最终的资源统计结果
	 * 
	 * @param resource_stat_temp 资源统计临时数据指针
	 */
	static void CalculateResourceStatResult(ResourceStatTempData* resource_stat_temp);
	
	/**
	 * @brief 计算消息统计结果
	 * 
	 * 根据临时数据计算最终的消息统计结果
	 * 
	 * @param message_stat_temp 消息统计临时数据指针
	 */
	static void CalculateMessageStatResult(MessageStatTempData* message_stat_temp);

private:
	/** @brief 消息总数计数器 */
	uint32_t m_message_counts;
	/** @brief 失败消息总数计数器 */
	uint32_t m_failure_message_counts;
	/** @brief 资源统计最终结果存储 */
	ResourceStatResult m_resource_stat_result;
	/** @brief 消息统计最终结果存储 */
	MessageStatResult m_message_stat_result;

	/** @brief 资源统计临时数据类型定义 */
	typedef std::unordered_map<std::string, ResourceStatTempData> ResourceStatTemp;
	/** @brief 消息统计临时数据类型定义 */
	typedef std::unordered_map<std::string, MessageStatTempData> MessageStatTemp;
	/** @brief 资源统计临时数据存储 */
	ResourceStatTemp m_resource_stat_temp;
	/** @brief 消息统计临时数据存储 */
	MessageStatTemp m_message_stat_temp;
};

