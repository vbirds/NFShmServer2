#pragma once

#include "NFServerComm/NFServerCommon/NFIDescStore.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "NFLogicCommon/NFDescStoreTypeDefines.h"
#include "NFServerLogicMessage/pet_s.h"

#define MAX_PET_DECOMPOSE_NUM 20

class PetDecomposeDesc : public NFIDescStore
{
public:
	PetDecomposeDesc();
	virtual ~PetDecomposeDesc();
	int CreateInit();
	int ResumeInit();
	const proto_ff_s::E_PetDecompose_s* GetDesc(int64_t id) const;
	proto_ff_s::E_PetDecompose_s* GetDesc(int64_t id);
	int GetDescIndex(int id) const;
	const proto_ff_s::E_PetDecompose_s* GetDescByIndex(int index) const;
	proto_ff_s::E_PetDecompose_s* GetDescByIndex(int index);
public:
IMPL_RES_HASH_DESC(PetDecomposeDesc, proto_ff_s::E_PetDecompose_s, E_PetDecompose, MAX_PET_DECOMPOSE_NUM);
DECLARE_IDCREATE(PetDecomposeDesc);
};