// -------------------------------------------------------------------------
//    @FileName         :    NFResFileDb.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFResFileDb.h
//    @Desc             :    NFShmXFrame文件资源数据库接口定义
//                          提供基于文件系统的资源数据库功能
//                          支持配置表数据的文件存储和访问
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFResDb.h"
#include <unordered_map>

/**
 * @brief 最大读取缓冲区大小
 * 
 * 定义文件读取时的最大缓冲区大小，当前为50MB
 */
const int C_READBUFFSIZE_MAX = 1024 * 1024 * 50;

class NFFileResDB;

/**
 * @brief 文件资源表类
 * 
 * NFFileResTable是NFResTable的具体实现，
 * 提供基于文件系统的资源表操作功能。
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
class NFFileResTable : public NFResTable
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化文件资源表对象
	 * 
	 * @param p 插件管理器指针
	 * @param pFileResDB 文件资源数据库指针
	 * @param name 表名
	 */
	NFFileResTable(NFIPluginManager* p, NFFileResDB* pFileResDB, const std::string& name);

	/**
	 * @brief 获取表名
	 *
	 * @return 表名字符串
	 */
	const std::string& GetName() { return m_name; }

	/**
	 * @brief 查找所有记录
	 *
	 * 从文件中查找所有匹配的记录
	 *
	 * @param serverId 服务器ID
	 * @param pMessage 输出消息数据
	 * @return 操作结果状态码
	 */
	virtual int FindAllRecord(const std::string& serverId, google::protobuf::Message* pMessage) override;

	/**
	 * @brief 保存单条记录
	 *
	 * 将单条记录保存到文件中
	 *
	 * @param serverId 服务器ID
	 * @param pMessage 要保存的消息数据
	 * @return 操作结果状态码
	 */
	virtual int SaveOneRecord(const std::string& serverId, const google::protobuf::Message* pMessage) override;

	/**
	 * @brief 查找单条记录
	 *
	 * 从文件中查找单条匹配的记录
	 *
	 * @param serverId 服务器ID
	 * @param pMessage 输出消息数据
	 * @return 操作结果状态码
	 */
	virtual int FindOneRecord(const std::string& serverId, google::protobuf::Message* pMessage) override;

	/**
	 * @brief 插入单条记录
	 *
	 * 向文件中插入单条新记录
	 *
	 * @param serverId 服务器ID
	 * @param pMessage 要插入的消息数据
	 * @return 操作结果状态码
	 */
	virtual int InsertOneRecord(const std::string& serverId, const google::protobuf::Message* pMessage) override;

	/**
	 * @brief 删除单条记录
	 *
	 * 从文件中删除单条匹配的记录
	 *
	 * @param serverId 服务器ID
	 * @param pMessage 要删除的消息数据
	 * @return 操作结果状态码
	 */
	virtual int DeleteOneRecord(const std::string& serverId, const google::protobuf::Message* pMessage) override;

private:
	std::string m_name; ///< 表名
	NFFileResDB* m_pFileResDB; ///< 文件资源数据库指针
};

/**
 * @brief 文件资源数据库类
 *
 * NFFileResDB是NFResDb的具体实现，
 * 提供基于文件系统的资源数据库功能。
 *
 * 该类管理多个文件资源表，支持：
 * - 表的创建和获取
 * - 文件路径管理
 * - 表对象的生命周期管理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFFileResDB : public NFResDb
{
public:
	/**
	 * @brief 构造函数
	 *
	 * 初始化文件资源数据库对象
	 *
	 * @param p 插件管理器指针
	 * @param szResFilePath 资源文件路径
	 */
	NFFileResDB(NFIPluginManager* p, const std::string& szResFilePath);

	/**
	 * @brief 获取表对象
	 *
	 * 根据表名获取对应的表对象
	 *
	 * @param name 表名
	 * @return 表对象指针，如果不存在则创建新的表对象
	 */
	virtual NFResTable* GetTable(const std::string& name);

	/**
	 * @brief 获取资源文件路径
	 *
	 * @return 资源文件路径字符串
	 */
	const std::string& GetPath() { return m_szResFilePath; }

private:
	std::unordered_map<std::string, NFFileResTable*> m_tablesMap; ///< 表对象映射表
	std::string m_szResFilePath; ///< 资源文件路径
};
