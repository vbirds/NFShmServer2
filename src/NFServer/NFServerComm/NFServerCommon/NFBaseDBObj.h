// -------------------------------------------------------------------------
//    @FileName         :    NFBaseDBObj.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFBaseDBObj.h
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFSeqOP.h"

#define MAX_FAIL_RETRY_TIMES 100
#define MAX_SAVED_OBJ_PRE_SEC 100

// 对象初始化失败后的操作
enum eDealWithLoadFailed
{
    EN_DW_LOG_FAIL  = 1, //记录错误
    EN_DW_RETRY     = 2, //反复重试
    EN_DW_SHUTDOWN  = 3, //立即停止服务器
    EN_DW_RETRY_ANY_SHUTDOWN = 4, //重试一定次数后如果不能成功，关闭服务器
};

// db_table_id, db_table_name, save_to_db_dis_sec, failed_operation
#define DECLARE_DB_OBJ_SETTING(db_name, save_dis, failed_op ) \
public :\
    virtual int MakeLoadData(google::protobuf::Message* data); \
    virtual int MakeSaveData(google::protobuf::Message* data); \
    virtual int InitWithDBData(const google::protobuf::Message* pData); \
    virtual int InitWithoutDBData(); \
    uint32_t GetSaveDis() { return save_dis; } \
    eDealWithLoadFailed DealWithFailed() { return failed_op; } \
    virtual google::protobuf::Message* CreateTempProtobufData() { return new proto_ff::db_name; }

class NFBaseDBObj : public NFObjectTemplate<NFBaseDBObj, EOT_BASE_DB_OBJ, NFObject>, public NFSeqOP
{
public:
    NFBaseDBObj();
    virtual ~NFBaseDBObj();

    //非继承函数, 不要加virtual
    int CreateInit();
    //非继承函数, 不要加virtual
    int ResumeInit();
public:
    virtual int MakeLoadData(google::protobuf::Message* data) { return -1; }
    virtual int MakeSaveData(google::protobuf::Message* data) { return -1; }
    virtual int InitWithDBData(const google::protobuf::Message* pData) { return -1; }
    virtual int InitWithoutDBData() { return -1; }
    virtual int CheckWhenAllDataLoaded() { return 0; }

    virtual google::protobuf::Message* CreateTempProtobufData() { return NULL; }
    virtual uint32_t GetSaveDis() { return 900; }
    virtual eDealWithLoadFailed DealWithFailed() { return EN_DW_RETRY; }
public:
    void SetModeKey(uint64_t mod_key) { m_uModKey = mod_key; }
    uint64_t GetModeKey() { return m_uModKey; }
    bool IsDataInited() { return m_bDataInited; }
    void SetLastDBOpTime(uint64_t t) { m_iLastDBOpTime = t; }
    uint64_t GetLastDBOpTime() const { return m_iLastDBOpTime; }
    void SetTransID(int transId) { m_iTransID = transId; }
    int GetTransID() const { return m_iTransID; }
    void SetRetryTimes(int times) { m_iRetryTimes = times; }
    int GetRetryTimes() const { return m_iRetryTimes; }
    void SetNeedInsertDB(bool b) { m_bNeedInsertDB = b; }
    bool GetNeedInsertDB() const { return m_bNeedInsertDB; }
    void SetServerType(NF_SERVER_TYPE type) { m_iServerType = type; }
    NF_SERVER_TYPE GetServerType() const { return m_iServerType; }
protected:
    bool m_bDataInited;
protected:
    int m_iTransID;
	uint64_t m_iLastDBOpTime;
    uint64_t m_uModKey;
    int      m_iRetryTimes;
    bool     m_bNeedInsertDB;
    NF_SERVER_TYPE m_iServerType;
};