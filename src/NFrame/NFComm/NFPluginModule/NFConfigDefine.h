// -------------------------------------------------------------------------
//    @FileName         :    NFConfigDefine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    配置系统定义文件，包含系统配置相关的常量、枚举和数据结构
//                           提供插件配置、日志配置、服务器配置等核心配置功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFILuaLoader.h"
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFKernelMessage/FrameComm.nanopb.h"
#include "NFComm/NFPluginModule/NFProtobufCommon.h"
#include <unordered_map>

/// @brief Lua配置中插件加载函数名称常量
#define DEFINE_LUA_STRING_LOAD_PLUGIN            "LoadPlugin"

/// @brief Lua配置中框架插件配置项名称
#define DEFINE_LUA_STRING_FRAME_PLUGINS         "FramePlugins"

/// @brief Lua配置中服务器插件配置项名称  
#define DEFINE_LUA_STRING_SERVER_PLUGINS        "ServerPlugins"

/// @brief Lua配置中工作插件配置项名称
#define DEFINE_LUA_STRING_WORK_PLUGINS          "WorkPlugins"

/// @brief Lua配置中日志级别配置项名称
#define DEFINE_LUA_STRING_LOG_LEVEL                "LogLevel"

/// @brief Lua配置中日志刷新级别配置项名称
#define DEFINE_LUA_STRING_LOG_FLUSH_LEVEL        "LogFlushLevel"

/// @brief Lua配置中日志信息配置项名称
#define DEFINE_LUA_STRING_LOG_INFO                "LogInfo"

/// @brief Lua配置中外部数据配置项名称
#define DEFINE_LUA_STRING_EXTERNAL_DATA         "ExternalData"

/**
 * @brief 日志信息配置项在Lua数组中的索引枚举
 * 
 * 定义了日志配置信息在Lua配置数组中的索引位置，
 * 用于解析和访问具体的日志配置参数。
 */
enum EnumLogInfoLuaString
{
    LOG_INFO_LOG_ID = 0,     ///< 日志ID索引
    LOG_INFO_DISPLAY = 1,    ///< 日志显示配置索引
    LOG_INFO_LEVEL = 2,      ///< 日志级别索引
    LOG_INFO_LOG_NAME = 3,   ///< 日志名称索引
    LOG_INFO_LOG_GUID = 4,   ///< 日志GUID索引
};

/// @brief 插件配置类型别名，基于protobuf消息定义
typedef pbPluginConfig NFPluginConfig;

/**
 * @brief 日志配置结构体
 * 
 * 管理系统的日志配置信息，包括多个日志配置项的集合。
 * 支持配置清理和重新加载功能。
 */
struct NFLogConfig
{
    /**
     * @brief 默认构造函数
     * 
     * 初始化日志配置并清理所有配置项。
     */
    NFLogConfig()
    {
        Clear();
    }
    
    /**
     * @brief 清理所有日志配置
     * 
     * 清空所有已加载的日志配置项，通常在重新加载配置时调用。
     */
    void Clear()
    {
        mLineConfigList.clear();
    }
    
    /// @brief 日志配置项列表，存储所有的日志配置信息
    std::vector<LogInfoConfig> mLineConfigList;
};

/**
 * @brief 服务器配置结构体
 * 
 * 继承自protobuf定义的服务器配置，提供完整的服务器配置管理功能：
 * 
 * 1. 基础配置管理：
 *    - 服务器ID、名称、总线ID
 *    - 区域ID、世界ID等层次结构
 * 
 * 2. 时间配置管理：
 *    - 开服时间配置和计算
 *    - 零点时间计算和缓存
 * 
 * 3. 数据库配置管理：
 *    - 默认数据库和跨服数据库配置
 *    - 表配置映射管理
 * 
 * 4. 网络配置管理：
 *    - 心跳超时配置
 *    - 客户端保活超时配置
 * 
 * 5. 业务配置管理：
 *    - 最大在线玩家数配置
 *    - 跨服标识配置
 *    - 外部数据配置
 */
struct NFServerConfig : public pbNFServerConfig
{
public:
    /**
     * @brief 默认构造函数
     * 
     * 初始化服务器配置，设置默认值。
     */
    NFServerConfig()
    {
        m_openZeroTime = 0;
    }
    
    /**
     * @brief 解析配置数据
     * 
     * 解析protobuf配置数据，建立表配置映射和计算派生数据。
     * 包括表配置映射的建立和开服零点时间的计算。
     */
    void Parse()
    {
        for (int i = 0; i < (int) MysqlConfig.TBConfList.size(); i++)
        {
            struct pbTableConfig &tableConfig = MysqlConfig.TBConfList[i];
            mTBConfMap.emplace(tableConfig.TableName, tableConfig);
        }

        m_openZeroTime = NFTime::GetZeroTime(ServerOpenTime);
    }

    /**
     * @brief 获取开服时间
     * @return 返回服务器开放时间戳
     */
    uint64_t GetOpenTime() const { return ServerOpenTime; }
    
    /**
     * @brief 获取服务器开放时间
     * @return 返回服务器开放时间戳
     * 
     * 与GetOpenTime()功能相同，提供兼容接口。
     */
    uint64_t GetServerOpenTime() const { return ServerOpenTime; }
    
    /**
     * @brief 获取开服当天零点时间
     * @return 返回开服当天零点的时间戳
     * 
     * 返回开服日期当天零点的时间戳，用于各种时间计算和重置逻辑。
     */
    uint64_t GetOpenDayZeroTime() { return m_openZeroTime; }
    
    /**
     * @brief 获取服务器ID
     * @return 返回服务器唯一标识字符串
     */
    std::string GetServerId() const { return ServerId; }
    
    /**
     * @brief 获取服务器名称
     * @return 返回服务器显示名称
     */
    std::string GetServerName() const { return ServerName; }
    
    /**
     * @brief 获取总线ID
     * @return 返回服务器总线ID
     * 
     * 总线ID是服务器在整个系统中的唯一标识符。
     */
    uint32_t GetBusId() const { return BusId; }
    
    /**
     * @brief 获取区域ID
     * @return 返回从总线ID中解析出的区域ID
     * 
     * 从总线ID中提取区域标识，用于区域管理和路由。
     */
    uint32_t GetZoneId() const { return NFServerIDUtil::GetZoneID(BusId); }
    
    /**
     * @brief 获取世界ID
     * @return 返回从总线ID中解析出的世界ID
     * 
     * 从总线ID中提取世界标识，用于跨世界通信和管理。
     */
    uint32_t GetWorldId() const { return NFServerIDUtil::GetWorldID(BusId); }
    
    /**
     * @brief 获取默认数据库名称
     * @return 返回默认数据库名称字符串
     */
    std::string GetDefaultDBName() const { return DefaultDBName; }
    
    /**
     * @brief 获取跨服数据库名称
     * @return 返回跨服数据库名称字符串
     */
    std::string GetCrossDBName() const { return CrossDBName; }
    
    /**
     * @brief 获取最大在线玩家数
     * @return 返回服务器支持的最大在线玩家数量
     */
    uint32_t GetMaxOnlinePlayerNum() const { return MaxOnlinePlayerNum; }
    
    /**
     * @brief 获取心跳超时时间
     * @return 返回心跳超时时间（秒）
     */
    uint32_t GetHeartBeatTimeout() const { return HeartBeatTimeout; }
    
    /**
     * @brief 获取客户端保活超时时间
     * @return 返回客户端连接保活超时时间（秒）
     */
    uint32_t GetClientKeepAliveTimeout() const { return ClientKeepAliveTimeout; }
    
    /**
     * @brief 检查是否为跨服服务器
     * @return 如果是跨服服务器返回true，否则返回false
     */
    bool IsCrossServer() const { return CrossServer; }

    /**
     * @brief 获取外部数据配置
     * @tparam T 数据类型，必须支持ToPb()方法
     * @param data 用于接收配置数据的对象引用
     * @return 获取成功返回0，失败返回非0值
     * 
     * 从外部数据配置中获取指定类型的配置信息。
     * 支持任何实现了ToPb()方法的数据类型。
     */
    template<typename T>
    int GetExternData(T& data)
    {
        auto pb = data.ToPb();
        bool flag = NFProtobufCommon::LuaToProtoMessage(m_externData, &pb);
        if (!flag)
        {
            return -1;
        }

        data.FromPb(pb);
        return 0;
    }

    std::unordered_map<std::string, pbTableConfig> mTBConfMap;
    uint64_t m_openZeroTime;            //开服那天的0点时间
    LuaIntf::LuaRef m_externData;
};
