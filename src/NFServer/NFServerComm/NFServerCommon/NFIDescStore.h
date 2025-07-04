// -------------------------------------------------------------------------
//    @FileName         :    NFIDescStore.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDescStore.h
//
// -------------------------------------------------------------------------

#pragma once

#include "NFServerComm/NFDescStorePlugin/NFDescStoreDefine.h"
#include "NFIDescStoreModule.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFShmStl/NFShmString.h"
#include "NFComm/NFObjCommon/NFResDb.h"
#include <fstream>
#include <iostream>
#include <sstream>

#define REGISTER_DESCSTORE(className)  \
    assert((TIsDerived<className, NFIDescStore>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->RegisterDescStore(#className, className::GetStaticClassType());\
    REGISTER_SINGLETON_SHM_OBJ_GLOBAL(className)      \

#define REGISTER_DESCSTORE_WITH_DBNAME(className, dbName)  \
    assert((TIsDerived<className, NFIDescStore>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->RegisterDescStore(#className, className::GetStaticClassType(), dbName);\
    REGISTER_SINGLETON_SHM_OBJ_GLOBAL(className)                  \


#define REGISTER_DESCSTORE_EX(className)  \
    assert((TIsDerived<className, NFIDescStoreEx>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->RegisterDescStoreEx(#className, className::GetStaticClassType());\
    REGISTER_SINGLETON_SHM_OBJ_GLOBAL(className)      \

class NFIDescStore : public NFObject
{
public:
    NFIDescStore();

    virtual ~NFIDescStore();

    int CreateInit();

    int ResumeInit();

    virtual int Load(NFResDb *pDB) = 0;

    virtual int Reload(NFResDb *pDB) = 0;

    virtual int CheckWhenAllDataLoaded() = 0;

    virtual int Initialize() = 0;

    virtual int CalcUseRatio() = 0;

    virtual std::string GetFileName() = 0;

    virtual int GetResNum() const = 0;

    virtual int SaveDescStore() = 0;

    virtual int SaveDescStoreToDB(const google::protobuf::Message *pMessage);

    virtual int InsertDescStoreToDB(const google::protobuf::Message *pMessage);

    virtual int DeleteDescStoreToDB(const google::protobuf::Message *pMessage);

    virtual int StartSaveTimer();

    //must be virtual
    virtual int OnTimer(int timeId, int callcount);

    virtual int PrepareReload()
    {
        return 0;
    }

    virtual bool IsFileLoad()
    {
        return true;
    }

    virtual bool IsNeedSpecialCheck()
    {
        return false;
    }

    virtual bool IsNeedReload()
    {
        return false;
    }

    void SetValid()
    {
        m_bValid = true;
    }

    bool IsValid()
    {
        return m_bValid;
    }

    void SetLoaded(bool bIsLoaded)
    {
        m_bIsLoaded = bIsLoaded;
    }

    bool IsLoaded()
    {
        return m_bIsLoaded;
    }

    void SetReLoading(bool bIsLoaded)
    {
        m_bIsReLoading = bIsLoaded;
    }

    bool IsReloading()
    {
        return m_bIsReLoading;
    }

    void SetChecked(bool bIsChecked)
    {
        m_bIsChecked = bIsChecked;
    }

    bool IsChecked()
    {
        return m_bIsChecked;
    }

    void SetMD5(const std::string &pszMD5)
    {
        m_szMD5 = pszMD5;
    }

    std::string GetFileMD5()
    {
        return m_szMD5.GetString();
    }

    std::string GetFilePathName()
    {
        return m_filePathName.GetString();
    }

    void SetFilePathName(const std::string &filePath)
    {
        m_filePathName = filePath;
    }

    std::string GetDBName()
    {
        return m_dbName.GetString();
    }

    void SetDBName(const std::string &dbName)
    {
        m_dbName = dbName;
    }

protected:
    bool m_bValid;
    bool m_bIsLoaded;
    bool m_bIsChecked;
    bool m_bIsReLoading;
    int m_bSaveTimer;
    NFShmString<MAX_MD5_STR_LEN> m_szMD5;
    NFShmString<MAX_DESC_FILE_PATH_STR_LEN> m_filePathName;
    NFShmString<MAX_DESC_NAME_LEN> m_dbName;
};
