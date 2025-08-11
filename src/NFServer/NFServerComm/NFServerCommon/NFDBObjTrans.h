// -------------------------------------------------------------------------
//    @FileName         :    NFDBObjTrans.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDBObjTrans.h
//    @Desc             :    数据库对象事务头文件，提供数据库对象的事务处理功能。
//                          该文件定义了数据库对象事务类，包括事务初始化、数据操作、
//                          事务超时处理、事务完成处理。
//                          主要功能包括提供数据库对象的事务处理、支持数据插入和保存、
//                          支持数据加载、提供事务超时和完成处理。
//                          数据库对象事务是NFShmXFrame框架的数据库事务处理组件，负责：
//                          - 数据库对象的事务处理
//                          - 数据插入、保存、加载操作
//                          - 事务超时和完成处理
//                          - 事务状态管理和生命周期
//                          - 数据库操作的原子性保证
//                          - 跨服务器事务同步
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFTransBase.h"

/**
 * @brief 数据库对象事务类
 * 
 * 该类是数据库对象的事务处理类，提供了：
 * - 数据库对象的事务处理
 * - 数据插入、保存、加载操作
 * - 事务超时和完成处理
 * - 事务状态管理和生命周期
 * 
 * 主要功能：
 * - 提供数据库对象的事务处理
 * - 支持数据插入、保存、加载操作
 * - 提供事务超时和完成处理
 * - 支持事务状态管理和生命周期
 * - 支持数据库操作的原子性保证
 * 
 * 使用方式：
 * @code
 * NFDBObjTrans* pTrans = NFDBObjTrans::CreateObj();
 * pTrans->Init(serverType, objId, seqOP);
 * pTrans->Insert(modKey, data);
 * @endcode
 */
class NFDBObjTrans : public NFObjectTemplate<NFDBObjTrans, EOT_TRANS_DB_OBJ, NFTransBase>
{
public:
    /**
     * @brief 构造函数
     */
    NFDBObjTrans();

    /**
     * @brief 析构函数
     */
    virtual ~NFDBObjTrans();

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化数据库对象事务
     * 
     * @return 初始化结果
     */
    //非继承函数, 不要加virtual
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复数据库对象事务状态
     * 
     * @return 初始化结果
     */
    //非继承函数, 不要加virtual
    int ResumeInit();
public:
    /**
     * @brief 初始化事务
     * 
     * 初始化数据库对象事务，设置服务器类型、对象ID和序列操作
     * 
     * @param eType 服务器类型
     * @param iObjID 对象ID
     * @param iSeqOP 序列操作
     * @return 初始化结果
     */
    int Init(NF_SERVER_TYPE eType, int iObjID, uint32_t iSeqOP);

    /**
     * @brief 插入数据
     * 
     * 向数据库插入数据
     * 
     * @param iModKey 模块键值
     * @param data 数据对象
     * @return 插入结果
     */
    int Insert(uint64_t iModKey, google::protobuf::Message* data);

    /**
     * @brief 保存数据
     * 
     * 保存数据到数据库
     * 
     * @param iModKey 模块键值
     * @param data 数据对象
     * @return 保存结果
     */
    int Save(uint64_t iModKey, google::protobuf::Message* data);

    /**
     * @brief 加载数据
     * 
     * 从数据库加载数据
     * 
     * @param iModKey 模块键值
     * @param data 数据对象
     * @return 加载结果
     */
    int Load(uint64_t iModKey, google::protobuf::Message* data);

    /**
     * @brief 事务超时处理
     * 
     * 当事务超时时调用，处理超时逻辑
     * 
     * @return 处理结果
     */
    virtual int OnTimeOut();

    /**
     * @brief 处理事务完成
     * 
     * 当事务完成时调用，处理完成逻辑
     * 
     * @param iRunLogicRetCode 运行逻辑返回码
     * @return 处理结果
     */
    virtual int HandleTransFinished(int iRunLogicRetCode);

    /**
     * @brief 获取关联对象ID
     * 
     * 获取事务关联的数据库对象ID
     * 
     * @return 关联对象ID
     */
    int GetLinkedObjID() { return m_iLinkedObjID; }

    /**
     * @brief 获取对象序列操作
     * 
     * 获取对象的序列操作类型
     * 
     * @return 序列操作
     */
    uint32_t GetObjSeqOP() { return m_iObjSeqOP; }
private:
    int m_iLinkedObjID; ///< 关联对象ID
    uint32_t m_iObjSeqOP; ///< 对象序列操作
    int m_iDBOP; ///< 数据库操作类型
    NF_SERVER_TYPE m_iServerType; ///< 服务器类型
};