// -------------------------------------------------------------------------
//    @FileName         :    NFBusppNaming.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFBusppNaming
//    @Description      :    Buspp命名服务头文件，提供轻量级的命名服务实现
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFNaming.h"
#include <map>

/**
 * @class NFBusppNaming
 * @brief Buspp命名服务实现类
 *
 * NFBusppNaming是NFNaming接口的轻量级实现，提供简化的命名服务功能：
 * 
 * 设计特点：
 * - 轻量级实现：提供最小化的命名服务功能
 * - 空操作模式：所有操作都返回成功，适用于测试环境
 * - 接口兼容：完全兼容NFNaming接口规范
 * - 零依赖：不依赖外部服务如Zookeeper
 * 
 * 功能特性：
 * - 模拟注册：模拟服务注册功能但不实际存储
 * - 模拟查询：模拟服务查询功能但返回空结果
 * - 模拟监控：模拟服务监控功能但不实际监控
 * - 配置存根：提供配置接口的存根实现
 * 
 * 应用场景：
 * - 单机测试：在单机环境下进行功能测试
 * - 开发调试：开发阶段的快速原型验证
 * - 离线环境：无外部依赖的离线运行环境
 * - 性能测试：避免命名服务成为性能瓶颈
 * - 集成测试：简化测试环境的配置复杂度
 * 
 * 限制说明：
 * - 不提供真实的服务发现功能
 * - 不支持分布式服务注册
 * - 不提供服务状态监控
 * - 不保存任何注册信息
 * 
 * @note 这是一个存根实现，主要用于测试和开发环境
 * @note 生产环境应使用真正的命名服务实现如Zookeeper
 * @warning 不应在生产环境中使用此实现
 */
class NFBusppNaming : public NFNaming
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFBusppNaming(NFIPluginManager* p):NFNaming(p) { }
	
	/**
	 * @brief 析构函数
	 */
	virtual ~NFBusppNaming() { }

	/**
	 * @brief 初始化连接（存根实现）
	 * @param host 地址，如"127.0.0.1:1888,127.0.0.1:2888"
	 * @param time_out_ms 连接超时时间（毫秒），默认20秒，<=0时使用默认值
	 * @return 始终返回0表示成功
	 * 
	 * @note 存根实现，不执行实际的连接操作
	 */
	int32_t Init(const std::string& host, int32_t time_out_ms = 20000)
	{
		return 0;
	}

	/**
	 * @brief 结束调用回收资源（存根实现）
	 * @return 始终返回0表示成功
	 * 
	 * @note 存根实现，不执行实际的资源清理
	 */
	int32_t Fini()
	{
		return 0;
	}

	/**
	 * @brief 设置查询缓存（存根实现）
	 * @param use_cache 是否启用查询缓存
	 * @param refresh_time_ms 缓存自动刷新时间，最小为5000
	 * @param invalid_time_ms 缓存失效时间，最小为10000且大于刷新时间
	 * @return 始终返回0表示成功
	 * 
	 * @note 存根实现，不执行实际的缓存配置
	 */
	int32_t SetCache(bool use_cache = true, int32_t refresh_time_ms = 300000, int32_t invalid_time_ms = 1800000)
	{
		return 0;
	}

public:
	// 实现Naming接口（存根实现）

	/**
	 * @brief 设置应用信息和密钥（存根实现）
	 * @param app_id 应用ID
	 * @param app_key 应用密钥
	 * @return 始终返回0表示成功
	 * 
	 * Init调用后调用此接口添加需要访问的各个应用的授权信息。
	 * 
	 * @note 存根实现，不执行实际的应用信息设置
	 */
	virtual int32_t SetAppInfo(const std::string& app_id, const std::string& app_key)
	{
		return 0;
	}

	/**
	 * @brief 检查应用是否已初始化（存根实现）
	 * @return 始终返回false
	 * 
	 * @note 存根实现，始终返回未初始化状态
	 */
	virtual bool IsInitApp() const
	{
		return false;
	}

	/**
	 * @brief 完成应用初始化（存根实现）
	 * 
	 * @note 存根实现，不执行任何操作
	 */
	virtual void FinishInitApp() { }

	/**
	 * @brief 注册服务（存根实现）
	 * @param name 服务名称（带完整路径）
	 * @param url 服务地址
	 * @param instance_id 实例ID，默认为0
	 * @return 始终返回0表示成功
	 * 
	 * @note 存根实现，不执行实际的服务注册
	 */
	virtual int32_t Register(const std::string& name,
		const std::string& url,
		int64_t instance_id = 0)
	{
		return 0;
	}

	/**
	 * @brief 批量注册服务（存根实现）
	 * @param name 服务名称（带完整路径）
	 * @param urls 地址列表
	 * @param instance_id 实例ID，默认为0
	 * @return 始终返回0表示成功
	 * 
	 * 对于同步接口调用如果不设置协程调度器或不在协程内调用会一直阻塞等待。
	 * 
	 * @note 存根实现，不执行实际的批量服务注册
	 */
	virtual int32_t Register(const std::string& name, const std::vector<std::string>& urls, int64_t instance_id = 0)
	{
		return 0;
	}

	virtual int32_t RegisterAsync(const std::string& name,
		const std::string& url,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return 0;
	}

	/// @brief 异步注册到名字服务
	/// @param name 名字(带完整路径)
	/// @param urls 地址列表
	/// @param cb 异步操作结果的回调返回
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	virtual int32_t RegisterAsync(const std::string& name,
		const std::vector<std::string>& urls,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return 0;
	}

	/// @brief 注销名字同步接口
	/// @param name 名字(带完整路径)
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	/// @note 对于同步接口调用如果不设置协程调度器或不在协程内调用会一直阻塞等待
	virtual int32_t UnRegister(const std::string& name, int64_t instance_id = 0)
	{
		return 0;
	}

	/// @brief 注销名字同步接口
	/// @param name 名字(带完整路径)
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	/// @note 对于同步接口调用如果不设置协程调度器或不在协程内调用会一直阻塞等待
	virtual int32_t ForceDelete(const std::string& name, const std::string& url, int64_t instance_id = 0)
	{
		return 0;
	}

	virtual int32_t ForceDeleteAsync(const std::string& name,
		const std::string& url,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return 0;
	}

	virtual int32_t ForceDeleteAsync(const std::string& name,
		const std::vector<std::string>& urls,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return 0;
	}

	/// @brief 注销名字异步接口
	/// @param name 名字(带完整路径)
	/// @param cb 异步操作结果的回调返回
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	/// @note 对于同步接口调用如果不设置协程调度器或不在协程内调用会一直阻塞等待
	virtual int32_t UnRegisterAsync(const std::string& name,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return 0;
	}

	/// @brief 同步获取名字的地址列表
	/// @param name 名字(带完整路径，支持*通配符号，格式"/a/b"或"/a*/*b/c*d/*")
	/// @param urls 地址列表
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	virtual int32_t GetUrlsByName(const std::string& name, std::vector<std::string>* urls)
	{
		return 0;
	}

	/// @brief 异步获取名字的地址列表
	/// @param name 名字(带完整路径，支持*通配符号，格式"/a/b"或"/a*/*b/c*d/*")
	/// @param cb 异步操作结果的回调返回
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	virtual int32_t GetUrlsByNameAsync(const std::string& name, const NFNamingCbReturnValue& cb)
	{
		return 0;
	}

	/// @brief 同步监控名字变化
	/// @param name 名字(带完整路径，支持*通配符号，格式"/a/b"或"/a*/*b/c*d/*")
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	/// @note  注册监控成功时会在此接口中回调 CbNodeChanged 通知地址信息。\n
	///        即使当前节点不存在或其它原因失败导致zk节点上没有添加成功监控点，在恢复后也可以监控
	virtual int32_t WatchName(const std::string& name, const NFNamingWatchFunc& wc)
	{
		return 0;
	}

	/// @brief 异步监控名字变化
	/// @param name 名字(带完整路径，支持*通配符号，格式"/a/b"或"/a*/*b/c*d/*")
	/// @param cb 异步操作结果的回调返回
	/// @return 0成功，其他失败，错误码意义见@ref ZookeeperErrorCode
	/// @note  注册监控成功时，在cb回调前，会通过 CbNodeChanged 通知地址信息。\n
	///        即使当前节点不存在或其它原因失败导致zk节点上没有添加成功监控点，在恢复后也可以监控
	virtual int32_t WatchNameAsync(const std::string& name, const NFNamingWatchFunc& wc, const NFNamingCbReturnCode& cb)
	{
		return 0;
	}

	/// @brief 驱动异步更新
	virtual int32_t Update()
	{
		return 0;
	}

	/// @brief 获取上次的错误信息
	virtual const char* GetLastError() { return "nfbuspp"; }
};

