// -------------------------------------------------------------------------
//    @FileName         :    NFCLogicServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCLogicServerModule
//
// -------------------------------------------------------------------------

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFLogicServerModule.h"

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFServerComm/NFServerMessage/ServerMsg.pb.h"


NFCLogicServerModule::NFCLogicServerModule(NFIPluginManager *p) : NFILogicServerModule(p)
{
}

NFCLogicServerModule::~NFCLogicServerModule()
{
}

int NFCLogicServerModule::Awake()
{
    BindServer();
    return 0;
}

int NFCLogicServerModule::OnHandleServerMessage(uint64_t unLinkId, NFDataPackage &packet)
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

int NFCLogicServerModule::Init()
{
    ConnectMasterServer();
    return 0;
}

int NFCLogicServerModule::Tick()
{
    return 0;
}

int NFCLogicServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_LOGIC_SERVER);
    return 0;
}

