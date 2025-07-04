// -------------------------------------------------------------------------
//    @FileName         :    NFTimerObj.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------
#include "NFTimerObj.h"
#include "NFIPluginManager.h"
#include "NFITimerModule.h"

NFTimerObj::NFTimerObj(NFIPluginManager* pPluginManager) : NFBaseObj(pPluginManager)
{
}

NFTimerObj::~NFTimerObj()
{
	m_pObjPluginManager->FindModule<NFITimerModule>()->KillAllTimer(this);
}

//设置定时器
bool NFTimerObj::SetTimer(uint32_t nTimerID, uint64_t nInterVal, uint32_t nCallCount)
{
	return m_pObjPluginManager->FindModule<NFITimerModule>()->SetTimer(nTimerID, nInterVal, this, nCallCount);
}

//关闭定时器
bool NFTimerObj::KillTimer(uint32_t nTimerID)
{
	return m_pObjPluginManager->FindModule<NFITimerModule>()->KillTimer(nTimerID, this);
}

//关闭所有定时器
bool NFTimerObj::KillAllTimer()
{
	return m_pObjPluginManager->FindModule<NFITimerModule>()->KillAllTimer(this);
}

//设置固定时间的定时器
bool NFTimerObj::SetFixTimer(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterSec, uint32_t nCallCount)
{
	return m_pObjPluginManager->FindModule<NFITimerModule>()->SetClocker(nTimerID, nStartTime, nInterSec, this, nCallCount);
}

