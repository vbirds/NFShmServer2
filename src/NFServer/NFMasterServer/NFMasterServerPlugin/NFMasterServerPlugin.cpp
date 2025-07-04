// -------------------------------------------------------------------------
//    @FileName         :    NFMasterServerPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMasterServerPlugin
//
// -------------------------------------------------------------------------

#include "NFMasterServerPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFMasterServerModule.h"

#ifdef NF_DYNAMIC_PLUGIN

NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFMasterServerPlugin)

};

NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFMasterServerPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

int NFMasterServerPlugin::GetPluginVersion()
{
	return 0;
}

std::string NFMasterServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NFMasterServerPlugin);
}

void NFMasterServerPlugin::Install()
{
	NFGlobalSystem::Instance()->AddServerType(NF_ST_MASTER_SERVER);
	REGISTER_MODULE(m_pObjPluginManager, NFIMasterServerModule, NFCMasterServerModule)
}

void NFMasterServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(m_pObjPluginManager, NFIMasterServerModule, NFCMasterServerModule)
}
