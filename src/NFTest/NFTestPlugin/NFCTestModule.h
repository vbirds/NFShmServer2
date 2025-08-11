// -------------------------------------------------------------------------
//    @FileName         :    NFCTestActorModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTestPlugin
//
// -------------------------------------------------------------------------

#pragma once

#include <string>

#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFPluginModule/NFIDynamicModule.h"

class NFCTestModule : public NFIDynamicModule
{
public:
	explicit NFCTestModule(NFIPluginManager* p);
	virtual ~NFCTestModule();

	virtual int Init() override;

	virtual int Tick() override;

	virtual int BeforeShut() override;

	virtual int Shut() override;

	virtual int OnTimer(uint32_t nTimerID) override;
};
