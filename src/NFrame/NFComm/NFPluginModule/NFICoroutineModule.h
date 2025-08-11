// -------------------------------------------------------------------------
//    @FileName         :    NFICoroutineModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFICoroutineModule.h
//    @Description      :    协程模块接口定义，提供协程任务的创建、管理和调度功能
//                           支持协程的挂起恢复、状态管理和用户关联等高级特性
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFIRpcService.h"
#include "NFITimerEventModule.h"

class NFCoroutineTask;

/// @brief 解决Windows平台Yield宏定义冲突
#ifdef Yield
#undef Yield
#endif

/**
 * @brief 协程模块接口类
 * 
 * NFICoroutineModule 提供了完整的协程管理功能，支持异步编程模式：
 * 
 * 1. 协程生命周期管理：
 *    - 协程的创建、启动、删除
 *    - 协程状态跟踪和管理
 *    - 协程任务的添加和移除
 * 
 * 2. 协程调度控制：
 *    - 协程的挂起（Yield）和恢复（Resume）
 *    - 基于时间的协程超时处理
 *    - 协程执行结果的传递
 * 
 * 3. 协程状态查询：
 *    - 协程运行状态检查
 *    - 当前协程信息获取
 *    - 协程存活状态判断
 * 
 * 4. 用户协程管理：
 *    - 用户关联的协程追踪
 *    - 用户协程数量统计
 *    - 用户协程清理管理
 * 
 * 5. 协程数据管理：
 *    - 协程用户数据存储
 *    - protobuf消息传递支持
 * 
 * 协程系统特性：
 * - 轻量级的异步任务处理
 * - 非阻塞的I/O操作支持
 * - 简化的异步编程模型
 * - 高效的协程调度算法
 * - 完善的错误处理机制
 * 
 * 使用场景：
 * - 异步网络请求处理
 * - 数据库操作的异步化
 * - 复杂业务逻辑的状态管理
 * - 定时任务和延迟执行
 */
class NFICoroutineModule : public NFITimerEventModule {
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    NFICoroutineModule(NFIPluginManager *p) : NFITimerEventModule(p) {

    }

    /**
     * @brief 虚析构函数
     * 
     * 确保派生类能够正确析构，清理所有协程资源。
     */
    virtual ~NFICoroutineModule() {

    }

    /**
     * @brief 创建并启动协程
     * @param func 协程执行的函数对象
     * @param is_immediately 是否立即执行，默认为true
     * @return 返回协程ID，失败返回-1
     * 
     * 创建一个新的协程并执行指定的函数。这是创建协程的便捷方法，
     * 适用于简单的函数式协程任务。
     * 
     * 使用示例：
     * @code
     * auto coId = MakeCoroutine([this]() {
     *     // 协程执行的代码
     *     auto result = SomeAsyncOperation();
     *     // 处理结果
     * });
     * @endcode
     */
    virtual int64_t MakeCoroutine(const std::function<void()> &func, bool is_immediately = true) = 0;

    /**
     * @brief 启动协程任务
     * @param pTask 协程任务对象指针
     * @param is_immediately 是否立即执行，默认为true
     * @return 返回协程ID，失败返回-1
     * 
     * 启动一个协程任务对象，执行其Run方法。适用于需要复杂
     * 状态管理和生命周期控制的协程任务。
     */
    virtual int64_t Start(NFCoroutineTask *pTask, bool is_immediately = true) = 0;

    /**
     * @brief 删除协程任务
     * @param pTask 要删除的协程任务对象
     * @return 删除成功返回0，失败返回非0值
     * 
     * 从协程管理器中删除指定的协程任务，释放相关资源。
     */
    virtual int DeleteTask(NFCoroutineTask *pTask) = 0;

    /**
     * @brief 添加协程任务
     * @param pTask 要添加的协程任务对象
     * @return 添加成功返回0，失败返回非0值
     * 
     * 将协程任务添加到管理器中，但不立即执行。
     */
    virtual int AddTask(NFCoroutineTask *pTask) = 0;

    /**
     * @brief 获取当前协程数量
     * @return 当前活跃的协程数量
     * 
     * 返回协程管理器中当前存在的协程任务总数，
     * 用于监控和调试。
     */
    virtual int Size() const = 0;

    /**
     * @brief 获取当前正在运行的协程任务
     * @return 正在运行的协程任务对象指针，如果不在协程中则返回nullptr
     * 
     * 此函数必须在协程上下文中调用，用于获取当前执行的协程任务对象。
     */
    virtual NFCoroutineTask *CurrentTask() const = 0;

    /**
     * @brief 获取当前正在运行的协程ID
     * @return 正在运行的协程ID，如果不在协程中则返回-1
     * 
     * 此函数必须在协程上下文中调用，用于获取当前协程的唯一标识符。
     */
    virtual int64_t CurrentTaskId() const = 0;

    /**
     * @brief 检查当前是否在协程中
     * @return 如果在协程中返回true，否则返回false
     * 
     * 用于判断当前代码是否在协程上下文中执行，避免在
     * 非协程环境中调用协程专用函数。
     */
    virtual bool IsInCoroutine() const = 0;

    /**
     * @brief 挂起当前协程
     * @param timeout_ms 超时时间（毫秒），默认-1表示无超时，<=0时不进行超时处理
     * @return 处理结果，具体见CoroutineErrorCode
     * 
     * 挂起当前协程的执行，释放CPU控制权。协程可以通过Resume函数恢复执行，
     * 或在超时后自动恢复。此函数必须在协程上下文中调用。
     * 
     * 使用示例：
     * @code
     * // 挂起等待外部事件
     * int result = Yield(5000); // 最多等待5秒
     * if (result == TIMEOUT) {
     *     // 处理超时情况
     * }
     * @endcode
     */
    virtual int32_t Yield(int32_t timeout_ms = -1) = 0;

    /**
     * @brief 恢复指定ID的协程
     * @param id 协程ID
     * @param result 恢复时传递的结果值，默认为0
     * @return 处理结果，具体见CoroutineErrorCode
     * 
     * 恢复之前被挂起的协程执行。result参数会作为Yield函数的返回值
     * 传递给协程，用于传递恢复原因或结果数据。
     */
    virtual int32_t Resume(int64_t id, int32_t result = 0) = 0;

    /**
     * @brief 获取指定协程的状态
     * @param id 协程ID
     * @return 协程状态值
     * 
     * 查询指定协程的当前运行状态，如运行中、挂起、已结束等。
     */
    virtual int Status(int64_t id) = 0;

    /**
     * @brief 获取协程的用户数据
     * @param id 协程ID
     * @return 用户数据的protobuf消息指针，未设置返回nullptr
     * 
     * 获取与协程关联的用户自定义数据，支持protobuf消息格式。
     */
    virtual google::protobuf::Message *GetUserData(int64_t id) = 0;
    
    /**
     * @brief 设置当前协程的用户数据
     * @param pUserData 用户数据的protobuf消息指针
     * @return 设置成功返回0，失败返回非0值
     * 
     * 为当前协程设置用户自定义数据，必须在协程上下文中调用。
     */
    virtual int SetUserData(google::protobuf::Message *pUserData) = 0;

    /**
     * @brief 添加用户协程关联
     * @param userId 用户ID
     * @return 添加成功返回0，失败返回非0值
     * 
     * 将当前协程与指定用户ID关联，用于跟踪用户相关的协程任务。
     * 通常在处理用户请求的协程中调用。
     */
    virtual int AddUserCo(uint64_t userId) = 0;

    /**
     * @brief 删除用户协程关联
     * @param userId 用户ID
     * @return 删除成功返回0，失败返回非0值
     * 
     * 移除用户与当前协程的关联关系，通常在协程结束时调用。
     */
    virtual int DelUserCo(uint64_t userId) = 0;

    /**
     * @brief 检查用户是否还有协程在运行
     * @param userId 用户ID
     * @return 如果用户还有协程在运行返回true，否则返回false
     * 
     * 用于检查指定用户是否还有未完成的协程任务，常用于
     * 用户断线重连或资源清理时的状态检查。
     */
    virtual bool IsExistUserCo(uint64_t userId) = 0;

    /**
     * @brief 检查协程是否已死亡
     * @param id 协程ID
     * @return 如果协程已死亡返回true，否则返回false
     * 
     * 检查指定协程是否已经执行完毕或被强制终止。
     */
    virtual bool IsDead(int64_t id) = 0;

    /**
     * @brief 检查协程是否正在运行
     * @param id 协程ID
     * @return 如果协程正在运行返回true，否则返回false
     * 
     * 检查指定协程是否处于活跃执行状态。
     */
    virtual bool IsRunning(int64_t id) = 0;

    /**
     * @brief 检查协程是否正在挂起
     * @param id 协程ID
     * @return 如果协程正在挂起返回true，否则返回false
     * 
     * 检查指定协程是否处于挂起状态，等待被恢复执行。
     */
    virtual bool IsYielding(int64_t id) = 0;
};