// -------------------------------------------------------------------------
//    @FileName         :    NFServerSyncDataTrans.h
//    @Author           :    gaoyi
//    @Date             :    2025-07-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerSyncDataTrans
//    @Desc             :    服务器同步数据事务头文件，提供服务器间数据同步的事务处理功能。
//                          该文件定义了服务器同步数据事务类，包括事务初始化、
//                          同步请求处理、响应处理、事务完成处理。
//                          主要功能包括提供服务器间数据同步的事务处理、支持事务初始化、
//                          支持同步请求和响应处理、提供事务完成处理。
//                          服务器同步数据事务是NFShmXFrame框架的服务器同步事务组件，负责：
//                          - 服务器间数据同步的事务处理
//                          - 事务初始化和状态管理
//                          - 同步请求和响应处理
//                          - 事务完成和错误处理
//                          - 跨服务器事务通信
//                          - 数据一致性保证
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFTransBase.h"

/**
 * @brief 服务器同步数据事务类
 * 
 * 该类是服务器间数据同步的事务处理类，提供了：
 * - 事务初始化和状态管理
 * - 同步请求和响应处理
 * - 事务完成和错误处理
 * - 跨服务器事务通信
 * 
 * 主要功能：
 * - 提供服务器间数据同步的事务处理
 * - 支持事务初始化和状态管理
 * - 支持同步请求和响应处理
 * - 提供事务完成和错误处理
 * - 支持跨服务器事务通信
 * - 支持数据一致性保证
 * 
 * 使用方式：
 * @code
 * NFServerSyncDataTrans* pTrans = NFServerSyncDataTrans::CreateObj();
 * pTrans->Init(serverType, dstType, dstBusId, moduleId, msgId, objId);
 * pTrans->SyncReq(data);
 * @endcode
 */
class NFServerSyncDataTrans : public NFObjectTemplate<NFServerSyncDataTrans, EOT_TRANS_SERVER_SYNC_DATA_OBJ, NFTransBase>
{
public:
    /**
     * @brief 构造函数
     */
    NFServerSyncDataTrans();

    /**
     * @brief 析构函数
     */
    ~NFServerSyncDataTrans() override;

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化服务器同步数据事务
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复服务器同步数据事务状态
     * 
     * @return 初始化结果
     */
    int ResumeInit();
public:
    /**
     * @brief 初始化事务
     * 
     * 初始化服务器同步数据事务，设置相关参数
     * 
     * @param type 服务器类型
     * @param dstType 目标服务器类型
     * @param dstBusId 目标总线ID
     * @param moduleId 模块ID
     * @param msgId 消息ID
     * @param iObjID 对象ID
     * @return 初始化结果
     */
    int Init(NF_SERVER_TYPE type, NF_SERVER_TYPE dstType, uint32_t dstBusId, uint32_t moduleId, uint32_t msgId, int iObjID);

    /**
     * @brief 同步请求
     * 
     * 发送数据同步请求
     * 
     * @param pData 数据对象
     * @return 请求结果
     */
    int SyncReq(google::protobuf::Message* pData);
public:
    /**
     * @brief 处理分发服务器响应
     * 
     * 处理来自分发服务器的响应消息
     * 
     * @param nMsgId 消息ID
     * @param packet 数据包
     * @param reqTransId 请求事务ID
     * @param rspTransId 响应事务ID
     * @return 处理结果
     */
    int HandleDispSvrRes(uint32_t nMsgId, const NFDataPackage& packet, uint32_t reqTransId, uint32_t rspTransId) override;

    /**
     * @brief 处理事务完成
     * 
     * 当事务完成时调用，处理完成逻辑
     * 
     * @param iRunLogicRetCode 运行逻辑返回码
     * @return 处理结果
     */
    int HandleTransFinished(int iRunLogicRetCode) override;
private:
    int m_iLinkedObjID; ///< 关联对象ID
    NF_SERVER_TYPE m_iServerType; ///< 服务器类型
    NF_SERVER_TYPE m_iDstServerType; ///< 目标服务器类型
    uint32_t m_iDstBusId; ///< 目标总线ID
    uint32_t m_iModuleId; ///< 模块ID
    uint32_t m_iMsgId; ///< 消息ID
};
