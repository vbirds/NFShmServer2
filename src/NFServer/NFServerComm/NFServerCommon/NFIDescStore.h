// -------------------------------------------------------------------------
//    @FileName         :    NFIDescStore.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDescStore.h
//    @Desc             :    描述存储接口头文件，提供配置数据的存储和管理功能。
//                          该文件定义了描述存储接口类，包括数据加载接口、数据保存接口、
//                          数据重载接口、数据库操作接口。
//                          主要功能包括提供配置数据的存储和管理、支持文件加载和数据库加载、
//                          支持数据重载和热更新、提供数据验证和完整性检查。
//                          描述存储接口是NFShmXFrame框架的配置管理组件，负责：
//                          - 配置数据的存储和管理
//                          - 文件加载和数据库加载
//                          - 数据重载和热更新
//                          - 数据验证和完整性检查
//                          - MD5校验和文件监控
//                          - 数据库操作和事务处理
//                          - 内存管理和共享内存支持
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

/**
 * @brief 注册描述存储宏
 * @param className 类名
 * @note 用于注册描述存储模块到系统中，同时注册共享内存对象
 * @warning 必须确保类继承自NFIDescStore
 */
#define REGISTER_DESCSTORE(className)  \
    assert((TIsDerived<className, NFIDescStore>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->RegisterDescStore(#className, className::GetStaticClassType());\
    REGISTER_SINGLETON_SHM_OBJ(className)      \

/**
 * @brief 注册描述存储宏（带数据库名）
 * @param className 类名
 * @param dbName 数据库名
 * @note 用于注册描述存储模块到系统中，指定数据库名称
 * @warning 必须确保类继承自NFIDescStore
 */
#define REGISTER_DESCSTORE_WITH_DBNAME(className, dbName)  \
    assert((TIsDerived<className, NFIDescStore>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->RegisterDescStore(#className, className::GetStaticClassType(), dbName);\
    REGISTER_SINGLETON_SHM_OBJ(className)                  \

/**
 * @brief 注册扩展描述存储宏
 * @param className 类名
 * @note 用于注册扩展描述存储模块到系统中
 * @warning 必须确保类继承自NFIDescStoreEx
 */
#define REGISTER_DESCSTORE_EX(className)  \
    assert((TIsDerived<className, NFIDescStoreEx>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->RegisterDescStoreEx(#className, className::GetStaticClassType());\
    REGISTER_SINGLETON_SHM_OBJ(className)      \

/**
 * @brief 反注册描述存储宏
 * @param className 类名
 * @note 用于动态卸载描述存储模块，同时释放共享内存对象
 * @warning 必须确保类继承自NFIDescStore，且在模块卸载时调用
 */
#define UNREGISTER_DESCSTORE(className)  \
    assert((TIsDerived<className, NFIDescStore>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->UnRegisterDescStore(#className);\
    UNREGISTER_SHM_OBJ(className)      \

/**
 * @brief 反注册扩展描述存储宏
 * @param className 类名
 * @note 用于动态卸载扩展描述存储模块，同时释放共享内存对象
 * @warning 必须确保类继承自NFIDescStoreEx，且在模块卸载时调用
 */
#define UNREGISTER_DESCSTORE_EX(className)  \
    assert((TIsDerived<className, NFIDescStoreEx>::Result)); \
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIDescStoreModule>()->UnRegisterDescStoreEx(#className);\
    UNREGISTER_SHM_OBJ(className)      \

/**
 * @brief 描述存储接口类
 * 
 * 该类是配置数据存储和管理的基础接口，提供了：
 * - 数据加载和保存功能
 * - 数据库操作接口
 * - 数据重载和热更新
 * - 数据验证和完整性检查
 * - MD5校验和文件监控
 * - 内存管理和共享内存支持
 * 
 * 主要功能：
 * - 从文件或数据库加载配置数据
 * - 将配置数据保存到文件或数据库
 * - 支持数据重载和热更新
 * - 提供数据验证和完整性检查
 * - 支持MD5校验和文件监控
 * - 提供内存管理和共享内存支持
 * 
 * 使用方式：
 * @code
 * class MyDescStore : public NFIDescStore {
 * public:
 *     virtual int Load(NFResDb* pDB) override;
 *     virtual int SaveDescStore() override;
 *     virtual std::string GetFileName() override { return "myconfig.xlsx"; }
 * };
 * 
 * // 注册描述存储
 * REGISTER_DESCSTORE(MyDescStore);
 * @endcode
 */
class NFIDescStore : public NFObject
{
public:
    /**
     * @brief 构造函数
     */
    NFIDescStore();

    /**
     * @brief 析构函数
     */
    virtual ~NFIDescStore();

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化描述存储对象
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复描述存储对象状态
     * 
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 加载数据
     * 
     * 从数据库加载配置数据
     * 
     * @param pDB 数据库对象指针
     * @return 加载结果
     */
    virtual int Load(NFResDb *pDB) = 0;

    /**
     * @brief 从数据库加载数据
     * 
     * 从数据库加载配置数据的具体实现
     * 
     * @param pDB 数据库对象指针
     * @return 加载结果
     */
    virtual int LoadDB(NFResDb *pDB) = 0;

    /**
     * @brief 重载数据
     * 
     * 重新加载配置数据，支持热更新
     * 
     * @param pDB 数据库对象指针
     * @return 重载结果
     */
    virtual int Reload(NFResDb *pDB) = 0;

    /**
     * @brief 检查所有数据加载完成
     * 
     * 在所有数据加载完成后进行检查
     * 
     * @return 检查结果
     */
    virtual int CheckWhenAllDataLoaded() = 0;

    /**
     * @brief 初始化
     * 
     * 初始化描述存储对象
     * 
     * @return 初始化结果
     */
    virtual int Initialize() = 0;

    /**
     * @brief 计算使用率
     * 
     * 计算内存使用率
     * 
     * @return 使用率
     */
    virtual int CalcUseRatio() = 0;

    /**
     * @brief 获取文件名
     * 
     * 获取配置文件名
     * 
     * @return 文件名
     */
    virtual std::string GetFileName() = 0;

    /**
     * @brief 获取资源数量
     * 
     * 获取配置数据的资源数量
     * 
     * @return 资源数量
     */
    virtual int GetResNum() const = 0;

    /**
     * @brief 保存描述存储
     * 
     * 将配置数据保存到文件
     * 
     * @return 保存结果
     */
    virtual int SaveDescStore() = 0;

    /**
     * @brief 保存描述存储到数据库
     * 
     * 将配置数据保存到数据库
     * 
     * @param pMessage 消息对象
     * @return 保存结果
     */
    virtual int SaveDescStoreToDB(const google::protobuf::Message *pMessage);

    /**
     * @brief 插入描述存储到数据库
     * 
     * 将配置数据插入到数据库
     * 
     * @param pMessage 消息对象
     * @return 插入结果
     */
    virtual int InsertDescStoreToDB(const google::protobuf::Message *pMessage);

    /**
     * @brief 从数据库删除描述存储
     * 
     * 从数据库删除配置数据
     * 
     * @param pMessage 消息对象
     * @return 删除结果
     */
    virtual int DeleteDescStoreToDB(const google::protobuf::Message *pMessage);

    /**
     * @brief 启动保存定时器
     * 
     * 启动自动保存定时器
     * 
     * @return 启动结果
     */
    virtual int StartSaveTimer();

    /**
     * @brief 定时器回调
     * 
     * 处理定时器事件
     * 
     * @param timeId 定时器ID
     * @param callcount 调用次数
     * @return 处理结果
     */
    virtual int OnTimer(int timeId, int callcount);

    /**
     * @brief 准备重载
     * 
     * 准备重载数据
     * 
     * @return 准备结果
     */
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

    void SetDBLoaded(bool b)
    {
        m_bIsDBLoaded = b;
    }

    bool IsDBLoaded() const
    {
        return m_bIsDBLoaded;
    }

protected:
    bool m_bValid;
    bool m_bIsLoaded;
    bool m_bIsChecked;
    bool m_bIsReLoading;
    bool m_bIsDBLoaded;
    int m_bSaveTimer;
    NFShmString<MAX_MD5_STR_LEN> m_szMD5;
    NFShmString<MAX_DESC_FILE_PATH_STR_LEN> m_filePathName;
    NFShmString<MAX_DESC_NAME_LEN> m_dbName;
};
