// -------------------------------------------------------------------------
//    @FileName         :    NFOnlineServerModule.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFOnlineServerModule
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFServerComm/NFServerCommon/NFIOnlineServerModule.h"

class NFOnlineServerModule : public NFIOnlineServerModule
{
public:
    explicit NFOnlineServerModule(NFIPluginManager *p);

    virtual ~NFOnlineServerModule();

    virtual int Awake() override;

    virtual int Init() override;

    virtual int Tick() override;

    virtual int OnDynamicPlugin() override;

    /**
     * @brief 处理来自服务器的信息
     * @param unLinkId
     * @param packet
     * @return
     */
    virtual int OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet) override;
};