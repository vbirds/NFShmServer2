// -------------------------------------------------------------------------
//    @FileName         :    NFTransBase.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTransBase.h
//
// -------------------------------------------------------------------------

/**
 * @file NFTransBase.h
 * @brief 事务处理基类定义文件
 * @details 定义了NFrame框架中事务处理的基类NFTransBase，提供了事务的生命周期管理、
 *          超时控制、运行次数限制、事务状态管理等功能。用于处理需要多步骤执行的
 *          复杂业务逻辑，如数据库操作、网络请求等异步处理场景。
 *
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 *
 * @note 该文件包含了：
 *       - NFTransBase: 事务处理基类
 *       - 事务超时控制相关常量定义
 *       - 事务状态管理
 *       - 事务运行次数控制
 *       - 事务完成状态检查
 *
 * @warning 使用事务时需要注意：
 *          - 事务有最大运行次数限制（MAX_TRANS_RUNED_TIMES）
 *          - 事务有活跃超时时间限制（TRANS_ACTIVE_TIMEOUT）
 *          - 必须正确调用SetFinished()来标记事务完成
 *          - 避免在事务中执行耗时过长的操作
 */

#pragma once

#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"

/**
 * @brief 事务相关错误码和常量定义
 * @details 定义了事务处理过程中使用的各种常量和错误码
 */
enum
{
    ERR_TRANS_RUNED_TOO_MUCH = 100057,  ///< 事务运行次数过多错误码
    MAX_TRANS_RUNED_TIMES = 10000,      ///< 事务最大运行次数
    TRANS_TIMEOUT_VALUE = 300,          ///< 事务超时值（秒）
#ifdef NF_DEBUG_MODE
    TRANS_ACTIVE_TIMEOUT = 200,         ///< DEBUG模式下事务每一步的最大生命周期（秒），方便调试
#else
    TRANS_ACTIVE_TIMEOUT = 10,          ///< 生产环境下事务每一步的最大生命周期（秒）
#endif
    SAVE_TRANS_ACTIVE_TIMEOUT = 10,     ///< 保存事务的活跃超时时间，与TRANS_ACTIVE_TIMEOUT最小值相同
    MAX_WAIT_TRANS_FINISH_TIME = TRANS_ACTIVE_TIMEOUT + 1,  ///< 等待事务完成的最大时间
};

#define MAX_WAIT_TRANS_FINISH_TIME ()

/**
 * @class NFTransBase
 * @brief 事务处理基类
 * @details 提供了完整的事务生命周期管理功能，包括事务的创建、执行、完成、超时处理等。
 *          事务是一种用于处理需要多个步骤或异步操作的业务逻辑单元，例如数据库操作、
 *          网络请求等。该类提供了事务状态管理、超时控制、运行次数限制等核心功能。
 *
 * @note 事务的主要特性：
 *       - 状态管理：跟踪事务的当前执行状态
 *       - 超时控制：防止事务无限期执行
 *       - 运行次数限制：避免无限循环
 *       - 异步处理：支持多步骤异步操作
 *       - 错误处理：提供完善的错误处理机制
 *
 * @warning 使用事务时的注意事项：
 *          - 必须在适当的时候调用SetFinished()标记事务完成
 *          - 避免在事务中执行耗时过长的同步操作
 *          - 注意处理事务超时情况
 *          - 确保事务的幂等性，避免重复执行产生副作用
 */
class NFTransBase : public NFObjectTemplate<NFTransBase, EOT_TRANS_BASE, NFObject>
{
public:
    /**
     * @brief 构造函数
     */
    NFTransBase();

    /**
     * @brief 析构函数
     */
    ~NFTransBase() override;

    /**
     * @brief 创建时初始化
     * @return int 初始化结果，0表示成功
     */
    int CreateInit();

    /**
     * @brief 恢复时初始化
     * @return int 初始化结果，0表示成功
     */
    int ResumeInit();

    /**
     * @brief 事务初始化
     * @details 在事务开始执行前调用，进行必要的初始化工作
     * @return int 初始化结果，0表示成功
     */
    int Init();

    /**
     * @brief 增加运行次数
     * @details 每次事务执行时调用，用于跟踪事务的运行次数
     */
    void IncreaseRunTimes();

    /**
     * @brief 设置事务完成状态
     * @details 标记事务已完成，设置返回码并更新完成状态
     * @param iRetCode 事务执行的返回码，0表示成功，非0表示失败
     */
    void SetFinished(int iRetCode);

    /**
     * @brief 检查事务是否已完成
     * @return bool true表示事务已完成，false表示还在执行中
     */
    bool IsFinished() const;

    /**
     * @brief 处理客户端到服务器的消息请求
     * @param pCsMsgReq 客户端消息请求指针
     * @return int 处理结果，0表示成功
     */
    int ProcessCSMsgReq(const google::protobuf::Message* pCsMsgReq);

    /**
     * @brief 处理分发服务器的响应消息
     * @param nMsgId 消息ID
     * @param packet 数据包
     * @param reqTransId 请求事务ID
     * @param rspTransId 响应事务ID
     * @return int 处理结果，0表示成功
     */
    int ProcessDispSvrRes(uint32_t nMsgId, const NFDataPackage& packet, uint32_t reqTransId, uint32_t rspTransId);

    /**
     * @brief 处理数据库消息响应
     * @param pSsMsgRes 数据库响应消息指针
     * @param cmd 命令类型
     * @param tableId 表ID
     * @param seq 序列号
     * @param errCode 错误码
     * @return int 处理结果，0表示成功
     */
    int ProcessDBMsgRes(const google::protobuf::Message* pSsMsgRes, uint32_t cmd, uint32_t tableId, uint32_t seq, int32_t errCode);

    /**
     * @brief 处理数据库消息响应（框架包版本）
     * @param svrPkg 服务器数据包
     * @return int 处理结果，0表示成功
     */
    int ProcessDBMsgRes(NFrame::Proto_FramePkg& svrPkg);

    /**
     * @brief 检查事务是否可以被释放
     * @details 判断事务是否已完成且满足释放条件
     * @return bool true表示可以释放，false表示还不能释放
     */
    bool IsCanRelease() const;

    /**
     * @brief 获取事务的调试信息字符串
     * @details 返回包含事务状态、运行次数等信息的调试字符串
     * @return std::string 调试信息字符串
     */
    virtual std::string DebugString() const;

    /**
     * @brief 定时检查函数，用于少数需要定时处理的trans（事务）场景。
     *
     * 该函数为虚函数，默认返回0，具体实现应由派生类根据需要进行重写。
     * 建议在具体的trans类中增加字段记录tick间隔，以便更好地控制定时检查的频率。
     *
     * @return int 返回值为0，表示默认的tick处理结果。派生类可以根据需要返回其他值。
     */
    virtual int Tick() { return 0; }

    /**
     * @brief 处理定时检查
     * @details 内部调用Tick()方法，并进行一些通用的定时处理逻辑
     * @return int 处理结果，0表示成功
     */
    int ProcessTick();

    /**
     * @brief 获取事务当前状态
     * @return uint16_t 事务状态值
     */
    uint16_t GetState() const { return m_wCurState; }

    /**
     * @brief 设置事务状态
     * @param wState 要设置的状态值
     */
    void SetState(uint16_t wState) { m_wCurState = wState; }

    /**
     * @brief 获取事务开始时间
     * @return uint32_t 事务开始的时间戳
     */
    uint32_t GetStartTime() const { return m_dwStartTime; }

    /**
     * @brief 获取事务活跃时间
     * @return uint32_t 事务最后活跃的时间戳
     */
    uint32_t GetActiveTime() const { return m_dwActiveTime; }

    /**
     * @brief 设置事务活跃时间
     * @param dwActiveTime 要设置的活跃时间戳
     */
    void SetActiveTime(uint32_t dwActiveTime) { m_dwActiveTime = dwActiveTime; }

    /**
     * @brief 获取活跃超时时间
     * @return int 活跃超时时间（秒）
     */
    int GetActiveTimeOut() const { return m_iActiveTimeOut; }

    /**
     * @brief 设置活跃超时时间
     * @param timeOut 超时时间（秒）
     */
    void SetActiveTimeOut(int timeOut) { m_iActiveTimeOut = timeOut; }

    /**
     * @brief 获取最大运行次数
     * @return uint32_t 最大运行次数
     */
    uint32_t GetMaxRunTimes() const { return m_dwMaxRunTimes; }

    /**
     * @brief 设置最大运行次数
     * @param dwMaxRunTimes 最大运行次数
     */
    void SetMaxRunTimes(uint32_t dwMaxRunTimes) { m_dwMaxRunTimes = dwMaxRunTimes; }

    /**
     * @brief 检查事务自身是否超时
     * @return bool true表示事务已超时
     */
    bool IsSelfTimeOut() const { return m_bIsSelfTransTimeout; }

    /**
     * @brief 处理事务超时
     * @details 当事务超时时调用此方法，派生类可以重写以实现自定义的超时处理逻辑
     * @return int 处理结果，0表示成功
     */
    virtual int OnTimeOut();

    /**
     * @brief 检查事务是否超时
     * @details 判断事务是否已经超过了允许的执行时间
     * @return bool true表示事务已超时
     */
    virtual bool IsTimeOut();

    /**
     * @brief 事务完成时的回调函数
     * @details 当事务完成时调用，派生类可以重写以实现自定义的完成处理逻辑
     * @param iRunLogicRetCode 事务执行的逻辑返回码
     * @return int 处理结果，0表示成功
     */
    virtual int OnTransFinished(int iRunLogicRetCode) { return 0; }

    /**
     * @brief 处理事务完成
     * @details 事务完成时的通用处理逻辑，会调用OnTransFinished()
     * @param iRunLogicRetCode 事务执行的逻辑返回码
     * @return int 处理结果，0表示成功
     */
    virtual int HandleTransFinished(int iRunLogicRetCode) { return 0; }

protected:
    /**
     * @brief 处理客户端到服务器的消息请求（实现方法）
     * @details 派生类需要重写此方法来处理具体的消息
     * @param pCsMsgReq 客户端消息请求指针
     * @return int 处理结果，0表示成功
     */
    virtual int HandleCSMsgReq(const google::protobuf::Message* pCsMsgReq);

    /**
     * @brief 处理分发服务器的响应消息（实现方法）
     * @details 派生类需要重写此方法来处理具体的响应
     * @param nMsgId 消息ID
     * @param packet 数据包
     * @param reqTransId 请求事务ID
     * @param rspTransId 响应事务ID
     * @return int 处理结果，0表示成功
     */
    virtual int HandleDispSvrRes(uint32_t nMsgId, const NFDataPackage& packet, uint32_t reqTransId, uint32_t rspTransId);

    /**
     * @brief 处理数据库消息响应（实现方法）
     * @details 派生类需要重写此方法来处理具体的数据库响应
     * @param pSsMsgRes 数据库响应消息指针
     * @param cmd 命令类型
     * @param tableId 表ID
     * @param seq 序列号
     * @param errCode 错误码
     * @return int 处理结果，0表示成功
     */
    virtual int HandleDBMsgRes(const google::protobuf::Message* pSsMsgRes, uint32_t cmd, uint32_t tableId, uint32_t seq, int32_t errCode);

protected:
    /**
     * @brief 检查事务是否可以立即运行
     * @details 检查事务的运行条件，如运行次数是否超限等
     * @return int 检查结果，0表示可以运行
     */
    int CheckCanRunNow() const;

    /**
     * @brief 运行通用逻辑
     * @details 执行事务的通用处理逻辑
     * @return int 执行结果，0表示成功
     */
    int RunCommLogic();

protected:
    /**
     * @brief 该结构体用于管理事务（trans）的状态和相关信息。
     *
     * 该结构体包含多个成员变量，用于记录事务的启动时间、活跃时间、当前状态、运行次数、最大运行次数、运行逻辑返回值、RPC ID、事务是否超时以及超时设置等信息。
     *
     * 成员变量说明：
     * - m_bIsFinished: 标识事务是否已完成。
     * - m_dwStartTime: 记录事务的启动时间。
     * - m_dwActiveTime: 记录事务的活跃时间。
     * - m_wCurState: 记录事务的当前状态。
     * - m_wRunedTimes: 记录事务已经运行的次数。
     * - m_dwMaxRunTimes: 记录事务的最大允许运行次数。
     * - m_iRunLogicRetCode: 记录事务运行逻辑的返回值。
     * - m_rpcId: 记录事务的RPC ID。
     * - m_bIsSelfTransTimeout: 标识事务自身是否超时。由于外部可能会设置m_iRunLogicRetCode，因此使用该变量来确保超时判断的准确性。
     * - m_iActiveTimeOut: 设置事务的超时时间。
     */
    bool m_bIsFinished;         ///< 标识事务是否已完成
    uint32_t m_dwStartTime;     ///< 记录事务的启动时间戳
    uint32_t m_dwActiveTime;    ///< 记录事务的活跃时间戳
    uint16_t m_wCurState;       ///< 记录事务的当前状态
    uint16_t m_wRunedTimes;     ///< 记录事务已经运行的次数
    uint32_t m_dwMaxRunTimes;   ///< 记录事务的最大允许运行次数
    int m_iRunLogicRetCode;     ///< 记录事务运行逻辑的返回值
    int64_t m_rpcId;            ///< 记录事务的RPC ID

    /**
     * @brief 标识事务自身是否超时。
     *
     * 由于外部可能会设置m_iRunLogicRetCode，因此使用该变量来确保超时判断的准确性。
     */
    bool m_bIsSelfTransTimeout; ///< 标识事务自身是否已超时

    /**
     * @brief 设置事务的超时时间。
     */
    int m_iActiveTimeOut;       ///< 事务的活跃超时时间（秒）
};

/**
 * @brief 检查错误码并完成事务的宏定义
 * @details 如果返回码不为0，则设置事务完成状态并记录错误日志
 * @param iRetCode 要检查的返回码
 * @param pTrans 事务对象指针
 * @param format 日志格式字符串
 * @param ... 日志参数
 */
#define CHECK_ERR_AND_FIN_TRANS(iRetCode, pTrans, format, ...)\
    if( iRetCode != 0 )\
    {\
        pTrans->SetFinished( iRetCode );\
        std::stringstream ss;\
        ss << format;\
        std::string log_event = NF_FORMAT(ss.str().c_str(), ##__VA_ARGS__);\
        NFLogError(NF_LOG_DEFAULT, 0, "CHCK TRANS RetCode:{} failed:{}", iRetCode, log_event);\
        return iRetCode;\
    }

/**
 * @brief 检查表达式并完成事务的宏定义
 * @details 如果表达式为假，则设置事务完成状态并记录错误日志
 * @param expr 要检查的表达式
 * @param iRetCode 失败时的返回码
 * @param pTrans 事务对象指针
 * @param format 日志格式字符串
 * @param ... 日志参数
 */
#define CHECK_EXPR_AND_FIN_TRANS(expr, iRetCode, pTrans, format, ...)\
    if( !(expr) )\
    {\
        pTrans->SetFinished( iRetCode );\
        std::stringstream ss;\
        ss << format;\
        std::string log_event = NF_FORMAT(ss.str().c_str(), ##__VA_ARGS__);\
        NFLogError(NF_LOG_DEFAULT, 0, "CHECK Trans:{} RetCode:{} failed:{}", #expr, iRetCode, log_event);\
        return iRetCode;\
    }
