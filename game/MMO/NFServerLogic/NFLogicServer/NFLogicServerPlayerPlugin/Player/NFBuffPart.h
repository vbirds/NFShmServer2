// -------------------------------------------------------------------------
//    @FileName         :    NFBuffPart.h
//    @Author           :    gaoyi
//    @Date             :    23-10-20
//    @Email			:    445267987@qq.com
//    @Module           :    NFBuffPart
//
// -------------------------------------------------------------------------

#pragma once


#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFShmCore/NFShmObj.h"
#include "NFComm/NFShmCore/NFShmMgr.h"
#include "Part/NFPart.h"
#include "NFLogicCommon/NFLogicShmTypeDefines.h"
#include "NFComm/NFShmCore/NFISharedMemModule.h"

class NFBuffPart : public NFPart
{
public:
    NFBuffPart();

    virtual ~NFBuffPart();

    int CreateInit();

    int ResumeInit();
public:
    //��ȡս������
    int64_t GetFightAttr(uint32_t ANum) { return 0;}
private:
DECLARE_IDCREATE(NFBuffPart)
};