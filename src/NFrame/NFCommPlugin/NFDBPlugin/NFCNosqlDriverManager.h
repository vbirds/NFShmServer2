// -------------------------------------------------------------------------
//    @FileName         :    NFCNosqlDriverManager.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFCNosqlDriverManager
//    @Desc             :    NoSQL驱动管理器头文件，负责管理多个NoSQL数据库连接。
//                          该文件定义了NFShmXFrame框架的NoSQL驱动管理器类，负责管理多个NoSQL数据库驱动实例，
//                          主要是Redis驱动，提供驱动的添加、获取、连接状态检查和定期维护功能。
//                          主要功能包括管理多个NoSQL服务器连接、连接池管理维护驱动实例的生命周期、
//                          自动重连检测断线并自动重连、状态监控定期检查连接状态和认证状态、
//                          负载均衡为不同的服务器提供独立的驱动实例
//    @Description      :    NoSQL驱动管理器头文件，负责管理多个NoSQL数据库连接
//
// -------------------------------------------------------------------------

#pragma once

#include "NFCommPlugin/NFDBPlugin/NFRedisDriver.h"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFCore/NFCommMap.hpp"

/**
 * @class NFCNosqlDriverManager
 * @brief NoSQL驱动管理器类
 * 
 * 负责管理多个NoSQL数据库驱动实例，主要是Redis驱动。
 * 提供驱动的添加、获取、连接状态检查和定期维护功能。
 * 
 * 主要功能：
 * - 管理多个NoSQL服务器连接：支持同时连接多个Redis服务器
 * - 连接池管理：维护驱动实例的生命周期
 * - 自动重连：检测断线并自动重连
 * - 状态监控：定期检查连接状态和认证状态
 * - 负载均衡：为不同的服务器提供独立的驱动实例
 */
class NFCNosqlDriverManager
{
public:
    /**
     * @brief 构造函数
     * 初始化NoSQL驱动管理器，设置默认参数
     */
    NFCNosqlDriverManager();
    
    /**
     * @brief 析构函数
     * 释放所有NoSQL驱动实例，清理资源
     */
    virtual ~NFCNosqlDriverManager();

public:
    /**
     * @brief 主循环处理函数
     * @return 返回0表示成功，非0表示失败
     * 
     * 执行所有NoSQL驱动的主循环处理，包括：
     * - 执行各个驱动的Execute方法
     * - 定期检查连接状态
     * - 处理重连逻辑
     */
    virtual int Tick();

public:
    /**
     * @brief 添加NoSQL服务器（使用默认端口）
     * @param strID 服务器唯一标识符
     * @param strIP 服务器IP地址
     * @return 返回0表示添加成功，-1表示失败
     * 
     * 使用默认Redis端口（6379）和空密码连接服务器
     */
    virtual int AddNosqlServer(const std::string& strID, const std::string& strIP);
    
    /**
     * @brief 添加NoSQL服务器（指定端口）
     * @param strID 服务器唯一标识符
     * @param strIP 服务器IP地址
     * @param nPort 服务器端口号
     * @return 返回0表示添加成功，-1表示失败
     * 
     * 使用指定端口和空密码连接服务器
     */
    virtual int AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort);
    
    /**
     * @brief 添加NoSQL服务器（完整参数）
     * @param strID 服务器唯一标识符
     * @param strIP 服务器IP地址
     * @param nPort 服务器端口号
     * @param strPass 服务器连接密码
     * @return 返回0表示添加成功，-1表示失败
     * 
     * 使用完整参数连接服务器，包括认证密码
     */
    virtual int AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort, const std::string& strPass);

public:
    /**
     * @brief 检查所有连接是否可用
     * @return 返回false（当前实现为占位符）
     * 
     * @note 当前实现返回固定值，预留给子类扩展
     */
    virtual bool Enable();
    
    /**
     * @brief 检查是否有驱动处于繁忙状态
     * @return 返回false（当前实现为占位符）
     * 
     * @note 当前实现返回固定值，预留给子类扩展
     */
    virtual bool Busy();
    
    /**
     * @brief 保持所有连接活跃
     * @return 返回false（当前实现为占位符）
     * 
     * @note 当前实现返回固定值，预留给子类扩展
     */
    virtual bool KeepLive();

public:
    /**
     * @brief 定期检查NoSQL连接状态
     * 
     * 检查所有NoSQL驱动的连接状态，处理以下情况：
     * - 连接正常但未认证：执行认证操作
     * - 连接断开：尝试重新连接
     * 
     * 该方法每15秒执行一次检查
     */
    void CheckNoSql();

public:
    /**
     * @brief 获取指定的NoSQL驱动
     * @param strID 服务器唯一标识符
     * @return 返回可用的NoSQL驱动指针，如果不存在或不可用则返回nullptr
     * 
     * 只返回状态为Enable的驱动实例，确保返回的驱动可以正常使用
     */
    virtual NFINosqlDriver* GetNosqlDriver(const std::string& strID);

protected:
    /**
     * @brief NoSQL驱动映射表
     * 存储服务器ID到NoSQL驱动实例的映射关系
     */
    NFCommMap<std::string, NFINosqlDriver> mxNoSqlDriver;
    
    /**
     * @brief 最后一次检查时间
     * 用于控制CheckNoSql方法的执行频率
     */
    int mLastCheckTime = 0;
};