// -------------------------------------------------------------------------
//    @FileName         :    NFCStoreServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCStoreServerModule
//    @Desc             :    NFShmXFrame存储服务器模块实现
//                          实现存储服务器模块的核心功能，包括数据库操作和RPC服务
//                          支持数据库查询、插入、修改、删除等操作，提供完整的存储服务
//
// -------------------------------------------------------------------------

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFCStoreServerModule.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFIAsyDBModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFProtobufCommon.h"
#include "NFComm/NFPluginModule/NFIMysqlModule.h"
#include "NFCommPlugin/NFKernelPlugin/NFCoroutine.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"
#include "NFComm/NFPluginModule/NFINosqlModule.h"
#include "NFComm/NFPluginModule/NFIAsyMysqlModule.h"

#define STORE_SERVER_TIMER_CLOSE_MYSQL 200

/**
 * @brief 构造函数
 * 
 * 初始化存储服务器模块，设置基本配置
 * 
 * @param p 插件管理器指针
 */
NFCStoreServerModule::NFCStoreServerModule(NFIPluginManager* p) : NFIStoreServerModule(p)
{
    m_useCache = false;
    SetConnectProxyAgentServer(false);
}

/**
 * @brief 析构函数
 *
 * 清理存储服务器模块资源
 */
NFCStoreServerModule::~NFCStoreServerModule()
{
}

/**
 * @brief 模块唤醒
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 注册RPC服务
 * - 加载Protobuf定义并检查数据库
 * - 连接数据库服务器
 * - 绑定服务器
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCStoreServerModule::Awake()
{
    //////rpc service//////////////////////
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_SELECTOBJ>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleSelectObjRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_SELECT>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleSelectRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_INSERTOBJ>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleInsertObjRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_MODIFYOBJ>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleModifyObjRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_MODIFY>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleModifyRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_UPDATE>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleUpdateRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_UPDATEOBJ>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleUpdateObjRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_EXECUTE>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleExecuteRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_EXECUTE_MORE>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleExecuteMoreRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_DELETE>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleDeleteRpc, true);
    FindModule<NFIMessageModule>()->AddRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_DELETEOBJ>(NF_ST_STORE_SERVER, this, &NFCStoreServerModule::OnHandleDeleteObjRpc, true);
    //////other server///////////////////////////////////
    RegisterServerMessageWithModule(NF_ST_STORE_SERVER, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD);

    int iRet = 0;
    iRet = LoadPbAndCheckDB();
    CHECK_ERR(0, iRet, "LoadPbAndCheckDB Failed");

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    CHECK_NULL_WF(0, pConfig, "GetAppConfig Failed, server type:{}", NF_ST_STORE_SERVER);

    if (pConfig->RedisConfig.RedisIp.empty())
    {
        iRet = FindModule<NFIAsyMysqlModule>()->AddMysqlServer(pConfig->MysqlConfig.MysqlDbName, pConfig->MysqlConfig.MysqlIp, pConfig->MysqlConfig.MysqlPort, pConfig->MysqlConfig.MysqlDbName, pConfig->MysqlConfig.MysqlUser, pConfig->MysqlConfig.MysqlPassword);
    }
    else
    {
        iRet = FindModule<NFIAsyDbModule>()->AddDbServer(pConfig->MysqlConfig.MysqlDbName, pConfig->MysqlConfig.MysqlIp, pConfig->MysqlConfig.MysqlPort, pConfig->MysqlConfig.MysqlDbName, pConfig->MysqlConfig.MysqlUser, pConfig->MysqlConfig.MysqlPassword, pConfig->RedisConfig.RedisIp,
                                                         pConfig->RedisConfig.RedisPort, pConfig->RedisConfig.RedisPass);
        m_useCache = true;
    }

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "store server connect db failed");
        return -1;
    }

    BindServer();

    return 0;
}

/**
 * @brief 定时器回调
 *
 * 处理定时器事件，包括关闭MySQL连接等操作
 *
 * @param nTimerID 定时器ID
 * @return 处理结果状态码
 */
int NFCStoreServerModule::OnTimer(uint32_t nTimerID)
{
    NFIStoreServerModule::OnTimer(nTimerID);
    if (nTimerID == STORE_SERVER_TIMER_CLOSE_MYSQL)
    {
        FindModule<NFIMysqlModule>()->CloseMysql(INFORMATION_SCHEMA);
        KillTimer(STORE_SERVER_TIMER_CLOSE_MYSQL);
        for (int i = 0; i < (int)NFProtobufCommon::Instance()->m_pOldPoolVec.size(); i++) { NF_SAFE_DELETE(NFProtobufCommon::Instance()->m_pOldPoolVec[i]); }
        NFProtobufCommon::Instance()->m_pOldPoolVec.clear();
    }

    return 0;
}

/**
 * @brief 数据库表列创建信息结构体
 *
 * 用于存储数据库表创建时的相关信息
 */
struct DBTableColCreateInfo
{
    bool bExistTable = false; ///< 表是否存在
    std::map<std::string, DBTableColInfo> primaryKey; ///< 主键信息
    std::multimap<uint32_t, std::string> needCreateColumn; ///< 需要创建的列信息
};

/**
 * @brief 动态加载protobuf信息，并检查数据库，可能的话建立数据库，表格，列
 *
 * 动态加载Protobuf定义文件，检查并创建数据库结构：
 * - 加载Protobuf定义文件
 * - 连接MySQL服务器
 * - 检查数据库是否存在，不存在则创建
 * - 检查表结构，自动创建缺失的表和列
 *
 * @return 加载结果状态码，0表示成功
 */
int NFCStoreServerModule::LoadPbAndCheckDB()
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", NF_ST_STORE_SERVER);

    int iRet = NFProtobufCommon::Instance()->LoadProtoDsFile(m_pObjPluginManager->GetConfigPath() + "/" + m_pObjPluginManager->GetGame() + "/" + pConfig->LoadProtoDs);
    if (iRet == 0) { NFLogInfo(NF_LOG_DEFAULT, 0, "Reload proto ds success:{}", pConfig->LoadProtoDs); }
    else
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "Reload proto ds fail:{}", pConfig->LoadProtoDs);
        return -1;
    }

    KillTimer(STORE_SERVER_TIMER_CLOSE_MYSQL);

    iRet = FindModule<NFIMysqlModule>()->AddMysqlServer(INFORMATION_SCHEMA, pConfig->MysqlConfig.MysqlIp, pConfig->MysqlConfig.MysqlPort, INFORMATION_SCHEMA, pConfig->MysqlConfig.MysqlUser, pConfig->MysqlConfig.MysqlPassword);
    if (iRet != 0)
    {
        NFLogInfo(NF_LOG_DEFAULT, -1, "store server connect mysql failed");
        return -1;
    }

    bool bExistDB;
    iRet = FindModule<NFIMysqlModule>()->ExistsDB(INFORMATION_SCHEMA, pConfig->MysqlConfig.MysqlDbName, bExistDB);
    if (iRet != 0)
    {
        NFLogInfo(NF_LOG_DEFAULT, -1, "store server ExistsDB failed");
        return -1;
    }

    if (!bExistDB)
    {
        iRet = FindModule<NFIMysqlModule>()->CreateDB(INFORMATION_SCHEMA, pConfig->MysqlConfig.MysqlDbName);
        if (iRet != 0)
        {
            NFLogInfo(NF_LOG_DEFAULT, -1, "store server CreateDB failed");
            return -1;
        }
    }

    std::map<std::string, DBTableColCreateInfo> tbCreateInfo;
    for (int i = 0; i < (int)pConfig->MysqlConfig.TBConfList.size(); i++)
    {
        struct pbTableConfig& tableConfig = pConfig->MysqlConfig.TBConfList[i];
        if (tableConfig.TableName.empty())
            continue;

        DBTableColCreateInfo& createInfo = tbCreateInfo[tableConfig.TableName];

        iRet = FindModule<NFIMysqlModule>()->QueryTableInfo(INFORMATION_SCHEMA, pConfig->MysqlConfig.MysqlDbName, tableConfig.TableName, createInfo.bExistTable, createInfo.primaryKey, createInfo.needCreateColumn);
        if (iRet != 0)
        {
            NFLogInfo(NF_LOG_DEFAULT, -1, "store server CreateDB failed");
            return -1;
        }
    }

    iRet = FindModule<NFIMysqlModule>()->SelectDB(INFORMATION_SCHEMA, pConfig->MysqlConfig.MysqlDbName);
    if (iRet != 0)
    {
        NFLogInfo(NF_LOG_DEFAULT, -1, "store server SelectDB failed");
        return -1;
    }

    for (auto iter = tbCreateInfo.begin(); iter != tbCreateInfo.end(); iter++)
    {
        std::string tableName = iter->first;
        DBTableColCreateInfo& createInfo = iter->second;
        if (!createInfo.bExistTable)
        {
            iRet = FindModule<NFIMysqlModule>()->CreateTable(INFORMATION_SCHEMA, tableName, createInfo.primaryKey, createInfo.needCreateColumn);
            if (iRet != 0)
            {
                NFLogInfo(NF_LOG_DEFAULT, -1, "store server CreateTable failed");
                return -1;
            }
        }
        else
        {
            iRet = FindModule<NFIMysqlModule>()->AddTableRow(INFORMATION_SCHEMA, tableName, createInfo.needCreateColumn);
            if (iRet != 0)
            {
                NFLogInfo(NF_LOG_DEFAULT, -1, "store server CreateTable failed");
                return -1;
            }
        }
    }

    iRet = FindModule<NFIMysqlModule>()->SelectDB(INFORMATION_SCHEMA, INFORMATION_SCHEMA);
    if (iRet != 0)
    {
        NFLogInfo(NF_LOG_DEFAULT, -1, "store server SelectDB failed");
        return -1;
    }

    SetTimer(STORE_SERVER_TIMER_CLOSE_MYSQL, 60000, 0);
    return 0;
}

/**
 * @brief 模块初始化
 *
 * 初始化存储服务器模块，连接主服务器
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCStoreServerModule::Init()
{
    ConnectMasterServer();
    return 0;
}

/**
 * @brief 模块定时更新
 *
 * 每帧调用，处理模块的定时任务
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::Tick() { return 0; }

/**
 * @brief 动态插件处理
 *
 * 处理动态插件的加载和卸载，关闭所有连接
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_STORE_SERVER);
    return 0;
}

/**
 * @brief 重新加载配置
 *
 * 当配置文件发生变化时，重新加载Protobuf定义并检查数据库
 *
 * @return 重载结果状态码，0表示成功
 */
int NFCStoreServerModule::OnReloadConfig() { return LoadPbAndCheckDB(); }

/**
 * @brief 处理来自服务器的信息
 *
 * 处理来自其他服务器的消息请求，根据消息ID分发到相应的处理函数
 *
 * @param unLinkId 连接ID
 * @param packet 数据包
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    int retCode = 0;
    switch (packet.nMsgId)
    {
        case NFrame::NF_SERVER_TO_STORE_SERVER_DB_CMD:
            retCode = OnHandleStoreReq(unLinkId, packet);
            break;
        default: NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) not handle", packet.ToString());
            break;
    }

    if (retCode != 0) { NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) handle exist error", packet.ToString()); }
    return 0;
}

/**
 * @brief 处理数据库请求
 *
 * 处理来自客户端的数据库操作请求，包括查询、插入、修改、删除等操作：
 * - 解析请求消息
 * - 根据操作类型分发到相应的处理函数
 * - 处理分表逻辑
 * - 支持缓存机制
 *
 * @param unLinkId 连接ID
 * @param packet 数据包
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleStoreReq(uint64_t unLinkId, NFDataPackage& packet)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    NFrame::Proto_FramePkg xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);
    uint64_t sendLinkId = GetUnLinkId(NF_IS_NONE, NF_ST_STORE_SERVER, pConfig->BusId, 0);
    uint64_t destLinkId = GetUnLinkId(NF_IS_NONE, 0, packet.nSrcId, 0);

    NFrame::Proto_FramePkg retMsg;
    retMsg.set_msg_id(xMsg.msg_id());
    *retMsg.mutable_disp_info() = xMsg.disp_info();
    *retMsg.mutable_store_info() = xMsg.store_info();

    switch (xMsg.msg_id())
    {
        case NFrame::NF_STORESVR_C2S_SELECT:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_SELECT);

            NFrame::storesvr_sel select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.cond().mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->SelectByCond(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_sel_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_BUSY); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_SELECTOBJ:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_SELECTOBJ);

            NFrame::storesvr_selobj select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->SelectObj(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_selobj_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0)
                {
                    if (iRet == NFrame::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY && select_res.opres().errmsg().empty()) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY); }
                    else { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_UNKNOWN); }
                }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_INSERTOBJ:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_INSERTOBJ);

            NFrame::storesvr_insertobj select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->InsertObj(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_insertobj_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_INSERTFAILED); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_DELETE:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_DELETE);

            NFrame::storesvr_del select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.cond().mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->DeleteByCond(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_del_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_UNKNOWN); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_DELETEOBJ:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_DELETEOBJ);

            NFrame::storesvr_delobj select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->DeleteObj(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_delobj_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_DELETEFAILED); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_MODIFY:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_MODIFY);

            NFrame::storesvr_mod select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.cond().mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->ModifyByCond(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_mod_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEFAILED); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_MODIFYOBJ:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_MODIFYOBJ);

            NFrame::storesvr_modobj select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->ModifyObj(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_modobj_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEFAILED); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_UPDATE:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_UPDATE);

            NFrame::storesvr_update select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.cond().mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }
                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->UpdateByCond(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_update_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEINSERTFAILED); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_UPDATEOBJ:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_UPDATEOBJ);

            NFrame::storesvr_updateobj select;
            select.ParsePartialFromString(xMsg.msg_data());

            bool cache = false;
            auto iter = pConfig->mTBConfMap.find(select.baseinfo().tbname());
            if (iter != pConfig->mTBConfMap.end())
            {
                uint32_t count = iter->second.TableCount;
                if (count > 1)
                {
                    uint32_t index = select.mod_key() % count;
                    std::string newTableName = select.baseinfo().tbname() + "_" + NFCommon::tostr(index);
                    select.mutable_baseinfo()->set_tbname(newTableName);
                }

                cache = iter->second.Cache;
            }

            pNFIAsySqlModule->UpdateObj(select.baseinfo().dbname(), select, cache, [=](int iRet, NFrame::storesvr_updateobj_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEINSERTFAILED); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        case NFrame::NF_STORESVR_C2S_EXECUTE:
        {
            retMsg.set_msg_id(NFrame::NF_STORESVR_S2C_EXECUTE);

            NFrame::storesvr_execute select;
            select.ParsePartialFromString(xMsg.msg_data());

            pNFIAsySqlModule->Execute(select.baseinfo().dbname(), select, [=](int iRet, NFrame::storesvr_execute_res& select_res) mutable
            {
                select_res.mutable_opres()->set_err_code(iRet);
                if (iRet != 0) { retMsg.mutable_disp_info()->set_err_code(NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEFAILED); }
                else { retMsg.set_msg_data(select_res.SerializePartialAsString()); }

                NFLogTrace(NF_LOG_DEFAULT, 0, "ret msg:{}", retMsg.Utf8DebugString());
                if (retMsg.store_info().id() > 0)
                    FindModule<NFIMessageModule>()->Send(unLinkId, NF_MODULE_FRAME, NFrame::NF_STORE_SERVER_TO_SERVER_DB_CMD, retMsg, 0, 0, sendLinkId, destLinkId);
            });
        }
        break;
        default:
        {
        }
        break;
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

/**
 * @brief 处理对象查询RPC请求
 *
 * 处理对象查询RPC请求，支持分表和缓存：
 * - 解析查询请求
 * - 处理分表逻辑
 * - 执行数据库查询
 * - 返回查询结果
 *
 * @param request 查询请求数据
 * @param respone 查询响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleSelectObjRpc(NFrame::storesvr_selobj& request, NFrame::storesvr_selobj_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }

        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->SelectObj(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_selobj_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "SelectObj, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            if (iRet == NFrame::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY && select_res.opres().errmsg().empty()) { iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY; }
            else
            {
                iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_SELECTFAILED;
                NFLogError(NF_LOG_DEFAULT, 0, "SelectObj Failed, iRet:{}", iRet);
            }
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "SelectObj Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->SelectObj Failed, iRet:{}", iRet);
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理查询RPC请求
 *
 * 处理查询RPC请求，支持分表、缓存和回调：
 * - 解析查询请求
 * - 处理分表逻辑
 * - 执行数据库查询
 * - 支持批量查询和回调处理
 *
 * @param request 查询请求数据
 * @param respone 查询响应数据
 * @param cb 回调函数
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleSelectRpc(NFrame::storesvr_sel& request, NFrame::storesvr_sel_res& respone, const std::function<void()>& cb)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.cond().mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }
        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();

    int iRet = pNFIAsySqlModule->SelectByCond(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_sel_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "SelectByCond, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_BUSY;
            NFLogError(NF_LOG_DEFAULT, 0, "SelectByCond Failed, iRet:{}", iRet);
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "SelectByCond Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->SelectByCond Failed, iRet:{}", iRet);
        return iRet;
    }

    do
    {
        iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
        if (iRet != 0) { break; }

        if (respone.is_lastbatch()) { break; }
        else { if (cb) { cb(); } }
    }
    while (true);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理对象插入RPC请求
 *
 * 处理对象插入RPC请求，支持分表和缓存：
 * - 解析插入请求
 * - 处理分表逻辑
 * - 执行数据库插入操作
 * - 返回插入结果
 *
 * @param request 插入请求数据
 * @param respone 插入响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleInsertObjRpc(NFrame::storesvr_insertobj& request, NFrame::storesvr_insertobj_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }

        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->InsertObj(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_insertobj_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "SaveObj, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "SelectObj Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_INSERTFAILED;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "SelectObj Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->SelectObj Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理对象修改RPC请求
 *
 * 处理对象修改RPC请求，支持分表和缓存
 *
 * @param request 修改请求数据
 * @param respone 修改响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleModifyObjRpc(NFrame::storesvr_modobj& request, NFrame::storesvr_modobj_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }

        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->ModifyObj(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_modobj_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "ModifyObj, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "ModifyObj Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEFAILED;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "ModifyObj Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->ModifyObj Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理修改RPC请求
 *
 * 处理修改RPC请求，支持分表和缓存
 *
 * @param request 修改请求数据
 * @param respone 修改响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleModifyRpc(NFrame::storesvr_mod& request, NFrame::storesvr_mod_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.cond().mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }
        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->ModifyByCond(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_mod_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "ModifyByCond, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "ModifyByCond Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEFAILED;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "ModifyByCond Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->ModifyByCond Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理更新RPC请求
 *
 * 处理更新RPC请求，支持分表和缓存：
 * - 解析更新请求
 * - 处理分表逻辑
 * - 执行数据库更新操作
 * - 返回更新结果
 *
 * @param request 更新请求数据
 * @param respone 更新响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleUpdateRpc(NFrame::storesvr_update& request, NFrame::storesvr_update_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.cond().mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }
        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->UpdateByCond(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_update_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "UpdateByCond, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "UpdateByCond Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEINSERTFAILED;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "UpdateByCond Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->UpdateByCond Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理对象更新RPC请求
 *
 * 处理对象更新RPC请求，支持分表和缓存：
 * - 解析对象更新请求
 * - 处理分表逻辑
 * - 执行数据库对象更新操作
 * - 返回更新结果
 *
 * @param request 对象更新请求数据
 * @param respone 对象更新响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleUpdateObjRpc(NFrame::storesvr_updateobj& request, NFrame::storesvr_updateobj_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }

        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->UpdateObj(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_updateobj_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "UpdateObj, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "UpdateObj Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEINSERTFAILED;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "UpdateObj Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->UpdateObj Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

int NFCStoreServerModule::OnHandleExecuteRpc(NFrame::storesvr_execute& request, NFrame::storesvr_execute_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->Execute(request.baseinfo().dbname(), request, [this, coId, &respone](int iRet, NFrame::storesvr_execute_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Execute, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "Execute Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_UNKNOWN;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "Execute Success select_res:{}", select_res.Utf8DebugString()); }

        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->DeleteByCond Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理批量执行RPC请求
 *
 * 处理批量执行RPC请求，支持回调处理
 *
 * @param request 批量执行请求数据
 * @param respone 批量执行响应数据
 * @param cb 回调函数
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleExecuteMoreRpc(NFrame::storesvr_execute_more& request, NFrame::storesvr_execute_more_res& respone, const std::function<void()>& cb)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();

    int iRet = pNFIAsySqlModule->ExecuteMore(request.baseinfo().dbname(), request, [this, coId, &respone](int iRet, NFrame::storesvr_execute_more_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "ExecuteMore, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_BUSY;
            NFLogError(NF_LOG_DEFAULT, 0, "ExecuteMore Failed, iRet:{}", iRet);
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "ExecuteMore Success select_res:{}", select_res.Utf8DebugString()); }
        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->ExecuteMore Failed, iRet:{}", iRet);
        return iRet;
    }

    do
    {
        iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
        if (iRet != 0) { break; }

        if (respone.is_lastbatch()) { break; }
        else { if (cb) { cb(); } }
    }
    while (true);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理删除RPC请求
 *
 * 处理删除RPC请求，支持分表和缓存
 *
 * @param request 删除请求数据
 * @param respone 删除响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleDeleteRpc(NFrame::storesvr_del& request, NFrame::storesvr_del_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.cond().mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }
        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->DeleteByCond(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_del_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "DeleteByCond, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "DeleteByCond Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_UPDATEINSERTFAILED;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "DeleteByCond Success select_res:{}", select_res.Utf8DebugString()); }
        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->DeleteByCond Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

/**
 * @brief 处理对象删除RPC请求
 *
 * 处理对象删除RPC请求，支持分表和缓存
 *
 * @param request 对象删除请求数据
 * @param respone 对象删除响应数据
 * @return 处理结果状态码，0表示成功
 */
int NFCStoreServerModule::OnHandleDeleteObjRpc(NFrame::storesvr_delobj& request, NFrame::storesvr_delobj_res& respone)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    NF_ASSERT(FindModule<NFICoroutineModule>()->IsInCoroutine());

    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_STORE_SERVER);
    NF_ASSERT(pConfig);

    NFIAsySqlModule* pNFIAsySqlModule = NULL;
    if (m_useCache) { pNFIAsySqlModule = FindModule<NFIAsyDbModule>(); }
    else { pNFIAsySqlModule = FindModule<NFIAsyMysqlModule>(); }
    NF_ASSERT(pNFIAsySqlModule);

    bool cache = false;
    auto iter = pConfig->mTBConfMap.find(request.baseinfo().tbname());
    if (iter != pConfig->mTBConfMap.end())
    {
        uint32_t count = iter->second.TableCount;
        if (count > 1)
        {
            uint32_t index = request.mod_key() % count;
            std::string newTableName = request.baseinfo().tbname() + "_" + NFCommon::tostr(index);
            request.mutable_baseinfo()->set_tbname(newTableName);
        }

        cache = iter->second.Cache;
    }

    int64_t coId = FindModule<NFICoroutineModule>()->CurrentTaskId();
    int iRet = pNFIAsySqlModule->DeleteObj(request.baseinfo().dbname(), request, cache, [this, coId, &respone](int iRet, NFrame::storesvr_delobj_res& select_res) mutable
    {
        if (!FindModule<NFICoroutineModule>()->IsYielding(coId))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "DeleteObj, But Coroutine Status Error..........Not Yielding");
            return;
        }

        if (iRet != 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "DeleteObj Failed, iRet:{}", GetErrorStr(iRet));
            iRet = NFrame::ERR_CODE_STORESVR_ERRCODE_DELETEFAILED;
            select_res.mutable_opres()->set_err_code(iRet);
        }
        else { NFLogTrace(NF_LOG_DEFAULT, 0, "DeleteObj Success select_res:{}", select_res.Utf8DebugString()); }
        respone.CopyFrom(select_res);

        FindModule<NFICoroutineModule>()->Resume(coId, 0);
    });

    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "FindModule<NFIAsyMysqlModule>()->DeleteObj Failed, iRet:{}", GetErrorStr(iRet));
        return iRet;
    }

    iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS / 2);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}
