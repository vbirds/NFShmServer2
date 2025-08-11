// -------------------------------------------------------------------------
//    @FileName         :    NFTransBase.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTransBase.cpp
//
// -------------------------------------------------------------------------

/**
 * @file NFTransBase.cpp
 * @brief 事务处理基类实现文件
 * @details 实现了NFrame框架中事务处理基类NFTransBase的所有功能，包括事务生命周期
 *          管理、超时控制、运行次数限制、状态管理等。为复杂的异步业务处理提供
 *          了可靠的事务框架，确保业务逻辑的完整性和可追踪性。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 实现的主要功能：
 *       - 事务初始化和生命周期管理
 *       - 事务运行次数和超时控制
 *       - 事务状态跟踪和完成检测
 *       - 协程支持的异步处理
 *       - 客户端消息处理接口
 *       - 事务结果回调处理
 * 
 * @warning 使用注意事项：
 *          - 事务有最大运行次数限制，超过会失败
 *          - 事务有活跃时间限制，超时会被清理
 *          - 必须正确调用SetFinished标记事务完成
 *          - 事务处理应该避免长时间阻塞操作
 */

#include "NFTransBase.h"

#include <NFComm/NFCore/NFServerTime.h>

#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFCore/NFTimeUtil.h"
#include "NFComm/NFKernelMessage/FrameSqlData.pb.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFError.h"
#include "NFComm/NFPluginModule/NFICoroutineModule.h"

NFTransBase::NFTransBase()
{
	if (NFShmMgr::Instance()->GetCreateMode() == EN_OBJ_MODE_INIT)
	{
		CreateInit();
	}
	else
	{
		ResumeInit();
	}
}

NFTransBase::~NFTransBase()
{
}

int NFTransBase::CreateInit()
{
	m_wCurState = 0;
	m_dwStartTime = 0;
	m_dwActiveTime = 0;
	m_iActiveTimeOut = TRANS_ACTIVE_TIMEOUT;
	m_bIsFinished = false;
	m_wRunedTimes = 0;
	m_iRunLogicRetCode = 0;
	m_dwMaxRunTimes = 0;
	m_rpcId = INVALID_ID;
	Init();
	return 0;
}

int NFTransBase::ResumeInit()
{
	m_dwActiveTime = NF_TIMENOW();
	return 0;
}

int NFTransBase::Init()
{
	m_wCurState = 0;
	m_dwStartTime = NF_TIMENOW();
	m_dwActiveTime = NF_TIMENOW();
	m_wRunedTimes = 0;
	return 0;
}

void NFTransBase::IncreaseRunTimes()
{
	m_wRunedTimes++;
}

bool NFTransBase::IsFinished() const
{
	return m_bIsFinished;
}

void NFTransBase::SetFinished(int iRetCode)
{
	if (iRetCode != 0)
	{
		m_iRunLogicRetCode = iRetCode;
	}

	if (m_bIsFinished)
	{
		return;
	}

	m_bIsFinished = true;

	HandleTransFinished(m_iRunLogicRetCode);
	OnTransFinished(m_iRunLogicRetCode);
}

int NFTransBase::ProcessCSMsgReq(const google::protobuf::Message* pCsMsgReq)
{
	int iRetCode = 0;
	do
	{
		iRetCode = RunCommLogic();
		CHECK_ERR_BREAK_WF(0, iRetCode, "RunCommLogic Failed RunedTimes:%d. GlobalID:%d Type:%d(%s)", m_wRunedTimes, GetGlobalId(), GetClassType(), GetClassName().c_str());

		iRetCode = HandleCSMsgReq(pCsMsgReq);
		CHECK_ERR_BREAK_WF(0, iRetCode, "HandleCSMsgReq Failed RunedTimes:%d. GlobalID:%d Type:%d(%s)", m_wRunedTimes, GetGlobalId(), GetClassType(), GetClassName().c_str());
	}
	while (false);

	if (iRetCode != 0)
	{
		m_iRunLogicRetCode = iRetCode;
		SetFinished(iRetCode);
	}

	return 0;
}


int NFTransBase::ProcessDBMsgRes(NFrame::Proto_FramePkg& svrPkg)
{
	switch (svrPkg.msg_id())
	{
	case NFrame::NF_STORESVR_S2C_SELECT:
		{
			NFrame::storesvr_sel_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_SELECTOBJ:
		{
			NFrame::storesvr_selobj_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_INSERTOBJ:
		{
			NFrame::storesvr_insertobj_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_DELETE:
		{
			NFrame::storesvr_del_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_DELETEOBJ:
		{
			NFrame::storesvr_delobj_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());;
		}
		break;
	case NFrame::NF_STORESVR_S2C_MODIFY:
		{
			NFrame::storesvr_mod_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_MODIFYOBJ:
		{
			NFrame::storesvr_modobj_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_UPDATE:
		{
			NFrame::storesvr_update_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_UPDATEOBJ:
		{
			NFrame::storesvr_updateobj_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	case NFrame::NF_STORESVR_S2C_EXECUTE:
		{
			NFrame::storesvr_execute_res selectRes;
			selectRes.ParsePartialFromString(svrPkg.msg_data());
			ProcessDBMsgRes(&selectRes, svrPkg.msg_id(),
			                svrPkg.store_info().table_id(),
			                svrPkg.disp_info().seq(),
			                svrPkg.disp_info().err_code());
		}
		break;
	default:
		{
		}
		break;
	}

	return 0;
}

int NFTransBase::ProcessDBMsgRes(const google::protobuf::Message* pSsMsgRes, uint32_t cmd, uint32_t tableId, uint32_t seq, int32_t errCode)
{
	int iRetCode = 0;
	do
	{
		iRetCode = RunCommLogic();
		CHECK_ERR_BREAK_WF(0, iRetCode, "RunCommLogic Failed RunedTimes:%d. GlobalID:%d Type:%d(%s)", m_wRunedTimes, GetGlobalId(), GetClassType(), GetClassName().c_str());

		iRetCode = HandleDBMsgRes(pSsMsgRes, cmd, tableId, seq, errCode);
		CHECK_ERR_BREAK_WF(0, iRetCode, "HandleDBMsgRes Failed RunedTimes:%d. GlobalID:%d Type:%d(%s)", m_wRunedTimes, GetGlobalId(), GetClassType(), GetClassName().c_str());
	}
	while (false);

	if (iRetCode != 0)
	{
		m_iRunLogicRetCode = iRetCode;
		SetFinished(iRetCode);
	}

	return 0;
}

int NFTransBase::HandleDBMsgRes(const google::protobuf::Message* pSsMsgRes, uint32_t cmd, uint32_t tableId, uint32_t seq, int32_t errCode)
{
	NFLogFatal(NF_LOG_DEFAULT, 0, "Fatal Err Calling the Method");
	NF_ASSERT(false);
	return -1;
}

int NFTransBase::ProcessDispSvrRes(uint32_t nMsgId, const NFDataPackage& packet, uint32_t reqTransId, uint32_t rspTransId)
{
	int iRetCode = 0;
	do
	{
		iRetCode = RunCommLogic();
		CHECK_ERR_BREAK_WF(0, iRetCode, "RunCommLogic Failed RunedTimes:%d. GlobalID:%d Type:%d(%s)", m_wRunedTimes, GetGlobalId(), GetClassType(), GetClassName().c_str());

		iRetCode = HandleDispSvrRes(nMsgId, packet, reqTransId, rspTransId);
		CHECK_ERR_BREAK_WF(0, iRetCode, "HandleDispSvrRes Failed RunedTimes:%d. GlobalID:%d Type:%d(%s)", m_wRunedTimes, GetGlobalId(), GetClassType(), GetClassName().c_str());
	}
	while (false);

	if (iRetCode != 0)
	{
		m_iRunLogicRetCode = iRetCode;
		SetFinished(iRetCode);
	}

	return 0;
}

int NFTransBase::HandleDispSvrRes(uint32_t nMsgId, const NFDataPackage& packet, uint32_t reqTransId, uint32_t rspTransId)
{
	NFLogError(NF_LOG_DEFAULT, 0, "the msg not handle, nMsgId:{} reqTransId:{} rspTransId:{}", nMsgId, reqTransId, rspTransId);
	NF_ASSERT(false);
	return -1;
}

bool NFTransBase::IsCanRelease() const
{
	if (IsFinished())
	{
		NFLogTrace(NF_LOG_DEFAULT, 0, "This Trans can release, {} return code:{}", DebugString(), m_iRunLogicRetCode);
		return true;
	}
	return false;
}

std::string NFTransBase::DebugString() const
{
	std::string str = NF_FORMAT("GlobalID:{} Type:{} ClassName:{} ItemCount:{} UsedCount:{} FreeCount:{} CurState:{} StartTime:{} ActiveTime:{} ActiveTimeOUt:{} RunedTimes:{}",
	                            GetGlobalId(), GetClassType(), GetClassName(), GetItemCount(), GetUsedCount(), GetFreeCount(), m_wCurState,
	                            NFTimeUtil::DateTimeToStr(static_cast<int>(m_dwStartTime)), NFTimeUtil::DateTimeToStr(static_cast<int>(m_dwActiveTime)), m_iActiveTimeOut, m_wRunedTimes)	;
	return str;
}

bool NFTransBase::IsTimeOut()
{
	if (m_rpcId > 0 && !FindModule<NFICoroutineModule>()->IsDead(m_rpcId))
	{
		return false;
	}

	if (NF_ADJUST_TIMENOW() >= m_dwActiveTime + m_iActiveTimeOut)
	{
		m_bIsSelfTransTimeout = true;
		NFLogError(NF_LOG_DEFAULT, 0, "This Trans Active TimeOut Info:{}", DebugString());
		OnTimeOut();
		return true;
	}

	if (NF_ADJUST_TIMENOW() >= m_dwStartTime + TRANS_TIMEOUT_VALUE)
	{
		m_bIsSelfTransTimeout = true;
		NFLogError(NF_LOG_DEFAULT, 0, "This Trans TimeOut Info:{}", DebugString());
		OnTimeOut();
		return true;
	}
	return false;
}

int NFTransBase::OnTimeOut()
{
	return 0;
}

int NFTransBase::CheckCanRunNow() const
{
	uint32_t dwMaxRunTimes = GetMaxRunTimes();
	if (dwMaxRunTimes <= 0)
	{
		dwMaxRunTimes = MAX_TRANS_RUNED_TIMES;
	}

	CHECK_EXPR(m_wRunedTimes < dwMaxRunTimes, -1, " RunedTiems Too Much:{} Max:{}", m_wRunedTimes,
	           MAX_TRANS_RUNED_TIMES);
	return 0;
}

int NFTransBase::RunCommLogic()
{
	int iRetCode = 0;
	iRetCode = CheckCanRunNow();

	if (iRetCode != 0)
	{
		m_iRunLogicRetCode = ERR_TRANS_RUNED_TOO_MUCH;
		return ERR_TRANS_RUNED_TOO_MUCH;
	}
	else
	{
		m_dwActiveTime = NF_TIMENOW();
		IncreaseRunTimes();
		return 0;
	}
}

int NFTransBase::HandleCSMsgReq(const google::protobuf::Message* pCsMsgReq)
{
	NFLogFatal(NF_LOG_DEFAULT, 0, "Fatal Err Calling the Method");
	NF_ASSERT(false);
	return -1;
}

int NFTransBase::ProcessTick()
{
	int iRetCode = Tick();
	if (0 != iRetCode)
	{
		LOG_WARN(0, iRetCode, "Tick failed");
	}
	return 0;
}
