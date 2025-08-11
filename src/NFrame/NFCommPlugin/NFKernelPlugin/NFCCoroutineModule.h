// -------------------------------------------------------------------------
//    @FileName         :    NFCCoroutineModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCCoroutineModule
//    @Desc             :    协程模块头文件，提供高性能的协程调度和管理功能。
//                          该文件定义了NFShmXFrame框架的协程模块，提供完整的协程调度和管理系统，
//                          包括协程创建和销毁、协程调度、协程状态管理、协程通信、超时机制等功能。
//                          主要功能包括基于协作式调度的高效协程切换、零拷贝调度、内存池管理、
//                          智能垃圾回收、模板化任务、函数式协程、玩家协程管理
//    @Description      :    协程模块头文件，提供高性能的协程调度和管理功能
//
// -------------------------------------------------------------------------

#pragma once

#include <unordered_map>
#include "NFComm/NFPluginModule/NFICoroutineModule.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"

class NFCoroutineSchedule;

/**
 * @class NFCCoroutineModule
 * @brief 协程模块实现类
 *
 * NFCCoroutineModule提供了完整的协程调度和管理系统，实现了用户态的轻量级线程：
 * 
 * 协程核心功能：
 * - 协程创建和销毁：支持动态创建和自动回收协程
 * - 协程调度：基于协作式调度的高效协程切换
 * - 协程状态管理：运行、挂起、死亡等状态跟踪
 * - 协程通信：协程间数据传递和结果返回
 * 
 * 高级特性：
 * - 超时机制：支持协程挂起超时自动恢复
 * - 用户数据：协程可携带自定义protobuf数据
 * - 玩家协程管理：按玩家ID分组管理协程
 * - 模板化任务：类型安全的协程任务创建
 * - 函数式协程：支持lambda函数直接创建协程
 * 
 * 性能优化：
 * - 零拷贝调度：高效的上下文切换
 * - 内存池管理：协程栈内存复用
 * - 批量调度：减少系统调用开销
 * - 智能垃圾回收：自动清理死亡协程
 * 
 * 应用场景：
 * - 异步网络I/O：数据库查询、HTTP请求等
 * - 游戏逻辑流程：复杂的状态机和时序控制
 * - 定时任务：延时执行和周期性任务
 * - 并发处理：大量并发请求的高效处理
 * 
 * 安全保障：
 * - 协程生命周期管理：防止野指针和内存泄漏
 * - 异常隔离：单个协程异常不影响其他协程
 * - 资源限制：防止协程数量过多导致的资源耗尽
 * 
 * @note 协程基于ucontext或fiber实现，提供毫秒级调度精度
 * @note 支持协程嵌套和递归调用
 * @warning 协程内不能使用阻塞式系统调用
 */
class NFCCoroutineModule : public NFICoroutineModule {
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    explicit NFCCoroutineModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     */
    virtual ~NFCCoroutineModule();

    /**
     * @brief 定时器回调处理
     * @param nTimerID 定时器ID
     * @return 返回0表示成功，非0表示失败
     */
    virtual int OnTimer(uint32_t nTimerID) override;

    /**
     * @brief 事件执行处理
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 源类型
     * @param nSrcID 源ID
     * @param pMessage 消息指针
     * @return 返回0表示成功，非0表示失败
     */
    virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage) override;

    /**
     * @brief 模块觉醒初始化
     * @return 返回0表示成功，非0表示失败
     */
    virtual int Awake() override;

    /**
     * @brief 更新用户协程状态
     * 
     * 检查和清理已完成的用户协程，维护用户协程映射表。
     */
    void UpdateUser();

    /**
     * @brief 检查服务器停止状态
     * @return 返回0表示可以停止，非0表示还有协程在运行
     * 
     * 检查是否还有未完成的协程，用于优雅停机。
     */
    virtual int CheckStopServer() override;

    /**
     * @brief 关闭协程模块
     * @return 返回0表示成功，非0表示失败
     */
    virtual int Shut() override;

    /**
     * @brief 最终化处理
     * @return 返回0表示成功，非0表示失败
     */
    virtual int Finalize() override;

    /**
     * @brief 协程调度主循环
     * @return 返回0表示成功，非0表示失败
     * 
     * 执行协程调度，处理协程的创建、运行、挂起和销毁。
     */
    virtual int Tick() override;

    /**
     * @brief 启动协程任务
     * @param pTask 协程任务对象，必须继承自NFCoroutineTask
     * @param is_immediately 是否立即执行，默认为true
     * @return 返回协程ID，-1表示失败
     * 
     * 创建并启动一个新的协程，执行任务的Run方法。
     */
    virtual int64_t Start(NFCoroutineTask *pTask, bool is_immediately = true) override;

    /**
     * @brief 删除协程任务
     * @param pTask 要删除的协程任务对象
     * @return 返回0表示成功，非0表示失败
     * 
     * 标记协程为删除状态，将在下次调度时清理资源。
     */
    virtual int DeleteTask(NFCoroutineTask *pTask) override;

    /**
     * @brief 添加协程任务到队列
     * @param pTask 要添加的协程任务对象
     * @return 返回0表示成功，非0表示失败
     * 
     * 将协程任务添加到调度队列，等待执行。
     */
    virtual int AddTask(NFCoroutineTask *pTask) override;

    /**
     * @brief 获取当前协程数量
     * @return 返回当前活跃的协程数量
     */
    virtual int Size() const override;

    /**
     * @brief 获取当前正在运行的协程任务
     * @return 返回正在运行的协程任务对象指针，非协程内调用返回nullptr
     * 
     * @note 此函数必须在协程中调用
     */
    virtual NFCoroutineTask *CurrentTask() const override;

    /**
     * @brief 获取当前正在运行的协程ID
     * @return 返回正在运行的协程ID，非协程内调用返回-1
     * 
     * @note 此函数必须在协程中调用
     */
    virtual int64_t CurrentTaskId() const override;

    /**
     * @brief 检查当前是否在协程中
     * @return 返回true表示在协程中，false表示在主线程中
     * 
     * 用于判断当前执行上下文是否为协程环境。
     */
    virtual bool IsInCoroutine() const override;

    /**
     * @brief 挂起当前协程
     * @param timeout_ms 超时时间（毫秒），默认-1表示无超时，<=0表示不进行超时处理
     * @return 返回处理结果，参见CoroutineErrorCode
     * 
     * 暂停当前协程的执行，让出CPU给其他协程或主线程。
     * 
     * @note 此函数必须在协程中调用
     * @note 协程被Resume时会从此处继续执行
     */
    virtual int32_t Yield(int32_t timeout_ms = -1) override;

    /**
     * @brief 激活指定ID的协程
     * @param id 要激活的协程ID
     * @param result Resume时传递的结果，默认为0
     * @return 返回处理结果，参见CoroutineErrorCode
     * 
     * 唤醒指定的挂起协程，使其继续执行。
     */
    virtual int32_t Resume(int64_t id, int32_t result = 0) override;

    /**
     * @brief 获取指定协程的状态
     * @param id 协程ID
     * @return 返回协程状态，参见CoroutineStatus
     */
    virtual int Status(int64_t id) override;

    /**
     * @brief 获取协程的用户数据
     * @param id 协程ID
     * @return 返回协程携带的protobuf消息指针，不存在返回nullptr
     */
    virtual google::protobuf::Message *GetUserData(int64_t id) override;
    
    /**
     * @brief 设置当前协程的用户数据
     * @param pUserData protobuf消息指针
     * @return 返回0表示成功，非0表示失败
     * 
     * @note 此函数必须在协程中调用
     */
    virtual int SetUserData(google::protobuf::Message *pUserData)  override;

    /**
     * @brief 添加玩家协程关联
     * @param userId 玩家ID
     * @return 返回0表示成功，非0表示失败
     * 
     * 将当前协程与指定玩家ID关联，用于玩家下线时清理相关协程。
     */
    virtual int AddUserCo(uint64_t userId) override;

    /**
     * @brief 移除玩家协程关联
     * @param userId 玩家ID
     * @return 返回0表示成功，非0表示失败
     * 
     * 移除当前协程与指定玩家ID的关联。
     */
    virtual int DelUserCo(uint64_t userId) override;

    /**
     * @brief 检查玩家是否还有协程在运行
     * @param userId 玩家ID
     * @return 返回true表示还有协程在运行，false表示没有
     * 
     * 用于判断玩家是否可以安全下线。
     */
    virtual bool IsExistUserCo(uint64_t userId) override;

    /**
     * @brief 检查协程是否已死亡
     * @param id 协程ID
     * @return 返回true表示协程已死亡，false表示协程存在
     */
    virtual bool IsDead(int64_t id) override;

    /**
     * @brief 检查协程是否正在运行
     * @param id 协程ID
     * @return 返回true表示协程正在运行，false表示协程未运行
     */
    virtual bool IsRunning(int64_t id) override;

    /**
     * @brief 检查协程是否正在挂起
     * @param id 协程ID
     * @return 返回true表示协程正在挂起，false表示协程未挂起
     */
    virtual bool IsYielding(int64_t id) override;

    /**
     * @brief 模板方法：创建新的协程任务
     * @tparam TASK 协程任务类型，必须继承自NFCoroutineTask
     * @return 返回创建的任务对象指针，失败返回nullptr
     * 
     * 使用此方法生成的task对象指针会在协程结束后自动delete掉。
     * 
     * @note 不能在协程内调用此方法
     */
    template<typename TASK>
    TASK *NewTask() {
        if (CurrentTaskId() != -1) {
            return NULL;
        }
        TASK *task = new TASK(m_pObjPluginManager);
        if (AddTask(task)) {
            delete task;
            task = NULL;
        }
        return task;
    }

    /**
     * @brief 创建函数式协程
     * @param func 要执行的函数对象或lambda表达式
     * @param is_immediately 是否立即执行，默认为true
     * @return 返回协程ID，-1表示失败
     * 
     * 直接使用函数或lambda创建协程，无需定义协程任务类。
     */
    virtual int64_t MakeCoroutine(const std::function<void()> &func, bool is_immediately = true) override;

private:
    /**
     * @brief 协程调度器
     * 
     * 负责底层的协程调度、上下文切换和生命周期管理。
     */
    NFCoroutineSchedule *m_pCorSched;
    
    /**
     * @brief 用户协程映射表
     * 
     * 以用户ID为键，关联的协程ID集合为值的映射表。
     * 用于管理玩家与协程的关联关系，支持玩家下线时的协程清理。
     */
    std::unordered_map<uint64_t, std::unordered_set<int64_t>> m_userCoIdMap;
};