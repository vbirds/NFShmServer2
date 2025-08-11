// -------------------------------------------------------------------------
//    @FileName         :    NFCMysqlDriverManager.h
//    @Author           :    Chuanbo.Guo
//    @Date             :   2022-09-18
//    @Module           :    NFCMysqlDriverManager
//    @Desc             :    MySQL驱动管理器头文件，负责管理多个MySQL数据库连接。
//                          该文件定义了NFShmXFrame框架的MySQL驱动管理器类，负责管理多个MySQL数据库驱动实例，
//                          提供连接池管理功能，支持多个MySQL服务器的连接，自动处理连接检查和重连逻辑。
//                          主要功能包括管理多个MySQL服务器连接、连接池管理维护有效和无效连接的分类管理、
//                          自动重连定期检查连接状态并处理断线重连、负载分发根据服务器ID分配相应的数据库连接、
//                          状态监控实时监控各连接的健康状态
//    @Description      :    MySQL驱动管理器头文件，负责管理多个MySQL数据库连接
//
// -------------------------------------------------------------------------

#pragma once

#include "NFCMysqlDriver.h"
#include "NFComm/NFCore/NFCommMap.hpp"

/**
 * @class NFCMysqlDriverManager
 * @brief MySQL驱动管理器类
 * 
 * 负责管理多个MySQL数据库驱动实例，提供连接池管理功能。
 * 支持多个MySQL服务器的连接，自动处理连接检查和重连逻辑。
 * 
 * 主要功能：
 * - 管理多个MySQL服务器连接：支持同时连接多个MySQL数据库
 * - 连接池管理：维护有效和无效连接的分类管理
 * - 自动重连：定期检查连接状态并处理断线重连
 * - 负载分发：根据服务器ID分配相应的数据库连接
 * - 状态监控：实时监控各连接的健康状态
 */
class NFCMysqlDriverManager
{
public:
	/**
	 * @brief 构造函数
	 * 初始化MySQL驱动管理器，设置默认参数
	 */
	NFCMysqlDriverManager();

	/**
	 * @brief 析构函数
	 * 释放所有MySQL驱动实例，清理资源
	 */
	virtual ~NFCMysqlDriverManager();

	/**
	 * @brief 添加MySQL服务器连接
	 * @param serverID 服务器唯一标识符
	 * @param strIP MySQL服务器IP地址
	 * @param nPort MySQL服务器端口号
	 * @param strDBName 数据库名称
	 * @param strDBUser 数据库用户名
	 * @param strDBPwd 数据库密码
	 * @param nRconnectTime 重连时间间隔（秒），默认为10秒
	 * @param nRconneCount 重连次数，默认为-1（无限重连）
	 * @return 返回0表示添加成功，非0表示失败
	 * 
	 * 创建新的MySQL驱动实例并建立数据库连接，
	 * 如果连接失败会根据重连参数进行自动重试
	 */
	int AddMysqlServer(const std::string& serverID, const std::string& strIP, int nPort, std::string strDBName,
	                    std::string strDBUser, std::string strDBPwd, int nRconnectTime/* = 10*/,
	                    int nRconneCount/* = -1*/);

	/**
	 * @brief 获取指定的MySQL驱动
	 * @param serverID 服务器唯一标识符
	 * @return 返回对应的MySQL驱动指针，如果不存在或连接无效则返回nullptr
	 * 
	 * 根据服务器ID获取相应的MySQL驱动实例，
	 * 只返回连接状态正常的驱动，确保数据库操作的可靠性
	 */
    NFCMysqlDriver* GetMysqlDriver(const std::string& serverID);

	/**
	 * @brief 检查所有MySQL连接状态
	 * 
	 * 定期检查所有MySQL连接的状态，处理以下情况：
	 * - 将断开的连接从有效连接池移到无效连接池
	 * - 尝试重新连接无效连接池中的连接
	 * - 将重连成功的连接移回有效连接池
	 * 
	 * 该方法应该定期调用以维护连接池的健康状态
	 */
	void CheckMysql();

    /**
     * @brief 关闭指定的MySQL连接
     * @param serverID 服务器唯一标识符
     * @return 返回0表示关闭成功，非0表示失败
     * 
     * 主动关闭指定服务器的MySQL连接，释放相关资源
     */
    int CloseMysql(const std::string& serverID);

protected:
	/**
	 * @brief 有效的MySQL驱动映射表
	 * 存储连接状态正常的MySQL驱动实例
	 */
	NFCommMap<std::string, NFCMysqlDriver> mvMysql;
	
	/**
	 * @brief 无效的MySQL驱动映射表
	 * 存储连接断开或异常的MySQL驱动实例，用于重连尝试
	 */
	NFCommMap<std::string, NFCMysqlDriver> mvInvalidMsyql;
	
	/**
	 * @brief 最后一次检查时间
	 * 记录上次执行连接状态检查的时间戳，用于控制检查频率
	 */
	uint64_t mnLastCheckTime;
};
