// -------------------------------------------------------------------------
//    @FileName         :    NFSnsRelationPart.cpp
//    @Author           :    gaoyi
//    @Date             :    23-10-20
//    @Email			:    445267987@qq.com
//    @Module           :    NFSnsRelationPart
//
// -------------------------------------------------------------------------

#include "NFSnsRelationPart.h"
#include "NFLogicCommon/NFLogicShmTypeDefines.h"

IMPLEMENT_IDCREATE_WITHTYPE(NFSnsRelationPart, EOT_SNS_PART_ID+SNS_PART_RELATION, NFSnsPart)

NFSnsRelationPart::NFSnsRelationPart()
{
    if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
    {
        CreateInit();
    }
    else
    {
        ResumeInit();
    }
}

NFSnsRelationPart::~NFSnsRelationPart()
{
}

int NFSnsRelationPart::CreateInit()
{
    return 0;
}

int NFSnsRelationPart::ResumeInit()
{
    return 0;
}