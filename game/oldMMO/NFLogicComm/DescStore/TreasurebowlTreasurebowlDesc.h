#pragma once

#include "NFServerComm/NFDescStorePlugin/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/treasureBowl_s.h"

#define MAX_TREASUREBOWL_TREASUREBOWL_NUM 20

class TreasurebowlTreasurebowlDesc : public NFIDescStore
{
public:
	TreasurebowlTreasurebowlDesc(NFIPluginManager* pPluginManager);
	virtual ~TreasurebowlTreasurebowlDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::treasureBowltreasureBowl_s* GetDesc(int id) const;
	proto_ff_s::treasureBowltreasureBowl_s* GetDesc(int id);
	const NFShmHashMap<uint64_t, proto_ff_s::treasureBowltreasureBowl_s, MAX_TREASUREBOWL_TREASUREBOWL_NUM>* GetAllDesc() const { return &m_astDesc; }
	NFShmHashMap<uint64_t, proto_ff_s::treasureBowltreasureBowl_s, MAX_TREASUREBOWL_TREASUREBOWL_NUM>* GetAllDesc() { return &m_astDesc; }
public:
IMPL_RES_HASH_DESC(proto_ff_s::treasureBowltreasureBowl_s, treasureBowltreasureBowl, MAX_TREASUREBOWL_TREASUREBOWL_NUM);
DECLARE_IDCREATE(TreasurebowlTreasurebowlDesc);
};