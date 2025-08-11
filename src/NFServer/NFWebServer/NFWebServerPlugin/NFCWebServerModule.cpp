// -------------------------------------------------------------------------
//    @FileName         :    NFCWebServerModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCWebServerModule
//    @Desc             :    NFShmXFrame Web服务器模块实现
//                          实现Web服务器模块的核心功能，包括HTTP服务和Web管理
//                          支持HTTP请求处理、Web界面管理和API服务
// -------------------------------------------------------------------------

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFCWebServerModule.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFServerComm/NFServerMessage/ServerMsg.pb.h"

/**
 * @brief 构造函数
 *
 * 初始化Web服务器模块，设置插件管理器
 *
 * @param p 插件管理器指针
 */
NFCWebServerModule::NFCWebServerModule(NFIPluginManager* p):NFIWebServerModule(p)
{
}

/**
 * @brief 析构函数
 *
 * 清理Web服务器模块资源
 */
NFCWebServerModule::~NFCWebServerModule()
{
}

/**
 * @brief 模块唤醒
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 获取Web服务器配置
 * - 绑定HTTP服务器
 * - 绑定服务器
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCWebServerModule::Awake() {
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_WEB_SERVER);
    if (pConfig) {
        std::string httpUrl = NF_FORMAT("http://{}:{}", pConfig->ServerIp, pConfig->HttpPort);
        uint64_t ret = FindModule<NFIMessageModule>()->BindServer(NF_ST_WEB_SERVER, httpUrl, pConfig->NetThreadNum, pConfig->MaxConnectNum, PACKET_PARSE_TYPE_INTERNAL);
        if (ret == 0)
        {
            NFLogInfo(NF_LOG_DEFAULT, 0, "web server listen http failed!, serverId:{}, ip:{}, httpport:{}", pConfig->ServerId, pConfig->ServerIp, pConfig->HttpPort);
            return -1;
        }

        BindServer();
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "I Can't get the web Server config!");
        return -1;
    }

    return 0;
}

/**
 * @brief 模块初始化
 *
 * 初始化Web服务器模块，包括连接Master服务器等
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCWebServerModule::Init()
{
    ConnectMasterServer();
    return 0;
}

/**
 * @brief 模块定时更新
 *
 * 每帧调用，处理模块的定时任务
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCWebServerModule::Tick()
{
    return 0;
}

/**
 * @brief 动态插件处理
 *
 * 处理动态插件的加载和卸载，关闭所有连接
 *
 * @return 处理结果状态码，0表示成功
 */
int NFCWebServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_WEB_SERVER);
    return 0;
}

/**
 * @brief 处理来自服务器的信息
 *
 * 处理来自其他服务器的消息请求，根据消息ID进行路由分发
 *
 * @param unLinkId 连接ID
 * @param packet 数据包
 * @return 处理结果状态码，0表示成功
 */
int NFCWebServerModule::OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    int retCode = 0;
    switch (packet.nMsgId)
    {
        default:
            NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) not handle", packet.ToString());
            break;
    }

    if (retCode != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) handle exist error", packet.ToString());
    }
    return 0;
}