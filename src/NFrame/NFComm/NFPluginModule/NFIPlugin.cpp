// -------------------------------------------------------------------------
//    @FileName         :    NFIPlugin.cpp
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFIPlugin
//    @Description      :    插件接口基类的默认实现
//                           提供插件生命周期管理和模块调度的通用实现
//
// -------------------------------------------------------------------------

#include "NFIPlugin.h"
#include "NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @brief 所有插件加载完成后的处理
 * 
 * 遍历插件中的所有模块，依次调用每个模块的AfterLoadAllPlugin方法。
 * 这个阶段可以进行跨插件的模块依赖初始化。
 * 
 * @return 成功返回0，失败返回非0值
 */
int NFIPlugin::AfterLoadAllPlugin()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            int iRet = pModule->AfterLoadAllPlugin();
        	CHECK_ERR(0, iRet, "{} AfterLoadAllPlugin failed!", pModule->m_strName);
        }
    }
    return 0;
}

/**
 * @brief 配置加载阶段的处理
 * 
 * 遍历插件中的所有模块，依次调用每个模块的LoadConfig方法。
 * 在这个阶段加载各种配置文件，如Excel配置、XML配置等。
 * 
 * @return 成功返回0，失败返回非0值
 */
int NFIPlugin::LoadConfig()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            int iRet = pModule->LoadConfig();
			CHECK_ERR(0, iRet, "{} AfterInitShmMem failed!", pModule->m_strName);
        }
    }
    return 0;
}

/**
 * @brief 插件唤醒阶段的处理
 * 
 * 遍历插件中的所有模块，依次调用每个模块的Awake方法。
 * 这是模块的早期初始化阶段，用于创建基础数据结构。
 * 
 * @return 成功返回0，失败返回非0值
 */
int NFIPlugin::Awake()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			int iRet = pModule->Awake();
			CHECK_ERR(0, iRet, "{} Awake failed!", pModule->m_strName);
		}
	}
	return 0;
}

/**
 * @brief 插件初始化阶段的处理
 * 
 * 遍历插件中的所有模块，依次调用每个模块的Init方法。
 * 这是模块的主要初始化阶段，完成模块的完整初始化工作。
 * 
 * @return 成功返回0，失败返回非0值
 */
int NFIPlugin::Init()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			int iRet = pModule->Init();
			CHECK_ERR(0, iRet, "{} Init failed!", pModule->m_strName);
		}
	}
	return 0;
}

/**
 * @brief 准备进入Tick循环阶段的处理
 * 
 * 遍历插件中需要Tick的模块，调用每个模块的ReadyTick方法。
 * 这是在模块开始接收Tick调用之前的最后准备工作。
 * 
 * @return 成功返回0，失败返回非0值
 */
int NFIPlugin::ReadyTick()
{
	for (size_t i = 0; i < m_vecTickModule.size(); i++)
	{
		NFIModule* pModule = m_vecTickModule[i];
		if (pModule)
		{
			pModule->ReadyTick();
		}
	}

	return true;
}

/**
 * @brief 插件Tick函数的处理
 * 
 * 遍历插件中需要Tick的模块，依次调用每个模块的Tick方法。
 * 同时进行性能监控，记录每个模块的执行时间，当执行时间超过阈值时记录警告。
 * 
 * @return 成功返回0，失败返回非0值
 */
int NFIPlugin::Tick()
{
	for (size_t i = 0; i < m_vecTickModule.size(); i++)
	{
		NFIModule* pModule = m_vecTickModule[i];
		if (pModule)
		{
            m_pObjPluginManager->BeginProfiler(pModule->m_strName + "--Loop");
            int iRet = pModule->Tick();
            if (iRet != 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "{} Tick failed!", pModule->m_strName);
            }
            uint64_t useTime = m_pObjPluginManager->EndProfiler();
            if (useTime >= 30000) //>= 10毫秒
            {
                if (!m_pObjPluginManager->IsLoadAllServer())
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "mainthread:{} use time:{} ms", pModule->m_strName + "--Loop", useTime / 1000);
                }
            }


		}
	}

	return 0;
}

int NFIPlugin::BeforeShut()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			NFLogInfo(NF_LOG_DEFAULT, 0, "{} BeforeShut................", pModule->m_strName);
			int iRet = pModule->BeforeShut();
			if (iRet != 0)
			{
				NFLogError(NF_LOG_DEFAULT, 0, "{} BeforeShut failed!", pModule->m_strName);
			}
		}
	}
	return 0;
}

int NFIPlugin::Shut()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			NFLogInfo(NF_LOG_DEFAULT, 0, "{} Shut................", pModule->m_strName);
			int iRet = pModule->Shut();
			if (iRet != 0)
			{
				NFLogError(NF_LOG_DEFAULT, 0, "{} Shut failed!", pModule->m_strName);
			}
		}
	}

	return 0;
}

int NFIPlugin::Finalize()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			NFLogInfo(NF_LOG_DEFAULT, 0, "{} Finalize................", pModule->m_strName);
			int iRet = pModule->Finalize();
			if (iRet != 0)
			{
				NFLogError(NF_LOG_DEFAULT, 0, "{} Finalize failed!", pModule->m_strName);
			}
		}
	}

	return 0;
}

int NFIPlugin::OnReloadConfig()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			int iRet = pModule->OnReloadConfig();
			if (iRet != 0)
			{
				NFLogError(NF_LOG_DEFAULT, 0, "{} OnReloadConfig failed!", pModule->m_strName);
			}
		}
	}

	return 0;
}

int NFIPlugin::AfterOnReloadConfig()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            int iRet = pModule->AfterOnReloadConfig();
            if (iRet != 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "{} AfterOnReloadConfig failed!", pModule->m_strName);
            }
        }
    }

    return 0;
}

bool NFIPlugin::InitShmObjectRegister()
{
	return 0;
}

void NFIPlugin::AddModule(const std::string& moduleName, NFIModule* pModule)
{
	if (m_mapModule.find(moduleName) != m_mapModule.end())
	{
		NF_ASSERT_MSG(false, moduleName + " Has Registerd! Exit!");
		exit(0);
	}

	m_mapModule.emplace(moduleName, pModule);
	m_vecModule.push_back(pModule);
}

void NFIPlugin::AddModuleTick(const std::string& moduleName, NFIModule* pModule)
{
	if (m_mapModule.find(moduleName) != m_mapModule.end())
	{
		NF_ASSERT_MSG(false, moduleName + " Has Registerd! Exit!");
		exit(0);
	}

	m_mapModule.emplace(moduleName, pModule);
	m_vecModule.push_back(pModule);
	m_vecTickModule.push_back(pModule);
}

void NFIPlugin::RemoveModule(const std::string& moduleName)
{
	auto it = m_mapModule.find(moduleName);
	if (it != m_mapModule.end())
	{
		for (auto vecIt = m_vecModule.begin(); vecIt != m_vecModule.end(); ++vecIt)
		{
			if (*vecIt == it->second)
			{
				m_vecModule.erase(vecIt);
				break;
			}
		}
		m_mapModule.erase(it);
	}
}

bool NFIPlugin::IsDynamicLoad()
{
	return true;
}

int NFIPlugin::OnDynamicPlugin()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			int iRet = pModule->OnDynamicPlugin();
			if (iRet != 0)
			{
				NFLogError(NF_LOG_DEFAULT, 0, "{} OnDynamicPlugin failed!", pModule->m_strName);
			}
		}
	}

	return 0;
}

int NFIPlugin::HotfixServer()
{
	int iRet = 0;
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            int iRetTemp = pModule->HotfixServer();
            if (iRetTemp != 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "{} HotfixServer failed!", pModule->m_strName);
            	iRet = iRetTemp;
            }
        }
    }

    return iRet;
}

int NFIPlugin::OnServerKilling()
{
	int iRet = 0;
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            int iRetTemp = pModule->OnServerKilling();
        	if (iRet != 0)
        	{
        		iRet = iRetTemp;
        	}
        }
    }

    return iRet;
}

int NFIPlugin::CheckStopServer()
{
	int iRet = 0;
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            int iRetTemp = pModule->CheckStopServer();
        	if (iRetTemp != 0)
            {
        		iRet = iRetTemp;
            }
        }
    }

    return iRet;
}

int NFIPlugin::StopServer()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
			pModule->StopServer();
        }
    }

    return 0;
}

int NFIPlugin::AfterAllConnectFinish()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            if (pModule->AfterAllConnectFinish() != 0)
            {
                NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAllConnectFinish Failed", GetPluginName());
            }
        }
    }

    return 0;
}

int NFIPlugin::AfterAllDescStoreLoaded()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            if (pModule->AfterAllDescStoreLoaded() != 0)
            {
                NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAllDescStoreLoaded Failed", GetPluginName());
            }
        }
    }

    return 0;
}

int NFIPlugin::AfterAllConnectAndAllDescStore()
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			if (pModule->AfterAllConnectAndAllDescStore() != 0)
			{
				NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAllConnectAndAllDescStore Failed", GetPluginName());
			}
		}
	}

	return 0;
}

int NFIPlugin::AfterObjFromDBLoaded()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            if (pModule->AfterObjFromDBLoaded() != 0)
            {
                NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterObjFromDBLoaded Failed", GetPluginName());
            }
        }
    }

    return 0;
}

int NFIPlugin::AfterServerSyncData()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            if (pModule->AfterServerSyncData() != 0)
            {
                NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterServerRegisterFinish Failed", GetPluginName());
            }
        }
    }

    return 0;
}

int NFIPlugin::AfterAppInitFinish()
{
    for (size_t i = 0; i < m_vecModule.size(); i++)
    {
        NFIModule* pModule = m_vecModule[i];
        if (pModule)
        {
            if (pModule->AfterAppInitFinish() != 0)
            {
                NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAppInitFinish Failed", GetPluginName());
            }
        }
    }

    return 0;
}


int NFIPlugin::AfterAllConnectFinish(NF_SERVER_TYPE serverType)
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			if (pModule->AfterAllConnectFinish(serverType) != 0)
			{
				NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAllConnectFinish Failed", GetPluginName());
			}
		}
	}

	return 0;
}

int NFIPlugin::AfterAllDescStoreLoaded(NF_SERVER_TYPE serverType)
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			if (pModule->AfterAllDescStoreLoaded(serverType) != 0)
			{
				NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAllDescStoreLoaded Failed", GetPluginName());
			}
		}
	}

	return 0;
}

int NFIPlugin::AfterAllConnectAndAllDescStore(NF_SERVER_TYPE serverType)
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			if (pModule->AfterAllConnectAndAllDescStore(serverType) != 0)
			{
				NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAllConnectAndAllDescStore Failed", GetPluginName());
			}
		}
	}

	return 0;
}

int NFIPlugin::AfterObjFromDBLoaded(NF_SERVER_TYPE serverType)
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			if (pModule->AfterObjFromDBLoaded(serverType) != 0)
			{
				NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterObjFromDBLoaded Failed", GetPluginName());
			}
		}
	}

	return 0;
}

int NFIPlugin::AfterServerSyncData(NF_SERVER_TYPE serverType)
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			if (pModule->AfterServerSyncData(serverType) != 0)
			{
				NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterServerRegisterFinish Failed", GetPluginName());
			}
		}
	}

	return 0;
}

int NFIPlugin::AfterAppInitFinish(NF_SERVER_TYPE serverType)
{
	for (size_t i = 0; i < m_vecModule.size(); i++)
	{
		NFIModule* pModule = m_vecModule[i];
		if (pModule)
		{
			if (pModule->AfterAppInitFinish(serverType) != 0)
			{
				NFLogWarning(NF_LOG_DEFAULT, 0, "Plugin:{} AfterAppInitFinish Failed", GetPluginName());
			}
		}
	}

	return 0;
}
