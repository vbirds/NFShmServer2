﻿// -------------------------------------------------------------------------
//    @FileName         :    NFCCenterServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2023-10-19
//    @Email			:    445267987@qq.com
//    @Module           :    NFCCenterServerModule
//
// -------------------------------------------------------------------------

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFCenterServerModule.h"

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFServerComm/NFServerMessage/ServerMsg.pb.h"

NFCCenterServerModule::NFCCenterServerModule(NFIPluginManager* p): NFICenterServerModule(p)
{
}

NFCCenterServerModule::~NFCCenterServerModule()
{
}

bool NFCCenterServerModule::Awake()
{
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_CENTER_SERVER);
    CHECK_EXPR_ASSERT(pConfig, false, "GetAppConfig Failed, server type:{}", NF_ST_CENTER_SERVER);
    
    if (!pConfig->IsCrossServer())
    {
        SetCheckCenterServer(true);
    }
    BindServer();
	return true;
}

bool NFCCenterServerModule::Init()
{
    ConnectMasterServer();
    return true;
}

bool NFCCenterServerModule::Execute()
{
	return true;
}

bool NFCCenterServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_CENTER_SERVER);
	return true;
}

int NFCCenterServerModule::OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    int retCode = 0;
    switch (packet.nMsgId)
    {
        default:
            NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) not handle", packet.ToString());
            break;
    }

    if (retCode != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) handle exist error", packet.ToString());
    }
    return 0;
}
