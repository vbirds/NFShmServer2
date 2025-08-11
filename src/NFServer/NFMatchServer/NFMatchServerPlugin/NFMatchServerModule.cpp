// -------------------------------------------------------------------------
//    @FileName         :    NFMatchServerModule.cpp
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFMatchServerModule
//    @Desc             :    NFShmXFrame匹配服务器模块实现
//                          实现匹配服务器模块的核心功能，包括玩家匹配和队列管理
//                          支持匹配算法、队列管理和匹配结果处理
// -------------------------------------------------------------------------

#include "NFMatchServerModule.h"

#include <NFServerComm/NFServerCommon/NFServerCommonDefine.h>
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @brief 构造函数
 *
 * 初始化匹配服务器模块，设置插件管理器
 *
 * @param p 插件管理器指针
 */
NFMatchServerModule::NFMatchServerModule(NFIPluginManager* p): NFIMatchServerModule(p)
{
}

/**
 * @brief 析构函数
 *
 * 清理匹配服务器模块资源
 */
NFMatchServerModule::~NFMatchServerModule()
{
}

/**
 * @brief 模块唤醒
 *
 * 在模块初始化完成后调用，进行必要的初始化工作：
 * - 绑定服务器，建立网络监听
 * - 为匹配服务器创建网络连接端点
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFMatchServerModule::Awake()
{
    BindServer();
    return 0;
}

/**
 * @brief 模块初始化
 *
 * 初始化匹配服务器模块，包括连接Master服务器等：
 * - 连接到Master服务器进行注册
 * - 建立与其他服务器的通信链路
 * - 准备接收匹配请求
 *
 * @return 初始化结果状态码，0表示成功
 */
int NFMatchServerModule::Init()
{
    ConnectMasterServer();
    return 0;
}

/**
 * @brief 模块定时更新
 *
 * 每帧调用，处理模块的定时任务：
 * - 处理匹配队列
 * - 执行匹配算法
 * - 发送匹配结果
 * - 清理过期匹配请求
 *
 * @return 处理结果状态码，0表示成功
 */
int NFMatchServerModule::Tick()
{
    return 0;
}

/**
 * @brief 动态插件处理
 *
 * 处理动态插件的加载和卸载，关闭所有连接：
 * - 关闭所有网络连接
 * - 清理匹配队列
 * - 保存匹配状态
 *
 * @return 处理结果状态码，0表示成功
 */
int NFMatchServerModule::OnDynamicPlugin()
{
    FindModule<NFIMessageModule>()->CloseAllLink(NF_ST_MATCH_SERVER);
    return 0;
}

/**
 * @brief 处理来自服务器的信息
 *
 * 处理来自其他服务器的消息请求，根据消息ID进行路由分发：
 * - 解析消息类型和内容
 * - 根据消息ID分发到对应的处理函数
 * - 处理匹配请求、取消匹配、查询匹配状态等
 * - 返回处理结果
 *
 * @param unLinkId 连接ID，标识消息来源的连接
 * @param packet 数据包，包含消息内容和元数据
 * @return 处理结果状态码，0表示成功
 */
int NFMatchServerModule::OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet)
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