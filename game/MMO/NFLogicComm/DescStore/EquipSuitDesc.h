#pragma once

#include "NFServerComm/NFDescStorePlugin/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/equip_s.h"

#define MAX_EQUIP_SUIT_NUM 120

class EquipSuitDesc : public NFIDescStore
{
public:
	EquipSuitDesc();
	virtual ~EquipSuitDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::E_EquipSuit_s* GetDesc(int id) const;
	proto_ff_s::E_EquipSuit_s* GetDesc(int id);
public:
IMPL_RES_HASH_DESC(proto_ff_s::E_EquipSuit_s, E_EquipSuit, MAX_EQUIP_SUIT_NUM);
DECLARE_IDCREATE(EquipSuitDesc);
};