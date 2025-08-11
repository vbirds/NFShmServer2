// -------------------------------------------------------------------------
//    @FileName         :    NFResMysqlDb.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFResMysqlDb.h
//    @Desc             :    NFShmXFrame MySQL资源数据库接口定义
//                          提供基于MySQL数据库的资源数据库功能
//                          支持配置表数据的数据库存储和访问
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFResDb.h"
#include <unordered_map>

class NFResMysqlDB;

/**
 * @brief MySQL资源表类
 * 
 * NFMysqlResTable是NFResTable的具体实现，
 * 提供基于MySQL数据库的资源表操作功能。
 * 
 * 该类支持以下操作：
 * - 查找所有记录
 * - 保存单条记录
 * - 查找单条记录
 * - 插入单条记录
 * - 删除单条记录
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFMysqlResTable : public NFResTable
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化MySQL资源表对象
     * 
     * @param p 插件管理器指针
     * @param pFileResDB MySQL资源数据库指针
     * @param name 表名
     */
    NFMysqlResTable(NFIPluginManager* p, NFResMysqlDB* pFileResDB, const std::string& name);

    /**
     * @brief 析构函数
     * 
     * 清理MySQL资源表对象资源
     */
    virtual ~NFMysqlResTable();

    /**
     * @brief 获取表名
     * 
     * @return 表名字符串
     */
    const std::string& GetName() { return m_name; }

    /**
     * @brief 查找所有记录
     * 
     * 从MySQL数据库中查找所有匹配的记录
     * 
     * @param serverId 服务器ID
     * @param pMessage 输出消息数据
     * @return 操作结果状态码
     */
    virtual int FindAllRecord(const std::string& serverId, google::protobuf::Message* pMessage) override;

    /**
     * @brief 保存单条记录
     * 
     * 将单条记录保存到MySQL数据库中
     * 
     * @param serverId 服务器ID
     * @param pMessage 要保存的消息数据
     * @return 操作结果状态码
     */
    virtual int SaveOneRecord(const std::string& serverId, const google::protobuf::Message* pMessage) override;

    /**
     * @brief 查找单条记录
     * 
     * 从MySQL数据库中查找单条匹配的记录
     * 
     * @param serverId 服务器ID
     * @param pMessage 输出消息数据
     * @return 操作结果状态码
     */
    virtual int FindOneRecord(const std::string& serverId, google::protobuf::Message* pMessage) override;

    /**
     * @brief 插入单条记录
     * 
     * 向MySQL数据库中插入单条新记录
     * 
     * @param serverId 服务器ID
     * @param pMessage 要插入的消息数据
     * @return 操作结果状态码
     */
    virtual int InsertOneRecord(const std::string& serverId, const google::protobuf::Message* pMessage) override;

    /**
     * @brief 删除单条记录
     * 
     * 从MySQL数据库中删除单条匹配的记录
     * 
     * @param serverId 服务器ID
     * @param pMessage 要删除的消息数据
     * @return 操作结果状态码
     */
    virtual int DeleteOneRecord(const std::string& serverId, const google::protobuf::Message* pMessage) override;

private:
    std::string m_name; ///< 表名
    NFResMysqlDB* m_pMysqlResDB; ///< MySQL资源数据库指针
};

/**
 * @brief MySQL资源数据库类
 * 
 * NFResMysqlDB是NFResDb的具体实现，
 * 提供基于MySQL数据库的资源数据库功能。
 * 
 * 该类管理多个MySQL资源表，支持：
 * - 表的创建和获取
 * - 数据库连接管理
 * - 表对象的生命周期管理
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFResMysqlDB : public NFResDb
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化MySQL资源数据库对象
     * 
     * @param p 插件管理器指针
     */
    NFResMysqlDB(NFIPluginManager* p);

    /**
     * @brief 析构函数
     * 
     * 清理MySQL资源数据库对象资源
     */
    virtual ~NFResMysqlDB();

    /**
     * @brief 获取表对象
     * 
     * 根据表名获取对应的表对象
     * 
     * @param name 表名
     * @return 表对象指针，如果不存在则创建新的表对象
     */
    virtual NFResTable* GetTable(const std::string& name);

private:
    std::unordered_map<std::string, NFMysqlResTable*> m_tablesMap; ///< 表对象映射表
};
