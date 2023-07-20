﻿// -------------------------------------------------------------------------
//    @FileName         :    NFWorldLoginModule.cpp
//    @Author           :    gaoyi
//    @Date             :   2023-07-10
//    @Email			:    445267987@qq.com
//    @Module           :    NFWorldLoginModule
//
// -------------------------------------------------------------------------

#include "NFLogicPlayerModule.h"

#include "NFLogicServer/NFLogicServerPlayerPlugin/Player/NFPlayer.h"
#include "NFLogicServer/NFLogicServerPlayerPlugin/Player/NFPlayerMgr.h"
#include "ServerInternalCmd.pb.h"
#include "NFLogicCommon/NFLogicBindRpcService.h"
#include "DBProto.pb.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFCore/NFRandom.hpp"

NFCLogicPlayerModule::NFCLogicPlayerModule(NFIPluginManager *p) : NFIDynamicModule(p)
{

}

NFCLogicPlayerModule::~NFCLogicPlayerModule()
{
}

bool NFCLogicPlayerModule::Awake()
{
    ////////////proxy msg////player login,disconnect,reconnet/////////////////////

    FindModule<NFIMessageModule>()->AddRpcService<proto_ff::NF_WTL_PLAYER_LOGIN_REQ>(NF_ST_LOGIC_SERVER, this,
                                                                                     &NFCLogicPlayerModule::OnRpcServicePlayerLogin, true);
    FindModule<NFIMessageModule>()->AddRpcService<proto_ff::NF_WTL_PLAYER_RECONNECT_MSG_REQ>(NF_ST_LOGIC_SERVER, this,
                                                                                     &NFCLogicPlayerModule::OnRpcServicePlayerReconnect, true);

    RegisterServerMessage(NF_ST_LOGIC_SERVER, proto_ff::NF_WTL_PLAYER_DISCONNECT_MSG);
    //////////player enter game////////////////////////////////////
    return true;
}

bool NFCLogicPlayerModule::Execute()
{
    return true;
}

bool NFCLogicPlayerModule::OnDynamicPlugin()
{
    return true;
}

int NFCLogicPlayerModule::OnHandleClientMessage(uint32_t msgId, NFDataPackage &packet, uint64_t param1, uint64_t param2)
{
    if (!m_pObjPluginManager->IsInited())
    {
        NFLogError(NF_LOG_SYSTEMLOG, packet.nParam1, "Logic Server not inited, drop client msg:{}", packet.ToString());
        return -1;
    }

    if (m_pObjPluginManager->IsServerStopping())
    {
        NFLogError(NF_LOG_SYSTEMLOG, packet.nParam1, "Logic Server is Stopping, drop client msg:{}", packet.ToString());
        return -1;
    }

    switch (packet.nMsgId)
    {
        default:
        {
            NFLogError(NF_LOG_SYSTEMLOG, 0, "Client MsgId:{} Register, But Not Handle, Package:{}", packet.nMsgId, packet.ToString());
            break;
        }
    }
    return 0;
}


int NFCLogicPlayerModule::OnHandleServerMessage(uint32_t msgId, NFDataPackage &packet, uint64_t param1, uint64_t param2)
{
    if (!m_pObjPluginManager->IsInited())
    {
        NFLogError(NF_LOG_SYSTEMLOG, packet.nParam1, "Logic Server not inited, drop client msg:{}", packet.ToString());
        return -1;
    }

    if (m_pObjPluginManager->IsServerStopping())
    {
        NFLogError(NF_LOG_SYSTEMLOG, packet.nParam1, "Logic Server is Stopping, drop client msg:{}", packet.ToString());
        return -1;
    }

    switch (packet.nMsgId)
    {
        case proto_ff::NF_WTL_PLAYER_DISCONNECT_MSG:
        {
            OnHandlePlayerDisconnectMsg(msgId, packet, param1, param2);
            break;
        }
        default:
        {
            NFLogError(NF_LOG_SYSTEMLOG, 0, "Server MsgId:{} Register, But Not Handle, Package:{}", packet.nMsgId, packet.ToString());
            break;
        }
    }
    return 0;
}

int NFCLogicPlayerModule::OnRpcServicePlayerLogin(proto_ff::Proto_WorldToLogicLoginReq &request, proto_ff::Proto_LogicToWorldLoginRsp &respone)
{
    NFLogTrace(NF_LOG_SYSTEMLOG, 0, "---------------------------------- begin ---------------------------------- ");
    respone.set_user_id(request.user_id());
    respone.set_result(0);
    auto pSnsSync = respone.mutable_sns_sync();
    pSnsSync->set_create_player_db_data(false);

    proto_ff::tbFishPlayerData selectobj;
    NFPlayer* pPlayer = NFPlayerMgr::Instance(m_pObjPluginManager)->GetPlayer(request.user_id());
    if (pPlayer == NULL)
    {
        selectobj.set_player_id(request.user_id());
        int iRet = FindModule<NFIServerMessageModule>()->GetRpcSelectObjService(NF_ST_LOGIC_SERVER, request.user_id(),
                                                                                selectobj);
        if (iRet != 0)
        {
            if (iRet == proto_ff::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY)
            {
                pSnsSync->set_create_player_db_data(true);

                proto_ff::tbFishPlayerData insertObj;
                insertObj.set_player_id(request.user_id());
                insertObj.set_nickname("gaoyi");
                insertObj.set_age(18);
                insertObj.set_gender(0);
                insertObj.set_regdate(NFTime::Now().UnixSec());

                insertObj.set_faceid(1);
                insertObj.set_jetton(1000000);

                insertObj.set_regdate(NFTime::Now().UnixSec());
                insertObj.set_phonenum(0);
                insertObj.set_ip(request.client_ip());

                NFLogTrace(NF_LOG_SYSTEMLOG, 0, "Ready Create Player InTo Mysql:{}", insertObj.DebugString());

                iRet = FindModule<NFIServerMessageModule>()->GetRpcInsertObjService(NF_ST_LOGIC_SERVER,
                                                                                    request.user_id(), insertObj);
                if (iRet != 0)
                {
                    NFLogInfo(NF_LOG_SYSTEMLOG, 0, "Insert Player:{} Failed, iRet:{}", request.user_id(), GetErrorStr(iRet));
                    respone.set_result(iRet);
                    return 0;
                }

                iRet = FindModule<NFIServerMessageModule>()->GetRpcSelectObjService(NF_ST_LOGIC_SERVER,
                                                                                    request.user_id(), selectobj);
                if (iRet != 0)
                {
                    NFLogInfo(NF_LOG_SYSTEMLOG, 0, "Insert Player:{} Success, But Select Player Failed, iRet:{}", request.user_id(),
                              GetErrorStr(iRet));
                    respone.set_result(iRet);
                    return 0;
                }

                pPlayer = NFPlayerMgr::Instance(m_pObjPluginManager)->CreatePlayer(selectobj.player_id(), selectobj, true);
                if (pPlayer == NULL)
                {
                    NFLogInfo(NF_LOG_SYSTEMLOG, 0, "NFPlayerMgr CreatePlayer:{} Failed", request.user_id());
                    respone.set_result(proto_ff::ERR_CODE_SYSTEM_ERROR);
                    return 0;
                }
            }
            else
            {
                NFLogInfo(NF_LOG_SYSTEMLOG, 0, "Select Player:{} Failed, iRet:{}", request.user_id(), GetErrorStr(iRet));
                respone.set_result(iRet);
                return 0;
            }
        }
        else
        {
            pPlayer = NFPlayerMgr::Instance(m_pObjPluginManager)->CreatePlayer(selectobj.player_id(), selectobj, false);
            if (pPlayer == NULL)
            {
                NFLogInfo(NF_LOG_SYSTEMLOG, 0, "NFPlayerMgr CreatePlayer:{} Failed", request.user_id());
                respone.set_result(proto_ff::ERR_CODE_SYSTEM_ERROR);
                return 0;
            }
        }
    }

    CHECK_NULL(pPlayer);

    int iRet = pPlayer->OnLogin();
    if (iRet != 0)
    {
        NFLogInfo(NF_LOG_SYSTEMLOG, 0, "NFPlayer:{} OnLogin:{} Failed", request.user_id(), GetErrorStr(iRet));
        respone.set_result(proto_ff::ERR_CODE_SYSTEM_ERROR);
        return 0;
    }

    pSnsSync->set_nick_name(pPlayer->GetNickName());
    pSnsSync->set_face_id(pPlayer->GetFaceId());

    NFLogTrace(NF_LOG_SYSTEMLOG, 0, "---------------------------------- end ---------------------------------- ");
    return 0;
}

int NFCLogicPlayerModule::OnRpcServicePlayerReconnect(proto_ff::WTLPlayerReconnectReq& request, proto_ff::LTWPlayerReconnectRsp& respone)
{
    NFLogTrace(NF_LOG_SYSTEMLOG, 0, "---------------------------------- begin ---------------------------------- ");
    respone.set_player_id(request.player_id());
    respone.set_result(proto_ff::ERR_CODE_OK);

    NFPlayer* pPlayer = NFPlayerMgr::Instance(m_pObjPluginManager)->GetPlayer(request.player_id());
    if (pPlayer == NULL)
    {
        respone.set_result(proto_ff::ERR_CODE_PLAYER_NOT_EXIST);
        return 0;
    }

    pPlayer->SetProxyId(request.proxy_bus_id());
    pPlayer->OnReconnect();

    NFLogInfo(NF_LOG_SYSTEMLOG, pPlayer->GetPlayerId(), "player:{} reconnect success", pPlayer->GetPlayerId());

    NFLogTrace(NF_LOG_SYSTEMLOG, 0, "---------------------------------- end ---------------------------------- ");
    return 0;
}

int NFCLogicPlayerModule::OnHandlePlayerDisconnectMsg(uint32_t msgId, NFDataPackage &packet, uint64_t param1, uint64_t param2)
{
    NFLogTrace(NF_LOG_SYSTEMLOG, 0, "---------------------------------- begin ---------------------------------- ");

    proto_ff::NotifyPlayerDisconnect xMsg;
    CLIENT_MSG_PROCESS_WITH_PRINTF(packet, xMsg);

    NFPlayer* pPlayer = NFPlayerMgr::Instance(m_pObjPluginManager)->GetPlayer(xMsg.player_id());
    if (pPlayer)
    {
        pPlayer->OnDisconnect();
    }

    NFLogTrace(NF_LOG_SYSTEMLOG, xMsg.player_id(), "player:{} disconnect..............", xMsg.player_id());

    NFLogTrace(NF_LOG_SYSTEMLOG, 0, "---------------------------------- end ---------------------------------- ");
    return 0;
}
