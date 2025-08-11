// -------------------------------------------------------------------------
//    @FileName         :    NFCTestActorModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTestPlugin
//
// -------------------------------------------------------------------------

#include "NFCTestModule.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIMysqlModule.h"

NFCTestModule::NFCTestModule(NFIPluginManager* p): NFIDynamicModule(p)
{
}

NFCTestModule::~NFCTestModule()
{
}


int NFCTestModule::Init()
{
	SetTimer(1, 1000, 1);
	return 0;
}


int NFCTestModule::OnTimer(uint32_t nTimerID)
{
    return 0;
}

int NFCTestModule::Tick()
{
	return 0;
}

int NFCTestModule::BeforeShut()
{
	return 0;
}

int NFCTestModule::Shut()
{
	return 0;
}
