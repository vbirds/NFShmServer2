// -------------------------------------------------------------------------
//    @FileName         :    NFCBusMessage.cpp
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFBusPlugin
//    @Desc             :    Bus消息管理实现文件，负责Bus通信系统的消息路由和处理
//
// -------------------------------------------------------------------------

#include "NFCBusMessage.h"
#include <sstream>
#include <string.h>
#include "NFCBusClient.h"
#include "NFCBusServer.h"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFNetPackagePool.h"

/**
 * @file NFCBusMessage.cpp
 * @brief Bus消息管理实现文件
 * 
 * 该文件实现了Bus通信系统的消息管理功能，包括：
 * - Bus消息管理类的初始化和生命周期管理
 * - 服务器和客户端连接的管理
 * - 消息路由和分发处理
 * - 心跳机制和连接状态监控
 * - 消息发送和接收的统一接口
 * 
 * 主要功能：
 * - 统一的消息管理接口
 * - 支持服务器和客户端模式
 * - 自动心跳检测
 * - 连接状态监控
 * - 消息路由分发
 * 
 * @author Yi.Gao
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief Bus消息管理构造函数
 * 
 * 初始化Bus消息管理系统，包括：
 * - 设置绑定连接为空
 * - 初始化心跳定时器
 * - 设置心跳检查定时器
 * 
 * @param p 插件管理器指针
 * @param serverType 服务器类型
 */
NFCBusMessage::NFCBusMessage(NFIPluginManager* p, NF_SERVER_TYPE serverType) : NFINetMessage(p, serverType)
{
    m_bindConnect = nullptr;
#ifdef NF_DEBUG_MODE
    SetTimer(ENUM_SERVER_CLIENT_TIMER_HEART, ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 3);
    SetTimer(ENUM_SERVER_TIMER_CHECK_HEART, ENUM_SERVER_TIMER_CHECK_HEART_TIME_LONGTH);
#else
    SetTimer(ENUM_SERVER_CLIENT_TIMER_HEART, ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH*3);
    SetTimer(ENUM_SERVER_TIMER_CHECK_HEART, ENUM_SERVER_TIMER_CHECK_HEART_TIME_LONGTH);
#endif
}

/**
 * @brief Bus消息管理析构函数
 * 
 * 清理消息管理资源
 */
NFCBusMessage::~NFCBusMessage()
{
}

/**
 * @brief 定时器更新函数
 * 
 * 处理绑定连接的定时更新
 * 
 * @return 处理结果，0表示成功
 */
int NFCBusMessage::Tick()
{
    if (m_bindConnect)
    {
        m_bindConnect->Tick();
    }
    return 0;
}

/**
 * @brief 关闭消息管理
 * 
 * 关闭所有Bus连接
 * 
 * @return 关闭结果，0表示成功
 */
int NFCBusMessage::Shut()
{
    auto pConn = m_busConnectMap.First();
    while (pConn)
    {
        pConn->Shut();
        pConn = m_busConnectMap.Next();
    }

    return 0;
}

/**
 * @brief 最终化消息管理
 * 
 * 清理所有Bus连接资源
 * 
 * @return 最终化结果，0表示成功
 */
int NFCBusMessage::Finalize()
{
    auto pConn = m_busConnectMap.First();
    while (pConn)
    {
        pConn->Finalize();
        pConn = m_busConnectMap.Next();
    }
    m_bindConnect = nullptr;
    m_busConnectMap.ClearAll();

    return 0;
}

/**
 * @brief 准备就绪更新函数
 * 
 * @return 处理结果，0表示成功
 */
int NFCBusMessage::ReadyTick()
{
    return 0;
}

/**
 * @brief 绑定服务器
 * 
 * 创建并初始化Bus服务器，包括：
 * - 创建Bus服务器实例
 * - 设置消息对等回调函数
 * - 初始化服务器连接
 * - 添加到连接映射表
 * 
 * @param flag 消息标志，包含服务器配置信息
 * @return 绑定结果，成功返回服务器ID，失败返回0
 */
uint64_t NFCBusMessage::BindServer(const NFMessageFlag& flag)
{
    CHECK_EXPR(m_bindConnect == NULL, 0, "BindServer Failed!");
    NF_SHARE_PTR<NFCBusServer> pServer = std::make_shared<NFCBusServer>(m_pObjPluginManager, m_serverType, flag);
    CHECK_NULL(0, pServer);

    pServer->SetMsgPeerCallback(std::bind(&NFCBusMessage::OnHandleMsgPeer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                          std::placeholders::_4));


    int iRet = pServer->Init();
    CHECK_ERR(0, iRet, "NFCBusServer Init Failed!");
    m_busConnectMap.AddElement(pServer->GetLinkId(), pServer);

    m_bindConnect = pServer;
    ResumeConnect();
    return pServer->GetLinkId();
}

/**
 * @brief 连接服务器
 * 
 * 创建并初始化Bus客户端连接，包括：
 * - 创建Bus客户端实例
 * - 设置消息对等回调函数
 * - 初始化客户端连接
 * - 添加到连接映射表
 * 
 * @param flag 消息标志，包含服务器配置信息
 * @return 连接结果，成功返回连接ID，失败返回0
 */
uint64_t NFCBusMessage::ConnectServer(const NFMessageFlag& flag)
{
    CHECK_EXPR(m_bindConnect, 0, "ConnectServer Failed, muset bindserver");

    NF_SHARE_PTR<NFCBusClient> pConn = std::make_shared<NFCBusClient>(m_pObjPluginManager, m_serverType, flag, m_bindConnect->GetBindFlag());
    CHECK_NULL(0, pConn);

    int iRet = pConn->Init();
    CHECK_ERR(0, iRet, "NFCBusClient Init Failed");
    m_busConnectMap.AddElement(pConn->GetLinkId(), pConn);

    return pConn->GetLinkId();
}

/**
 * @brief 发送原始数据
 * 
 * 向指定连接发送原始数据
 * 
 * @param usLinkId 连接ID
 * @param packet 数据包
 * @param msg 消息数据
 * @param nLen 数据长度
 * @return 发送结果
 */
bool NFCBusMessage::Send(uint64_t usLinkId, NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    auto pConn = m_busConnectMap.GetElement(usLinkId);

    if (pConn)
    {
        return pConn->Send(packet, msg, nLen);
    }

    NFLogError(NF_LOG_DEFAULT, 0, "usLinkId:{} not find", usLinkId);
    return false;
}

/**
 * @brief 发送Protobuf消息
 * 
 * 向指定连接发送Protobuf格式的消息
 * 
 * @param usLinkId 连接ID
 * @param packet 数据包
 * @param xData Protobuf消息对象
 * @return 发送结果
 */
bool NFCBusMessage::Send(uint64_t usLinkId, NFDataPackage& packet, const google::protobuf::Message& xData)
{
    auto pConn = m_busConnectMap.GetElement(usLinkId);

    if (pConn)
    {
        return pConn->Send(packet, xData);
    }

    NFLogError(NF_LOG_DEFAULT, 0, "usLinkId:{} not find", usLinkId);
    return false;
}

/**
 * @brief 获得连接IP地址
 * 
 * 获取指定连接的IP地址信息
 * 
 * @param usLinkId 连接ID
 * @return IP地址字符串
 */
std::string NFCBusMessage::GetLinkIp(uint64_t usLinkId)
{
    auto pConn = m_busConnectMap.GetElement(usLinkId);
    CHECK_EXPR(pConn, "", "usLinkId:{} not find", usLinkId);

    return pConn->GetLinkIp();
}

/**
 * @brief 获取连接端口
 * 
 * Bus连接不使用端口，返回0
 * 
 * @param usLinkId 连接ID
 * @return 端口号，Bus连接返回0
 */
uint32_t NFCBusMessage::GetPort(uint64_t usLinkId)
{
    return 0;
}

/**
 * @brief 关闭连接
 * 
 * 关闭指定的连接
 * 
 * @param usLinkId 连接ID
 */
void NFCBusMessage::CloseLinkId(uint64_t usLinkId)
{
    auto pConn = m_busConnectMap.GetElement(usLinkId);
    CHECK_EXPR(pConn, , "usLinkId:{} not find", usLinkId);

    return pConn->CloseLinkId();
}

/**
 * @brief 处理对等消息
 * 
 * 处理来自其他Bus连接的消息，包括：
 * - 心跳消息处理
 * - 连接状态管理
 * - 消息路由分发
 * 
 * @param type 消息类型
 * @param serverLinkId 服务器连接ID
 * @param objectLinkId 对象连接ID
 * @param package 数据包
 */
void NFCBusMessage::OnHandleMsgPeer(eMsgType type, uint64_t serverLinkId, uint64_t objectLinkId, NFDataPackage& package)
{
    if (!NFGlobalSystem::Instance()->IsSpecialMsg(package.mModuleId, package.nMsgId))
    {
        NFLogTrace(NF_LOG_DEFAULT, 0, "recv msg:{} ", package.ToString());
    }

    uint32_t fromBusId = (key_t)GetBusIdFromUnlinkId(objectLinkId);
    uint64_t fromLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, fromBusId, 0);
    switch (type)
    {
    case eMsgType_RECIVEDATA:
        {
            if (package.mModuleId == NF_MODULE_FRAME)
            {
                if (package.nMsgId == NFrame::NF_SERVER_TO_SERVER_HEART_BEAT)
                {
                    auto pConn = m_busConnectMap.GetElement(fromLinkId);
                    if (pConn)
                    {
                        pConn->SetLastHeartBeatTime(NFGetTime());
                        pConn->SendBusHeartBeatRspMsg(m_bindConnect->GetBusId(), m_bindConnect->GetBusLength());
                        return;
                    }
                    else
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "BusMessage OnHandleMsgPeer Error, busId:{} can't find",
                                   NFServerIDUtil::GetBusNameFromBusID(fromBusId));
                    }
                }

                if (package.nMsgId == NFrame::NF_SERVER_TO_SERVER_HEART_BEAT_RSP)
                {
                    auto pConn = m_busConnectMap.GetElement(fromLinkId);
                    if (pConn)
                    {
                        pConn->SetLastHeartBeatTime(NFGetTime());
                        return;
                    }
                    else
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "BusMessage OnHandleMsgPeer Error, busId:{} can't find",
                                   NFServerIDUtil::GetBusNameFromBusID(fromBusId));
                    }
                }
            }

            if (m_recvCb)
            {
                auto pConn = m_busConnectMap.GetElement(fromLinkId);
                if (pConn)
                {
                    package.nServerLinkId = m_bindConnect->GetLinkId();
                    package.nObjectLinkId = pConn->GetLinkId();
                    m_recvCb(m_bindConnect->GetLinkId(), pConn->GetLinkId(), package);
                }
                else
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "BusMessage OnHandleMsgPeer Error, busId:{} can't find",
                               NFServerIDUtil::GetBusNameFromBusID(fromBusId));
                }
            }
        }
        break;
    case eMsgType_CONNECTED:
        {
            NFMessageFlag flag;
            flag.mBusId = package.nParam1;
            flag.mBusLength = package.nParam2;
            flag.bActivityConnect = false;
            if (static_cast<uint64_t>(fromBusId) != flag.mBusId)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "BusMessage fromBusId:{} != busId:{}", fromBusId, flag.mBusId);
            }

            auto pConn = m_busConnectMap.GetElement(fromLinkId);
            if (pConn == nullptr)
            {
                ConnectServer(flag);
                pConn = m_busConnectMap.GetElement(fromLinkId);
                CHECK_EXPR_MSG(pConn, "m_busConnectMap.GetElement busId:{} == NULL", flag.mBusId);
            }

            if (pConn)
            {
                pConn->SetConnected(true);
                if (package.mModuleId == NF_MODULE_FRAME && package.nMsgId == NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_REQ)
                {
                    pConn->SendBusConnectRspMsg(m_bindConnect->GetBusId(), m_bindConnect->GetBusLength());
                    if (m_eventCb)
                    {
                        package.nServerLinkId = m_bindConnect->GetLinkId();
                        package.nObjectLinkId = pConn->GetLinkId();
                        m_eventCb(type, m_bindConnect->GetLinkId(), pConn->GetLinkId());
                    }
                }
                else
                {
                    if (m_eventCb)
                    {
                        package.nServerLinkId = m_bindConnect->GetLinkId();
                        package.nObjectLinkId = pConn->GetLinkId();
                        m_eventCb(type, pConn->GetLinkId(), pConn->GetLinkId());
                    }
                }
            }
        }
        break;
    case eMsgType_DISCONNECTED:
        {
            auto pConn = m_busConnectMap.GetElement(fromLinkId);
            if (pConn)
            {
                if (m_eventCb)
                {
                    package.nServerLinkId = m_bindConnect->GetLinkId();
                    package.nObjectLinkId = pConn->GetLinkId();
                    m_eventCb(type, serverLinkId, pConn->GetLinkId());
                }
            }
        }
        break;
    default:
        break;
    }
}

/**
 * @brief 恢复连接
 * 
 * 恢复所有断开的连接，包括：
 * - 检查共享内存中的连接信息
 * - 重新建立断开的连接
 * - 更新连接状态
 * 
 * @return 恢复结果，0表示成功
 */
int NFCBusMessage::ResumeConnect()
{
    CHECK_NULL(0, m_bindConnect);
    CHECK_NULL(0, m_bindConnect->GetShmRecord());

    auto head = (NFShmChannelHead*)m_bindConnect->GetShmRecord()->m_nBuffer;
    CHECK_NULL(0, head);
    CHECK_EXPR(head->m_nShmAddr.m_dstLinkId == m_bindConnect->GetShmRecord()->m_nUnLinkId, -1,
               "head->m_nShmAddr.mDstLinkId == m_nShmBindRecord.m_nUnLinkId");

    for (size_t i = 0; i < ARRAYSIZE(head->m_nShmAddr.m_srcLinkId); i++)
    {
        if (head->m_nShmAddr.m_srcLinkId[i] > 0)
        {
            uint64_t linkId = head->m_nShmAddr.m_srcLinkId[i];
            NFMessageFlag flag;
            flag.mBusId = GetBusIdFromUnlinkId(linkId);
            flag.mBusLength = head->m_nShmAddr.m_srcBusLength[i];
            flag.mPacketParseType = head->m_nShmAddr.m_srcParseType[i];
            bool bActivityConnect = head->m_nShmAddr.m_bActiveConnect[i];
            flag.bActivityConnect = !bActivityConnect;
            uint32_t serverType = GetServerTypeFromUnlinkId(linkId);

            NF_SHARE_PTR<NFServerData> pServerData = FindModule<NFIMessageModule>()->GetServerByServerId(m_serverType, flag.mBusId);
            if (!pServerData)
            {
                NFrame::ServerInfoReport xData;
                xData.set_bus_id(flag.mBusId);
                xData.set_bus_length(flag.mBusLength);
                xData.set_link_mode("bus");
                xData.set_server_type(serverType);
                std::string busName = NFServerIDUtil::GetBusNameFromBusID(flag.mBusId);
                std::string url = NF_FORMAT("bus://{}:{}", busName, flag.mBusLength);
                xData.set_url(url);
                pServerData = FindModule<NFIMessageModule>()->CreateServerByServerId(m_serverType, flag.mBusId, static_cast<NF_SERVER_TYPE>(serverType), xData);

                pServerData->mUnlinkId = GetUnLinkId(NF_IS_BUS, m_serverType, flag.mBusId, 0);
                FindModule<NFIMessageModule>()->CreateLinkToServer(m_serverType, flag.mBusId, pServerData->mUnlinkId);
                if (bActivityConnect)
                {
                    NFDataPackage packet;
                    packet.nSendBusLinkId = GetUnLinkId(NF_IS_BUS, serverType, flag.mBusId, 0);
                    packet.mModuleId = NF_MODULE_FRAME;
                    packet.nMsgId = NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_REQ;
                    packet.nParam1 = flag.mBusId;
                    packet.nParam2 = flag.mBusLength;

                    OnHandleMsgPeer(eMsgType_CONNECTED, pServerData->mUnlinkId, pServerData->mUnlinkId, packet);
                }
                else
                {
                    uint64_t connectLinkId = ConnectServer(flag);
                    if (connectLinkId != pServerData->mUnlinkId)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "ReConnect Server Error, connectLinkId:{} != pServerData->mUnlinkId:{}", connectLinkId,
                                   pServerData->mUnlinkId);
                    }
                }
            }
        }
    }

    return 0;
}

/**
 * @brief 定时器回调函数
 * 
 * 处理各种定时器事件，包括：
 * - 心跳消息发送
 * - 服务器心跳检查
 * 
 * @param nTimerId 定时器ID
 * @return 处理结果，0表示成功
 */
int NFCBusMessage::OnTimer(uint32_t nTimerId)
{
    if (nTimerId == ENUM_SERVER_CLIENT_TIMER_HEART)
    {
        SendHeartMsg();
    }
    else if (nTimerId == ENUM_SERVER_TIMER_CHECK_HEART)
    {
        CheckServerHeartBeat();
    }
    return 0;
}

/**
 * @brief 发送心跳消息
 * 
 * 向所有活跃的客户端连接发送心跳消息
 * 保持连接活跃状态
 */
void NFCBusMessage::SendHeartMsg()
{
    auto pConn = m_busConnectMap.First();
    while (pConn)
    {
        if (pConn->IsActivityConnect() && pConn->GetConnectionType() == NF_CONNECTION_TYPE_TCP_CLIENT)
        {
            pConn->SendBusHeartBeatMsg(m_bindConnect->GetBusId(), m_bindConnect->GetBusLength());
        }
        pConn = m_busConnectMap.Next();
    }
}

/**
 * @brief 检查服务器心跳
 * 
 * 检查所有服务器连接的心跳状态
 * 清理超时的连接
 */
void NFCBusMessage::CheckServerHeartBeat()
{
    uint64_t nowTime = NFGetTime();
    auto pConn = m_busConnectMap.First();
    while (pConn)
    {
        if (pConn->GetConnectionType() == NF_CONNECTION_TYPE_TCP_CLIENT)
        {
            if (pConn->IsActivityConnect())
            {
                //debug 30min
#ifdef NF_DEBUG_MODE
                if (pConn->GetLastHeartBeatTime() > 0 && nowTime - pConn->GetLastHeartBeatTime() > ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 20 * 60)
                {
                    pConn->CloseLinkId();

                    NFDataPackage packet;
                    packet.nSendBusLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    packet.nServerLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, m_bindConnect->GetBusId(), 0);
                    packet.nObjectLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    OnHandleMsgPeer(eMsgType_DISCONNECTED, packet.nObjectLinkId, packet.nObjectLinkId, packet);
                }
#else
                if (pConn->GetLastHeartBeatTime() > 0 && nowTime - pConn->GetLastHeartBeatTime() > ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 20)
                {
                    pConn->CloseLinkId();

                    NFDataPackage packet;
                    packet.nSendBusLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    packet.nServerLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, m_bindConnect->GetBusId(), 0);
                    packet.nObjectLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    OnHandleMsgPeer(eMsgType_DISCONNECTED, packet.nObjectLinkId, packet.nObjectLinkId, packet);
                }
#endif
            }
            else
            {
#ifdef NF_DEBUG_MODE
                if (pConn->GetLastHeartBeatTime() > 0 && nowTime - pConn->GetLastHeartBeatTime() > ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 20 * 60)
                {
                    pConn->CloseLinkId();

                    NFDataPackage packet;
                    packet.nSendBusLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    packet.nServerLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, m_bindConnect->GetBusId(), 0);
                    packet.nObjectLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    OnHandleMsgPeer(eMsgType_DISCONNECTED, packet.nServerLinkId, packet.nObjectLinkId, packet);
                }
#else
                if (pConn->GetLastHeartBeatTime() > 0 && nowTime - pConn->GetLastHeartBeatTime() > ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 20)
                {
                    pConn->CloseLinkId();

                    NFDataPackage packet;
                    packet.nSendBusLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    packet.nServerLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, m_bindConnect->GetBusId(), 0);
                    packet.nObjectLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, pConn->GetBusId(), 0);
                    OnHandleMsgPeer(eMsgType_DISCONNECTED, packet.nServerLinkId, packet.nObjectLinkId, packet);
                }
#endif
            }
        }
        pConn = m_busConnectMap.Next();
    }
}

