// -------------------------------------------------------------------------
//    @FileName         :    NFMemTimer.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMemTimer.cpp
//    @Desc             :    内存定时器实现文件，提供内存定时器的创建和管理功能，包括定时器的创建和销毁、
//                          定时器调度和执行、定时器槽位管理、各种类型的定时器支持。
//                          该文件实现了NFShmXFrame框架的内存定时器类，提供时间轮算法实现、一次性定时器、
//                          循环定时器、日历定时器、日/周/月循环定时器、定时器对象池管理等功能。
//                          主要功能包括定时器生命周期管理、定时器调度和执行、定时器槽位管理、
//                          支持多种定时器类型
//
// -------------------------------------------------------------------------

#include "NFMemTimer.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFMemTimerMng.h"

NFMemTimer::NFMemTimer()
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

NFMemTimer::~NFMemTimer()
{
    NFMemTimerMng::Instance()->ClearShmObjTimer(this);
    DeleteFunc();
}

void NFMemTimer::DeleteFunc()
{
    // 是在SubscriberSlot 创建的，必须在这销毁
    m_shmObj = nullptr;
    m_shmObjId = INVALID_ID;
    m_rawShmObj = nullptr;
}

int NFMemTimer::CreateInit()
{
    m_shmObj = nullptr;
    m_shmObjId = INVALID_ID;
    m_rawShmObj = nullptr;

    m_type = ONCE_TIMER;
    m_beginTime = 0;
    m_nextRun = 0;
    m_interval = 0;
    m_delFlag = false;
    m_round = 0;
    m_slotIndex = -1;
    m_waitDel = false;
    m_listIndex = -1;
    m_callCount = 0;
    m_curCallCount = 0;
    return 0;
}

int NFMemTimer::ResumeInit()
{
    return 0;
}

/**
 * @brief 获取定时器共享内存对象
 * 
 * 该函数返回定时器关联的共享内存对象指针。
 * 
 * @return 共享内存对象指针
 */
NFObject* NFMemTimer::GetTimerShmObj()
{
    return m_shmObj.GetPoint();
}

/**
 * @brief 获取定时器共享内存对象ID
 * 
 * 该函数返回定时器关联的共享内存对象的全局ID。
 * 
 * @return 共享内存对象ID
 */
int NFMemTimer::GetTimerShmObjId() const
{
    return m_shmObjId;
}

/**
 * @brief 设置定时器共享内存对象
 * 
 * 该函数设置定时器关联的共享内存对象。
 * 
 * @param pObj 共享内存对象指针
 */
void NFMemTimer::SetTimerShmObj(const NFObject* pObj)
{
    m_shmObj = pObj;
    m_shmObjId = pObj->GetGlobalId();
}

/**
 * @brief 设置定时器原始共享内存对象
 * 
 * 该函数设置定时器关联的原始共享内存对象。
 * 
 * @param pObj 原始共享内存对象指针
 */
void NFMemTimer::SetTimerRawShmObj(const NFRawObject* pObj)
{
    m_rawShmObj = pObj;
}

/**
 * @brief 获取定时器原始共享内存对象
 * 
 * 该函数返回定时器关联的原始共享内存对象指针。
 * 
 * @return 原始共享内存对象指针
 */
NFRawObject* NFMemTimer::GetTimerRawShmObj()
{
    return m_rawShmObj;
}

/**
 * @brief 打印调试信息
 * 
 * 该函数用于打印定时器的调试信息。
 */
void NFMemTimer::PrintfDebug() const
{
    LOG_DEBUG(0, "timer debug:{}", GetDetailStructMsg());
}

/**
 * @brief 获取详细结构信息
 * 
 * 该函数返回定时器的详细结构信息字符串，包括：
 * - 定时器类型
 * - 开始时间
 * - 下次运行时间
 * - 时间间隔
 * - 删除标志
 * - 轮次信息
 * - 槽位索引
 * - 等待删除标志
 * - 链表索引
 * - 对象ID
 * - 全局ID
 * - 共享内存对象ID（调试模式）
 * 
 * @return 详细结构信息字符串
 */
std::string NFMemTimer::GetDetailStructMsg() const
{
    std::ostringstream oss;

    oss << " type:" << m_type
        << " begintime:" << m_beginTime
        << " nextRun:" << m_nextRun
        << " interval:" << m_interval
        << " delFlag:" << m_delFlag
        << " round:" << m_round
        << " slotIndex:" << m_slotIndex
        << " waitDel:" << m_waitDel
        << " listIndex:" << m_listIndex
        << " objID:" << GetObjId()
        << " globalID:" << GetGlobalId();

#ifdef NF_DEBUG_MODE
    oss << " shmobj gloablid:" << m_shmObjId;
#endif

    return oss.str();
}

/**
 * @brief 检查定时器是否超时
 * 
 * 该函数检查定时器是否已经超时。
 * 检查过程包括：
 * 1. 减少轮次计数
 * 2. 检查当前时间是否超过下次运行时间
 * 3. 检查轮次是否已用完
 * 
 * @param tick 当前时间戳
 * @return 是否超时
 */
bool NFMemTimer::IsTimeOut(int64_t tick)
{
    --m_round;
    //	LOGSVR_TRACE("is time out: " << GetDetailStructMsg());
    if (tick - m_nextRun >= 0 || m_round <= 0)
    {
        return true;
    }

    return false;
}

/**
 * @brief 定时器心跳处理
 * 
 * 该函数是定时器的核心处理函数，负责：
 * 1. 检查定时器是否超时
 * 2. 如果超时，则执行定时器回调
 * 3. 更新调用次数计数
 * 4. 处理定时器状态
 * 
 * @param tick 当前时间戳
 * @return 处理结果
 */
NFTimerRetType NFMemTimer::OnTick(int64_t tick)
{
    if (tick - m_nextRun >= 0 || m_round <= 0)
    {
        if (m_shmObj)
        {
            //			LOGSVR_TRACE("time out: " << GetDetailStructMsg());
            // 更新调用次数计数
            if (m_callCount != static_cast<int32_t>(NFSHM_INFINITY_CALL) && m_callCount > 0)
            {
                m_callCount--;
            }
            m_curCallCount++;

            return HandleTimer(GetObjId(), m_curCallCount);
        }
        NFLogError(NF_LOG_DEFAULT, 0, "timer ontick error:{} ", GetDetailStructMsg());
        return E_TIMER_HANDLER_NULL;
    }

    return E_TIMER_NOT_TRIGGER;
}

NFTimerRetType NFMemTimer::HandleTimer(int timeId, int callCount)
{
    if (!m_shmObj)
    {
        return E_TIMER_HANDLER_NULL;
    }

#if NF_DEBUG_MODE
    if (m_shmObjId >= 0)
    {
        NF_ASSERT(m_shmObjId == m_shmObj->GetGlobalId());
        NFObject* pObjGetObjFromTypeIndex = FindModule<NFIMemMngModule>()->GetObjByGlobalIdWithNoCheck(m_shmObjId);
        NF_ASSERT(pObjGetObjFromTypeIndex == m_shmObj.GetPoint());
    }
#endif

    if (m_rawShmObj)
    {
        m_rawShmObj->OnTimer(timeId, callCount);
    }
    else
    {
        m_shmObj->OnTimer(timeId, callCount);
    }

    return E_TIMER_TYPE_SUCCESS;
}

bool NFMemTimer::IsDelete() const
{
    return m_delFlag;
}

void NFMemTimer::SetDelete()
{
    m_delFlag = true;
}

bool NFMemTimer::IsWaitDelete() const
{
    return m_waitDel;
}

void NFMemTimer::SetWaitDelete()
{
    m_waitDel = true;
}


