// -------------------------------------------------------------------------
//    @FileName         :    NFCDescStoreModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCDescStoreModule
//
// -------------------------------------------------------------------------

#pragma once

#include "NFServerComm/NFServerCommon/NFIDescStoreModule.h"
#include "NFServerComm/NFServerCommon/NFIDescStore.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFServerComm/NFServerCommon/NFIDescStoreEx.h"
#include <unordered_map>

class NFCDescStoreModule : public NFIDescStoreModule
{
public:
	NFCDescStoreModule(NFIPluginManager* p);
	virtual ~NFCDescStoreModule();

    virtual bool AfterInitShmMem() override;

	virtual bool Awake() override;

    virtual bool Execute() override;

	virtual bool OnReloadConfig() override;

	/*
	 * 停服之前，检查服务器是否满足停服条件
	 * */
	virtual bool CheckStopServer() override;

	/*
	 * 停服之前，做一些操作，满足停服条件
	 * */
	virtual bool StopServer() override;

	virtual void RegisterDescStore(const std::string& strClassName, int objType, const std::string& dbName) override;
    virtual void RegisterDescStore(const std::string& strClassName, int objType) override;
    virtual void RegisterDescStoreEx(const std::string& strClassName, int objType) override;

	virtual NFIDescStore* FindDescStore(const std::string& strDescName) override;

    virtual NFIDescStore* FindDescStoreByFileName(const std::string& strDescName) override;

	virtual int Initialize();

	virtual int LoadFileDestSotre();
    virtual int LoadDBDestSotre();

	virtual void AddDescStore(const std::string& strDescName, NFIDescStore* pDesc);

    virtual void AddDescStoreEx(const std::string& strDescName, NFIDescStoreEx* pDescEx);

	virtual void RemoveDescStore(const std::string& strDescName);

    virtual int InsertDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message *pMessage) override;
    virtual int DeleteDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message *pMessage) override;;
    virtual int SaveDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message *pMessage) override;;

	virtual void InitAllDescStore();
    virtual void InitAllDescStoreEx();

	virtual int InitDescStore(const std::string& descClass, NFIDescStore* pDescStore);
    virtual int InitDescStoreEx(const std::string& descClass, NFIDescStoreEx* pDescStore);
	virtual int ExtraInitializeWhenRecover();
    virtual bool IsAllDescStoreLoad() override;;

    virtual int LoadFile();
    virtual int LoadDB();
	virtual int Reload();


	virtual int LoadDescStore(NFIDescStore *pDescStore);
	virtual int ReLoadDescStore(NFIDescStore *pDescStore);
	bool HasDBDescStore();

	virtual int CheckWhenAllDataLoaded();

	int GetFileContainMD5(const std::string& strFileName, std::string& fileMd5);

    NFResDb *CreateResDBFromRealDB();

    NFResDb *CreateResDBFromFiles(const std::string& dir);

    virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage) override;

    virtual void runAfterShmInit();
public:
    virtual int GetDescStoreByRpc(NF_SERVER_TYPE eType, const std::string& dbName, const std::string &table_name, google::protobuf::Message *pMessage) override;
private:
	std::unordered_map<std::string, NFIDescStore*> mDescStoreMap;
    std::unordered_map<std::string, NFIDescStore*> mDescStoreFileMap;
	std::unordered_map<std::string, int> mDescStoreRegister;
    std::vector<std::string> mDescStoreRegisterList;    //记录注册顺序，根据顺序来加载
	std::unordered_map<std::string, std::string> mDescStoreDBNameMap;
    NFResDb* m_pResFileDB;
    NFResDb* m_pResSqlDB;
    bool m_bStartInit;
    bool m_bFinishAllLoaded;
private:
    std::unordered_map<std::string, NFIDescStoreEx*> mDescStoreExMap;
    std::unordered_map<std::string, int> mDescStoreExRegister;
    std::vector<std::string> mDescStoreExRegisterList;    //记录注册顺序，根据顺序来加载
};
