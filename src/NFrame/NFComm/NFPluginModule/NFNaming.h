// -------------------------------------------------------------------------
//    @FileName         :    NFNaming.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNaming.h
//    @Description      :    命名服务接口定义，提供服务注册发现、监控和缓存功能
//                           支持ZooKeeper等分布式协调服务的抽象封装
//
// -------------------------------------------------------------------------

#pragma once

#include <functional>
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"
#include "NFBaseObj.h"

/// @brief 命名服务监控回调函数类型
/// @param name 服务名称
/// @param urls 服务地址列表
typedef std::function<void(const string &name, const std::vector<std::string>& urls)> NFNamingWatchFunc;

/// @brief 异步注册接口的执行结果回调函数类型
/// @param rc 执行结果返回值，0表示成功，其他值表示失败
typedef std::function<void(int rc)> NFNamingCbReturnCode;

/// @brief 异步查询接口的执行结果回调函数类型
/// @param rc 执行结果返回值，0表示成功，其他值表示失败
/// @param urls 查询结果（服务地址列表）
typedef std::function<void(int rc, const std::vector<std::string>& urls)> NFNamingCbReturnValue;

/**
 * @brief 命名服务接口类
 * 
 * NFNaming 提供了完整的分布式服务注册与发现解决方案：
 * 
 * 1. 服务注册功能：
 *    - 同步和异步服务注册
 *    - 支持单个和批量URL注册
 *    - 实例ID管理和去重
 *    - 自动故障转移
 * 
 * 2. 服务发现功能：
 *    - 基于名称的服务查询
 *    - 通配符模式匹配
 *    - 异步查询回调机制
 *    - 结果缓存优化
 * 
 * 3. 服务监控功能：
 *    - 实时服务变化监控
 *    - 服务上下线通知
 *    - 自动重连和恢复
 *    - 批量监控管理
 * 
 * 4. 缓存机制：
 *    - 查询结果缓存
 *    - 自动刷新策略
 *    - 缓存失效管理
 *    - 性能优化
 * 
 * 5. 权限管理：
 *    - 应用ID和密钥认证
 *    - 多应用授权支持
 *    - 安全连接管理
 * 
 * 设计特点：
 * - 支持多种后端存储（ZooKeeper等）
 * - 协程友好的异步接口
 * - 自动故障恢复
 * - 高性能缓存机制
 * - 灵活的通配符匹配
 * 
 * 使用场景：
 * - 微服务架构
 * - 服务网格
 * - 分布式系统
 * - 负载均衡
 * - 服务治理
 * 
 * 注意事项：
 * - 同步接口需要协程环境支持
 * - 异步接口适用于回调驱动的场景
 * - 监控功能会持续占用连接资源
 * - 缓存配置需要根据业务特点调优
 */
class NFNaming : public NFBaseObj
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * 
     * 初始化命名服务对象，设置基础参数。
     */
    NFNaming(NFIPluginManager* p):NFBaseObj(p) { }
    
    /**
     * @brief 析构函数
     * 
     * 清理命名服务资源，关闭连接和释放内存。
     */
    virtual ~NFNaming() { }

	/**
	 * @brief 初始化连接到命名服务后端
	 * @param host 服务器地址，格式如 "127.0.0.1:1888,127.0.0.1:2888"
	 * @param time_out_ms 连接超时时间（毫秒），默认20秒，<=0时使用默认值
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 建立与命名服务后端（如ZooKeeper）的连接。支持多地址集群配置，
	 * 系统会自动选择可用的服务器建立连接。
	 * 
	 * 使用示例：
	 * @code
	 * NFNaming naming;
	 * int ret = naming.Init("127.0.0.1:2181,127.0.0.1:2182", 30000);
	 * if (ret == 0) {
	 *     // 连接成功，可以进行后续操作
	 * }
	 * @endcode
	 * 
	 * @note 此方法必须在其他操作之前调用
	 * @note 支持集群地址，用逗号分隔
	 */
	virtual int32_t Init(const std::string& host, int32_t time_out_ms = 20000)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 结束命名服务并回收资源
	 * @return 0表示成功
	 * 
	 * 关闭与命名服务的连接，清理所有注册的服务和监控，
	 * 释放相关的内存和网络资源。
	 * 
	 * @note 调用此方法后，对象将无法再使用
	 * @note 会自动注销所有已注册的服务
	 */
	virtual int32_t Fini()
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 设置查询缓存配置
	 * @param use_cache 是否启用查询缓存，默认为true
	 * @param refresh_time_ms 缓存自动刷新时间（毫秒），最小值5000，默认5分钟
	 * @param invaild_time_ms 缓存失效时间（毫秒），最小值10000且必须大于刷新时间，默认30分钟
	 * @return 0表示成功
	 * 
	 * 配置查询结果的缓存策略，用于提高查询性能和减少网络开销。
	 * 缓存会在指定时间间隔自动刷新，并在失效时间后自动清理。
	 * 
	 * 使用示例：
	 * @code
	 * // 启用缓存，5分钟刷新，30分钟失效
	 * naming.SetCache(true, 300000, 1800000);
	 * 
	 * // 禁用缓存（适用于对实时性要求极高的场景）
	 * naming.SetCache(false);
	 * @endcode
	 * 
	 * @note 刷新时间应小于失效时间
	 * @note 缓存策略会影响数据一致性，需要根据业务需求权衡
	 */
	virtual int32_t SetCache(bool use_cache = true, int32_t refresh_time_ms = 300000, int32_t invaild_time_ms = 1800000)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}
	
public:
	// 实现Naming接口

	/**
	 * @brief 设置应用认证信息
	 * @param app_id 应用ID，用于标识不同的应用系统
	 * @param app_key 应用密钥，用于权限验证
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 在Init调用后设置应用的身份认证信息，支持多应用场景。
	 * 每个应用都需要独立的ID和密钥进行授权访问。
	 * 
	 * 使用示例：
	 * @code
	 * naming.Init("127.0.0.1:2181");
	 * naming.SetAppInfo("my-service", "secret-key-123");
	 * @endcode
	 * 
	 * @note 必须在Init之后调用
	 * @note 支持设置多个应用的授权信息
	 */
	virtual int32_t SetAppInfo(const std::string& app_id, const std::string& app_key)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 检查应用是否已初始化
	 * @return true表示已初始化，false表示未初始化
	 * 
	 * 检查应用认证信息是否已设置完成，确保可以进行后续操作。
	 */
	virtual bool IsInitApp() const
	{
		return false;
	}

	/**
	 * @brief 完成应用初始化
	 * 
	 * 标记应用初始化过程完成，允许系统开始正常的服务注册和发现操作。
	 */
	virtual void FinishInitApp() { }

	/**
	 * @brief 注册单个服务地址（同步）
	 * @param name 服务名称（完整路径）
	 * @param url 服务地址
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 同步方式注册单个服务地址到命名服务。
	 * 
	 * @note 同步接口需要在协程环境中调用，否则会阻塞线程
	 */
	virtual int32_t Register(const std::string& name,
		const std::string& url,
		int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 注册多个服务地址（同步）
	 * @param name 服务名称（完整路径）
	 * @param urls 服务地址列表
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 同步方式批量注册多个服务地址到命名服务。
	 * 支持一次性注册同一服务的多个实例。
	 * 
	 * 使用示例：
	 * @code
	 * std::vector<std::string> urls = {
	 *     "tcp://192.168.1.10:8080",
	 *     "tcp://192.168.1.11:8080",
	 *     "tcp://192.168.1.12:8080"
	 * };
	 * int ret = naming.Register("/services/web-api", urls, 1001);
	 * @endcode
	 * 
	 * @note 同步接口需要在协程环境中调用，否则会阻塞线程
	 * @note 所有地址会作为同一服务的不同实例注册
	 */
	virtual int32_t Register(const std::string& name, const std::vector<std::string>& urls, int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 异步注册单个服务地址
	 * @param name 服务名称（完整路径）
	 * @param url 服务地址
	 * @param cb 异步操作结果回调函数
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示请求已发送，其他值表示发送失败
	 * 
	 * 异步方式注册单个服务地址，通过回调函数获取操作结果。
	 * 
	 * 使用示例：
	 * @code
	 * auto callback = [](int rc) {
	 *     if (rc == 0) {
	 *         std::cout << "注册成功" << std::endl;
	 *     } else {
	 *         std::cout << "注册失败，错误码: " << rc << std::endl;
	 *     }
	 * };
	 * naming.RegisterAsync("/services/my-api", "tcp://127.0.0.1:8080", callback);
	 * @endcode
	 */
	virtual int32_t RegisterAsync(const std::string& name,
		const std::string& url,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 异步注册多个服务地址
	 * @param name 服务名称（完整路径）
	 * @param urls 服务地址列表
	 * @param cb 异步操作结果回调函数
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示请求已发送，其他值表示发送失败
	 * 
	 * 异步方式批量注册多个服务地址，通过回调函数获取操作结果。
	 * 
	 * 使用示例：
	 * @code
	 * std::vector<std::string> urls = {
	 *     "tcp://server1:8080",
	 *     "tcp://server2:8080"
	 * };
	 * auto callback = [](int rc) {
	 *     // 处理注册结果
	 * };
	 * naming.RegisterAsync("/services/cluster", urls, callback, 2001);
	 * @endcode
	 */
	virtual int32_t RegisterAsync(const std::string& name,
		const std::vector<std::string>& urls,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 注销服务（同步）
	 * @param name 服务名称（完整路径）
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 同步方式注销指定的服务注册。服务注销后，
	 * 其他客户端将无法通过名称发现该服务。
	 * 
	 * @note 同步接口需要在协程环境中调用，否则会阻塞线程
	 * @note 注销操作通常在服务关闭时调用
	 */
	virtual int32_t UnRegister(const std::string& name, int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 强制删除服务地址（同步）
	 * @param name 服务名称（完整路径）
	 * @param url 要删除的服务地址
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 强制删除指定的服务地址，即使服务实例仍在运行。
	 * 适用于清理僵尸服务或异常状态的服务实例。
	 * 
	 * @note 同步接口需要在协程环境中调用，否则会阻塞线程
	 * @warning 此操作可能导致正在运行的服务不可访问
	 */
	virtual int32_t ForceDelete(const std::string& name, const std::string& url, int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 异步强制删除单个服务地址
	 * @param name 服务名称（完整路径）
	 * @param url 要删除的服务地址
	 * @param cb 异步操作结果回调函数
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示请求已发送，其他值表示发送失败
	 * 
	 * 异步方式强制删除指定的服务地址。
	 */
	virtual int32_t ForceDeleteAsync(const std::string& name,
		const std::string& url,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 异步强制删除多个服务地址
	 * @param name 服务名称（完整路径）
	 * @param urls 要删除的服务地址列表
	 * @param cb 异步操作结果回调函数
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示请求已发送，其他值表示发送失败
	 * 
	 * 异步方式批量强制删除多个服务地址。
	 */
	virtual int32_t ForceDeleteAsync(const std::string& name,
		const std::vector<std::string>& urls,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 异步注销服务
	 * @param name 服务名称（完整路径）
	 * @param cb 异步操作结果回调函数
	 * @param instance_id 实例ID，默认为0
	 * @return 0表示请求已发送，其他值表示发送失败
	 * 
	 * 异步方式注销指定的服务注册，通过回调函数获取操作结果。
	 * 
	 * 使用示例：
	 * @code
	 * auto callback = [](int rc) {
	 *     if (rc == 0) {
	 *         std::cout << "注销成功" << std::endl;
	 *     }
	 * };
	 * naming.UnRegisterAsync("/services/my-api", callback);
	 * @endcode
	 */
	virtual int32_t UnRegisterAsync(const std::string& name,
		const NFNamingCbReturnCode& cb,
		int64_t instance_id = 0)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 根据服务名称获取地址列表（同步）
	 * @param name 服务名称（完整路径，支持通配符）
	 * @param urls 输出参数，存储查询到的地址列表
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 同步方式查询指定服务名称对应的所有地址。
	 * 支持通配符模式匹配，格式如"/a/b"或"/a*\/\*b/c*d/\*"。
	 * 
	 * 使用示例：
	 * @code
	 * std::vector<std::string> urls;
	 * int ret = naming.GetUrlsByName("/services/web-api", &urls);
	 * if (ret == 0) {
	 *     for (const auto& url : urls) {
	 *         std::cout << "发现服务: " << url << std::endl;
	 *     }
	 * }
	 * 
	 * // 使用通配符查询
	 * naming.GetUrlsByName("/services/web-*", &urls);
	 * @endcode
	 * 
	 * @note 同步接口需要在协程环境中调用，否则会阻塞线程
	 * @note 支持*通配符进行模糊匹配
	 */
	virtual int32_t GetUrlsByName(const std::string& name, std::vector<std::string>* urls)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 根据服务名称异步获取地址列表
	 * @param name 服务名称（完整路径，支持通配符）
	 * @param cb 异步操作结果回调函数
	 * @return 0表示请求已发送，其他值表示发送失败
	 * 
	 * 异步方式查询指定服务名称对应的所有地址。
	 * 支持通配符模式匹配。
	 * 
	 * 使用示例：
	 * @code
	 * auto callback = [](int rc, const std::vector<std::string>& urls) {
	 *     if (rc == 0) {
	 *         std::cout << "找到 " << urls.size() << " 个服务实例" << std::endl;
	 *         for (const auto& url : urls) {
	 *             std::cout << "  - " << url << std::endl;
	 *         }
	 *     }
	 * };
	 * naming.GetUrlsByNameAsync("/services/user-*", callback);
	 * @endcode
	 */
	virtual int32_t GetUrlsByNameAsync(const std::string& name, const NFNamingCbReturnValue& cb)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 监控服务名称变化（同步）
	 * @param name 服务名称（完整路径，支持通配符）
	 * @param wc 服务变化监控回调函数
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 同步方式注册对指定服务名称的监控。当服务地址发生变化时，
	 * 会通过回调函数通知最新的地址列表。
	 * 
	 * 监控特点：
	 * - 注册成功时会立即回调当前地址信息
	 * - 服务上下线时会实时通知
	 * - 支持自动重连和恢复
	 * - 即使初始注册失败，恢复后也会继续监控
	 * 
	 * 使用示例：
	 * @code
	 * auto watchFunc = [](const std::string& name, const std::vector<std::string>& urls) {
	 *     std::cout << "服务 " << name << " 发生变化:" << std::endl;
	 *     for (const auto& url : urls) {
	 *         std::cout << "  - " << url << std::endl;
	 *     }
	 * };
	 * naming.WatchName("/services/critical-service", watchFunc);
	 * @endcode
	 * 
	 * @note 监控会持续到显式取消或对象销毁
	 * @note 支持通配符模式监控多个相关服务
	 */
	virtual int32_t WatchName(const std::string& name, const NFNamingWatchFunc& wc)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 取消对服务名称的监控
	 * @param name 服务名称（完整路径）
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 取消对指定服务名称的监控，停止接收变化通知。
	 * 
	 * 使用示例：
	 * @code
	 * // 取消监控
	 * naming.UnWatchName("/services/old-service");
	 * @endcode
	 */
	virtual int32_t UnWatchName(const std::string& name)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 异步监控服务名称变化
	 * @param name 服务名称（完整路径，支持通配符）
	 * @param wc 服务变化监控回调函数
	 * @param cb 异步操作结果回调函数
	 * @return 0表示请求已发送，其他值表示发送失败
	 * 
	 * 异步方式注册对指定服务名称的监控。
	 * 监控注册成功时，会先通过变化回调通知当前地址信息，
	 * 然后再调用结果回调确认操作完成。
	 * 
	 * 使用示例：
	 * @code
	 * auto watchFunc = [](const std::string& name, const std::vector<std::string>& urls) {
	 *     // 处理服务变化
	 * };
	 * auto callback = [](int rc) {
	 *     if (rc == 0) {
	 *         std::cout << "监控注册成功" << std::endl;
	 *     }
	 * };
	 * naming.WatchNameAsync("/services/dynamic-*", watchFunc, callback);
	 * @endcode
	 */
	virtual int32_t WatchNameAsync(const std::string& name, const NFNamingWatchFunc& wc, const NFNamingCbReturnCode& cb)
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 驱动异步操作更新
	 * @return 0表示成功，其他值表示失败
	 * 
	 * 主动触发异步操作的处理和状态更新。
	 * 在事件循环中定期调用，确保异步回调能够及时执行。
	 * 
	 * @note 通常在主循环中定期调用此方法
	 * @note 对于基于事件的异步模型特别重要
	 */
	virtual int32_t Update()
	{
		return NFrame::ERR_CODE_NAMING_NOT_SUPPORTTED;
	}

	/**
	 * @brief 获取上次操作的错误信息
	 * @return 错误信息字符串，NULL表示无错误
	 * 
	 * 获取最近一次操作失败的详细错误描述，
	 * 用于问题诊断和错误处理。
	 */
    virtual const char* GetLastError() { return NULL; }
    
public:
    /**
     * @brief 生成标准化的服务名称
     * @param app_id 应用ID
     * @param service_dir 服务目录
     * @param service 服务名称
     * @param name 输出参数，生成的完整服务名称
     * @return 0表示成功，其他值表示失败
     * 
     * 根据应用ID、服务目录和服务名称生成标准化的完整服务路径。
     * 用于统一服务命名规范，确保服务名称的一致性。
     * 
     * 使用示例：
     * @code
     * std::string fullName;
     * int ret = NFNaming::MakeName(1001, "/api/v1", "user-service", &fullName);
     * // fullName 可能为 "/app/1001/api/v1/user-service"
     * @endcode
     */
    static int32_t MakeName(int64_t app_id,
                            const std::string& service_dir,
                            const std::string& service,
                            std::string* name);

    /**
     * @brief 生成TBUSPP格式的URL
     * @param name 服务名称
     * @param inst_id 实例ID
     * @param url 输出参数，生成的URL
     * @return 0表示成功，其他值表示失败
     * 
     * 为兼容性需要，生成TBUSPP协议格式的服务URL。
     * 
     * @note 这是为了向后兼容而保留的接口
     */
    static int32_t MakeTbusppUrl(const std::string& name,
                                 int64_t inst_id,
                                 std::string* url);

    /**
     * @brief 格式化服务名称字符串
     * @param name 输入输出参数，要格式化的服务名称
     * @return 0表示成功，其他值表示失败
     * 
     * 对服务名称进行标准化格式处理，确保符合命名规范。
     * 包括路径分隔符规范化、特殊字符处理等。
     * 
     * 使用示例：
     * @code
     * std::string serviceName = "//service//user-api///";
     * NFNaming::FormatNameStr(&serviceName);
     * // serviceName 变为 "/service/user-api"
     * @endcode
     */
    static int32_t FormatNameStr(std::string* name);
};



