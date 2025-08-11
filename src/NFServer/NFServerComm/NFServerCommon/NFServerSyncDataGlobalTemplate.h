// -------------------------------------------------------------------------
//    @FileName         :    NFServerSyncDataGlobalTemplate.h
//    @Author           :    gaoyi
//    @Date             :    2025-07-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerSyncDataGlobalTemplate
//    @Desc             :    服务器同步数据全局模板头文件，提供服务器间数据同步的全局模板实现。
//                          该文件定义了服务器同步数据全局模板类，包括全局同步对象管理、
//                          数据同步请求、Protobuf数据转换、同步状态管理。
//                          主要功能包括提供服务器间数据同步的全局模板实现、支持全局同步对象管理、
//                          支持数据同步请求和Protobuf转换、提供同步状态管理。
//                          服务器同步数据全局模板是NFShmXFrame框架的服务器同步全局组件，负责：
//                          - 服务器间数据同步的全局模板实现
//                          - 全局同步对象管理和生命周期
//                          - 数据同步请求和Protobuf转换
//                          - 同步状态管理和错误处理
//                          - 跨服务器全局数据同步
//                          - 数据一致性保证
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFCore/NFCommon.h>
#include <NFComm/NFPluginModule/NFStoreProtoCommon.h>

#include "NFServerSyncDataObj.h"
#include "NFServerSyncDataObjMgr.h"

/**
 * @brief 服务器同步数据全局模板类
 * 
 * 该类是服务器间数据同步的全局模板实现，提供了：
 * - 全局同步对象管理和生命周期
 * - 数据同步请求和Protobuf转换
 * - 同步状态管理和错误处理
 * - 跨服务器全局数据同步
 * 
 * @tparam className 类名
 * @tparam pbClass Protobuf类名
 * @tparam classType 类类型
 * 
 * 主要功能：
 * - 提供服务器间数据同步的全局模板实现
 * - 支持全局同步对象管理和生命周期
 * - 支持数据同步请求和Protobuf转换
 * - 提供同步状态管理和错误处理
 * - 支持跨服务器全局数据同步
 * - 支持数据一致性保证
 * 
 * 使用方式：
 * @code
 * class MyGlobalSyncData : public NFServerSyncDataGlobalTemplate<MyGlobalSyncData, MyProtoClass, EOT_MY_GLOBAL_SYNC_DATA> {
 * public:
 *     virtual int SyncData(const MyProtoClass& data) override;
 * };
 * @endcode
 */
template<typename className, typename pbClass, int classType>
class NFServerSyncDataGlobalTemplate : public NFObjectGlobalTemplate<className, classType, NFServerSyncDataObj>
{
public:
    /**
     * @brief 构造函数
     * 
     * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
     */
    NFServerSyncDataGlobalTemplate()
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
     * 在对象创建时调用，初始化服务器同步数据全局模板
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
     * 在对象恢复时调用，恢复服务器同步数据全局模板状态
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
    ~NFServerSyncDataGlobalTemplate() override
    {
    }

    /**
     * @brief 同步请求（完整版本）
     * 
     * 发送数据同步请求，包含所有参数
     * 
     * @param serverType 服务器类型
     * @param dstServerType 目标服务器类型
     * @param dstBusId 目标总线ID
     * @param moduleId 模块ID
     * @param msgId 消息ID
     * @return 请求结果
     */
    virtual int SyncReq(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, uint32_t moduleId, uint32_t msgId)
    {
        NFServerSyncDataObj::SetServerType(serverType);
        NFServerSyncDataObj::SetDstServerType(dstServerType);
        NFServerSyncDataObj::SetDstBusId(dstBusId);
        NFServerSyncDataObj::SetMsgId(msgId);
        NFServerSyncDataObj::SetModuleId(moduleId);
        return NFServerSyncDataObjMgr::Instance()->SyncReq(this);
    }

    /**
     * @brief 同步请求（简化版本）
     * 
     * 发送数据同步请求，使用默认模块ID
     * 
     * @param serverType 服务器类型
     * @param dstServerType 目标服务器类型
     * @param dstBusId 目标总线ID
     * @param msgId 消息ID
     * @return 请求结果
     */
    virtual int SyncReq(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, uint32_t msgId)
    {
        NFServerSyncDataObj::SetServerType(serverType);
        NFServerSyncDataObj::SetDstServerType(dstServerType);
        NFServerSyncDataObj::SetDstBusId(dstBusId);
        NFServerSyncDataObj::SetMsgId(msgId);
        NFServerSyncDataObj::SetModuleId(NF_MODULE_SERVER);
        return NFServerSyncDataObjMgr::Instance()->SyncReq(this);
    }

    /**
     * @brief 制作同步数据
     * 
     * 将对象数据转换为同步数据格式
     * 
     * @param data 数据对象
     * @return 处理结果
     */
    int MakeSyncData(google::protobuf::Message *data) override
    {
        CHECK_NULL(0, data);
        pbClass *p = dynamic_cast<pbClass *>(data);
        CHECK_NULL(0, p);

        return 0;
    }

    /**
     * @brief 使用同步数据初始化
     * 
     * 使用从同步数据初始化对象
     * 
     * @param pData 同步数据
     * @return 初始化结果
     */
    int InitWithSyncData(const google::protobuf::Message *pData) override
    {
        const pbClass *pGlobal = dynamic_cast<const pbClass *>(pData);
        CHECK_NULL(0, pGlobal);

        int iRet = SyncData(*pGlobal);
        CHECK_EXPR(iRet == 0, -1, "SyncData failed!");

        this->m_bDataInited = true;
        return 0;
    }

    /**
     * @brief 定时器同步数据处理
     * 
     * 在定时器中处理同步数据
     * 
     * @param pData 同步数据
     * @return 处理结果
     */
    int TimerWithSyncData(const google::protobuf::Message* pData) override
    {
        const pbClass *pGlobal = dynamic_cast<const pbClass *>(pData);
        CHECK_NULL(0, pGlobal);

        int iRet = SyncData(*pGlobal);
        CHECK_EXPR(iRet == 0, -1, "SyncData failed!");

        return 0;
    }

    /**
     * @brief 同步数据
     * 
     * 处理同步数据，需要子类实现
     * 
     * @param dbData 同步数据
     * @return 处理结果
     */
    virtual int SyncData(const pbClass &dbData) = 0;

    virtual google::protobuf::Message *CreateTempProtobufData() { return new pbClass(); }
};

