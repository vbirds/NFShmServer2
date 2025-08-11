// -------------------------------------------------------------------------
//    @FileName         :    NFCBusClient.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCBusClient
//    @Desc             :    Bus客户端实现文件，提供进程间通信的客户端连接功能
//
// -------------------------------------------------------------------------

#include "NFCBusClient.h"
#include <sstream>
#include <string.h>
#include <NFCommPlugin/NFNetPlugin/NFPacketParseMgr.h>
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFNetPackagePool.h"

#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @file NFCBusClient.cpp
 * @brief Bus客户端实现文件
 * 
 * 该文件实现了Bus进程间通信的客户端功能，包括：
 * - Bus客户端的初始化和连接
 * - 与服务器的通信管理
 * - 消息发送和接收
 * - 连接状态监控
 * - 共享内存通信
 * 
 * 主要功能：
 * - 与Bus服务器建立连接
 * - 处理进程间消息传输
 * - 管理共享内存通信
 * - 消息发送和接收
 * - 连接状态管理
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief Bus客户端析构函数
 * 
 * 清理客户端资源，包括共享内存连接等
 */
NFCBusClient::~NFCBusClient()
{

}

/**
 * @brief 客户端心跳处理
 * 
 * 处理客户端的定时任务，包括连接状态检查等
 * 
 * @return 处理结果，0表示成功
 */
int NFCBusClient::Tick()
{
    return 0;
}

/**
 * @brief 初始化客户端
 * 
 * 建立与Bus服务器的连接，初始化共享内存通信
 * 
 * @return 初始化结果，0表示成功，-1表示失败
 */
int NFCBusClient::Init()
{
    // 连接到Bus服务器
    uint64_t linkId = ConnectServer(m_flag, m_bindFlag);
    if (linkId == 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "ConnectServer Failed!");
        return -1;
    }
    return 0;
}

/**
 * @brief 关闭客户端
 * 
 * 关闭客户端连接，清理相关资源
 * 
 * @return 关闭结果，0表示成功
 */
int NFCBusClient::Shut()
{
    return 0;
}

/**
 * @brief 释放客户端资源
 * 
 * 清理客户端占用的所有资源
 * 
 * @return 释放结果，0表示成功
 */
int NFCBusClient::Finalize()
{
    return 0;
}

/**
 * @brief 连接到Bus服务器
 * 
 * 建立与指定Bus服务器的连接，包括：
 * - 验证Bus参数的有效性
 * - 附加或初始化共享内存
 * - 设置连接记录信息
 * - 注册客户端到服务器
 * 
 * @param flag 连接标志
 * @param bindFlag 绑定标志
 * @return 连接ID，0表示连接失败
 */
uint64_t NFCBusClient::ConnectServer(const NFMessageFlag& flag, const NFMessageFlag&)
{
    // 验证Bus参数
    if (flag.mBusId <= 0 || flag.mBusLength <= 4096)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "busid:{} busLength:{} error!", NFServerIDUtil::GetBusNameFromBusID(flag.mBusId), flag.mBusLength);
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
    NFShmRecordType * pShmRecord = GetShmRecord();
    if (pShmRecord == nullptr)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "GetShmRecord failed, busid:{} ", NFServerIDUtil::GetBusNameFromBusID(flag.mBusId));
        return 0;
    }

    // 设置连接记录信息
    pShmRecord->m_nOwner = false;
    pShmRecord->m_nBusId = flag.mBusId;
    pShmRecord->m_nBusLength = flag.mBusLength;
    pShmRecord->m_packetParseType = flag.mPacketParseType;
    pShmRecord->m_nUnLinkId = GetUnLinkId(NF_IS_BUS, m_serverType, flag.mBusId, 0);
    SetLinkId(pShmRecord->m_nUnLinkId);
    SetConnectionType(NF_CONNECTION_TYPE_TCP_CLIENT);

    // 查找并注册到服务器
    bool find = false;
    auto head = (NFShmChannelHead*)pShmRecord->m_nBuffer;
    for (size_t i = 0; i < ARRAYSIZE(head->m_nShmAddr.m_srcLinkId); i++)
    {
        if (head->m_nShmAddr.m_srcLinkId[i] == m_bindFlag.mLinkId)
        {
            find = true;
            break;
        }
    }

    if (!find)
    {
        for (size_t i = 0; i < ARRAYSIZE(head->m_nShmAddr.m_srcLinkId); i++)
        {
            if (head->m_nShmAddr.m_srcLinkId[i] == m_bindFlag.mLinkId)
            {
                break;
            }

            uint64_t curValue = 0;
            bool f = head->m_nShmAddr.m_srcLinkId[i].compare_exchange_strong(curValue, m_bindFlag.mLinkId);
            if (f)
            {
                find = true;
                head->m_nShmAddr.m_srcBusLength[i] = m_bindFlag.mBusLength;
                head->m_nShmAddr.m_srcParseType[i] = m_bindFlag.mPacketParseType;
                if (flag.bActivityConnect)
                {
                    head->m_nShmAddr.m_bActiveConnect[i] = true;
                }
                else
                {
                    head->m_nShmAddr.m_bActiveConnect[i] = false;
                }
                break;
            }
        }
    }

    if (!find)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "ConnectServer:{} failed! the bus no seat, too many connection!", NFServerIDUtil::GetBusNameFromBusID(flag.mBusId));
    }

    if (flag.bActivityConnect)
    {
        SendBusConnectMsg(m_bindFlag.mBusId, m_bindFlag.mBusLength);
    }

    return static_cast<int64_t>(pShmRecord->m_nUnLinkId);
}

/**
 * @brief 通过共享内存通道发送数据
 * 
 * 通过指定的共享内存通道发送数据包
 * 
 * @param pChannel 共享内存通道指针
 * @param packetParseType 数据包解析类型
 * @param packet 数据包
 * @param msg 消息数据
 * @param nLen 数据长度
 * @return 发送结果
 */
bool NFCBusClient::Send(NFShmChannel* pChannel, int packetParseType, const NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    m_buffer.Clear();
    NFPacketParseMgr::EnCode(packetParseType, packet, msg, nLen, m_buffer, m_bindFlag.mLinkId);

    int iRet = ShmSend(pChannel, m_buffer.ReadAddr(), m_buffer.ReadableSize());
    if (iRet == 0)
    {
        return true;
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "ShmSend from:{} to:{} error:{}", NFServerIDUtil::GetBusNameFromBusID(m_bindFlag.mBusId), NFServerIDUtil::GetBusNameFromBusID(m_flag.mBusId), iRet);
    }
    return false;
}

/**
 * @brief 发送原始数据
 * 
 * 发送包含数据头的原始数据
 * 
 * @param packet 数据包
 * @param msg 发送的数据
 * @param nLen 数据的大小
 * @return 发送结果
 */
bool NFCBusClient::Send(NFDataPackage& packet, const char* msg, uint32_t nLen)
{
    NFShmRecordType * pShmRecord = GetShmRecord();
    if (pShmRecord == nullptr)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "GetShmRecord failed,");
        return false;
    }

    if (pShmRecord->m_nOwner == true)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "bus owner can't send data, unlinkId:{} ", pShmRecord->m_nUnLinkId);
        return false;
    }

    if (pShmRecord->m_nBuffer == nullptr)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "buffer = null, unlinkId:{} ", pShmRecord->m_nUnLinkId);
        return false;
    }


    auto head = (NFShmChannelHead*)pShmRecord->m_nBuffer;
    NFShmChannel* pChannel = nullptr; //&head->m_nShmChannel;
    if (NFGlobalSystem::Instance()->IsSpecialMsg(packet.mModuleId, packet.nMsgId))
    {
        pChannel = &head->m_nConnectChannel;
    }
    else {
        pChannel = &head->m_nShmChannel;
    }

    if (pChannel)
    {
        return Send(pChannel, pShmRecord->m_packetParseType, packet, msg, nLen);
    }

    return false;
}

/**
 * @brief 发送Protobuf消息
 * 
 * 发送Protobuf格式的消息
 * 
 * @param packet 数据包
 * @param xData Protobuf消息对象
 * @return 发送结果
 */
bool NFCBusClient::Send(NFDataPackage& packet, const google::protobuf::Message& xData)
{
    m_sendBuffer.Clear();
    int byteSize = xData.ByteSize();
    CHECK_EXPR(static_cast<int>(m_sendBuffer.WritableSize()) >= byteSize, false, "mxSendBuffer.WritableSize():{} < byteSize:{} msg:{}", m_sendBuffer.WritableSize(), byteSize, xData.DebugString());

    auto start = reinterpret_cast<uint8_t*>(m_sendBuffer.WriteAddr());
    uint8_t* end = xData.SerializeWithCachedSizesToArray(start);
    CHECK_EXPR(end - start == byteSize, false, "xData.SerializeWithCachedSizesToArray Failed:{}", xData.DebugString());
    m_sendBuffer.Produce(byteSize);

    return Send(packet, m_sendBuffer.ReadAddr(), m_sendBuffer.ReadableSize());
}

/**
 * @brief 检查是否已连接
 * 
 * 返回当前客户端的连接状态
 * 
 * @return 连接状态，true表示已连接，false表示未连接
 */
bool NFCBusClient::IsConnected()
{
    return m_isConnected;
}

/**
 * @brief 设置连接状态
 * 
 * 设置客户端的连接状态
 * 
 * @param connected 连接状态
 */
void NFCBusClient::SetConnected(bool connected)
{
    m_isConnected = connected;
}