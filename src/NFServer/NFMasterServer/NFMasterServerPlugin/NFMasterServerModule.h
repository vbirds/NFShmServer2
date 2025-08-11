// -------------------------------------------------------------------------
//    @FileName         :    NFCMasterServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCMasterServerModule
//    @Desc             :    NFShmXFrame主服务器模块接口定义
//                          提供服务器管理和协调功能
//                          支持服务器注册、状态管理和负载均衡
//
//    @Summary          :    主服务器是分布式游戏服务器架构中的核心组件
//                          负责管理所有其他服务器的注册、状态监控和协调工作
//                          提供服务器集群管理、负载均衡、系统监控等功能
//                          支持HTTP管理接口和服务器控制命令
//
//    @Architecture     :    1. 服务器注册管理：处理各种类型服务器的注册请求
//                          2. 状态监控：实时监控所有服务器的运行状态
//                          3. 负载均衡：协调服务器间的负载分配
//                          4. 集群管理：管理服务器集群的拓扑结构
//                          5. 系统控制：提供服务器启动、停止、重启等控制功能
//                          6. HTTP接口：提供Web管理界面
//                          7. 全局服务器：连接全局服务器进行跨服管理
//
//    @Server Types     :    1. Game Server: 游戏逻辑服务器
//                          2. Login Server: 登录服务器
//                          3. World Server: 世界服务器
//                          4. Proxy Server: 代理服务器
//                          5. Route Server: 路由服务器
//                          6. Route Agent Server: 路由代理服务器
//                          7. Center Server: 中心服务器
//                          8. Store Server: 存储服务器
//                          9. SNS Server: 社交网络服务器
//                          10. Web Server: Web服务器
//
//    @Management       :    1. 服务器注册：处理新服务器的注册请求
//                          2. 状态同步：向所有服务器同步服务器状态
//                          3. 服务器报告：接收和处理服务器状态报告
//                          4. 负载监控：监控各服务器的负载情况
//                          5. 故障检测：检测和处理服务器故障
//                          6. 自动恢复：支持服务器的自动重启和恢复
//
//    @HTTP Interface   :    1. /reload: 重新加载指定服务器
//                          2. /reloadall: 重新加载所有服务器
//                          3. /restart: 重启指定服务器
//                          4. /restartall: 重启所有服务器
//                          5. /start: 启动指定服务器
//                          6. /startall: 启动所有服务器
//                          7. /stop: 停止指定服务器
//                          8. /stopall: 停止所有服务器
//                          9. /killall: 强制关闭所有服务器
//
//    @Error Handling   :    1. 服务器断开检测和处理
//                          2. 注册失败的错误处理
//                          3. 状态同步失败的重试机制
//                          4. 服务器控制命令的响应处理
//                          5. HTTP请求的异常处理
//
//    @Performance      :    1. 高效的服务器状态管理
//                          2. 异步消息处理机制
//                          3. 定时器驱动的状态同步
//                          4. 内存优化的服务器数据存储
//                          5. 并发安全的服务器操作
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFCore/NFCommMap.hpp"
#include "NFServerComm/NFServerCommon/NFIMasterServerModule.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"

/**
 * @brief 主服务器模块实现类
 *
 * NFCMasterServerModule是NFIMasterServerModule接口的具体实现，
 * 负责管理所有其他服务器的注册、状态监控和协调工作。
 *
 * 该模块提供以下主要功能：
 * - 服务器注册管理：处理各种类型服务器的注册请求，维护服务器列表
 * - 服务器状态监控：实时监控所有服务器的运行状态和健康度
 * - 负载均衡协调：根据服务器负载情况协调消息分发
 * - 服务器间通信路由：管理服务器间的消息路由和转发
 * - 系统配置管理：管理服务器集群的配置信息
 * - 服务器集群管理：维护服务器集群的拓扑结构和连接关系
 * - HTTP管理接口：提供Web管理界面，支持服务器控制命令
 * - 服务器控制命令：支持启动、停止、重启、重载等控制操作
 * - 全局服务器连接：连接全局服务器进行跨服管理
 * - 故障检测和恢复：自动检测服务器故障并支持自动恢复
 *
 * @details 核心特性：
 * - 支持多种服务器类型的注册和管理
 * - 提供完整的HTTP REST API接口
 * - 支持服务器状态的实时同步
 * - 具备故障检测和自动恢复能力
 * - 支持服务器集群的动态扩展
 * - 提供详细的服务器状态监控
 * - 支持服务器控制命令的批量操作
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCMasterServerModule : public NFIMasterServerModule
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化主服务器模块，设置插件管理器
	 *
	 * @param p 插件管理器指针
	 */
	explicit NFCMasterServerModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 *
	 * 清理主服务器模块资源
	 */
	virtual ~NFCMasterServerModule();

	/**
	 * @brief 模块唤醒 - 主服务器模块初始化
	 *
	 * 在模块初始化完成后调用，进行必要的初始化工作：
	 * - 注册RPC服务和消息回调
	 * - 添加HTTP请求处理器
	 * - 绑定服务器端口
	 * - 订阅服务器事件
	 * - 设置定时器
	 *
	 * @details 初始化流程：
	 * 1. 注册Master RPC服务，处理服务器注册请求
	 * 2. 注册各种消息回调，处理服务器报告和控制命令
	 * 3. 添加HTTP请求处理器，提供Web管理接口
	 * 4. 绑定服务器端口，监听客户端连接
	 * 5. 订阅服务器死亡事件，处理服务器故障
	 * 6. 设置定时器，定期保存服务器数据和清理过期数据
	 *
	 * @return 初始化结果状态码，0表示成功，-1表示失败
	 */
	virtual int Awake() override;

	/**
	 * @brief 模块初始化 - 连接全局服务器
	 *
	 * 初始化主服务器模块，包括连接全局服务器等：
	 * - 连接全局服务器进行跨服管理
	 * - 注册到全局服务器
	 * - 建立全局服务器通信
	 *
	 * @details 初始化流程：
	 * 1. 获取全局服务器配置信息
	 * 2. 建立到全局服务器的连接
	 * 3. 注册当前主服务器到全局服务器
	 * 4. 设置全局服务器事件回调
	 * 5. 建立全局服务器消息处理机制
	 *
	 * @return 初始化结果状态码，0表示成功，-1表示失败
	 */
	virtual int Init() override;

	/**
	 * @brief 模块定时更新 - 服务器状态报告
	 *
	 * 每帧调用，处理模块的定时任务：
	 * - 向全局服务器报告当前服务器状态
	 * - 处理服务器状态同步
	 * - 执行定期维护任务
	 *
	 * @details 处理流程：
	 * 1. 收集当前主服务器的状态信息
	 * 2. 向全局服务器发送状态报告
	 * 3. 处理服务器状态同步任务
	 * 4. 执行定期维护和清理任务
	 * 5. 监控服务器集群健康状态
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	virtual int Tick() override;

	/**
	 * @brief 动态插件处理
	 *
	 * 处理动态插件的加载和卸载
	 *
	 * @return 处理结果状态码
	 */
	virtual int OnDynamicPlugin() override;

	/**
	 * @brief 定时器回调
	 *
	 * 处理定时器事件
	 *
	 * @param nTimerID 定时器ID
	 * @return 处理结果状态码
	 */
	virtual int OnTimer(uint32_t nTimerID) override;

	/**
	 * @brief 执行事件
	 *
	 * 处理服务器间的事件消息
	 *
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 源类型
	 * @param nSrcID 源ID
	 * @param pMessage 消息内容
	 * @return 处理结果状态码
	 */
    virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage) override;

	/**
	 * @brief 连接全局服务器
	 *
	 * 建立与全局服务器的连接
	 *
	 * @return 连接结果状态码
	 */
	int ConnectGlobalServer();

	/**
	 * @brief 服务器报告
	 *
	 * 向全局服务器报告当前服务器状态
	 *
	 * @return 报告结果状态码
	 */
    int ServerReport();

	/**
	 * @brief 同步服务器信息到其他服务器
	 *
	 * @param pServerData 服务器数据
	 * @return 同步结果状态码
	 */
	/**
	 * @brief 同步服务器信息到其他服务器
	 *
	 * 将新注册的服务器信息同步到所有其他已注册的服务器：
	 * - 获取所有已注册的服务器列表
	 * - 过滤出需要同步的目标服务器
	 * - 发送服务器信息到目标服务器
	 * - 处理同步结果
	 *
	 * @param pServerData 要同步的服务器数据指针
	 *
	 * @details 处理流程：
	 * 1. 获取所有已注册的服务器列表
	 * 2. 过滤出需要同步的目标服务器（排除自身和系统服务器）
	 * 3. 构建服务器信息报告列表
	 * 4. 向每个目标服务器发送服务器信息
	 * 5. 处理发送结果，记录成功和失败的数量
	 * 6. 记录同步日志
	 *
	 * @note 重要说明：
	 * - 同步的目标服务器包括游戏服务器、登录服务器等
	 * - 系统服务器（如主服务器、路由服务器等）不参与同步
	 * - 同步失败不会影响主服务器的正常运行
	 * - 同步操作是异步的，不会阻塞主服务器
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int SynServerToOthers(NF_SHARE_PTR<NFServerData> pServerData);

	/**
	 * @brief 同步其他服务器信息到指定服务器
	 *
	 * @param pServerData 服务器数据
	 * @return 同步结果状态码
	 */
    	/**
	 * @brief 同步其他服务器信息到指定服务器
	 *
	 * 将所有其他已注册的服务器信息同步到指定的目标服务器：
	 * - 获取所有已注册的服务器列表
	 * - 过滤出要同步的源服务器
	 * - 发送服务器信息到目标服务器
	 * - 处理同步结果
	 *
	 * @param pServerData 目标服务器数据指针
	 *
	 * @details 处理流程：
	 * 1. 获取所有已注册的服务器列表
	 * 2. 过滤出要同步的源服务器（排除目标服务器和系统服务器）
	 * 3. 构建服务器信息报告列表
	 * 4. 向目标服务器发送所有其他服务器的信息
	 * 5. 处理发送结果
	 * 6. 记录同步日志
	 *
	 * @note 重要说明：
	 * - 同步的源服务器包括游戏服务器、登录服务器等
	 * - 系统服务器（如主服务器、路由服务器等）不参与同步
	 * - 目标服务器会收到所有其他服务器的完整信息
	 * - 同步操作是异步的，不会阻塞主服务器
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int SynOtherServerToServer(NF_SHARE_PTR<NFServerData> pServerData);

	/**
	 * @brief 处理服务器注册
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
	/**
	 * @brief 处理服务器注册 - 主服务器注册处理
	 *
	 * 处理来自各种服务器的注册请求，维护服务器列表和状态：
	 * - 解析服务器注册信息
	 * - 验证服务器信息有效性
	 * - 创建或更新服务器数据
	 * - 同步服务器信息到其他服务器
	 * - 发送注册响应
	 *
	 * @param unLinkId 连接ID，标识注册服务器的连接
	 * @param packet 数据包，包含服务器注册信息
	 *
	 * @details 处理流程：
	 * 1. 解析服务器信息报告列表
	 * 2. 验证服务器信息的完整性和有效性
	 * 3. 检查服务器是否已存在，如果不存在则创建新的服务器数据
	 * 4. 更新服务器的连接ID和服务器信息
	 * 5. 将新服务器信息同步到所有其他已注册的服务器
	 * 6. 发送注册成功响应给注册的服务器
	 * 7. 记录详细的注册日志
	 *
	 * @note 重要约束：
	 * - 每个服务器只能注册一次
	 * - 服务器重新连接时会更新连接信息
	 * - 注册成功后会自动同步到所有其他服务器
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int OnServerRegisterProcess(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理服务器注册RPC服务
	 *
	 * @param unLinkId 连接ID
	 * @param reqeust 请求数据
	 * @param respone 响应数据
	 * @return 处理结果状态码
	 */
    int OnServerRegisterRpcService(uint64_t unLinkId, NFrame::ServerInfoReportList& reqeust, NFrame::ServerInfoReportListRespne& respone);

	/**
	 * @brief 处理服务器报告
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
	/**
	 * @brief 处理服务器报告 - 服务器状态报告处理
	 *
	 * 处理来自各种服务器的状态报告，更新服务器状态信息：
	 * - 解析服务器状态报告
	 * - 更新服务器运行状态
	 * - 监控服务器健康度
	 * - 处理服务器状态变化
	 * - 同步状态信息到其他服务器
	 *
	 * @param unLinkId 连接ID，标识报告服务器的连接
	 * @param packet 数据包，包含服务器状态报告信息
	 *
	 * @details 处理流程：
	 * 1. 解析服务器状态报告列表
	 * 2. 更新服务器的运行状态和系统信息
	 * 3. 监控服务器的CPU、内存等资源使用情况
	 * 4. 检测服务器状态变化，如启动、停止、故障等
	 * 5. 将状态变化同步到其他相关服务器
	 * 6. 记录服务器状态变化日志
	 * 7. 触发相应的状态处理逻辑
	 *
	 * @note 重要说明：
	 * - 服务器状态报告包含CPU、内存、连接数等系统信息
	 * - 状态变化会触发相应的处理逻辑
	 * - 故障服务器会被标记并通知其他服务器
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int OnServerReportProcess(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理服务器信息转储
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
	int OnServerDumpInfoProcess(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理杀死所有服务器
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int OnServerKillAllServerProcess(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理重载服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    	/**
	 * @brief 处理重新加载服务器 - HTTP接口
	 *
	 * 通过HTTP接口重新加载指定服务器：
	 * - 验证服务器ID的有效性
	 * - 发送重新加载命令到目标服务器
	 * - 处理重新加载响应
	 * - 返回HTTP响应结果
	 *
	 * @param serverId 目标服务器ID
	 * @param req HTTP请求对象
	 *
	 * @details 处理流程：
	 * 1. 验证服务器ID是否存在和有效
	 * 2. 检查目标服务器的当前状态
	 * 3. 发送重新加载命令到目标服务器
	 * 4. 等待服务器重新加载响应
	 * 5. 处理重新加载结果
	 * 6. 返回HTTP响应，包含操作结果
	 *
	 * @note HTTP接口说明：
	 * - 请求路径：/reload?serverId=xxx
	 * - 请求方法：GET
	 * - 响应格式：JSON
	 * - 成功响应：{"code":0,"msg":"success"}
	 * - 失败响应：{"code":-1,"msg":"error message"}
	 *
	 * @return 处理结果，true表示成功，false表示失败
	 */
	bool HandleReloadServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理重载所有服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    	/**
	 * @brief 处理重新加载所有服务器 - HTTP接口
	 *
	 * 通过HTTP接口重新加载所有服务器：
	 * - 获取所有已注册的服务器列表
	 * - 批量发送重新加载命令
	 * - 处理批量重新加载响应
	 * - 返回HTTP响应结果
	 *
	 * @param serverId 服务器ID（此参数在此接口中不使用）
	 * @param req HTTP请求对象
	 *
	 * @details 处理流程：
	 * 1. 获取所有已注册的服务器列表
	 * 2. 过滤出可重新加载的服务器
	 * 3. 批量发送重新加载命令到所有服务器
	 * 4. 等待所有服务器的重新加载响应
	 * 5. 统计重新加载成功和失败的服务器数量
	 * 6. 返回HTTP响应，包含批量操作结果
	 *
	 * @note HTTP接口说明：
	 * - 请求路径：/reloadall
	 * - 请求方法：GET
	 * - 响应格式：JSON
	 * - 成功响应：{"code":0,"msg":"success","data":{"success":10,"failed":2}}
	 * - 失败响应：{"code":-1,"msg":"error message"}
	 *
	 * @return 处理结果，true表示成功，false表示失败
	 */
	bool HandleReloadAllServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理重启服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    	/**
	 * @brief 处理重启服务器 - HTTP接口
	 *
	 * 通过HTTP接口重启指定服务器：
	 * - 验证服务器ID的有效性
	 * - 发送重启命令到目标服务器
	 * - 处理重启响应
	 * - 返回HTTP响应结果
	 *
	 * @param serverId 目标服务器ID
	 * @param req HTTP请求对象
	 *
	 * @details 处理流程：
	 * 1. 验证服务器ID是否存在和有效
	 * 2. 检查目标服务器的当前状态
	 * 3. 发送重启命令到目标服务器
	 * 4. 等待服务器重启响应
	 * 5. 处理重启结果
	 * 6. 返回HTTP响应，包含操作结果
	 *
	 * @note HTTP接口说明：
	 * - 请求路径：/restart?serverId=xxx
	 * - 请求方法：GET
	 * - 响应格式：JSON
	 * - 成功响应：{"code":0,"msg":"success"}
	 * - 失败响应：{"code":-1,"msg":"error message"}
	 *
	 * @return 处理结果，true表示成功，false表示失败
	 */
	bool HandleRestartServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理重启所有服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    bool HandleRestartAllServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理启动服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    	/**
	 * @brief 处理启动服务器 - HTTP接口
	 *
	 * 通过HTTP接口启动指定服务器：
	 * - 验证服务器ID的有效性
	 * - 发送启动命令到目标服务器
	 * - 处理启动响应
	 * - 返回HTTP响应结果
	 *
	 * @param serverId 目标服务器ID
	 * @param req HTTP请求对象
	 *
	 * @details 处理流程：
	 * 1. 验证服务器ID是否存在和有效
	 * 2. 检查目标服务器的当前状态
	 * 3. 发送启动命令到目标服务器
	 * 4. 等待服务器启动响应
	 * 5. 处理启动结果
	 * 6. 返回HTTP响应，包含操作结果
	 *
	 * @note HTTP接口说明：
	 * - 请求路径：/start?serverId=xxx
	 * - 请求方法：GET
	 * - 响应格式：JSON
	 * - 成功响应：{"code":0,"msg":"success"}
	 * - 失败响应：{"code":-1,"msg":"error message"}
	 *
	 * @return 处理结果，true表示成功，false表示失败
	 */
	bool HandleStartServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理启动所有服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    bool HandleStartAllServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理停止服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    	/**
	 * @brief 处理停止服务器 - HTTP接口
	 *
	 * 通过HTTP接口停止指定服务器：
	 * - 验证服务器ID的有效性
	 * - 发送停止命令到目标服务器
	 * - 处理停止响应
	 * - 返回HTTP响应结果
	 *
	 * @param serverId 目标服务器ID
	 * @param req HTTP请求对象
	 *
	 * @details 处理流程：
	 * 1. 验证服务器ID是否存在和有效
	 * 2. 检查目标服务器的当前状态
	 * 3. 发送停止命令到目标服务器
	 * 4. 等待服务器停止响应
	 * 5. 处理停止结果
	 * 6. 返回HTTP响应，包含操作结果
	 *
	 * @note HTTP接口说明：
	 * - 请求路径：/stop?serverId=xxx
	 * - 请求方法：GET
	 * - 响应格式：JSON
	 * - 成功响应：{"code":0,"msg":"success"}
	 * - 失败响应：{"code":-1,"msg":"error message"}
	 *
	 * @return 处理结果，true表示成功，false表示失败
	 */
	bool HandleStopServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理停止所有服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    bool HandleStopAllServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理杀死所有服务器HTTP请求
	 *
	 * @param serverId 服务器ID
	 * @param req HTTP请求
	 * @return 处理结果
	 */
    	/**
	 * @brief 处理强制关闭所有服务器 - HTTP接口
	 *
	 * 通过HTTP接口强制关闭所有服务器：
	 * - 获取所有已注册的服务器列表
	 * - 批量发送强制关闭命令
	 * - 处理批量关闭响应
	 * - 返回HTTP响应结果
	 *
	 * @param serverId 服务器ID（此参数在此接口中不使用）
	 * @param req HTTP请求对象
	 *
	 * @details 处理流程：
	 * 1. 获取所有已注册的服务器列表
	 * 2. 过滤出可关闭的服务器
	 * 3. 批量发送强制关闭命令到所有服务器
	 * 4. 等待所有服务器的关闭响应
	 * 5. 统计关闭成功和失败的服务器数量
	 * 6. 返回HTTP响应，包含批量操作结果
	 *
	 * @note HTTP接口说明：
	 * - 请求路径：/killall
	 * - 请求方法：GET
	 * - 响应格式：JSON
	 * - 成功响应：{"code":0,"msg":"success","data":{"success":10,"failed":2}}
	 * - 失败响应：{"code":-1,"msg":"error message"}
	 * - 警告：此操作会强制关闭所有服务器，请谨慎使用
	 *
	 * @return 处理结果，true表示成功，false表示失败
	 */
	bool HandleKillAllServer(uint32_t serverId, const NFIHttpHandle &req);

	/**
	 * @brief 处理停止服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleStopSeverRsp(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理启动服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleStartSeverRsp(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理重启服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleRestartSeverRsp(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理重载服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleReloadSeverRsp(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理停止所有服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleStopAllSeverRsp(uint64_t unLinkId, NFDataPackage& packetn);

	/**
	 * @brief 处理启动所有服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleStartAllSeverRsp(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理重启所有服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleRestartAllSeverRsp(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理重载所有服务器响应
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int HandleReloadAllSeverRsp(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理全局服务器Socket事件
	 *
	 * @param nEvent 事件类型
	 * @param unLinkId 连接ID
	 * @return 处理结果状态码
	 */
    	/**
	 * @brief 处理全局服务器Socket事件
	 *
	 * 处理全局服务器相关的Socket连接事件：
	 * - 连接成功事件处理
	 * - 连接断开事件处理
	 * - 注册到全局服务器
	 * - 完成启动任务
	 *
	 * @param nEvent 事件类型（连接成功/断开）
	 * @param unLinkId 连接ID
	 *
	 * @details 处理流程：
	 * 1. 根据事件类型进行相应处理
	 * 2. 连接成功时：
	 *    - 记录连接成功日志
	 *    - 注册当前主服务器到全局服务器
	 *    - 完成启动任务
	 * 3. 连接断开时：
	 *    - 记录连接断开日志
	 *    - 清理相关资源
	 *
	 * @note 重要说明：
	 * - 全局服务器用于跨服管理和协调
	 * - 连接成功后会立即注册到全局服务器
	 * - 连接断开不会影响主服务器的正常运行
	 * - 支持自动重连机制
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int OnGlobalSocketEvent(eMsgType nEvent, uint64_t unLinkId);

	/**
	 * @brief 处理全局服务器其他消息
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
    int OnHandleGlobalOtherMessage(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 注册到全局服务器
	 *
	 * @return 注册结果状态码
	 */
    	/**
	 * @brief 注册到全局服务器
	 *
	 * 向全局服务器注册当前主服务器的信息：
	 * - 收集当前主服务器信息
	 * - 构建注册消息
	 * - 发送注册请求到全局服务器
	 * - 处理注册结果
	 *
	 * @details 处理流程：
	 * 1. 获取当前主服务器的配置信息
	 * 2. 构建服务器信息报告
	 * 3. 设置服务器状态为正常
	 * 4. 发送注册消息到全局服务器
	 * 5. 记录注册日志
	 *
	 * @note 重要说明：
	 * - 注册信息包含服务器ID、IP、端口等基本信息
	 * - 服务器状态设置为EST_NARMAL表示正常运行
	 * - 注册成功后会建立与全局服务器的管理关系
	 * - 注册失败不会影响主服务器的正常运行
	 *
	 * @return 注册结果状态码，0表示成功，-1表示失败
	 */
	int RegisterGlobalServer();

protected:
	/**
	 * @brief 处理代理服务器Socket事件
	 *
	 * @param nEvent 事件类型
	 * @param unLinkId 连接ID
	 * @return 处理结果状态码
	 */
	/**
	 * @brief 处理代理服务器Socket事件
	 *
	 * 处理代理服务器相关的Socket连接事件：
	 * - 连接成功事件处理
	 * - 连接断开事件处理
	 * - 客户端断开处理
	 * - 完成启动任务
	 *
	 * @param nEvent 事件类型（连接成功/断开）
	 * @param unLinkId 连接ID
	 *
	 * @details 处理流程：
	 * 1. 根据事件类型进行相应处理
	 * 2. 连接成功时：
	 *    - 记录连接成功日志
	 *    - 完成启动任务
	 * 3. 连接断开时：
	 *    - 记录连接断开日志
	 *    - 调用客户端断开处理
	 *    - 清理相关资源
	 *
	 * @note 重要说明：
	 * - 代理服务器用于客户端连接管理
	 * - 连接成功后会完成相应的启动任务
	 * - 连接断开会触发客户端断开处理
	 * - 支持自动重连机制
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int OnProxySocketEvent(eMsgType nEvent, uint64_t unLinkId);

	/**
	 * @brief 处理其他消息
	 *
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return 处理结果状态码
	 */
	/**
	 * @brief 处理其他消息 - 代理服务器消息处理
	 *
	 * 处理来自代理服务器的其他类型消息：
	 * - 解析消息内容
	 * - 记录消息日志
	 * - 处理未注册的协议消息
	 * - 转发消息到目标服务器
	 *
	 * @param unLinkId 连接ID，标识消息来源
	 * @param packet 数据包，包含消息内容
	 *
	 * @details 处理流程：
	 * 1. 解析消息数据包
	 * 2. 记录消息处理日志
	 * 3. 根据消息类型进行相应处理
	 * 4. 转发消息到目标服务器（如果需要）
	 * 5. 处理消息响应
	 *
	 * @note 重要说明：
	 * - 主要用于处理代理服务器转发的客户端消息
	 * - 支持消息的转发和路由
	 * - 记录详细的消息处理日志
	 * - 处理未注册的协议消息
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int OnHandleOtherMessage(uint64_t unLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理客户端断开连接
	 *
	 * @param unLinkId 连接ID
	 * @return 处理结果状态码
	 */
	/**
	 * @brief 处理客户端断开连接
	 *
	 * 处理客户端断开连接事件：
	 * - 查找对应的客户端连接
	 * - 清理连接资源
	 * - 更新连接状态
	 * - 记录断开日志
	 *
	 * @param unLinkId 断开连接的ID，标识断开的客户端连接
	 *
	 * @details 处理流程：
	 * 1. 根据连接ID查找对应的客户端连接
	 * 2. 更新连接状态为断开
	 * 3. 清理相关的连接资源
	 * 4. 记录详细的断开日志
	 * 5. 通知相关系统客户端断开事件
	 *
	 * @note 重要说明：
	 * - 客户端断开后，该连接将无法接收消息
	 * - 连接资源会被自动清理
	 * - 断开事件会通知相关系统
	 * - 支持客户端重新连接
	 *
	 * @return 处理结果状态码，0表示成功，-1表示失败
	 */
	int OnClientDisconnect(uint64_t unLinkId);
};
