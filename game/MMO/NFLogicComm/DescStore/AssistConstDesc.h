#pragma once

#include "NFServerComm/NFDescStorePlugin/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/assist_s.h"

#define MAX_ASSIST_CONST_NUM 20

class AssistConstDesc : public NFIDescStore
{
public:
	AssistConstDesc();
	virtual ~AssistConstDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::E_AssistConst_s* GetDesc(int id) const;
	proto_ff_s::E_AssistConst_s* GetDesc(int id);
public:
IMPL_RES_HASH_DESC(proto_ff_s::E_AssistConst_s, E_AssistConst, MAX_ASSIST_CONST_NUM);
DECLARE_IDCREATE(AssistConstDesc);
};