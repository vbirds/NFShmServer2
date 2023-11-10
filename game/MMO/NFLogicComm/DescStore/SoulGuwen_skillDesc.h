#pragma once

#include "NFServerComm/NFServerCommon/NFIDescStore.h"
#include "NFServerComm/NFServerCommon/NFIDescTemplate.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/E_Soul_s.h"

#define MAX_SOUL_GUWEN_SKILL_NUM 8

class SoulGuwen_skillDesc : public NFIDescTemplate<SoulGuwen_skillDesc, proto_ff_s::E_SoulGuwen_skill_s, EOT_CONST_SOUL_GUWEN_SKILL_DESC_ID, MAX_SOUL_GUWEN_SKILL_NUM>
{
public:
	SoulGuwen_skillDesc();
	virtual ~SoulGuwen_skillDesc();
	int CreateInit();
	int ResumeInit();
public:
	virtual int Load(NFResDB *pDB) override;
	virtual int CheckWhenAllDataLoaded() override;
};