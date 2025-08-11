// -------------------------------------------------------------------------
//    @FileName         :    NFCProxyClientModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCProxyClientModule
//    @Desc             :    NFShmXFrame代理客户端模块实现
//                          实现代理客户端模块的核心功能，包括客户端连接和消息处理
//                          支持客户端连接管理、消息路由和代理通信
// -------------------------------------------------------------------------

#include <NFComm/NFCore/NFTime.h>
#include <NFComm/NFPluginModule/NFCheck.h>
#include "NFProxyClientModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"

/**
 * @brief 构造函数
 *
 * 初始化代理客户端模块，设置插件管理器和客户端连接ID
 *
 * @param p 插件管理器指针
 */
NFCProxyClientModule::NFCProxyClientModule(NFIPluginManager *p) : NFIProxyClientModule(p)
{
    m_proxyClientLinkId = 0;
}

/**
 * @brief 析构函数
 *
 * 清理代理客户端模块资源
 */
NFCProxyClientModule::~NFCProxyClientModule()
{
}

/**
 * @brief 模块唤醒
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 验证服务器配置
 * - 绑定外部客户端监听
 * - 设置客户端连接ID
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCProxyClientModule::Awake()
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_PROXY_SERVER);
    if (pConfig)
    {
        if (!m_pObjPluginManager->IsLoadAllServer())
        {
            if (pConfig->ServerType != NF_ST_PROXY_SERVER)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "server config error, server id not match the server type!");
                exit(0);
            }
        }

        std::string externUrl = NF_FORMAT("tcp://{}:{}", pConfig->ExternalServerIp, pConfig->ExternalServerPort);
        uint64_t extern_unlinkId = FindModule<NFIMessageModule>()->BindServer(NF_ST_PROXY_SERVER, externUrl, pConfig->NetThreadNum,
                                                                              pConfig->MaxConnectNum,
                                                                              pConfig->ParseType, pConfig->Security);
        if (extern_unlinkId > 0)
        {
            /*
            注册服务器事件
            */
            m_proxyClientLinkId = extern_unlinkId;
            FindModule<NFIMessageModule>()->SetClientLinkId(NF_ST_PROXY_SERVER, extern_unlinkId);
            NFLogInfo(NF_LOG_DEFAULT, 0, "proxy client listen success, serverId:{}, ip:{}, port:{}",
                      pConfig->ServerId, pConfig->ExternalServerIp, pConfig->ExternalServerPort);
        }
        else
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "proxy client listen failed!, serverId:{}, ip:{}, port:{}",
                      pConfig->ServerId, pConfig->ExternalServerIp, pConfig->ExternalServerPort);
            return -1;
        }
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the Proxy Server config!");
        return -1;
    }

    return 0;
}

/**
 * @brief 模块初始化
 *
 * 初始化代理客户端模块，包括连接管理等
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCProxyClientModule::Init()
{
    return 0;
}

/**
 * @brief 模块定时更新
 *
 * 每帧调用，处理模块的定时任务
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCProxyClientModule::Tick()
{
    return 0;
}

/**
 * @brief 动态插件处理
 *
 * 处理动态插件的加载和卸载，关闭客户端连接
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCProxyClientModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseLinkId(m_proxyClientLinkId);
    return 0;
}