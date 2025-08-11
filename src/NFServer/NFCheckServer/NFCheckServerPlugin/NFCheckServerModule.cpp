// -------------------------------------------------------------------------
//    @FileName         :    NFCheckServerModule.cpp
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFCheckServerModule
//    @Desc             :    NFShmXFrame检查服务器模块实现
//                          实现检查服务器模块的核心功能，包括系统健康检查和监控
//                          支持系统状态检查、性能监控和故障诊断
// -------------------------------------------------------------------------

#include "NFCheckServerModule.h"

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @brief 构造函数
 *
 * 初始化检查服务器模块，设置插件管理器
 *
 * @param p 插件管理器指针
 */
NFCheckServerModule::NFCheckServerModule(NFIPluginManager* p): NFICheckServerModule(p)
{
}

/**
 * @brief 析构函数
 *
 * 清理检查服务器模块资源
 */
NFCheckServerModule::~NFCheckServerModule()
{
}

/**
 * @brief 模块唤醒
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 绑定服务器
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCheckServerModule::Awake()
{
    BindServer();
    return 0;
}

/**
 * @brief 模块初始化
 *
 * 初始化检查服务器模块，包括连接Master服务器等
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFCheckServerModule::Init()
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
int NFCheckServerModule::Tick()
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
int NFCheckServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_CHECK_SERVER);
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
int NFCheckServerModule::OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet)
{
    int retCode = 0;
    /*switch (packet.nMsgId)
    {
        default:
            NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) not handle", packet.ToString());
        break;
    }*/

    if (retCode != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "msg:({}) handle exist error", packet.ToString());
    }
    return 0;
}
