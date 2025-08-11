// -------------------------------------------------------------------------
//    @FileName         :    NFIConfigModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    配置模块接口定义，提供系统配置信息的统一管理和访问
//                           支持插件配置、服务器配置、数据库配置等多种配置类型
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIModule.h"

#include "NFIPluginManager.h"
#include "NFServerDefine.h"

#include <unordered_map>
#include <vector>

#include "NFConfigDefine.h"

/**
 * @brief 配置模块接口类
 * 
 * NFIConfigModule 提供了系统配置信息的统一管理功能：
 * 
 * 1. 插件配置管理：
 *    - 插件级别的配置信息
 *    - 插件启用状态管理
 * 
 * 2. 服务器配置管理：
 *    - 不同服务器类型的配置信息
 *    - 服务器连接和通信配置
 * 
 * 3. 数据库配置管理：
 *    - MySQL数据库连接配置
 *    - Redis缓存配置
 *    - 跨服数据库配置
 * 
 * 4. 日志配置管理：
 *    - 日志级别和输出配置
 *    - 日志文件路径配置
 * 
 * 5. 框架配置管理：
 *    - 核心框架参数配置
 *    - 系统运行时配置
 * 
 * 配置系统特性：
 * - 统一的配置访问接口
 * - 类型安全的配置获取
 * - 运行时配置热重载
 * - 多层次的配置管理
 * - 配置验证和错误处理
 */
class NFIConfigModule : public NFIModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFIConfigModule(NFIPluginManager* p) :NFIModule(p)
	{

	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构，释放配置资源。
	 */
	virtual ~NFIConfigModule()
	{

	}

	/**
	 * @brief 获取插件配置信息
	 * @param pluginName 插件名称
	 * @return 返回插件配置对象指针，未找到返回nullptr
	 * 
	 * 根据插件名称获取对应的配置信息，包括插件的启用状态、
	 * 配置参数等信息。
	 */
	virtual NFPluginConfig* GetPluginConfig(const std::string& pluginName) = 0;
	
	/**
	 * @brief 获取日志配置信息
	 * @return 返回日志配置对象指针
	 * 
	 * 获取系统的日志配置信息，包括日志级别、输出格式、
	 * 文件路径等设置。
	 */
	virtual NFLogConfig* GetLogConfig() = 0;
	
	/**
	 * @brief 加载框架配置
	 * @return 加载成功返回0，失败返回非0值
	 * 
	 * 加载核心框架的配置信息，包括系统基础参数、
	 * 运行时设置等。
	 */
	virtual int LoadFrameConfig() = 0;
	
    /**
     * @brief 获取服务器配置信息
     * @param eServerType 服务器类型
     * @return 返回服务器配置对象指针，未找到返回nullptr
     * 
     * 根据服务器类型获取对应的配置信息，包括服务器的
     * 网络配置、业务配置等。
     */
    virtual NFServerConfig* GetServerConfig(NF_SERVER_TYPE eServerType) = 0;
    
    /**
     * @brief 获取应用配置信息
     * @param eServerType 服务器类型
     * @return 返回应用配置对象指针，未找到返回nullptr
     * 
     * 获取特定服务器类型的应用级配置信息，可能包含
     * 应用特有的业务参数和设置。
     */
    virtual NFServerConfig* GetAppConfig(NF_SERVER_TYPE eServerType) = 0;
    
    /**
     * @brief 获取默认数据库名称
     * @param nfServerTypes 服务器类型
     * @return 返回数据库名称字符串
     * 
     * 根据服务器类型获取对应的默认MySQL数据库名称，
     * 用于数据持久化操作。
     */
    virtual std::string GetDefaultDBName(NF_SERVER_TYPE nfServerTypes) = 0;
    
    /**
     * @brief 获取跨服数据库名称
     * @param nfServerTypes 服务器类型
     * @return 返回跨服数据库名称字符串
     * 
     * 获取跨服数据共享使用的数据库名称，用于跨服
     * 数据同步和共享功能。
     */
    virtual std::string GetCrossDBName(NF_SERVER_TYPE nfServerTypes) = 0;
    
    /**
     * @brief 获取Redis服务器IP地址
     * @param nfServerTypes 服务器类型
     * @return 返回Redis IP地址字符串
     * 
     * 根据服务器类型获取对应的Redis缓存服务器IP地址。
     */
    virtual std::string GetRedisIp(NF_SERVER_TYPE nfServerTypes) = 0;
    
    /**
     * @brief 获取Redis服务器端口
     * @param nfServerTypes 服务器类型
     * @return 返回Redis端口号
     * 
     * 根据服务器类型获取对应的Redis缓存服务器端口号。
     */
    virtual uint32_t GetRedisPort(NF_SERVER_TYPE nfServerTypes) = 0;
    
    /**
     * @brief 获取Redis服务器密码
     * @param nfServerTypes 服务器类型
     * @return 返回Redis密码字符串
     * 
     * 根据服务器类型获取对应的Redis缓存服务器访问密码。
     */
    virtual std::string GetRedisPass(NF_SERVER_TYPE nfServerTypes) = 0;
    
    /**
     * @brief 获取默认主服务器信息
     * @param eServerType 服务器类型
     * @return 返回主服务器信息报告对象
     * 
     * 获取指定服务器类型对应的主服务器（Master）信息，
     * 包括服务器地址、端口、ID等连接所需的信息。
     */
    virtual NFrame::ServerInfoReport GetDefaultMasterInfo(NF_SERVER_TYPE eServerType) = 0;
};
