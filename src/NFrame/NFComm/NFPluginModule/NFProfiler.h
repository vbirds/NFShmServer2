// -------------------------------------------------------------------------
//    @FileName         :    NFProfiler.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//    @Description      :    高性能代码性能分析器，提供函数调用时间统计和分析
//
// -------------------------------------------------------------------------

/**
 * @file NFProfiler.h
 * @brief 高性能代码性能分析器
 * @details 该文件实现了一个轻量级的性能分析工具，主要功能包括：
 * 
 * **核心功能：**
 * - **函数计时**：精确测量函数执行时间
 * - **调用统计**：统计函数调用次数和总时间
 * - **嵌套分析**：支持函数调用栈的嵌套分析
 * - **报告生成**：生成详细的性能分析报告
 * - **最小开销**：设计为最小化对程序性能的影响
 * 
 * **技术特点：**
 * - 使用高精度计时器（纳秒级）
 * - 支持多线程环境（主线程分析）
 * - 栈式管理函数调用层次
 * - 自动构建调用关系树
 * - 动态开关控制，支持运行时启用/禁用
 * 
 * **适用场景：**
 * - 性能瓶颈识别和分析
 * - 函数执行时间监控
 * - 代码优化效果验证
 * - 系统性能基准测试
 * - 实时性能监控
 * 
 * **使用模式：**
 * @code
 * // 全局分析器实例
 * NFProfiler profiler;
 * 
 * // 函数性能分析
 * PROFILE_TIMER timer("MyFunction");
 * profiler.BeginProfiler(&timer);
 * // ... 被分析的代码
 * profiler.EndProfiler();
 * 
 * // 生成报告
 * std::string report = profiler.OutputTopProfilerTimer();
 * @endcode
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

#pragma once

#include <time.h>
#include <string.h>
#include <string>
#include <vector>
#include "NFComm/NFCore/NFPlatform.h"
#include <map>
#include <atomic>

#ifdef _MSC_VER
#include <windows.h>
#endif

// ========================================================================
// 性能分析器配置常量
// ========================================================================

/// @brief 最大定时器数量限制，防止内存过度使用
const unsigned PROFILER_MAX_TIMER_COUNT = 4096;

/// @brief 最大调用栈深度，防止栈溢出
const unsigned PROFILER_MAX_STACK_LEVEL = 20;

/// @brief 定时器名称最大长度，确保字符串安全
const unsigned PROFILER_MAX_TIMER_NAME_LEN = 64;

/// @brief 表示无父节点的标识
const int PROFILER_NO_PARENT = -1;

/// @brief 表示多父节点的标识（用于调用树合并）
const int PROFILER_MULTI_PARENT = -2;

/**
 * @struct PROFILE_TIMER
 * @brief 性能分析定时器数据结构
 * @details 存储单个函数或代码块的性能统计信息。
 * 
 * **统计数据：**
 * - 调用次数和总时间
 * - 最小和最大执行时间
 * - 调用栈层次关系
 * - 执行上下文信息
 * 
 * **设计特点：**
 * - 紧凑的内存布局，减少缓存未命中
 * - 高精度时间统计（纳秒级）
 * - 支持嵌套调用分析
 * - 自动累计统计信息
 */
struct PROFILE_TIMER
{
	int index;                ///< 定时器在全局数组中的索引
	int parentIndex;          ///< 父定时器索引，用于构建调用树
	int level;                ///< 调用栈层级，用于报告格式化
	unsigned sampleCount;     ///< 采样次数（调用次数）
	long long beginTime;      ///< 当前调用的开始时间（纳秒）
	long long sampleTime;     ///< 累计执行时间（纳秒）
	long long minSampleTime;  ///< 最小执行时间（纳秒）
	long long maxSampleTime;  ///< 最大执行时间（纳秒）
	char name[PROFILER_MAX_TIMER_NAME_LEN];  ///< 定时器名称（函数名等）

    /**
     * @brief 带名称的构造函数
     * @param _name 定时器名称
     * 
     * 初始化定时器并设置名称，清除所有统计数据。
     */
	PROFILE_TIMER(const char* _name)
	{
		Clear();
		strncpy(name, _name, sizeof(name));
		name[sizeof(name) - 1] = '\0';
	}

    /**
     * @brief 默认构造函数
     * 
     * 创建空名称的定时器，后续可通过SetName设置名称。
     */
	PROFILE_TIMER() : PROFILE_TIMER("")
	{
	}

    /**
     * @brief 设置定时器名称
     * @param _name 新的定时器名称
     * 
     * 安全地设置定时器名称，确保不会超出缓冲区长度。
     */
	void SetName(const char* _name)
	{
		strncpy(name, _name, sizeof(name));
		name[sizeof(name) - 1] = '\0';
	}

    /**
     * @brief 清除统计数据
     * 
     * 重置所有计时和统计信息，保留名称不变。
     * 用于重新开始统计或重置分析器状态。
     */
	void Clear()
	{
		index = -1;
		parentIndex = PROFILER_NO_PARENT;
		level = -1;

		memset(&beginTime, 0, sizeof(beginTime));
		sampleCount = 0;
		sampleTime = 0;
        minSampleTime = -1;
        maxSampleTime = -1;
	}
};

/**
 * @struct CALL_TREE_NODE
 * @brief 调用关系树节点
 * @details 用于构建和表示函数调用关系的树形结构。
 * 
 * **树形结构：**
 * - 支持多叉树表示复杂调用关系
 * - 兄弟节点通过链表连接
 * - 父子关系明确，便于遍历
 * 
 * **统计汇总：**
 * - 聚合子函数的执行时间
 * - 计算调用频次和性能热点
 * - 支持性能瓶颈快速定位
 */
struct CALL_TREE_NODE
{
	const char* name;           ///< 节点名称（函数名）
	int parentIndex;            ///< 父节点索引
	unsigned sampleCount;       ///< 调用次数
	long long sampleTime;       ///< 总执行时间（纳秒）
	long long minSampleTime;    ///< 最小执行时间（纳秒）
	long long maxSampleTime;    ///< 最大执行时间（纳秒）

	CALL_TREE_NODE* firstChild; ///< 第一个子节点指针
	CALL_TREE_NODE* nextBrather; ///< 下一个兄弟节点指针
};

/**
 * @class NFProfiler
 * @brief 性能分析器主类
 * @details 提供完整的代码性能分析功能，支持函数级别的执行时间统计。
 * 
 * **核心特性：**
 * - **高精度计时**：纳秒级时间测量精度
 * - **低开销设计**：最小化对被测代码的性能影响
 * - **嵌套支持**：正确处理函数调用的嵌套关系
 * - **多线程友好**：支持主线程分析模式
 * - **动态控制**：运行时开启/关闭分析功能
 * 
 * **工作原理：**
 * 1. 使用栈结构追踪函数调用层次
 * 2. 记录每次函数调用的开始和结束时间
 * 3. 累计统计调用次数、总时间、最值等
 * 4. 构建调用关系树用于深度分析
 * 5. 生成可读的性能分析报告
 * 
 * **性能特点：**
 * - 分析开销通常 < 5% 原程序执行时间
 * - 支持大规模函数（数千个函数）
 * - 内存使用可控，避免无限增长
 * - 时间统计精度达到微秒级
 * 
 * **线程安全：**
 * @warning 当前实现主要针对单线程或主线程分析
 * @note 多线程支持需要额外的同步机制
 * 
 * **使用建议：**
 * - 开发阶段启用详细分析
 * - 生产环境可选择性启用
 * - 定期重置统计避免数据积累过多
 * - 结合业务场景解读分析结果
 */
class NFProfiler
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化性能分析器的所有内部数据结构。
     * 设置默认配置并准备开始性能分析。
     */
	NFProfiler()
	{
		mTimerCount = 0;
		mStackLevel = 0;
		mIsOpenProfiler = true;
		mProfileThreadID = 0;
		for (int i = 0; i < static_cast<int>(PROFILER_MAX_TIMER_COUNT); i++)
		{
			mTimers[i] = nullptr;
		}
		for (int i = 0; i < static_cast<int>(PROFILER_MAX_STACK_LEVEL); i++)
		{
			mStacks[i] = nullptr;
		}
	}

    /**
     * @brief 析构函数
     * 
     * 清理所有分配的资源，包括动态创建的定时器对象。
     * 确保不会有内存泄漏。
     */
	virtual ~NFProfiler()
	{
		for (auto iter = m_funcNameProfiler.begin(); iter != m_funcNameProfiler.end(); iter++)
		{
			NF_SAFE_DELETE(iter->second);
		}
		m_funcNameProfiler.clear();
	}

public:
    // ========================================================================
    // 核心分析接口
    // ========================================================================
    
    /**
     * @brief 开始性能分析（定时器对象版本）
     * @param timer 定时器对象指针，必须在整个分析期间保持有效
     * 
     * **功能说明：**
     * 开始对指定代码段进行性能分析，记录开始时间并更新调用栈。
     * 
     * **使用要求：**
     * - 定时器指针必须始终有效（建议使用静态变量）
     * - 必须与EndProfiler()成对使用
     * - 支持嵌套调用分析
     * 
     * **性能影响：**
     * - 调用开销通常 < 100纳秒
     * - 主要开销来自高精度时间获取
     * 
     * @warning 定时器对象必须在分析期间保持有效
     * @note 支持嵌套调用，自动管理调用栈
     */
	void BeginProfiler(PROFILE_TIMER* timer);
    
    /**
     * @brief 开始性能分析（Lua函数版本）
     * @param luaFunc Lua函数名称
     * 
     * **功能说明：**
     * 专门用于Lua脚本函数的性能分析，自动管理定时器对象。
     * 
     * **适用场景：**
     * - Lua脚本性能分析
     * - 动态函数名的性能统计
     * - 脚本层面的性能优化
     * 
     * **实现特点：**
     * - 自动创建和管理定时器对象
     * - 使用函数名作为唯一标识
     * - 支持动态函数名称
     */
	void BeginProfiler(const std::string& luaFunc);
    
    /**
     * @brief 结束性能分析
     * @return 本次调用的执行时间（微秒）
     * 
     * **功能说明：**
     * 结束当前函数的性能分析，计算执行时间并更新统计信息。
     * 
     * **统计更新：**
     * - 累计总执行时间
     * - 增加调用次数计数
     * - 更新最小/最大执行时间
     * - 恢复上层调用栈状态
     * 
     * **返回值：**
     * 返回本次调用的精确执行时间，便于实时监控。
     * 
     * @note 必须与BeginProfiler成对使用
     * @warning 调用顺序不匹配可能导致统计错误
     */
	uint64_t EndProfiler(); //return this time cost time(us)
    
    /**
     * @brief 设置分析器开关状态
     * @param b true启用分析，false禁用分析
     * 
     * **功能说明：**
     * 动态控制性能分析器的启用状态，便于运行时控制。
     * 
     * **使用场景：**
     * - 条件性能分析
     * - 减少生产环境开销
     * - 调试特定代码路径
     */
	void SetOpenProfiler(bool b) { mIsOpenProfiler = b; }
    
    /**
     * @brief 获取分析器开关状态
     * @return true表示已启用，false表示已禁用
     */
	bool IsOpenProfiler() const { return mIsOpenProfiler; }

public:
    // ========================================================================
    // 多线程支持接口
    // ========================================================================
    
    /**
     * @brief 设置分析器的目标线程ID
     * 
     * **功能说明：**
     * 设置当前线程为性能分析的目标线程，只有该线程的调用会被分析。
     * 
     * **多线程考虑：**
     * - 避免多线程间的数据竞争
     * - 确保分析结果的准确性
     * - 简化并发控制复杂度
     * 
     * @note 通常在主线程中调用此方法
     */
	void SetProfilerThreadID();
    
    /**
     * @brief 带线程检查的开始分析
     * @param timer 定时器对象指针
     * 
     * 只有在目标线程中才会执行实际的性能分析，
     * 其他线程的调用会被忽略。
     */
	void BeginProfilerWithThreadCheck(PROFILE_TIMER* timer);
    
    /**
     * @brief 带线程检查的结束分析
     * 
     * 对应BeginProfilerWithThreadCheck的结束调用。
     */
	void EndProfilerWithThreadCheck();
    
    /**
     * @brief 重置所有定时器统计数据
     * 
     * **功能说明：**
     * 清除所有已记录的性能统计数据，重新开始分析。
     * 
     * **使用场景：**
     * - 开始新一轮性能分析
     * - 清除历史数据的影响
     * - 重置基准测试环境
     */
	void ResetAllProfilerTimer();

public:
    // ========================================================================
    // 报告生成接口
    // ========================================================================
    
    /**
     * @brief 生成性能分析报告
     * @return 包含性能统计信息的格式化字符串
     * 
     * **功能说明：**
     * 分析所有收集的性能数据，生成易读的性能报告。
     * 
     * **报告内容：**
     * - 函数调用次数和总时间
     * - 平均、最小、最大执行时间
     * - 调用关系树和层次结构
     * - 性能热点和瓶颈分析
     * 
     * **报告格式：**
     * - 表格形式展示统计数据
     * - 树形结构显示调用关系
     * - 百分比显示相对性能开销
     * 
     * **使用建议：**
     * - 定期生成报告避免数据过多
     * - 结合具体业务场景分析
     * - 关注高频调用和高耗时函数
     */
	std::string OutputTopProfilerTimer();

protected:
    // ========================================================================
    // 内部实现方法
    // ========================================================================
    
    /**
     * @brief 按性能排序添加子节点
     * @param parent 父节点指针
     * @param child 要添加的子节点指针
     * 
     * 将子节点按照执行时间排序插入到父节点的子节点列表中。
     */
	void AddChildWithSort(CALL_TREE_NODE* parent, CALL_TREE_NODE* child);
    
    /**
     * @brief 构建调用关系树
     * @param head 树的根节点
     * @param callTree 存储树节点的向量
     * @return 构建是否成功
     * 
     * 根据收集的定时器数据构建完整的函数调用关系树。
     */
	bool BuildCallTree(CALL_TREE_NODE* head, std::vector<CALL_TREE_NODE>* callTree);
    
    /**
     * @brief 输出单个节点信息
     * @param node 要输出的节点
     * @param showSplitLine 是否显示分隔线
     * @param totalTime 总执行时间，用于计算百分比
     * @param level 缩进层级
     * @param report 输出报告字符串
     * 
     * 格式化输出单个节点的性能统计信息。
     */
	void OutputNode(const CALL_TREE_NODE& node, bool showSplitLine, long long totalTime, int level, std::string& report);
    
    /**
     * @brief 递归输出调用树
     * @param node 当前节点
     * @param totalTime 总时间
     * @param minShowTime 最小显示时间阈值
     * @param level 当前层级
     * @param report 输出报告字符串
     * 
     * 递归遍历调用树，输出完整的性能分析报告。
     */
	void OutputCallTree(const CALL_TREE_NODE& node, long long totalTime, long long minShowTime, int level, std::string& report);

private:
	atomic_bool mIsOpenProfiler;           ///< 分析器开关状态（原子变量，线程安全）
	NF_THREAD_ID mProfileThreadID;         ///< 目标分析线程ID

private:
	unsigned mTimerCount;                                          ///< 当前定时器数量
	PROFILE_TIMER* mTimers[PROFILER_MAX_TIMER_COUNT];             ///< 定时器对象数组

	unsigned mStackLevel;                                          ///< 当前调用栈深度
	PROFILE_TIMER* mStacks[PROFILER_MAX_STACK_LEVEL];             ///< 调用栈数组

	std::map<std::string, PROFILE_TIMER*> m_funcNameProfiler;     ///< 函数名到定时器的映射表
};



