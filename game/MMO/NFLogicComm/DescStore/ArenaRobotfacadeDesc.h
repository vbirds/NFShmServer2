#pragma once

#include "NFServerComm/NFDescStorePlugin/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/arena_s.h"

#define MAX_ARENA_ROBOTFACADE_NUM 20

class ArenaRobotfacadeDesc : public NFIDescStore
{
public:
	ArenaRobotfacadeDesc();
	virtual ~ArenaRobotfacadeDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::E_ArenaRobotfacade_s* GetDesc(int id) const;
	proto_ff_s::E_ArenaRobotfacade_s* GetDesc(int id);
public:
IMPL_RES_HASH_DESC(proto_ff_s::E_ArenaRobotfacade_s, E_ArenaRobotfacade, MAX_ARENA_ROBOTFACADE_NUM);
DECLARE_IDCREATE(ArenaRobotfacadeDesc);
};