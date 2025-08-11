// -------------------------------------------------------------------------
//    @FileName         :    NFRouter.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFRouter
//    @Description      :    路由器头文件，提供分布式服务路由和负载均衡功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFNaming.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"

/**
 * @enum RoutePolicyType
 * @brief 路由策略类型枚举
 *
 * 定义了支持的各种路由策略，用于不同场景的负载均衡和服务选择。
 */
typedef enum {
    NF_USER_ROUTE,        ///< 用户自定义路由类型
    NF_QUALITY_ROUTE,     ///< 根据访问质量的路由类型
    NF_ROUND_ROUTE,       ///< 轮询路由类型
    NF_MOD_ROUTE,         ///< 取模路由类型
    NF_HASH_ROUTE = NF_MOD_ROUTE,   ///< 哈希路由类型，由外部传入hash_key，因此等价于NF_MOD_ROUTE
}RoutePolicyType;

/**
 * @class NFIRoutePolicy
 * @brief 路由策略接口基类
 *
 * 定义了路由策略的标准接口，所有路由策略实现都需要继承此接口。
 * 提供了统一的路由选择方法，支持扩展自定义路由算法。
 */
class NFIRoutePolicy
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~NFIRoutePolicy() {}
    
    /**
     * @brief 获取路由目标
     * @param key 路由键值，用于路由算法计算
     * @param handles 可用的目标句柄列表
     * @return 返回选中的目标句柄，失败返回错误码
     * 
     * 根据提供的键值和可用目标列表，使用特定算法选择最佳目标。
     */
    virtual int64_t GetRoute(uint64_t key, const std::vector<int64_t>& handles) = 0;
};

/**
 * @class NFRoundRoutePolicy
 * @brief 轮询路由策略实现类
 *
 * 实现轮询（Round Robin）路由算法，按顺序循环选择目标服务器：
 * 
 * 算法特点：
 * - 公平分配：每个服务器轮流处理请求
 * - 简单高效：O(1)时间复杂度的选择算法
 * - 无状态感知：不考虑服务器当前负载
 * - 适用场景：服务器性能相近的负载均衡
 */
class NFRoundRoutePolicy      : public NFIRoutePolicy
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化轮询计数器为0。
     */
    NFRoundRoutePolicy() : m_round(0) {}
    
    /**
     * @brief 获取轮询路由目标
     * @param key 路由键值（此策略忽略key参数）
     * @param handles 可用的目标句柄列表
     * @return 返回按轮询选中的目标句柄，无可用目标时返回错误码
     */
    int64_t GetRoute(uint64_t key, const std::vector<int64_t>& handles)
    {
        if (0 == handles.size()) {
            return NFrame::ERR_CODE_ROUTER_NONE_VALID_HANDLE;
        }
        return handles[(m_round++) % handles.size()];
    }

private:
    uint32_t    m_round;    ///< 轮询计数器
};

/**
 * @class NFModRoutePolicy
 * @brief 取模路由策略实现类
 *
 * 实现基于哈希取模的路由算法，根据键值的哈希结果选择目标：
 * 
 * 算法特点：
 * - 一致性路由：相同key总是路由到相同服务器
 * - 高性能：O(1)时间复杂度的计算
 * - 键值敏感：依赖输入key的分布特性
 * - 适用场景：需要会话保持或数据分片的场景
 */
class NFModRoutePolicy        : public NFIRoutePolicy
{
public:
    /**
     * @brief 获取取模路由目标
     * @param key 路由键值，用于取模计算
     * @param handles 可用的目标句柄列表
     * @return 返回基于key取模选中的目标句柄，无可用目标时返回错误码
     */
    int64_t GetRoute(uint64_t key, const std::vector<int64_t>& handles)
    {
        if (0 == handles.size()) {
            return NFrame::ERR_CODE_ROUTER_NONE_VALID_HANDLE;
        }
        return handles[key % handles.size()];
    }
};

/**
 * @brief 目标地址列表变化回调函数类型
 * @param handles 变化后的全量handle列表
 * 
 * 当路由器检测到目标地址列表发生变化时，会调用此回调函数通知上层应用。
 */
typedef std::function<void(const std::vector<int64_t>& handles)> OnAddressChanged;

/**
 * @class NFRouter
 * @brief 分布式服务路由器实现类
 *
 * NFRouter提供了完整的分布式服务路由和负载均衡解决方案：
 * 
 * 路由核心功能：
 * - 服务发现：集成命名服务进行服务发现
 * - 负载均衡：支持多种负载均衡算法
 * - 动态更新：自动感知服务地址变化
 * - 路由选择：根据策略智能选择目标服务
 * 
 * 支持的路由策略：
 * - 轮询路由：公平分配请求到各个服务器
 * - 哈希路由：基于key的一致性路由
 * - 质量路由：根据服务质量选择最佳服务器
 * - 自定义路由：支持用户自定义路由算法
 * 
 * 高可用特性：
 * - 故障检测：自动检测服务器故障并摘除
 * - 故障恢复：服务器恢复后自动加入路由
 * - 动态伸缩：支持服务器的动态扩容和缩容
 * - 优雅降级：在极端情况下的服务降级
 * 
 * 性能优化：
 * - 本地缓存：缓存服务地址减少查询开销
 * - 快速路由：O(1)复杂度的路由选择
 * - 批量更新：批量处理地址变化通知
 * - 内存高效：优化的数据结构和算法
 * 
 * 应用场景：
 * - 微服务架构：服务间的负载均衡和路由
 * - 分布式游戏：游戏服务器的智能路由
 * - API网关：API请求的路由和分发
 * - 数据分片：基于key的数据分片路由
 * 
 * @note 需要配合NFNaming使用以实现服务发现
 * @note 支持运行时动态更换路由策略
 * @warning 路由策略的选择需要根据具体业务场景决定
 */
class NFRouter
{
public:
    /**
     * @brief 构造函数
     * @param name_path 服务名字的绝对路径，同NFNaming中的name_path格式
     * 
     * 根据服务名称创建路由器实例。
     */
    explicit NFRouter(const std::string& name_path);

    /**
     * @brief 析构函数
     */
    virtual ~NFRouter();

    /**
     * @brief 初始化路由器
     * @param naming 命名服务实例指针
     * @return 返回0表示成功，其他值表示失败，参见RouterErrorCode
     * 
     * 初始化路由器并从命名服务获取目标地址列表。
     * 路由器会自动处理地址变化的监控和更新。
     */
    virtual int32_t Init(NFNaming* naming);

    /**
     * @brief 设置路由策略
     * @param policy_type 路由策略类型
     * @param policy 用户自定义路由策略，仅当policy_type为NF_USER_ROUTE时有效
     * @return 返回0表示成功，其他值表示失败，参见RouterErrorCode
     * 
     * 配置路由器使用的负载均衡算法。
     */
    virtual int32_t SetRoutePolicy(RoutePolicyType policy_type, NFIRoutePolicy* policy);

    /**
     * @brief 根据key获取路由目标
     * @param key 路由键值，默认为0
     * @return 返回目标句柄（非负数表示成功），负数表示失败，参见RouterErrorCode
     * 
     * 根据当前路由策略和提供的key选择最佳的目标服务器。
     */
    virtual int64_t GetRoute(uint64_t key = 0);

    /**
     * @brief 设置地址变化回调函数
     * @param on_address_changed 地址列表变化时的回调函数
     * 
     * 当路由器监测到服务地址列表发生变化时，会调用此回调函数。
     */
    virtual void SetOnAddressChanged(const OnAddressChanged& on_address_changed);

protected:
    /**
     * @brief 名称监控回调
     * @param name 服务名称
     * @param urls 更新后的URL列表
     * 
     * 内部方法，处理来自命名服务的地址变化通知。
     */
    void NameWatch(const std::string& name, const std::vector<std::string>& urls);

    std::string             m_route_name;           ///< 路由名称
    RoutePolicyType         m_route_type;           ///< 当前使用的路由策略类型
    NFIRoutePolicy*         m_route_policy;         ///< 当前使用的路由策略实例
    NFNaming*               m_naming;               ///< 命名服务实例指针
    std::vector<int64_t>    m_route_handles;        ///< 当前可用的路由目标句柄列表
    OnAddressChanged        m_on_address_changed;   ///< 地址变化回调函数
};

/**
 * @class RouterFactory
 * @brief 路由器工厂类
 *
 * 提供路由器实例的创建和管理，支持不同类型路由器的统一创建接口。
 * 可以通过继承此类实现自定义的路由器工厂。
 */
class RouterFactory {
public:
    /**
     * @brief 构造函数
     */
    RouterFactory() {}
    
    /**
     * @brief 虚析构函数
     */
    virtual ~RouterFactory() {}
    
    /**
     * @brief 获取路由器实例
     * @param name_path 服务名称路径
     * @return 返回创建的路由器实例指针
     * 
     * 根据服务名称创建对应的路由器实例。
     */
    virtual NFRouter* GetRouter(const std::string& name_path) {
        return new NFRouter(name_path);
    }
};

/**
 * @brief 设置指定类型的路由器工厂
 * @param type 路由器的类型标识
 * @param factory 路由器工厂实例的智能指针
 * @return 返回0表示成功，非0表示失败
 * 
 * 注册特定类型的路由器工厂，支持多种路由器类型的管理。
 */
int32_t SetRouterFactory(int32_t type, const std::shared_ptr<RouterFactory>& factory);

/**
 * @brief 获取指定类型的路由器工厂
 * @param type 路由器的类型标识
 * @return 返回路由器工厂实例的智能指针，为nullptr时说明未设置该类型的工厂
 * 
 * 根据类型获取对应的路由器工厂实例。
 */
std::shared_ptr<RouterFactory> GetRouterFactory(int32_t type);