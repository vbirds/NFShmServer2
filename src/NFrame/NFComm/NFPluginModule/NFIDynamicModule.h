// -------------------------------------------------------------------------
//    @FileName         :    NFIDynamicModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDynamicModule
//    @Description      :    动态模块基础类，提供热更新和运行时模块管理能力
//                           支持动态加载、消息处理、RPC服务和事件系统集成
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFConfigDefine.h"
#include "NFEventObj.h"
#include "NFITimerEventModule.h"

/**
 * @brief 动态模块基础类
 * 
 * NFIDynamicModule 为可动态加载的模块提供了完整的基础功能：
 * 
 * 1. 设计理念：
 *    - 热更新支持：支持运行时模块的动态加载和卸载
 *    - 多重继承：继承定时器和事件模块，提供完整功能
 *    - 统一接口：为动态模块提供标准化的接口和行为
 *    - 安全管理：确保动态加载过程的稳定性和安全性
 * 
 * 2. 核心功能：
 *    - 定时器管理：继承NFITimerEventModule的定时器功能
 *    - 事件处理：集成事件系统，支持事件订阅和处理
 *    - 消息处理：支持客户端和服务器消息的注册和处理
 *    - RPC服务：提供类型安全的RPC服务注册和调用
 *    - 任务管理：集成异步任务系统，支持应用任务的管理
 * 
 * 3. 动态加载特性：
 *    - 模块生命周期：完整的加载、初始化、运行、卸载流程
 *    - 状态保持：动态加载时保持必要的运行状态
 *    - 依赖管理：处理模块间的依赖关系
 *    - 版本控制：支持模块版本的管理和升级
 * 
 * 4. 消息处理系统：
 *    - 客户端消息：注册和处理来自客户端的消息
 *    - 服务器消息：处理服务器间的通信消息
 *    - 模块化消息：支持按模块ID分类的消息处理
 *    - 协程支持：可选的协程化消息处理
 * 
 * 5. RPC集成：
 *    - 类型安全：编译时类型检查的RPC调用
 *    - 自动序列化：Protocol Buffer消息的自动处理
 *    - 异步支持：支持异步RPC调用和响应
 *    - 错误处理：完善的RPC错误处理机制
 * 
 * 6. 任务系统集成：
 *    - 任务注册：注册应用级别的任务类型
 *    - 任务查询：检查任务的存在状态
 *    - 任务完成：标记任务完成状态
 *    - 任务分组：支持任务的分组管理
 * 
 * 使用场景：
 * - 游戏逻辑模块：可热更新的游戏逻辑实现
 * - 业务模块：独立的业务功能模块
 * - 扩展模块：第三方或可选功能模块
 * - 测试模块：开发和测试阶段的临时模块
 * 
 * 使用示例：
 * @code
 * class GameLogicModule : public NFIDynamicModule {
 * public:
 *     GameLogicModule(NFIPluginManager* p) : NFIDynamicModule(p) {
 *         // 注册客户端消息
 *         RegisterClientMessage(NF_ST_GAME, PLAYER_LOGIN_REQ);
 *         
 *         // 注册RPC服务
 *         AddRpcService<PLAYER_DATA_RPC, PlayerDataReq, PlayerDataRsp>(NF_ST_GAME);
 *         
 *         // 注册应用任务
 *         RegisterAppTask(NF_ST_GAME, TASK_INIT_PLAYER, "初始化玩家", 0);
 *     }
 * 
 *     virtual int OnHandleClientMessage(uint64_t unLinkId, NFDataPackage& packet) override {
 *         // 处理客户端消息
 *         return 0;
 *     }
 * 
 *     virtual int OnTimer(uint32_t nTimerID) override {
 *         // 处理定时器事件
 *         return 0;
 *     }
 * 
 *     virtual int OnExecute(uint32_t serverType, uint32_t nEventID, 
 *                          uint32_t bySrcType, uint64_t nSrcID, 
 *                          const google::protobuf::Message* pMessage) override {
 *         // 处理事件
 *         return 0;
 *     }
 * };
 * @endcode
 * 
 * 设计优势：
 * - 降低了动态模块开发的复杂性
 * - 提供了统一的模块接口和行为
 * - 简化了消息处理和RPC服务的实现
 * - 支持现代C++的开发模式
 */
class NFIDynamicModule : public NFITimerEventModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 * 
	 * 初始化动态模块的基本状态：
	 * - 调用基类构造函数
	 * - 设置插件管理器引用
	 * - 准备模块的运行环境
	 * - 初始化必要的成员变量
	 */
	NFIDynamicModule(NFIPluginManager* p);

	/**
	 * @brief 虚析构函数
	 * 
	 * 清理动态模块的所有资源：
	 * - 取消所有消息注册
	 * - 清理RPC服务注册
	 * - 取消事件订阅
	 * - 释放相关资源
	 */
	virtual ~NFIDynamicModule();

	/**
	 * @brief 定时器回调函数
	 * @param nTimerID 定时器ID
	 * @return 处理结果，0表示成功
	 * 
	 * 重写基类的定时器处理函数：
	 * - 由定时器系统自动调用
	 * - 子类可以重写实现自定义逻辑
	 * - 支持多个定时器的并发处理
	 * - 提供定时器的精确控制
	 */
	virtual int OnTimer(uint32_t nTimerID) override;

	/**
	 * @brief 事件处理函数
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 事件源类型
	 * @param nSrcID 事件源ID
	 * @param pMessage 事件消息指针
	 * @return 处理结果，0表示成功
	 * 
	 * 重写基类的事件处理函数：
	 * - 由事件系统自动调用
	 * - 子类可以重写实现事件处理逻辑
	 * - 支持各种类型的系统事件
	 * - 提供类型安全的消息处理
	 */
	virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage) override;

	/**
	 * @brief 注册应用任务
	 * @param eServerType 服务器类型
	 * @param taskType 任务类型ID
	 * @param desc 任务描述信息
	 * @param taskGroup 任务分组ID，默认为0
	 * @return 注册结果，0表示成功
	 * 
	 * 向插件管理器注册一个应用级任务：
	 * - 用于跟踪应用的初始化进度
	 * - 支持任务分组管理
	 * - 提供任务描述便于调试
	 * - 与任务系统集成
	 * 
	 * 任务类型：
	 * - 初始化任务：系统启动时的初始化步骤
	 * - 配置任务：配置加载和验证任务
	 * - 连接任务：网络连接建立任务
	 * - 数据任务：数据库连接和数据加载任务
	 * 
	 * 使用示例：
	 * @code
	 * // 注册初始化任务
	 * RegisterAppTask(NF_ST_GAME, TASK_INIT_CONFIG, "加载配置文件", 0);
	 * RegisterAppTask(NF_ST_GAME, TASK_INIT_DB, "连接数据库", 1);
	 * @endcode
	 */
	virtual int RegisterAppTask(NF_SERVER_TYPE eServerType, uint32_t taskType, const std::string& desc,
								uint32_t taskGroup)
	{
		return m_pObjPluginManager->RegisterAppTask(eServerType, taskType, desc, taskGroup);
	}

	/**
	 * @brief 检查应用任务是否存在
	 * @param eServerType 服务器类型
	 * @param taskGroup 任务分组ID
	 * @param taskType 任务类型ID
	 * @return 任务存在返回true，否则返回false
	 * 
	 * 查询指定的应用任务是否已经注册：
	 * - 用于任务依赖关系的检查
	 * - 避免重复注册相同任务
	 * - 支持条件性的任务注册
	 * - 便于任务状态的查询
	 * 
	 * 使用场景：
	 * - 检查前置任务是否完成
	 * - 验证任务注册状态
	 * - 实现任务间的依赖逻辑
	 * 
	 * 使用示例：
	 * @code
	 * if (IsHasAppTask(NF_ST_GAME, 0, TASK_INIT_CONFIG)) {
	 *     NFLogInfo(NF_LOG_DEFAULT, 0, "配置初始化任务已注册");
	 * }
	 * @endcode
	 */
	virtual bool IsHasAppTask(NF_SERVER_TYPE eServerType, uint32_t taskGroup, uint32_t taskType) const
	{
		return m_pObjPluginManager->IsHasAppTask(eServerType, taskGroup, taskType);
	}

	/**
	 * @brief 完成应用任务
	 * @param eServerType 服务器类型
	 * @param taskType 任务类型ID
	 * @param taskGroup 任务分组ID
	 * @return 操作结果，0表示成功
	 * 
	 * 标记指定的应用任务已完成：
	 * - 更新任务的完成状态
	 * - 通知依赖任务可以继续
	 * - 用于任务进度的跟踪
	 * - 支持任务完成回调
	 * 
	 * 完成流程：
	 * 1. 验证任务的存在性
	 * 2. 更新任务状态为已完成
	 * 3. 检查依赖任务的执行条件
	 * 4. 触发相关的完成事件
	 * 
	 * 使用示例：
	 * @code
	 * // 完成配置加载任务
	 * if (LoadConfigSuccess()) {
	 *     FinishAppTask(NF_ST_GAME, TASK_INIT_CONFIG, 0);
	 * }
	 * @endcode
	 */
	virtual int FinishAppTask(NF_SERVER_TYPE eServerType, uint32_t taskType,
							  uint32_t taskGroup)
	{
		return m_pObjPluginManager->FinishAppTask(eServerType, taskType, taskGroup);
	}

	/**
	 * @brief 添加RPC服务（模板函数）
	 * @tparam msgId 消息ID，编译时常量
	 * @tparam RequestType 请求消息类型
	 * @tparam ResponeType 响应消息类型
	 * @param serverType 服务器类型
	 * @param createCo 是否创建协程，默认为false
	 * @return 注册成功返回true，失败返回false
	 * 
	 * 注册一个类型安全的RPC服务：
	 * - 编译时类型检查确保类型安全
	 * - 自动处理消息的序列化和反序列化
	 * - 支持协程化的RPC处理
	 * - 提供统一的RPC服务接口
	 * 
	 * 模板参数：
	 * - msgId: 编译时确定的消息ID常量
	 * - RequestType: Protocol Buffer请求消息类型
	 * - ResponeType: Protocol Buffer响应消息类型
	 * 
	 * RPC特性：
	 * - 类型安全：编译时检查消息类型匹配
	 * - 自动路由：根据服务器类型自动路由
	 * - 错误处理：内置的错误处理和超时机制
	 * - 性能优化：高效的消息序列化和传输
	 * 
	 * 使用示例：
	 * @code
	 * // 注册玩家数据RPC服务
	 * AddRpcService<PLAYER_DATA_RPC, PlayerDataReq, PlayerDataRsp>(NF_ST_GAME, true);
	 * 
	 * // 注册物品操作RPC服务
	 * AddRpcService<ITEM_OPERATE_RPC, ItemOperateReq, ItemOperateRsp>(NF_ST_GAME);
	 * @endcode
	 */
	template<size_t msgId, typename RequestType, typename ResponeType>
	bool AddRpcService(NF_SERVER_TYPE serverType, bool createCo = false)
	{
		return FindModule<NFIMessageModule>()->AddRpcService<msgId, NFIDynamicModule, RequestType, ResponeType>(serverType, this, &NFIDynamicModule::OnHandleRpcMessage, createCo);
	}

	/**
	 * @brief 注册客户端消息处理函数
	 * @param eType 服务器类型
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return 注册成功返回true，失败返回false
	 * 
	 * 注册处理来自客户端消息的回调函数：
	 * - 绑定到OnHandleClientMessage方法
	 * - 支持协程化的消息处理
	 * - 提供统一的客户端消息接口
	 * - 自动处理消息的反序列化
	 * 
	 * 消息处理流程：
	 * 1. 网络层接收客户端消息
	 * 2. 消息系统根据消息ID路由到注册的处理函数
	 * 3. 调用OnHandleClientMessage进行具体处理
	 * 4. 返回处理结果给客户端
	 * 
	 * 协程支持：
	 * - createCo=true时，在独立协程中处理消息
	 * - 支持异步操作，如数据库查询
	 * - 提高系统的并发处理能力
	 * 
	 * 使用示例：
	 * @code
	 * // 注册玩家登录消息
	 * RegisterClientMessage(NF_ST_GAME, PLAYER_LOGIN_REQ, true);
	 * 
	 * // 注册聊天消息
	 * RegisterClientMessage(NF_ST_GAME, CHAT_MESSAGE_REQ);
	 * @endcode
	 */
	virtual bool RegisterClientMessage(NF_SERVER_TYPE eType, uint32_t nMsgID, bool createCo = false);

	/**
	 * @brief 注册带模块ID的客户端消息处理函数
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return 注册成功返回true，失败返回false
	 * 
	 * 注册按模块分类的客户端消息处理：
	 * - 支持模块级别的消息分组
	 * - 便于大型系统的消息管理
	 * - 提供更细粒度的消息控制
	 * - 支持模块间的消息隔离
	 * 
	 * 模块化消息优势：
	 * - 清晰的消息分类和组织
	 * - 避免消息ID冲突
	 * - 便于模块的独立开发和测试
	 * - 支持动态模块的消息管理
	 * 
	 * 使用示例：
	 * @code
	 * // 注册背包模块的消息
	 * RegisterClientMessageWithModule(NF_ST_GAME, MODULE_BAG, BAG_GET_INFO_REQ);
	 * 
	 * // 注册任务模块的消息
	 * RegisterClientMessageWithModule(NF_ST_GAME, MODULE_TASK, TASK_LIST_REQ);
	 * @endcode
	 */
	virtual bool RegisterClientMessageWithModule(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, bool createCo = false);

	/**
	 * @brief 注册服务器消息处理函数
	 * @param eType 服务器类型
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return 注册成功返回true，失败返回false
	 * 
	 * 注册处理来自其他服务器消息的回调函数：
	 * - 绑定到OnHandleServerMessage方法
	 * - 支持服务器间的通信
	 * - 提供统一的服务器消息接口
	 * - 支持分布式系统的消息处理
	 * 
	 * 服务器消息特点：
	 * - 高可靠性：确保消息的可靠传递
	 * - 低延迟：优化的服务器间通信
	 * - 安全性：内置的身份验证和加密
	 * - 监控支持：完整的消息监控和日志
	 * 
	 * 使用示例：
	 * @code
	 * // 注册服务器状态同步消息
	 * RegisterServerMessage(NF_ST_GAME, SERVER_STATUS_SYNC, true);
	 * 
	 * // 注册跨服数据传输消息
	 * RegisterServerMessage(NF_ST_GAME, CROSS_SERVER_DATA_REQ);
	 * @endcode
	 */
	virtual bool RegisterServerMessage(NF_SERVER_TYPE eType, uint32_t nMsgID, bool createCo = false);

	/**
	 * @brief 注册带模块ID的服务器消息处理函数
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return 注册成功返回true，失败返回false
	 * 
	 * 注册按模块分类的服务器消息处理：
	 * - 支持模块级别的服务器消息管理
	 * - 便于分布式系统的模块化设计
	 * - 提供模块间的消息隔离
	 * - 支持大规模服务器集群的消息管理
	 * 
	 * 使用示例：
	 * @code
	 * // 注册数据库模块的服务器消息
	 * RegisterServerMessageWithModule(NF_ST_GAME, MODULE_DB, DB_SYNC_REQ);
	 * @endcode
	 */
	virtual bool RegisterServerMessageWithModule(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, bool createCo = false);

	/**
	 * @brief 处理客户端消息
	 * @param unLinkId 连接ID，标识客户端连接
	 * @param packet 数据包，包含消息内容
	 * @return 处理结果，0表示成功
	 * 
	 * 客户端消息的统一处理入口：
	 * - 由消息系统自动调用
	 * - 子类可以重写实现具体逻辑
	 * - 支持各种类型的客户端消息
	 * - 提供连接管理和状态跟踪
	 * 
	 * 处理流程：
	 * 1. 验证连接的有效性
	 * 2. 解析消息包的内容
	 * 3. 执行具体的业务逻辑
	 * 4. 返回处理结果或响应消息
	 * 
	 * 使用示例：
	 * @code
	 * virtual int OnHandleClientMessage(uint64_t unLinkId, NFDataPackage& packet) override {
	 *     switch(packet.nMsgId) {
	 *         case PLAYER_LOGIN_REQ:
	 *             return HandlePlayerLogin(unLinkId, packet);
	 *         case CHAT_MESSAGE_REQ:
	 *             return HandleChatMessage(unLinkId, packet);
	 *         default:
	 *             return -1;
	 *     }
	 * }
	 * @endcode
	 */
	virtual int OnHandleClientMessage(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理客户端消息（带附加参数版本）
	 * @param msgId 消息ID
	 * @param packet 数据包
	 * @param param1 附加参数1
	 * @param param2 附加参数2
	 * @return 处理结果，0表示成功
	 * 
	 * 支持附加参数的客户端消息处理：
	 * - 提供更多的上下文信息
	 * - 支持复杂的消息处理场景
	 * - 便于传递额外的处理参数
	 * - 兼容不同的消息处理模式
	 */
	virtual int OnHandleClientMessage(uint32_t msgId, NFDataPackage& packet, uint64_t param1, uint64_t param2);

	/**
	 * @brief 处理来自服务器的消息
	 * @param unLinkId 连接ID，标识服务器连接
	 * @param packet 数据包，包含消息内容
	 * @return 处理结果，0表示成功
	 * 
	 * 服务器消息的统一处理入口：
	 * - 处理来自其他服务器的消息
	 * - 支持分布式系统的服务器间通信
	 * - 提供服务器连接的管理
	 * - 支持高可靠性的消息传递
	 * 
	 * 使用示例：
	 * @code
	 * virtual int OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet) override {
	 *     switch(packet.nMsgId) {
	 *         case SERVER_STATUS_SYNC:
	 *             return HandleServerStatusSync(unLinkId, packet);
	 *         case CROSS_SERVER_DATA_REQ:
	 *             return HandleCrossServerData(unLinkId, packet);
	 *         default:
	 *             return -1;
	 *     }
	 * }
	 * @endcode
	 */
	virtual int OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理来自服务器的消息（带附加参数版本）
	 * @param msgId 消息ID
	 * @param packet 数据包
	 * @param param1 附加参数1
	 * @param param2 附加参数2
	 * @return 处理结果，0表示成功
	 * 
	 * 支持附加参数的服务器消息处理，便于传递额外的上下文信息。
	 */
	virtual int OnHandleServerMessage(uint32_t msgId, NFDataPackage& packet, uint64_t param1, uint64_t param2);

	/**
	 * @brief 处理RPC消息
	 * @param msgId 消息ID
	 * @param request 请求消息引用
	 * @param respone 响应消息引用
	 * @param param1 附加参数1
	 * @param param2 附加参数2
	 * @return 处理结果，0表示成功
	 * 
	 * RPC消息的统一处理入口：
	 * - 处理类型安全的RPC调用
	 * - 自动处理请求和响应的序列化
	 * - 支持同步和异步的RPC处理
	 * - 提供完整的错误处理机制
	 * 
	 * RPC处理特点：
	 * - 类型安全：编译时检查消息类型
	 * - 自动路由：根据消息ID自动分发
	 * - 错误处理：内置的超时和错误处理
	 * - 性能优化：高效的消息序列化
	 * 
	 * 处理模式：
	 * - 同步处理：直接在当前线程中处理
	 * - 异步处理：在协程或工作线程中处理
	 * - 批量处理：支持批量RPC请求
	 * 
	 * 使用示例：
	 * @code
	 * virtual int OnHandleRpcMessage(uint32_t msgId, google::protobuf::Message& request, 
	 *                               google::protobuf::Message& respone, 
	 *                               uint64_t param1, uint64_t param2) override {
	 *     switch(msgId) {
	 *         case PLAYER_DATA_RPC:
	 *             return HandlePlayerDataRpc(request, respone, param1, param2);
	 *         case ITEM_OPERATE_RPC:
	 *             return HandleItemOperateRpc(request, respone, param1, param2);
	 *         default:
	 *             return -1;
	 *     }
	 * }
	 * @endcode
	 */
	virtual int OnHandleRpcMessage(uint32_t msgId, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1, uint64_t param2);
};
