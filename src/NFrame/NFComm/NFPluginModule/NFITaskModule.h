// -------------------------------------------------------------------------
//    @FileName         :    NFITaskModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFITaskModule
//    @Description      :    任务模块接口定义，提供Actor模式的多线程任务处理系统
//                           支持任务分发、Actor管理、组件系统和线程池管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIModule.h"

#include <vector>

class NFTaskActorMessage;
class NFTask;
class NFITaskComponent;

/**
 * @brief 任务模块的线程让出策略枚举
 * 
 * 定义了当没有工作可做时，工作线程的让出策略。
 * 不同策略影响CPU使用率和响应延迟的平衡。
 */
enum TaskModule_YieldStrategy
{
	TaskModule_YIELD_STRATEGY_CONDITION = 0,       ///< 线程在没有工作时等待条件变量（低CPU占用）
	TaskModule_YIELD_STRATEGY_HYBRID,              ///< 混合策略：先短暂自旋，然后让出给其他线程
	TaskModule_YIELD_STRATEGY_SPIN,                ///< 线程忙等待，不让出CPU（低延迟，高CPU占用）

	// 遗留兼容性定义
	TaskModule_YIELD_STRATEGY_BLOCKING = 0,        ///< 已弃用 - 使用YIELD_STRATEGY_CONDITION
	TaskModule_YIELD_STRATEGY_POLITE = 0,          ///< 已弃用 - 使用YIELD_STRATEGY_CONDITION
	TaskModule_YIELD_STRATEGY_STRONG = 1,          ///< 已弃用 - 使用YIELD_STRATEGY_HYBRID
	TaskModule_YIELD_STRATEGY_AGGRESSIVE = 2       ///< 已弃用 - 使用YIELD_STRATEGY_SPIN
};

/**
 * @brief 任务组相关常量定义
 */
enum
{
    NF_TASK_GROUP_DEFAULT = 0,        ///< 默认任务组ID
    NF_TASK_MAX_GROUP_DEFAULT = 1,    ///< 默认最大任务组数量
};

/**
 * @brief 任务模块接口类
 * 
 * NFITaskModule 提供了基于Actor模式的多线程任务处理系统：
 * 
 * 1. 线程池管理：
 *    - Actor线程池的初始化和配置
 *    - 多任务组的线程管理
 *    - 线程让出策略的配置
 * 
 * 2. Actor管理：
 *    - Actor的创建和分配
 *    - Actor ID的管理和查询
 *    - Actor组件系统
 * 
 * 3. 任务分发：
 *    - 任务到Actor的分发机制
 *    - 广播任务到所有Actor
 *    - 消息传递系统
 * 
 * 4. 状态监控：
 *    - 队列中未处理任务数量统计
 *    - 线程使用情况监控
 * 
 * 5. 组件系统：
 *    - Actor组件的添加和管理
 *    - 组件生命周期管理
 * 
 * 设计特点：
 * - 基于Actor模式的无锁并发
 * - 支持多种线程让出策略
 * - 灵活的任务组管理
 * - 完善的组件系统
 * - 高效的消息传递
 * 
 * 使用场景：
 * - 高并发任务处理
 * - 异步数据库操作
 * - 网络I/O处理
 * - CPU密集型计算
 * - 消息队列处理
 */
class NFITaskModule : public NFIModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFITaskModule(NFIPluginManager* p) :NFIModule(p)
	{

	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构，清理所有线程和任务资源。
	 */
	virtual ~NFITaskModule()
	{

	}
	
	/**
	 * @brief 初始化Actor线程系统
	 * @param actorGroup 任务组ID，用于区分不同的任务组
	 * @param thread_num 线程数目，至少为1，建议不超过CPU核心数
	 * @param yieldstrategy 线程让出策略，见TaskModule_YieldStrategy枚举，默认为0
	 * @return 初始化成功返回0或正值，失败返回负值
	 * 
	 * 初始化指定任务组的Actor线程池，配置线程数量和让出策略。
	 * 不同的让出策略适用于不同的应用场景：
	 * - CONDITION: 适用于任务较少，注重节能的场景
	 * - HYBRID: 适用于任务量中等，平衡延迟和CPU使用率
	 * - SPIN: 适用于高频任务，要求极低延迟的场景
	 * 
	 * 使用示例：
	 * @code
	 * // 初始化4个线程的任务组，使用混合策略
	 * int result = InitActorThread(0, 4, TaskModule_YIELD_STRATEGY_HYBRID);
	 * if (result >= 0) {
	 *     // 初始化成功
	 * }
	 * @endcode
	 */
	virtual int InitActorThread(int actorGroup, int thread_num, int yieldstrategy = 0) = 0;

	/**
	 * @brief 添加任务到指定任务组
	 * @param actorGroup 任务组ID
	 * @param pTask 要异步处理的任务对象指针
	 * @return 添加成功返回0或正值，失败返回负值
	 * 
	 * 将任务添加到指定任务组中，系统会自动选择合适的Actor来处理该任务。
	 * 任务将在Actor线程中异步执行。
	 */
	virtual int AddTask(int actorGroup, NFTask* pTask) = 0;

	/**
	 * @brief 向任务组中的每个Actor添加相同的任务
	 * @tparam NFTaskType 任务类型，必须是NFTask的派生类
	 * @param actorGroup 任务组ID
	 * @param task 要分发的任务对象（将被复制到每个Actor）
	 * @return 分发成功返回0，失败返回非0值
	 * 
	 * 这是一个模板函数，用于向指定任务组中的所有Actor广播相同的任务。
	 * 每个Actor将收到该任务的一个副本并独立执行。
	 * 
	 * 适用场景：
	 * - 配置更新通知
	 * - 全局状态同步
	 * - 统计信息收集
	 * 
	 * 使用示例：
	 * @code
	 * ConfigUpdateTask configTask;
	 * configTask.SetNewConfig(newConfig);
	 * AddTaskToEveryActor(0, configTask);
	 * @endcode
	 * 
	 * @note 该函数会为每个Actor创建任务的独立副本
	 * @note 任务对象必须支持拷贝构造
	 */
	template<typename NFTaskType>
	int AddTaskToEveryActor(int actorGroup, const NFTaskType& task)
	{
		std::vector<int> vecActorId = GetAllActorId(actorGroup);
		for (size_t i = 0; i < vecActorId.size(); i++)
		{
			auto pTask = new NFTaskType();
			*pTask = task;
			SendMsgToActor(vecActorId[i], pTask);
		}
		return 0;
	}

	/**
	 * @brief 向指定Actor发送消息
	 * @param actorGroup 任务组ID
	 * @param nActorIndex Actor的唯一索引
	 * @param pData 要发送给Actor的任务数据
	 * @return 发送成功返回0或正值，失败返回负值
	 * 
	 * 主线程通过保存的Actor索引将数据发送给指定的Actor线程。
	 * 这提供了精确的任务路由控制，适用于需要特定Actor处理的场景。
	 * 
	 * 使用示例：
	 * @code
	 * // 向特定Actor发送任务
	 * auto pTask = new SpecialTask();
	 * int result = SendMsgToActor(0, actorIndex, pTask);
	 * @endcode
	 */
	virtual int SendMsgToActor(int actorGroup, int nActorIndex, NFTask* pData) = 0;

	/**
	 * @brief 添加任务到指定Actor
	 * @param actorGroup 任务组ID
	 * @param actorId Actor ID
	 * @param pTask 要异步处理的任务对象指针
	 * @return 添加成功返回0或正值，失败返回负值
	 * 
	 * 将任务直接分配给指定的Actor进行处理，提供精确的任务控制。
	 * 适用于需要特定Actor处理或负载均衡的场景。
	 */
	virtual int AddTask(int actorGroup, int actorId, NFTask* pTask) = 0;

	/**
	 * @brief 请求分配一个新的Actor
	 * @param actorGroup 任务组ID
	 * @return 返回新分配的Actor的唯一索引，失败返回负值
	 * 
	 * 向系统请求分配一个新的Actor用于任务处理。
	 * 返回的索引可用于后续的任务分配和消息发送。
	 * 
	 * 使用示例：
	 * @code
	 * int actorId = RequireActor(0);
	 * if (actorId > 0) {
	 *     // 使用actorId进行任务分配
	 *     AddTask(0, actorId, pTask);
	 * }
	 * @endcode
	 */
	virtual int RequireActor(int actorGroup) = 0;

	/**
	 * @brief 获取队列中未处理的任务数量
	 * @return 返回系统中所有未完成的任务总数
	 * 
	 * 获取当前系统中排队等待处理的任务数量，用于：
	 * - 系统负载监控
	 * - 性能调优参考
	 * - 流控决策依据
	 * 
	 * @note 返回值包括所有任务组中的任务总数
	 */
	virtual int GetNumQueuedMessages() = 0;

	/**
	 * @brief 为Actor添加组件
	 * @param actorGroup 任务组ID
	 * @param nActorIndex Actor索引
	 * @param pComonnet 要添加的组件指针
	 * @return 添加成功返回0或正值，失败返回负值
	 * 
	 * 为指定的Actor添加功能组件，扩展Actor的处理能力。
	 * 组件可以包含特定的业务逻辑、状态管理或数据处理功能。
	 * 
	 * 使用示例：
	 * @code
	 * auto pComponent = new DatabaseComponent();
	 * int result = AddActorComponent(0, actorIndex, pComponent);
	 * @endcode
	 */
	virtual int AddActorComponent(int actorGroup, int nActorIndex, NFITaskComponent* pComonnet) = 0;

	/**
	 * @brief 获取Actor的组件
	 * @param actorGroup 任务组ID
	 * @param nActorIndex Actor索引
	 * @return 返回Actor关联的组件指针，未找到返回nullptr
	 * 
	 * 获取指定Actor关联的组件实例，用于访问组件提供的功能。
	 * 组件提供了Actor的扩展能力和状态管理。
	 */
	virtual NFITaskComponent* GetTaskComponent(int actorGroup, int nActorIndex) = 0;

	/**
	 * @brief 获取任务组的最大线程数
	 * @param actorGroup 任务组ID
	 * @return 返回指定任务组配置的最大线程数量
	 * 
	 * 获取指定任务组中配置的最大工作线程数量，
	 * 用于系统监控和资源规划。
	 */
	virtual int GetMaxThreads(int actorGroup) = 0;

	/**
	 * @brief 获取任务组中所有Actor的ID列表
	 * @param actorGroup 任务组ID
	 * @return 返回包含所有Actor ID的向量
	 * 
	 * 获取指定任务组中所有已分配Actor的ID列表，
	 * 用于遍历所有Actor或进行批量操作。
	 * 
	 * 使用示例：
	 * @code
	 * std::vector<int> actorIds = GetAllActorId(0);
	 * for (int actorId : actorIds) {
	 *     // 对每个Actor执行操作
	 * }
	 * @endcode
	 */
	virtual std::vector<int> GetAllActorId(int actorGroup) const = 0;
};
