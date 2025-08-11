// -------------------------------------------------------------------------
//    @FileName         :    NFCoroutineSchedule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCoroutineSchedule
//    @Description      :    协程任务调度器头文件，提供高级协程任务管理和超时控制功能
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFCoroutine.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"
#include <unordered_map>
#include <set>

class NFSchedule;

#ifdef Yield
#undef Yield
#endif

class NFCoroutineTask;
class NFCoroutineSchedule;

/**
 * @enum OnTimerCallbackReturnCode
 * @brief 定时器超时回调函数返回码定义
 */
typedef enum {
    kTIMER_BE_REMOVED = -1,     ///< 超时后定时器被停止并移除
    kTIMER_BE_CONTINUED = 0,    ///< 超时后定时器仍然保持，重新计时
    kTIMER_BE_RESETED           ///< 超时后定时器仍然保持，使用返回值作为超时时间(ms)重新计时
} OnTimerCallbackReturnCode;

/**
 * @class NFCoroutineTaskTimer
 * @brief 协程任务定时器类
 *
 * 专门为协程任务设计的定时器，主要用于协程的超时控制：
 * 
 * 超时控制功能：
 * - Yield超时：协程挂起超时自动恢复
 * - 任务超时：长时间运行任务的超时检测
 * - 自动清理：定时器到期后的自动清理
 * - 状态管理：定时器的生命周期管理
 * 
 * @note 与NFCoroutineSchedule紧密配合使用
 */
class NFCoroutineTaskTimer : public NFTimerObj {
    /**
     * @enum EnumNFCoroutineTaskTimer
     * @brief 协程任务定时器类型枚举
     */
    enum EnumNFCoroutineTaskTimer {
        ENUM_NF_COROUTINE_TASK_TIMER_YIELD = 1, ///< Yield超时定时器
    };

public:
    /**
     * @brief 构造函数
     * @param pCoSche 协程调度器指针
     * @param taskId 关联的任务ID
     */
    NFCoroutineTaskTimer(NFCoroutineSchedule* pCoSche, int64_t taskId);
    
    /**
     * @brief 析构函数
     */
    virtual ~NFCoroutineTaskTimer();

    /**
     * @brief 设置Yield超时时间
     * @param timeout_ms 超时时间（毫秒），-1表示无超时
     * 
     * 设置协程挂起的超时时间，超时后会自动恢复协程执行。
     */
    virtual void SetYieldTimeout(int32_t timeout_ms = -1);
    
    /**
     * @brief 定时器回调处理
     * @param nTimerID 定时器ID
     * @return 返回定时器处理结果
     */
    virtual int OnTimer(uint32_t nTimerID);
    
    /**
     * @brief 标记定时器为删除状态
     */
    virtual void SetDelete();
    
    /**
     * @brief 检查定时器是否被标记为删除
     * @return 返回true表示已标记删除，false表示正常状态
     */
	virtual bool IsDelete() const { return m_delete; }

private:
    NFCoroutineSchedule* m_pCoSche;     ///< 协程调度器指针
    int64_t m_taskId;                   ///< 关联的任务ID
    bool m_delete;                      ///< 删除标记
};

/**
 * @class NFCoroutineSchedule
 * @brief 协程任务调度器类
 *
 * NFCoroutineSchedule是高级协程调度器，在NFSchedule基础上提供了任务级别的协程管理：
 * 
 * 任务管理功能：
 * - 协程任务：以任务为单位管理协程
 * - 预启动队列：支持协程的预启动机制
 * - 任务映射：维护任务ID到协程的映射关系
 * - 生命周期：完整的任务生命周期管理
 * 
 * 超时控制功能：
 * - Yield超时：协程挂起超时自动恢复
 * - 超时回调：灵活的超时处理机制
 * - 定时器管理：集成的定时器系统
 * - 超时策略：支持多种超时处理策略
 * 
 * 高级特性：
 * - 用户数据：任务可携带protobuf数据
 * - 状态查询：实时查询任务执行状态
 * - 优雅关闭：服务停止时的任务清理
 * - 调试支持：任务执行的调试和监控
 * 
 * 性能优化：
 * - 任务池：复用已结束的任务对象
 * - 批量调度：批量处理协程状态变化
 * - 内存管理：优化的内存分配策略
 * - 快速查找：基于哈希表的任务查找
 * 
 * 应用场景：
 * - 异步业务逻辑：复杂的异步业务流程
 * - 网络请求：超时控制的网络操作
 * - 数据处理：长时间运行的数据处理任务
 * - 游戏逻辑：游戏中的状态机和流程控制
 * 
 * @note 与NFCCoroutineModule是友元类
 * @note 继承自NFTimerObj，具有定时器功能
 * @warning 任务执行时间不宜过长，避免阻塞调度器
 */
class NFCoroutineSchedule : public NFTimerObj {
    friend class NFCCoroutineModule;

public:
    /**
     * @brief 构造函数
     * @param pPluginManager 插件管理器指针
     */
    explicit NFCoroutineSchedule(NFIPluginManager* pPluginManager);

    /**
     * @brief 析构函数
     */
    virtual ~NFCoroutineSchedule();

	/**
	 * @brief 定时器回调处理
	 * @param nTimerID 定时器ID
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int OnTimer(uint32_t nTimerID);

	/**
	 * @brief 清理所有定时器
	 * 
	 * 清理调度器相关的所有定时器资源。
	 */
	virtual void ClearTimer();

    /**
     * @brief 检查服务器停止状态
     * @return 返回0表示可以停止，非0表示还有任务在运行
     * 
     * 检查是否还有未完成的协程任务，用于优雅停机。
     */
    virtual int CheckStopServer();

public:
    /**
     * @brief 初始化协程调度器
     * @param stack_size 协程栈大小，默认256KB
     * @return 返回0表示成功，-1表示失败
     * 
     * 创建底层调度器并初始化相关资源。
     */
    int Init(uint32_t stack_size = 256 * 1024);

    /**
     * @brief 关闭协程调度器
     * @return 返回还未结束的协程数量
     * 
     * 释放所有协程资源，清理调度器状态。
     */
    int Close();

    /**
     * @brief 获取当前协程数量
     * @return 返回当前活跃的协程任务数量
     */
    int Size() const;

    /**
     * @brief 获取当前正在运行的协程任务
     * @return 返回正在运行的协程任务对象指针，非协程内调用返回nullptr
     * 
     * @note 此函数必须在协程中调用
     */
    NFCoroutineTask *CurrentTask() const;

    /**
     * @brief 获取当前正在运行的协程ID
     * @return 返回正在运行的协程ID，非协程内调用返回-1
     * 
     * @note 此函数必须在协程中调用
     */
    int64_t CurrentTaskId() const;

    /**
     * @brief 检查当前是否在协程中
     * @return 返回true表示在协程中，false表示在主线程中
     * 
     * 用于判断当前执行上下文是否为协程环境。
     */
    bool IsInCoroutine() const;

    /**
     * @brief 挂起当前协程
     * @param timeout_ms 超时时间（毫秒），默认-1表示无超时，<=0表示不进行超时处理
     * @return 返回处理结果，参见CoroutineErrorCode
     * 
     * 暂停当前协程的执行，支持超时自动恢复。
     * 
     * @note 此函数必须在协程中调用
     */
    int32_t Yield(int32_t timeout_ms = -1);

    /**
     * @brief 激活指定ID的协程
     * @param id 协程ID
     * @param result Resume时传递的结果，默认为0
     * @return 返回处理结果，参见CoroutineErrorCode
     * 
     * 唤醒指定的挂起协程，使其继续执行。
     */
    int32_t Resume(int64_t id, int32_t result = 0);

    /**
     * @brief 获取指定协程的状态
     * @param id 协程ID
     * @return 返回协程状态，参见协程状态枚举
     */
    int Status(int64_t id);

    /**
     * @brief 获取协程的用户数据
     * @param id 协程ID
     * @return 返回协程携带的protobuf消息指针，不存在返回nullptr
     */
    google::protobuf::Message *GetUserData(int64_t id);
    
    /**
     * @brief 设置协程的用户数据
     * @param id 协程ID
     * @param pUserData protobuf消息指针
     * @return 返回0表示成功，非0表示失败
     */
    int SetUserData(int64_t id, google::protobuf::Message *pUserData);

    /**
     * @brief 添加任务到调度器
     * @param task 要添加的协程任务
     * @return 返回0表示成功，非0表示失败
     * 
     * 将协程任务添加到调度器的管理中。
     */
    int AddTaskToSchedule(NFCoroutineTask *task);
    
    /**
     * @brief 处理协程超时
     * @param id 超时的协程ID
     * @return 返回处理结果
     * 
     * 当协程发生超时时的处理回调。
     */
    int32_t OnTimeout(int64_t id);
    
    /**
     * @brief 查找指定ID的协程任务
     * @param id 协程任务ID
     * @return 返回协程任务对象指针，不存在返回nullptr
     */
    NFCoroutineTask *Find(int64_t id) const;

private:
    /**
     * @brief 底层协程调度器
     * 
     * 指向NFSchedule实例，提供底层的协程调度功能。
     */
    NFSchedule *schedule_;
    
    /**
     * @brief 任务映射表
     * 
     * 以任务ID为键，协程任务对象为值的映射表，
     * 用于快速查找和管理协程任务。
     */
    std::unordered_map<int64_t, NFCoroutineTask *> task_map_;
    
    /**
     * @brief 预启动任务集合
     * 
     * 存储等待启动的协程任务，支持延迟启动机制。
     */
    std::set<NFCoroutineTask *> pre_start_task_;
    
    /**
     * @brief 任务定时器列表
     * 
     * 管理所有协程任务相关的定时器，
     * 用于超时控制和生命周期管理。
     */
    std::list<NFCoroutineTaskTimer*> m_taskTimer;
};
