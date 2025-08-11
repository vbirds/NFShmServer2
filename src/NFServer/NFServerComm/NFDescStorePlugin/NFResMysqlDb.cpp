// -------------------------------------------------------------------------
//    @FileName         :    NFResMysqlDb.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFResMysqlDb.cpp
//    @Desc             :    NFShmXFrame MySQL资源数据库实现
//                          实现基于MySQL数据库的资源数据库功能
//                          支持配置表数据的数据库存储和访问操作
//
// -------------------------------------------------------------------------

#include "NFResMysqlDb.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFICoroutineModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIDescStoreModule.h"
#include <fstream>

/**
 * @brief MySQL资源表构造函数
 * 
 * 初始化MySQL资源表对象，设置表名和数据库指针
 * 
 * @param p 插件管理器指针
 * @param pFileResDB MySQL资源数据库指针
 * @param name 表名
 */
NFMysqlResTable::NFMysqlResTable(NFIPluginManager* p, NFResMysqlDB* pFileResDB, const std::string& name):NFResTable(p)
{
    m_name = name;
    m_pMysqlResDB = pFileResDB;
}

/**
 * @brief MySQL资源表析构函数
 * 
 * 清理MySQL资源表对象资源
 */
NFMysqlResTable::~NFMysqlResTable()
{

}

/**
 * @brief 查找所有记录
 * 
 * 通过RPC调用从存储服务器获取所有记录数据
 * 
 * @param serverId 服务器ID
 * @param pMessage 输出消息数据
 * @return 操作结果状态码，0表示成功
 */
int NFMysqlResTable::FindAllRecord(const std::string &serverId, google::protobuf::Message *pMessage)
{
    CHECK_EXPR(pMessage, -1, "pMessage == NULL");

    int iRet = 0;
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
    CHECK_NULL(0, pConfig);
    iRet = FindModule<NFIDescStoreModule>()->GetDescStoreByRpc((NF_SERVER_TYPE)pConfig->ServerType, serverId, m_name, pMessage);
	CHECK_EXPR(iRet == 0, iRet, "GetDescStoreByRpc Failed! iRet:{}", GetErrorStr(iRet));

    return 0;
}

/**
 * @brief 查找单条记录
 * 
 * 从MySQL数据库中查找单条匹配的记录（当前实现为空）
 * 
 * @param serverId 服务器ID
 * @param pMessage 输出消息数据
 * @return 操作结果状态码，0表示成功
 */
int NFMysqlResTable::FindOneRecord(const std::string &serverId, google::protobuf::Message *pMessage)
{
    CHECK_EXPR(pMessage, -1, "pMessage == NULL");
    return 0;
}

/**
 * @brief 插入单条记录
 * 
 * 通过RPC调用向存储服务器插入单条新记录
 * 
 * @param serverId 服务器ID
 * @param pMessage 要插入的消息数据
 * @return 操作结果状态码
 */
int NFMysqlResTable::InsertOneRecord(const std::string &serverId, const google::protobuf::Message *pMessage)
{
    CHECK_EXPR(pMessage, -1, "pMessage == NULL");

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
    CHECK_NULL(0, pConfig);

    std::string tempDBName = serverId;
    if (serverId.empty())
    {
        tempDBName = pConfig->DefaultDBName;
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    return FindModule<NFIServerMessageModule>()->SendTransToStoreServer((NF_SERVER_TYPE)pConfig->ServerType, 0,
                                                                        NFrame::NF_STORESVR_C2S_INSERTOBJ, 0, tempDBName, m_name, *pMessage, 0, 0, std::hash<std::string>()(m_name), pMessage->GetDescriptor()->name());
}

/**
 * @brief 删除单条记录
 * 
 * 通过RPC调用从存储服务器删除单条匹配的记录
 * 
 * @param serverId 服务器ID
 * @param pMessage 要删除的消息数据
 * @return 操作结果状态码
 */
int NFMysqlResTable::DeleteOneRecord(const std::string &serverId, const google::protobuf::Message *pMessage)
{
    CHECK_EXPR(pMessage, -1, "pMessage == NULL");

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
    CHECK_NULL(0, pConfig);

    std::string tempDBName = serverId;
    if (serverId.empty())
    {
        tempDBName = pConfig->DefaultDBName;
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    return FindModule<NFIServerMessageModule>()->SendTransToStoreServer((NF_SERVER_TYPE)pConfig->ServerType, 0,
                                                                        NFrame::NF_STORESVR_C2S_DELETEOBJ, 0, tempDBName, m_name, *pMessage, 0, 0, std::hash<std::string>()(m_name), pMessage->GetDescriptor()->name());
}

/**
 * @brief 保存单条记录
 * 
 * 通过RPC调用向存储服务器保存单条记录
 * 
 * @param serverId 服务器ID
 * @param pMessage 要保存的消息数据
 * @return 操作结果状态码
 */
int NFMysqlResTable::SaveOneRecord(const std::string &serverId, const google::protobuf::Message *pMessage) {
    CHECK_EXPR(pMessage, -1, "pMessage == NULL");

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
    CHECK_NULL(0, pConfig);

    std::string tempDBName = serverId;
    if (serverId.empty())
    {
        tempDBName = pConfig->DefaultDBName;
    }
    CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

    return FindModule<NFIServerMessageModule>()->SendTransToStoreServer((NF_SERVER_TYPE)pConfig->ServerType, 0,
                                                                        NFrame::NF_STORESVR_C2S_UPDATEOBJ, 0, tempDBName, m_name, *pMessage, 0, 0, std::hash<std::string>()(m_name), pMessage->GetDescriptor()->name());
}

/**
 * @brief MySQL资源数据库构造函数
 * 
 * 初始化MySQL资源数据库对象
 * 
 * @param p 插件管理器指针
 */
NFResMysqlDB::NFResMysqlDB(NFIPluginManager* p):NFResDb(p)
{
}

/**
 * @brief MySQL资源数据库析构函数
 * 
 * 清理MySQL资源数据库对象资源，释放所有表对象
 */
NFResMysqlDB::~NFResMysqlDB()
{
    for(auto iter = m_tablesMap.begin(); iter != m_tablesMap.end(); iter++)
    {
        if (iter->second)
        {
            NF_SAFE_DELETE(iter->second);
        }
    }
    m_tablesMap.clear();
}

/**
 * @brief 获取表对象
 * 
 * 根据表名获取对应的表对象，如果不存在则创建新的表对象
 * 
 * @param name 表名
 * @return 表对象指针
 */
NFResTable *NFResMysqlDB::GetTable(const std::string &name) {
    auto iter = m_tablesMap.find(name);
    if (iter != m_tablesMap.end()) {
        return iter->second;
    }

    NFMysqlResTable *pTable = new NFMysqlResTable(m_pObjPluginManager, this, name);
    m_tablesMap.emplace(name, pTable);
    return pTable;
}

