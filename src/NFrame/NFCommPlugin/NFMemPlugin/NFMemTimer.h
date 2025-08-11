// -------------------------------------------------------------------------
//    @FileName         :    NFMemTimer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMemTimer.h
//    @Desc             :    内存定时器头文件，提供内存定时器的创建和管理功能，包括定时器的创建和销毁、
//                          定时器调度和执行、定时器槽位管理、各种类型的定时器支持。
//                          该文件定义了NFShmXFrame框架的内存定时器类，提供时间轮算法实现、一次性定时器、
//                          循环定时器、日历定时器、日/周/月循环定时器、定时器对象池管理等功能。
//                          主要功能包括定时器生命周期管理、定时器调度和执行、定时器槽位管理、
//                          支持多种定时器类型
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFNodeList.h"
#include "NFComm/NFObjCommon/NFObjPtr.h"
#include "NFComm/NFObjCommon/NFRawObject.h"

#define NFSHM_INFINITY_CALL                0xffffffff    // 调用无限次

/**
 * @brief 定时器返回类型枚举
 */
enum NFTimerRetType
{
    E_TIMER_TYPE_SUCCESS = 0, ///< 执行成功
    E_TIMER_HANDLER_NULL = 1, ///< 回调为空
    E_TIMER_NOT_TRIGGER = 2,  ///< 没有触发
};

/**
 * @brief 内存定时器类，提供内存定时器的创建和管理功能
 * 
 * 该类继承自NFObjectTemplate和NFListNodeObjWithGlobalId，提供内存定时器的创建和管理功能，
 * 包括定时器的创建和销毁、定时器调度和执行、定时器槽位管理、各种类型的定时器支持。
 * 提供时间轮算法实现、一次性定时器、循环定时器、日历定时器、日/周/月循环定时器、定时器对象池管理等功能。
 */
class NFMemTimer final : public NFObjectTemplate<NFMemTimer, EOT_TYPE_TIMER_OBJ, NFObject>, public NFListNodeObjWithGlobalId<NFMemTimer>
{
public:
    /**
     * @brief 定时器类型枚举
     */
    enum NFMemTimerType
    {
        LOOP_TIMER,        ///< 循环定时器
        ONCE_TIMER,        ///< 一次性定时器
        MONTH_LOOP_TIMER,  ///< 月循环定时器
    };

    /**
     * @brief 构造函数
     */
    NFMemTimer();

    /**
     * @brief 析构函数
     */
    ~NFMemTimer() override;

    /**
     * @brief 创建初始化
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 获取定时器共享内存对象
     * @return 共享内存对象指针
     */
    NFObject* GetTimerShmObj();

    /**
     * @brief 获取定时器共享内存对象ID
     * @return 对象ID
     */
    int GetTimerShmObjId() const;

    /**
     * @brief 设置定时器共享内存对象
     * @param pObj 共享内存对象指针
     */
    void SetTimerShmObj(const NFObject* pObj);

    /**
     * @brief 设置定时器原始共享内存对象
     * @param pObj 原始共享内存对象指针
     */
    void SetTimerRawShmObj(const NFRawObject* pObj);

    /**
     * @brief 获取定时器原始共享内存对象
     * @return 原始共享内存对象指针
     */
    NFRawObject* GetTimerRawShmObj();

    /**
     * @brief 打印调试信息
     */
    void PrintfDebug() const;

    /**
     * @brief 处理定时器
     * @param timeId 定时器ID
     * @param callCount 调用次数
     * @return 处理结果
     */
    NFTimerRetType HandleTimer(int timeId, int callCount);

    /**
     * @brief 判断是否超时
     * @param tick 当前时间戳
     * @return 是否超时
     */
    bool IsTimeOut(int64_t tick);

    /**
     * @brief 定时器触发处理
     * @param tick 当前时间戳
     * @return 处理结果
     */
    NFTimerRetType OnTick(int64_t tick);

    /**
     * @brief 判断是否已删除
     * @return 是否已删除
     */
    bool IsDelete() const;

    /**
     * @brief 设置删除标志
     */
    void SetDelete();

    /**
     * @brief 判断是否等待删除
     * @return 是否等待删除
     */
    bool IsWaitDelete() const;

    /**
     * @brief 设置等待删除标志
     */
    void SetWaitDelete();

    /**
     * @brief 获取定时器类型
     * @return 定时器类型
     */
    NFMemTimerType GetType() const { return m_type; }

    /**
     * @brief 获取开始时间
     * @return 开始时间戳
     */
    int64_t GetBeginTime() const { return m_beginTime; }

    /**
     * @brief 获取间隔时间
     * @return 间隔时间戳
     */
    int64_t GetInterval() const { return m_interval; }

    /**
     * @brief 获取下次运行时间
     * @return 下次运行时间戳
     */
    int64_t GetNextRun() const { return m_nextRun; }

    /**
     * @brief 获取调用次数
     * @return 调用次数
     */
    int32_t GetCallCount() const { return m_callCount; }

    /**
     * @brief 获取槽位索引
     * @return 槽位索引
     */
    int GetSlotIndex() const { return m_slotIndex; }

    /**
     * @brief 获取链表索引
     * @return 链表索引
     */
    int GetListIndex() const { return m_listIndex; }

    /**
     * @brief 获取详细结构信息
     * @return 详细结构信息字符串
     */
    std::string GetDetailStructMsg() const;

    /**
     * @brief 设置定时器类型
     * @param type 定时器类型
     */
    void SetType(NFMemTimerType type) { m_type = type; }

    /**
     * @brief 设置调用次数
     * @param t 调用次数
     */
    void SetCallCount(int32_t t) { m_callCount = t; }

    /**
     * @brief 设置间隔时间
     * @param interval 间隔时间戳
     */
    void SetInterval(int64_t interval) { m_interval = interval; }

    /**
     * @brief 设置下次运行时间（增加间隔）
     */
    void SetNextRun() { m_nextRun += m_interval; }

    /**
     * @brief 设置下次运行时间
     * @param nextTime 下次运行时间戳
     */
    void SetNextRun(int64_t nextTime) { m_nextRun = nextTime; }

    /**
     * @brief 设置开始时间
     * @param time 开始时间戳
     */
    void SetBeginTime(int64_t time) { m_beginTime = time; }

    /**
     * @brief 设置轮数
     * @param round 轮数
     */
    void SetRound(int round) { m_round = round; }

    /**
     * @brief 设置槽位索引
     * @param index 槽位索引
     */
    void SetSlotIndex(int index) { m_slotIndex = index; }

    /**
     * @brief 设置链表索引
     * @param index 链表索引
     */
    void SetListIndex(int index) { m_listIndex = index; }

private:
    /**
     * @brief 删除函数
     */
    void DeleteFunc();

private:
    NFObjPtr<NFObject> m_shmObj;
    NFRawShmPtr<NFRawObject> m_rawShmObj;
    int m_shmObjId;
    NFMemTimerType m_type;

    int64_t m_beginTime; // 开始的时间
    int64_t m_nextRun; // 下一次执行的时间
    int64_t m_interval; // 间隔时间
    int32_t m_callCount;
    int32_t m_curCallCount;
    bool m_delFlag; // 是否删除
    int m_round;
    int m_slotIndex; // 槽id
    bool m_waitDel; // 等待删除标记
    int m_listIndex; // 绑定的list的序号，当为-1时，代表已经脱离绑定
};
