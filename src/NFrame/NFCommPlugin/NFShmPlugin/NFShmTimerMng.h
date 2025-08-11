// -------------------------------------------------------------------------
//    @FileName         :    NFShmTimerMng.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmTimerMng.h
//    @Desc             :    共享内存定时器管理器头文件，提供定时器管理和调度功能。
//                          该文件定义了共享内存定时器管理器类，提供定时器的创建和销毁、
//                          定时器调度和执行、定时器槽位管理、各种类型的定时器支持。
//                          主要功能包括时间轮算法实现、一次性定时器、循环定时器、
//                          日历定时器、日/周/月循环定时器
//
// -------------------------------------------------------------------------

#pragma once

#include "NFShmTimer.h"
#include "NFComm/NFShmStl/NFShmHashMap.h"
#include "NFComm/NFObjCommon/NFNodeList.h"
#include <list>

#define SLOT_COUNT 600                    ///< 槽位总数
#define SLOT_TICK_TIME 32                 ///< 槽位心跳时间
//#define ALL_TIMER_COUNT 300000
#define ALL_TIMER_COUNT 30000             ///< 所有定时器总数
#define CUR_SLOT_TICK_MAX 500             ///< 当前槽位最大心跳数

/**
 * @brief 定时器ID数据结构
 * 
 * 用于管理定时器在链表中的位置信息
 */
struct NFTimerIdData
{
    int m_preIndex;                       ///< 前一个索引
    int m_nextIndex;                      ///< 下一个索引
    int m_curIndex;                       ///< 当前索引
    int m_objId;                          ///< timer的objectid
    int m_slotId;                         ///< slot的m_index
    bool m_isValid;                       ///< 是否非法，做简单判断用
};

class NFShmTimerMng;

/**
 * @brief 共享内存定时器槽位
 * 
 * 管理单个时间槽位中的定时器链表
 */
class NFShmTimerSlot
{
public:
    /**
     * @brief 构造函数
     */
    NFShmTimerSlot();

    /**
     * @brief 析构函数
     */
    ~NFShmTimerSlot();

    /**
     * @brief 创建初始化
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 设置索引
     * 
     * @param i 索引值
     */
    void SetIndex(int i) { m_index = i; }

    /**
     * @brief 添加定时器
     * 
     * @param timer 定时器指针
     * @param idData ID数据指针
     * @param allIdData 所有ID数据指针
     * @return 添加结果
     */
    int AddTimer(NFShmTimer* timer, NFTimerIdData* idData, NFTimerIdData* allIdData);

    /**
     * @brief 心跳处理
     * 
     * @param pTimerManager 定时器管理器指针
     * @param tick 当前时间戳
     * @param timeoutList 超时列表
     * @param seq 序列号
     * @param allIdData 所有ID数据指针
     * @return 处理结果
     */
    bool OnTick(NFShmTimerMng* pTimerManager, int64_t tick, std::list<NFShmTimer*>& timeoutList, uint32_t seq, NFTimerIdData* allIdData);

    /**
     * @brief 删除定时器
     * 
     * @param pTimerManager 定时器管理器指针
     * @param timer 定时器指针
     * @param allIdData 所有ID数据指针
     * @return 删除结果
     */
    bool DeleteTimer(NFShmTimerMng* pTimerManager, NFShmTimer* timer, NFTimerIdData* allIdData);

    /**
     * @brief 解绑链表定时器
     * 
     * @param pTimerManager 定时器管理器指针
     * @param timer 定时器指针
     * @param tmpData 临时数据指针
     * @param allIdData 所有ID数据指针
     * @return ID数据指针
     */
    NFTimerIdData* UnBindListTimer(NFShmTimerMng* pTimerManager, NFShmTimer* timer, const NFTimerIdData* tmpData, NFTimerIdData* allIdData);

    /**
     * @brief 清除运行状态
     * 
     * @param seq 序列号
     */
    void ClearRunStatus(uint32_t seq);

    /**
     * @brief 获取数量
     * 
     * @return 定时器数量
     */
    int GetCount() const { return m_count; }

private:
    //	map<uint32_t, Timer* > m_mapTimer;
    //	TIMER_SLOT_LIST m_mapTimer;
    NFTimerIdData m_headData;             ///< 头数据
    int m_index;                          ///< 索引
    uint32_t m_slotSeq;                   ///< 每次tick的seq
    int m_curRunIndex;                    ///< 当前运行到的链表的序号
    int m_count;                          ///< 定时器的个数
};

/**
 * @brief 共享内存定时器管理器
 * 
 * 负责管理所有定时器的创建、调度和执行
 * 使用时间轮算法实现高效的定时器管理
 */
class NFShmTimerMng final : public NFObjectGlobalTemplate<NFShmTimerMng, EOT_TYPE_TIMER_MNG, NFObject>
{
public:
    /**
     * @brief 构造函数
     */
    NFShmTimerMng();

    /**
     * @brief 析构函数
     */
    ~NFShmTimerMng() override;

    /**
     * @brief 创建初始化
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 心跳处理
     * 
     * @param tick 当前时间戳
     */
    void OnTick(int64_t tick);

    /**
     * @brief 删除此定时器
     * 
     * @param objectId 对象ID
     * @return 删除结果
     */
    int Delete(int objectId);

    /**
     * @brief 获取定时器
     * 
     * @param objectId 对象ID
     * @return 定时器指针
     */
    NFShmTimer* GetTimer(int objectId);

    /**
     * @brief 释放定时器ID数据
     * 
     * @param index 索引
     */
    void ReleaseTimerIdData(int index);

    /**
     * @brief 注册距离现在多少时间执行一次的定时器
     * 
     * hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒, 只执行一次
     * 
     * @param pObj 对象指针
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param microSec 微秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetTimer(const NFObject* pObj, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册某一个时间点执行一次的定时器
     * 
     * hour  minutes  second为第一次执行的时间点时分秒, 只执行一次
     * 
     * @param pObj 对象指针
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetCalender(const NFObject* pObj, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册某一个时间点执行一次的定时器
     * 
     * timestamp为第一次执行的时间点的时间戳,单位是秒, 只执行一次
     * 
     * @param pObj 对象指针
     * @param timestamp 时间戳
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetCalender(const NFObject* pObj, uint64_t timestamp, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册循环执行定时器
     * 
     * hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,  interval 为循环间隔时间，为毫秒
     * 
     * @param pObj 对象指针
     * @param interval 间隔时间
     * @param callCount 调用次数
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param microSec 微秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetTimer(const NFObject* pObj, int interval, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册循环执行定时器
     * 
     * hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒
     * 
     * @param pObj 对象指针
     * @param callCount 调用次数
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param microSec 微秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetDayTime(const NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册某一个时间点日循环执行定时器
     * 
     * hour  minutes  second为一天中开始执行的时间点，    23：23：23     每天23点23分23秒执行
     * 
     * @param pObj 对象指针
     * @param callCount 调用次数
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetDayCalender(const NFObject* pObj, int callCount, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 周循环
     * 
     * hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒
     * 
     * @param pObj 对象指针
     * @param callCount 调用次数
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param microSec 微秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetWeekTime(const NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册某一个时间点周循环执行定时器
     * 
     * weekDay  hour  minutes  second 为一周中某一天开始执行的时间点
     * 
     * @param pObj 对象指针
     * @param callCount 调用次数
     * @param weekDay 星期几
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetWeekCalender(const NFObject* pObj, int callCount, int weekDay, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 月循环
     * 
     * hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,最好是同一天
     * 
     * @param pObj 对象指针
     * @param callCount 调用次数
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param microSec 微秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetMonthTime(const NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, const NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册某一个时间点月循环执行定时器
     * 
     * day  hour  minutes  second 为一月中某一天开始执行的时间点
     * 
     * @param pObj 对象指针
     * @param callCount 调用次数
     * @param day 日期
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param pRawShmObj 原始共享内存对象
     * @return 注册结果
     */
    int SetMonthCalender(const NFObject* pObj, int callCount, int day, int hour, int minutes, int second, const NFRawObject* pRawShmObj = nullptr);

public:
    /**
     * @brief 添加共享内存对象定时器
     * 
     * @param pObj 对象指针
     * @param pTimer 定时器指针
     * @return 添加结果
     */
    int AddShmObjTimer(const NFObject* pObj, NFShmTimer* pTimer);

    /**
     * @brief 清除共享内存对象定时器
     * 
     * @param pTimer 定时器指针
     * @return 清除结果
     */
    int ClearShmObjTimer(NFShmTimer* pTimer);

    /**
     * @brief 清除所有定时器
     * 
     * @param pObj 对象指针
     * @return 清除结果
     */
    int ClearAllTimer(const NFObject* pObj);

    /**
     * @brief 清除所有定时器
     * 
     * @param pObj 对象指针
     * @param pRawShmObj 原始共享内存对象
     * @return 清除结果
     */
    int ClearAllTimer(const NFObject* pObj, const NFRawObject* pRawShmObj);

private:
    /**
     * @brief 附加定时器
     * 
     * @param timer 定时器指针
     * @param tick 时间戳
     * @param isNewTimer 是否新定时器
     * @return 附加结果
     */
    bool AttachTimer(NFShmTimer* timer, int64_t tick, bool isNewTimer);

    /**
     * @brief 添加定时器
     * 
     * @param timer 定时器指针
     * @param tick 时间戳
     * @param isNewTimer 是否新定时器
     * @return 添加结果
     */
    int AddTimer(NFShmTimer* timer, int64_t tick, bool isNewTimer = true);

    /**
     * @brief 添加定时器
     * 
     * @param timer 定时器指针
     * @param slot 槽位
     * @return 添加结果
     */
    int AddTimer(NFShmTimer* timer, int slot);

    /**
     * @brief 设置距离时间
     * 
     * @param stime 定时器指针
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param microSec 微秒
     * @param interval 间隔时间
     * @param callCount 调用次数
     * @return 设置结果
     */
    bool SetDistanceTime(NFShmTimer* stime, int hour, int minutes, int second, int microSec, int interval = 0, int callCount = 1);

    /**
     * @brief 设置日时间
     * 
     * @param stime 定时器指针
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param interval 间隔时间
     * @param callCount 调用次数
     * @return 设置结果
     */
    bool SetDayTime(NFShmTimer* stime, int hour, int minutes, int second, int interval = 0, int callCount = 1);

    /**
     * @brief 设置日时间
     * 
     * @param stime 定时器指针
     * @param timestamp 时间戳
     * @param interval 间隔时间
     * @param callCount 调用次数
     * @return 设置结果
     */
    bool SetDayTime(NFShmTimer* stime, uint64_t timestamp, int interval = 0, int callCount = 1);

    /**
     * @brief 设置周时间
     * 
     * @param stime 定时器指针
     * @param weekDay 星期几
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param callCount 调用次数
     * @return 设置结果
     */
    bool SetWeekTime(NFShmTimer* stime, int weekDay, int hour, int minutes, int second, int callCount = 1);

    /**
     * @brief 设置月时间
     * 
     * @param stime 定时器指针
     * @param day 日期
     * @param hour 小时
     * @param minutes 分钟
     * @param second 秒
     * @param callCount 调用次数
     * @return 设置结果
     */
    bool SetMonthTime(NFShmTimer* stime, int day, int hour, int minutes, int second, int callCount = 1);

    /**
     * @brief 获取空闲的链表结构
     * 
     * @return 定时器ID数据指针
     */
    NFTimerIdData* GetFreeTimerIdData();

    /**
     * @brief 检查是否已满
     * 
     * @return 是否已满
     */
    bool CheckFull() const;

private:
    NFShmTimerSlot m_slots[SLOT_COUNT];   ///< 槽位数组
    uint32_t m_currSlot;                  ///< 当前槽位
    int64_t m_beforeTick;                 ///< 上一次执行的tick数

    NFTimerIdData m_timerIdData[ALL_TIMER_COUNT + 1];  ///< 定时器ID数据数组
    int m_iFreeIndex;                     ///< 空闲索引
    uint32_t m_timerSeq;                  ///< 每次tick的seq,只有当前m_currSlot已经遍历完了，才会++

    NFShmHashMap<int, NFNodeObjList<NFShmTimer>, ALL_TIMER_COUNT> m_shmObjTimer;  ///< 共享内存对象定时器映射
};
