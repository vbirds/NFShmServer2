// -------------------------------------------------------------------------
//    @FileName         :    NFTimerAxis.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------
#pragma once
#include <list>
#include <vector>
#include "NFComm/NFCore/NFSingleton.hpp"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"

class NFTimerObjBase;

//时间轴
class NFTimerAxis
{
public:
	NFTimerAxis();
	virtual ~NFTimerAxis();

	bool Init();
	bool UnInit();

	//设置定时器
	bool SetTimer(uint32_t nTimerID, uint64_t nInterVal, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);
	//关闭定时器
	bool KillTimer(uint32_t nTimerID, NFTimerObjBase* handler);
	//关闭所有定时器
	bool KillAllTimer(NFTimerObjBase* handler);
	//更新定时器
	void Update();

	//设置固定时间的定时器
	bool SetClocker(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterSec, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);
	bool SetCalender(uint32_t nTimerID, const std::string& timeStr, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);

private:
	//设置秒定时器
	bool SetTimerSec(uint32_t nTimerID, uint64_t nInterVal, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);
	//检查tick
	void CheckTick();
	//更新秒定时器
	void UpdateSec();

protected:
	static uint64_t GetTick()
	{
		return NF_ADJUST_TIMENOW_MS();
	}

	static uint64_t GetUnixSec()
	{
		return NF_ADJUST_TIMENOW();
	}

	struct Timer
	{
		uint32_t nTimerID; //定时器ID
		uint64_t nInterVal; //间隔
		uint32_t nCallCount; //调用次数
		uint64_t nLastTick; //最后一次tick
		NFTimerObjBase* pHandler;//回调指针
		uint8_t byType; //类型 0 - 毫秒定时器， 1 - 秒定时器
		uint32_t nGridIndex; //所在的时间刻度
		std::list<Timer*>::iterator pos; //在时间轴中的位置，便于快速定位
		Timer()
		{
			nTimerID = 0;
			nInterVal = 0;
			nCallCount = 0;
			nLastTick = 0;
			pHandler = nullptr;
			byType = 0;
			nGridIndex = 0;
		}
	};

	//重置定时器在时间轴上的位置
	static bool ResetTimerPos(Timer* pTimer);

	typedef std::list<Timer*> TIMER_LIST; //某个刻度中定时器列表
	typedef std::vector<TIMER_LIST*> TIMER_AXIS; //所有事件轴的定时器

	TIMER_AXIS m_TimerAxis; //毫秒事件轴
	uint64_t m_nInitTick; //毫秒时间轴初始Tick
	uint64_t m_nLastTick; //毫秒时间轴最后一次检查的tick

	TIMER_AXIS m_TimerAxisSec; //秒时间轴
	uint64_t m_nInitSec; //秒时间轴初始时间
	uint64_t m_nLastSec; //秒时间轴最后一次检查的时间
};


