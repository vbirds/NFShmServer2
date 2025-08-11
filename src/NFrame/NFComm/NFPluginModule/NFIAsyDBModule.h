// -------------------------------------------------------------------------
//    @FileName         :    NFIAsyDBModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFIAsyDBModule
//    @Description      :    异步数据库模块接口定义，提供SQL和NoSQL的统一数据库访问
//                           支持MySQL、Redis等多种数据库的异步操作和连接管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIAsySqlModule.h"

/**
 * @brief 异步数据库模块接口类
 * 
 * NFIAsyDbModule 提供了统一的异步数据库访问接口：
 * 
 * 1. 多数据库支持：
 *    - 关系型数据库（MySQL等）
 *    - NoSQL数据库（Redis等）
 *    - 统一的配置管理
 *    - 混合数据库架构
 * 
 * 2. 连接管理功能：
 *    - 数据库服务器配置
 *    - 自动重连机制
 *    - 连接池管理
 *    - 故障转移支持
 * 
 * 3. 异步操作特性：
 *    - 继承SQL模块的异步能力
 *    - 非阻塞数据库操作
 *    - 回调驱动的结果处理
 *    - 高并发支持
 * 
 * 4. 配置和管理：
 *    - 灵活的服务器配置
 *    - 认证信息管理
 *    - 重连策略配置
 *    - 运行时参数调整
 * 
 * 设计特点：
 * - 基于NFIAsySqlModule的扩展
 * - 支持SQL和NoSQL混合使用
 * - 提供统一的配置接口
 * - 简化数据库集成过程
 * 
 * 使用场景：
 * - 游戏数据库系统
 * - 分布式数据存储
 * - 缓存和持久化结合
 * - 多数据源应用
 * 
 * 架构优势：
 * - 解耦SQL和NoSQL操作
 * - 统一的错误处理
 * - 配置驱动的连接管理
 * - 易于扩展和维护
 */
class NFIAsyDbModule
	: public NFIAsySqlModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 * 
	 * 初始化异步数据库模块，继承SQL模块的所有功能。
	 */
	explicit NFIAsyDbModule(NFIPluginManager* p) : NFIAsySqlModule(p)
	{

	}

	/**
	 * @brief 析构函数
	 * 
	 * 清理数据库连接和相关资源。
	 */
	~NFIAsyDbModule() override = default;

	/**
	 * @brief 添加混合数据库服务器配置
	 * @param nServerID 数据库服务器的唯一标识符
	 * @param strIP SQL数据库服务器的IP地址
	 * @param nPort SQL数据库服务器的端口号
	 * @param strDBName SQL数据库名称
	 * @param strDBUser SQL数据库用户名
	 * @param strDBPwd SQL数据库密码
	 * @param noSqlIp NoSQL数据库（如Redis）的IP地址
	 * @param nosqlPort NoSQL数据库的端口号
	 * @param noSqlPass NoSQL数据库的认证密码
	 * @param nReconnectTime 连接断开后的重连间隔时间（秒），默认10秒
	 * @param nReconnectCount 最大重连尝试次数，-1表示无限重连，默认无限重连
	 * @return 返回操作结果，0表示成功，非0表示失败
	 * 
	 * 该函数用于配置同时支持SQL和NoSQL的混合数据库环境：
	 * 
	 * 配置功能：
	 * - 同时配置关系型数据库和NoSQL数据库
	 * - 设置各自的连接参数和认证信息
	 * - 配置自动重连策略
	 * - 支持独立的端口和地址配置
	 * 
	 * 重连机制：
	 * - 自动检测连接状态
	 * - 按配置间隔重连
	 * - 支持有限或无限重连
	 * - 连接失败时的降级处理
	 * 
	 * 使用示例：
	 * @code
	 * // 配置MySQL + Redis的混合环境
	 * int result = dbModule->AddDbServer(
	 *     "main_db",           // 服务器标识
	 *     "192.168.1.100",     // MySQL IP
	 *     3306,                // MySQL 端口
	 *     "game_db",           // 数据库名
	 *     "dbuser",            // MySQL 用户名
	 *     "password123",       // MySQL 密码
	 *     "192.168.1.101",     // Redis IP
	 *     6379,                // Redis 端口
	 *     "redis_pass",        // Redis 密码
	 *     15,                  // 15秒重连间隔
	 *     5                    // 最多重连5次
	 * );
	 * @endcode
	 * 
	 * 配置建议：
	 * - 根据网络环境调整重连间隔
	 * - 生产环境建议设置有限重连次数
	 * - 测试环境可使用无限重连便于调试
	 * - 密码应使用安全的存储方式
	 * 
	 * @note 函数会验证所有连接参数的有效性
	 * @note 支持在运行时动态添加数据库配置
	 * @note 同一ServerID不能重复添加
	 * @warning 密码信息会在内存中明文存储，注意安全性
	 */
	virtual int AddDbServer(const std::string& nServerID, const std::string& strIP, int nPort, const std::string& strDBName, const std::string& strDBUser, const std::string& strDBPwd,
	                        const std::string& noSqlIp, int nosqlPort, const std::string& noSqlPass,
	                        int nReconnectTime = 10, int nReconnectCount = -1) = 0;
};
