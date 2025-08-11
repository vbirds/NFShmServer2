// -------------------------------------------------------------------------
//    @FileName         :    NFCTaskModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCTaskModule
//    @Desc             :    任务模块头文件，提供Actor模式的异步任务处理系统。
//                          该文件定义了NFShmXFrame框架的任务模块，基于Actor模型实现高性能异步任务处理系统，
//                          包括Actor线程池、消息传递、任务管理、负载均衡、任务组管理、组件系统等功能。
//                          主要功能包括动态负载均衡、线程池配置、零拷贝消息传递、NUMA感知调度、
//                          缓存友好设计、内存池管理、原子操作保证数据一致性
//    @Description      :    任务模块头文件，提供Actor模式的异步任务处理系统
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFITaskModule.h"
#include "NFComm/NFCore/NFQueue.hpp"
#include "NFTaskActor.h"

#include "NFComm/NFCore/NFMutex.h"
#include "NFComm/NFCore/NFLock.h"
#include "NFComm/NFCore/NFAtomic.h"
#include <map>

class NFTaskGroup;

/**
 * @class NFCTaskModule
 * @brief 任务模块实现类
 *
 * NFCTaskModule基于Actor模型实现的高性能异步任务处理系统：
 * 
 * Actor模型核心：
 * - Actor线程池：独立的工作线程处理任务
 * - 消息传递：线程间通过消息队列通信
 * - 无锁设计：减少锁竞争，提高并发性能
 * - 任务隔离：每个Actor独立处理任务，避免相互影响
 * 
 * 任务管理功能：
 * - 动态负载均衡：根据任务特征智能分配Actor
 * - 任务组管理：支持多个独立的Actor组
 * - 组件系统：Actor可挂载多个功能组件
 * - 任务队列：高效的无锁任务队列实现
 * 
 * 高级特性：
 * - 线程池配置：可配置Actor线程数量和调度策略
 * - 负载均衡：基于balanceId的一致性哈希分配
 * - 随机分配：支持随机选择Actor处理任务
 * - 任务监控：实时统计未完成任务数量
 * - 优雅关闭：支持Actor池的安全关闭
 * 
 * 应用场景：
 * - 数据库操作：避免阻塞主线程的数据库I/O
 * - 文件处理：大文件读写和图片处理
 * - 网络请求：HTTP/TCP客户端请求处理
 * - CPU密集任务：复杂计算和数据分析
 * - 游戏逻辑：玩家状态计算和AI处理
 * 
 * 性能优化：
 * - 零拷贝消息传递：高效的任务数据传输
 * - NUMA感知调度：针对多核架构优化
 * - 缓存友好设计：减少CPU缓存失效
 * - 内存池管理：复用任务对象减少分配开销
 * 
 * 线程安全：
 * - 原子操作：使用原子变量保证数据一致性
 * - 无锁队列：高性能的无锁消息队列
 * - 线程隔离：每个Actor独立运行，避免竞争
 * - 安全关闭：保证所有任务完成后才关闭
 * 
 * @note Actor模型天然支持分布式扩展
 * @note 适用于CPU密集型和I/O密集型任务
 * @warning 任务执行时间不宜过长，避免阻塞Actor线程
 */
class NFCTaskModule final : public NFITaskModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFCTaskModule(NFIPluginManager* p);
	
	/**
	 * @brief 析构函数
	 */
	virtual ~NFCTaskModule();

	/**
	 * @brief 模块觉醒初始化
	 * @return 返回0表示成功，非0表示失败
	 */
	int Awake() override;

	/**
	 * @brief 模块初始化
	 * @return 返回0表示成功，非0表示失败
	 */
	int Init() override;

	/**
	 * @brief 关闭前处理
	 * @return 返回0表示成功，非0表示失败
	 */
	int BeforeShut() override;

	/**
	 * @brief 关闭模块
	 * @return 返回0表示成功，非0表示失败
	 */
	int Shut() override;

	/**
	 * @brief 最终化处理
	 * @return 返回0表示成功，非0表示失败
	 */
	int Finalize() override;

	/**
	 * @brief 模块定时更新
	 * @return 返回0表示成功，非0表示失败
	 */
	int Tick() override;

public:
	/**
	 * @brief 初始化Actor线程池
	 * @param actorGroup Actor组ID，用于区分不同的任务类型
	 * @param thread_num 线程数量，至少为1
	 * @param yieldstrategy 让出策略，默认为0
	 * @return 返回0表示成功，小于0表示失败
	 * 
	 * 配置指定Actor组的线程数量和调度策略。
	 * 不同组可以有不同的线程配置以适应不同类型的任务。
	 */
	virtual int InitActorThread(int actorGroup, int thread_num, int yieldstrategy = 0);

	/**
	 * @brief 申请一个Actor
	 * @param actorGroup Actor组ID
	 * @return 返回Actor的唯一索引，失败返回-1
	 * 
	 * 从指定组中申请一个可用的Actor用于处理任务。
	 */
	virtual int RequireActor(int actorGroup) override;

	/**
	 * @brief 为Actor添加组件
	 * @param actorGroup Actor组ID
	 * @param nActorIndex Actor索引
	 * @param pComponent 要添加的组件对象
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 为指定Actor添加功能组件，扩展Actor的处理能力。
	 */
	virtual int AddActorComponent(int actorGroup, int nActorIndex, NFITaskComponent* pComponent) override;

	/**
	 * @brief 获取Actor的组件
	 * @param actorGroup Actor组ID
	 * @param nActorIndex Actor索引
	 * @return 返回组件指针，不存在返回nullptr
	 * 
	 * 获取指定Actor关联的组件对象。
	 */
	virtual NFITaskComponent* GetTaskComponent(int actorGroup, int nActorIndex) override;

	/**
	 * @brief 发送消息到指定Actor
	 * @param actorGroup Actor组ID
	 * @param nActorIndex Actor索引
	 * @param pData 要发送的任务数据
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 主线程通过Actor索引将任务数据发送给对应的Actor线程处理。
	 */
	virtual int SendMsgToActor(int actorGroup, int nActorIndex, NFTask* pData);

	/**
	 * @brief 发送消息到指定Actor（直接传递Actor对象）
	 * @param actorGroup Actor组ID
	 * @param pActor Actor对象指针
	 * @param pData 要发送的任务数据
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 通过Actor对象直接发送任务，避免索引查找开销。
	 */
	virtual int SendMsgToActor(int actorGroup, NFTaskActor* pActor, NFTask* pData);

	/**
	 * @brief 释放Actor资源
	 * 
	 * 释放所有Actor相关的资源，用于模块关闭时的清理。
	 */
	virtual void ReleaseActor();

	/**
	 * @brief 通过索引获取Actor对象
	 * @param actorGroup Actor组ID
	 * @param nActorIndex Actor索引
	 * @return 返回Actor对象指针，不存在返回nullptr
	 * 
	 * 根据Actor索引获取对应的Actor对象。
	 */
	virtual NFTaskActor* GetActor(int actorGroup, int nActorIndex);

	/**
	 * @brief 获取未完成任务数量
	 * @return 返回系统中排队等待处理的任务数量
	 * 
	 * 用于监控系统负载和性能分析。
	 */
	virtual int GetNumQueuedMessages() override;

	/**
	 * @brief 关闭Actor池
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 安全关闭所有Actor线程，等待当前任务完成。
	 */
	virtual int CloseActorPool();

	/**
	 * @brief 通过负载均衡ID获取Actor
	 * @param actorGroup Actor组ID
	 * @param balanceId 负载均衡ID，通常是数据记录的主键
	 * @return 返回Actor索引
	 * 
	 * 为了防止数据库操作冲突，使用一致性哈希算法根据balanceId
	 * 确保对同一数据记录的操作总是分配到同一个Actor处理。
	 * 
	 * @note 这是数据一致性的重要保证机制
	 */
	int GetBalanceActor(int actorGroup, uint64_t balanceId);

	/**
	 * @brief 随机获取一个Actor
	 * @param actorGroup Actor组ID
	 * @return 返回随机选择的Actor索引
	 * 
	 * 适用于无状态任务的随机负载分配。
	 */
	int GetRandActor(int actorGroup);

	/**
	 * @brief 添加异步任务（自动分配Actor）
	 * @param actorGroup Actor组ID
	 * @param pTask 要异步处理的任务对象
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 系统自动选择合适的Actor来处理任务。
	 */
	virtual int AddTask(int actorGroup, NFTask* pTask) override;

	/**
	 * @brief 添加异步任务（指定Actor）
	 * @param actorGroup Actor组ID
	 * @param actorId 指定的Actor ID
	 * @param pTask 要异步处理的任务对象
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 将任务分配给指定的Actor处理。
	 */
	virtual int AddTask(int actorGroup, int actorId, NFTask* pTask) override;

	/**
	 * @brief 获取最大Actor线程数
	 * @param actorGroup Actor组ID
	 * @return 返回指定组的最大线程数
	 */
	virtual int GetMaxThreads(int actorGroup) override;

	/**
	 * @brief 获取所有Actor ID列表
	 * @param actorGroup Actor组ID
	 * @return 返回包含所有Actor ID的向量
	 * 
	 * 用于遍历和管理Actor集合。
	 */
	virtual std::vector<int> GetAllActorId(int actorGroup) const override;

protected:
	/**
	 * @brief 任务组列表
	 * 
	 * 存储所有的Actor组，每个组包含一组相同配置的Actor线程。
	 * 支持多个独立的任务组，不同组可以有不同的线程配置和调度策略。
	 */
    std::vector<NFTaskGroup*> m_taskGroups;
};
