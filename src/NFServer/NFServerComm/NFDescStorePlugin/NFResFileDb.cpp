// -------------------------------------------------------------------------
//    @FileName         :    NFResFileDb.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFResFileDb.cpp
//    @Desc             :    NFShmXFrame文件资源数据库实现
//                          实现基于文件系统的资源数据库功能
//                          支持配置表数据的文件存储和访问操作
//
// -------------------------------------------------------------------------

#include "NFResFileDb.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include <fstream>
#include "NFComm/NFCore/NFCommon.h"

/**
 * @brief 文件资源表构造函数
 * 
 * 初始化文件资源表对象，设置表名和数据库指针
 * 
 * @param p 插件管理器指针
 * @param pFileResDB 文件资源数据库指针
 * @param name 表名
 */
NFFileResTable::NFFileResTable(NFIPluginManager* p, NFFileResDB* pFileResDB, const std::string& name):NFResTable(p)
{
    m_name = name;
	m_pFileResDB = pFileResDB;
}

/**
 * @brief 查找所有记录
 * 
 * 从二进制文件中读取所有记录数据并解析到Protobuf消息中
 * 
 * @param gamePath 游戏路径（未使用）
 * @param pMessage 输出消息数据
 * @return 操作结果状态码，0表示成功
 */
int NFFileResTable::FindAllRecord(const std::string &gamePath, google::protobuf::Message *pMessage)
{
    CHECK_EXPR(pMessage, -1, "pMessage == NULL");

    std::string szFileName = m_pFileResDB->GetPath() + "/" + m_name + ".bin";
    std::fstream input(szFileName.c_str(), std::ios::in | std::ios::binary);
    bool ok = pMessage->ParseFromIstream(&input);
    input.close();

    CHECK_EXPR(ok, -1, "parse error:{} {}", pMessage->InitializationErrorString(), pMessage->DebugString());

    NFLogTrace(NF_LOG_DEFAULT, 0, "parse file:{} Success", szFileName);

	return 0;
}

/**
 * @brief 查找单条记录
 * 
 * 从文件中查找单条匹配的记录（当前实现为空）
 * 
 * @param serverId 服务器ID
 * @param pMessage 输出消息数据
 * @return 操作结果状态码，0表示成功
 */
int NFFileResTable::FindOneRecord(const std::string &serverId, google::protobuf::Message *pMessage)
{
   return 0;
}

/**
 * @brief 保存单条记录
 * 
 * 将单条记录保存到文件中（当前实现为空）
 * 
 * @param serverId 服务器ID
 * @param pMessage 要保存的消息数据
 * @return 操作结果状态码，0表示成功
 */
int NFFileResTable::SaveOneRecord(const std::string &serverId, const google::protobuf::Message *pMessage) {
    return 0;
}

/**
 * @brief 插入单条记录
 * 
 * 向文件中插入单条新记录（当前实现为空）
 * 
 * @param serverId 服务器ID
 * @param pMessage 要插入的消息数据
 * @return 操作结果状态码，0表示成功
 */
int NFFileResTable::InsertOneRecord(const std::string &serverId, const google::protobuf::Message *pMessage)
{
    return 0;
}

/**
 * @brief 删除单条记录
 * 
 * 从文件中删除单条匹配的记录（当前实现为空）
 * 
 * @param serverId 服务器ID
 * @param pMessage 要删除的消息数据
 * @return 操作结果状态码，0表示成功
 */
int NFFileResTable::DeleteOneRecord(const std::string &serverId, const google::protobuf::Message *pMessage)
{
    return 0;
}

/**
 * @brief 文件资源数据库构造函数
 * 
 * 初始化文件资源数据库对象，设置资源文件路径
 * 
 * @param p 插件管理器指针
 * @param szResFilePath 资源文件路径
 */
NFFileResDB::NFFileResDB(NFIPluginManager* p, const std::string &szResFilePath):NFResDb(p)
{
    m_szResFilePath = szResFilePath;
}

/**
 * @brief 获取表对象
 * 
 * 根据表名获取对应的表对象，如果不存在则创建新的表对象
 * 
 * @param name 表名
 * @return 表对象指针
 */
NFResTable *NFFileResDB::GetTable(const std::string &name) {
    auto iter = m_tablesMap.find(name);
    if (iter != m_tablesMap.end()) {
        return iter->second;
    }

    NFFileResTable *pTable = new NFFileResTable(m_pObjPluginManager, this, name);
    m_tablesMap.emplace(name, pTable);
    return pTable;
}