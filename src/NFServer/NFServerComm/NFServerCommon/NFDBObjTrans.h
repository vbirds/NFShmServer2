// -------------------------------------------------------------------------
//    @FileName         :    NFDBObjTrans.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDBObjTrans.h
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFTransBase.h"

class NFDBObjTrans : public NFObjectTemplate<NFDBObjTrans, EOT_TRANS_DB_OBJ, NFTransBase>
{
public:
    NFDBObjTrans();
    virtual ~NFDBObjTrans();

    //非继承函数, 不要加virtual
    int CreateInit();
    //非继承函数, 不要加virtual
    int ResumeInit();
public:
    int Init(NF_SERVER_TYPE eType, int iObjID, uint32_t iSeqOP);
    int Insert(uint64_t iModKey, google::protobuf::Message* data);
    int Save(uint64_t iModKey, google::protobuf::Message* data);
    int Load(uint64_t iModKey, google::protobuf::Message* data);

    virtual int OnTimeOut();
    virtual int HandleTransFinished(int iRunLogicRetCode);

    int GetLinkedObjID() { return m_iLinkedObjID; }
    uint32_t GetObjSeqOP() { return m_iObjSeqOP; }
private:
    int m_iLinkedObjID;
    uint32_t m_iObjSeqOP;
    int m_iDBOP;
    NF_SERVER_TYPE m_iServerType;
};