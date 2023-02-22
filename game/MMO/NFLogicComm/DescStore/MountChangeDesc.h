#pragma once

#include "NFServerComm/NFDescStorePlugin/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/mount_s.h"

#define MAX_MOUNT_CHANGE_NUM 20

class MountChangeDesc : public NFIDescStore
{
public:
	MountChangeDesc();
	virtual ~MountChangeDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::E_MountChange_s* GetDesc(int id) const;
	proto_ff_s::E_MountChange_s* GetDesc(int id);
public:
IMPL_RES_HASH_DESC(proto_ff_s::E_MountChange_s, E_MountChange, MAX_MOUNT_CHANGE_NUM);
DECLARE_IDCREATE(MountChangeDesc);
};