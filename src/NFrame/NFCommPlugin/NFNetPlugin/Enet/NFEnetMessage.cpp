// -------------------------------------------------------------------------
//    @FileName         :    NFEnetMessage.cpp
//    @Author           :    gaoyi
//    @Date             :    2025-03-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFEnetNetMessage
//    @Desc             :    基于ENet库的网络消息处理实现文件，提供UDP可靠传输连接管理和消息处理功能
//
// -------------------------------------------------------------------------

#include "NFEnetMessage.h"

#include <NFComm/NFPluginModule/NFCodeQueue.h>
#include <NFCommPlugin/NFNetPlugin/NFPacketParseMgr.h>

#include "EnetObject.h"
#include "NFEnetServer.h"
#include "NFEnetClient.h"

/**
 * @file NFEnetMessage.cpp
 * @brief Enet网络消息处理实现文件
 * 
 * 该文件实现了基于ENet库的网络消息处理功能，包括：
 * - 网络消息处理类的初始化和销毁
 * - 服务器绑定和客户端连接
 * - 网络对象的生命周期管理
 * - 消息接收和发送处理
 * - 连接事件回调处理
 * - 数据包解析和路由
 * - 定时器处理
 * 
 * 主要功能：
 * - 管理多个网络连接
 * - 处理网络事件和消息
 * - 提供消息发送接口
 * - 支持连接状态监控
 * - 自动资源管理
 * 
 * @author gaoyi
 * @date 2025-03-13
 * @version 1.0
 */

/**
 * @brief Enet网络消息处理类构造函数
 * 
 * 初始化网络消息处理模块，包括：
 * - 读取服务器配置
 * - 设置发送和接收缓冲区
 * - 设置定时器
 * - 初始化网络对象数组
 * - 设置消息处理参数
 * - 初始化空闲连接ID队列
 * 
 * @param p 插件管理器指针
 * @param serverType 服务器类型
 */
NFEnetMessage::NFEnetMessage(NFIPluginManager* p, NF_SERVER_TYPE serverType): NFINetMessage(p, serverType)
{
    // 读取服务器配置
    auto pServerConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT_NOT_RET(pServerConfig, "m_serverType:{} Config Not Find", m_serverType);

    // 设置发送和接收缓冲区
    m_sendBuffer.AssureSpace(MAX_SEND_BUFFER_SIZE);
    m_recvBuffer.AssureSpace(MAX_RECV_BUFFER_SIZE);
    m_sendComBuffer.AssureSpace(MAX_SEND_BUFFER_SIZE);
    m_recvCodeList.AssureSpace(MAX_RECV_BUFFER_SIZE);

    // 设置定时器
#ifdef NF_DEBUG_MODE
    SetTimer(ENUM_SERVER_CLIENT_TIMER_HEART, ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 3);
    SetTimer(ENUM_SERVER_TIMER_CHECK_HEART, ENUM_SERVER_TIMER_CHECK_HEART_TIME_LONGTH);
#else
    SetTimer(ENUM_SERVER_CLIENT_TIMER_HEART, ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH*3);
    SetTimer(ENUM_SERVER_TIMER_CHECK_HEART, ENUM_SERVER_TIMER_CHECK_HEART_TIME_LONGTH);
#endif

    // 初始化网络对象数组（从1开始，0作为错误处理）
    m_netObjectArray.resize(MAX_CLIENT_INDEX);
    for (size_t i = 1; i < m_netObjectArray.size(); i++)
    {
        m_netObjectArray[i] = nullptr;
    }

    // 设置消息处理参数
    m_handleMsgNumPerFrame = pServerConfig->HandleMsgNumPerFrame;

    // 初始化空闲连接ID队列
    for (int i = 1; i < MAX_CLIENT_INDEX; i++)
    {
        uint64_t unlinkId = GetUnLinkId(NF_IS_ENET, m_serverType, pServerConfig->BusId, i);
        m_freeLinks.push(unlinkId);
    }

    // 设置消息处理数量限制
    m_handleMsgNumPerFrame = NF_NO_FIX_FAME_HANDLE_MAX_MSG_COUNT;

    // 初始化计数器
    m_curHandleMsgNum = 0;
}

/**
 * @brief Enet网络消息处理类析构函数
 * 
 * 清理所有网络对象和资源
 */
NFEnetMessage::~NFEnetMessage()
{
    // 清理网络对象映射
    for (auto iter = m_netObjectMap.begin(); iter != m_netObjectMap.end(); ++iter)
    {
        auto pObject = iter->second;
        if (pObject)
        {
            m_netObjectPool.FreeObj(pObject);
        }
    }
    m_netObjectMap.clear();
    
    // 清理网络对象数组
    for (size_t i = 1; i < m_netObjectArray.size(); i++)
    {
        m_netObjectArray[i] = nullptr;
    }
}

/**
 * @brief 绑定Enet服务器
 * 
 * 创建并初始化Enet服务器，设置回调函数
 * 
 * @param flag 绑定标志
 * @return 绑定ID，0表示绑定失败
 */
uint64_t NFEnetMessage::BindServer(const NFMessageFlag& flag)
{
    // 创建Enet服务器
    auto pServer = NF_NEW NFEnetServer(m_pObjPluginManager, m_serverType, flag);
    CHECK_NULL(0, pServer);

    // 设置连接ID和回调函数
    uint64_t unLinkId = GetFreeUnLinkId();
    pServer->SetLinkId(unLinkId);
    pServer->SetConnCallback(std::bind(&NFEnetMessage::ConnectionCallback, this, std::placeholders::_1, std::placeholders::_2, unLinkId));
    pServer->SetMessageCallback(std::bind(&NFEnetMessage::MessageCallback, this, std::placeholders::_1, std::placeholders::_2, unLinkId));
    
    // 初始化服务器
    int iRet = pServer->Init();
    CHECK_ERR_RE_VAL(0, iRet, 0, "pServer Init Failed");
    m_connectionList.push_back(pServer);
    return unLinkId;
}

/**
 * @brief 连接Enet服务器
 * 
 * 创建并初始化Enet客户端，设置回调函数
 * 
 * @param flag 连接标志
 * @return 连接ID，0表示连接失败
 */
/**
 * @brief 连接Enet服务器
 * 
 * 创建并初始化Enet客户端，设置回调函数
 * 
 * @param flag 连接标志
 * @return 连接ID，0表示连接失败
 */
uint64_t NFEnetMessage::ConnectServer(const NFMessageFlag& flag)
{
    // 创建Enet客户端
    auto pClient = NF_NEW NFEnetClient(m_pObjPluginManager, m_serverType, flag);

    if (pClient)
    {
        // 设置连接ID和回调函数
        uint64_t unLinkId = GetFreeUnLinkId();
        pClient->SetLinkId(unLinkId);
        pClient->SetConnCallback(std::bind(&NFEnetMessage::ConnectionCallback, this, std::placeholders::_1, std::placeholders::_2, unLinkId));
        pClient->SetMessageCallback(std::bind(&NFEnetMessage::MessageCallback, this, std::placeholders::_1, std::placeholders::_2, unLinkId));
        int iRet = pClient->Init();
        CHECK_ERR_RE_VAL(0, iRet, 0, "pClient Init Failed");
        m_connectionList.push_back(pClient);
        return unLinkId;
    }
    return 0;
}

/**
 * @brief 添加网络对象（自动分配连接ID）
 * 
 * 自动获取空闲连接ID并创建网络对象
 * 
 * @param pConn ENet连接对象指针
 * @param parseType 数据包解析类型
 * @param bSecurity 安全连接标志
 * @return 网络对象指针，失败返回nullptr
 */
EnetObject* NFEnetMessage::AddNetObject(ENetPeer* pConn, uint32_t parseType, bool bSecurity)
{
    uint64_t usLinkId = GetFreeUnLinkId();
    if (usLinkId == 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "GetFreeUnLinkId Failed, connected count:{}  Can't add connect", m_netObjectArray.size());
        return nullptr;
    }

    return AddNetObject(usLinkId, pConn, parseType, bSecurity);
}

/**
 * @brief 添加网络对象（指定连接ID）
 * 
 * 使用指定的连接ID创建网络对象，包括：
 * - 参数验证
 * - 从对象池分配对象
 * - 设置连接信息
 * - 添加到管理容器
 * 
 * @param unLinkId 连接ID
 * @param pConn ENet连接对象指针
 * @param parseType 数据包解析类型
 * @param bSecurity 安全连接标志
 * @return 网络对象指针，失败返回nullptr
 */
EnetObject* NFEnetMessage::AddNetObject(uint64_t unLinkId, ENetPeer* pConn, uint32_t parseType, bool bSecurity)
{
    CHECK_EXPR_ASSERT(pConn != NULL, NULL, "");
    int index = GetServerIndexFromUnlinkId(unLinkId);
    CHECK_EXPR_ASSERT(index > 0 && index < (int)m_netObjectArray.size(), NULL, "unLinkId:{} index:{} > 0 && index < m_netObjectArray.size()", unLinkId, index);
    CHECK_EXPR_ASSERT(m_netObjectArray[index] == NULL, NULL, "unLinkId:{} index:{} Exist", unLinkId, index);
    CHECK_EXPR_ASSERT(m_netObjectMap.find(unLinkId) == m_netObjectMap.end(), NULL, "unLinkId:{} index:{} Exist", unLinkId, index);

    auto pObject = m_netObjectPool.MallocObjWithArgs(pConn);
    CHECK_EXPR_ASSERT(pObject, NULL, "m_netObjectPool.Alloc() Failed");

    m_netObjectArray[index] = pObject;
    m_netObjectMap.emplace(unLinkId, pObject);

    pObject->SetLinkId(unLinkId);

    if (pConn)
    {
        char ip[32];
        if (enet_address_get_host_ip(&pConn->address, ip, 32) != 0)
        {
            LOG_ERR(0, -1, "enet_address_get_host_ip failed");
        }

        pObject->SetStrIp(ip);
        pObject->SetPort(pConn->address.port);
    }

    pObject->SetPacketParseType(parseType);
    pObject->SetSecurity(bSecurity);

    return pObject;
}

/**
 * @brief 根据连接ID获取网络对象
 * 
 * 通过连接ID查找对应的网络对象，包括：
 * - 服务器类型验证
 * - 索引计算和边界检查
 * - 返回对应的网络对象
 * 
 * @param linkId 连接ID
 * @return 网络对象指针，如果未找到则返回nullptr
 */
EnetObject* NFEnetMessage::GetNetObject(uint64_t linkId) const
{
    uint32_t serverType = GetServerTypeFromUnlinkId(linkId);
    CHECK_EXPR(serverType == m_serverType, NULL, "serverType != m_serverType, this usLinkId:{} is not of the server:{}", linkId, GetServerName(m_serverType).c_str());

    int index = GetServerIndexFromUnlinkId(linkId);
    CHECK_EXPR_ASSERT(index > 0 && index < (int)m_netObjectArray.size(), NULL, "unLinkId:{} index:{} > 0 && index < m_netObjectArray.size()", linkId, index);

    auto pObject = m_netObjectArray[index];

    if (pObject)
    {
        return pObject;
    }

    return nullptr;
}

void NFEnetMessage::ConnectionCallback(ENetEventType eventType, ENetPeer* pConn, uint64_t serverLinkId)
{
    CHECK_NULL_RET_VOID(0, pConn);
    if (eventType == ENET_EVENT_TYPE_CONNECT)
    {
        for (size_t i = 0; i < m_connectionList.size(); i++)
        {
            auto pConnection = m_connectionList[i];
            if (pConnection->GetLinkId() == serverLinkId)
            {
                if (pConnection->GetConnectionType() == NF_CONNECTION_TYPE_TCP_CLIENT)
                {
                    EnetObject* pObject = GetNetObject(pConnection->GetLinkId());
                    if (pObject == nullptr)
                    {
                        pObject = AddNetObject(pConnection->GetLinkId(), pConn, pConnection->GetPacketParseType(), pConnection->IsSecurity());
                        CHECK_EXPR_ASSERT_NOT_RET(pObject != NULL, "AddNetObject Failed");
                        CHECK_EXPR_ASSERT_NOT_RET(pConn->data == NULL, "pConn->data != NULL");
                        auto pData = new ENetPeerData();
                        pConn->data = pData;
                        pData->m_objectLinkId = pConnection->GetLinkId();
                        pData->m_packetParseType = pConnection->GetPacketParseType();
                        pData->m_security = pConnection->IsSecurity();
                    }
                    CHECK_EXPR_ASSERT_NOT_RET(pConnection->GetLinkId() == pObject->GetLinkId(), "pConnection->GetLinkId() != pObject->m_usLinkId, Error..........");

                    pObject->SetConnPtr(pConn);
                    pObject->SetIsServer(false);
                    NFDataPackage tmpPacket;
                    OnHandleMsgPeer(eMsgType_CONNECTED, pConnection->GetLinkId(), pObject->m_usLinkId, tmpPacket);
                }
                else
                {
                    uint64_t objectLinkId = GetFreeUnLinkId();
                    auto pObject = GetNetObject(objectLinkId);
                    CHECK_EXPR_ASSERT_NOT_RET(pObject == NULL, "GetNetObject(pMsg->m_objectLinkId:{}) Exist", objectLinkId);
                    pObject = AddNetObject(objectLinkId, pConn, pConnection->GetPacketParseType(), pConnection->IsSecurity());
                    CHECK_EXPR_ASSERT_NOT_RET(pObject != NULL, "AddNetObject Failed");
                    CHECK_EXPR_ASSERT_NOT_RET(pConn->data == NULL, "pConn->data != NULL");
                    auto pData = new ENetPeerData();
                    pConn->data = pData;
                    pData->m_objectLinkId = objectLinkId;
                    pData->m_packetParseType = pConnection->GetPacketParseType();
                    pData->m_security = pConnection->IsSecurity();

                    NFDataPackage tmpPacket;
                    OnHandleMsgPeer(eMsgType_CONNECTED, pConnection->GetLinkId(), pObject->GetLinkId(), tmpPacket);
                }
                break;
            }
        }
    }
    else if (eventType == ENET_EVENT_TYPE_DISCONNECT)
    {
        if (pConn->data != nullptr)
        {
            /**
             * @brief 不允许出现pMsg->nObjectLinkId找不到的情况，说明代码设置有考虑不周到的情况
             */
            auto pData = static_cast<ENetPeerData*>(pConn->data);
            CHECK_EXPR_ASSERT_NOT_RET(pData != NULL, "");
            auto pObject = GetNetObject(pData->m_objectLinkId);
            CHECK_EXPR_ASSERT_NOT_RET(pObject != NULL, "net disconnect, tcp context error, can't find the net object:{}", pData->m_objectLinkId);
            if (pObject->GetNeedRemove() == false)
            {
                if (pObject->IsServer())
                {
                    pObject->SetNeedRemove(true);
                }
            }

            NF_SAFE_DELETE(pData);
            pConn->data = nullptr;
            pObject->m_connPtr = nullptr;
            NFDataPackage tmpPacket;
            OnHandleMsgPeer(eMsgType_DISCONNECTED, serverLinkId, pObject->GetLinkId(), tmpPacket);
        }
        else
        {
            /**
             * @brief   处理客户端连接服务器掉线, 这里相当于NFClient主动连接服务器，没有连接上
             *         这里的conn其实是一个临时的对象
             */
            NFLogError(NF_LOG_DEFAULT, 0, "net client:{} disconnect, can't connect the server", serverLinkId);

            NFDataPackage tmpPacket;
            OnHandleMsgPeer(eMsgType_DISCONNECTED, serverLinkId, serverLinkId, tmpPacket);
        }
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "net server  error");
    }
}

void NFEnetMessage::MessageCallback(const ENetPeer* pConn, const ENetPacket* pPacket, uint64_t serverLinkId)
{
    CHECK_NULL_RET_VOID(0, pConn);
    CHECK_NULL_RET_VOID(0, pConn->data);
    CHECK_NULL_RET_VOID(0, pPacket);
    ENetPeerData* pData = static_cast<ENetPeerData*>(pConn->data);
    CHECK_EXPR_ASSERT_NOT_RET(pData != NULL, "");

    char* outData = nullptr;
    uint32_t outLen = 0;
    uint32_t allLen = 0;

    NFDataPackage codePackage;
    int ret = NFPacketParseMgr::DeCode(pData->m_packetParseType, reinterpret_cast<const char*>(pPacket->data), pPacket->dataLength, outData, outLen, allLen, codePackage);
    if (ret != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "net server parse data failed!");
    }
    else
    {
        if (!NFGlobalSystem::Instance()->IsSpecialMsg(codePackage.mModuleId, codePackage.nMsgId))
        {
            NFLogTrace(NF_LOG_DEFAULT, 0, "recv msg:{} ", codePackage.ToString());
        }

        auto pRecvQueue = reinterpret_cast<NFCodeQueue*>(m_recvCodeList.ReadAddr());
        CHECK_NULL_RET_VOID(0, pRecvQueue);

        codePackage.nMsgLen = outLen;
        codePackage.nServerLinkId = serverLinkId;
        codePackage.nObjectLinkId = pData->m_objectLinkId;

        int iRet = pRecvQueue->Put(reinterpret_cast<const char*>(&codePackage), sizeof(NFDataPackage), outData, outLen);
        if (iRet != 0)
        {
            if (iRet == -1)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "pRecvQueue->Put((const char*)&codePackage, sizeof(NFDataPackage), (const char*)outData, outLen) param error");
            }
            else if (iRet == -2)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "Recv Queue Full error, Can't Parse Data");
            }
        }
    }
}

void NFEnetMessage::ProcessCodeQueue()
{
    m_curHandleMsgNum = m_handleMsgNumPerFrame;
    auto pQueue = reinterpret_cast<NFCodeQueue*>(m_recvCodeList.ReadAddr());
    ProcessCodeQueue(pQueue);
}

void NFEnetMessage::ProcessCodeQueue(NFCodeQueue* pRecvQueue)
{
    CHECK_NULL_RET_VOID(0, pRecvQueue);
    while (pRecvQueue->HasCode() && m_curHandleMsgNum >= 0)
    {
        m_recvBuffer.Clear();
        int iCodeLen = 0;
        int iRet = pRecvQueue->Get(m_recvBuffer.WriteAddr(), m_recvBuffer.WritableSize(), iCodeLen);
        if (iRet || iCodeLen < static_cast<int>(sizeof(NFDataPackage)))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "get code from pRecvQueue failed ret={}, codelen={}", iRet, iCodeLen);
            continue;
        }
        m_recvBuffer.Produce(iCodeLen);

        // 先获取NetHead
        auto pCodePackage = reinterpret_cast<NFDataPackage*>(m_recvBuffer.ReadAddr());
        if (iCodeLen != static_cast<int>(sizeof(NFDataPackage)) + static_cast<int>(pCodePackage->nMsgLen)) // 长度不一致
        {
            NFLogError(NF_LOG_DEFAULT, 0, "code length invalid. iCodeLen:{} != sizeof(NFDataPackage):{} + pCodePackage->nMsgLen:{}", iCodeLen,
                       sizeof(NFDataPackage), pCodePackage->nMsgLen);
            continue;
        }
        pCodePackage->nBuffer = m_recvBuffer.ReadAddr() + sizeof(NFDataPackage);

        auto pObject = GetNetObject(pCodePackage->nObjectLinkId);
        if (pObject)
        {
            OnHandleMsgPeer(eMsgType_RECIVEDATA, pCodePackage->nServerLinkId, pCodePackage->nObjectLinkId, *pCodePackage);
        }
        else
        {
            NFLogError(NF_LOG_DEFAULT, 0, "net server recv data, tcp context error");
        }
        m_curHandleMsgNum--;
    }
}

void NFEnetMessage::OnHandleMsgPeer(eMsgType type, uint64_t serverLinkId, uint64_t objectLinkId, NFDataPackage& packet)
{
    switch (type)
    {
    case eMsgType_RECIVEDATA:
        {
            if (packet.mModuleId == NF_MODULE_FRAME)
            {
                if (packet.nMsgId == NFrame::NF_SERVER_TO_SERVER_HEART_BEAT)
                {
                    auto pObject = GetNetObject(objectLinkId);
                    if (pObject && pObject->m_isServer)
                    {
                        pObject->SetLastHeartBeatTime(NFGetTime());
                        NFDataPackage tempPacket;
                        tempPacket.mModuleId = NF_MODULE_FRAME;
                        tempPacket.nMsgId = NFrame::NF_SERVER_TO_SERVER_HEART_BEAT_RSP;
                        Send(pObject->GetLinkId(), tempPacket, nullptr, 0);
                        return;
                    }
                    else
                    {
                        NFLogErrorIf(pObject == NULL, NF_LOG_DEFAULT, 0, "GetNetObject Failed, usLinkId:{}", objectLinkId);
                    }
                }

                if (packet.nMsgId == NFrame::NF_SERVER_TO_SERVER_HEART_BEAT_RSP)
                {
                    auto pObject = GetNetObject(objectLinkId);
                    if (pObject && pObject->m_isServer == false)
                    {
                        pObject->SetLastHeartBeatTime(NFGetTime());
                        return;
                    }
                    else
                    {
                        NFLogErrorIf(pObject == NULL, NF_LOG_DEFAULT, 0, "GetNetObject Failed, usLinkId:{}", objectLinkId);
                    }
                }
            }

            if (m_recvCb)
            {
                m_recvCb(serverLinkId, objectLinkId, packet);
            }
        }
        break;
    case eMsgType_CONNECTED:
        {
            if (m_eventCb)
            {
                m_eventCb(type, serverLinkId, objectLinkId);
            }
        }
        break;
    case eMsgType_DISCONNECTED:
        {
            if (m_eventCb)
            {
                m_eventCb(type, serverLinkId, objectLinkId);
            }

            if (objectLinkId > 0)
            {
                uint32_t serverType = GetServerTypeFromUnlinkId(objectLinkId);
                NF_ASSERT_MSG(serverType == m_serverType, "the unlinkId is not of the server");

                uint32_t index = GetServerIndexFromUnlinkId(objectLinkId);
                /**
                 * @brief 处理特殊情况，比如客户端主动连接服务器，连接不上
                 */
                if (index == 0)
                {
                    return;
                }

                auto pObject = GetNetObject(objectLinkId);
                if (pObject && pObject->GetNeedRemove())
                {
                    CHECK_EXPR_ASSERT_NOT_RET(index > 0 && index < m_netObjectArray.size(), "unLinkId:{} index:{} Error", objectLinkId, index);
                    CHECK_EXPR_ASSERT_NOT_RET(m_netObjectMap.find(objectLinkId) != m_netObjectMap.end(), "unLinkId:{} index:{} Error", objectLinkId, index);
                    m_netObjectArray[index] = nullptr;
                    m_netObjectPool.FreeObj(pObject);
                    m_netObjectMap.erase(objectLinkId);
                    m_freeLinks.push(objectLinkId);
                }
                else
                {
                    NFLogErrorIf(pObject == NULL, NF_LOG_DEFAULT, 0, "GetNetObject Failed, usLinkId:{}", objectLinkId);
                }
            }
        }
        break;
    default:
        break;
    }
}

/**
 * @brief 关闭网络模块
 * 
 * 关闭所有连接和清理资源
 * 
 * @return 关闭结果，0表示成功
 */
int NFEnetMessage::Shut()
{
    return NFINetMessage::Shut();
}

/**
 * @brief 释放网络模块资源
 * 
 * 清理所有占用的资源
 * 
 * @return 释放结果，0表示成功
 */
int NFEnetMessage::Finalize()
{
    return NFINetMessage::Finalize();
}

/**
 * @brief 服务器每帧执行
 * 
 * 处理网络事件、消息队列和连接管理
 * 
 * @return 处理结果，0表示成功
 */
int NFEnetMessage::Tick()
{
    for (size_t i = 0; i < m_connectionList.size(); ++i)
    {
        if (m_connectionList[i])
        {
            m_connectionList[i]->Tick();
        }
    }
    ProcessCodeQueue();
    return 0;
}

/**
 * @brief 获得连接IP地址
 * 
 * 根据连接ID获取对应的IP地址
 * 
 * @param usLinkId 连接ID
 * @return IP地址字符串
 */
std::string NFEnetMessage::GetLinkIp(uint64_t usLinkId)
{
    auto pObject = GetNetObject(usLinkId);
    if (pObject)
    {
        return pObject->GetStrIp();
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "GetNetObject Failed, usLinkId:{}", usLinkId);
    }

    return std::string("");
}

/**
 * @brief 获取连接端口号
 * 
 * 根据连接ID获取对应的端口号
 * 
 * @param usLinkId 连接ID
 * @return 端口号
 */
uint32_t NFEnetMessage::GetPort(uint64_t usLinkId)
{
    auto pObject = GetNetObject(usLinkId);
    if (pObject)
    {
        return pObject->GetPort();
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "GetNetObject Failed, usLinkId:{}", usLinkId);
    }
    return 0;
}

/**
 * @brief 关闭指定连接
 * 
 * 关闭指定ID的连接并清理相关资源，包括：
 * - 查找对应的网络对象
 * - 如果是客户端连接，清理连接列表
 * - 标记对象为需要移除
 * - 关闭对象连接
 * 
 * @param usLinkId 连接ID
 */
void NFEnetMessage::CloseLinkId(uint64_t usLinkId)
{
    auto pObject = GetNetObject(usLinkId);
    if (pObject)
    {
        if (pObject->m_isServer == false)
        {
            for (auto iter = m_connectionList.begin(); iter != m_connectionList.end(); ++iter)
            {
                auto pConnection = *iter;
                if (pConnection->GetConnectionType() == NF_CONNECTION_TYPE_TCP_CLIENT)
                {
                    if (pConnection->GetLinkId() == usLinkId)
                    {
                        pConnection->Shut();
                        pConnection->Finalize();
                        NF_SAFE_DELETE(pConnection);
                        m_connectionList.erase(iter);
                        break;
                    }
                }
            }
        }

        pObject->SetNeedRemove(true);
        pObject->CloseObject();
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "can't find the unLinkId:{}", usLinkId);
    }
}

/**
 * @brief 获取空闲连接ID
 * 
 * 从空闲连接ID队列中获取一个可用的连接ID
 * 
 * @return 空闲连接ID，如果没有可用ID则返回0
 */
uint64_t NFEnetMessage::GetFreeUnLinkId()
{
    if (!m_freeLinks.empty())
    {
        uint64_t unlinkId = m_freeLinks.front();
        m_freeLinks.pop();
        return unlinkId;
    }

    NFLogError(NF_LOG_DEFAULT, 0, "GetFreeUnLinkId failed!");
    return 0;
}

/**
 * @brief 发送消息（原始数据）
 * 
 * 向指定连接发送原始数据消息
 * 
 * @param usLinkId 连接ID
 * @param packet 数据包
 * @param msg 消息数据
 * @param nLen 数据长度
 * @return true 发送成功，false 发送失败
 */
bool NFEnetMessage::Send(uint64_t usLinkId, NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    auto pObject = GetNetObject(usLinkId);
    if (pObject)
    {
        return Send(pObject, packet, msg, nLen);
    }
    else
    {
        NFLogErrorIf(pObject == NULL, NF_LOG_DEFAULT, 0, "GetNetObject Failed, usLinkId:{}", usLinkId);
    }

    return false;
}

/**
 * @brief 发送消息（Protobuf数据）
 * 
 * 向指定连接发送Protobuf格式的消息，包括：
 * - 序列化Protobuf消息
 * - 检查缓冲区大小
 * - 调用底层发送接口
 * 
 * @param usLinkId 连接ID
 * @param packet 数据包
 * @param xData Protobuf消息对象
 * @return true 发送成功，false 发送失败
 */
bool NFEnetMessage::Send(uint64_t usLinkId, NFDataPackage& packet, const google::protobuf::Message& xData)
{
    auto pObject = GetNetObject(usLinkId);
    if (pObject)
    {
        m_sendBuffer.Clear();
        int byteSize = xData.ByteSize();
        CHECK_EXPR(static_cast<int>(m_sendBuffer.WritableSize()) >= byteSize, false, "m_sendBuffer.WritableSize():{} < byteSize:{} msg:{}", m_sendBuffer.WritableSize(), byteSize, xData.DebugString());

        auto start = reinterpret_cast<uint8_t*>(m_sendBuffer.WriteAddr());
        uint8_t* end = xData.SerializeWithCachedSizesToArray(start);
        CHECK_EXPR(end - start == byteSize, false, "xData.SerializeWithCachedSizesToArray Failed:{}", xData.DebugString());
        m_sendBuffer.Produce(byteSize);

        return Send(pObject, packet, m_sendBuffer.ReadAddr(), m_sendBuffer.ReadableSize());
    }
    else
    {
        NFLogErrorIf(pObject == NULL, NF_LOG_DEFAULT, 0, "GetNetObject Failed, usLinkId:{}", usLinkId);
    }

    return false;
}

/**
 * @brief 发送消息到指定对象
 * 
 * 向指定的网络对象发送消息，包括：
 * - 检查对象状态和连接状态
 * - 设置数据包参数
 * - 编码数据包
 * - 创建ENet数据包
 * - 发送到对端
 * 
 * @param pObject 网络对象指针
 * @param packet 数据包
 * @param msg 消息数据
 * @param nLen 数据长度
 * @return true 发送成功，false 发送失败
 */
bool NFEnetMessage::Send(const EnetObject* pObject, NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    if (pObject && !pObject->GetNeedRemove() && pObject->m_connPtr && pObject->IsConnect())
    {
        packet.nPacketParseType = pObject->m_packetParseType;
        packet.isSecurity = pObject->IsSecurity();
        packet.nObjectLinkId = pObject->GetLinkId();
        packet.nMsgLen = nLen;

        m_sendBuffer.Clear();
        NFPacketParseMgr::EnCode(pObject->m_packetParseType, packet, msg, nLen, m_sendComBuffer);
        m_sendComBuffer.Clear();

        ENetPacket* packet = enet_packet_create(msg, nLen, ENET_PACKET_FLAG_RELIABLE);
        if (nullptr == packet)
        {
            LOG_ERR(0, -1, "enet create packet {} bytes fail", sizeof(int)+nLen);
            return false;
        }

        if (enet_peer_send(pObject->m_connPtr, 0, packet))
        {
            LOG_ERR(0, -1, "enet create packet {} bytes fail to peer fail", packet->dataLength);
            return false;
        }

        return true;
    }

    return false;
}

/**
 * @brief 定时器回调
 * 
 * 处理定时器事件，包括心跳检测等
 * 
 * @param timerId 定时器ID
 * @return 处理结果，0表示成功
 */
int NFEnetMessage::OnTimer(uint32_t timerId)
{
    return 0;
}
