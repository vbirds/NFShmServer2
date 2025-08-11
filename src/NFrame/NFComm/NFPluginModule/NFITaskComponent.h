// -------------------------------------------------------------------------
//    @FileName         :    NFITaskComponent.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFITaskComponent
//    @Description      :    任务组件接口定义，提供任务处理的基础组件框架
//
//
//                    .::::.
//                  .::::::::.
//                 :::::::::::  FUCK YOU
//             ..:::::::::::'
//           '::::::::::::'
//             .::::::::::
//        '::::::::::::::..
//             ..::::::::::::.
//           ``::::::::::::::::
//            ::::``:::::::::'        .:::.
//           ::::'   ':::::'       .::::::::.
//         .::::'      ::::     .:::::::'::::.
//        .:::'       :::::  .:::::::::' ':::::.
//       .::'        :::::.:::::::::'      ':::::.
//      .::'         ::::::::::::::'         ``::::.
//  ...:::           ::::::::::::'              ``::.
// ```` ':.          ':::::::::'                  ::::..
//                    '.:::::'                    ':'````..
//
// -------------------------------------------------------------------------

/**
 * @file NFITaskComponent.h
 * @brief 任务组件接口定义
 * @details 该文件定义了任务处理组件的基础接口，提供了：
 *          - 任务执行的生命周期管理
 *          - Actor模式的任务处理架构
 *          - 任务死锁和超时检测机制
 *          - 可扩展的任务处理组件框架
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFTask.h"

class NFTaskActor;

/**
 * @class NFITaskComponent
 * @brief 任务组件接口基类
 * @details 该接口定义了任务处理组件的基础框架，主要功能包括：
 * 
 * **核心特性：**
 * - **生命周期管理**：提供任务开始、执行、结束的完整生命周期
 * - **Actor模式支持**：与Actor系统集成，支持分布式任务处理
 * - **异常监控**：内置任务死锁和超时检测机制
 * - **可扩展性**：允许自定义任务处理逻辑和组件行为
 * 
 * **设计模式：**
 * - 采用模板方法模式，定义任务处理的标准流程
 * - 支持组件化架构，便于功能模块的组合和扩展
 * - 提供钩子方法，允许子类自定义特定阶段的处理逻辑
 * 
 * **使用场景：**
 * - 多线程任务处理系统
 * - 分布式计算任务调度
 * - 游戏服务器业务逻辑处理
 * - 异步任务执行框架
 * 
 * **注意事项：**
 * - 子类需要根据具体需求重写虚拟方法
 * - 任务处理应当是线程安全的
 * - 需要正确处理异常情况以避免死锁
 */
class NFITaskComponent
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化任务组件，设置默认的Actor ID为-1，
	 * 表示组件尚未与任何Actor关联。
	 */
	NFITaskComponent()
	{
		mActorId = -1;
	}

	/**
	 * @brief 析构函数
	 * 
	 * 清理任务组件资源。子类应在析构函数中
	 * 处理组件特定的清理逻辑。
	 */
	virtual ~NFITaskComponent()
	{
	}

	/**
	 * @brief 任务开始前的预处理
	 * @param pTask 待处理的任务指针
	 * 
	 * 在任务开始执行前调用，可用于：
	 * - 任务预处理和验证
	 * - 资源准备和初始化
	 * - 上下文设置和环境检查
	 * - 日志记录和监控统计
	 * 
	 * @note 默认实现为空，子类可根据需要重写
	 */
	virtual void ProcessTaskStart(NFTask* pTask)
	{
	}

	/**
	 * @brief 执行任务的核心处理逻辑
	 * @param pTask 待处理的任务指针
	 * 
	 * 任务执行的主要入口点，负责：
	 * - 任务有效性检查
	 * - 调用任务的线程处理方法
	 * - 错误处理和异常捕获
	 * 
	 * @details 默认实现会检查任务指针的有效性，
	 *          然后调用任务的ThreadProcess()方法进行实际处理。
	 *          子类可以重写此方法来实现自定义的任务处理逻辑。
	 * 
	 * @warning 该方法在多线程环境中调用，需要确保线程安全
	 */
	virtual void ProcessTask(NFTask* pTask)
	{
		if (pTask)
		{
			//NFLogError("actor id:{}, threadid:{}", this->GetAddress().AsInteger(), ThreadId());
			pTask->ThreadProcess();
		}
	}

	/**
	 * @brief 任务完成后的后处理
	 * @param pTask 已处理的任务指针
	 * 
	 * 在任务执行完成后调用，可用于：
	 * - 结果收集和统计
	 * - 资源清理和释放
	 * - 状态更新和通知
	 * - 性能监控和日志记录
	 * 
	 * @note 默认实现为空，子类可根据需要重写
	 */
	virtual void ProcessTaskEnd(NFTask* pTask)
	{

	}

	/**
	 * @brief 获取Actor ID
	 * @return 当前组件关联的Actor ID，-1表示未关联
	 * 
	 * Actor ID用于标识组件所属的Actor实例，
	 * 在分布式系统中用于任务路由和负载均衡。
	 */
	virtual int GetActorId() const
	{
		return mActorId;
	}

	/**
	 * @brief 设置Actor ID
	 * @param actorId 要设置的Actor ID
	 * 
	 * 将组件与指定的Actor实例关联，用于：
	 * - 任务路由和分发
	 * - 组件生命周期管理
	 * - 性能监控和负载统计
	 */
	virtual void SetActorId(int actorId)
	{
		mActorId = actorId;
	}

	/**
	 * @brief 获取组件名称
	 * @return 组件的名称字符串
	 * 
	 * 组件名称用于：
	 * - 日志记录和调试
	 * - 配置管理和查找
	 * - 监控统计和报告
	 * - 动态组件管理
	 */
	virtual const std::string GetComponentName() const
	{
		return m_componentName;
	}

	/**
	 * @brief 设置组件名称
	 * @param name 要设置的组件名称
	 * 
	 * 为组件指定一个有意义的名称，便于：
	 * - 系统监控和管理
	 * - 错误诊断和调试
	 * - 配置文件中的引用
	 */
	virtual void SetComponentName(const std::string& name)
	{
		m_componentName = name;
	}

	/**
	 * @brief 处理任务死锁情况
	 * @param taskName 发生死锁的任务名称
	 * @param useTime 任务已经运行的时间（毫秒）
	 * 
	 * 当检测到任务可能出现死锁时调用，用于：
	 * - 死锁检测和报告
	 * - 错误日志记录
	 * - 系统监控告警
	 * - 恢复策略执行
	 * 
	 * @note 默认实现为空，子类应根据需要实现具体的死锁处理逻辑
	 * @warning 死锁可能导致系统性能下降或服务不可用
	 */
	virtual void HandleTaskDeadCycle(const std::string& taskName, uint64_t useTime)
	{

	}

	/**
	 * @brief 处理任务超时情况
	 * @param taskName 超时的任务名称
	 * @param useTime 任务已经运行的时间（毫秒）
	 * 
	 * 当任务执行时间超过预设阈值时调用，用于：
	 * - 超时监控和报告
	 * - 性能分析和优化
	 * - 系统负载评估
	 * - 异常处理和恢复
	 * 
	 * @note 默认实现为空，子类应根据需要实现具体的超时处理逻辑
	 * @see 建议结合系统监控工具进行超时分析
	 */
	virtual void HandleTaskTimeOut(const std::string& taskName, uint64_t useTime)
	{

	}
private:
	int mActorId;                    ///< Actor ID，用于标识组件所属的Actor实例
	std::string m_componentName;     ///< 组件名称，用于标识和管理组件
};


