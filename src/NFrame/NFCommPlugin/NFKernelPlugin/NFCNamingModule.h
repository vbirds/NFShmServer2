// -------------------------------------------------------------------------
//    @FileName         :    NFCNamingModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCNamingModule
//    @Desc             :    命名服务模块头文件，提供分布式服务发现和注册功能。
//                          该文件定义了NFShmXFrame框架的命名服务模块，基于Zookeeper实现分布式服务发现和注册系统，
//                          包括服务注册功能、服务发现功能、高级特性、分布式特性等功能。
//                          主要功能包括应用信息注册、数据库信息注册、动态服务注册、多类型地址注册、
//                          服务地址查询、负载均衡支持、实时监控、缓存机制、多服务器类型支持、
//                          外部服务访问、路由代理支持、应用授权、通配符支持、集群感知、故障恢复、
//                          数据一致性、会话管理、本地缓存、异步操作、批量操作、连接复用
//    @Description      :    命名服务模块头文件，提供分布式服务发现和注册功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFINamingModule.h"
#include "NFComm/NFPluginModule/NFNaming.h"

/**
 * @class NFCNamingModule
 * @brief 命名服务模块实现类
 *
 * NFCNamingModule基于Zookeeper实现的分布式服务发现和注册系统：
 * 
 * 服务注册功能：
 * - 应用信息注册：注册服务基本信息和配置
 * - 数据库信息注册：注册数据库连接信息
 * - 动态服务注册：支持服务的动态上线和下线
 * - 多类型地址注册：支持TCP、Bus、路由代理等多种地址类型
 * 
 * 服务发现功能：
 * - 服务地址查询：根据服务类型获取可用服务地址
 * - 负载均衡支持：获取多个服务实例进行负载分配
 * - 实时监控：监控服务状态变化并及时通知
 * - 缓存机制：提供查询缓存以提高性能
 * 
 * 高级特性：
 * - 多服务器类型支持：区分不同类型的服务器
 * - 外部服务访问：支持外部服务IP和端口查询
 * - 路由代理支持：提供路由代理服务发现
 * - 应用授权：基于appId和appKey的应用授权机制
 * - 通配符支持：支持通配符模式的服务名匹配
 * 
 * 分布式特性：
 * - 集群感知：自动感知Zookeeper集群状态
 * - 故障恢复：支持网络分区后的自动恢复
 * - 数据一致性：保证服务注册信息的强一致性
 * - 会话管理：自动维护与Zookeeper的会话连接
 * 
 * 应用场景：
 * - 微服务架构：服务间的动态发现和调用
 * - 游戏服务器集群：游戏服务器的自动发现和负载均衡
 * - 数据库服务发现：动态数据库实例的发现和连接
 * - 配置中心：分布式配置信息的统一管理
 * - 健康检查：服务健康状态的监控和管理
 * 
 * 性能优化：
 * - 本地缓存：减少Zookeeper查询频率
 * - 异步操作：非阻塞的服务注册和发现
 * - 批量操作：支持批量服务注册和查询
 * - 连接复用：复用Zookeeper连接减少开销
 * 
 * @note 基于Apache Zookeeper实现，提供CP（一致性+分区容错性）保证
 * @note 支持多数据中心和跨网络的服务发现
 * @warning Zookeeper连接中断时服务发现功能受限
 */
class NFCNamingModule : public NFINamingModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    explicit NFCNamingModule(NFIPluginManager* p);

    /**
     * @brief 析构函数
     */
    virtual ~NFCNamingModule();

    /**
     * @brief 关闭前处理
     * @return 返回0表示成功，非0表示失败
     */
    int BeforeShut() override;
    
    /**
     * @brief 关闭模块
     * @return 返回0表示成功，非0表示失败
     */
    int Shut() override;
    
    /**
     * @brief 模块定时更新
     * @return 返回0表示成功，非0表示失败
     */
    int Tick() override;
    
    /**
     * @brief 配置重载处理
     * @return 返回0表示成功，非0表示失败
     */
    int OnReloadConfig() override;

public:
    /**
     * @brief 获取默认Master服务器信息
     * @param eServerType 服务器类型
     * @return 返回Master服务器信息结构体
     */
	virtual NFrame::ServerInfoReport GetDefaultMasterInfo(NF_SERVER_TYPE eServerType);
	
    /**
     * @brief 初始化应用信息
     * @param eServerType 服务器类型
     * @param time_out_ms 连接超时时间（毫秒），默认20秒，<=0时使用默认值
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 初始化与Zookeeper的连接并准备应用信息注册。
     */
    virtual int32_t InitAppInfo(NF_SERVER_TYPE eServerType, int time_out_ms = 20000) override;

    /**
     * @brief 检查应用是否已初始化
     * @param eServerType 服务器类型
     * @return 返回true表示已初始化，false表示未初始化
     */
    virtual bool IsInitApp(NF_SERVER_TYPE eServerType) override;

    /**
     * @brief 完成应用初始化
     * @param eServerType 服务器类型
     * 
     * 标记应用初始化完成，开始提供服务。
     */
    virtual void FinishInitApp(NF_SERVER_TYPE eServerType) override;

    /**
     * @brief 注册应用信息
     * @param eServerType 服务器类型
     * @return 返回0表示成功，其他值表示失败
     * 
     * 将当前应用的基本信息注册到Zookeeper。
     */
    virtual int32_t RegisterAppInfo(NF_SERVER_TYPE eServerType) override;

    /**
     * @brief 注册数据库信息
     * @param eServerType 服务器类型
     * @param content 数据库配置内容
     * @return 返回0表示成功，其他值表示失败
     * 
     * 注册数据库连接和配置信息，供其他服务发现和使用。
     */
    virtual int32_t RegisterDBInfo(NF_SERVER_TYPE eServerType, const std::string& content) override;

    /**
     * @brief 清除数据库信息
     * @param eServerType 服务器类型
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t ClearDBInfo(NF_SERVER_TYPE eServerType) override;
    
    /**
     * @brief 根据名称获取数据库信息
     * @param eServerType 服务器类型
     * @param tcpUrlVec 输出的TCP URL列表
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t GetDBInfoByName(NF_SERVER_TYPE eServerType, std::vector<std::string>& tcpUrlVec);

    /**
     * @brief 根据服务名获取TCP地址列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param tcpUrl 输出的TCP地址列表
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 支持通配符模式，
     */
    virtual int32_t GetTcpUrlsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, std::vector<std::string>& tcpUrl) override;
    
    /**
     * @brief 根据服务名和业务ID获取TCP地址列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param busId 业务ID
     * @param tcpUrl 输出的TCP地址列表
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t GetTcpUrlsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, uint32_t busId, std::vector<std::string>& tcpUrl);

    /**
     * @brief 根据服务名获取Bus地址列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param busUrl 输出的Bus地址列表
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * Bus地址用于内部服务间的高性能通信。
     */
    virtual int32_t GetBusUrlsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, std::vector<std::string>& busUrl) override;
    
    /**
     * @brief 根据服务名和业务ID获取Bus地址列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param busId 业务ID
     * @param busUrl 输出的Bus地址列表
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t GetBusUrlsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, uint32_t busId, std::vector<std::string>& busUrl);

    /**
     * @brief 根据服务名获取路由代理列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param routeAgent 输出的路由代理列表
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 路由代理用于跨网络区域的服务访问。
     */
    virtual int32_t GetRouteAgentsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, std::vector<std::string>& routeAgent) override;
    
    /**
     * @brief 根据服务名和业务ID获取路由代理列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param busId 业务ID
     * @param routeAgent 输出的路由代理列表
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t GetRouteAgentsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, uint32_t busId, std::vector<std::string>& routeAgent);

    /**
     * @brief 根据服务名获取外部服务器IP列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param externalServerIp 输出的外部服务器IP列表
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 用于获取供外部客户端访问的服务器IP地址。
     */
    virtual int32_t GetExternalServerIpsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, std::vector<std::string>& externalServerIp) override;
    
    /**
     * @brief 根据服务名和业务ID获取外部服务器IP列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param busId 业务ID
     * @param externalServerIp 输出的外部服务器IP列表
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t GetExternalServerIpsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, uint32_t busId, std::vector<std::string>& externalServerIp);

    /**
     * @brief 根据服务名获取外部服务器端口列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param externalServerPort 输出的外部服务器端口列表
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 用于获取供外部客户端访问的服务器端口。
     */
    virtual int32_t GetExternalServerPortsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, std::vector<std::string>& externalServerPort) override;
    
    /**
     * @brief 根据服务名和业务ID获取外部服务器端口列表
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param busId 业务ID
     * @param externalServerPort 输出的外部服务器端口列表
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t GetExternalServerPortsByName(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, uint32_t busId, std::vector<std::string>& externalServerPort);

    /**
     * @brief 根据服务器获取数据库名称
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param destBusId 目标业务ID
     * @param dbName 输出的数据库名称列表
     * @return 返回0表示成功，其他值表示失败
     */
    virtual int32_t GetDBNameByServer(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, uint32_t destBusId, std::vector<std::string>& dbName) override;

    /**
     * @brief 监控TCP地址变化
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param wc 监控回调函数
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 注册对TCP地址变化的监控，当地址发生变化时会触发回调通知。
     * 即使当前节点不存在或监控添加失败，在恢复后也可以继续监控。
     */
    virtual int32_t WatchTcpUrls(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, const NFNamingServerWatchFunc& wc)  override;

    /**
     * @brief 监控Bus地址变化
     * @param eServerType 当前服务器类型
     * @param destServerType 目标服务器类型
     * @param wc 监控回调函数
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 注册对Bus地址变化的监控，当地址发生变化时会触发回调通知。
     * 即使当前节点不存在或监控添加失败，在恢复后也可以继续监控。
     */
    virtual int32_t WatchBusUrls(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServerType, const NFNamingServerWatchFunc& wc) override;

    /**
     * @brief 反初始化应用信息
     * @param eServerType 服务器类型
     * @return 返回0表示成功，其他值表示失败
     * 
     * 清理应用相关的Zookeeper连接和注册信息。
     */
    virtual int32_t UnInitAppInfo(NF_SERVER_TYPE eServerType);

public:
    /**
     * @brief 初始化Zookeeper连接
     * @param eServerType 服务器类型
     * @param host Zookeeper地址，如"127.0.0.1:1888,127.0.0.1:2888"
     * @param time_out_ms 连接超时时间（毫秒），默认20秒，<=0时使用默认值
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 建立与Zookeeper集群的连接，为后续操作做准备。
     */
    virtual int32_t Init(NF_SERVER_TYPE eServerType, const std::string& host, int32_t time_out_ms = 20000) override;

    /**
     * @brief 设置查询缓存
     * @param eServerType 服务器类型
     * @param use_cache 是否启用查询缓存，默认true
     * @param refresh_time_ms 缓存自动刷新时间（毫秒），最小5000
     * @param invalid_time_ms 缓存失效时间（毫秒），最小10000且大于刷新时间
     * @return 返回0表示成功
     * 
     * 配置本地缓存参数以提高查询性能。
     */
    virtual int32_t SetCache(NF_SERVER_TYPE eServerType, bool use_cache = true, int32_t refresh_time_ms = 300000, int32_t invalid_time_ms = 1800000) override;

public:
    // 实现基础Naming接口

    /**
     * @brief 设置应用信息和密钥
     * @param eServerType 服务器类型
     * @param app_id 应用ID
     * @param app_key 应用密钥
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 在Init调用后设置需要访问的各个应用的授权信息。
     */
    virtual int32_t SetAppInfo(NF_SERVER_TYPE eServerType, const std::string& app_id, const std::string& app_key) override;

    /**
     * @brief 注册服务名称和地址
     * @param eServerType 服务器类型
     * @param name 服务名称（带完整路径）
     * @param url 服务地址
     * @param instance_id 实例ID，默认为0
     * @return 返回0表示成功，其他值表示失败
     * 
     * 将服务名称和对应的访问地址注册到Zookeeper。
     */
    virtual int32_t Register(NF_SERVER_TYPE eServerType, const std::string& name,
                             const std::string& url,
                             int64_t instance_id = 0) override;

    /**
     * @brief 注销服务名称
     * @param eServerType 服务器类型
     * @param name 服务名称（带完整路径）
     * @param instance_id 实例ID，默认为0
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * @note 对于同步接口，如果不设置协程调度器或不在协程内调用会一直阻塞等待
     */
    virtual int32_t UnRegister(NF_SERVER_TYPE eServerType, const std::string& name, int64_t instance_id = 0) override;

    /**
     * @brief 强制删除服务名称
     * @param eServerType 服务器类型
     * @param name 服务名称（带完整路径）
     * @param url 服务地址
     * @param instance_id 实例ID，默认为0
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * @note 对于同步接口，如果不设置协程调度器或不在协程内调用会一直阻塞等待
     */
    virtual int32_t ForceDelete(NF_SERVER_TYPE eServerType, const std::string& name, const std::string& url, int64_t instance_id = 0) override;

    /**
     * @brief 根据名称获取地址列表
     * @param eServerType 服务器类型
     * @param name 服务名称（带完整路径，支持*通配符，）
     * @param urls 输出的地址列表
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     */
    virtual int32_t GetUrlsByName(NF_SERVER_TYPE eServerType, const std::string& name, std::vector<std::string>* urls) override;

    /**
     * @brief 监控服务名称变化
     * @param eServerType 服务器类型
     * @param name 服务名称（带完整路径，支持*通配符，）
     * @param wc 监控回调函数
     * @return 返回0表示成功，其他值表示失败，错误码参见ZookeeperErrorCode
     * 
     * 注册对服务名称变化的监控，成功时会在回调中通知地址信息。
     * 即使当前节点不存在或监控添加失败，在恢复后也可以继续监控。
     */
    virtual int32_t WatchName(NF_SERVER_TYPE eServerType, const std::string& name, const NFNamingWatchFunc& wc) override;

private:
    /**
     * @brief 命名服务实例列表
     * 
     * 存储每个服务器类型对应的Naming实例，
     * 支持多个独立的命名服务连接。
     */
    std::vector<NFNaming*> m_namingList;
};
