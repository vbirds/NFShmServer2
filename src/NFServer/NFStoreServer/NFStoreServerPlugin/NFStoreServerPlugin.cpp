// -------------------------------------------------------------------------
//    @FileName         :    NFGameServerPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFGameServerPlugin
//
// -------------------------------------------------------------------------

#include "NFStoreServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFCStoreServerModule.h"

#ifdef NF_DYNAMIC_PLUGIN

NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFStoreServerPlugin)

};

NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFStoreServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

int NFStoreServerPlugin::GetPluginVersion()
{
	return 0;
}

std::string NFStoreServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFStoreServerPlugin);
}

void NFStoreServerPlugin::Install()
{
	NFGlobalSystem::Instance()->AddServerType(NF_ST_STORE_SERVER);
	REGISTER_MODULE(m_pObjPluginManager, NFIStoreServerModule, NFCStoreServerModule);
}

void NFStoreServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFIStoreServerModule, NFCStoreServerModule);
}
