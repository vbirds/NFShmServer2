// -------------------------------------------------------------------------
//    @FileName         :    NFMemTimerMng.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMemTimerMng.h
//    @Desc             :    内存定时器管理器头文件，提供定时器管理和调度功能。
//                          该文件定义了NFShmXFrame框架的内存定时器管理器，负责定时器的创建和销毁、
//                          定时器调度和执行、定时器槽位管理、各种类型的定时器支持。
//                          主要功能包括时间轮算法实现、一次性定时器、循环定时器、
//                          日历定时器、日/周/月循环定时器、定时器对象池管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFMemTimer.h"
#include "NFComm/NFShmStl/NFShmHashMap.h"
#include "NFComm/NFObjCommon/NFNodeList.h"
#include <list>

class NFMemTimerMng;

#define SLOT_COUNT 600
#define SLOT_TICK_TIME 32
//#define ALL_TIMER_COUNT 300000
#define ALL_TIMER_COUNT 30000
#define CUR_SLOT_TICK_MAX 500

/**
 * @brief 内存定时器ID数据结构，用于管理定时器的链表头
 * 
 * 该结构体用于存储定时器在链表中的位置信息，包括前驱索引、后继索引、
 * 当前索引、对象ID、槽位ID和有效性标志。
 */
struct NFMemTimerIdData
{
    int m_preIndex;    ///< 前驱索引
    int m_nextIndex;   ///< 后继索引
    int m_curIndex;    ///< 当前索引
    int m_objId;       ///< timer的objectid
    int m_slotId;      ///< slot的m_index
    bool m_isValid;    ///< 是否非法，做简单判断用
};

/**
 * @brief 内存定时器槽位类，用于管理时间轮算法中的单个槽位
 * 
 * 该类负责管理时间轮算法中的单个槽位，包括定时器的添加、删除、
 * 超时检查和链表管理等。每个槽位维护一个定时器链表，用于管理
 * 在该时间点需要触发的定时器。
 */
class NFMemTimerSlot
{
public:
    /**
     * @brief 构造函数
     */
    NFMemTimerSlot();
    
    /**
     * @brief 析构函数
     */
    ~NFMemTimerSlot();

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
     * @brief 设置索引
     * @param i 索引值
     */
    void SetIndex(int i) { m_index = i; }

    /**
     * @brief 添加定时器到槽位
     * @param timer 定时器指针
     * @param idData 定时器ID数据
     * @param allIdData 所有定时器ID数据数组
     * @return 添加结果
     */
    int AddTimer(NFMemTimer* timer, NFMemTimerIdData* idData, NFMemTimerIdData* allIdData);

    /**
     * @brief 处理槽位超时检查
     * @param pTimerManager 定时器管理器指针
     * @param tick 当前时间戳
     * @param timeoutList 超时定时器列表
     * @param seq 序列号
     * @param allIdData 所有定时器ID数据数组
     * @return 是否处理完成
     */
    bool OnTick(NFMemTimerMng* pTimerManager, int64_t tick, std::list<NFMemTimer*>& timeoutList, uint32_t seq, NFMemTimerIdData* allIdData);

    /**
     * @brief 删除定时器
     * @param pTimerManager 定时器管理器指针
     * @param timer 定时器指针
     * @param allIdData 所有定时器ID数据数组
     * @return 删除结果
     */
    bool DeleteTimer(NFMemTimerMng* pTimerManager, NFMemTimer* timer, NFMemTimerIdData* allIdData);

    /**
     * @brief 解绑链表中的定时器
     * @param pTimerManager 定时器管理器指针
     * @param timer 定时器指针
     * @param tmpData 临时数据
     * @param allIdData 所有定时器ID数据数组
     * @return 解绑的定时器ID数据
     */
    NFMemTimerIdData* UnBindListTimer(NFMemTimerMng* pTimerManager, NFMemTimer* timer, const NFMemTimerIdData* tmpData, NFMemTimerIdData* allIdData);

    /**
     * @brief 清除运行状态
     * @param seq 序列号
     */
    void ClearRunStatus(uint32_t seq);

    /**
     * @brief 获取定时器数量
     * @return 定时器数量
     */
    int GetCount() const { return m_count; }

private:
    /**
     * @brief 内存定时器ID数据结构，用于管理定时器的链表头
     */
    NFMemTimerIdData m_headData;

    /**
     * @brief 当前索引，用于标识定时器在链表中的位置
     */
    int m_index;

    /**
     * @brief 每次tick的序列号，用于标识每次时间轮转的序号
     */
    uint32_t m_slotSeq;

    /**
     * @brief 当前运行到的链表的序号，用于标识当前正在处理的链表位置
     */
    int m_curRunIndex;

    /**
     * @brief 定时器的个数，用于记录当前管理的定时器数量
     */
    int m_count;
};

/**
 * @brief 内存定时器管理器类，提供定时器管理和调度功能
 * 
 * 该类继承自NFObjectGlobalTemplate，负责定时器的创建和销毁、
 * 定时器调度和执行、定时器槽位管理、各种类型的定时器支持。
 * 提供时间轮算法实现、一次性定时器、循环定时器、
 * 日历定时器、日/周/月循环定时器、定时器对象池管理等功能。
 */
class NFMemTimerMng final : public NFObjectGlobalTemplate<NFMemTimerMng, EOT_TYPE_TIMER_MNG, NFObject>
{
public:
    /**
     * @brief 构造函数
     */
    NFMemTimerMng();

    ~NFMemTimerMng() override;

    int CreateInit();

    int ResumeInit();

    void OnTick(int64_t tick);

    // 删除此定时器
    int Delete(int objectId);

    NFMemTimer* GetTimer(int objectId);

    void ReleaseTimerIdData(int index);

    //注册距离现在多少时间执行一次的定时器(hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒, 只执行一次)
    int SetTimer(const NFObject* pObj, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    //注册某一个时间点执行一次的定时器(hour  minutes  second为第一次执行的时间点时分秒, 只执行一次)
    int SetCalender(const NFObject* pObj, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

    //注册某一个时间点执行一次的定时器(timestamp为第一次执行的时间点的时间戳,单位是秒, 只执行一次)
    int SetCalender(const NFObject* pObj, uint64_t timestamp, const NFRawObject* pRawShmObj = nullptr);

    //注册循环执行定时器（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,  interval 为循环间隔时间，为毫秒）
    int SetTimer(const NFObject* pObj, int interval, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    //注册循环执行定时器（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒）
    int SetDayTime(const NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    //注册某一个时间点日循环执行定时器（hour  minutes  second为一天中开始执行的时间点，    23：23：23     每天23点23分23秒执行）
    int SetDayCalender(const NFObject* pObj, int callCount, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

    //周循环（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒）
    int SetWeekTime(const NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    //注册某一个时间点周循环执行定时器（ weekDay  hour  minutes  second 为一周中某一天开始执行的时间点）
    int SetWeekCalender(const NFObject* pObj, int callCount, int weekDay, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

    //月循环（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,最好是同一天）
    int SetMonthTime(const NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    //注册某一个时间点月循环执行定时器（ day  hour  minutes  second 为一月中某一天开始执行的时间点）
    int SetMonthCalender(const NFObject* pObj, int callCount, int day, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

public:
    int AddShmObjTimer(const NFObject* pObj, NFMemTimer* pTimer);
    int ClearShmObjTimer(NFMemTimer* pTimer);
    int ClearAllTimer(const NFObject* pObj);
    int ClearAllTimer(const NFObject* pObj, const NFRawObject* pRawShmObj);

private:
    bool AttachTimer(NFMemTimer* timer, int64_t tick, bool isNewTimer);

    int AddTimer(NFMemTimer* timer, int64_t tick, bool isNewTimer = true);

    int AddTimer(NFMemTimer* timer, int slot);

    bool SetDistanceTime(NFMemTimer* stime, int hour, int minutes, int second, int microSec, int interval = 0, int callCount = 1);

    bool SetDayTime(NFMemTimer* stime, int hour, int minutes, int second, int interval = 0, int callCount = 1);

    bool SetDayTime(NFMemTimer* stime, uint64_t timestamp, int interval = 0, int callCount = 1);

    bool SetWeekTime(NFMemTimer* stime, int weekDay, int hour, int minutes, int second, int callCount = 1);

    bool SetMonthTime(NFMemTimer* stime, int day, int hour, int minutes, int second, int callCount = 1);

    NFMemTimerIdData* GetFreeTimerIdData();

    bool CheckFull() const;

private:
    NFMemTimerSlot m_slots[SLOT_COUNT];
    uint32_t m_currSlot;
    int64_t m_beforeTick; //上一次执行的tick数

    NFMemTimerIdData m_timerIdData[ALL_TIMER_COUNT + 1];
    int m_iFreeIndex;
    uint32_t m_timerSeq; // 每次tick的seq,只有当前m_currSlot已经遍历完了，才会++

    std::unordered_map<int, NFNodeObjList<NFMemTimer>> m_shmObjTimer;
};
