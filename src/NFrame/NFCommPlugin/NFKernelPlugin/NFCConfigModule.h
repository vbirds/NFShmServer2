// -------------------------------------------------------------------------
//    @FileName         :    NFCConfigModule.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFCConfigModule
//    @Desc             :    配置模块头文件，负责加载和管理系统各种配置。
//                          该文件定义了NFShmXFrame框架的配置管理模块，提供系统配置的加载和管理功能，
//                          包括插件配置、服务器配置、日志配置、应用程序配置、数据库配置、服务发现配置。
//                          主要功能包括配置热重载、Lua脚本配置支持、按服务器类型组织配置、
//                          统一配置访问接口、运行时配置重载
//    @Description      :    配置模块头文件，负责加载和管理系统各种配置
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIConfigModule.h"

#include "NFComm/NFPluginModule/NFILuaLoader.h"
#include <string>

/**
 * @class NFCConfigModule
 * @brief 配置模块实现类
 *
 * NFCConfigModule负责管理NFShmXFrame框架的所有配置信息，包括：
 * 
 * 配置类型：
 * - 插件配置(Plugin Config)：各个插件的配置信息
 * - 服务器配置(Server Config)：服务器运行参数和网络配置
 * - 日志配置(Log Config)：日志系统的配置
 * - 应用程序配置(App Config)：应用级别的配置
 * 
 * 数据库配置：
 * - 默认数据库名称和连接信息
 * - 跨服数据库配置
 * - Redis缓存配置（IP、端口、密码）
 * 
 * 服务发现：
 * - Master服务器信息配置
 * - 各类型服务器的配置信息
 * 
 * 特性：
 * - 支持配置热重载
 * - 继承Lua加载器接口，支持Lua脚本配置
 * - 按服务器类型组织配置信息
 * - 提供统一的配置访问接口
 * 
 * @note 该模块是框架启动的关键组件，在日志模块之后第二个启动
 * @note 支持运行时配置重载，无需重启服务器
 */
class NFCConfigModule : public NFIConfigModule, public NFILuaLoader
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFCConfigModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 */
	virtual ~NFCConfigModule();

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
	 * @brief 最终化处理
	 * @return 返回0表示成功，非0表示失败
	 */
	int Finalize() override;
	
	/**
	 * @brief 模块定时更新
	 * @return 返回0表示成功，非0表示失败
	 */
	int Tick() override;
	
	/**
	 * @brief 重新加载配置
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 支持运行时重新加载所有配置文件，实现配置热更新功能。
	 */
	int OnReloadConfig() override;

	/**
	 * @brief 加载框架配置
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 加载框架的所有核心配置，包括插件配置、服务器配置和日志配置。
	 * 这是配置模块的主要入口函数。
	 */
	virtual int LoadFrameConfig() override;

public:
	/**
	 * @brief 加载插件配置
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 从配置文件中加载各个插件的配置信息。
	 */
	bool LoadPluginConfig();
	
	/**
	 * @brief 加载服务器配置
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 加载服务器的网络配置、运行参数等信息。
	 */
	bool LoadServerConfig();
	
	/**
	 * @brief 加载日志配置
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 加载日志系统的配置，包括日志级别、输出目录等。
	 */
	bool LoadLogConfig();

public:
	/**
	 * @brief 获取插件配置
	 * @param pluginName 插件名称
	 * @return 返回插件配置指针，不存在则返回nullptr
	 * 
	 * 根据插件名称获取对应的配置信息。
	 */
	NFPluginConfig* GetPluginConfig(const std::string& pluginName) override;
	
	/**
	 * @brief 获取日志配置
	 * @return 返回日志配置指针
	 * 
	 * 获取系统的日志配置信息。
	 */
	NFLogConfig* GetLogConfig() override;

public:
	/**
	 * @brief 根据服务器类型获取服务器配置
	 * @param eServerType 服务器类型枚举
	 * @return 返回服务器配置指针，不存在则返回nullptr
	 */
    virtual NFServerConfig* GetServerConfig(NF_SERVER_TYPE eServerType) override;
    
	/**
	 * @brief 根据服务器类型获取应用配置
	 * @param eServerType 服务器类型枚举
	 * @return 返回应用配置指针，不存在则返回nullptr
	 */
    virtual NFServerConfig* GetAppConfig(NF_SERVER_TYPE eServerType) override;
    
	/**
	 * @brief 获取默认数据库名称
	 * @param nfServerTypes 服务器类型枚举
	 * @return 返回数据库名称字符串
	 */
    virtual std::string GetDefaultDBName(NF_SERVER_TYPE nfServerTypes) override;
    
	/**
	 * @brief 获取跨服数据库名称
	 * @param nfServerTypes 服务器类型枚举
	 * @return 返回跨服数据库名称字符串
	 */
    virtual std::string GetCrossDBName(NF_SERVER_TYPE nfServerTypes) override;
    
	/**
	 * @brief 获取Redis服务器IP地址
	 * @param nfServerTypes 服务器类型枚举
	 * @return 返回Redis IP地址字符串
	 */
    virtual std::string GetRedisIp(NF_SERVER_TYPE nfServerTypes) override;
    
	/**
	 * @brief 获取Redis服务器端口
	 * @param nfServerTypes 服务器类型枚举
	 * @return 返回Redis端口号
	 */
    virtual uint32_t GetRedisPort(NF_SERVER_TYPE nfServerTypes) override;
    
	/**
	 * @brief 获取Redis服务器密码
	 * @param nfServerTypes 服务器类型枚举
	 * @return 返回Redis密码字符串
	 */
    virtual std::string GetRedisPass(NF_SERVER_TYPE nfServerTypes) override;
    
	/**
	 * @brief 获取默认Master服务器信息
	 * @param eServerType 服务器类型枚举
	 * @return 返回Master服务器信息结构体
	 * 
	 * 获取指定服务器类型对应的Master服务器的连接信息。
	 */
    virtual NFrame::ServerInfoReport GetDefaultMasterInfo(NF_SERVER_TYPE eServerType) override;

protected:
	/**
	 * @brief 插件配置映射表
	 * 
	 * 以插件名称为键，插件配置对象为值的映射表。
	 * 用于快速查找特定插件的配置信息。
	 */
	std::unordered_map<std::string, NFPluginConfig*> mPluginConfig; //pluginName--key
	
	/**
	 * @brief 服务器配置列表
	 * 
	 * 存储所有服务器的配置信息，以服务器ID为索引。
	 */
	std::vector<NFServerConfig*> mServerConfig; //serverid--key
	
	/**
	 * @brief 应用程序配置
	 * 
	 * 当前应用程序的配置信息指针。
	 */
    NFServerConfig* m_appConfig;
    
	/**
	 * @brief 日志配置对象
	 * 
	 * 存储系统日志的配置信息。
	 */
	NFLogConfig mLogConfig;
};
