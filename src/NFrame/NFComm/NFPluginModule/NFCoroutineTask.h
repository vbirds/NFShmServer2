// -------------------------------------------------------------------------
//    @FileName         :    NFCoroutineTask.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    协程任务基础类，提供Actor模式的协程任务抽象和实现
//                           支持异步执行、超时控制和协程生命周期管理
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFBaseObj.h"
#include <functional>

#ifdef Yield
#undef Yield  ///< 取消可能与系统宏冲突的Yield定义
#endif

/**
 * @brief 协程任务抽象基类
 * 
 * NFCoroutineTask 提供了基于协程的异步任务执行框架：
 * 
 * 1. 核心设计理念：
 *    - Actor模式：每个协程任务作为独立的执行单元
 *    - 异步执行：支持立即执行和延迟执行模式
 *    - 生命周期管理：完整的创建、运行、挂起、销毁流程
 *    - 超时控制：提供灵活的超时机制防止死锁
 * 
 * 2. 协程特性：
 *    - 用户态线程：在用户空间实现的轻量级线程
 *    - 协作式调度：通过Yield主动让出执行权
 *    - 状态保持：自动保存和恢复执行上下文
 *    - 无锁设计：避免传统多线程的锁竞争
 * 
 * 3. 使用模式：
 *    - 继承实现：继承本类并实现Run()方法
 *    - 函数式：使用NFCommonCoroutineTask传入函数对象
 *    - 任务链：支持协程间的相互调用和等待
 *    - 事件驱动：与事件系统集成实现响应式编程
 * 
 * 4. 应用场景：
 *    - 网络I/O操作的异步处理
 *    - 数据库操作的非阻塞调用
 *    - 复杂业务逻辑的分步执行
 *    - 定时任务和周期性作业
 *    - 状态机的实现和管理
 * 
 * 5. 性能优势：
 *    - 低内存开销：相比系统线程占用更少内存
 *    - 快速切换：上下文切换成本极低
 *    - 高并发：支持数万个协程并发执行
 *    - 可控调度：精确控制任务执行时机
 * 
 * 使用示例：
 * @code
 * // 方式1：继承实现
 * class MyTask : public NFCoroutineTask {
 * public:
 *     MyTask(NFIPluginManager* mgr) : NFCoroutineTask(mgr) {}
 *     
 *     virtual void Run() override {
 *         // 执行一些操作
 *         ProcessStep1();
 *         
 *         // 挂起等待外部事件
 *         Yield(5000); // 等待5秒或外部唤醒
 *         
 *         // 继续执行
 *         ProcessStep2();
 *     }
 * };
 * 
 * // 启动协程
 * MyTask* task = new MyTask(pluginManager);
 * int64_t taskId = task->Start(true); // 立即执行
 * 
 * // 方式2：函数式
 * auto task = new NFCommonCoroutineTask(pluginManager, []() {
 *     NFLogInfo(NF_LOG_DEFAULT, 0, "协程开始执行");
 *     // 执行业务逻辑
 * });
 * task->Start();
 * @endcode
 * 
 * 协程生命周期：
 * ```
 * [Created] -> [Running] -> [Suspended] -> [Running] -> [Finished]
 *     |           |            |             |            |
 *   Start()    Run()        Yield()       Resume()    Destroy()
 * ```
 * 
 * 设计优势：
 * - 简化异步编程的复杂性
 * - 提供同步编程的直观性
 * - 支持大规模并发处理
 * - 便于调试和错误处理
 * - 与框架其他组件无缝集成
 */
class NFCoroutineTask : public NFBaseObj {
public:
    /**
     * @brief 构造函数
     * @param pPluginManager 插件管理器指针，用于访问框架服务
     * 
     * 初始化协程任务的基本状态：
     * - 设置插件管理器的引用
     * - 初始化协程ID为无效值
     * - 准备协程执行环境
     * - 注册到协程管理系统
     */
    NFCoroutineTask(NFIPluginManager *pPluginManager);

    /**
     * @brief 虚析构函数
     * 
     * 清理协程任务的资源：
     * - 确保协程已经正确结束
     * - 释放协程栈空间
     * - 从协程管理系统中注销
     * - 清理相关的回调和事件
     * 
     * 注意：析构时如果协程仍在运行，会强制终止协程
     */
    virtual ~NFCoroutineTask();

    /**
     * @brief 启动协程任务
     * @param is_immediately 是否立即执行，true表示立即执行，false表示延迟到下一帧
     * @return 返回协程ID，可用于后续的协程管理操作
     * 
     * 协程启动流程：
     * 1. 验证协程状态是否允许启动
     * 2. 分配协程ID和执行栈
     * 3. 注册到协程调度器
     * 4. 根据is_immediately参数决定执行时机
     * 5. 调用Run()方法开始执行协程体
     * 
     * 启动模式：
     * - 立即执行：在当前调用栈中直接开始协程执行
     * - 延迟执行：加入调度队列，在下一个调度周期执行
     * 
     * 返回值说明：
     * - 正数：协程ID，可用于协程间通信和管理
     * - 0或负数：启动失败，检查错误日志获取详细信息
     * 
     * 使用注意：
     * - 每个协程任务只能启动一次
     * - 重复调用Start()会返回错误
     * - 协程ID在整个进程生命周期内唯一
     */
    int64_t Start(bool is_immediately = true);

    /**
     * @brief 协程任务的执行体（纯虚函数）
     * 
     * 这是协程的核心逻辑实现，子类必须重写此方法：
     * 
     * 实现要求：
     * - 包含具体的业务逻辑
     * - 可以调用Yield()进行协程挂起
     * - 应该处理可能的异常情况
     * - 执行完成后协程自动结束
     * 
     * 执行环境：
     * - 在独立的协程栈中执行
     * - 可以访问插件管理器的所有服务
     * - 支持与其他协程进行通信
     * - 可以订阅和触发事件
     * 
     * 挂起和恢复：
     * - 通过Yield()可以主动挂起协程
     * - 外部事件可以唤醒挂起的协程
     * - 超时机制可以自动恢复协程执行
     * 
     * 错误处理：
     * - 未捕获的异常会导致协程终止
     * - 建议使用try-catch保护关键代码
     * - 可以通过日志记录执行状态
     * 
     * 性能考虑：
     * - 避免长时间占用CPU的计算
     * - 适时调用Yield()让出执行权
     * - 合理使用超时机制避免死锁
     */
    virtual void Run() = 0;

    /**
     * @brief 获取协程任务的唯一ID
     * @return 协程ID，在协程生命周期内保持不变
     * 
     * 协程ID的特点：
     * - 全局唯一：在整个进程中不会重复
     * - 持久有效：从创建到销毁始终有效
     * - 递增分配：按照创建顺序递增分配
     * - 非零保证：有效的协程ID总是大于0
     * 
     * 使用场景：
     * - 协程间通信的地址标识
     * - 调试和日志记录的跟踪ID
     * - 协程管理系统的索引键
     * - 外部系统的协程引用
     * 
     * 注意事项：
     * - 协程未启动时ID为0或无效值
     * - 协程销毁后ID会被回收利用
     * - 不要长期持有已结束协程的ID
     */
    inline int64_t id() {
        return id_;
    }

    /**
     * @brief 挂起当前协程
     * @param timeout_ms 超时时间（毫秒），默认-1表示无限等待
     * @return 恢复原因码，参见CoroutineErrorCode枚举
     * 
     * 挂起机制说明：
     * - 保存当前执行上下文（寄存器、栈指针等）
     * - 将协程标记为挂起状态
     * - 让出CPU给调度器处理其他任务
     * - 等待外部唤醒或超时自动恢复
     * 
     * 超时处理：
     * - timeout_ms < 0：无限等待，直到外部唤醒
     * - timeout_ms = 0：立即返回，用于状态检查
     * - timeout_ms > 0：指定毫秒后自动恢复执行
     * 
     * 唤醒条件：
     * - 定时器超时自动唤醒
     * - 外部调用Resume()主动唤醒
     * - 事件触发导致的唤醒
     * - 系统关闭时的强制唤醒
     * 
     * 返回值含义：
     * - 0：正常唤醒（外部调用或事件触发）
     * - 正数：超时唤醒
     * - 负数：错误或异常唤醒
     * 
     * 使用模式：
     * @code
     * // 等待5秒或外部唤醒
     * int result = Yield(5000);
     * if (result == 0) {
     *     // 外部唤醒，继续处理
     * } else if (result > 0) {
     *     // 超时，执行超时逻辑
     * } else {
     *     // 错误，进行错误处理
     * }
     * @endcode
     * 
     * 注意事项：
     * - 只能在协程内部调用（Run方法中）
     * - 在主线程调用会导致错误
     * - 频繁的Yield会影响性能
     * - 超时值应该合理设置避免死锁
     */
    int32_t Yield(int32_t timeout_ms = -1);

public:
    int64_t id_;  ///< 协程的唯一标识符，由协程管理系统分配
};

/**
 * @brief 基于函数对象的通用协程任务实现
 * 
 * NFCommonCoroutineTask 提供了一种便捷的方式来创建协程任务：
 * 
 * 1. 设计目的：
 *    - 简化协程的创建过程
 *    - 支持lambda表达式和函数对象
 *    - 避免为简单任务创建专门的类
 *    - 提供函数式编程的协程支持
 * 
 * 2. 核心特性：
 *    - 接受std::function<void()>类型的可调用对象
 *    - 支持lambda、函数指针、函数对象等
 *    - 提供运行时的函数绑定和替换
 *    - 继承所有协程基类的功能
 * 
 * 3. 使用场景：
 *    - 简单的异步任务执行
 *    - 事件处理的协程化
 *    - 临时性的协程作业
 *    - 脚本系统的协程集成
 * 
 * 4. 优势特点：
 *    - 语法简洁：一行代码创建协程
 *    - 灵活性高：支持各种可调用对象
 *    - 性能良好：最小的运行时开销
 *    - 易于测试：便于单元测试的编写
 * 
 * 使用示例：
 * @code
 * // 使用lambda创建协程
 * auto task1 = new NFCommonCoroutineTask(pluginManager, []() {
 *     NFLogInfo(NF_LOG_DEFAULT, 0, "Hello from coroutine!");
 * });
 * 
 * // 使用函数对象创建协程
 * std::function<void()> func = std::bind(&MyClass::Method, myObject);
 * auto task2 = new NFCommonCoroutineTask(pluginManager, func);
 * 
 * // 动态设置执行函数
 * auto task3 = new NFCommonCoroutineTask(pluginManager);
 * task3->Init([=]() {
 *     // 捕获外部变量的协程执行体
 *     ProcessData(capturedData);
 * });
 * 
 * // 启动协程
 * task1->Start(true);
 * task2->Start(false);
 * task3->Start();
 * @endcode
 * 
 * 设计优势：
 * - 降低协程使用的门槛
 * - 提高代码的可读性
 * - 支持现代C++的编程风格
 * - 便于与现有代码的集成
 */
class NFCommonCoroutineTask : public NFCoroutineTask {
public:
	/**
	 * @brief 默认构造函数
	 * @param pPluginManager 插件管理器指针
	 * 
	 * 创建一个空的协程任务，需要后续调用Init()设置执行函数。
	 */
	NFCommonCoroutineTask(NFIPluginManager *pPluginManager):NFCoroutineTask(pPluginManager) {}
	
	/**
	 * @brief 带执行函数的构造函数
	 * @param pPluginManager 插件管理器指针
	 * @param run 协程执行函数，可以是lambda、函数指针或函数对象
	 * 
	 * 创建协程任务并设置执行函数，可以直接调用Start()启动。
	 */
	NFCommonCoroutineTask(NFIPluginManager *pPluginManager, const std::function<void(void)>& run):NFCoroutineTask(pPluginManager),m_run(run) {}

    /**
     * @brief 虚析构函数
     * 
     * 清理函数对象相关的资源。
     */
    virtual ~NFCommonCoroutineTask() {}

    /**
     * @brief 初始化或更新协程执行函数
     * @param run 新的执行函数
     * 
     * 设置协程的执行体，可以在协程启动前调用：
     * - 支持lambda表达式
     * - 支持std::function对象
     * - 支持函数指针和成员函数绑定
     * - 可以多次调用以更新执行逻辑
     * 
     * 注意：协程启动后调用此方法无效。
     */
    void Init(const std::function<void(void)>& run) { m_run = run; }

    /**
     * @brief 执行协程任务体
     * 
     * 重写基类的Run方法，调用用户设置的函数对象：
     * - 检查函数对象的有效性
     * - 在协程上下文中执行用户函数
     * - 处理可能的异常情况
     * - 确保协程正常结束
     * 
     * 执行流程：
     * 1. 验证m_run是否有效
     * 2. 调用用户函数
     * 3. 处理执行过程中的异常
     * 4. 清理和资源释放
     */
    virtual void Run() {
        m_run();
    }

private:
    std::function<void(void)> m_run;  ///< 用户设置的协程执行函数
};
