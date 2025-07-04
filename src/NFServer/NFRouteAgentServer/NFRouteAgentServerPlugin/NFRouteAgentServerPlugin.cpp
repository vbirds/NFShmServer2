// -------------------------------------------------------------------------
//    @FileName         :    NFGameServerPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFGameServerPlugin
//
// -------------------------------------------------------------------------

#include "NFRouteAgentServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFRouteAgentServerModule.h"

#ifdef NF_DYNAMIC_PLUGIN

NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFRouteAgentServerPlugin)

};

NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFRouteAgentServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

int NFRouteAgentServerPlugin::GetPluginVersion()
{
	return 0;
}

std::string NFRouteAgentServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFRouteAgentServerPlugin);
}

void NFRouteAgentServerPlugin::Install()
{
	NFGlobalSystem::Instance()->AddServerType(NF_ST_ROUTE_AGENT_SERVER);
	REGISTER_MODULE(m_pObjPluginManager, NFIRouteAgentServerModule, NFCRouteAgentServerModule);
}

void NFRouteAgentServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFIRouteAgentServerModule, NFCRouteAgentServerModule);
}
