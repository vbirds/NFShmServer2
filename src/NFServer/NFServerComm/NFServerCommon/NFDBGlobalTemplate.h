// -------------------------------------------------------------------------
//    @FileName         :    NFDBGlobalTemplate.h
//    @Author           :    gaoyi
//    @Date             :    23-11-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDBGlobalTemplate
//    @Desc             :    数据库全局模板头文件，提供数据库对象的模板化实现。
//                          该文件定义了数据库对象模板类，包括对象模板、全局模板、
//                          Protobuf数据转换、数据库操作接口。
//                          主要功能包括提供数据库对象的模板化实现、支持Protobuf数据转换、
//                          支持数据库操作接口、提供全局对象管理。
//                          数据库全局模板是NFShmXFrame框架的数据库操作模板组件，负责：
//                          - 数据库对象的模板化实现
//                          - Protobuf数据转换和验证
//                          - 数据库操作接口的统一实现
//                          - 全局对象管理和生命周期
//                          - 数据库主键管理和验证
//                          - 跨服务器数据库同步支持
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFCore/NFCommon.h>
#include <NFComm/NFPluginModule/NFStoreProtoCommon.h>

#include "NFBaseDBObj.h"
#include "NFDBObjMgr.h"

/**
 * @brief 数据库对象模板类
 * 
 * 该类是数据库对象的模板实现，提供了：
 * - 数据库对象的模板化实现
 * - Protobuf数据转换和验证
 * - 数据库操作接口的统一实现
 * - 对象生命周期管理
 * 
 * @tparam className 类名
 * @tparam pbClass Protobuf类名
 * @tparam classType 类类型
 * 
 * 主要功能：
 * - 提供数据库对象的模板化实现
 * - 支持Protobuf数据转换和验证
 * - 提供数据库操作接口的统一实现
 * - 支持对象生命周期管理
 * - 支持数据库主键管理和验证
 * 
 * 使用方式：
 * @code
 * class MyDBObj : public NFDBObjectTemplate<MyDBObj, MyProtoClass, EOT_MY_DB_OBJ> {
 * public:
 *     virtual int LoadFromDB(const MyProtoClass& dbData) override;
 *     virtual int SaveToDB(const MyProtoClass& dbData) override;
 * };
 * @endcode
 */
template<typename className, typename pbClass, int classType>
class NFDBObjectTemplate : public NFObjectTemplate<className, classType, NFBaseDBObj>
{
public:
    /**
     * @brief 构造函数
     * 
     * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
     */
    NFDBObjectTemplate()
    {
        if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化数据库对象模板
     * 
     * @return 初始化结果
     */
    int CreateInit()
    {
        return 0;
    }

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复数据库对象模板状态
     * 
     * @return 初始化结果
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 析构函数
     */
    virtual ~NFDBObjectTemplate()
    {
    }

    /**
     * @brief 加载数据
     * 
     * 设置服务器类型并从数据库加载对象数据
     * 
     * @param serverType 服务器类型
     * @return 加载结果
     */
    virtual int Load(NF_SERVER_TYPE serverType)
    {
        NFBaseDBObj::SetServerType(serverType);
        return NFDBObjMgr::Instance()->LoadFromDB(this);
    }

    /**
     * @brief 制作加载数据
     * 
     * 将对象数据转换为Protobuf消息格式用于加载，设置主键
     * 
     * @param data 数据消息对象
     * @return 处理结果
     */
    virtual int MakeLoadData(google::protobuf::Message *data)
    {
        CHECK_NULL(0, data);
        pbClass *p = dynamic_cast<pbClass *>(data);
        CHECK_NULL(0, p);

        int iRetCode = NFProtobufCommon::SetPrimarykeyFromMessage(p, NFCommon::tostr(GetDbId()));
        CHECK_EXPR(iRetCode == 0, -1, "SetPrimarykeyFromMessage Failed From {}", data->GetTypeName());

        return 0;
    }

    /**
     * @brief 制作保存数据
     * 
     * 将对象数据转换为Protobuf消息格式用于保存，设置主键并保存到数据库
     * 
     * @param data 数据消息对象
     * @return 处理结果
     */
    virtual int MakeSaveData(google::protobuf::Message *data)
    {
        if (!this->m_bDataInited)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "not inited !");
            return -1;
        }
        CHECK_NULL(0, data);

        pbClass *p = dynamic_cast<pbClass *>(data);
        CHECK_NULL(0, p);

        int iRetCode = NFProtobufCommon::SetPrimarykeyFromMessage(p, NFCommon::tostr(GetDbId()));
        CHECK_EXPR(iRetCode == 0, -1, "SetPrimarykeyFromMessage Failed From {}", data->GetTypeName());
        return SaveToDB(*p);
    }

    /**
     * @brief 使用数据库数据初始化
     * 
     * 使用从数据库加载的数据初始化对象，验证主键并加载数据
     * 
     * @param pData 数据库数据
     * @return 初始化结果
     */
    virtual int InitWithDBData(const google::protobuf::Message *pData)
    {
        const pbClass *pGlobal = dynamic_cast<const pbClass *>(pData);
        CHECK_NULL(0, pGlobal);

        std::string dbId;
        int iRetCode = NFProtobufCommon::GetPrimarykeyFromMessage(pGlobal, dbId);
        CHECK_EXPR(iRetCode == 0, -1, "GetPrimarykeyFromMessage Failed From {}", pData->GetTypeName());
        CHECK_EXPR(NFCommon::strto<int>(dbId) == GetDbId(), -1, "invalid data!");
        int iRet = LoadFromDB(*pGlobal);
        CHECK_EXPR(iRet == 0, -1, "parse failed!");

        this->m_bDataInited = true;
        return 0;
    }

    virtual int InitWithoutDBData()
    {
        int iRet = InitConfig();
        CHECK_EXPR(iRet == 0, -1, "InitConfig failed!");
        this->m_bDataInited = true;
        return 0;
    }

    /**
     * @brief 从数据库加载数据
     * 
     * 从Protobuf数据加载对象数据，需要子类实现
     * 
     * @param dbData 数据库数据
     * @return 加载结果
     */
    virtual int LoadFromDB(const pbClass &dbData) = 0;

    /**
     * @brief 保存到数据库
     * 
     * 将对象数据保存到数据库，需要子类实现
     * 
     * @param dbData 数据库数据
     * @return 保存结果
     */
    virtual int SaveToDB(pbClass &dbData) = 0;

    virtual int InitConfig() = 0;

    virtual int GetDbId() = 0;

    virtual google::protobuf::Message *CreateTempProtobufData() { return new pbClass(); }
    virtual uint32_t GetSaveDis() { return 60; }

    /**
     * @brief 处理失败策略
     * 
     * 获取初始化失败时的处理策略
     * 
     * @return 失败处理策略
     */
    virtual eDealWithLoadFailed DealWithFailed() { return EN_DW_SHUTDOWN; }
};

/**
 * @brief 数据库全局模板类
 * 
 * 该类是数据库全局对象的模板实现，提供了：
 * - 数据库全局对象的模板化实现
 * - Protobuf数据转换和验证
 * - 数据库操作接口的统一实现
 * - 全局对象生命周期管理
 * 
 * @tparam className 类名
 * @tparam pbClass Protobuf类名
 * @tparam classType 类类型
 * 
 * 主要功能：
 * - 提供数据库全局对象的模板化实现
 * - 支持Protobuf数据转换和验证
 * - 提供数据库操作接口的统一实现
 * - 支持全局对象生命周期管理
 * - 支持数据库主键管理和验证
 * 
 * 使用方式：
 * @code
 * class MyGlobalDBObj : public NFDBGlobalTemplate<MyGlobalDBObj, MyProtoClass, EOT_MY_GLOBAL_DB_OBJ> {
 * public:
 *     virtual int LoadFromDB(const MyProtoClass& dbData) override;
 *     virtual int SaveToDB(const MyProtoClass& dbData) override;
 * };
 * @endcode
 */
template<typename className, typename pbClass, int classType>
class NFDBGlobalTemplate : public NFObjectGlobalTemplate<className, classType, NFBaseDBObj>
{
public:
    /**
     * @brief 构造函数
     * 
     * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
     */
    NFDBGlobalTemplate()
    {
        if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化数据库全局对象模板
     * 
     * @return 初始化结果
     */
    int CreateInit()
    {
        return 0;
    }

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复数据库全局对象模板状态
     * 
     * @return 初始化结果
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 析构函数
     */
    virtual ~NFDBGlobalTemplate()
    {
    }

    /**
     * @brief 加载数据
     * 
     * 设置服务器类型并从数据库加载全局对象数据
     * 
     * @param serverType 服务器类型
     * @return 加载结果
     */
    virtual int Load(NF_SERVER_TYPE serverType)
    {
        NFBaseDBObj::SetServerType(serverType);
        return NFDBObjMgr::Instance()->LoadFromDB(this);
    }

    /**
     * @brief 制作加载数据
     * 
     * 将全局对象数据转换为Protobuf消息格式用于加载，设置主键
     * 
     * @param data 数据消息对象
     * @return 处理结果
     */
    virtual int MakeLoadData(google::protobuf::Message *data)
    {
        CHECK_NULL(0, data);
        pbClass *p = dynamic_cast<pbClass *>(data);
        CHECK_NULL(0, p);

        int iRetCode = NFProtobufCommon::SetPrimarykeyFromMessage(p, NFCommon::tostr(GetDbId()));
        CHECK_EXPR(iRetCode == 0, -1, "SetPrimarykeyFromMessage Failed From {}", data->GetTypeName());

        return 0;
    }

    /**
     * @brief 制作保存数据
     * 
     * 将全局对象数据转换为Protobuf消息格式用于保存，设置主键并保存到数据库
     * 
     * @param data 数据消息对象
     * @return 处理结果
     */
    virtual int MakeSaveData(google::protobuf::Message *data)
    {
        if (!this->m_bDataInited)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "not inited !");
            return -1;
        }
        CHECK_NULL(0, data);

        pbClass *p = dynamic_cast<pbClass *>(data);
        CHECK_NULL(0, p);

        int iRetCode = NFProtobufCommon::SetPrimarykeyFromMessage(p, NFCommon::tostr(GetDbId()));
        CHECK_EXPR(iRetCode == 0, -1, "SetPrimarykeyFromMessage Failed From {}", data->GetTypeName());
        return SaveToDB(*p);
    }

    /**
     * @brief 使用数据库数据初始化
     * 
     * 使用从数据库加载的数据初始化全局对象，验证主键并加载数据
     * 
     * @param pData 数据库数据
     * @return 初始化结果
     */
    virtual int InitWithDBData(const google::protobuf::Message *pData)
    {
        const pbClass *pGlobal = dynamic_cast<const pbClass *>(pData);
        CHECK_NULL(0, pGlobal);

        std::string dbId;
        int iRetCode = NFProtobufCommon::GetPrimarykeyFromMessage(pGlobal, dbId);
        CHECK_EXPR(iRetCode == 0, -1, "GetPrimarykeyFromMessage Failed From {}", pData->GetTypeName());
        CHECK_EXPR(NFCommon::strto<int>(dbId) == GetDbId(), -1, "invalid data!");
        int iRet = LoadFromDB(*pGlobal);
        CHECK_EXPR(iRet == 0, -1, "parse failed!");

        this->m_bDataInited = true;
        return 0;
    }

    /**
     * @brief 不使用数据库数据初始化
     * 
     * 在没有数据库数据的情况下初始化全局对象
     * 
     * @return 初始化结果
     */
    virtual int InitWithoutDBData()
    {
        int iRet = InitConfig();
        CHECK_EXPR(iRet == 0, -1, "InitConfig failed!");
        this->m_bDataInited = true;
        return 0;
    }

    /**
     * @brief 从数据库加载数据
     * 
     * 从Protobuf数据加载全局对象数据，需要子类实现
     * 
     * @param dbData 数据库数据
     * @return 加载结果
     */
    virtual int LoadFromDB(const pbClass &dbData) = 0;

    /**
     * @brief 保存到数据库
     * 
     * 将全局对象数据保存到数据库，需要子类实现
     * 
     * @param dbData 数据库数据
     * @return 保存结果
     */
    virtual int SaveToDB(pbClass &dbData) = 0;

    virtual int InitConfig() = 0;

    virtual int GetDbId() = 0;

    virtual google::protobuf::Message *CreateTempProtobufData() { return new pbClass(); }
    virtual uint32_t GetSaveDis() { return 60; }

    /**
     * @brief 处理失败策略
     * 
     * 获取初始化失败时的处理策略
     * 
     * @return 失败处理策略
     */
    virtual eDealWithLoadFailed DealWithFailed() { return EN_DW_SHUTDOWN; }
};


