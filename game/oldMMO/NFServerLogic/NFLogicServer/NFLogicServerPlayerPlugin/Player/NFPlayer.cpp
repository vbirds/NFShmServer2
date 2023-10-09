// -------------------------------------------------------------------------
//    @FileName         :    NFUserDetail.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFUserDetail.cpp
//
// -------------------------------------------------------------------------

#include "NFPlayer.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFLogicCommon/NFLogicCommonDefine.h"
#include "NFPlayerMgr.h"
#include "NFComm/NFShmCore/NFTransBase.h"
#include "Trans/NFTransSaveRoleDetail.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "ClientServerCmd.pb.h"
#include "DescStoreEx/NFAttrMgr.h"
#include "DescStoreEx/NFRoleDescStoreEx.h"
#include "DescStore/RoleExpDesc.h"
#include "NFLogicCommon/NFEventDefine.h"
#include "Event.pb.h"
#include "NFLogicCommon/PackageDefine.h"
#include "DescStore/RoleMastermaleDesc.h"
#include "DescStore/RoleMasterfemaleDesc.h"
#include "DescStore/RoleSicklemaleDesc.h"
#include "DescStore/RoleSicklefemaleDesc.h"
#include "DescStore/RoleSwordmaleDesc.h"
#include "DescStore/RoleSwordfemaleDesc.h"
#include "DescStore/RoleTaidaomaleDesc.h"
#include "DescStore/RoleTaidaofemaleDesc.h"
#include "DescStoreEx/NFMapDescStoreEx.h"
#include "Trans/NFTransTransScene.h"
#include "ServerInternalCmd.pb.h"
#include "ServerInternalCmd2.pb.h"
#include "DescStore/AttributeAttributeDesc.h"
#include "NFLogicCommon/NFGameMath.h"

IMPLEMENT_IDCREATE_WITHTYPE(NFPlayer, EOT_LOGIC_PLAYER_ID, NFShmObj)

NFPlayer::NFPlayer() : NFShmObj()
{
    if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
    {
        CreateInit();
    }
    else
    {
        ResumeInit();
    }
}

NFPlayer::~NFPlayer()
{

}

int NFPlayer::CreateInit()
{
    m_color = 0;
    m_lastFacade.CreateInit();
    m_pPart.resize(m_pPart.max_size());
    m_calcfight = false;
    return 0;
}

int NFPlayer::ResumeInit()
{
    return 0;
}


int NFPlayer::Init(const proto_ff::RoleDBData &dbData)
{
    NFPlayerBase::Init(m_pObjPluginManager, dbData);
    SetStatus(PLAYER_STATUS_NONE);
    for (uint32_t i = PART_NONE + 1; i < PART_MAX; ++i)
    {
        m_pPart[i] = CreatePart(i, dbData);
        if (nullptr == m_pPart[i])
        {
            NFLogError(NF_LOG_SYSTEMLOG, 0, "Player Init, Create Part Failed, roleId:{} uid:{} part:{}", m_cid, m_uid, i);
            return -1;
        }
    }

    Subscribe(NF_ST_LOGIC_SERVER, EVENT_SYNC_SCENE_POS, CREATURE_PLAYER, GetRoleId(), __FUNCTION__);
    Subscribe(NF_ST_LOGIC_SERVER, EVENT_SYNC_SCENE_STATE, CREATURE_PLAYER, GetRoleId(), __FUNCTION__);

    return 0;
}

int NFPlayer::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage)
{
    switch(nEventID)
    {
        case EVENT_SYNC_SCENE_POS:
        {
            const proto_ff::SyncScenePos *pEvent = dynamic_cast<const proto_ff::SyncScenePos *>(pMessage);
            if (pEvent)
            {
                NFLogInfo(NF_LOG_SYSTEMLOG, GetRoleId(), "SyncScenePos Event:{}", pEvent->DebugString());
                if (pEvent->map_id() == m_mapId && pEvent->scene_id() == m_sceneId)
                {
                    SetPos(NFPoint3<float>(pEvent->pos()));
                }
            }
            break;
        }
        case EVENT_SYNC_SCENE_STATE:
        {
            const proto_ff::SyncSceneState *pEvent = dynamic_cast<const proto_ff::SyncSceneState *>(pMessage);
            if (pEvent)
            {
                NFLogInfo(NF_LOG_SYSTEMLOG, GetRoleId(), "SyncSceneState Event:{}", pEvent->DebugString());
                m_curstate = pEvent->cur_state();
                m_laststate = pEvent->last_state();
                if (pEvent->cur_state() == proto_ff::state_fight)
                {

                }
                else if (pEvent->cur_state() == proto_ff::state_seat)
                {

                }
            }
            break;
        }
        default:break;
    }
    return 0;
}

int NFPlayer::UnInit()
{
    NFPlayerBase::UnInit(m_pObjPluginManager);
    for (uint32_t i = PART_NONE + 1; i < PART_MAX; ++i)
    {
        RecylePart(m_pPart[i]);
    }

    return 0;
}

NFPart *NFPlayer::CreatePart(uint32_t partType, const proto_ff::RoleDBData &dbData)
{
    NFPart *pPart = NULL;
    switch (partType)
    {
        case PART_FUNCTIONUNLOCK:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_FUNCTIONUNLOCK_PART_ID));
            break;
        }
        case PART_PACKAGE:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_PACKAGE_PART_ID));
            break;
        }
        case PART_GROW:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_GROW_PART_ID));
            break;
        }
        case PART_MINI:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_MINI_PART_ID));
            break;
        }
        case PART_CHAT:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_CHAT_PART_ID));
            break;
        }
        case PART_EQUIP:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_EQUIP_PART_ID));
            break;
        }
        case PART_TITLE:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_TITLE_PART_ID));
            break;
        };
        case PART_MISSION:
        {
            pPart = dynamic_cast<NFPart *>(FindModule<NFISharedMemModule>()->CreateObj(EOT_LOGIC_MISSION_PART_ID));
            break;
        }
        default:
        {
            NFLogError(NF_LOG_SYSTEMLOG, m_cid, "Create Part Failed, partType Not Handle:{}", partType);
            break;
        }
    }

    if (pPart)
    {
        int iRet = pPart->Init(this, partType, dbData);
        if (iRet != 0)
        {
            NFLogError(NF_LOG_SYSTEMLOG, 0, "{}::Init Failed", pPart->GetClassName());
            FindModule<NFISharedMemModule>()->DestroyObj(pPart);
            return NULL;
        }
    }
    return pPart;
}

int NFPlayer::RecylePart(NFPart *pPart)
{
    CHECK_NULL(pPart);
    pPart->UnInit();
    FindModule<NFISharedMemModule>()->DestroyObj(pPart);
    return 0;
}

int NFPlayer::Tick()
{
    switch (m_status)
    {
        case PLAYER_STATUS_NONE:
        {
            if (m_curObjCreateTime + LOGIC_PLAYER_CLIENT_DISCONNECT_WAITTIME < (uint64_t) NFTime::Now().UnixSec())
            {
                OnDisconnect(ROLE_DISCONN_FROM_SERVER);
            }
        }
            break;
        case PLAYER_STATUS_ONLINE:
        {
            Update();
        }
            break;
        case PLAYER_STATUS_OFFLINE:
        {
            if (m_disonnectType == ROLE_DISCONN_FROM_CLIENT)
            {
                if ((uint64_t) NFTime::Now().UnixSec() - GetLastDiconnectTime() < LOGIC_PLAYER_CLIENT_DISCONNECT_WAITTIME &&
                    !m_pObjPluginManager->IsServerStopping())
                {
                    break;
                }
            }

            if (IsInBattle() && !m_pObjPluginManager->IsServerStopping())
            {
                break;
            }

            if (GetGameId() != 0 && !m_pObjPluginManager->IsServerStopping())
            {
                if (!IsInTransSceneing())
                {
                    LeaveScene(0, 0);
                }
                break;
            }

            if (m_transNum > 0 && !m_pObjPluginManager->IsServerStopping())
            {
                if ((uint64_t) NFTime::Now().UnixSec() - GetLastDiconnectTime() < LOGIC_PLAYER_CLIENT_DISCONNECT_WAITTIME_IN_GAME)
                    break;
            }

            OnLogout();
        }
            break;
        case PLAYER_STATUS_LOGOUT:
        default:
        {
            if (IsRoleDetailSaved())
            {
                if (m_transNum <= 0)
                {
                    NFPlayerMgr::Instance(m_pObjPluginManager)->OnLogout(this);
                }
            }
            else if (!IsRoleDetailNowSaving())
            {
                if ((m_lastSendSaveRoleDetailTime + PLAYER_ONLINESYN_TIME_GAP < (uint64_t) NFTime::Now().UnixSec() &&
                     m_lastSendSaveRoleDetailTime + PLAYER_SAVEROLE_MAX_TRYTIME > (uint64_t) NFTime::Now().UnixSec()) ||
                    m_pObjPluginManager->IsServerStopping())
                {
                    NFPlayerMgr::Instance(m_pObjPluginManager)->SavePlayerRoleDetail(this, TRANS_SAVE_ROLE_DETAIL_LOGOUT);
                }
            }
        }
        break;
    }
    return 0;
}

int NFPlayer::Update()
{
    if (IsRoleDetailNeedSave() && !IsRoleDetailNowSaving())
    {
        NFPlayerMgr::Instance(m_pObjPluginManager)->SavePlayerRoleDetail(this, TRANS_SAVE_ROLE_DETAIL_NORMAL);
    }

    for (uint32_t i = PART_NONE + 1; i < PART_MAX; ++i)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->Update(NFTime::Now().UnixSec());
        }
    }
    return 0;
}

uint32_t NFPlayer::GetCurRoleDetailSeq() const
{
    uint32_t seq = NFPlayerBase::GetCurSeq();
    for (uint32_t i = PART_NONE + 1; i < PART_MAX; ++i)
    {
        if (m_pPart[i])
        {
            seq += m_pPart[i]->GetCurRoleDetailSeq();
        }
    }
    return seq;
}

int NFPlayer::SaveDB(proto_ff::RoleDBData &dbData)
{
    NFPlayerBase::SaveDB(dbData);
    for (uint32_t i = PART_NONE + 1; i < PART_MAX; ++i)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->SaveDB(dbData);
        }
    }

    return 0;
}

void NFPlayer::SetFacadeProto(proto_ff::RoleFacadeProto &outproto)
{
    outproto.set_color(m_color);
    outproto.set_prof(GetAttr(proto_ff::A_PROF));
    for (uint32_t i = PART_NONE + 1; i < PART_MAX; ++i)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->SetFacadeProto(outproto);
        }
    }
}

void NFPlayer::SyncFacade()
{
    proto_ff::RoleFacadeProto syncFacade;
    SetFacadeProto(syncFacade);
    m_lastFacade.read_from_pbmsg(syncFacade);
    FireGame(EVENT_SYNC_SCENE_FACADE, CREATURE_PLAYER, GetRoleId(), syncFacade);
}

void NFPlayer::SetEnterSceneProto(proto_ff::RoleEnterSceneData &outproto)
{
    NFPlayerBase::SetEnterSceneProto(outproto);
    for (uint32_t i = PART_NONE + 1; i < PART_MAX; ++i)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->SetEnterSceneProto(outproto);
        }
    }

    auto &setAttr = NFAttrMgr::Instance(m_pObjPluginManager)->GameSyncAttr();
    for (auto iter = setAttr.begin(); iter != setAttr.end(); ++iter)
    {
        uint32_t attrid = (*iter);
        proto_ff::Attr64 *pAttr = outproto.mutable_attr()->add_attr_lst();
        if (nullptr != pAttr)
        {
            pAttr->set_id(attrid);
            pAttr->set_value(GetAttr(attrid));
        }
    }
}

int NFPlayer::OnLoad(bool isLoadDB)
{
    NFPlayerMgr::Instance(m_pObjPluginManager)->OnLoad(this, isLoadDB);
    return 0;
}

int NFPlayer::OnLogin(bool isLoadDB)
{
    SetStatus(PLAYER_STATUS_ONLINE);
    SetState(proto_ff::state_normal);
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->OnLogin();
        }
    }

    NFPlayerMgr::Instance(m_pObjPluginManager)->OnLogin(this, isLoadDB);
    return 0;
}

int NFPlayer::OnLogout()
{
    NFLogInfo(NF_LOG_SYSTEMLOG, GetUid(), "uid:{}, cid:{} status:{} change to PLAYER_STATUS_LOGOUT",  GetUid(), GetRoleId(), GetStatus());

    SetStatus(PLAYER_STATUS_LOGOUT);
    SetLastLogoutTime(NFTime::Now().UnixSec());

    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->OnLogout();
        }
    }

    MarkDirty();

    return 0;
}

int NFPlayer::OnDisconnect(uint32_t reason)
{
    if (GetStatus() == PLAYER_STATUS_OFFLINE)
    {
        return 0;
    }

    NFLogInfo(NF_LOG_SYSTEMLOG, GetUid(), "player:{} status:{} change to PLAYER_STATUS_OFFLINE", GetUid(), GetStatus());

    SetStatus(PLAYER_STATUS_OFFLINE);
    SetIsDisconnect(true);
    SetDisconnectType(reason);
    SetProxyId(0);
    SetClientId(0);
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->OnDisconnect();
        }
    }
    MarkDirty();

    NFPlayerMgr::Instance(m_pObjPluginManager)->OnDisconnect(this);
    return 0;
}

int NFPlayer::OnReconnect()
{
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->OnReconnect();
        }
    }

    NFPlayerMgr::Instance(m_pObjPluginManager)->OnReconnect(this);
    return 0;
}

int NFPlayer::SendMsgToClient(uint32_t nMsgId, const google::protobuf::Message &xData)
{
    FindModule<NFIServerMessageModule>()->SendMsgToProxyServer(NF_ST_LOGIC_SERVER, m_proxyId, NF_MODULE_CLIENT, nMsgId, xData, m_uid, m_clientId);
    return 0;
}

int NFPlayer::SendMsgToSnsServer(uint32_t nMsgId, const google::protobuf::Message &xData)
{
    FindModule<NFIServerMessageModule>()->SendMsgToSnsServer(NF_ST_LOGIC_SERVER, nMsgId, xData, m_uid, m_cid);
    return 0;
}

int NFPlayer::SendTransToSnsServer(uint32_t msgId, const google::protobuf::Message &xData, uint32_t req_trans_id, uint32_t rsp_trans_id)
{
    FindModule<NFIServerMessageModule>()->SendTransToSnsServer(NF_ST_LOGIC_SERVER, msgId, xData, req_trans_id, rsp_trans_id);
    return 0;
}

int NFPlayer::SendMsgToWorldServer(uint32_t nMsgId, const google::protobuf::Message &xData)
{
    FindModule<NFIServerMessageModule>()->SendMsgToWorldServer(NF_ST_LOGIC_SERVER, nMsgId, xData, m_uid, m_cid);
    return 0;
}

int NFPlayer::SendTransToWorldServer(uint32_t msgId, const google::protobuf::Message &xData, uint32_t req_trans_id, uint32_t rsp_trans_id)
{
    FindModule<NFIServerMessageModule>()->SendTransToWorldServer(NF_ST_LOGIC_SERVER, msgId, xData, req_trans_id, rsp_trans_id);
    return 0;
}

int NFPlayer::SendMsgToGameServer(uint32_t nMsgId, const google::protobuf::Message &xData)
{
    FindModule<NFIServerMessageModule>()->SendMsgToGameServer(NF_ST_LOGIC_SERVER, m_gameId, nMsgId, xData, m_uid, m_cid);
    return 0;
}

int NFPlayer::SendTransToGameServer(uint32_t msgId, const google::protobuf::Message &xData, uint32_t req_trans_id, uint32_t rsp_trans_id)
{
    FindModule<NFIServerMessageModule>()->SendTransToGameServer(NF_ST_LOGIC_SERVER, m_gameId, msgId, xData, req_trans_id, rsp_trans_id);
    return 0;
}

int NFPlayer::FireGame(uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self/* = false*/)
{
    if (GetGameId() > 0)
    {
        FireBroadcast(NF_ST_LOGIC_SERVER, NF_ST_GAME_SERVER, GetGameId(), nEventID, bySrcType, nSrcID, message, self);
    }
    return 0;
}

int NFPlayer::FireSns(uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self/* = false*/)
{
    FireBroadcast(NF_ST_LOGIC_SERVER, NF_ST_SNS_SERVER, nEventID, bySrcType, nSrcID, message, self);
    return 0;
}

int NFPlayer::EnterScene(uint64_t mapId, uint64_t sceneId, const NFPoint3<float> &dstPos)
{
    NFTransTransScene *pTrans = dynamic_cast<NFTransTransScene *>(FindModule<NFISharedMemModule>()->CreateTrans(EOT_TRANS_LOGIC_TRANS_SCENE));
    CHECK_EXPR(pTrans, -1, "CreateTrans NFTransCreateRole failed!");
    pTrans->Init(this, proto_ff::WORLD_TO_LOGIC_LOGIN_FINISH_NOTIFY);
    pTrans->InitStaticMapInfo(mapId, sceneId, dstPos);
    int iRet = pTrans->SendEnterScene();
    if (iRet != 0)
    {
        pTrans->SetFinished(iRet);
    }
    return 0;
}

int NFPlayer::LeaveScene(int type, uint32_t reqTransId)
{
    NFTransTransScene *pTrans = dynamic_cast<NFTransTransScene *>(FindModule<NFISharedMemModule>()->CreateTrans(EOT_TRANS_LOGIC_TRANS_SCENE));
    CHECK_EXPR(pTrans, -1, "CreateTrans NFTransCreateRole failed!");
    pTrans->Init(this, proto_ff::NOTIFY_LOGIC_LEAVE_GAME_REQ, 0, reqTransId);
    pTrans->InitStaticMapInfo(m_mapId, m_sceneId, m_pos);
    int iRet = pTrans->SendLeaveScene();
    if (iRet != 0)
    {
        pTrans->SetFinished(iRet);
    }
    return 0;
}

int NFPlayer::TransScene(uint64_t mapId, uint32_t transType, uint64_t dstId)
{
    NFTransTransScene *pTrans = dynamic_cast<NFTransTransScene *>(FindModule<NFISharedMemModule>()->CreateTrans(EOT_TRANS_LOGIC_TRANS_SCENE));
    CHECK_EXPR(pTrans, -1, "CreateTrans NFTransCreateRole failed!");
    pTrans->Init(this, proto_ff::CLIENT_SCENE_TRANS_REQ, 0, 0);
    int iRet = pTrans->TransScene(mapId, transType, dstId);
    if (iRet != 0)
    {
        pTrans->SetFinished(iRet);
    }
    return iRet;
}

int NFPlayer::NotifyPlayerInfo()
{
    auto pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_LOGIC_SERVER);
    CHECK_NULL(pConfig);

    proto_ff::PlayerInfoRsp rsp;
    rsp.set_cid(GetRoleId());
    rsp.set_name(GetName());
    rsp.set_zid(GetZid());
    rsp.set_opentime(pConfig->ServerOpenTime);
    rsp.set_createtime(m_curObjCreateTime);
    rsp.set_state(0);

    for (uint32_t j = 1; j < proto_ff::A_COMMON_END; j++)
    {
        if (NFAttrMgr::Instance(m_pObjPluginManager)->IsSynClient(j))
        {
            proto_ff::Attr64 *pAttr = rsp.add_attr();
            if (nullptr != pAttr)
            {
                pAttr->set_id(j);
                pAttr->set_value(GetAttr(j));
            }
        }
    }

    rsp.set_mapid(GetMapId());
    proto_ff::Vector3PB *protopos = rsp.mutable_pos();
    if (nullptr != protopos)
    {
        protopos->set_x(m_pos.x);
        protopos->set_y(m_pos.y);
        protopos->set_z(m_pos.z);
    }

    //其他部件客户端数据 这里统一改到 Player.cpp LoginGame接口中了
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
            m_pPart[i]->OnLogin(rsp);
    }

    //外观的数据获取要放到 m_pPart[i]->OnLogin(rsp) 后面
    proto_ff::RoleFacadeProto *protofacade = rsp.mutable_facade();
    if (nullptr != protofacade)
    {
        SetFacadeProto(*protofacade);
    }

    SendMsgToClient(proto_ff::CLIENT_PLAYER_INFO_RSP, rsp);

    return 0;
}


//添加属性
bool NFPlayer::AddAttr(uint32_t ANum, int64_t attrValue, SCommonSource *pSource /*= nullptr*/, bool syn /*= false*/)
{
    if (!AddAttrCache(ANum, attrValue, pSource))
    {
        return false;
    }

    if (syn)
    {
        //同步客户端
        SynAttrToClient();
    }

    return true;
}

//添加属性 不同步到客户端
bool NFPlayer::AddAttrCache(uint32_t ANum, int64_t attrValue, SCommonSource *pSource /*= nullptr*/)
{
    if (nullptr == m_pAttr || 0 == attrValue) return false;
    if (!CanAddAttr(ANum, attrValue, pSource)) return false;
    //
    int64_t oldVal = m_pAttr->GetAttr(ANum);
    bool chgflag = false;//属性值是否有改变
    bool ret = m_pAttr->AddAttr(ANum, attrValue, &chgflag);
    if (!ret) return false;
    //属性值未改变
    if (!chgflag) return true;
    //
    int64_t newVal = m_pAttr->GetAttr(ANum);
    //计算属性
    CalcAttr(ANum);
    //
    OnAddAttr(ANum, oldVal, attrValue, newVal, pSource);
    //属性改变
    OnAttrChange(ANum, oldVal, newVal, pSource);

    return true;
}

//设置属性
bool NFPlayer::SetAttr(uint32_t ANum, int64_t attrValue, SCommonSource *pSource /*= nullptr*/, bool syn /*= false*/)
{
    if (!SetAttrCache(ANum, attrValue, pSource))
    {
        return false;
    }
    if (syn)
    {
        /*	if (m_creatureKind == CREATURE_PLAYER && pSource != nullptr && ((ANum == A_CUR_HP)||(ANum == A_MAX_HP) ))
            {
                LogInfoFmtPrint(" Creature::SetAttr cid:%llu source:%d param1:%llu param2:%llu param3:%llu", m_Cid, (int)pSource->source, pSource->param1, pSource->param2, pSource->param3);
            }	*/
        //同步客户端
        SynAttrToClient();
    }
    return true;
}

//设置属性 不同步客户端
bool NFPlayer::SetAttrCache(uint32_t ANum, int64_t attrValue, SCommonSource *pSource/* = nullptr*/)
{
    if (nullptr == m_pAttr || attrValue < 0) return false;
    if (!CanSetAttr(ANum, attrValue, pSource)) return false;

    int64_t oldVal = m_pAttr->GetAttr(ANum);
    bool chgflag = false;
    bool ret = m_pAttr->SetAttr(ANum, attrValue, &chgflag);
    if (!ret) return false;
    //属性值未改变
    if (!chgflag) return true;

    int64_t newVal = m_pAttr->GetAttr(ANum);
    //计算属性
    CalcAttr(ANum);
    //
    OnSetAttr(ANum, oldVal, attrValue, newVal, pSource);
    //
    OnAttrChange(ANum, oldVal, newVal, pSource);
    //
    return true;
}

//计算属性
void NFPlayer::CalcAttr(uint32_t ANum)
{
    if (NFAttrMgr::Instance(m_pObjPluginManager)->IsSynClient(ANum))
        m_attrCache[ANum] = GetAttr(ANum);
    if (NFAttrMgr::Instance(m_pObjPluginManager)->IsBroadClient(ANum))
        m_attrBroadCache[ANum] = GetAttr(ANum);
}

//计算属性组属性 主要是把属性组中的属性汇总到总属性中 ANum:属性组中的属性ID
void NFPlayer::CalcAttrGroup(uint32_t attrgroup, uint32_t ANum, MAP_UINT32_INT64 &mapchg)
{
    if (nullptr == m_pFightAttr)
    {
        return;
    }
    if (attrgroup <= 0)
    {
        return;
    }
    if (ANum > 0)
    {
        bool chgflag = false;
        m_pFightAttr->CalcAttr(ANum, &chgflag);
        if (chgflag)
        {
            int64_t val = GetAttr(ANum);
            mapchg[ANum] = val;
            if (NFAttrMgr::Instance(m_pObjPluginManager)->IsSynClient(ANum))
                m_attrCache[ANum] = val;
            if (NFAttrMgr::Instance(m_pObjPluginManager)->IsBroadClient(ANum))
                m_attrBroadCache[ANum] = val;
        }
    }
    else
    {
        m_pFightAttr->CalcAttr(mapchg);
        for (auto &iter: mapchg)
        {
            if (NFAttrMgr::Instance(m_pObjPluginManager)->IsSynClient(ANum))
                m_attrCache[iter.first] = iter.second;
            if (NFAttrMgr::Instance(m_pObjPluginManager)->IsBroadClient(ANum))
                m_attrBroadCache[iter.first] = iter.second;
        }
    }
}

//获取属性值
int64_t NFPlayer::GetAttr(uint32_t ANum)
{
    if (NFAttrMgr::Instance(m_pObjPluginManager)->ValidFightAttrId(ANum))
    {
        if (nullptr != m_pFightAttr)
        {
            return m_pFightAttr->GetAttr(ANum);
        }
    }
    else
    {
        if (nullptr != m_pAttr)
        {
            return m_pAttr->GetAttr(ANum);
        }
    }

    return 0;
}


int64_t NFPlayer::GetAttrGroup(uint32_t attrGroup, uint32_t ANum)
{
    if (nullptr != m_pFightAttr)
    {
        return m_pFightAttr->GetAttrGroup(attrGroup, ANum);
    }
    return 0;
}

// 清除属性组
bool NFPlayer::ClearAttrGroup(uint32_t attrGroup, SCommonSource *pSource, bool syn)
{
    if (!ClearAttrGroupCache(attrGroup, pSource))
    {
        return false;
    }
    if (syn)
    {
        SynAttrToClient();
    }
    return true;
}

bool NFPlayer::ClearAttrGroupCache(uint32_t attrGroup, SCommonSource *pSource)
{
    if (nullptr == m_pFightAttr) return false;
    bool chgflag = false;
    MAP_UINT32_INT64 mapoldchg;
    m_pFightAttr->GetAttr(mapoldchg);
    bool ret = m_pFightAttr->ClearAttrGroup(attrGroup, &chgflag);
    if (!ret) return false;
    if (!chgflag) return true;
    //
    MAP_UINT32_INT64 mapnewchg;
    CalcAttrGroup(attrGroup, 0, mapnewchg);
    //
    for (auto &iterold: mapoldchg)
    {
        auto iternew = mapnewchg.find(iterold.first);
        if (iternew != mapnewchg.end() && iternew->second != iterold.second)
        {
            OnAttrChange(iterold.first, iterold.second, iternew->second, pSource);
        }
    }
    return true;
}

//增加属性组属性
bool NFPlayer::AddAttrGroup(uint32_t attrGroup, uint32_t ANum, int64_t val, SCommonSource *pSource, bool syn)
{
    if (!AddAttrGroupCache(attrGroup, ANum, val, pSource))
    {
        return false;
    }
    if (syn)
    {
        SynAttrToClient();
    }
    return true;
}

bool NFPlayer::AddAttrGroupCache(uint32_t attrGroup, uint32_t ANum, int64_t val, SCommonSource *pSource)
{
    if (nullptr == m_pFightAttr) return false;
    bool chgflag = false;
    int64_t oldval = m_pFightAttr->GetAttrGroup(attrGroup, ANum);
    bool ret = m_pFightAttr->AddAttrGroup(attrGroup, ANum, val, &chgflag);
    if (!ret) return false;
    if (!chgflag) return true;
    //
    MAP_UINT32_INT64 mapchg;
    CalcAttrGroup(attrGroup, ANum, mapchg);
    //
    int64_t newval = m_pFightAttr->GetAttrGroup(attrGroup, ANum);
    if (oldval != newval)
    {
        OnAttrChange(ANum, oldval, newval, pSource);
    }
    return true;
}

//设置属性组属性
bool NFPlayer::SetAttrGroup(uint32_t attrGroup, uint32_t ANum, int64_t val, SCommonSource *pSource, bool syn)
{
    if (!SetAttrGroupCache(attrGroup, ANum, val, pSource))
    {
        return false;
    }
    if (syn)
    {
        SynAttrToClient();
    }
    return true;
}

bool NFPlayer::SetAttrGroupCache(uint32_t attrGroup, uint32_t ANum, int64_t val, SCommonSource *pSource)
{
    if (nullptr == m_pFightAttr) return false;
    bool chgflag = false;
    int64_t oldval = m_pFightAttr->GetAttrGroup(attrGroup, ANum);
    bool ret = m_pFightAttr->SetAttrGroup(attrGroup, ANum, val, &chgflag);
    if (!ret) return false;
    if (!chgflag) return true;
    //
    MAP_UINT32_INT64 mapchg;
    CalcAttrGroup(attrGroup, ANum, mapchg);
    //
    int64_t newval = m_pFightAttr->GetAttrGroup(attrGroup, ANum);
    if (oldval != newval)
    {
        OnAttrChange(ANum, oldval, newval, pSource);
    }
    return true;
}

//同步属性
void NFPlayer::SynAttrToClient()
{
    if (!m_attrCache.empty())
    {
        proto_ff::CreatureAttrSyn rsp;
        rsp.set_cid(GetRoleId());
        string strlog;
        for (auto &iter: m_attrCache)
        {
            proto_ff::Attr64 *proto = rsp.add_attr();
            if (nullptr != proto)
            {
                proto->set_id(iter.first);
                proto->set_value(iter.second);
                //
                strlog += std::to_string(iter.first);
                strlog += "-";
                strlog += std::to_string(iter.second);
                strlog += ",";
            }
        }
        SendMsgToClient(proto_ff::CREATURE_ATTR_SYN, rsp);
        m_attrCache.clear();

        FireGame(EVENT_SYNC_CREATURE_ATTR, CREATURE_PLAYER, GetRoleId(), rsp);
    }

    if (!m_attrBroadCache.empty())
    {
        proto_ff::CreatureAttrBroadRsp rsp;
        rsp.set_cid(GetRoleId());
        for (auto &iter: m_attrBroadCache)
        {
            proto_ff::Attr64 *proto = rsp.add_attr();
            if (nullptr != proto)
            {
                proto->set_id(iter.first);
                proto->set_value(iter.second);
            }
        }
        m_attrBroadCache.clear();
        FireGame(EVENT_SYNC_CREATURE_BROADCAST_ATTR, CREATURE_PLAYER, GetRoleId(), rsp);
    }
}

void NFPlayer::SynAttrToSns(const proto_ff::CenterRoleProto& proto)
{
    FireSns(EVENT_SYNC_SNS_ATTR, CREATURE_PLAYER, GetRoleId(), proto);
}

NFPart *NFPlayer::GetPart(uint32_t partType)
{
    if (partType > PART_NONE && partType < (uint32_t) m_pPart.size())
    {
        return m_pPart[partType].GetPoint();
    }

    return NULL;
}

int NFPlayer::DailyUpdate(uint64_t unixSec)
{
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->DailyUpdate(unixSec);
        }
    }
    return 0;
}

int NFPlayer::WeekUpdate(uint64_t unixSec)
{
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->WeekUpdate(unixSec);
        }
    }
    return 0;
}

int NFPlayer::DailyZeroUpdate(uint64_t unixSec)
{
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->DailyZeroUpdate(unixSec);
        }
    }
    return 0;
}

int NFPlayer::WeekZeroUpdate(uint64_t unixSec)
{
    for (size_t i = 0; i < PART_MAX; i++)
    {
        if (m_pPart[i])
        {
            m_pPart[i]->WeekZeroUpdate(unixSec);
        }
    }
    return 0;
}

void NFPlayer::AddVirAttr(MAP_UINT32_INT64 &mapAttr, bool costFlag, SCommonSource *pSource, bool syn)
{
    MAP_UINT32_INT64::iterator iter = mapAttr.begin();
    for (; iter != mapAttr.end(); ++iter)
    {
        int64_t addVal = iter->second;
        if (costFlag)
        {
            addVal = -iter->second;
        }
/*		if (A_FIELD_EXP == iter->first)
		{
			//领域经验的，历史原因，特殊处理下
			FieldPart *pFieldPart = dynamic_cast<FieldPart*>(GetPart(PART_FIELD));
			if (nullptr != pFieldPart)
			{
				bool result = pFieldPart->addExp(addVal);
				if (result && addVal > 0)
				{
					if (addVal >= INT64_MAX)
					{
						LogWarningFmtPrint("Player::OnAddAttr....attrValue >= INT64_MAX...cid:%lu,anum:%d,value:%ld", m_Cid, iter->first, addVal);
					}
					LIST_ITEM lstItem;
					lstItem.clear();
					SItem item;
					item.nItemID = g_GetItemCfgMgr()->ItemIdByAttrId(A_FIELD_EXP);
					item.nNum = addVal;
					SyncShowToClient(EClientShowType_Virtual, 0, lstItem);
				}
			}
		}
		else*/
        {
            AddAttr(iter->first, addVal, pSource, syn);
        }
    }
}

bool NFPlayer::EnoughVirAttr(MAP_UINT32_INT64 &mapAttr)
{
    MAP_UINT32_INT64::iterator iter = mapAttr.begin();
    for (; iter != mapAttr.end(); ++iter)
    {
/*		if (A_FIELD_EXP == iter->first)
		{
			//领域经验的，历史原因，特殊处理下
			FieldPart *pFieldPart = dynamic_cast<FieldPart*>(GetPart(PART_FIELD));
			if (nullptr == pFieldPart || (int64_t)pFieldPart->getExp() < iter->second)
			{
				return false;
			}
		}
		else*/ if (GetAttr(iter->first) < iter->second)
        {
            return false;
        }
    }
    return true;
}

bool NFPlayer::CanAddAttr(uint32_t ANum, int64_t attrValue, SCommonSource *pSource)
{
    //达到最高等级之后, 玩家不在加经验了
    if (proto_ff::A_EXP == ANum)
    {
        if (attrValue < 0) //经验不允许减少
        {
            int32_t source = 0;
            int64_t param1 = 0;
            int64_t param2 = 0;
            if (nullptr != pSource)
            {
                source = pSource->src;
                param1 = pSource->param1;
                param2 = pSource->param2;
            }
            NFLogError(NF_LOG_SYSTEMLOG, GetRoleId(), "[logic] Player::CanAddAttr..attrValue < 0..cid:{},attrValue:{},source:{},param1:{},param2:{}",
                       GetRoleId(), attrValue, source, param1, param2);

            return false;
        }

        if (m_pAttr->GetAttr(proto_ff::A_LEVEL) >= NFRoleDescStoreEx::Instance(m_pObjPluginManager)->MaxLevel())
        {
            return false;
        }
    }
    else if (proto_ff::A_LEVEL == ANum || proto_ff::A_PROF == ANum)
    {
        //等级，不允许直接增加，只能增加经验那里计算得到
        //性别，职业 角色出生就确定了，后期不允许调用增加属性接口 修改
        return false;
    }

    return true;
}

void NFPlayer::OnAddAttr(uint32_t ANum, int64_t oldVal, int64_t attrValue, int64_t newVal, SCommonSource *pSource)
{
    if (proto_ff::A_EXP == ANum) //经验
    {
        CheckExp(oldVal, pSource);
    }
    else if (proto_ff::A_CUR_HP == ANum)
    {

    }
    else if (proto_ff::A_GOLD == ANum)
    {

    }

    //处理虚拟物品的tip提示
    if (proto_ff::A_EXP != ANum && attrValue > 0)//经验的再ChekExp中已经处理了，这里过滤下
    {
        if (pSource && !pSource->bTips)
        {
            //do nothing
        }
        else
        {
            /*uint64_t virItemId = g_GetItemCfgMgr()->ItemIdByAttrId(ANum);
            if (virItemId > 0)
            {
                if (attrValue >= INT64_MAX)
                {
                    LogWarningFmtPrint("Player::OnAddAttr....attrValue >= INT64_MAX...cid:%lu,anum:%d,value:%ld", m_Cid, ANum, attrValue);
                }
                //
                LIST_ITEM lstItem;
                lstItem.clear();
                SItem item;
                item.nItemID = virItemId;
                item.nNum = attrValue;
                lstItem.push_back(item);
                SyncShowToClient(EClientShowType_Virtual, 0, lstItem);
            }*/
        }
    }

    //内部日志
    /*if (A_DIAMOND == ANum || A_DIAMOND_BIND == ANum || A_GOLD == ANum || A_ARENA_COIN == ANum || A_HOLY_DIAMOND == ANum || A_HY_COIN == ANum)
    {
        LogInnerCurrency innerlog;
        innerlog.preval = oldVal;
        innerlog.newval = m_attrs[ANum];
        innerlog.optval = attrValue;
        //货币类型  1-蓝钻 2-紫钻 3-金币 4-功勋币(竞技币) 5-公会贡献 6-神钻 7-(游佳版本的金券)  8-活跃币(悠谷版本)
        if (A_DIAMOND == ANum) innerlog.currencyType = 1;
        else if (A_DIAMOND_BIND == ANum) innerlog.currencyType = 2;
        else if (A_GOLD == ANum) innerlog.currencyType = 3;
        else if (A_ARENA_COIN == ANum) innerlog.currencyType = 4;
        else if (A_HOLY_DIAMOND == ANum) innerlog.currencyType = 6;
        else if (A_HY_COIN == ANum) innerlog.currencyType = 8;
        //
        SCommonSource source;
        if (nullptr != pSource)
        {
            source = *pSource;
        }
        g_GetGLogInnerMgr()->Currency(this, innerlog, source);
    }
    */
}

bool NFPlayer::CanSetAttr(uint32_t ANum, int64_t attrValue, SCommonSource *pSource)
{
    //达到最高等级之后, 玩家不在加经验了
    if (proto_ff::A_EXP == ANum)
    {
        if (attrValue < 0)
        {
            int32_t source = 0;
            int64_t param1 = 0;
            int64_t param2 = 0;
            if (nullptr != pSource)
            {
                source = pSource->src;
                param1 = pSource->param1;
                param2 = pSource->param2;
            }
            NFLogError(NF_LOG_SYSTEMLOG, GetRoleId(), "Player::CanSetAttr...cid:{},attrValue:{},source:{},param1:{},param2:{}", GetRoleId(),
                       attrValue, source, param1, param2);
            //
            return false;
        }

        if (m_pAttr->GetAttr(proto_ff::A_LEVEL) >= NFRoleDescStoreEx::Instance(m_pObjPluginManager)->MaxLevel())
        {
            return false;
        }
    }

    return true;
}

void NFPlayer::OnSetAttr(uint32_t ANum, int64_t oldVal, int64_t attrValue, int64_t newVal, SCommonSource *pSource)
{
    if (proto_ff::A_EXP == ANum) //经验
    {
        CheckExp(oldVal, pSource);
    }
    else if (proto_ff::A_VIP_LEVEL == ANum)
    {
        if (oldVal != newVal)
        {
            proto_ff::CenterRoleProto proto;
            proto.set_vip_level(newVal);
            SynAttrToSns(proto);
        }
    }
    else if (proto_ff::A_CUR_HP == ANum)
    {

    }
}

void NFPlayer::OnAttrChange(int32_t ANum, int64_t oldVal, int64_t newVal, SCommonSource *pSource)
{
//    int64_t attrValue = newVal - oldVal;
    //判断属性改变引起的vip变化
/*    VipPart * pVipPart = dynamic_cast<VipPart*>(m_pPart[PART_VIP]);
    if (pVipPart)
    {
        pVipPart->DoAttrChange(ANum, newVal, attrValue, pSource);
    }*/
    if (ANum > proto_ff::A_NONE && ANum < proto_ff::A_FIGHT_END && m_pFightAttr->GetFightChg())
    {
        auto pcfg = AttributeAttributeDesc::Instance(m_pObjPluginManager)->GetDesc(ANum);
        if (nullptr != pcfg && pcfg->m_power > EPS)
        {
            SetCalcFight(true);
        }
    }
    if (proto_ff::A_CUR_HP == ANum)
    {
/*        uint64_t nowTick = Time::Tick();
        if (m_lastHpTick == 0 || (nowTick - m_lastHpTick) > (2 * 1000))
        {
            if (GetAttr(proto_ff::A_TEAM_ID) > 0)
            {
                proto_ff::CenterRoleProto proto;
                proto.set_hp(GetAttr(proto_ff::A_CUR_HP));
                SynAttrToSns(proto);
            }
        }*/
/*        BuffPart* pbuffpart = dynamic_cast<BuffPart*>(GetPart(PART_BUFF));
        if (nullptr != pbuffpart) pbuffpart->OnHpChange(newVal,GetAttr(A_MAX_HP));*/
    }
    if (proto_ff::A_MAX_HP == ANum)
    {
        if (GetAttr(proto_ff::A_TEAM_ID) > 0)
        {
            proto_ff::CenterRoleProto proto;
            proto.set_max_hp(GetAttr(proto_ff::A_MAX_HP));
            SynAttrToSns(proto);
        }
    }
    if (proto_ff::A_ATK == ANum )
    {
/*        PetPart* pPetPart = dynamic_cast<PetPart*>(GetPart(PART_PET));
        if (pPetPart)
        {
            pPetPart->OnChgAttr();
        }*/
    }
}

//检查经验值
void NFPlayer::CheckExp(int64_t oldexp, SCommonSource *pSource)
{
    int64_t curexp = m_pAttr->GetAttr(proto_ff::A_EXP);
    int32_t level = m_pAttr->GetAttr(proto_ff::A_LEVEL);
    int64_t addexp = curexp - oldexp;
    //获得经验日志
    SCommonSource logsrc;
    if (nullptr != pSource)
    {
        logsrc = *pSource;
    }
    //
    uint32_t maxLv = NFRoleDescStoreEx::Instance(m_pObjPluginManager)->MaxLevel();
    bool levFlag = false;
    auto pExpCfg = RoleExpDesc::Instance(m_pObjPluginManager)->GetDesc(level);
    if (nullptr != pExpCfg)
    {
        while (nullptr != pExpCfg && curexp >= pExpCfg->m_exp)
        {
            if (level >= (int32_t) maxLv)
            {
                break;
            }
            //升级
            curexp -= pExpCfg->m_exp;
            level += 1;
            m_pAttr->SetAttr(proto_ff::A_LEVEL, level);
            m_pAttr->SetAttr(proto_ff::A_EXP, curexp);
            if (level >= (int32_t) maxLv)
            {
                m_pAttr->SetAttr(proto_ff::A_EXP, 0);
            }
            MarkDirty();
            //升级标记
            levFlag = true;
            //发送玩家升级事件
            proto_ff::PlayerLeveUpEvent levelupInfo;
            levelupInfo.set_cid(GetRoleId());
            levelupInfo.set_level(level);
            FireExecute(NF_ST_LOGIC_SERVER, EVENT_LEVELUP, GetRoleId(), CREATURE_PLAYER, levelupInfo);
            //
            pExpCfg = RoleExpDesc::Instance(m_pObjPluginManager)->GetDesc(level);
        }

        if (levFlag)
        {
            //记录日志
            //g_GetGLogStatisMgr()->LevelUp(this);
            //g_GetGLogInnerMgr()->Levup(this);
            //
            CalcAttr(proto_ff::A_LEVEL);
            CalcAttr(proto_ff::A_EXP);
            SetAttr(proto_ff::A_CUR_HP, GetAttr(proto_ff::A_MAX_HP));
            CalcLevelAttr(true);
            //等级同步到中心服
            //proto_ff::CenterRoleProto proto;
            //proto.set_level(GetAttr(proto_ff::A_LEVEL));
            //SynAttrToCenter(proto);
            //
/*            SkillPart *pskillpart = dynamic_cast<SkillPart *>(GetPart(PART_SKILL));
            if (nullptr != pskillpart)
            {
                pskillpart->OnLevelUp(true);
            }*/
        }
        if (addexp > 0)
        {
            //前端飘经验
            LIST_ITEM lstItem;
            if (nullptr != pSource && S_KillMonsExp == pSource->src)
            {
                //SyncShowToClient(EClientShowType::MonsExp, addexp, lstItem);
            }
            else
            {
                //SyncShowToClient(EClientShowType::Exp, addexp, lstItem);
            }
        }
    }
}

//计算等级属性
void NFPlayer::CalcLevelAttr(bool sync)
{
    int32_t level = m_pAttr->GetAttr(proto_ff::A_LEVEL);
    int32_t prof = m_pAttr->GetAttr(proto_ff::A_PROF);
    ClearAttrGroupCache(proto_ff::EAttrGroup_Level);
    switch (prof)
    {
        case proto_ff::EROLE_PROF_MALE_MASTER:
        {
            auto pcfg = RoleMastermaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        case proto_ff::EROLE_PROF_FEMALE_MASTER:
        {
            auto pcfg = RoleMasterfemaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        case proto_ff::EROLE_PROF_MALE_SICKLE:
        {
            auto pcfg = RoleSicklemaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        case proto_ff::EROLE_PROF_FEMALE_SICKLE:
        {
            auto pcfg = RoleSicklefemaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        case proto_ff::EROLE_PROF_MALE_SWORD:
        {
            auto pcfg = RoleSwordmaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        case proto_ff::EROLE_PROF_FEMALE_SWORD:
        {
            auto pcfg = RoleSwordfemaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        case proto_ff::EROLE_PROF_MALE_KNIFE:
        {
            auto pcfg = RoleTaidaomaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        case proto_ff::EROLE_PROF_FEMALE_KNIFE:
        {
            auto pcfg = RoleTaidaofemaleDesc::Instance(m_pObjPluginManager)->GetDesc(level);
            if (nullptr != pcfg)
            {
                for (int i = 0; i < (int) pcfg->m_attribute.size(); i++)
                {
                    auto pInfo = &pcfg->m_attribute.at(i);
                    if (pInfo->m_type > 0 && pInfo->m_value > 0)
                    {
                        SetAttrGroupCache(proto_ff::EAttrGroup_Level, pInfo->m_type, pInfo->m_value);
                    }
                }
            }
        }
            break;
        default:
            break;
    }
    //
    int64_t speed = GetAttrGroup(proto_ff::EAttrGroup_Level, proto_ff::A_SPEED);
    auto pborncfg = NFMapDescStoreEx::Instance(m_pObjPluginManager)->GetBornCfg(prof);
    if (nullptr != pborncfg)
    {
        speed += pborncfg->m_mspeed;
        SetAttrGroupCache(proto_ff::EAttrGroup_Level, proto_ff::A_SPEED, speed);
    }

    if (sync)
    {
        SynAttrToClient();
    }
}

int NFPlayer::LoginGame(const CharLoginInfo &loginInfo, bool change)
{
    NFPoint3<float> enterpos = loginInfo.pos;
    //先设置坐标和场景
    m_sceneId = loginInfo.sceneid;
    m_mapId = loginInfo.mapid;
    m_pos = enterpos;

    //如果是切换逻辑节点，设置下上一个地图和坐标
    if (change)
    {
        m_lastMapId = loginInfo.lastMapId;
        m_lastSceneId = loginInfo.lastSceneId;
        m_lastPos = loginInfo.lastpos;
    }

    m_proxyId = loginInfo.gateId;
    m_channelId = loginInfo.channelId;
    m_clientId = loginInfo.clientId;

    MarkDirty();

    NotifyPlayerInfo();

    OnLogin(true);

    EnterScene(m_mapId, m_sceneId, m_pos);
    return 0;
}

int NFPlayer::LogoutGame(int type, uint32_t reqTransId)
{
    OnDisconnect(ROLE_DISCONN_FROM_SERVER);
    if (GetGameId() > 0)
    {
        LeaveScene(type, reqTransId);
    }

    return 0;
}

//计算战力
void NFPlayer::CalcFight(bool sync)
{
//    int64_t fight = 0;
/*    BuffPart* pbuffpart = dynamic_cast<BuffPart*>(GetPart(PART_BUFF));
    SkillPart* pskillpart = dynamic_cast<SkillPart*>(GetPart(PART_SKILL));
    MAP_UINT32_INT64 mapattr;
    m_pFightAttr->GetFightAttr(mapattr);
    //
    for (auto &iter : mapattr)
    {
        const AttributeAttributeCfgInfo* pcfg = g_GetAttributeAttributeCfgTable()->GetAttributeAttributeCfgInfo(iter.first);
        if (nullptr == pcfg || pcfg->power <= EPS) continue;
        int64_t value = iter.second;
        if (nullptr != pbuffpart) value += pbuffpart->GetFightAttr(iter.first);
        fight += (int64_t)(pcfg->power * value);
    }
    if (nullptr != pskillpart) fight += pskillpart->SkillFight();
    //
    if (fight != m_pAttr->GetAttr(A_FIGHT))
    {
        //LogDebugFmtPrint("[logic] Player::CalcFight....cid:%lu, old:%lu, new:%lu, sync:%d ", Cid(), GetAttr(A_FIGHT), fight, sync);
        SetAttr(A_FIGHT, fight, nullptr, sync);
        if (sync)
        {
            //同步到中心服
            CenterRoleProto proto;
            proto.set_fight(GetAttr(A_FIGHT));
            SynAttrToCenter(proto);
        }
    }*/
    m_pFightAttr->ClearFightChg();
    SetCalcFight(false);
}

