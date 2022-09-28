#pragma once

#include "NFServerComm/NFDescStorePlugin/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/conditionshop_s.h"

#define MAX_CONDITIONSHOP_CONDITIONSHOP_NUM 20

class ConditionshopConditionshopDesc : public NFIDescStore
{
public:
	ConditionshopConditionshopDesc(NFIPluginManager* pPluginManager);
	virtual ~ConditionshopConditionshopDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::conditionshopconditionshop_s* GetDesc(int id) const;
	proto_ff_s::conditionshopconditionshop_s* GetDesc(int id);
	const NFShmHashMap<uint64_t, proto_ff_s::conditionshopconditionshop_s, MAX_CONDITIONSHOP_CONDITIONSHOP_NUM>* GetAllDesc() const { return &m_astDesc; }
	NFShmHashMap<uint64_t, proto_ff_s::conditionshopconditionshop_s, MAX_CONDITIONSHOP_CONDITIONSHOP_NUM>* GetAllDesc() { return &m_astDesc; }
public:
IMPL_RES_HASH_DESC(proto_ff_s::conditionshopconditionshop_s, conditionshopconditionshop, MAX_CONDITIONSHOP_CONDITIONSHOP_NUM);
DECLARE_IDCREATE(ConditionshopConditionshopDesc);
};