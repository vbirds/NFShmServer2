#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFProtobufCommon.h"
#include "NFRedisDriver.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFCore/NFCommon.h"

#define PRIVATE_KEY_EXIST_TIME 86400*2

NFRedisDriver::NFRedisDriver()
{
    mnPort = 0;
    mbBusy = false;
    mbAuthed = false;
    m_pRedisClientSocket = new NFRedisClientSocket();
}

bool NFRedisDriver::Enable()
{
    return m_pRedisClientSocket->IsConnect();
}

bool NFRedisDriver::Authed()
{
    return mbAuthed;
}

bool NFRedisDriver::Busy()
{
    return mbBusy;
}

bool NFRedisDriver::Connect(const std::string &ip, const int port, const std::string &auth)
{
    int64_t nFD = m_pRedisClientSocket->Connect(ip, port);
    if (nFD > 0)
    {
        mstrIP = ip;
        mnPort = port;
        mstrAuthKey = auth;
        while (m_pRedisClientSocket->IsNone())
        {
            Execute();
            NFSLEEP(1000);
        }
        if (!auth.empty())
        {
            if (Enable() && !Authed())
            {
                if (!AUTH(GetAuthKey()))
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "Redis AUTH failed! ip:{} port:{} auth:{}", ip, port, auth);
                    return false;
                }
            }
        }

        return Enable();
    }

    return false;
}

bool NFRedisDriver::SelectDB(int dbnum)
{
    NFRedisCommand cmd(GET_NAME(Select));
    cmd << dbnum;

    // if password error, redis will return REDIS_REPLY_ERROR
    // pReply will be null
    NF_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
    if (pReply == nullptr)
    {
        return false;
    }

    if (pReply->type == REDIS_REPLY_STATUS)
    {
        if (std::string("OK") == std::string(pReply->str, pReply->len) ||
            std::string("ok") == std::string(pReply->str, pReply->len))
        {
            return true;
        }
    }

    return false;
}

bool NFRedisDriver::KeepLive()
{
    return false;
}

bool NFRedisDriver::ReConnect()
{
    this->mbAuthed = false;
    return m_pRedisClientSocket->ReConnect(mstrIP, mnPort);
}

bool NFRedisDriver::IsConnect()
{
    if (m_pRedisClientSocket)
    {
        return m_pRedisClientSocket->IsConnect();
    }

    return false;
}

bool NFRedisDriver::Execute()
{
    m_pRedisClientSocket->Execute();

    return true;
}

int NFRedisDriver::SelectByCond(const NFrame::storesvr_sel& select, std::string& privateKey, std::unordered_set<std::string>& fields, std::unordered_set<std::string>& privateKeySet)
{
    std::string className = select.baseinfo().clname();
    std::string packageName = select.baseinfo().package_name();

    int iRet = GetPrivateKey(packageName, className, privateKey);
    CHECK_ERR(0, iRet, "GetPrivateKey Failed, packageName:{}, className:{}", packageName, className);

    if (select.baseinfo().sel_fields_size() > 0)
    {
        for (int i = 0; i < (int)select.baseinfo().sel_fields_size(); i++)
        {
            fields.insert(select.baseinfo().sel_fields(i));
        }
    }

    for (int i = 0; i < select.cond().private_keys_size(); ++i)
    {
        privateKeySet.insert(select.cond().private_keys(i));
    }

    return 0;
}

int NFRedisDriver::GetPrivateKey(const std::string& packageName, const std::string& className, std::string& privateKey)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    std::string full_name;
    if (packageName.empty())
    {
        full_name = DEFINE_DEFAULT_PROTO_PACKAGE_ADD + className;
    }
    else
    {
        full_name = packageName + "." + className;
    }

    const google::protobuf::Descriptor* pDescriptor = NFProtobufCommon::Instance()->FindDynamicMessageTypeByName(full_name);
    CHECK_EXPR(pDescriptor, -1, "NFProtobufCommon::FindDynamicMessageTypeByName:{} Failed", full_name);

    int iRet = NFProtobufCommon::GetPrivateKeyFromMessage(pDescriptor, privateKey);
    CHECK_EXPR(iRet == 0, -1, "NFProtobufCommon::GetPrivateKeyFromMessage:{} Failed", full_name);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

int NFRedisDriver::SelectByCond(const NFrame::storesvr_sel &select, const std::string &privateKey,
                                const std::unordered_set<std::string> &fields,
                                const std::unordered_set<std::string> &privateKeySet, std::unordered_set<std::string> &leftPrivateKeySet,
                                ::google::protobuf::RepeatedPtrField<NFrame::storesvr_sel_res> &vecSelectRes)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");
    std::string className = select.baseinfo().clname();
    CHECK_EXPR(className.size() > 0, -1, "className empty!");
    std::string packageName = select.baseinfo().package_name();

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} ", NFREDIS_DB1, tableName);
        return 1;
    }

    NFrame::storesvr_sel_res *select_res = NULL;
    if (vecSelectRes.size() == 0)
    {
        select_res = vecSelectRes.Add();
        select_res->mutable_baseinfo()->CopyFrom(select.baseinfo());
        select_res->mutable_opres()->set_mod_key(select.cond().mod_key());
        select_res->set_is_lastbatch(false);
    }
    else
    {
        select_res = vecSelectRes.Mutable(vecSelectRes.size() - 1);
        if ((int) select_res->record_size() >= (int) select.baseinfo().max_records())
        {
            select_res = vecSelectRes.Add();

            select_res->mutable_baseinfo()->CopyFrom(select.baseinfo());
            select_res->mutable_opres()->set_mod_key(select.cond().mod_key());
            select_res->set_is_lastbatch(false);
        }
    }

    std::string errmsg;
    int count = 0;
    for (auto iter = privateKeySet.begin(); iter != privateKeySet.end(); iter++)
    {
        std::string db_key = GetPrivateKeys(tableName, privateKey, *iter);
        std::string value;
        bRet = GetObj(packageName, className, db_key, fields, value);
        if (!bRet)
        {
            leftPrivateKeySet.insert(*iter);
            continue;
        }

        EXPIRE(db_key, PRIVATE_KEY_EXIST_TIME);

        select_res->add_record(value);

        count++;
        select_res->set_row_count(count);

        if ((int) select_res->record_size() >= (int) select.baseinfo().max_records())
        {
            count = 0;
            select_res = vecSelectRes.Add();

            select_res->mutable_baseinfo()->CopyFrom(select.baseinfo());
            select_res->mutable_opres()->set_mod_key(select.cond().mod_key());
            select_res->set_is_lastbatch(false);
        }
    }

    if (leftPrivateKeySet.empty())
    {
        select_res->set_is_lastbatch(true);
        NFLogInfo(NF_LOG_DEFAULT, 0, "SelectByCond Success");
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::DeleteByCond(const NFrame::storesvr_del &select, const std::string &privateKey,
                                const std::unordered_set<std::string> &privateKeySet,
                                NFrame::storesvr_del_res &select_res)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} ", NFREDIS_DB1, tableName);
        return 1;
    }

    for (auto iter = privateKeySet.begin(); iter != privateKeySet.end(); iter++)
    {
        std::string db_key = GetPrivateKeys(tableName, privateKey, *iter);
        DEL(db_key);
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "DeleteByCond Success");

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::GetObjFields(const std::string& packageName, const std::string& className, const std::string& record, std::map<std::string, std::string> &keyMap,
                              std::map<std::string, std::string> &kevValueMap)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    std::string full_name;
    if (packageName.empty())
    {
        full_name = DEFINE_DEFAULT_PROTO_PACKAGE_ADD + className;
    }
    else
    {
        full_name = packageName + "." + className;
    }
    google::protobuf::Message *pMessageObject = NFProtobufCommon::Instance()->CreateDynamicMessageByName(full_name);
    CHECK_EXPR(pMessageObject, -1, "NFProtobufCommon::CreateMessageByName:{} Failed", full_name);
    CHECK_EXPR(pMessageObject->ParsePartialFromString(record), -1, "ParsePartialFromString Failed:{}", full_name);
    NFLogTrace(NF_LOG_DEFAULT, 0, "CreateSql From message:{}", pMessageObject->DebugString());

    NFProtobufCommon::GetMapDBFieldsFromMessage(*pMessageObject, keyMap, kevValueMap);
    delete pMessageObject;
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::UpdateByCond(const NFrame::storesvr_update &select, const std::string& privateKey,
                 const std::unordered_set<std::string>& privateKeySet, std::unordered_set<std::string>& leftPrivateKeySet,
                 NFrame::storesvr_update_res &select_res)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");
    std::string className = select.baseinfo().clname();
    CHECK_EXPR(className.size() > 0, -1, "className empty!");
    std::string packageName = select.baseinfo().package_name();

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} ", NFREDIS_DB1, tableName);
        return 1;
    }

    std::string errmsg;
    for (auto iter = privateKeySet.begin(); iter != privateKeySet.end(); iter++)
    {
        std::string db_key = GetPrivateKeys(tableName, privateKey, *iter);

        if (!EXISTS(db_key))
        {
            leftPrivateKeySet.insert(*iter);
            continue;
        }

        std::map<std::string, std::string> keyMap;
        std::map<std::string, std::string> kevValueMap;
        int iRet = GetObjFields(packageName, className, select.record(), keyMap, kevValueMap);
        if (iRet != 0)
        {
            leftPrivateKeySet.insert(*iter);
            continue;
        }

        std::vector<string_pair> vecFieldValues;
        vecFieldValues.insert(vecFieldValues.end(), kevValueMap.begin(), kevValueMap.end());
        bool bRet = HMSET(db_key, vecFieldValues);
        if (!bRet)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "HMSET:{} Failed! dbName:{} ", db_key, tableName);
            return -1;
        }

        EXPIRE(db_key, PRIVATE_KEY_EXIST_TIME);
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "UpdateByCond Success");

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::ModifyByCond(const NFrame::storesvr_mod &select, const std::string& privateKey,
                 const std::unordered_set<std::string>& privateKeySet, std::unordered_set<std::string>& leftPrivateKeySet,
                 NFrame::storesvr_mod_res &select_res)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");
    std::string className = select.baseinfo().clname();
    CHECK_EXPR(className.size() > 0, -1, "className empty!");
    std::string packageName = select.baseinfo().package_name();

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} ", NFREDIS_DB1, tableName);
        return -1;
    }

    std::map<std::string, std::string> keyMap;
    std::map<std::string, std::string> kevValueMap;
    int iRet = GetObjFields(packageName, className, select.record(), keyMap, kevValueMap);
    if (iRet != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} ", NFREDIS_DB1, tableName);
        return -1;
    }

    std::vector<string_pair> vecFieldValues;
    vecFieldValues.insert(vecFieldValues.end(), kevValueMap.begin(), kevValueMap.end());

    std::string errmsg;
    for (auto iter = privateKeySet.begin(); iter != privateKeySet.end(); iter++)
    {
        std::string db_key = GetPrivateKeys(tableName, privateKey, *iter);

        if (!EXISTS(db_key))
        {
            leftPrivateKeySet.insert(*iter);
            continue;
        }

        bool bRet = HMSET(db_key, vecFieldValues);
        if (!bRet)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "HMSET:{} Failed! dbName:{} ", db_key, tableName);
            return -1;
        }

        EXPIRE(db_key, PRIVATE_KEY_EXIST_TIME);
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "ModifyByCond Success");

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::TransTableRowToMessage(const std::vector<std::string> &vecFields, const std::vector<std::string> &vecValues,
                                          const std::string &packageName, const std::string &className,
                                          google::protobuf::Message **pMessage)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin --");
    CHECK_EXPR(vecFields.size() == vecValues.size(), -1, "TransTableRowToMessage Failed");

    std::string proto_fullname;
    if (packageName.empty())
    {
        proto_fullname = DEFINE_DEFAULT_PROTO_PACKAGE_ADD + className;
    }
    else
    {
        proto_fullname = packageName + "." + className;
    }

    //通过protobuf默认便利new出来一个新的proto_fullname变量
    ::google::protobuf::Message *pMessageObject = NFProtobufCommon::Instance()->CreateDynamicMessageByName(proto_fullname);
    CHECK_EXPR(pMessageObject, -1, "{} New Failed", proto_fullname);

    if (pMessage)
    {
        *pMessage = pMessageObject;
    }

    std::map<std::string, std::string> mapResult;
    for (int i = 0; i < (int) vecFields.size(); i++)
    {
        mapResult.emplace(vecFields[i], vecValues[i]);
    }
    NFProtobufCommon::GetDBMessageFromMapFields(mapResult, pMessageObject);
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

bool NFRedisDriver::GetObj(const std::string& packageName, const std::string& className, const std::string& key, std::string& value)
{
    return GetObj(packageName, className, key, std::unordered_set<std::string>(), value);
}

bool NFRedisDriver::GetObj(const std::string &packageName, const std::string &className, const std::string &key,
                           const std::unordered_set<std::string> &fields, std::string &value)
{
    std::string full_name;
    if (packageName.empty())
    {
        full_name = DEFINE_DEFAULT_PROTO_PACKAGE_ADD + className;
    }
    else
    {
        full_name = packageName + "." + className;
    }

    const google::protobuf::Descriptor *pDesc = NFProtobufCommon::Instance()->FindDynamicMessageTypeByName(full_name);
    CHECK_EXPR(pDesc, false, "NFProtobufCommon::FindDynamicMessageTypeByName:{} Failed", full_name);

    std::vector<std::string> vecFields;
    if (fields.empty())
    {
        NFProtobufCommon::GetDBFieldsFromDesc(pDesc, vecFields);
    }
    else
    {
        vecFields.insert(vecFields.end(), fields.begin(), fields.end());
    }

    std::vector<std::string> vecValues;
    bool bRet = HMGET(key, vecFields, vecValues);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} ", NFREDIS_DB1, className);
        return false;
    }

    if (vecFields.size() > 0 && vecValues.empty())
    {
        return false;
    }

    CHECK_EXPR(vecFields.size() == vecValues.size(), false, "HMGET key:{} error", key);
    google::protobuf::Message *pMessage = NULL;
    int iRet = TransTableRowToMessage(vecFields, vecValues, packageName, className, &pMessage);
    if (iRet == 0 && pMessage != NULL)
    {
        value = pMessage->SerializePartialAsString();
        NFLogTrace(NF_LOG_DEFAULT, 0, "{}", pMessage->Utf8DebugString());
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "TransTableRowToMessage Failed, fields:{} values:{} className:{}", NFCommon::tostr(vecFields),
                   NFCommon::tostr(vecValues), className);
        return false;
    }
    if (pMessage != NULL)
    {
        NF_SAFE_DELETE(pMessage);
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "GetObj:{} Success", key);

    return true;
}

bool NFRedisDriver::SetObj(const std::string &packageName, const std::string &className, const std::string &key, const std::string &value)
{
    std::string full_name;
    if (packageName.empty())
    {
        full_name = DEFINE_DEFAULT_PROTO_PACKAGE_ADD + className;
    }
    else
    {
        full_name = packageName + "." + className;
    }

    google::protobuf::Message *pMessageObject = NFProtobufCommon::Instance()->CreateDynamicMessageByName(full_name);
    CHECK_EXPR(pMessageObject, false, "NFProtobufCommon::CreateMessageByName:{} Failed", full_name);
    CHECK_EXPR(pMessageObject->ParsePartialFromString(value), false, "ParsePartialFromString Failed:{}", full_name);

    std::map<std::string, std::string> resultMap;
    NFProtobufCommon::GetDBMapFieldsFromMessage(*pMessageObject, resultMap);
    delete pMessageObject;

    std::vector<string_pair> values(resultMap.begin(), resultMap.end());
    bool bRet = HMSET(key, values);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} ", NFREDIS_DB1, className);
        return false;
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "SetObj:{} Success", key);

    return true;
}

int NFRedisDriver::SelectObj(const NFrame::storesvr_selobj &select,
                             NFrame::storesvr_selobj_res &select_res)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");
    std::string className = select.baseinfo().clname();
    CHECK_EXPR(className.size() > 0, -1, "className empty!");
    std::string packageName = select.baseinfo().package_name();

    int iRet = 0;
    std::string field;
    std::string fieldKey;
    std::string db_key;
    iRet = GetObjKey(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record(), field, fieldKey, db_key);
    CHECK_EXPR(iRet == 0, -1, "GetPrivateFields Failed:{}", tableName);

    *select_res.mutable_baseinfo() = select.baseinfo();
    select_res.mutable_opres()->set_mod_key(select.mod_key());
    std::string errmsg;

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} modkey:{}", NFREDIS_DB1, select.baseinfo().tbname(), select.mod_key());
        return -1;
    }

    if (!EXISTS(db_key))
    {
        return 1;
    }

    std::string value;
    bRet = GetObj(packageName, className, db_key, value);
    if (!bRet)
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "SelectObj:{} storesvr_selobj Failed", db_key);
        return 1;
    }

    EXPIRE(db_key, PRIVATE_KEY_EXIST_TIME);

    select_res.set_record(value);

    NFLogInfo(NF_LOG_DEFAULT, 0, "SelectObj:{} storesvr_selobj Success", db_key);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::SaveObj(const std::string &packageName, const std::string &tableName, const std::string &clasname, const std::string& privateKey,
                           const std::map<std::string, std::string> &recordsMap)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{}", NFREDIS_DB1, tableName);
        return -1;
    }

    for (auto iter = recordsMap.begin(); iter != recordsMap.end(); iter++)
    {
        std::string errmsg;
        std::string db_key = GetPrivateKeys(tableName, privateKey, iter->first);

        bRet = SetObj(packageName, clasname, db_key, iter->second);
        if (!bRet)
        {
            return -1;
        }

        EXPIRE(db_key, PRIVATE_KEY_EXIST_TIME);
    }

    NFLogInfo(NF_LOG_DEFAULT, 0, "SaveObj Success");

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::SaveObj(const std::string& packageName, const std::string& tableName, const std::string& className, const std::string& record)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    int iRet = 0;
    std::string field;
    std::string fieldKey;
    std::string db_key;
    iRet = GetObjKey(packageName, tableName, className, record, field, fieldKey, db_key);
    if (iRet > 0)
    {
        return iRet;
    }
    else if (iRet < 0)
    {
        CHECK_EXPR(iRet == 0, 1, "GetPrivateFields Failed:{}", tableName);
    }

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} fieldKey:{}", NFREDIS_DB1, tableName, fieldKey);
        return -1;
    }

    bRet = SetObj(packageName, className, db_key, record);
    if (!bRet)
    {
        return -1;
    }

    EXPIRE(db_key, PRIVATE_KEY_EXIST_TIME);
    NFLogInfo(NF_LOG_DEFAULT, 0, "SaveObj tbName:{} field:{} fieldKey:{} Success", tableName, field, fieldKey);

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::SaveObj(const NFrame::storesvr_selobj &select,
                           NFrame::storesvr_selobj_res &select_res)
{
    *select_res.mutable_baseinfo() = select.baseinfo();
    select_res.mutable_opres()->set_mod_key(select.mod_key());

    return SaveObj(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record());
}

bool NFRedisDriver::ExistObj(const std::string& db_key)
{
    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        return false;
    }

    return EXISTS(db_key);
}

int NFRedisDriver::SaveObj(const NFrame::storesvr_modobj &select, NFrame::storesvr_modobj_res& select_res)
{
    *select_res.mutable_baseinfo() = select.baseinfo();
    select_res.mutable_opres()->set_mod_key(select.mod_key());
    return SaveObj(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record());
}

int NFRedisDriver::SaveObj(const NFrame::storesvr_updateobj &select, NFrame::storesvr_updateobj_res& select_res)
{
    *select_res.mutable_baseinfo() = select.baseinfo();
    select_res.mutable_opres()->set_mod_key(select.mod_key());
    return SaveObj(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record());
}

int NFRedisDriver::SaveObj(const NFrame::storesvr_insertobj &select, NFrame::storesvr_insertobj_res& select_res)
{
    *select_res.mutable_baseinfo() = select.baseinfo();
    select_res.mutable_opres()->set_mod_key(select.mod_key());
    return SaveObj(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record());
}

std::string NFRedisDriver::GetPrivateKeys(const std::string &tbname, const std::string &field, const std::string &fieldValue)
{
    return tbname + "_" + field + "_" + fieldValue;
}

int NFRedisDriver::GetPrivateFields(const std::string &packageName, const std::string &className, const std::string &record, std::string &field,
                                    std::string &fieldValue)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string full_name;
    if (packageName.empty())
    {
        full_name = DEFINE_DEFAULT_PROTO_PACKAGE_ADD + className;
    }
    else
    {
        full_name = packageName + "." + className;
    }
    google::protobuf::Message *pMessageObject = NFProtobufCommon::Instance()->CreateDynamicMessageByName(full_name);
    CHECK_EXPR(pMessageObject, -1, "NFProtobufCommon::CreateMessageByName:{} Failed", full_name);
    CHECK_EXPR(pMessageObject->ParsePartialFromString(record), -1, "ParsePartialFromString Failed:{}", full_name);

    int iRet = NFProtobufCommon::GetPrivateFieldsFromMessage(*pMessageObject, field, fieldValue);
    delete pMessageObject;
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

int NFRedisDriver::DeleteObj(const NFrame::storesvr_delobj &select)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");

    int iRet = 0;
    std::string field;
    std::string fieldKey;
    std::string db_key;
    iRet = GetObjKey(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record(), field, fieldKey, db_key);
    CHECK_EXPR(iRet == 0, -1, "GetPrivateFields Failed:{}", tableName);
    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} modkey:{}", NFREDIS_DB1, select.baseinfo().tbname(), select.mod_key());
        return -1;
    }

    bRet = EXISTS(db_key);
    if (bRet)
    {
        bRet = DEL(db_key);
        if (bRet == false)
        {
            return -1;
        }
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::DeleteObj(const NFrame::storesvr_insertobj &select)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");

    int iRet = 0;
    std::string field;
    std::string fieldKey;
    std::string db_key;
    iRet = GetObjKey(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record(), field, fieldKey, db_key);
    CHECK_EXPR(iRet == 0, -1, "GetPrivateFields Failed:{}", tableName);
    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} modkey:{}", NFREDIS_DB1, select.baseinfo().tbname(), select.mod_key());
        return -1;
    }

    bRet = EXISTS(db_key);
    if (bRet)
    {
        bRet = DEL(db_key);
        if (bRet == false)
        {
            return -1;
        }
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::DeleteObj(const NFrame::storesvr_modobj &select)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");
    std::string tableName = select.baseinfo().tbname();
    CHECK_EXPR(tableName.size() > 0, -1, "talbeName empty!");

    int iRet = 0;
    std::string field;
    std::string fieldKey;
    std::string db_key;
    iRet = GetObjKey(select.baseinfo().package_name(), select.baseinfo().tbname(), select.baseinfo().clname(), select.record(), field, fieldKey, db_key);
    CHECK_EXPR(iRet == 0, -1, "GetPrivateFields Failed:{}", tableName);

    bool bRet = SelectDB(NFREDIS_DB1);
    if (!bRet)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "SelectDB:{} Failed! dbName:{} modkey:{}", NFREDIS_DB1, select.baseinfo().tbname(), select.mod_key());
        return -1;
    }

    bRet = EXISTS(db_key);
    if (bRet)
    {
        bRet = DEL(db_key);
        if (bRet == false)
        {
            return -1;
        }
    }

    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return 0;
}

int NFRedisDriver::GetObjKey(const std::string& packageName, const std::string& tableName, const std::string& className, const std::string& record, std::string& privateKey, std::string& privateKeyValue, std::string& db_key)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- begin -- ");

    std::string full_name;
    if (packageName.empty())
    {
        full_name = DEFINE_DEFAULT_PROTO_PACKAGE_ADD + className;
    }
    else
    {
        full_name = packageName + "." + className;
    }
    google::protobuf::Message *pMessageObject = NFProtobufCommon::Instance()->CreateDynamicMessageByName(full_name);
    CHECK_EXPR(pMessageObject, -1, "NFProtobufCommon::CreateMessageByName:{} Failed", full_name);
    CHECK_EXPR(pMessageObject->ParsePartialFromString(record), -1, "ParsePartialFromString Failed:{}", full_name);

    int iRet = NFProtobufCommon::GetPrivateFieldsFromMessage(*pMessageObject, privateKey, privateKeyValue);
    delete pMessageObject;

    if (iRet == 0)
    {
        db_key = GetPrivateKeys(tableName, privateKey, privateKeyValue);
    }
    NFLogTrace(NF_LOG_DEFAULT, 0, "--- end -- ");
    return iRet;
}

NF_SHARE_PTR<redisReply> NFRedisDriver::BuildSendCmd(const NFRedisCommand &cmd)
{
    mbBusy = true;

    if (!IsConnect())
    {
        mbBusy = false;

        ReConnect();
        return nullptr;
    }
    else
    {
        std::string msg = cmd.Serialize();
        if (msg.empty())
        {
            mbBusy = false;
            return nullptr;
        }
        int nRet = m_pRedisClientSocket->Write(msg.data(), msg.length());
        if (nRet != 0)
        {
            mbBusy = false;
            return nullptr;
        }
    }


    return ParseForReply();
}

NF_SHARE_PTR<redisReply> NFRedisDriver::ParseForReply()
{
    struct redisReply *reply = nullptr;
    while (true)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // When the buffer is empty, reply will be null
        int ret = redisReaderGetReply(m_pRedisClientSocket->GetRedisReader(), (void **) &reply);
        if (ret == REDIS_OK && reply != nullptr)
        {
            break;
        }

        Execute();

        if (!IsConnect())
        {
            ReConnect();
            break;
        }
    }

    mbBusy = false;

    if (reply == nullptr)
    {
        return nullptr;
    }

    if (REDIS_REPLY_ERROR == reply->type)
    {
        // write log
        freeReplyObject(reply);
        return nullptr;
    }

    return NF_SHARE_PTR<redisReply>(reply, [](redisReply *r) { if (r) freeReplyObject(r); });
}

bool NFRedisDriver::AUTH(const std::string &auth)
{
    NFRedisCommand cmd(GET_NAME(AUTH));
    cmd << auth;

    // if password error, redis will return REDIS_REPLY_ERROR
    // pReply will be null
    NF_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
    if (pReply == nullptr)
    {
        return false;
    }

    if (pReply->type == REDIS_REPLY_STATUS)
    {
        if (std::string("OK") == std::string(pReply->str, pReply->len) ||
            std::string("ok") == std::string(pReply->str, pReply->len))
        {
            mbAuthed = true;
            return true;
        }
    }

    return false;
}
