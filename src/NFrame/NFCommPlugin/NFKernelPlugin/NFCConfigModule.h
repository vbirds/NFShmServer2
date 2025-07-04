// -------------------------------------------------------------------------
//    @FileName         :    NFCConfigModule.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFKernelPlugin
//    @Desc             :
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIConfigModule.h"

#include "NFComm/NFPluginModule/NFILuaLoader.h"
#include <string>

class NFCConfigModule : public NFIConfigModule, public NFILuaLoader
{
public:
	explicit NFCConfigModule(NFIPluginManager* p);

	virtual ~NFCConfigModule();

	bool BeforeShut() override;
	bool Shut() override;
	bool Finalize() override;
	bool Execute() override;
	bool OnReloadConfig() override;

	virtual bool LoadConfig() override;
public:
	
	bool LoadPluginConfig();
	bool LoadServerConfig();
	bool LoadLogConfig();
	bool CheckConfig();
public:
	NFPluginConfig* GetPluginConfig(const std::string& pluginName) override;
	NFLogConfig* GetLogConfig() override;
public:
    virtual NFServerConfig* GetServerConfig(NF_SERVER_TYPE eServerType) override;
    virtual NFServerConfig* GetAppConfig(NF_SERVER_TYPE eServerType) override;
    virtual std::string GetDefaultDBName(NF_SERVER_TYPE nfServerTypes) override;
    virtual std::string GetCrossDBName(NF_SERVER_TYPE nfServerTypes) override;
    virtual std::string GetRedisIp(NF_SERVER_TYPE nfServerTypes) override;
    virtual uint32_t GetRedisPort(NF_SERVER_TYPE nfServerTypes) override;
    virtual std::string GetRedisPass(NF_SERVER_TYPE nfServerTypes) override;
    virtual NFrame::ServerInfoReport GetDefaultMasterInfo(NF_SERVER_TYPE eServerType) override;
protected:
	std::unordered_map<std::string, NFPluginConfig*> mPluginConfig; //pluginName--key
	std::vector<NFServerConfig*> mServerConfig; //serverid--key
    NFServerConfig* m_appConfig;
	NFLogConfig mLogConfig;
};
