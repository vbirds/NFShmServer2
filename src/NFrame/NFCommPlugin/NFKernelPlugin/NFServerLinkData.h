// -------------------------------------------------------------------------
//    @FileName         :    NFServerLinkData.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerLinkData
//    @Description      :    服务器连接数据头文件，提供网络消息处理和服务器连接管理的数据结构
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFINetModule.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include <stdint.h>

/**
 * @struct NetRpcService
 * @brief 网络RPC服务数据结构
 *
 * NetRpcService封装了RPC服务的调用信息和性能统计数据：
 * 
 * RPC服务功能：
 * - 服务绑定：绑定目标模块和RPC服务实例
 * - 协程支持：支持协程模式的RPC调用
 * - 性能统计：统计调用次数、时间等性能指标
 * - 自动管理：自动管理RPC服务的生命周期
 * 
 * 性能监控：
 * - 调用计数：统计RPC服务的调用次数
 * - 时间统计：记录最小、最大、总执行时间
 * - 性能分析：提供RPC调用的性能分析数据
 * - 监控报告：生成RPC服务的监控报告
 * 
 * 应用场景：
 * - 远程过程调用：跨服务器的远程方法调用
 * - 分布式通信：分布式系统间的服务调用
 * - 微服务架构：微服务间的接口调用
 * - 性能监控：RPC服务的性能监控和分析
 * 
 * @note 支持协程和非协程两种执行模式
 * @note 自动统计性能数据用于监控和优化
 */
struct NetRpcService
{
	/**
	 * @brief 默认构造函数
	 * 
	 * 初始化RPC服务结构体，设置默认值和性能统计初始状态。
	 */
	NetRpcService()
	{
		m_pTarget = nullptr;
		m_pRpcService = nullptr;
		m_createCo = false;
		m_iCount = 0;
		m_iAllUseTime = 0;
		m_iMinTime = 1000000000;
		m_iMaxTime = 0;
	}

	/**
	 * @brief 完整构造函数
	 * @param pTarget 目标模块指针
	 * @param pRpcService RPC服务接口指针
	 * @param createCo 是否创建协程执行
	 * 
	 * 创建包含目标模块和RPC服务的完整RPC服务结构。
	 */
	NetRpcService(NFIDynamicModule* pTarget, NFIRpcService* pRpcService, bool createCo): m_pTarget(pTarget),
	                                                                                     m_pRpcService(pRpcService),
	                                                                                     m_createCo(createCo)
	{
		m_iCount = 0;
		m_iAllUseTime = 0;
		m_iMinTime = 1000000000;
		m_iMaxTime = 0;
	}

	/**
	 * @brief 简化构造函数
	 * @param pRpcService RPC服务接口指针
	 * @param createCo 是否创建协程执行
	 * 
	 * 创建只包含RPC服务的简化结构，目标模块为空。
	 */
	NetRpcService(NFIRpcService* pRpcService, bool createCo): m_pTarget(nullptr),
	                                                          m_pRpcService(pRpcService),
	                                                          m_createCo(createCo)
	{
		m_iCount = 0;
		m_iAllUseTime = 0;
		m_iMinTime = 1000000000;
		m_iMaxTime = 0;
	}

	/**
	 * @brief 拷贝构造函数
	 * @param functor 要拷贝的RPC服务结构
	 * 
	 * 深拷贝RPC服务结构，包括所有性能统计数据。
	 */
	NetRpcService(const NetRpcService& functor)
	{
		if (this != &functor)
		{
			m_pTarget = functor.m_pTarget;
			m_pRpcService = functor.m_pRpcService;
			m_createCo = functor.m_createCo;
			m_iCount = functor.m_iCount;;
			m_iAllUseTime = functor.m_iAllUseTime;
			m_iMinTime = functor.m_iMinTime;
			m_iMaxTime = functor.m_iMaxTime;
		}
	}

	/**
	 * @brief 赋值操作符
	 * @param functor 要赋值的RPC服务结构
	 * @return 返回当前对象引用
	 * 
	 * 赋值操作，复制所有成员变量包括性能统计数据。
	 */
	NetRpcService& operator=(const NetRpcService& functor)
	{
		if (this != &functor)
		{
			m_pTarget = functor.m_pTarget;
			m_pRpcService = functor.m_pRpcService;
			m_createCo = functor.m_createCo;
			m_iCount = functor.m_iCount;;
			m_iAllUseTime = functor.m_iAllUseTime;
			m_iMinTime = functor.m_iMinTime;
			m_iMaxTime = functor.m_iMaxTime;
		}

		return *this;
	}

	NFIDynamicModule* m_pTarget;        ///< 目标动态模块指针
	NFIRpcService* m_pRpcService;       ///< RPC服务接口指针
	bool m_createCo;                    ///< 是否创建协程执行标志
	uint64_t m_iCount;                  ///< 调用次数统计
	uint64_t m_iAllUseTime;             ///< 总执行时间统计（微秒）
	uint64_t m_iMinTime;                ///< 最小执行时间（微秒）
	uint64_t m_iMaxTime;                ///< 最大执行时间（微秒）
};

/**
 * @struct NetReceiveFunctor
 * @brief 网络消息接收函数器
 *
 * NetReceiveFunctor封装了网络消息接收的回调函数和性能统计：
 * 
 * 消息处理功能：
 * - 回调绑定：绑定消息处理的目标模块和回调函数
 * - 协程支持：支持协程模式的消息处理
 * - 性能统计：统计消息处理的性能指标
 * - 自动管理：自动管理回调函数的生命周期
 * 
 * 性能监控：
 * - 消息计数：统计处理的消息数量
 * - 时间统计：记录消息处理的时间分布
 * - 性能分析：提供消息处理的性能数据
 * - 瓶颈识别：识别消息处理的性能瓶颈
 * 
 * 应用场景：
 * - 网络消息处理：处理来自网络的各种消息
 * - 协议解析：解析和处理网络协议数据
 * - 业务分发：将消息分发到对应的业务处理模块
 * - 性能监控：监控消息处理的性能表现
 * 
 * @note 支持协程和非协程两种处理模式
 * @note 自动统计消息处理性能用于优化
 */
struct NetReceiveFunctor
{
	/**
	 * @brief 默认构造函数
	 * 
	 * 初始化网络接收函数器，设置默认值和性能统计初始状态。
	 */
	NetReceiveFunctor()
	{
		m_pTarget = nullptr;
		m_pFunctor = nullptr;
		m_iCount = 0;
		m_iAllUseTime = 0;
		m_iMinTime = 1000000000;
		m_iMaxTime = 0;
		m_createCo = false;
	}

	/**
	 * @brief 完整构造函数
	 * @param pTarget 目标模块指针
	 * @param functor 网络接收回调函数
	 * @param createCo 是否创建协程处理
	 * 
	 * 创建包含目标模块和回调函数的完整接收函数器。
	 */
	NetReceiveFunctor(NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& functor, bool createCo): m_pTarget(pTarget),
	                                                                                                 m_pFunctor(functor),
	                                                                                                 m_createCo(createCo)
	{
		m_iCount = 0;
		m_iAllUseTime = 0;
		m_iMinTime = 1000000000;
		m_iMaxTime = 0;
	}

	/**
	 * @brief 简化构造函数
	 * @param functor 网络接收回调函数
	 * @param createCo 是否创建协程处理
	 * 
	 * 创建只包含回调函数的简化结构，目标模块为空。
	 */
	NetReceiveFunctor(const NET_RECEIVE_FUNCTOR& functor, bool createCo): m_pTarget(nullptr),
	                                                                      m_pFunctor(functor),
	                                                                      m_createCo(createCo)
	{
		m_iCount = 0;
		m_iAllUseTime = 0;
		m_iMinTime = 1000000000;
		m_iMaxTime = 0;
	}

	/**
	 * @brief 拷贝构造函数
	 * @param functor 要拷贝的接收函数器
	 * 
	 * 深拷贝接收函数器，包括所有性能统计数据。
	 */
	NetReceiveFunctor(const NetReceiveFunctor& functor)
	{
		if (this != &functor)
		{
			m_pTarget = functor.m_pTarget;
			m_pFunctor = functor.m_pFunctor;
			m_iCount = functor.m_iCount;;
			m_iAllUseTime = functor.m_iAllUseTime;
			m_iMinTime = functor.m_iMinTime;
			m_iMaxTime = functor.m_iMaxTime;
			m_createCo = functor.m_createCo;
		}
	}

	/**
	 * @brief 赋值操作符
	 * @param functor 要赋值的接收函数器
	 * @return 返回当前对象引用
	 * 
	 * 赋值操作，复制所有成员变量包括性能统计数据。
	 */
	NetReceiveFunctor& operator=(const NetReceiveFunctor& functor)
	{
		if (this != &functor)
		{
			m_pTarget = functor.m_pTarget;
			m_pFunctor = functor.m_pFunctor;
			m_iCount = functor.m_iCount;;
			m_iAllUseTime = functor.m_iAllUseTime;
			m_iMinTime = functor.m_iMinTime;
			m_iMaxTime = functor.m_iMaxTime;
			m_createCo = functor.m_createCo;
		}

		return *this;
	}

	NFIDynamicModule* m_pTarget;        ///< 目标动态模块指针
	NET_RECEIVE_FUNCTOR m_pFunctor;     ///< 网络接收回调函数
	uint64_t m_iCount;                  ///< 消息处理次数统计
	uint64_t m_iAllUseTime;             ///< 总处理时间统计（微秒）
	uint64_t m_iMinTime;                ///< 最小处理时间（微秒）
	uint64_t m_iMaxTime;                ///< 最大处理时间（微秒）
	bool m_createCo;                    ///< 是否创建协程处理标志
};

/**
 * @struct NetEventFunctor
 * @brief 网络事件函数器
 *
 * NetEventFunctor封装了网络事件处理的回调函数：
 * 
 * 事件处理功能：
 * - 事件绑定：绑定网络事件的目标模块和回调函数
 * - 协程支持：支持协程模式的事件处理
 * - 事件分发：将网络事件分发到对应的处理模块
 * - 自动管理：自动管理事件回调的生命周期
 * 
 * 网络事件类型：
 * - 连接事件：客户端连接和断开事件
 * - 错误事件：网络错误和异常事件
 * - 状态事件：网络状态变化事件
 * - 自定义事件：用户自定义的网络事件
 * 
 * 应用场景：
 * - 连接管理：处理客户端的连接和断开
 * - 错误处理：处理网络通信中的错误
 * - 状态监控：监控网络连接状态变化
 * - 事件通知：通知业务模块网络事件
 * 
 * @note 相比NetReceiveFunctor，专门用于事件处理
 * @note 不包含性能统计，专注于事件的快速响应
 */
struct NetEventFunctor
{
	/**
	 * @brief 默认构造函数
	 * 
	 * 初始化网络事件函数器，设置默认值。
	 */
	NetEventFunctor()
	{
		m_pTarget = nullptr;
		m_pFunctor = nullptr;
		m_createCo = false;
	}

	/**
	 * @brief 完整构造函数
	 * @param pTarget 目标模块指针
	 * @param functor 网络事件回调函数
	 * @param createCo 是否创建协程处理
	 * 
	 * 创建包含目标模块和回调函数的完整事件函数器。
	 */
	NetEventFunctor(NFIDynamicModule* pTarget, const NET_EVENT_FUNCTOR& functor, bool createCo): m_pTarget(pTarget),
	                                                                                             m_pFunctor(functor),
	                                                                                             m_createCo(createCo)
	{
	}

	/**
	 * @brief 拷贝构造函数
	 * @param functor 要拷贝的事件函数器
	 * 
	 * 深拷贝事件函数器的成员变量。
	 */
	NetEventFunctor(const NetEventFunctor& functor)
	{
		if (this != &functor)
		{
			m_pTarget = functor.m_pTarget;
			m_pFunctor = functor.m_pFunctor;
		}
	}

	/**
	 * @brief 赋值操作符
	 * @param functor 要赋值的事件函数器
	 * @return 返回当前对象引用
	 * 
	 * 赋值操作，复制所有成员变量。
	 */
	NetEventFunctor& operator=(const NetEventFunctor& functor)
	{
		if (this != &functor)
		{
			m_pTarget = functor.m_pTarget;
			m_pFunctor = functor.m_pFunctor;
		}

		return *this;
	}

	NFIDynamicModule* m_pTarget;        ///< 目标动态模块指针
	NET_EVENT_FUNCTOR m_pFunctor;       ///< 网络事件回调函数
	bool m_createCo;                    ///< 是否创建协程处理标志
};

/**
 * @struct CallBack
 * @brief 网络回调管理结构体
 *
 * CallBack是网络消息系统的核心回调管理器，统一管理所有类型的网络回调：
 * 
 * 回调管理功能：
 * - 消息回调：管理网络消息的接收处理回调
 * - 事件回调：管理网络连接事件的处理回调
 * - RPC回调：管理远程过程调用的处理回调
 * - HTTP回调：管理HTTP请求的处理回调
 * - 过滤器：管理HTTP消息的过滤器回调
 * 
 * 分类管理：
 * - 模块分类：按模块类型分类管理回调
 * - 消息分类：按消息ID分类管理回调
 * - 类型分类：按HTTP类型分类管理回调
 * - 通用回调：管理全局和其他类型的回调
 * 
 * 高级特性：
 * - 动态扩容：根据模块和消息类型动态调整容量
 * - 快速查找：基于哈希表和向量的快速回调查找
 * - 类型安全：强类型的回调函数管理
 * - 内存优化：预分配和动态管理的内存优化
 * 
 * 应用场景：
 * - 消息分发：将网络消息分发到对应的处理回调
 * - 事件处理：处理网络连接生命周期事件
 * - 服务调用：处理RPC服务的调用和响应
 * - Web服务：处理HTTP请求和响应
 * 
 * @note 支持客户端和服务器端两种不同的消息ID范围
 * @note 自动根据模块类型分配合适的消息ID容量
 */
struct CallBack
{
	/**
	 * @brief 构造函数
	 * 
	 * 初始化回调管理器，预分配各种类型的回调容器。
	 * 根据模块类型自动分配合适的消息ID范围。
	 */
	CallBack()
	{
		mxReceiveCallBack.resize(NF_MODULE_MAX);
		for (int i = 0; i < (int)mxReceiveCallBack.size(); i++)
		{
			if (i <= NF_MODULE_CLIENT)
			{
				mxReceiveCallBack[i].resize(NF_NET_MAX_MSG_ID);
			}
			else
			{
				mxReceiveCallBack[i].resize(NF_NET_OTHER_MAX_MSG_ID);
			}
		}

		mxRpcCallBack.resize(NF_MODULE_MAX);
		for (int i = 0; i < (int)mxRpcCallBack.size(); i++)
		{
			if (i <= NF_MODULE_CLIENT)
			{
				mxRpcCallBack[i].resize(NF_NET_MAX_MSG_ID);
			}
			else
			{
				mxRpcCallBack[i].resize(NF_NET_OTHER_MAX_MSG_ID);
			}
		}
	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 清理回调管理器资源。
	 */
	virtual ~CallBack()
	{
	}

	/**
	 * @brief 消息接收回调二维数组
	 * 
	 * 按模块类型和消息ID组织的消息接收回调数组。
	 * 第一维：模块类型索引
	 * 第二维：消息ID索引
	 */
	std::vector<std::vector<NetReceiveFunctor>> mxReceiveCallBack;
	
	/**
	 * @brief 网络事件回调映射表
	 * 
	 * 以事件ID为键的网络事件处理回调映射表。
	 * 用于处理连接、断开、错误等网络事件。
	 */
	std::unordered_map<uint64_t, NetEventFunctor> mxEventCallBack;
	
	/**
	 * @brief 其他消息回调列表
	 * 
	 * 不在标准消息ID范围内的特殊消息回调列表。
	 * 用于处理自定义或扩展的消息类型。
	 */
	std::unordered_map<uint64_t, NetReceiveFunctor> mxOtherMsgCallBackList;
	
	/**
	 * @brief 全局消息回调
	 * 
	 * 处理所有消息的通用回调，用于消息监控、日志等。
	 */
	NetReceiveFunctor mxAllMsgCallBackList;
	
	/**
	 * @brief HTTP消息回调映射表
	 * 
	 * 按HTTP类型和URL路径组织的HTTP请求处理回调。
	 * 外层键：HTTP类型（GET、POST等）
	 * 内层键：URL路径字符串
	 */
	std::unordered_map<uint32_t, std::unordered_map<std::string, HTTP_RECEIVE_FUNCTOR>> mxHttpMsgCBMap;
	
	/**
	 * @brief HTTP其他消息回调映射表
	 * 
	 * 按HTTP类型组织的通用HTTP请求处理回调列表。
	 * 用于处理未匹配到具体路径的HTTP请求。
	 */
	std::unordered_map<uint32_t, std::vector<HTTP_RECEIVE_FUNCTOR>> mxHttpOtherMsgCBMap;
	
	/**
	 * @brief HTTP消息过滤器映射表
	 * 
	 * 按过滤器名称组织的HTTP消息过滤器回调。
	 * 用于在消息处理前进行预处理和过滤。
	 */
	std::unordered_map<std::string, HTTP_FILTER_FUNCTOR> mxHttpMsgFliterMap;
	
	/**
	 * @brief RPC回调二维数组
	 * 
	 * 按模块类型和消息ID组织的RPC服务回调数组。
	 * 第一维：模块类型索引
	 * 第二维：消息ID索引
	 */
	std::vector<std::vector<NetRpcService>> mxRpcCallBack;
};

/**
 * @struct ServerLinkData
 * @brief 服务器连接数据管理结构体
 *
 * ServerLinkData是分布式服务器架构的核心数据管理器，负责管理所有服务器连接信息：
 * 
 * 服务器管理功能：
 * - 服务器注册：管理各类型服务器的注册信息
 * - 连接管理：管理服务器间的网络连接
 * - 路由管理：管理消息路由和转发信息
 * - 负载均衡：提供多种负载均衡策略
 * - 跨服支持：支持跨服务器通信管理
 * 
 * 数据组织方式：
 * - 类型分组：按服务器类型分组管理
 * - 一致性哈希：使用一致性哈希进行负载均衡
 * - 跨服区分：区分跨服和非跨服的服务器
 * - 数据库分类：按数据库名称分类管理DB服务器
 * - 快速查找：多种索引方式支持快速查找
 * 
 * 高级特性：
 * - 动态扩容：支持服务器的动态添加和移除
 * - 故障处理：自动处理服务器故障和恢复
 * - 主从架构：支持主服务器和路由服务器
 * - 连接池：管理服务器间的连接池
 * - 智能路由：基于多种策略的智能消息路由
 * 
 * 应用场景：
 * - 分布式游戏：大型多人在线游戏的服务器集群
 * - 微服务架构：微服务间的服务发现和通信
 * - 负载均衡：请求在多个服务器间的分发
 * - 跨区通信：不同区域服务器间的通信
 * - 数据分片：数据在多个数据库服务器间的分片
 * 
 * @note 支持多种负载均衡算法和路由策略
 * @note 自动管理服务器生命周期和连接状态
 * @warning 服务器数据变更需要注意线程安全
 */
struct ServerLinkData
{
	/**
	 * @brief 构造函数
	 * 
	 * 初始化服务器连接数据管理器，预分配各种类型的服务器容器。
	 */
	ServerLinkData()
	{
		mServerType = NF_ST_NONE;
		m_serverLinkId = 0;
		m_clientLinkId = 0;
		mServerList.resize(NF_ST_MAX);
		mServerListMap.resize(NF_ST_MAX);
		mCrossServerListMap.resize(NF_ST_MAX);
		mNoCrossServerListMap.resize(NF_ST_MAX);
	}

	/**
	 * @brief 服务器映射表
	 * 
	 * 以服务器ID为键的所有服务器数据映射表，提供快速的服务器查找。
	 */
	NFCommMapEx<uint32_t, NFServerData> mServerMap;
	
	/**
	 * @brief 服务器列表数组
	 * 
	 * 按服务器类型组织的服务器列表数组，每个类型对应一个服务器列表。
	 * 索引：服务器类型枚举值
	 */
	std::vector<std::vector<NF_SHARE_PTR<NFServerData>>> mServerList;
	
	/**
	 * @brief 服务器一致性哈希映射数组
	 * 
	 * 按服务器类型组织的一致性哈希映射，用于负载均衡和服务选择。
	 * 包含所有类型的服务器（跨服+非跨服）。
	 */
	std::vector<NFConsistentCommMapEx<uint32_t, NFServerData>> mServerListMap;
	
	/**
	 * @brief 跨服服务器一致性哈希映射数组
	 * 
	 * 只包含跨服服务器的一致性哈希映射，用于跨服通信的负载均衡。
	 */
	std::vector<NFConsistentCommMapEx<uint32_t, NFServerData>> mCrossServerListMap;
	
	/**
	 * @brief 非跨服服务器一致性哈希映射数组
	 * 
	 * 只包含非跨服服务器的一致性哈希映射，用于本服通信的负载均衡。
	 */
	std::vector<NFConsistentCommMapEx<uint32_t, NFServerData>> mNoCrossServerListMap;
	
	/**
	 * @brief 数据库服务器映射表
	 * 
	 * 按数据库名称组织的数据库服务器一致性哈希映射。
	 * 外层键：数据库名称
	 * 内层：该数据库的服务器一致性哈希映射
	 */
	NFCommMapEx<std::string, NFConsistentCommMapEx<uint32_t, NFServerData>> mDBStoreServerMap;
	
	/**
	 * @brief 连接ID到服务器ID映射表
	 * 
	 * 将网络连接ID映射到对应的服务器业务ID，用于快速定位连接对应的服务器。
	 */
	std::map<uint64_t, uint32_t> mLinkIdToBusIdMap;
	
	/**
	 * @brief 路由服务器数据
	 * 
	 * 当前服务器连接的路由服务器信息，用于消息转发和路由。
	 */
	NFServerData m_routeData;
	
	/**
	 * @brief 主服务器数据
	 * 
	 * 当前服务器连接的主服务器信息，用于集群管理和协调。
	 */
	NFServerData m_masterServerData;
	
	/**
	 * @brief 当前服务器类型
	 * 
	 * 标识当前服务器的类型，用于确定服务器的角色和功能。
	 */
	NF_SERVER_TYPE mServerType;
	
	/**
	 * @brief 服务器连接ID
	 * 
	 * 当前服务器作为服务器端的网络连接ID。
	 */
	uint64_t m_serverLinkId;
	
	/**
	 * @brief 客户端连接ID
	 * 
	 * 当前服务器作为客户端的网络连接ID。
	 */
	uint64_t m_clientLinkId;

	/**
	 * @brief 获取服务器连接ID
	 * @return 返回当前服务器的连接ID
	 */
	uint64_t GetServerLinkId() const;

	/**
	 * @brief 设置服务器连接ID
	 * @param linkId 要设置的连接ID
	 */
	void SetServerLinkId(uint64_t linkId);

	/**
	 * @brief 获取客户端连接ID
	 * @return 返回当前客户端的连接ID
	 */
	uint64_t GetClientLinkId() const;

	/**
	 * @brief 设置客户端连接ID
	 * @param linkId 要设置的连接ID
	 */
	void SetClientLinkId(uint64_t linkId);

	/**
	 * @brief 根据服务器ID获取服务器数据
	 * @param busId 服务器业务ID
	 * @return 返回服务器数据的智能指针，不存在则返回空指针
	 */
	NF_SHARE_PTR<NFServerData> GetServerByServerId(uint32_t busId);

	/**
	 * @brief 根据连接ID获取服务器数据
	 * @param unlinkId 网络连接ID
	 * @return 返回服务器数据的智能指针，不存在则返回空指针
	 */
	NF_SHARE_PTR<NFServerData> GetServerByUnlinkId(uint64_t unlinkId);

	/**
	 * @brief 创建服务器数据
	 * @param busId 服务器业务ID
	 * @param busServerType 服务器类型
	 * @param data 服务器信息报告
	 * @return 返回创建的服务器数据智能指针
	 */
	NF_SHARE_PTR<NFServerData> CreateServerByServerId(uint32_t busId, NF_SERVER_TYPE busServerType, const NFrame::ServerInfoReport& data);

	/**
	 * @brief 关闭服务器连接
	 * @param destServer 目标服务器类型
	 * @param busId 服务器业务ID
	 * @param usLinkId 连接ID
	 */
	virtual void CloseServer(NF_SERVER_TYPE destServer, uint32_t busId, uint64_t usLinkId);

	/**
	 * @brief 创建到服务器的连接
	 * @param busId 服务器业务ID
	 * @param linkId 连接ID
	 */
	virtual void CreateLinkToServer(uint32_t busId, uint64_t linkId);

	/**
	 * @brief 删除服务器连接
	 * @param linkId 要删除的连接ID
	 */
	void DelServerLink(uint64_t linkId);

	/**
	 * @brief 获取路由服务器数据
	 * @return 返回路由服务器数据指针
	 */
	NFServerData* GetRouteData() { return &m_routeData; }

	/**
	 * @brief 获取路由服务器数据（常量版本）
	 * @return 返回路由服务器数据常量指针
	 */
	const NFServerData* GetRouteData() const { return &m_routeData; }

	/**
	 * @brief 获取主服务器数据
	 * @return 返回主服务器数据指针
	 */
	NFServerData* GetMasterData() { return &m_masterServerData; }

	/**
	 * @brief 获取主服务器数据（常量版本）
	 * @return 返回主服务器数据常量指针
	 */
	const NFServerData* GetMasterData() const { return &m_masterServerData; }

	/**
	 * @brief 关闭所有连接
	 * @param pMessageModule 消息模块指针
	 */
	void CloseAllLink(NFIMessageModule* pMessageModule);

	/**
	 * @brief 发送消息到主服务器
	 * @param pMessageModule 消息模块指针
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param valueId 值ID，默认为0
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendMsgToMasterServer(NFIMessageModule* pMessageModule, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t valueId = 0);

	/**
	 * @brief 根据服务器类型获取所有服务器
	 * @param serverTypes 服务器类型
	 * @return 返回该类型所有服务器的列表
	 */
	std::vector<NF_SHARE_PTR<NFServerData>> GetServerByServerType(NF_SERVER_TYPE serverTypes);

	/**
	 * @brief 根据服务器类型获取第一个服务器
	 * @param serverTypes 服务器类型
	 * @return 返回该类型第一个服务器的智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetFirstServerByServerType(NF_SERVER_TYPE serverTypes);

	/**
	 * @brief 根据服务器类型获取第一个服务器（指定跨服属性）
	 * @param serverTypes 服务器类型
	 * @param crossServer 是否跨服
	 * @return 返回该类型第一个服务器的智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetFirstServerByServerType(NF_SERVER_TYPE serverTypes, bool crossServer);

	/**
	 * @brief 根据服务器类型随机获取服务器
	 * @param serverTypes 服务器类型
	 * @return 返回随机选择的服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetRandomServerByServerType(NF_SERVER_TYPE serverTypes);

	/**
	 * @brief 根据服务器类型随机获取服务器（指定跨服属性）
	 * @param serverTypes 服务器类型
	 * @param crossServer 是否跨服
	 * @return 返回随机选择的服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetRandomServerByServerType(NF_SERVER_TYPE serverTypes, bool crossServer);

	/**
	 * @brief 根据服务器类型和数值获取合适的服务器
	 * @param serverTypes 服务器类型
	 * @param value 用于一致性哈希的数值
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE serverTypes, uint64_t value);

	/**
	 * @brief 根据服务器类型和数值获取合适的服务器（指定跨服属性）
	 * @param serverTypes 服务器类型
	 * @param value 用于一致性哈希的数值
	 * @param crossServer 是否跨服
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE serverTypes, uint64_t value, bool crossServer);

	/**
	 * @brief 根据服务器类型和字符串获取合适的服务器
	 * @param serverTypes 服务器类型
	 * @param value 用于一致性哈希的字符串
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE serverTypes, const std::string& value);

	/**
	 * @brief 根据服务器类型和字符串获取合适的服务器（指定跨服属性）
	 * @param serverTypes 服务器类型
	 * @param value 用于一致性哈希的字符串
	 * @param crossServer 是否跨服
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE serverTypes, const std::string& value, bool crossServer);

	/**
	 * @brief 获取第一个数据库服务器
	 * @param dbName 数据库名称
	 * @return 返回第一个数据库服务器的智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetFirstDbServer(const std::string& dbName);
	
	/**
	 * @brief 随机获取数据库服务器
	 * @param dbName 数据库名称
	 * @return 返回随机选择的数据库服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GeRandomDbServer(const std::string& dbName);
	
	/**
	 * @brief 根据数值获取合适的数据库服务器
	 * @param dbName 数据库名称
	 * @param value 用于一致性哈希的数值
	 * @return 返回通过一致性哈希选择的数据库服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetSuitDbServer(const std::string& dbName, uint64_t value);
	
	/**
	 * @brief 根据字符串获取合适的数据库服务器
	 * @param dbName 数据库名称
	 * @param value 用于一致性哈希的字符串
	 * @return 返回通过一致性哈希选择的数据库服务器智能指针
	 */
	NF_SHARE_PTR<NFServerData> GetSuitDbServer(const std::string& dbName, const std::string& value);

	/**
	 * @brief 获取所有服务器
	 * @return 返回所有服务器的列表
	 */
	std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer();

	/**
	 * @brief 获取指定类型的所有服务器
	 * @param serverTypes 服务器类型
	 * @return 返回该类型所有服务器的列表
	 */
	std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE serverTypes);

	/**
	 * @brief 获取指定类型的所有服务器（指定跨服属性）
	 * @param serverTypes 服务器类型
	 * @param isCrossServer 是否跨服
	 * @return 返回该类型所有服务器的列表
	 */
	std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE serverTypes, bool isCrossServer);

	/**
	 * @brief 获取所有数据库名称
	 * @return 返回所有数据库名称的列表
	 */
	std::vector<std::string> GetDBNames();
};
