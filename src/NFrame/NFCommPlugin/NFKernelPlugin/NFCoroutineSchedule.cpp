// -------------------------------------------------------------------------
//    @FileName         :    NFCoroutineSchedule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

#include "NFCoroutineSchedule.h"

#include "NFCoroutine.h"
#include "NFSchedule.h"
#include "NFComm/NFPluginModule/NFCoroutineTask.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"

NFCoroutineTaskTimer::NFCoroutineTaskTimer(NFCoroutineSchedule* pCoSche, int64_t taskId): NFTimerObj(pCoSche->m_pObjPluginManager), m_pCoSche(pCoSche), m_taskId(taskId)
{
    m_delete = false;
}

NFCoroutineTaskTimer::~NFCoroutineTaskTimer()
{

}

void NFCoroutineTaskTimer::SetYieldTimeout(int32_t timeout_ms)
{
   if (timeout_ms > 0)
   {
       SetTimer(ENUM_NF_COROUTINE_TASK_TIMER_YIELD, timeout_ms, 1);
   }
}

int NFCoroutineTaskTimer::OnTimer(uint32_t nTimerID)
{
    if (nTimerID == ENUM_NF_COROUTINE_TASK_TIMER_YIELD)
    {
        m_pCoSche->OnTimeout(m_taskId);
        m_delete = true;
    }
    return 0;
}

void NFCoroutineTaskTimer::SetDelete()
{
    KillAllTimer();
    m_delete = true;
}

NFCoroutineSchedule::NFCoroutineSchedule(NFIPluginManager* pPluginManager) : NFTimerObj(pPluginManager), schedule_(NULL), task_map_(), pre_start_task_() {
	SetTimer(1, 1000);
}

NFCoroutineSchedule::~NFCoroutineSchedule() {
    if (schedule_ != NULL) {
        Close();
    }
}

bool NFCoroutineSchedule::CheckStopServer()
{
    NFLogInfo(NF_LOG_DEFAULT, 0, "NFCoroutineSchedule StopServer.................");
    if (schedule_)
    {
        return schedule_->CheckStopServer();
    }

    if (!m_taskTimer.empty())
    {
        return false;
    }
    return true;
}

int NFCoroutineSchedule::OnTimer(uint32_t nTimerID)
{
	ClearTimer();
    return 0;
}

void NFCoroutineSchedule::ClearTimer()
{
	for (auto iter = m_taskTimer.begin(); iter != m_taskTimer.end();)
	{
		auto pTimer = *iter;
		if (pTimer && pTimer->IsDelete() == false)
		{
			iter++;
			continue;
		}

		if (pTimer)
		{
			NF_SAFE_DELETE(pTimer);
		}
		iter = m_taskTimer.erase(iter);
	}
}

int NFCoroutineSchedule::Init(uint32_t stack_size) {
    schedule_ = NF_NEW NFSchedule(stack_size);
    if (schedule_ == NULL) {
        NFLogTrace(NF_LOG_DEFAULT, 0, "NFCoroutineSchedule Init Failed, new NFSchedule Failed!");
        return -1;
    }
    return 0;
}

int NFCoroutineSchedule::Close() {
    int ret = 0;
    if (schedule_ != NULL) {
        NF_SAFE_DELETE(schedule_);
    }

    ret += pre_start_task_.size();
    for (std::set<NFCoroutineTask *>::iterator iter = pre_start_task_.begin(); iter != pre_start_task_.end(); iter++) {
        NFCoroutineTask *pTask = *iter;
        NF_SAFE_DELETE(pTask);
    }

    ret += task_map_.size();
    for (auto iter = task_map_.begin(); iter != task_map_.end();) {
        NF_SAFE_DELETE(iter->second);

        iter = task_map_.erase(iter);
    }

    return ret;
}

int NFCoroutineSchedule::Size() const {
    int ret = 0;
    ret += task_map_.size();
    ret += pre_start_task_.size();
    return ret;
}

NFCoroutineTask *NFCoroutineSchedule::CurrentTask() const {
    int64_t id = schedule_->CoroutineRunning();
    return Find(id);
}

NFCoroutineTask *NFCoroutineSchedule::Find(int64_t id) const {
    NFCoroutineTask *ret = NULL;
    auto iter = task_map_.find(id);
    if (iter != task_map_.end()) {
        ret = iter->second;
    }
    return ret;
}

int64_t NFCoroutineSchedule::CurrentTaskId() const {
    return schedule_->CoroutineRunning();
}

/**
 * @brief 当前是否在携程中
 * @return
 */
bool NFCoroutineSchedule::IsInCoroutine() const
{
    return CurrentTaskId() != INVALID_CO_ID;
}

int NFCoroutineSchedule::AddTaskToSchedule(NFCoroutineTask *task) {
    pre_start_task_.insert(task);
    return 0;
}

int32_t NFCoroutineSchedule::Yield(int32_t timeout_ms) {
    int64_t co_id   = INVALID_CO_ID;
    NFCoroutineTaskTimer* pTimer = NULL;
    if (timeout_ms > 0) {
        co_id = CurrentTaskId();
        if (INVALID_CO_ID == co_id) {
            return NFrame::ERR_CODE_CO_NOT_IN_COROUTINE;
        }

        pTimer = NF_NEW NFCoroutineTaskTimer(this, co_id);
        pTimer->SetYieldTimeout(timeout_ms);
        m_taskTimer.push_back(pTimer);
    }

    int32_t ret = schedule_->CoroutineYield();
    if (pTimer)
    {
        pTimer->SetDelete();
    }
    return ret;
}

int32_t NFCoroutineSchedule::Resume(int64_t id, int32_t result) {
    return schedule_->CoroutineResume(id, result);
}

int32_t NFCoroutineSchedule::OnTimeout(int64_t id) {
    Resume(id, NFrame::ERR_CODE_CO_TIMEOUT);
    return kTIMER_BE_REMOVED;
}

int32_t NFCoroutineSchedule::Status(int64_t id) {
    return schedule_->CoroutineStatus(id);
}

google::protobuf::Message *NFCoroutineSchedule::GetUserData(int64_t id)
{
    return schedule_->CoroutineUserData(id);
}

int NFCoroutineSchedule::SetUserData(int64_t id, google::protobuf::Message *pUserData)
{
    return schedule_->CoroutineSetUserData(id, pUserData);
}

