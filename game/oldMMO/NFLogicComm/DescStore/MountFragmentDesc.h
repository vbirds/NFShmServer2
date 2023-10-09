#pragma once

#include "NFServerComm/NFServerCommon/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/mount_s.h"

#define MAX_MOUNT_FRAGMENT_NUM 20

class MountFragmentDesc : public NFIDescStore
{
public:
	MountFragmentDesc();
	virtual ~MountFragmentDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::E_MountFragment_s* GetDesc(int64_t id) const;
	proto_ff_s::E_MountFragment_s* GetDesc(int64_t id);
	int GetDescIndex(int id) const;
	const proto_ff_s::E_MountFragment_s* GetDescByIndex(int index) const;
	proto_ff_s::E_MountFragment_s* GetDescByIndex(int index);
public:
IMPL_RES_HASH_DESC(MountFragmentDesc, proto_ff_s::E_MountFragment_s, E_MountFragment, MAX_MOUNT_FRAGMENT_NUM);
DECLARE_IDCREATE(MountFragmentDesc);
};