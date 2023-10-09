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
#include "NFLogicCommon/NFServerFrameTypeDefines.h"
#include "NFComm/NFShmStl/NFShmHashMap.h"
#include "NFWorldCommonDefine.h"
#include "NFLoginQueue.h"
#include "DBProto.pb.h"
#include <vector>

enum
{
    /**
     * @brief 通过roleId建立索引
     */
    NF_WORLD_PLAYER_ROLE_ID_INDEX = 0,
    NF_WORLD_PLAYER_MAX_INDEX = 1,
};

class NFWorldPlayer;
class NFWorldSession;
class NFWorldPlayerMgr : public NFShmObj {
public:
    NFWorldPlayerMgr();
    virtual ~NFWorldPlayerMgr();

    int CreateInit();
    int ResumeInit();

    virtual int OnTimer(int timeId, int callcount) override;
public:
    /**
     * @brief
     * @return
     */
    int PlayerTick();

    int SessionTick();
public:

    /**
     * @brief 通过uid获取玩家数据
     * @param uid
     * @return
     */
    NFWorldPlayer *GetPlayerByUid(uint64_t uid);

    /**
     * @brief 通过uid创建玩家数据, 如果玩家存在，返回NULL
     * @param uid
     * @return
     */
    NFWorldPlayer *CreatePlayerByUid(uint64_t uid);

    /**
     * @brief 删除玩家数据
     * @param pPlayer
     * @return
     */
    int DeletePlayer(NFWorldPlayer *pPlayer);

public:
    /**
     * @brief 排队
     */

    uint32_t GetCurrentLoginNum() const;

    void SetCurrentLoginNum(uint32_t currentLoginNum);

    uint32_t GetMaxQueueNum() const;

    void SetMaxQueueNum(uint32_t maxQueueNum);

    uint32_t GetStartQueueNum() const;

    void SetStartQueueNum(uint32_t startQueueNum);

    /**
     * @brief 是否需要排队
     * @return
     */
    bool IsNeedLoginQueue() const;

    /**
     * @brief 是否排队队列已经满了
     * @return
     */
    bool IsLoginQueueFull() const;

    /**
     * @brief 是否在排队队列中
     * @param playerId
     * @return
     */
    bool IsInLoginQueue(uint64_t playerId) const;

     /**
      * @brief 加入到登陆排队中
      * @param playerId
      * @return
      */
    bool InsertLoginQueue(uint64_t playerId);

    /**
     * @brief 通过玩家ID获得排队数据
     * @param playerId
     * @return
     */
    NFLoginQueue* GetLoginQueue(uint64_t playerId);

    /**
     * @brief 获得当前排队人数
     * @return
     */
    uint32_t GetLoginQueueNum() const;

    /**
     * @brief 删除玩家的排队信息
     * @param playerId
     * @return
     */
    int DeleteLoginQueue(uint64_t playerId);

    int HandleLoginQueue(NFWorldPlayer* pPlayer);
public:
    /**
     * @brief
     * @param charDBBaseInfo
     * @param charSimpleInfo
     * @return
     */
    int CharDBToCharSimpleDB(const proto_ff::RoleDBData& charDBBaseInfo, proto_ff::LoginRoleProto& charSimpleInfo);

    /**
     * @brief 通知网关离开游戏
     * @param pPlayer
     * @param flag
     * @return
     */
    int NotifyGateLeave(uint64_t proxyId, uint64_t clientId, proto_ff::LOGOUT_TYPE flag = proto_ff::LOGOUT_NONE);

    /**
     * @brief
     * @param cid
     * @param uid
     * @param clientId
     * @param logicId
     * @param type
     * @return
     */
    int NotifyLogicLeave(NFWorldPlayer* pPlayer, NFWorldSession* pSession, proto_ff::LOGOUT_TYPE type, int reqTransId = 0);

    /**
     * @brief 通知ProxyServer玩家服务器的busId改变
     * @param pPlayer
     * @param serverType
     * @param busId
     * @return
     */
    int NotifyGateChangeServerBusId(NFWorldPlayer* pPlayer, uint32_t serverType, uint32_t busId);

    /**
     * @brief 通知LogicServer玩家掉线
     * @param pPlayer
     * @param reason
     * @return
     */
    int NotifyOtherServerPlayerDisconnect(NFWorldPlayer* pPlayer, uint32_t reason);
    int NotifyOtherServerPlayerDisconnect(uint64_t uid, uint64_t roleId, uint32_t logicId, uint32_t reason);

    int OnHandlePlayerDisconnect(NFWorldPlayer* pPlayer, uint32_t reason);
private:
    /**
     * @brief 玩家ticker定时
     */
    int m_playerTickTimer;
private:
    /**
     * @brief 排队
     */

    /**
     * @brief 记录排队uid表列
     */
    NFShmHashMap<uint64_t, NFLoginQueue, WORLD_SERVER_MAX_ONLINE_COUNT> m_loginQueueMap;

    /**
     * @brief 计算已经进入的登陆的人数  包含在选角色界面和进入游戏的
     */
    uint32_t m_currentLoginNum;

    /**
     * @brief 最多排队的人数
     */
    uint32_t m_maxQueueNum;

    /**
     * @brief 开始排队人数
     */
    uint32_t m_startQueueNum;
DECLARE_IDCREATE(NFWorldPlayerMgr)
};