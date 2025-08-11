// -------------------------------------------------------------------------
//    @FileName         :    NFServerSyncDataObj.h
//    @Author           :    gaoyi
//    @Date             :    2025-07-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerSyncDataObj
//    @Desc             :    服务器同步数据对象头文件，提供服务器间数据同步的对象功能。
//                          该文件定义了服务器同步数据对象类，包括同步状态管理、
//                          重试机制、数据转换、同步请求处理。
//                          主要功能包括提供服务器间数据同步的对象功能、支持同步状态管理、
//                          支持重试机制和错误处理、提供数据转换功能。
//                          服务器同步数据对象是NFShmXFrame框架的服务器同步组件，负责：
//                          - 服务器间数据同步的对象功能
//                          - 同步状态管理和生命周期
//                          - 重试机制和错误处理
//                          - 数据转换和消息处理
//                          - 同步进度监控
//                          - 跨服务器数据一致性
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFSeqOP.h"

/**
 * @brief 同步失败处理策略枚举
 * 
 * 定义了对象初始化失败后的处理策略
 */
enum eDealWithSyncFailed
{
    EN_DW_SYNC_LOG_FAIL  = 1, ///< 记录错误
    EN_DW_SYNC_RETRY     = 2, ///< 反复重试
    EN_DW_SYNC_SHUTDOWN  = 3, ///< 立即停止服务器
    EN_DW_SYNC_RETRY_ANY_SHUTDOWN = 4, ///< 重试一定次数后如果不能成功，关闭服务器
};

#define MAX_FAIL_RETRY_TIMES 100 ///< 最大失败重试次数
#define MAX_SAVED_OBJ_PRE_SEC 100 ///< 每秒最大保存对象数量

/**
 * @brief 服务器同步数据对象类
 * 
 * 该类是服务器间数据同步的对象，提供了：
 * - 同步状态管理和生命周期
 * - 重试机制和错误处理
 * - 数据转换和消息处理
 * - 同步进度监控
 * 
 * 主要功能：
 * - 提供服务器间数据同步的对象功能
 * - 支持同步状态管理和生命周期
 * - 支持重试机制和错误处理
 * - 提供数据转换和消息处理
 * - 支持同步进度监控
 * - 支持跨服务器数据一致性
 * 
 * 使用方式：
 * @code
 * class MySyncDataObj : public NFServerSyncDataObj {
 * public:
 *     virtual google::protobuf::Message* CreateTempProtobufData() override;
 *     virtual int MakeSyncData(google::protobuf::Message* data) override;
 *     virtual int InitWithSyncData(const google::protobuf::Message* pData) override;
 * };
 * @endcode
 */
class NFServerSyncDataObj : public NFObjectTemplate<NFServerSyncDataObj, EOT_SERVER_SYNC_DATA_OBJ, NFObject>
{
public:
    /**
     * @brief 构造函数
     */
    NFServerSyncDataObj();

    /**
     * @brief 析构函数
     */
    ~NFServerSyncDataObj() override;

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化服务器同步数据对象
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复服务器同步数据对象状态
     * 
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 定时器回调
     * 
     * 处理定时器事件
     * 
     * @param timeId 定时器ID
     * @param callCount 调用次数
     * @return 处理结果
     */
    int OnTimer(int timeId, int callCount) override;
public:
    /**
     * @brief 检查数据是否已初始化
     * 
     * @return 数据是否已初始化
     */
    bool IsDataInited() { return m_bDataInited; }

    /**
     * @brief 设置最后同步操作时间
     * 
     * @param t 时间戳
     */
    void SetLastSyncOpTime(uint64_t t) { m_iLastSyncOpTime = t; }

    /**
     * @brief 获取最后同步操作时间
     * 
     * @return 最后同步操作时间
     */
    uint64_t GetLastSyncOpTime() const { return m_iLastSyncOpTime; }

    /**
     * @brief 设置事务ID
     * 
     * @param transId 事务ID
     */
    void SetTransID(int transId) { m_iTransID = transId; }

    /**
     * @brief 获取事务ID
     * 
     * @return 事务ID
     */
    int GetTransID() const { return m_iTransID; }

    /**
     * @brief 设置重试次数
     * 
     * @param times 重试次数
     */
    void SetRetryTimes(int times) { m_iRetryTimes = times; }

    /**
     * @brief 获取重试次数
     * 
     * @return 重试次数
     */
    int GetRetryTimes() const { return m_iRetryTimes; }

    /**
     * @brief 设置服务器类型
     * 
     * @param type 服务器类型
     */
    void SetServerType(NF_SERVER_TYPE type) { m_iServerType = type; }

    /**
     * @brief 获取服务器类型
     * 
     * @return 服务器类型
     */
    NF_SERVER_TYPE GetServerType() const { return m_iServerType; }

    /**
     * @brief 设置目标服务器类型
     * 
     * @param type 目标服务器类型
     */
    void SetDstServerType(NF_SERVER_TYPE type) { m_iDstServerType = type; }

    /**
     * @brief 获取目标服务器类型
     * 
     * @return 目标服务器类型
     */
    NF_SERVER_TYPE GetDstServerType() const { return m_iDstServerType; }

    /**
     * @brief 设置目标总线ID
     * 
     * @param id 目标总线ID
     */
    void SetDstBusId(uint32_t id) { m_iDstBusId = id; }

    /**
     * @brief 获取目标总线ID
     * 
     * @return 目标总线ID
     */
    uint32_t GetDstBusId() const { return m_iDstBusId; }

    /**
     * @brief 设置模块ID
     * 
     * @param moduleId 模块ID
     */
    void SetModuleId(uint32_t moduleId) { m_iModuleId = moduleId; }

    /**
     * @brief 获取模块ID
     * 
     * @return 模块ID
     */
    uint32_t GetModuleId() { return m_iModuleId; }

    /**
     * @brief 设置消息ID
     * 
     * @param id 消息ID
     */
    void SetMsgId(uint32_t id) { m_iMsgId = id; }

    /**
     * @brief 获取消息ID
     * 
     * @return 消息ID
     */
    uint32_t GetMsgId() const { return m_iMsgId; }
public:
    /**
     * @brief 创建临时Protobuf数据
     * 
     * 创建用于同步的临时Protobuf消息对象
     * 
     * @return Protobuf消息对象指针
     */
    virtual google::protobuf::Message* CreateTempProtobufData() { return nullptr; }

    /**
     * @brief 获取同步间隔
     * 
     * 获取数据同步的时间间隔（秒）
     * 
     * @return 同步间隔
     */
    virtual uint32_t GetSyncDis() { return 300; }

    /**
     * @brief 获取重试间隔
     * 
     * 获取重试的时间间隔（秒）
     * 
     * @return 重试间隔
     */
    virtual uint32_t GetRetryDis() { return 1; }

    /**
     * @brief 处理失败策略
     * 
     * 获取同步失败时的处理策略
     * 
     * @return 失败处理策略
     */
    virtual eDealWithSyncFailed DealWithFailed();

    /**
     * @brief 同步请求重试
     * 
     * 执行同步请求重试操作
     * 
     * @return 重试结果
     */
    virtual int SyncReqRetry();

    /**
     * @brief 延迟同步请求重试
     * 
     * 延迟执行同步请求重试操作
     * 
     * @return 重试结果
     */
    virtual int DelaySyncReqRetry();
public:
    /**
     * @brief 制作同步数据
     * 
     * 将对象数据转换为同步数据格式
     * 
     * @param data 数据对象
     * @return 处理结果
     */
    virtual int MakeSyncData(google::protobuf::Message* data) { return 0; }

    /**
     * @brief 使用同步数据初始化
     * 
     * 使用从同步数据初始化对象
     * 
     * @param pData 同步数据
     * @return 初始化结果
     */
    virtual int InitWithSyncData(const google::protobuf::Message* pData) { return 0; }

    /**
     * @brief 定时器同步数据处理
     * 
     * 在定时器中处理同步数据
     * 
     * @param pData 同步数据
     * @return 处理结果
     */
    virtual int TimerWithSyncData(const google::protobuf::Message* pData) { return 0; }

    /**
     * @brief 检查所有数据加载完成
     * 
     * 检查所有同步数据是否已经加载完成
     * 
     * @return 检查结果
     */
    virtual int CheckWhenAllDataLoaded() { return 0; }
protected:
    bool m_bDataInited; ///< 数据是否已初始化
protected:
    int m_iTransID; ///< 事务ID
    uint64_t m_iLastSyncOpTime; ///< 最后同步操作时间
    int      m_iRetryTimes; ///< 重试次数
    NF_SERVER_TYPE m_iServerType; ///< 服务器类型
    NF_SERVER_TYPE m_iDstServerType; ///< 目标服务器类型
    uint32_t m_iDstBusId; ///< 目标总线ID
    uint32_t m_iModuleId; ///< 模块ID
    uint32_t m_iMsgId; ///< 消息ID
protected:
    int m_iRetryDelayTimer; ///< 重试延迟定时器
};
