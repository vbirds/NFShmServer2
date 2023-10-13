// -------------------------------------------------------------------------
//    @FileName         :    NFWorldPlayerIDHashTable.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFWorldPlayerIDHashTable.h
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFShmCore/NFShmObj.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFComm/NFShmCore/NFISharedMemModule.h"
#include "NFComm/NFShmStl/NFShmHashMap.h"
#include "DBProxy.pb.h"
#include <vector>

class NFWorldAccount;
class NFWorldSession;
class NFWorldAccountMgr : public NFShmObj {
public:
    NFWorldAccountMgr();
    virtual ~NFWorldAccountMgr();

    int CreateInit();
    int ResumeInit();
public:
    virtual int OnTimer(int timeId, int callcount);
    int AccountTick();
public:

    /**
     * @brief 通过uid获取玩家数据
     * @param uid
     * @return
     */
    NFWorldAccount *GetAccount(uint64_t uid);

    /**
     * @brief 通过uid创建玩家数据, 如果玩家存在，返回NULL
     * @param uid
     * @return
     */
    NFWorldAccount *CreateAccount(uint64_t uid);

    /**
     * @brief 删除玩家数据
     * @param pPlayer
     * @return
     */
    int DeleteAccount(NFWorldAccount *pPlayer);
public:
DECLARE_IDCREATE(NFWorldAccountMgr)
};