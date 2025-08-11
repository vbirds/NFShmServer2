// -------------------------------------------------------------------------
//    @FileName         :    NFTaskGroup.h
//    @Author           :    gaoyi
//    @Date             :    23-9-14
//    @Email			:    445267987@qq.com
//    @Module           :    NFTaskGroup
//    @Description      :    任务组头文件，基于Theron框架的Actor模式任务处理系统
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFAtomic.h"
#include "NFComm/NFCore/NFQueue.hpp"
#include "NFComm/NFPluginModule/NFIModule.h"
#include <Theron/Theron.h>

class NFTask;
class NFTaskActor;
class NFITaskComponent;
class NFTaskActorMessage;

/**
 * @class NFTaskGroup
 * @brief 任务组实现类
 *
 * NFTaskGroup基于Theron Actor框架实现的任务处理组，提供了完整的Actor模式异步任务处理：
 * 
 * Actor模式核心：
 * - Theron框架：基于成熟的Actor框架实现
 * - 消息传递：Actor间通过消息进行通信
 * - 无共享状态：避免线程间的状态共享和锁竞争
 * - 异步处理：非阻塞的任务执行和结果返回
 * 
 * 任务分发策略：
 * - 负载均衡：基于balanceId的一致性哈希分配
 * - 随机分配：支持随机选择Actor处理任务
 * - 指定Actor：允许将任务分配给特定Actor
 * - 组件扩展：Actor可挂载不同的功能组件
 * 
 * 生命周期管理：
 * - 动态创建：运行时动态创建和配置Actor
 * - 资源管理：自动管理Actor的生命周期
 * - 优雅关闭：确保所有任务完成后安全关闭
 * - 任务监控：实时监控任务执行状态和超时
 * 
 * 高级特性：
 * - 超时检测：检测和处理长时间运行的任务
 * - 任务统计：统计任务执行性能和成功率
 * - 错误隔离：单个Actor的错误不影响其他Actor
 * - 热重载：支持Actor代码的热更新
 * 
 * 应用场景：
 * - 数据库操作：避免阻塞主线程的数据库I/O
 * - 网络请求：异步HTTP/TCP请求处理
 * - 文件操作：大文件读写和处理
 * - 计算密集任务：复杂算法和数据处理
 * - 游戏逻辑：玩家AI、战斗计算等
 * 
 * @note 基于Theron框架，提供工业级的Actor实现
 * @note 支持跨平台和分布式扩展
 * @warning Actor处理函数不应有阻塞操作，以免影响整体性能
 */
class NFTaskGroup : public NFIModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    NFTaskGroup(NFIPluginManager *p);

    /**
     * @brief 析构函数
     */
    virtual ~NFTaskGroup();

public:
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
     * 
     * 处理Actor返回的消息，检查任务超时等。
     */
    int Tick() override;

public:
    /**
     * @brief 初始化Actor线程系统
     * @param taskGroup 任务组ID，用于标识不同的任务组
     * @param thread_num 线程数量，至少为1
     * @param yieldstrategy 让出策略，默认为0
     * @return 返回0表示成功，小于0表示失败
     * 
     * 配置Actor线程池的大小和调度策略。
     */
    virtual int InitActorThread(int taskGroup, int thread_num, int yieldstrategy = 0);

    /**
     * @brief 申请一个Actor
     * @return 返回Actor的唯一索引，失败返回-1
     * 
     * 从Actor池中获取一个可用的Actor用于处理任务。
     */
    virtual int RequireActor();

    /**
     * @brief 为Actor添加组件
     * @param nActorIndex Actor索引
     * @param pComponent 要添加的组件对象
     * @return 返回0表示成功，非0表示失败
     * 
     * 为指定Actor添加功能组件，扩展其处理能力。
     */
    virtual int AddActorComponent(int nActorIndex, NFITaskComponent* pComponent);

    /**
     * @brief 获取Actor的组件
     * @param nActorIndex Actor索引
     * @return 返回组件指针，不存在返回nullptr
     * 
     * 获取指定Actor关联的组件对象。
     */
    virtual NFITaskComponent* GetTaskComponent(int nActorIndex);

    /**
     * @brief 发送消息到指定Actor
     * @param nActorIndex Actor索引
     * @param pData 要发送的任务数据
     * @return 返回0表示成功，非0表示失败
     * 
     * 主线程通过Actor索引将任务数据发送给对应的Actor线程处理。
     */
    virtual int SendMsgToActor(int nActorIndex, NFTask* pData);

    /**
     * @brief 发送消息到指定Actor（直接传递Actor对象）
     * @param pActor Actor对象指针
     * @param pData 要发送的任务数据
     * @return 返回0表示成功，非0表示失败
     * 
     * 通过Actor对象直接发送任务，避免索引查找开销。
     */
    virtual int SendMsgToActor(NFTaskActor* pActor, NFTask* pData);

    /**
     * @brief 释放Actor资源
     * 
     * 释放所有Actor相关的资源，用于模块关闭时的清理。
     */
    virtual void ReleaseActor();

    /**
     * @brief 通过索引获取Actor对象
     * @param nActorIndex Actor索引
     * @return 返回Actor对象指针，不存在返回nullptr
     * 
     * 根据Actor索引获取对应的Actor对象。
     */
    virtual NFTaskActor* GetActor(int nActorIndex);

    /**
     * @brief 获取未完成任务数量
     * @return 返回系统中排队等待处理的任务数量
     * 
     * 用于监控系统负载和性能分析。
     */
    virtual int GetNumQueuedMessages();

    /**
     * @brief 关闭Actor池
     * @return 返回0表示成功，非0表示失败
     * 
     * 安全关闭所有Actor线程，等待当前任务完成。
     */
    virtual int CloseActorPool();

    /**
     * @brief 处理Actor返回的消息
     * @param message 消息数据
     * @param from 发送消息的Actor地址
     * @return 返回true表示成功，false表示失败
     * 
     * 当Actor处理完任务需要将结果返回给主线程时，
     * 在这个函数里将消息数据推送给主线程。
     * 
     * @note 此函数在Actor线程中执行
     */
    virtual bool HandlerEx(const NFTaskActorMessage& message, int from);

    /**
     * @brief 通过负载均衡ID获取Actor
     * @param balanceId 负载均衡ID，通常是数据记录的主键
     * @return 返回Actor索引
     * 
     * 为了防止数据库操作冲突，使用一致性哈希算法根据balanceId
     * 确保对同一数据记录的操作总是分配到同一个Actor处理。
     * 
     * @note 这是数据一致性的重要保证机制
     */
    int GetBalanceActor(uint64_t balanceId);

    /**
     * @brief 随机获取一个Actor
     * @return 返回随机选择的Actor索引
     * 
     * 适用于无状态任务的随机负载分配。
     */
    int GetRandActor();

    /**
     * @brief 添加异步任务（自动分配Actor）
     * @param pTask 要异步处理的任务对象
     * @return 返回0表示成功，非0表示失败
     * 
     * 系统自动选择合适的Actor来处理任务。
     */
    virtual int AddTask(NFTask* pTask);

    /**
     * @brief 添加异步任务（指定Actor）
     * @param actorId 指定的Actor ID
     * @param pTask 要异步处理的任务对象
     * @return 返回0表示成功，非0表示失败
     * 
     * 将任务分配给指定的Actor处理。
     */
    virtual int AddTask(int actorId, NFTask* pTask);

    /**
     * @brief 主线程处理Actor返回的任务
     * 
     * 在主线程的Tick中调用，处理Actor完成的任务结果。
     */
    void OnMainThreadTick();

    /**
     * @brief 获取最大Actor线程数
     * @return 返回当前配置的最大线程数
     */
    virtual int GetMaxThreads();

    /**
     * @brief 记录监控任务
     * @param pTask 要监控的任务对象
     * 
     * 将任务加入监控列表，用于超时检测和性能统计。
     */
    virtual void MonitorTask(NFTask* pTask);

    /**
     * @brief 检查超时任务
     * 
     * 检查是否有任务执行超时，进行相应的处理。
     */
    virtual void CheckTimeOutTask();

    /**
     * @brief 获取所有Actor ID列表
     * @return 返回包含所有Actor ID的向量
     * 
     * 用于遍历和管理Actor集合。
     */
    virtual std::vector<int> GetAllActorId() const;

protected:
    /**
     * @brief Theron框架实例
     * 
     * 底层的Theron Actor框架，负责Actor的创建、调度和消息传递。
     */
    Theron::Framework* m_pFramework;
    
    /**
     * @brief 主Actor实例
     * 
     * 主线程的Actor实例，用于接收其他Actor返回的消息。
     */
    NFTaskActor* m_pMainActor;

protected:
    /**
     * @brief Actor索引数组
     * 
     * 存储所有创建的Actor对象指针，通过索引进行快速访问。
     */
    std::vector<NFTaskActor*> m_vecActorPool;

    /**
     * @brief 负载均衡索引
     * 
     * 原子变量，用于随机选择Actor时的负载均衡，
     * 避免总是选择相同的Actor。
     */
    std::atomic<uint32_t> mnSuitIndex;

    /**
     * @brief 返回消息队列
     * 
     * 线程安全的消息队列，Actor线程将完成的任务结果放入队列，
     * 主线程从队列中取出数据进行处理。
     */
    NFQueueVector<NFTaskActorMessage> m_mQueue;

    /**
     * @brief 任务组ID
     * 
     * 标识当前任务组的唯一ID，用于区分不同的任务组。
     */
    uint32_t m_taskGroupId;
};