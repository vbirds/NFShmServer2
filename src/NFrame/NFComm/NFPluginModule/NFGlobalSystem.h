// -------------------------------------------------------------------------
//    @FileName         :    NFGlobalSystem.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    全局系统管理器，负责多服务器模式下的统一管理和控制
//                           提供服务器状态控制、插件管理器统一访问和特殊消息处理
//
// -------------------------------------------------------------------------
#pragma once

#include <FrameEnum.nanopb.h>

#include "NFComm/NFCore/NFSingleton.hpp"
#include "NFComm/NFKernelMessage/FrameComm.pb.h"
#include "NFILuaLoader.h"
#include <vector>
#include <unordered_set>

class NFIPluginManager;
class NFIModule;

/**
 * @brief 全局系统管理器类
 * 
 * NFGlobalSystem 是框架的全局控制中心，提供了统一的系统管理功能：
 * 
 * 1. 设计理念：
 *    - 单例模式：全局唯一的系统管理实例
 *    - 统一控制：多服务器模式下的集中管理
 *    - 状态同步：服务器间的状态协调和通信
 *    - 资源共享：插件管理器和配置的统一访问
 * 
 * 2. 核心功能：
 *    - 多服务器管理：支持AllServer模式的服务器集群
 *    - 状态控制：服务器的启动、重载、停止、杀死等状态管理
 *    - 插件协调：多个插件管理器实例的统一管理
 *    - 消息路由：特殊消息的注册和处理
 *    - 配置管理：全局配置的加载和访问
 * 
 * 3. 生命周期管理：
 *    - 服务器重载：支持热重载配置和代码
 *    - 优雅停止：安全的服务器关闭流程
 *    - 强制杀死：紧急情况下的立即终止
 *    - 热修复：运行时的代码更新和修复
 * 
 * 4. 多服务器支持：
 *    - 服务器类型管理：注册和查询不同类型的服务器
 *    - 插件管理器集合：维护所有服务器的插件管理器
 *    - 全局配置：所有服务器共享的配置信息
 *    - 模块查找：跨服务器的模块定位和访问
 * 
 * 5. Lua集成：
 *    - 继承NFILuaLoader：支持Lua脚本的动态加载
 *    - 配置脚本化：支持Lua格式的配置文件
 *    - 运行时扩展：通过Lua脚本扩展系统功能
 * 
 * 6. 特殊消息处理：
 *    - 消息注册：注册需要特殊处理的消息类型
 *    - 消息过滤：识别和路由特殊消息
 *    - 模块级消息：按模块分类的消息处理
 * 
 * 7. 性能特点：
 *    - 高效查找：基于哈希的快速模块和服务器查找
 *    - 内存优化：智能的资源管理和回收
 *    - 并发友好：支持多线程环境下的安全访问
 *    - 低开销：最小化全局操作的性能影响
 * 
 * 使用场景：
 * - 分布式游戏服务器架构
 * - 微服务集群的统一管理
 * - 多进程应用的协调控制
 * - 热更新和运维管理系统
 * 
 * 使用示例：
 * @code
 * // 获取全局系统实例
 * auto* globalSystem = NFGlobalSystem::GetSingletonPtr();
 * 
 * // 检查服务器状态
 * if (globalSystem->IsServerStopping()) {
 *     // 处理服务器停止逻辑
 * }
 * 
 * // 设置服务器状态
 * globalSystem->SetReloadServer(true);
 * 
 * // 查找模块
 * auto* module = globalSystem->FindModule("NFILogModule");
 * 
 * // 注册服务器类型
 * globalSystem->AddServerType(NF_ST_GAME);
 * @endcode
 * 
 * 设计优势：
 * - 提供了统一的系统控制接口
 * - 支持复杂的多服务器部署架构
 * - 简化了服务器间的通信和协调
 * - 便于运维管理和监控
 */
class NFGlobalSystem : public NFSingleton<NFGlobalSystem>, public NFILuaLoader
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化全局系统的基本状态：
     * - 初始化所有标志位为false
     * - 清空插件管理器列表
     * - 准备接收配置和管理请求
     */
    NFGlobalSystem();

    /**
     * @brief 虚析构函数
     * 
     * 清理全局系统的所有资源：
     * - 清理插件管理器列表
     * - 释放配置数据
     * - 确保资源的正确释放
     */
    virtual ~NFGlobalSystem();

public:
    /**
     * @brief 加载系统配置
     * @param path 配置文件路径
     * @return 加载成功返回true，失败返回false
     * 
     * 从指定路径加载系统配置文件：
     * - 支持多种配置格式（XML、JSON、Lua等）
     * - 解析多服务器配置信息
     * - 验证配置的完整性和正确性
     * - 初始化系统运行参数
     * 
     * 配置内容包括：
     * - 服务器类型和部署信息
     * - 插件加载配置
     * - 网络和通信参数
     * - 日志和监控设置
     */
    bool LoadConfig(const std::string &path);

public:
    /**
     * @brief 检查是否需要重载应用
     * @return 需要重载返回true，否则返回false
     * 
     * 用于查询当前是否有重载请求：
     * - 配置文件变更触发的重载
     * - 管理员手动发起的重载
     * - 定时或自动重载检查
     * - 热更新完成后的应用重载
     */
    bool IsReloadApp() const;

    /**
     * @brief 设置服务器重载状态
     * @param reloadApp 是否需要重载，true表示需要重载
     * 
     * 标记服务器需要重载：
     * - 配置更新后的重载请求
     * - 代码热更新后的应用重载
     * - 管理命令触发的重载操作
     * - 异常恢复时的重载重启
     * 
     * 重载流程：
     * 1. 保存当前运行状态
     * 2. 停止所有服务
     * 3. 重新加载配置和代码
     * 4. 恢复服务运行
     */
    void SetReloadServer(bool reloadApp);

    /**
     * @brief 检查服务器是否正在停止
     * @return 正在停止返回true，否则返回false
     * 
     * 用于查询服务器的停止状态：
     * - 优雅停止流程的状态查询
     * - 新连接拒绝的判断依据
     * - 资源清理的时机控制
     * - 停止流程的进度跟踪
     */
    bool IsServerStopping() const;

    /**
     * @brief 设置服务器停止状态
     * @param exitApp 是否停止服务器，true表示需要停止
     * 
     * 标记服务器进入停止流程：
     * - 优雅停止：完成当前请求后停止
     * - 拒绝新连接和新请求
     * - 开始资源清理和数据保存
     * - 通知其他服务器此服务器即将下线
     * 
     * 停止流程：
     * 1. 停止接受新连接
     * 2. 处理完当前请求
     * 3. 保存重要数据
     * 4. 清理资源和连接
     * 5. 退出程序
     */
    void SetServerStopping(bool exitApp);

    /**
     * @brief 检查服务器是否正在被强制杀死
     * @return 正在被杀死返回true，否则返回false
     * 
     * 用于查询服务器的强制终止状态：
     * - 紧急情况下的立即停止
     * - 异常处理中的强制退出
     * - 监控系统发起的强制关闭
     * - 系统资源不足时的强制终止
     */
    bool IsServerKilling() const;

    /**
     * @brief 设置服务器强制杀死状态
     * @param exitApp 是否强制杀死服务器，true表示立即终止
     * 
     * 标记服务器进入强制终止流程：
     * - 立即停止所有服务
     * - 跳过正常的清理流程
     * - 强制断开所有连接
     * - 直接退出程序
     * 
     * 适用场景：
     * - 系统异常无法正常停止
     * - 安全事件需要立即关闭
     * - 资源耗尽需要紧急处理
     * - 管理员强制终止命令
     */
    void SetServerKilling(bool exitApp);

    /**
     * @brief 检查服务器是否正在进行热修复
     * @return 正在热修复返回true，否则返回false
     * 
     * 用于查询服务器的热修复状态：
     * - 运行时代码更新的状态
     * - 配置热加载的进度
     * - 模块热替换的状态
     * - 脚本热更新的检查
     */
    bool IsHotfixServer() const;

    /**
     * @brief 设置服务器热修复状态
     * @param hotfixExitApp 是否进行热修复，true表示开始热修复
     * 
     * 标记服务器进入热修复流程：
     * - 动态加载新的代码模块
     * - 替换运行中的旧模块
     * - 更新配置和数据结构
     * - 保持服务的连续性
     * 
     * 热修复类型：
     * - 代码热更新：替换.so/.dll文件
     * - 脚本热加载：重新加载Lua脚本
     * - 配置热刷新：更新配置参数
     * - 数据热修复：修正运行时数据
     */
    void SetHotfixServer(bool hotfixExitApp);

public:
    /**
     * @brief 注册特殊消息类型
     * @param moduleId 模块ID
     * @param msgId 消息ID
     * @return 注册成功返回true，失败返回false
     * 
     * 将指定的消息类型标记为特殊消息：
     * - 需要特殊处理的系统消息
     * - 高优先级的管理消息
     * - 跨服务器的控制消息
     * - 监控和调试相关的消息
     * 
     * 特殊消息的特点：
     * - 优先处理：比普通消息有更高的处理优先级
     * - 特殊路由：可能需要特殊的转发逻辑
     * - 权限检查：可能需要额外的权限验证
     * - 日志记录：详细记录处理过程
     */
    virtual bool RegisterSpecialMsg(uint32_t moduleId, uint32_t msgId);
    
    /**
     * @brief 检查是否为特殊消息
     * @param moduleId 模块ID
     * @param msgId 消息ID
     * @return 是特殊消息返回true，否则返回false
     * 
     * 查询指定消息是否为特殊消息：
     * - 消息处理前的类型判断
     * - 路由策略的选择依据
     * - 权限检查的判断条件
     * - 日志级别的确定依据
     */
    virtual bool IsSpecialMsg(uint32_t moduleId, uint32_t msgId);

public:
    /**
     * @brief 释放单例资源
     * 
     * 手动释放全局系统的单例资源：
     * - 清理所有插件管理器引用
     * - 释放配置数据和缓存
     * - 重置所有状态标志
     * - 确保资源的完全释放
     * 
     * 使用场景：
     * - 程序正常退出时的清理
     * - 单元测试后的资源重置
     * - 异常恢复时的状态清理
     */
    void ReleaseSingleton();

public:
    /**
     * @brief 添加服务器类型
     * @param serverType 服务器类型枚举值
     * 
     * 注册一个新的服务器类型到全局系统：
     * - 多服务器架构中的类型管理
     * - 服务发现和路由的基础
     * - 负载均衡的分类依据
     * - 监控和统计的分组标准
     */
    void AddServerType(NF_SERVER_TYPE serverType) { mVecAllServeerType.insert(serverType); }
    
    /**
     * @brief 获取所有服务器类型
     * @return 包含所有服务器类型的无序集合的常引用
     * 
     * 返回当前注册的所有服务器类型：
     * - 用于遍历所有服务器类型
     * - 服务发现和查询的数据源
     * - 管理界面的展示数据
     * - 统计和监控的基础信息
     */
    const std::unordered_set<uint32_t>& GetAllServerType() const { return mVecAllServeerType; }

public:
    /**
     * @brief 查找指定名称的模块
     * @param strModuleName 模块名称
     * @return 找到返回模块指针，未找到返回nullptr
     * 
     * 在所有插件管理器中查找指定的模块：
     * - 支持跨服务器的模块查找
     * - 统一的模块访问接口
     * - 服务定位和依赖注入的基础
     * - 动态模块发现和调用
     */
    NFIModule *FindModule(const std::string &strModuleName);

private:
    bool m_gIsMoreServer;                                           ///< 是否为多服务器模式
    NFIPluginManager *m_gGlobalPluginManager;                      ///< 全局插件管理器指针
    std::vector<NFIPluginManager *> m_gGlobalPluginManagerList;    ///< 所有插件管理器列表
    NFrame::pbPluginConfig m_gAllMoreServerConfig;                 ///< 多服务器配置信息
    bool m_reloadApp;                                               ///< 应用重载标志
    bool m_serverStopping;                                          ///< 服务器停止标志
    bool m_serverKilling;                                           ///< 服务器强制杀死标志
    bool m_hotfixServer;                                            ///< 服务器热修复标志

    std::vector<std::vector<bool>> mSpecialMsgMap;                 ///< 特殊消息映射表
    std::unordered_set<uint32_t> mVecAllServeerType;              ///< 所有服务器类型集合

public:
    /**
     * @brief 检查是否为多服务器模式
     * @return 多服务器模式返回true，单服务器模式返回false
     * 
     * 查询当前的部署模式：
     * - 单服务器模式：所有功能在一个进程中
     * - 多服务器模式：功能分布在多个进程中
     * - 影响服务发现和通信策略
     * - 决定资源分配和负载均衡策略
     */
    bool IsMoreServer() const
    {
        return m_gIsMoreServer;
    }

    /**
     * @brief 设置多服务器模式
     * @param isMoreServer 是否为多服务器模式
     * 
     * 配置系统的部署模式：
     * - 影响插件加载策略
     * - 决定通信和路由机制
     * - 控制资源管理方式
     * - 影响监控和日志策略
     */
    void SetMoreServer(bool isMoreServer)
    {
        m_gIsMoreServer = isMoreServer;
    }

    /**
     * @brief 获取全局插件管理器
     * @return 全局插件管理器指针，可能为nullptr
     * 
     * 返回主要的插件管理器实例：
     * - 单服务器模式下的唯一管理器
     * - 多服务器模式下的主管理器
     * - 用于全局操作和协调
     * - 其他管理器的参考和同步点
     */
    NFIPluginManager *GetGlobalPluginManager() const
    {
        return m_gGlobalPluginManager;
    }

    /**
     * @brief 设置全局插件管理器
     * @param pPluginManager 插件管理器指针
     * 
     * 设置主要的插件管理器：
     * - 只能设置一次，后续调用被忽略
     * - 确保全局管理器的唯一性
     * - 作为其他管理器的协调中心
     * - 提供统一的插件管理入口
     */
    void SetGlobalPluginManager(NFIPluginManager *pPluginManager)
    {
        if (m_gGlobalPluginManager == NULL)
        {
            m_gGlobalPluginManager = pPluginManager;
        }
    }

    /**
     * @brief 添加插件管理器到列表
     * @param pPluginManager 要添加的插件管理器指针
     * 
     * 注册一个新的插件管理器实例：
     * - 多服务器模式下的管理器注册
     * - 支持动态添加新的服务器实例
     * - 统一管理所有插件管理器
     * - 便于全局操作和协调
     */
    void AddPluginManager(NFIPluginManager *pPluginManager)
    {
        m_gGlobalPluginManagerList.push_back(pPluginManager);
    }

    /**
     * @brief 获取所有插件管理器列表
     * @return 包含所有插件管理器指针的向量
     * 
     * 返回当前注册的所有插件管理器：
     * - 用于遍历所有服务器实例
     * - 全局操作的执行目标
     * - 状态查询和监控的数据源
     * - 资源统计和管理的基础
     */
    std::vector<NFIPluginManager *> GetPluginManagerList()
    {
        return m_gGlobalPluginManagerList;
    }

    /**
     * @brief 获取多服务器配置信息
     * @return 多服务器配置对象的常指针
     * 
     * 返回当前的多服务器配置：
     * - 服务器部署的详细配置
     * - 网络连接和通信参数
     * - 负载均衡和路由规则
     * - 监控和管理的配置信息
     */
    const NFrame::pbPluginConfig *GetAllMoreServerConfig() const
    {
        return &m_gAllMoreServerConfig;
    }
};