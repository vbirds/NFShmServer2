// -------------------------------------------------------------------------
//    @FileName         :    NFBaseDBObj.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFBaseDBObj.h
//    @Desc             :    基础数据库对象头文件，提供数据库对象的基础功能。
//                          该文件定义了基础数据库对象类，包括数据加载接口、数据保存接口、
//                          重试机制、事务处理接口。
//                          主要功能包括提供数据库对象的基础功能、支持数据加载和保存、
//                          支持重试机制和错误处理、提供事务处理接口。
//                          基础数据库对象是NFShmXFrame框架的数据库操作基础组件，负责：
//                          - 数据库对象的基础功能实现
//                          - 数据加载和保存接口
//                          - 重试机制和错误处理
//                          - 事务处理和状态管理
//                          - 数据库操作的生命周期管理
//                          - 跨服务器数据库同步
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFSeqOP.h"

#define MAX_FAIL_RETRY_TIMES 100  ///< 最大失败重试次数
#define MAX_SAVED_OBJ_PRE_SEC 100 ///< 每秒最大保存对象数量

/**
 * @brief 对象初始化失败后的操作枚举
 * 
 * 定义了当数据库对象初始化失败时的处理策略
 */
enum eDealWithLoadFailed
{
    EN_DW_LOG_FAIL  = 1, ///< 记录错误日志
    EN_DW_RETRY     = 2, ///< 反复重试
    EN_DW_SHUTDOWN  = 3, ///< 立即停止服务器
    EN_DW_RETRY_ANY_SHUTDOWN = 4, ///< 重试一定次数后如果不能成功，关闭服务器
};

/**
 * @brief 基础数据库对象类
 * 
 * 该类是数据库对象的基础类，提供了：
 * - 数据加载和保存功能
 * - 重试机制和错误处理
 * - 事务处理和状态管理
 * - 数据库操作的生命周期管理
 * 
 * 主要功能：
 * - 提供数据库对象的基础功能
 * - 支持数据加载和保存接口
 * - 支持重试机制和错误处理
 * - 提供事务处理和状态管理
 * - 支持跨服务器数据库同步
 * 
 * 使用方式：
 * @code
 * class MyDBObj : public NFBaseDBObj {
 * public:
 *     virtual int MakeLoadData(google::protobuf::Message* data) override;
 *     virtual int MakeSaveData(google::protobuf::Message* data) override;
 *     virtual int InitWithDBData(const google::protobuf::Message* pData) override;
 * };
 * @endcode
 */
class NFBaseDBObj : public NFObjectTemplate<NFBaseDBObj, EOT_BASE_DB_OBJ, NFObject>, public NFSeqOP
{
public:
    /**
     * @brief 构造函数
     */
    NFBaseDBObj();

    /**
     * @brief 析构函数
     */
    virtual ~NFBaseDBObj();

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化数据库对象
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复数据库对象状态
     * 
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 定时器回调
     * 
     * 处理定时器事件，包括重试延迟等
     * 
     * @param timeId 定时器ID
     * @param callCount 调用次数
     * @return 处理结果
     */
    int OnTimer(int timeId, int callCount) override;
public:
    /**
     * @brief 制作加载数据
     * 
     * 将对象数据转换为Protobuf消息格式用于加载
     * 
     * @param data 数据消息对象
     * @return 处理结果
     */
    virtual int MakeLoadData(google::protobuf::Message* data) { return -1; }

    /**
     * @brief 制作保存数据
     * 
     * 将对象数据转换为Protobuf消息格式用于保存
     * 
     * @param data 数据消息对象
     * @return 处理结果
     */
    virtual int MakeSaveData(google::protobuf::Message* data) { return -1; }

    /**
     * @brief 使用数据库数据初始化
     * 
     * 使用从数据库加载的数据初始化对象
     * 
     * @param pData 数据库数据
     * @return 初始化结果
     */
    virtual int InitWithDBData(const google::protobuf::Message* pData) { return -1; }

    /**
     * @brief 不使用数据库数据初始化
     * 
     * 在没有数据库数据的情况下初始化对象
     * 
     * @return 初始化结果
     */
    virtual int InitWithoutDBData() { return -1; }

    /**
     * @brief 检查所有数据加载完成
     * 
     * 在所有数据加载完成后进行检查
     * 
     * @return 检查结果
     */
    virtual int CheckWhenAllDataLoaded() { return 0; }

    /**
     * @brief 创建临时Protobuf数据
     * 
     * 创建用于临时存储的Protobuf消息对象
     * 
     * @return Protobuf消息对象指针
     */
    virtual google::protobuf::Message* CreateTempProtobufData() { return nullptr; }

    /**
     * @brief 获取保存间隔
     * 
     * 获取数据保存的时间间隔（秒）
     * 
     * @return 保存间隔
     */
    virtual uint32_t GetSaveDis() { return 300; }

    /**
     * @brief 获取重试间隔
     * 
     * 获取重试操作的时间间隔（秒）
     * 
     * @return 重试间隔
     */
    virtual uint32_t GetRetryDis() { return 10; }

    /**
     * @brief 处理失败策略
     * 
     * 获取初始化失败时的处理策略
     * 
     * @return 失败处理策略
     */
    virtual eDealWithLoadFailed DealWithFailed() { return EN_DW_RETRY; }

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
     * 执行延迟同步请求重试操作
     * 
     * @return 重试结果
     */
    virtual int DelaySyncReqRetry();
public:
    /**
     * @brief 设置模块键
     * @param mod_key 模块键值
     */
    void SetModeKey(uint64_t mod_key) { m_uModKey = mod_key; }

    /**
     * @brief 获取模块键
     * @return 模块键值
     */
    uint64_t GetModeKey() { return m_uModKey; }

    /**
     * @brief 检查数据是否已初始化
     * @return 是否已初始化
     */
    bool IsDataInited() { return m_bDataInited; }

    /**
     * @brief 设置最后数据库操作时间
     * @param t 时间戳
     */
    void SetLastDBOpTime(uint64_t t) { m_iLastDBOpTime = t; }

    /**
     * @brief 获取最后数据库操作时间
     * @return 时间戳
     */
    uint64_t GetLastDBOpTime() const { return m_iLastDBOpTime; }

    /**
     * @brief 设置事务ID
     * @param transId 事务ID
     */
    void SetTransID(int transId) { m_iTransID = transId; }

    /**
     * @brief 获取事务ID
     * @return 事务ID
     */
    int GetTransID() const { return m_iTransID; }

    /**
     * @brief 设置重试次数
     * @param times 重试次数
     */
    void SetRetryTimes(int times) { m_iRetryTimes = times; }

    /**
     * @brief 获取重试次数
     * @return 重试次数
     */
    int GetRetryTimes() const { return m_iRetryTimes; }

    /**
     * @brief 设置是否需要插入数据库
     * @param b 是否需要插入
     */
    void SetNeedInsertDB(bool b) { m_bNeedInsertDB = b; }

    /**
     * @brief 获取是否需要插入数据库
     * @return 是否需要插入
     */
    bool GetNeedInsertDB() const { return m_bNeedInsertDB; }

    /**
     * @brief 设置服务器类型
     * @param type 服务器类型
     */
    void SetServerType(NF_SERVER_TYPE type) { m_iServerType = type; }

    /**
     * @brief 获取服务器类型
     * @return 服务器类型
     */
    NF_SERVER_TYPE GetServerType() const { return m_iServerType; }
protected:
    bool m_bDataInited; ///< 数据是否已初始化
protected:
    int m_iTransID; ///< 事务ID
	uint64_t m_iLastDBOpTime; ///< 最后数据库操作时间
    uint64_t m_uModKey; ///< 模块键值
    int      m_iRetryTimes; ///< 重试次数
    bool     m_bNeedInsertDB; ///< 是否需要插入数据库
    NF_SERVER_TYPE m_iServerType; ///< 服务器类型
protected:
    int m_iRetryDelayTimer; ///< 重试延迟定时器
};