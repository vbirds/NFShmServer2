// -------------------------------------------------------------------------
//    @FileName         :    NFEvppNetMessage.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    基于Evpp库的网络消息处理实现文件，提供TCP连接管理和消息处理功能
//
// -------------------------------------------------------------------------

#include "NFEvppNetMessage.h"

#include <cstdint>
#include <list>
#include <string>
#include <vector>
#include <NFCommPlugin/NFNetPlugin/NFPacketParseMgr.h>

#include "NFComm/NFCore/NFCommon.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"

#include "NFEvppClient.h"
#include "NFEvppServer.h"
#include "NFComm/NFCore/NFStringUtility.h"
#include "NFComm/NFPluginModule/NFCodeQueue.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFCommPlugin/NFNetPlugin/Encrypt.h"

/**
 * @file NFEvppNetMessage.cpp
 * @brief Evpp网络消息处理实现文件
 * 
 * 该文件实现了基于evpp库的网络消息处理功能，包括：
 * - 网络消息处理类的初始化和销毁
 * - TCP连接管理和生命周期
 * - 网络对象的创建和管理
 * - 消息发送和接收处理
 * - HTTP服务器和客户端集成
 * - 心跳检测和连接监控
 * - 数据包解析和路由
 * 
 * 主要功能：
 * - 管理TCP连接和网络对象
 * - 处理网络事件和消息
 * - 提供HTTP服务器和客户端功能
 * - 支持心跳检测和重连
 * - 数据包解析和消息路由
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief Evpp网络消息处理类构造函数
 * 
 * 初始化网络消息处理模块，包括：
 * - 初始化网络对象池
 * - 读取服务器配置
 * - 设置发送和接收缓冲区
 * - 设置定时器
 * - 初始化HTTP服务器和客户端
 * - 初始化网络对象数组
 * - 设置消息处理参数
 * 
 * @param p 插件管理器指针
 * @param serverType 服务器类型
 */
NFEvppNetMessage::NFEvppNetMessage(NFIPluginManager* p, NF_SERVER_TYPE serverType) : NFINetMessage(p, serverType), m_netObjectPool(1000, false)
{
    // 读取服务器配置
    auto pServerConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    CHECK_EXPR_ASSERT_NOT_RET(pServerConfig, "m_serverType:{} Config Not Find", m_serverType);

    // 设置发送和接收缓冲区
    m_sendBuffer.AssureSpace(MAX_SEND_BUFFER_SIZE);
    m_recvBuffer.AssureSpace(MAX_RECV_BUFFER_SIZE);
    
    // 设置定时器
#ifdef NF_DEBUG_MODE
    SetTimer(ENUM_SERVER_CLIENT_TIMER_HEART, ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 3);
    SetTimer(ENUM_SERVER_TIMER_CHECK_HEART, ENUM_SERVER_TIMER_CHECK_HEART_TIME_LONGTH);
#else
    SetTimer(ENUM_SERVER_CLIENT_TIMER_HEART, ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH*3);
	SetTimer(ENUM_SERVER_TIMER_CHECK_HEART, ENUM_SERVER_TIMER_CHECK_HEART_TIME_LONGTH);
#endif
    
    // 初始化HTTP服务器和客户端
    m_httpServer = nullptr;
#if defined(EVPP_HTTP_SERVER_SUPPORTS_SSL)
    m_httpServerEnableSSL = false;
#endif
    m_httpClient = nullptr;

    // 如果加载所有服务器，则创建连接线程池
    if (m_pObjPluginManager->IsLoadAllServer())
    {
        m_connectionThreadPool.reset(NF_NEW evpp::EventLoopThreadPool(nullptr, 1));
        m_connectionThreadPool->Start(true);
    }

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
        uint64_t unlinkId = GetUnLinkId(NF_IS_NET, m_serverType, pServerConfig->BusId, i);
        while (!m_freeLinks.Enqueue(unlinkId))
        {
        }
    }

    // 设置消息处理数量限制
    m_handleMsgNumPerFrame = NF_NO_FIX_FAME_HANDLE_MAX_MSG_COUNT;

    // 初始化计数器
    m_curHandleMsgNum = 0;
    m_loopSendCount = 0;
}

/**
 * @brief Evpp网络消息处理类析构函数
 * 
 * 清理网络消息处理模块资源，包括：
 * - 停止所有连接
 * - 释放网络对象
 * - 清理HTTP服务器和客户端
 * - 释放线程池资源
 */
NFEvppNetMessage::~NFEvppNetMessage()
{

}

/**
 * @brief 处理消息逻辑线程
 * 
 * 从消息队列中取出消息进行处理，包括：
 * - 批量获取消息
 * - 消息解析和路由
 * - 调用相应的处理函数
 * - 处理连接建立和断开事件
 * - 管理网络对象生命周期
 */
void NFEvppNetMessage::ProcessMsgLogicThread()
{
    m_curHandleMsgNum = m_handleMsgNumPerFrame;

    while (!m_msgQueue.IsQueueEmpty() && m_curHandleMsgNum >= 0)
    {
        std::vector<MsgFromNetInfo> vecMsg;
        vecMsg.resize(200);

        m_msgQueue.TryDequeueBulk(vecMsg);

        for (size_t index = 0; index < vecMsg.size(); index++)
        {
            m_curHandleMsgNum--;
            MsgFromNetInfo* pMsg = &vecMsg[index];
            CHECK_EXPR_ASSERT_NOT_RET(pMsg->m_tcpConPtr != NULL, "");

            if (pMsg->m_type == eMsgType_SENDBUFFER)
            {
                if (pMsg->m_pRecvBuffer)
                {
                    m_recvCodeQueueList.push_back(pMsg->m_pRecvBuffer);
                    pMsg->m_tcpConPtr->loop()->RunEvery(evpp::Duration(2500000), std::bind(&NFEvppNetMessage::LoopSend, this, pMsg->m_tcpConPtr->loop()));
                }
            }
            else if (pMsg->m_type == eMsgType_CONNECTED)
            {
                for (size_t i = 0; i < m_connectionList.size(); i++)
                {
                    if (m_connectionList[i]->GetLinkId() == pMsg->m_serverLinkId)
                    {
                        if (m_connectionList[i]->GetConnectionType() == NF_CONNECTION_TYPE_TCP_CLIENT)
                        {
                            CHECK_EXPR_ASSERT_NOT_RET(m_connectionList[i]->GetLinkId() == pMsg->m_objectLinkId, "m_connectionList[i]->GetLinkId() != pMsg->m_objectLinkId, Error..........");

                            NetEvppObject* pObject = GetNetObject(pMsg->m_objectLinkId);
                            if (pObject == nullptr)
                            {
                                pObject = AddNetObject(pMsg->m_objectLinkId, pMsg->m_tcpConPtr, m_connectionList[i]->GetPacketParseType(), m_connectionList[i]->IsSecurity());
                                CHECK_EXPR_ASSERT_NOT_RET(pObject != NULL, "AddNetObject Failed");
                            }
                            CHECK_EXPR_ASSERT_NOT_RET(m_connectionList[i]->GetLinkId() == pObject->m_usLinkId, "m_connectionList[i]->GetLinkId() != pObject->m_usLinkId, Error..........");

                            pObject->SetConnPtr(pMsg->m_tcpConPtr);
                            pObject->SetIsServer(false);
                            NFDataPackage tmpPacket;
                            OnHandleMsgPeer(eMsgType_CONNECTED, m_connectionList[i]->GetLinkId(), pObject->m_usLinkId, tmpPacket);
                        }
                        else
                        {
                            NetEvppObject* pObject = GetNetObject(pMsg->m_objectLinkId);
                            CHECK_EXPR_ASSERT_NOT_RET(pObject == NULL, "GetNetObject(pMsg->m_objectLinkId:{}) Exist", pMsg->m_objectLinkId);
                            pObject = AddNetObject(pMsg->m_objectLinkId, pMsg->m_tcpConPtr, m_connectionList[i]->GetPacketParseType(), m_connectionList[i]->IsSecurity());
                            CHECK_EXPR_ASSERT_NOT_RET(pObject != NULL, "AddNetObject Failed");

                            NFDataPackage tmpPacket;
                            OnHandleMsgPeer(eMsgType_CONNECTED, m_connectionList[i]->GetLinkId(), pObject->m_usLinkId, tmpPacket);
                        }
                        break;
                    }
                }
            }
            else if (pMsg->m_type == eMsgType_DISCONNECTED)
            {
                if (pMsg->m_objectLinkId != 0)
                {
                    /**
                     * @brief 不允许出现pMsg->nObjectLinkId找不到的情况，说明代码设置有考虑不周到的情况
                     */
                    NetEvppObject* pObject = GetNetObject(pMsg->m_objectLinkId);
                    CHECK_EXPR_ASSERT_NOT_RET(pObject != NULL, "net disconnect, tcp context error, can't find the net object:{}", pMsg->m_objectLinkId);
                    if (pObject->GetNeedRemove() == false)
                    {
                        if (pObject->IsServer())
                        {
                            pObject->SetNeedRemove(true);
                        }
                    }

                    pObject->m_connPtr = nullptr;
                    NFDataPackage tmpPacket;
                    OnHandleMsgPeer(eMsgType_DISCONNECTED, pMsg->m_serverLinkId, pMsg->m_objectLinkId, tmpPacket);
                }
                else
                {
                    /**
                     * @brief   处理客户端连接服务器掉线, 这里相当于NFClient主动连接服务器，没有连接上
                     *         这里的conn其实是一个临时的对象
                     */
                    NFLogError(NF_LOG_DEFAULT, 0, "net client:{} disconnect, can't connect the server", pMsg->m_serverLinkId);

                    NFDataPackage tmpPacket;
                    OnHandleMsgPeer(eMsgType_DISCONNECTED, pMsg->m_serverLinkId, pMsg->m_serverLinkId, tmpPacket);
                }
            }
            else
            {
                NFLogError(NF_LOG_DEFAULT, 0, "net server  error");
            }
        }
    }
}

void NFEvppNetMessage::ProcessCodeQueue(NFCodeQueue* pRecvQueue)
{
    NF_ASSERT_MSG(pRecvQueue != NULL, "pRecvQueue == NULL");
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

        NetEvppObject* pObject = GetNetObject(pCodePackage->nObjectLinkId);
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


void NFEvppNetMessage::ProcessCodeQueue()
{
    m_curHandleMsgNum = m_handleMsgNumPerFrame;
    for (size_t i = 0; i < m_recvCodeQueueList.size(); i++)
    {
        NF_SHARE_PTR<NFBuffer> pRecvBuffer = m_recvCodeQueueList[i];
        if (pRecvBuffer)
        {
            auto pQueue = reinterpret_cast<NFCodeQueue*>(pRecvBuffer->ReadAddr());
            ProcessCodeQueue(pQueue);
        }
    }
}

/**
 * @brief 连接事件回调函数
 * 
 * 处理TCP连接建立和断开事件，包括：
 * - 初始化连接上下文（接收缓冲区、发送缓冲区、压缩缓冲区等）
 * - 设置连接参数（TCP_NODELAY等）
 * - 创建网络对象
 * - 处理连接建立和断开消息
 * - 管理连接映射表
*
 * @param conn TCP连接指针
 * @param serverLinkId 服务器连接ID
*/
void NFEvppNetMessage::ConnectionCallback(const evpp::TCPConnPtr& conn, uint64_t serverLinkId)
{
    if (conn->loop()->context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV).IsEmpty())
    {
        NF_SHARE_PTR<NFBuffer> pRecvBuffer = std::make_shared<NFBuffer>();
        pRecvBuffer->AssureSpace(MAX_CODE_QUEUE_SIZE);
        pRecvBuffer->Produce(MAX_CODE_QUEUE_SIZE);
        auto pQueue = reinterpret_cast<NFCodeQueue*>(pRecvBuffer->ReadAddr());
        pQueue->Init(pRecvBuffer->ReadableSize());
        conn->loop()->set_context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV, evpp::Any(pRecvBuffer));

        MsgFromNetInfo msg;
        msg.m_type = eMsgType_SENDBUFFER;
        msg.m_tcpConPtr = conn;
        msg.m_serverLinkId = serverLinkId;
        msg.m_pRecvBuffer = pRecvBuffer;
        while (!m_msgQueue.Enqueue(msg))
        {
        }
    }

    if (conn->loop()->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND).IsEmpty())
    {
        NF_SHARE_PTR<NFBuffer> pSendBuffer = std::make_shared<NFBuffer>();
        pSendBuffer->AssureSpace(MAX_CODE_QUEUE_SIZE);
        pSendBuffer->Produce(MAX_CODE_QUEUE_SIZE);
        auto pQueue = reinterpret_cast<NFCodeQueue*>(pSendBuffer->ReadAddr());
        pQueue->Init(pSendBuffer->ReadableSize());
        conn->loop()->set_context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND, evpp::Any(pSendBuffer));
    }

    if (conn->loop()->context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER).IsEmpty())
    {
        NF_SHARE_PTR<NFBuffer> pComBuffer = std::make_shared<NFBuffer>();
        pComBuffer->AssureSpace(MAX_RECV_BUFFER_SIZE);
        conn->loop()->set_context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER, evpp::Any(pComBuffer));
    }

    if (conn->loop()->context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP).IsEmpty())
    {
        NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>> pConnMap = std::make_shared<std::unordered_map<uint64_t, evpp::TCPConnPtr>>();
        conn->loop()->set_context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP, evpp::Any(pConnMap));
    }

    if (conn->loop()->context(EVPP_LOOP_CONTEXT_4_CODE_QUEUE_BUFFER).IsEmpty())
    {
        NF_SHARE_PTR<NFBuffer> pCodeQueueBuffer = std::make_shared<NFBuffer>();
        pCodeQueueBuffer->AssureSpace(MAX_RECV_BUFFER_SIZE);
        conn->loop()->set_context(EVPP_LOOP_CONTEXT_4_CODE_QUEUE_BUFFER, evpp::Any(pCodeQueueBuffer));
    }

    if (conn->IsConnected())
    {
        conn->SetTCPNoDelay(true);

        NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>> pConnMap = evpp::any_cast<NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>>>(conn->loop()->context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP));
        NF_ASSERT_MSG(pConnMap != NULL,
                      "evpp::any_cast<NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>>>(conn->loop()->context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP)) Failed");

        MsgFromNetInfo msg;
        msg.m_tcpConPtr = conn;
        msg.m_serverLinkId = serverLinkId;
        msg.m_type = eMsgType_CONNECTED;

        //client connect
        if (conn->type() == evpp::TCPConn::kOutgoing)
        {
            /**
             * @brief 客户端掉线，一定会清理调conn->context(), 不如程序有问题
             */
            CHECK_EXPR_ASSERT_NOT_RET(conn->context().IsEmpty(), "conn->context().IsEmpty() Error");
            msg.m_objectLinkId = serverLinkId;
            msg.m_tcpConPtr->set_context(evpp::Any(msg.m_objectLinkId));
            CHECK_EXPR_ASSERT_NOT_RET(pConnMap->find(serverLinkId) == pConnMap->end(), "pConnMap->find(connectLinkId) == pConnMap->end() Error");
            pConnMap->emplace(serverLinkId, msg.m_tcpConPtr);
        }
        else
        {
            CHECK_EXPR_ASSERT_NOT_RET(conn->context().IsEmpty(), "conn->context().IsEmpty() Error");
            msg.m_objectLinkId = GetFreeUnLinkId();
            if (msg.m_objectLinkId == 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "GetFreeUnLinkId() Failed, No Free UnlinkId");
                msg.m_tcpConPtr->Close();

                /**
                 * @brief 用来区分GetFreeUnLinkId Failed
                 */
                msg.m_tcpConPtr->set_context(1, evpp::Any(msg.m_objectLinkId));

                return;
            }
            msg.m_tcpConPtr->set_context(evpp::Any(msg.m_objectLinkId));
            CHECK_EXPR_ASSERT_NOT_RET(pConnMap->find(msg.m_objectLinkId) == pConnMap->end(), "pConnMap->find(connectLinkId) == pConnMap->end() Error");
            pConnMap->emplace(msg.m_objectLinkId, msg.m_tcpConPtr);
        }

        while (!m_msgQueue.Enqueue(msg))
        {
        }
    }
    else
    {
        NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>> pConnMap = evpp::any_cast<NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>>>(conn->loop()->context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP));
        NF_ASSERT_MSG(pConnMap != NULL,
                      "evpp::any_cast<NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>>>(conn->loop()->context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP)) Failed");

        MsgFromNetInfo msg;
        msg.Clear();
        msg.m_tcpConPtr = conn;
        msg.m_serverLinkId = serverLinkId;
        msg.m_type = eMsgType_DISCONNECTED;
        /**
         * @brief 处理客户端连接服务器掉线
         */
        if (conn->type() == evpp::TCPConn::kOutgoing)
        {
            if (!conn->context().IsEmpty())
            {
                msg.m_objectLinkId = evpp::any_cast<uint64_t>(conn->context());
                msg.m_tcpConPtr->set_context(evpp::Any());
                pConnMap->erase(msg.m_objectLinkId);
            }
            else
            {
                /**
                 * @brief   处理NFClient客户端连接服务器掉线, 这里相当于NFClient主动连接服务器，没有连接上, 这里的conn其实是一个临时的对象.
                 */
                CHECK_EXPR_ASSERT_NOT_RET(pConnMap->find(msg.m_serverLinkId) == pConnMap->end(), "pConnMap->find(connectLinkId) == pConnMap->end() Error");
                msg.m_objectLinkId = 0;
            }
        }
        else
        {
            /**
             * @brief 处理服务器的连接掉线
             */
            if (!conn->context().IsEmpty())
            {
                msg.m_objectLinkId = evpp::any_cast<uint64_t>(conn->context());
                msg.m_tcpConPtr->set_context(evpp::Any());
                pConnMap->erase(msg.m_objectLinkId);
            }
            else
            {
                /**
                 * @brief  GetFreeUnLinkId() Failed, NFServer的连接被主动关闭导致
                 */
                if (!conn->context(1).IsEmpty())
                {
                    NFLogError(NF_LOG_DEFAULT, 0, "GetFreeUnLinkId() Failed, net client disconnect, can't find the net context!");
                    return;
                }

                NFLogError(NF_LOG_DEFAULT, 0, "Unknow Failed, net client disconnect, can't find the net context!");
                msg.m_objectLinkId = 0;
            }
        }

        while (!m_msgQueue.Enqueue(msg))
        {
        }
    }
}

/**
* @brief 消息回调
*
* @return 消息回调
*/
void NFEvppNetMessage::MessageCallback(const evpp::TCPConnPtr& conn, evpp::Buffer* msg, uint64_t serverLinkId, uint32_t packetParse, bool bSecurity)
{
    if (msg)
    {
        if (bSecurity)
        {
            Decryption(const_cast<char*>(msg->data()), msg->size());
        }

        while (true)
        {
            char* outData = nullptr;
            uint32_t outLen = 0;
            uint32_t allLen = 0;

            NFDataPackage codePackage;
            int ret = NFPacketParseMgr::DeCode(packetParse, msg->data(), msg->size(), outData, outLen, allLen, codePackage);
            if (ret < 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "net server parse data failed!");
                msg->Reset();
                break;
            }
            else if (ret > 0)
            {
                break;
            }
            else
            {
                if (!NFGlobalSystem::Instance()->IsSpecialMsg(codePackage.mModuleId, codePackage.nMsgId))
                {
                    NFLogTrace(NF_LOG_DEFAULT, 0, "recv msg:{} ", codePackage.ToString());
                }

                if (codePackage.bCompress)
                {
                    CHECK_EXPR_ASSERT_NOT_RET(!conn->loop()->context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER).IsEmpty(), "conn->loop()->context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER).IsEmpty()");

                    NF_SHARE_PTR<NFBuffer> pComBuffer = evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(conn->loop()->context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER));
                    pComBuffer->Clear();

                    int decompressLen = NFPacketParseMgr::Decompress(packetParse, outData, outLen, pComBuffer->WriteAddr(), static_cast<int>(pComBuffer->WritableSize()));
                    if (decompressLen < 0)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "recv msg:{}, decompress failed!", codePackage.ToString());
                        msg->Skip(allLen);
                        continue;
                    }
                    pComBuffer->Produce(decompressLen);

                    CHECK_EXPR_ASSERT_NOT_RET(!conn->loop()->context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV).IsEmpty(), "conn->loop()->context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV).IsEmpty(), Recv Code Queue Not Exist, Can't Parse Data");
                    NF_SHARE_PTR<NFBuffer> pRecvBuffer = evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(conn->loop()->context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV));
                    NF_ASSERT(pRecvBuffer != NULL);
                    auto pRecvQueue = reinterpret_cast<NFCodeQueue*>(pRecvBuffer->ReadAddr());
                    NF_ASSERT(pRecvQueue != NULL);

                    codePackage.nMsgLen = pComBuffer->ReadableSize();
                    codePackage.nServerLinkId = serverLinkId;
                    if (!conn->context().IsEmpty())
                    {
                        codePackage.nObjectLinkId = evpp::any_cast<uint64_t>(conn->context());
                    }
                    else
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "net server diconnect, tcp context error");
                        codePackage.nObjectLinkId = 0;
                    }
                    int iRet = pRecvQueue->Put(reinterpret_cast<const char*>(&codePackage), sizeof(NFDataPackage), pComBuffer->ReadAddr(), pComBuffer->ReadableSize());
                    if (iRet != 0)
                    {
                        if (iRet == -1)
                        {
                            NFLogError(NF_LOG_DEFAULT, 0, "pRecvQueue->Put((const char*)&codePackage, sizeof(NFDataPackage), (const char*)outData, outLen) param error");
                        }
                        else if (iRet == -2)
                        {
                            NFLogError(NF_LOG_DEFAULT, 0, "Recv Queue Full error, can't put the error");
                        }
                    }
                }
                else
                {
                    CHECK_EXPR_ASSERT_NOT_RET(!conn->loop()->context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV).IsEmpty(), "conn->loop()->context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV).IsEmpty(), Recv Code Queue Not Exist, Can't Parse Data");
                    NF_SHARE_PTR<NFBuffer> pRecvBuffer = evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(conn->loop()->context(EVPP_LOOP_CONTEXT_0_MAIN_THREAD_RECV));
                    NF_ASSERT(pRecvBuffer != NULL);
                    auto pRecvQueue = reinterpret_cast<NFCodeQueue*>(pRecvBuffer->ReadAddr());
                    NF_ASSERT(pRecvQueue != NULL);

                    codePackage.nMsgLen = outLen;
                    codePackage.nServerLinkId = serverLinkId;
                    if (!conn->context().IsEmpty())
                    {
                        codePackage.nObjectLinkId = evpp::any_cast<uint64_t>(conn->context());
                    }
                    else
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "net server disconnect, tcp context error");
                        codePackage.nObjectLinkId = 0;
                    }

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

                msg->Skip(allLen);

                if (msg->length() <= 0)
                {
                    break;
                }
            }
        }
    }
}

uint64_t NFEvppNetMessage::BindServer(const NFMessageFlag& flag)
{
    if (flag.bHttp)
    {
        int iRet = BindHttpServer(flag.nPort, flag.nNetThreadNum);
        if (iRet == 0)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "BindHttpServer Failed! port:{}", flag.nPort);
        }
        return iRet;
    }
    else
    {
        auto pServer = NF_NEW NFEvppServer(m_pObjPluginManager, m_serverType, flag);

        uint64_t unLinkId = GetFreeUnLinkId();
        pServer->SetLinkId(unLinkId);
        pServer->SetConnCallback(
            std::bind(&NFEvppNetMessage::ConnectionCallback, this, std::placeholders::_1, unLinkId));
        pServer->SetMessageCallback(
            std::bind(&NFEvppNetMessage::MessageCallback, this, std::placeholders::_1, std::placeholders::_2,
                      unLinkId, flag.mPacketParseType, flag.mSecurity));
        int iRet = pServer->Init();
        CHECK_ERR_RE_VAL(0, iRet, 0, "pServer Init Failed");
        m_connectionList.push_back(pServer);
        return unLinkId;
    }

    return 0;
}

uint64_t NFEvppNetMessage::BindHttpServer(uint32_t listenPort, uint32_t netThreadNum)
{
    auto pServer = NF_NEW NFCHttpServer(m_serverType, netThreadNum);
    if (pServer)
    {
        pServer->SetRecvCb(m_httpReceiveCb);
        pServer->SetFilterCb(m_httpFilter);
#if defined(EVPP_HTTP_SERVER_SUPPORTS_SSL)
        pServer->SetPortSSLOption(listenPort, m_httpServerEnableSSL, m_httpServerCertificateChainFile.c_str(), m_httpServerPrivateKeyFile.c_str());
#endif
        if (pServer->InitServer(listenPort))
        {
            m_httpServer = pServer;
            return 1;
        }
    }
    return 0;
}

/**
* @brief	初始化
*
* @return 是否成功
*/
uint64_t NFEvppNetMessage::ConnectServer(const NFMessageFlag& flag)
{
    auto pClient = NF_NEW NFEvppClient(m_pObjPluginManager, m_serverType, flag);

    if (pClient)
    {
        uint64_t unLinkId = GetFreeUnLinkId();
        pClient->SetLinkId(unLinkId);
        pClient->SetConnCallback(std::bind(&NFEvppNetMessage::ConnectionCallback, this, std::placeholders::_1, unLinkId));
        pClient->SetMessageCallback(std::bind(&NFEvppNetMessage::MessageCallback, this, std::placeholders::_1, std::placeholders::_2, unLinkId, flag.mPacketParseType, flag.mSecurity));

        if (m_pObjPluginManager->IsLoadAllServer() && m_connectionThreadPool)
        {
            if (pClient->Init(m_connectionThreadPool->GetNextLoop()))
            {
                m_connectionList.push_back(pClient);

                return unLinkId;
            }
        }
        else
        {
            if (pClient->Init(nullptr))
            {
                m_connectionList.push_back(pClient);

                return unLinkId;
            }
        }
    }
    return 0;
}

std::string NFEvppNetMessage::GetLinkIp(uint64_t usLinkId)
{
    NetEvppObject* pObject = GetNetObject(usLinkId);
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

uint32_t NFEvppNetMessage::GetPort(uint64_t usLinkId)
{
    NetEvppObject* pObject = GetNetObject(usLinkId);
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
 * @brief 添加网络对象（自动分配连接ID）
 * 
 * 自动获取空闲连接ID并创建TCP网络连接对象，包括：
 * - 获取空闲连接ID
 * - 验证连接ID有效性
 * - 调用指定连接ID的添加函数
 * 
 * @param conn TCP连接指针
 * @param parseType 数据包解析类型
 * @param bSecurity 安全连接标志
 * @return 网络对象指针，失败返回nullptr
 */
NetEvppObject* NFEvppNetMessage::AddNetObject(const evpp::TCPConnPtr& conn, uint32_t parseType, bool bSecurity)
{
    uint64_t usLinkId = GetFreeUnLinkId();
    if (usLinkId == 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "GetFreeUnLinkId Failed, connected count:{}  Can't add connect", m_netObjectArray.size());
        return nullptr;
    }

    return AddNetObject(usLinkId, conn, parseType, bSecurity);
}

/**
 * @brief 添加网络对象（指定连接ID）
 * 
 * 使用指定的连接ID创建TCP网络连接对象，包括：
 * - 参数验证（连接ID有效性、索引范围检查）
 * - 从对象池分配网络对象
 * - 设置连接信息（IP、端口、解析类型、安全标志）
 * - 添加到管理容器（数组和映射表）
 * 
 * @param unLinkId 连接ID
 * @param conn TCP连接指针
 * @param parseType 数据包解析类型
 * @param bSecurity 安全连接标志
 * @return 网络对象指针，失败返回nullptr
 */
NetEvppObject* NFEvppNetMessage::AddNetObject(uint64_t unLinkId, const evpp::TCPConnPtr& conn, uint32_t parseType, bool bSecurity)
{
    int index = GetServerIndexFromUnlinkId(unLinkId);
    CHECK_EXPR_ASSERT(index > 0 && index < (int)m_netObjectArray.size(), NULL, "unLinkId:{} index:{} > 0 && index < m_netObjectArray.size()", unLinkId, index);
    CHECK_EXPR_ASSERT(m_netObjectArray[index] == NULL, NULL, "unLinkId:{} index:{} Exist", unLinkId, index);
    CHECK_EXPR_ASSERT(m_netObjectMap.find(unLinkId) == m_netObjectMap.end(), NULL, "unLinkId:{} index:{} Exist", unLinkId, index);

    auto pObject = m_netObjectPool.MallocObjWithArgs(conn);
    CHECK_EXPR_ASSERT(pObject, NULL, "m_netObjectPool.Alloc() Failed");

    m_netObjectArray[index] = pObject;
    m_netObjectMap.emplace(unLinkId, pObject);

    pObject->SetLinkId(unLinkId);

    if (conn)
    {
        std::string remoteAddr = conn->remote_addr();
        std::vector<std::string> vec;
        NFStringUtility::Split(remoteAddr, ":", &vec);
        if (vec.size() >= 2)
        {
            pObject->SetStrIp(vec[0]);
            pObject->SetPort(NFCommon::strto<uint32_t>(vec[1]));
        }
    }

    pObject->SetPacketParseType(parseType);
    pObject->SetSecurity(bSecurity);

    return pObject;
}

/**
 * @brief 根据连接ID获取网络对象
 * 
 * 根据连接ID查找并返回对应的网络对象，包括：
 * - 验证服务器类型匹配
 * - 计算数组索引
 * - 返回网络对象指针
 * 
 * @param linkId 连接ID
 * @return 网络对象指针，未找到返回nullptr
 */
NetEvppObject* NFEvppNetMessage::GetNetObject(uint64_t linkId) const
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

void NFEvppNetMessage::CloseLinkId(uint64_t usLinkId)
{
    auto pObject = GetNetObject(usLinkId);
    if (pObject)
    {
        if (pObject->m_isServer == false)
        {
            for (auto iter = m_connectionList.begin(); iter != m_connectionList.end(); ++iter)
            {
                NFIConnection* pConnection = *iter;
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

uint64_t NFEvppNetMessage::GetFreeUnLinkId()
{
    if (!m_freeLinks.IsQueueEmpty())
    {
        uint64_t unlinkId = 0;
        if (m_freeLinks.TryDequeue(unlinkId))
        {
            return unlinkId;
        }
    }

    NFLogError(NF_LOG_DEFAULT, 0, "GetFreeUnLinkId failed!");
    return 0;
}

/**
 * @brief 关闭网络消息处理模块
 * 
 * 关闭所有网络连接，包括：
 * - 关闭所有TCP连接
 * - 停止HTTP服务器和客户端
 * - 清理连接资源
 * 
 * @return 关闭结果，0表示成功
 */
int NFEvppNetMessage::Shut()
{
    for (size_t i = 0; i < m_connectionList.size(); i++)
    {
        NFIConnection* pConn = m_connectionList[i];
        if (pConn)
        {
            pConn->Shut();
        }
    }


    return 0;
}

/**
 * @brief 释放网络消息处理模块资源
 * 
 * 清理所有网络资源，包括：
 * - 释放所有连接对象
 * - 清理HTTP服务器和客户端
 * - 停止线程池
 * - 释放网络对象池
 * - 清理缓冲区
 * 
 * @return 释放结果，0表示成功
 */
int NFEvppNetMessage::Finalize()
{
    for (size_t i = 0; i < m_connectionList.size(); i++)
    {
        NFIConnection* pConn = m_connectionList[i];
        if (pConn)
        {
            pConn->Finalize();
        }
    }

    for (size_t i = 0; i < m_connectionList.size(); i++)
    {
        NFIConnection* pConn = m_connectionList[i];
        if (pConn)
        {
            NF_SAFE_DELETE(pConn);
        }
    }

    if (m_httpServer)
    {
        NF_SAFE_DELETE(m_httpServer);
        m_httpServer = nullptr;
    }
    if (m_httpClient)
    {
        NF_SAFE_DELETE(m_httpClient);
        m_httpClient = nullptr;
    }

    if (m_connectionThreadPool)
    {
        m_connectionThreadPool->Stop(true);
        NF_ASSERT(m_connectionThreadPool->IsStopped());
        m_connectionThreadPool->Join();
        m_connectionThreadPool.reset();
    }

    for (auto iter = m_netObjectMap.begin(); iter != m_netObjectMap.end(); ++iter)
    {
        auto pObject = iter->second;
        if (pObject)
        {
            m_netObjectPool.FreeObj(pObject);
        }
    }
    m_netObjectMap.clear();
    for (size_t i = 1; i < m_netObjectArray.size(); i++)
    {
        m_netObjectArray[i] = nullptr;
    }

    m_recvCodeQueueList.clear();

    return 0;
}

/**
 * @brief 网络消息处理模块主循环
 * 
 * 执行网络消息处理的主循环，包括：
 * - 处理消息逻辑线程
 * - 处理代码队列
 * - 执行HTTP服务器和客户端
 * 
 * @return 执行结果，0表示成功
 */
int NFEvppNetMessage::Tick()
{
    ProcessMsgLogicThread();
    ProcessCodeQueue();
    if (m_httpServer)
    {
        m_httpServer->Execute();
    }
    if (m_httpClient)
    {
        m_httpClient->Execute();
    }

    return 0;
}

/**
 * @brief 发送消息（原始数据）
 * 
 * 向指定连接发送原始数据消息，包括：
 * - 根据连接ID获取网络对象
 * - 验证网络对象有效性
 * - 调用底层发送接口
 * 
 * @param usLinkId 连接ID
 * @param packet 数据包
 * @param msg 消息数据
 * @param nLen 数据长度
 * @return true 发送成功，false 发送失败
 */
bool NFEvppNetMessage::Send(uint64_t usLinkId, NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    NetEvppObject* pObject = GetNetObject(usLinkId);
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
 * - 根据连接ID获取网络对象
 * - 序列化Protobuf消息
 * - 检查缓冲区大小
 * - 调用底层发送接口
 * 
 * @param usLinkId 连接ID
 * @param packet 数据包
 * @param xData Protobuf消息对象
 * @return true 发送成功，false 发送失败
 */
bool NFEvppNetMessage::Send(uint64_t usLinkId, NFDataPackage& packet, const google::protobuf::Message& xData)
{
    NetEvppObject* pObject = GetNetObject(usLinkId);
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
 * @brief 处理网络消息对等端
 * 
 * 处理来自网络的消息，包括：
 * - 接收数据消息处理
 * - 心跳消息处理
 * - 连接状态消息处理
 * - 消息路由和分发
 * 
 * @param type 消息类型
 * @param serverLinkId 服务器连接ID
 * @param objectLinkId 对象连接ID
 * @param packet 数据包
 */
void NFEvppNetMessage::OnHandleMsgPeer(eMsgType type, uint64_t serverLinkId, uint64_t objectLinkId, NFDataPackage& packet)
{
    switch (type)
    {
    case eMsgType_RECIVEDATA:
        {
            if (packet.mModuleId == NF_MODULE_FRAME)
            {
                if (packet.nMsgId == NFrame::NF_SERVER_TO_SERVER_HEART_BEAT)
                {
                    NetEvppObject* pObject = GetNetObject(objectLinkId);
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
                    NetEvppObject* pObject = GetNetObject(objectLinkId);
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
                    while (!m_freeLinks.Enqueue(objectLinkId))
                    {
                    }
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

bool NFEvppNetMessage::Send(NetEvppObject* pObject, NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    if (pObject && !pObject->GetNeedRemove() && pObject->m_connPtr && pObject->m_connPtr->IsConnected())
    {
        packet.nPacketParseType = pObject->m_packetParseType;
        packet.isSecurity = pObject->IsSecurity();
        packet.nObjectLinkId = pObject->GetLinkId();
        packet.nMsgLen = nLen;
        CHECK_EXPR_ASSERT(!pObject->m_connPtr->loop()->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND).IsEmpty(), false, "pConn->loop()->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND).IsEmpty() ERROR");
        NF_SHARE_PTR<NFBuffer> pSendBuffer = evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(pObject->m_connPtr->loop()->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND));
        CHECK_EXPR_ASSERT(pSendBuffer != NULL, false, "evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(pObject->mConnPtr->loop()->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND) NULL");
        auto pSendQueue = reinterpret_cast<NFCodeQueue*>(pSendBuffer->ReadAddr());
        CHECK_EXPR_ASSERT(pSendQueue != NULL, false, "(NFCodeQueue*)pSendBuffer->ReadAddr() NULL");

        int iRet = pSendQueue->Put(reinterpret_cast<const char*>(&packet), sizeof(NFDataPackage), msg, nLen);
        if (iRet != 0)
        {
            if (iRet == -1)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "pSendQueue->Put((const char*)&codePackage, sizeof(NFDataPackage), (const char*)msg, nLen) param error, package:({}) drop msg", packet.ToString());
            }
            else if (iRet == -2)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "Send Queue Full error, can't put the error, package:({}) drop msg", packet.ToString());
            }
            pObject->m_connPtr->loop()->RunInLoop(std::bind(&NFEvppNetMessage::LoopSend, this, pObject->m_connPtr->loop()));
        }

        if (m_loopSendCount.load() <= 0)
        {
            ++m_loopSendCount;
            pObject->m_connPtr->loop()->RunInLoop(std::bind(&NFEvppNetMessage::LoopSend, this, pObject->m_connPtr->loop()));
        }

        return true;
    }

    return false;
}

int NFEvppNetMessage::OnTimer(uint32_t timerId)
{
    if (timerId == ENUM_SERVER_CLIENT_TIMER_HEART)
    {
        SendHeartMsg();
    }
    else if (timerId == ENUM_SERVER_TIMER_CHECK_HEART)
    {
        CheckServerHeartBeat();
    }
    return 0;
}

void NFEvppNetMessage::SendHeartMsg()
{
    for (size_t i = 0; i < m_connectionList.size(); i++)
    {
        if (m_connectionList[i] && m_connectionList[i]->GetConnectionType() == NF_CONNECTION_TYPE_TCP_CLIENT && GetNetObject(m_connectionList[i]->GetLinkId()) != nullptr)
        {
            NFDataPackage packet;
            packet.mModuleId = NF_MODULE_FRAME;
            packet.nMsgId = NFrame::NF_SERVER_TO_SERVER_HEART_BEAT;
            Send(m_connectionList[i]->GetLinkId(), packet, nullptr, 0);
        }
    }
}

void NFEvppNetMessage::CheckServerHeartBeat()
{
    uint64_t nowTime = NFGetTime();
    for (auto iter = m_netObjectMap.begin(); iter != m_netObjectMap.end(); ++iter)
    {
        NetEvppObject* pObject = iter->second;
        if (pObject && pObject->m_isServer && pObject->m_packetParseType == PACKET_PARSE_TYPE_INTERNAL) //服务器与服务器之间有这个需求，协议必须是内网协议
        {
            //debug 30min
#ifdef NF_DEBUG_MODE
            if (pObject->m_lastHeartBeatTime > 0 && nowTime - pObject->m_lastHeartBeatTime > ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 20 * 60)
            {
                pObject->CloseObject();
            }
#else
            if (pObject->mLastHeartBeatTime > 0 && nowTime - pObject->mLastHeartBeatTime > ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH * 20)
			{
				pObject->CloseObject();
			}
#endif
        }
    }
}

/**
 * @brief 响应HTTP消息（根据HTTP句柄）
 * 
 * 通过HTTP请求句柄响应HTTP消息
 * 
 * @param req HTTP请求句柄
 * @param strMsg 响应消息内容
 * @param code HTTP状态码
 * @param reason 状态原因
 * @return true 响应成功，false 响应失败
 */
bool NFEvppNetMessage::ResponseHttpMsg(const NFIHttpHandle& req, const string& strMsg,
                                       NFWebStatus code, const string& reason)
{
    if (m_httpServer)
    {
        return m_httpServer->ResponseMsg(req, strMsg, code, reason);
    }
    return false;
}

bool NFEvppNetMessage::ResponseHttpMsg(uint64_t requestId, const string& strMsg,
                                       NFWebStatus code, const string& reason)
{
    if (m_httpServer)
    {
        return m_httpServer->ResponseMsg(requestId, strMsg, code, reason);
    }
    return false;
}

/**
 * @brief 发送HTTP GET请求
 * 
 * 发送HTTP GET请求，如果HTTP客户端不存在则创建
 * 
 * @param strUri 请求URI
 * @param respone 响应回调函数
 * @param xHeaders 请求头
 * @param timeout 超时时间
 * @return 请求结果
 */
int NFEvppNetMessage::HttpGet(const string& strUri, const HTTP_CLIENT_RESPONE& respone,
                              const map<std::string, std::string>& xHeaders, int timeout)
{
    if (!m_httpClient)
    {
        m_httpClient = NF_NEW NFCHttpClient();
    }

    return m_httpClient->HttpGet(strUri, respone, xHeaders, timeout);
}

/**
 * @brief 发送HTTP POST请求
 * 
 * 发送HTTP POST请求，如果HTTP客户端不存在则创建
 * 
 * @param strUri 请求URI
 * @param strPostData POST数据
 * @param respone 响应回调函数
 * @param xHeaders 请求头
 * @param timeout 超时时间
 * @return 请求结果
 */
int NFEvppNetMessage::HttpPost(const string& strUri, const string& strPostData, const HTTP_CLIENT_RESPONE& respone,
                               const map<std::string, std::string>& xHeaders, int timeout)
{
    if (!m_httpClient)
    {
        m_httpClient = NF_NEW NFCHttpClient();
    }

    return m_httpClient->HttpPost(strUri, strPostData, respone, xHeaders, timeout);
}

void NFEvppNetMessage::LoopSend(evpp::EventLoop* loop)
{
    --m_loopSendCount;
    CHECK_EXPR_ASSERT_NOT_RET(loop != NULL, "loop == NULL ERROR");
    CHECK_EXPR_ASSERT_NOT_RET(!loop->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND).IsEmpty(), "loop->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND).IsEmpty() ERROR");
    NF_SHARE_PTR<NFBuffer> pSendBuffer = evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(loop->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND));
    CHECK_EXPR_ASSERT_NOT_RET(pSendBuffer != NULL, "evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(loop->context(EVPP_LOOP_CONTEXT_1_MAIN_THREAD_SEND) NULL");
    auto pSendQueue = reinterpret_cast<NFCodeQueue*>(pSendBuffer->ReadAddr());
    CHECK_EXPR_ASSERT_NOT_RET(pSendQueue != NULL, "(NFCodeQueue*)pSendBuffer->ReadAddr() NULL");

    CHECK_EXPR_ASSERT_NOT_RET(!loop->context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER).IsEmpty(), "loop->context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER).IsEmpty() ERROR");
    NF_SHARE_PTR<NFBuffer> pComBuffer = evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(loop->context(EVPP_LOOP_CONTEXT_2_COMPRESS_BUFFER));
    CHECK_EXPR_ASSERT_NOT_RET(pComBuffer != NULL, "pComBuffer NULL");

    CHECK_EXPR_ASSERT_NOT_RET(!loop->context(EVPP_LOOP_CONTEXT_4_CODE_QUEUE_BUFFER).IsEmpty(), "loop->context(EVPP_LOOP_CONTEXT_4_CODE_QUEUE_BUFFER).IsEmpty() ERROR");
    NF_SHARE_PTR<NFBuffer> pCodeQueueBuffer = evpp::any_cast<NF_SHARE_PTR<NFBuffer>>(loop->context(EVPP_LOOP_CONTEXT_4_CODE_QUEUE_BUFFER));
    CHECK_EXPR_ASSERT_NOT_RET(pCodeQueueBuffer != NULL, "pCodeQueueBuffer NULL");

    while (pSendQueue->HasCode())
    {
        pCodeQueueBuffer->Clear();
        int iCodeLen = 0;
        int iRet = pSendQueue->Get(pCodeQueueBuffer->WriteAddr(), pCodeQueueBuffer->WritableSize(), iCodeLen);
        if (iRet || iCodeLen < static_cast<int>(sizeof(NFDataPackage)))
        {
            NFLogError(NF_LOG_DEFAULT, 0, "get code from pRecvQueue failed ret={}, codelen={}", iRet, iCodeLen);
            continue;
        }
        pCodeQueueBuffer->Produce(iCodeLen);

        // 先获取NetHead
        auto pCodePackage = reinterpret_cast<NFDataPackage*>(pCodeQueueBuffer->ReadAddr());
        if (iCodeLen != static_cast<int>(sizeof(NFDataPackage)) + static_cast<int>(pCodePackage->nMsgLen)) // 长度不一致
        {
            NFLogError(NF_LOG_DEFAULT, 0, "code length invalid. iCodeLen:{} != sizeof(NFDataPackage):{} + pCodePackage->nMsgLen:{}", iCodeLen,
                       sizeof(NFDataPackage), pCodePackage->nMsgLen);
            continue;
        }

        uint32_t parsePackageType = pCodePackage->nPacketParseType;
        bool isSecurity = pCodePackage->isSecurity;

        NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>> pConnMap = evpp::any_cast<NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>>>(loop->context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP));
        CHECK_EXPR_ASSERT_NOT_RET(pConnMap != NULL, "evpp::any_cast<NF_SHARE_PTR<std::unordered_map<uint64_t, evpp::TCPConnPtr>>>(loop->context(EVPP_LOOP_CONTEXT_3_CONNPTR_MAP)) Failed");

        auto iter = pConnMap->find(pCodePackage->nObjectLinkId);
        if (iter == pConnMap->end())
        {
            NFLogError(NF_LOG_DEFAULT, 0, "pConnMap->find(pCodePackage->m_objectLinkId) Failed, objectLinkId:{} maybe disconnect", pCodePackage->nObjectLinkId);
            continue;
        }

        evpp::TCPConnPtr pConn = iter->second;

        pComBuffer->Clear();
        NFPacketParseMgr::EnCode(parsePackageType, *pCodePackage, pCodeQueueBuffer->ReadAddr() + sizeof(NFDataPackage), pCodePackage->nMsgLen, *pComBuffer);
        pCodeQueueBuffer->Clear();

        if (isSecurity)
        {
            Encryption(pComBuffer->ReadAddr(), pComBuffer->ReadableSize());
        }

        pConn->Send(pComBuffer->ReadAddr(), pComBuffer->ReadableSize());

        pComBuffer->Clear();
    }
}
