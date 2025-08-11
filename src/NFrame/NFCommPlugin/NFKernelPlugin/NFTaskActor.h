// -------------------------------------------------------------------------
//    @FileName         :    NFTaskActor.h
//    @Author           :    Unknown
//    @Date             :   2022-09-18
//    @Module           :    NFTaskActor
//    @Description      :    任务Actor头文件，基于Theron框架的Actor模式异步任务处理实现
//
// -------------------------------------------------------------------------

#pragma once

#define THERON_USE_STD_THREADS 1

#include <vector>
#include <Theron/Theron.h>
#include <atomic>
#include "NFComm/NFCore/NFMutex.h"

class NFTask;
class NFTaskGroup;
class NFITaskComponent;

/**
 * @class NFTaskActorMessage
 * @brief Actor消息数据类
 *
 * NFTaskActorMessage封装了Actor间传递的消息数据，提供了完整的消息传递机制：
 * 
 * 消息传递功能：
 * - 消息类型：区分不同类型的消息用途
 * - 源Actor标识：记录消息发送方的Actor索引
 * - 任务数据：携带具体的任务处理数据
 * - 生命周期管理：自动管理消息的创建和销毁
 * 
 * 消息类型：
 * - ACTOR_MSG_TYPE_COMPONENT：需要处理的业务消息
 * - ACTOR_MSG_TYPE_END_FUNC：已处理完成，等待返回主线程的消息
 * 
 * 应用场景：
 * - 任务分发：主线程向Actor线程发送任务
 * - 结果返回：Actor线程向主线程返回处理结果
 * - 线程通信：跨线程的安全消息传递
 * 
 * @note 消息在Actor系统中自动管理生命周期
 */
class NFTaskActorMessage
{
public:
	/**
	 * @enum MessageType
	 * @brief 消息类型枚举
	 * 
	 * 定义了Actor消息的不同类型，用于区分消息的用途和处理方式。
	 */
	enum MessageType
	{
		/**
		 * @brief 组件消息类型
		 * 
		 * 表示这是一个需要Actor处理的业务任务消息。
		 */
		ACTOR_MSG_TYPE_COMPONENT,

		/**
		 * @brief 结束函数消息类型
		 * 
		 * 表示这是一个已经被Actor处理完成的消息，
		 * 等待返回主线程进行后续处理。
		 */
		ACTOR_MSG_TYPE_END_FUNC,
	};

	/**
	 * @brief 构造函数
	 * 
	 * 初始化消息对象，设置默认的消息类型和空数据。
	 */
	NFTaskActorMessage()
	{
		nFromActor = 0;
		pData = nullptr;
		nMsgType = ACTOR_MSG_TYPE_COMPONENT;
	}

	/**
	 * @brief 析构函数
	 * 
	 * 清理消息数据，重置所有字段为默认值。
	 */
	virtual ~NFTaskActorMessage()
	{
		nFromActor = 0;
		pData = nullptr;
		nMsgType = ACTOR_MSG_TYPE_COMPONENT;
	}

public:
	/**
	 * @brief 消息类型
	 * 
	 * 标识消息的类型，决定接收方如何处理这个消息。
	 */
	int nMsgType;

	/**
	 * @brief 发送消息的Actor索引
	 * 
	 * 记录发送此消息的Actor的唯一索引，用于回复和追踪。
	 */
	int nFromActor;

	/**
	 * @brief 消息数据
	 * 
	 * 指向具体的任务数据，包含了需要处理的业务逻辑信息。
	 */
	NFTask* pData;
};

/**
 * @class NFTaskActor
 * @brief 任务Actor基类
 *
 * NFTaskActor基于Theron框架实现的Actor模式任务处理器：
 * 
 * Actor模式核心：
 * - 独立线程：每个Actor在独立线程中执行
 * - 消息驱动：通过消息传递进行通信和任务分发
 * - 状态隔离：Actor间不共享状态，避免竞争条件
 * - 异步处理：非阻塞的任务执行和结果返回
 * 
 * 任务处理功能：
 * - 任务接收：接收来自主线程或其他Actor的任务
 * - 异步执行：在独立线程中执行耗时任务
 * - 结果返回：将处理结果返回给调用方
 * - 组件扩展：支持挂载不同的功能组件
 * 
 * 生命周期管理：
 * - 初始化：Actor的启动和资源准备
 * - 任务处理：持续的任务接收和处理循环
 * - 超时检测：检测和处理长时间运行的任务
 * - 清理关闭：安全的资源释放和Actor关闭
 * 
 * 高级特性：
 * - 组件系统：可插拔的功能组件架构
 * - 超时管理：任务执行超时的检测和处理
 * - 性能统计：任务处理性能的统计和监控
 * - 错误处理：异常情况的捕获和恢复
 * 
 * 应用场景：
 * - 数据库操作：避免阻塞主线程的数据库I/O
 * - 网络请求：异步的HTTP/TCP请求处理
 * - 文件操作：大文件的读写和处理
 * - 计算任务：CPU密集型的计算处理
 * - 业务逻辑：复杂的业务流程处理
 * 
 * @note 继承自Theron::Actor，提供工业级Actor实现
 * @note 需要在构造函数中调用RegisterHandler注册消息处理函数
 * @warning Actor处理函数应避免长时间阻塞，影响消息处理效率
 */
class NFTaskActor : public Theron::Actor
{
public:
	/**
	 * @brief 构造函数
	 * @param framework Theron框架引用
	 * @param pActorMgr Actor管理器指针
	 * 
	 * 初始化Actor实例并注册消息处理函数。
	 * 
	 * @note 必须调用RegisterHandler注册异步过程中用来处理的函数
	 */
	NFTaskActor(Theron::Framework& framework, NFTaskGroup* pActorMgr);

	/**
	 * @brief 析构函数
	 * 
	 * 清理Actor相关资源和组件。
	 */
	virtual ~NFTaskActor();

	/**
	 * @brief 初始化Actor
	 * @return 返回true表示初始化成功，false表示失败
	 * 
	 * 执行Actor的初始化操作，准备处理任务。
	 */
	virtual bool Init();

	/**
	 * @brief 向另外一个Actor发送消息
	 * @param address 接收消息的Actor地址
	 * @param message 要发送的消息
	 * @return 返回true表示发送成功，false表示发送失败
	 * 
	 * 实现Actor间的消息传递机制。
	 */
	virtual bool SendMsg(const Theron::Address& address, const NFTaskActorMessage& message);

	/**
	 * @brief 添加组件
	 * @param pComponent 要添加的组件指针
	 * @return 返回true表示添加成功，false表示失败
	 * 
	 * 为Actor添加功能组件，扩展其处理能力。
	 */
	virtual bool AddComponnet(NFITaskComponent* pComponent);

	/**
	 * @brief 获取Actor的组件
	 * @return 返回组件指针，没有组件返回nullptr
	 * 
	 * 获取Actor当前关联的功能组件。
	 */
	virtual NFITaskComponent* GetTaskComponent();

	/**
	 * @brief 获取Actor ID
	 * @return 返回Actor的唯一标识ID
	 * 
	 * 获取此Actor在系统中的唯一标识符。
	 */
	virtual int GetActorId() const;

protected:
	/**
	 * @brief 处理已经被处理过的消息
	 * @param message 要处理的消息
	 * @param from 发送消息的Actor地址
	 * 
	 * 处理从其他Actor返回的已处理消息，通常用于结果回调。
	 */
	virtual void HandlerEx(const NFTaskActorMessage& message, Theron::Address from);

	/**
	 * @brief 处理发送的数据
	 * @param message 要处理的消息
	 * @param from 发送消息的Actor地址
	 * 
	 * 处理接收到的新任务消息，执行具体的业务逻辑。
	 */
	virtual void Handler(const NFTaskActorMessage& message, Theron::Address from);

public:
	/**
	 * @brief 任务处理前的预处理
	 * @param pTask 要处理的任务对象
	 * 
	 * 在任务正式处理前执行的预处理操作，如资源准备、状态检查等。
	 */
	virtual void ProcessTaskStart(NFTask* pTask);

	/**
	 * @brief 任务处理后的后处理
	 * @param pTask 已处理的任务对象
	 * 
	 * 在任务处理完成后执行的后处理操作，如结果整理、资源清理等。
	 */
	virtual void ProcessTaskEnd(NFTask* pTask);

	/**
	 * @brief 异步处理任务系统
	 * @param pTask 要处理的任务对象
	 * 
	 * 核心的任务处理方法，在Actor线程中异步执行具体的业务逻辑。
	 */
	virtual void ProcessTask(NFTask* pTask);

	/**
	 * @brief 检查超时任务
	 * 
	 * 检查是否有任务执行超时，进行相应的超时处理。
	 * 
	 * @note 此方法在主线程中调用
	 */
	virtual void CheckTimeoutTask();

protected:
	/**
	 * @brief 默认的异步处理过程
	 * @param message 要处理的消息
	 * @param from 发送消息的Actor地址
	 * 
	 * 默认的消息处理方法，处理标准的任务分发和执行流程。
	 */
	virtual void DefaultHandler(const NFTaskActorMessage& message, Theron::Address from);

protected:
	/**
	 * @brief Actor管理器指针
	 * 
	 * 指向管理此Actor的TaskGroup实例，用于与管理器通信。
	 */
	NFTaskGroup* m_pTaskModule;

	/**
	 * @brief 组件管理指针
	 * 
	 * 指向Actor关联的功能组件，提供扩展的业务处理能力。
	 */
	NFITaskComponent* m_pComponent;
};
