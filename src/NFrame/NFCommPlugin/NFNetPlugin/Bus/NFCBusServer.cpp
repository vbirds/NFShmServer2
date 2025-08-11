// -------------------------------------------------------------------------
//    @FileName         :    NFCBusServer.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCBusServer
//    @Desc             :    Bus服务器实现文件，提供进程间通信的服务器端功能
//
// -------------------------------------------------------------------------


#include "NFCBusServer.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFNetPackagePool.h"
#include <string.h>
#include <NFCommPlugin/NFNetPlugin/NFPacketParseMgr.h>

#include "NFComm/NFPluginModule/NFIConfigModule.h"

/**
 * @file NFCBusServer.cpp
 * @brief Bus服务器实现文件
 * 
 * 该文件实现了Bus进程间通信的服务器端功能，包括：
 * - Bus服务器的初始化和绑定
 * - 客户端连接管理
 * - 消息路由和处理
 * - 共享内存通信管理
 * - 服务器状态监控
 * 
 * 主要功能：
 * - 监听客户端连接请求
 * - 管理多个客户端连接
 * - 处理进程间消息路由
 * - 维护共享内存通信通道
 * - 消息广播和转发
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief Bus服务器构造函数
 * 
 * 初始化Bus服务器，包括：
 * - 设置消息处理数量限制
 * - 初始化连接完成标志
 * - 读取服务器配置
 * 
 * @param p 插件管理器指针
 * @param serverType 服务器类型
 * @param flag 消息标志
 */
NFCBusServer::NFCBusServer(NFIPluginManager* p, NF_SERVER_TYPE serverType, const NFMessageFlag& flag): NFIBusConnection(p, serverType, flag)
{
    // 设置每帧处理消息数量限制
    m_handleMsgNumPerFrame = NF_NO_FIX_FAME_HANDLE_MAX_MSG_COUNT;
    m_isFinishConnect = false;
    
    // 读取服务器配置
    auto pServerConfig = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
    if (pServerConfig)
    {
        m_handleMsgNumPerFrame = pServerConfig->HandleMsgNumPerFrame;
    }
}

/**
 * @brief Bus服务器析构函数
 * 
 * 清理服务器资源，包括共享内存和连接信息
 */
NFCBusServer::~NFCBusServer()
{
}

/**
 * @brief 服务器心跳处理
 * 
 * 处理服务器的定时任务，包括消息处理
 * 
 * @return 处理结果，0表示成功
 */
int NFCBusServer::Tick()
{
    ProcessMsgLogicThread();
    return 0;
}

/**
 * @brief 初始化服务器
 * 
 * 绑定Bus服务器，初始化共享内存通信
 * 
 * @return 初始化结果，0表示成功，-1表示失败
 */
int NFCBusServer::Init()
{
    // 绑定Bus服务器
    uint64_t linkId = BindServer(m_flag);
    if (linkId == 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "BindServer Failed!");
        return -1;
    }

    return 0;
}

/**
 * @brief 关闭服务器
 * 
 * 清理服务器资源，包括回调函数
 * 
 * @return 关闭结果，0表示成功
 */
int NFCBusServer::Shut()
{
    m_busMsgPeerCb = nullptr;

    return 0;
}

/**
 * @brief 释放服务器资源
 * 
 * 清理服务器占用的所有资源
 * 
 * @return 释放结果，0表示成功
 */
int NFCBusServer::Finalize()
{
    return 0;
}

/**
 * @brief 绑定Bus服务器
 * 
 * 建立Bus服务器绑定，包括：
 * - 验证Bus参数的有效性
 * - 附加或初始化共享内存
 * - 设置连接记录信息
 * - 初始化服务器状态
 * 
 * @param flag 绑定标志
 * @return 绑定ID，0表示绑定失败
 */
uint64_t NFCBusServer::BindServer(const NFMessageFlag& flag)
{
    // 验证Bus参数
    if (flag.mBusId <= 0 || flag.mBusLength <= 4096)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "busid:{} busLength:{} error!", flag.mBusId, flag.mBusLength);
        return 0;
    }

    // 尝试附加共享内存，如果失败则初始化新的共享内存
    int ret = AttachShm(static_cast<key_t>(flag.mBusId), flag.mBusLength);
    if (ret < 0)
    {
        ret = InitShm(static_cast<key_t>(flag.mBusId), flag.mBusLength);
    }

    if (ret < 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "bus init failed:{} ", ret);
        return 0;
    }

    // 获取共享内存记录
    NFShmRecordType* pShmRecord = GetShmRecord();
    if (pShmRecord == nullptr)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "GetShmRecord failed, busid:{} ", flag.mBusId);
        return 0;
    }

    // 设置服务器记录信息
    pShmRecord->m_nOwner = true;
    pShmRecord->m_nBusId = flag.mBusId;
    pShmRecord->m_nBusLength = flag.mBusLength;
    pShmRecord->m_packetParseType = flag.mPacketParseType;
    pShmRecord->m_nUnLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, flag.mBusId, 0);
    m_bindFlag = flag;
    m_bindFlag.mLinkId = pShmRecord->m_nUnLinkId;
    SetLinkId(pShmRecord->m_nUnLinkId);
    SetConnectionType(NF_CONNECTION_TYPE_TCP_SERVER);

    NFShmChannelHead* head = (NFShmChannelHead*)pShmRecord->m_nBuffer;
    if (head->m_nShmAddr.m_dstLinkId == 0)
    {
        head->m_nShmAddr.m_dstLinkId = pShmRecord->m_nUnLinkId;
    }
    else
    {
        if (head->m_nShmAddr.m_dstLinkId != pShmRecord->m_nUnLinkId)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "shm dst linkId:{} != now linkId:{} ", head->m_nShmAddr.m_dstLinkId, pShmRecord->m_nUnLinkId);
            return 0;
        }
    }

    return pShmRecord->m_nUnLinkId;
}

/**
 * @brief 主线程处理消息队列
 */
void NFCBusServer::ProcessMsgLogicThread()
{
    size_t maxTimes = NF_NO_FIX_FAME_HANDLE_MAX_MSG_COUNT;
    if (!m_pObjPluginManager->IsLoadAllServer() && m_pObjPluginManager->IsFixedFrame())
    {
        maxTimes = NF_FIX_FRAME_HANDLE_MAX_MSG_COUNT;
    }

    NFShmRecordType* pShmRecord = GetShmRecord();
    if (pShmRecord->m_nOwner)
    {
        NFShmChannelHead* head = (NFShmChannelHead*)pShmRecord->m_nBuffer;
        NFShmChannel* pConnectChannel = &head->m_nConnectChannel;
        NFShmChannel* pChannel = &head->m_nShmChannel;
        size_t leftTimes = maxTimes;
        m_connectBuffer.Clear();
        while (true)
        {
            size_t recvLen = 0;
            int iRecvRet = ShmRecv(pConnectChannel, m_connectBuffer.WriteAddr(), m_connectBuffer.WritableSize(), &recvLen);

            if (iRecvRet == NFrame::ERR_CODE_NFBUS_ERR_NO_DATA)
            {
                break;
            }

            m_connectBuffer.Produce(recvLen);

            // 回调收到数据事件
            if (iRecvRet < 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "Shm Recv Error:{}", GetErrorStr(iRecvRet));
                break;
            }
            else
            {
                while (true)
                {
                    char* outData = nullptr;
                    uint32_t outLen = 0;
                    uint32_t allLen = 0;
                    NFDataPackage dataPacket;
                    int iRetCode = NFPacketParseMgr::DeCode(pShmRecord->m_packetParseType, m_connectBuffer.ReadAddr(), m_connectBuffer.ReadableSize(), outData, outLen, allLen, dataPacket);
                    if (iRetCode < 0)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "nfbus parse data failed!");
                        m_connectBuffer.Clear();
                        break;
                    }
                    else if (iRetCode > 0)
                    {
                        break;
                    }
                    else
                    {
                        m_connectBuffer.Consume(allLen);

                        dataPacket.nBuffer = outData;
                        dataPacket.nMsgLen = outLen;


                        if (dataPacket.mModuleId == NF_MODULE_FRAME && dataPacket.nMsgId == NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_REQ)
                        {
                            m_busMsgPeerCb(eMsgType_CONNECTED, dataPacket.nSendBusLinkId, dataPacket.nSendBusLinkId, dataPacket);
                        }
                        else if (dataPacket.mModuleId == NF_MODULE_FRAME && dataPacket.nMsgId == NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_RSP)
                        {
                            m_busMsgPeerCb(eMsgType_CONNECTED, dataPacket.nSendBusLinkId, dataPacket.nSendBusLinkId, dataPacket);
                        }
                        else
                        {
                            m_busMsgPeerCb(eMsgType_RECIVEDATA, dataPacket.nSendBusLinkId, dataPacket.nSendBusLinkId, dataPacket);
                        }
                    }
                }
            }
        }

        m_buffer.Clear();
        if (!m_isFinishConnect)
        {
            m_isFinishConnect = m_pObjPluginManager->IsFinishAppTask(m_serverType, APP_INIT_TASK_GROUP_SERVER_CONNECT);
        }
        while (leftTimes-- > 0 && m_isFinishConnect)
        {
            size_t recvLen = 0;
            int iRecvRet = ShmRecv(pChannel, m_buffer.WriteAddr(), m_buffer.WritableSize(), &recvLen);

            if (iRecvRet == NFrame::ERR_CODE_NFBUS_ERR_NO_DATA)
            {
                break;
            }

            m_buffer.Produce(recvLen);

            // 回调收到数据事件
            if (iRecvRet < 0)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "Shm Recv Error:{}", GetErrorStr(iRecvRet))
                break;
            }
            else
            {
                while (true)
                {
                    char* outData = nullptr;
                    uint32_t outLen = 0;
                    uint32_t allLen = 0;
                    NFDataPackage dataPacket;
                    int iDecodeRet = NFPacketParseMgr::DeCode(pShmRecord->m_packetParseType, m_buffer.ReadAddr(), m_buffer.ReadableSize(), outData, outLen, allLen, dataPacket);
                    if (iDecodeRet < 0)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "nfbus parse data failed!");
                        m_buffer.Clear();
                        break;
                    }
                    else if (iDecodeRet > 0)
                    {
                        break;
                    }
                    else
                    {
                        m_buffer.Consume(allLen);

                        dataPacket.nBuffer = outData;
                        dataPacket.nMsgLen = outLen;


                        if (dataPacket.mModuleId == NF_MODULE_FRAME && dataPacket.nMsgId == NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_REQ)
                        {
                            m_busMsgPeerCb(eMsgType_CONNECTED, dataPacket.nSendBusLinkId, dataPacket.nSendBusLinkId, dataPacket);
                        }
                        else if (dataPacket.mModuleId == NF_MODULE_FRAME && dataPacket.nMsgId == NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_RSP)
                        {
                            m_busMsgPeerCb(eMsgType_CONNECTED, dataPacket.nSendBusLinkId, dataPacket.nSendBusLinkId, dataPacket);
                        }
                        else
                        {
                            m_busMsgPeerCb(eMsgType_RECIVEDATA, dataPacket.nSendBusLinkId, dataPacket.nSendBusLinkId, dataPacket);
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 发送原始数据
 * 
 * Bus服务器不支持直接发送数据，记录错误日志
 * 
 * @param packet 数据包
 * @param msg 消息数据
 * @param nLen 数据长度
 * @return 始终返回false
 */
bool NFCBusServer::Send(NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    NFLogError(NF_LOG_DEFAULT, 0, "Bus Server Can't Send Data............., packet:{}", packet.ToString());
    return false;
}

/**
 * @brief 发送Protobuf消息
 * 
 * Bus服务器不支持直接发送数据，记录错误日志
 * 
 * @param packet 数据包
 * @param xData Protobuf消息对象
 * @return 始终返回false
 */
bool NFCBusServer::Send(NFDataPackage& packet, const google::protobuf::Message& xData)
{
    NFLogError(NF_LOG_DEFAULT, 0, "Bus Server Can't Send Data............., packet:{}", packet.ToString());
    return false;
}


