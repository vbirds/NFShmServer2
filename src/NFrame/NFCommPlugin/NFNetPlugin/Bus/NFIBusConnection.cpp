// -------------------------------------------------------------------------
//    @FileName         :    NFIBusConnect.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIBusConnect.cpp
//
// -------------------------------------------------------------------------

#include "NFIBusConnection.h"
#include <iomanip>
#include <sstream>
#include <string.h>
#include "NFBusHash.h"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFCore/NFStringUtility.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFNetPackagePool.h"
#if NF_PLATFORM == NF_PLATFORM_WIN
#else
#include <sys/mman.h>
#endif

/**
 * @brief 获得连接IP
 *
 * @param  usLinkId
 * @return std::string
 */
std::string NFIBusConnection::GetLinkIp()
{
    return m_flag.mBusName;
}

/**
* @brief 关闭连接
*
* @param  usLinkId
* @return
*/
void NFIBusConnection::CloseLinkId()
{
    SetConnected(false);
}

/**
 * @brief 获取数据节点head
 * @param channel 内存通道
 * @param index 节点索引
 * @param data 数据区起始地址
 * @param dataLen 到缓冲区末尾的长度
 * @return 节点head指针
 */
volatile NFShmNodeHead* NFIBusConnection::GetNodeHead(NFShmChannel* channel, size_t index, void** data, size_t* dataLen)
{
    assert(channel);
    assert(index < channel->m_nNodeCount);

    char *buf = (char *) channel;
    buf += channel->m_nAreaHeadOffset - channel->m_nAreaChannelOffset;
    buf += index * NFShmBlock::NODE_HEAD_SIZE;

    if (data || dataLen)
    {
        char *data_ = (char *) channel + channel->m_nAreaDataOffset - channel->m_nAreaChannelOffset;
        data_ += index * NFShmBlock::NODE_DATA_SIZE;

        if (data) (*data) = (void *) data_;

        if (dataLen) (*dataLen) = channel->m_nAreaEndOffset - channel->m_nAreaChannelOffset + (char*)channel - data_;
    }

    return (volatile NFShmNodeHead *) (void *) buf;
}

/**
 * @brief 获取数据块head
 * @param channel 内存通道
 * @param index 节点索引
 * @return 数据块head指针
 */
NFShmBlockHead* NFIBusConnection::GetBlockHead(NFShmChannel* channel, size_t index, void** data, size_t* dataLen)
{
    assert(channel);
    assert(index < channel->m_nNodeCount);

    char *buf = (char *) channel + channel->m_nAreaDataOffset - channel->m_nAreaChannelOffset;
    buf += index * NFShmBlock::NODE_DATA_SIZE;

    if (data) (*data) = (void*)(buf + NFShmBlock::BLOCK_HEAD_SIZE);

    if (dataLen)
        (*dataLen) = channel->m_nAreaEndOffset - channel->m_nAreaChannelOffset + (char*)channel - buf - NFShmBlock::BLOCK_HEAD_SIZE;

    return (NFShmBlockHead *) (void *) buf;
}

/**
 * @brief 获取后面的数据块index
 * @param channel 内存通道
 * @param index 节点索引
 * @param offset 索引偏移
 * @return 数据块head指针
 */
size_t NFIBusConnection::GetNextIndex(NFShmChannel *channel, size_t index, size_t offset)
{
    assert(channel);
    return (index + offset) % channel->m_nNodeCount;
}

/**
 * @brief 获取可用的数据节点数量
 * @param channel 内存通道
 * @param readCur 当前读游标
 * @param writeCur 当前写游标
 * @return 可用的节点数量
 */
size_t NFIBusConnection::GetAvailableNodeCount(NFShmChannel* channel, size_t readCur, size_t writeCur)
{
    assert(channel && channel->m_nNodeCount);

    // 要留下一个node做tail, 所以多减1
    size_t ret = (readCur + channel->m_nNodeCount - writeCur - 1) % channel->m_nNodeCount;
    if (ret >= channel->m_nConf.m_nProtectNodeCount)
    {
        ret -= channel->m_nConf.m_nProtectNodeCount;
    } else
    {
        ret = 0;
    }

    return ret;
}

/**
 * @brief 获取使用的数据节点数量
 * @param channel 内存通道
 * @param beginCur 起始游标
 * @param endCur 结束游标
 * @return 使用的数据节点数量
 */
size_t NFIBusConnection::GetNodeRangeCount(NFShmChannel* channel, size_t beginCur, size_t endCur)
{
    assert(channel && channel->m_nNodeCount);
    return (endCur + channel->m_nNodeCount - beginCur) % channel->m_nNodeCount;
}

/**
 * @brief 获取前面的的数据块index
 * @param channel 内存通道
 * @param index 节点索引
 * @param offset 索引偏移
 * @return 数据块head指针
 */
// static inline size_t mem_previous_index(mem_channel* channel, size_t index, size_t offset) {
//    assert(channel);
//    return (index + channel->node_count - offset) % channel->node_count;
//}

uint32_t NFIBusConnection::FetchOperationSeq(NFShmChannel *channel)
{
    uint32_t ret = ++channel->m_nAtomicOperationSeq;
    while (0 == ret)
    {
        ret = ++channel->m_nAtomicOperationSeq;
    }

    return ret;
}

/**
 * @brief 计算一定长度数据需要的数据node数量
 * @param len 数据长度
 * @return 数据长度需要的数据块数量
 */
size_t NFIBusConnection::CalcNodeNum(NFShmChannel *channel, size_t len)
{
    assert(channel);
    // channel->node_size 必须是2的N次方，所以使用优化算法
    return (len + NFShmBlock::BLOCK_HEAD_SIZE + channel->m_nNodeSize - 1) >> channel->m_nNodeSizeBinPower;
}

/**
 * @brief 生成校验码
 * @param src 源数据
 * @param len 数据长度
 * @note Hash 快速校验
 */
NFDataAlignType NFIBusConnection::FastCheck(const void *src, size_t len)
{
    return static_cast<NFDataAlignType>(HashFactor<sizeof(NFDataAlignType) >= sizeof(uint64_t)>::Hash(0, src, len));
}

void NFIBusConnection::ShowShmChannel(NFShmChannel* channel, std::ostream& out, bool needNodeStatus, size_t needNodeData)
{
    if (nullptr == channel)
    {
        return;
    }

    size_t read_cur = channel->m_nAtomicReadCur.load();
    size_t write_cur = channel->m_nAtomicWriteCur.load();
    size_t available_node = GetAvailableNodeCount(channel, read_cur, write_cur);
    size_t node_size = channel->m_nNodeSize;
    size_t node_count = channel->m_nNodeCount;

    out << "Summary:" << std::endl
        << "\tchannel node size: " << node_size << std::endl
        << "\tchannel node count: " << node_count << std::endl
        << "\tchannel using memory size: " << (channel->m_nAreaEndOffset - channel->m_nAreaChannelOffset) << std::endl
        << "\tchannel available node number: " << available_node << std::endl
        << "\tchannel buffer size: " << (node_size * node_count) << std::endl
        << "\tchannel used buffer size: " << ((node_size - available_node) * node_count) << std::endl
        << "\tchannel free buffer size: " << (available_node * node_count) << std::endl
        << std::endl;

    out << "Configure:" << std::endl
        << "\tsend timeout(ms): " << channel->m_nConf.m_nConfSendTimeoutMs << std::endl
        << "\tprotect memory size(Bytes): " << channel->m_nConf.m_nProtectMemorySize << std::endl
        << "\tprotect node number: " << channel->m_nConf.m_nProtectNodeCount << std::endl
        << "\twrite retry times: " << channel->m_nConf.m_nWriteRetryTimes << std::endl
        << std::endl;

    out << "IO:" << std::endl
        << "\tfirst waiting time: " << channel->m_nFirstFailedWritingTime << std::endl
        << "\tread index: " << read_cur << std::endl
        << "\twrite index: " << write_cur << std::endl
        << "\toperation sequence: " << channel->m_nAtomicOperationSeq << std::endl
        << std::endl;

    out << "Statistics:" << std::endl
        << "\twrite - check sequence failed: " << channel->m_nWriteCheckSequenceFailedCount << std::endl
        << "\twrite - retry times: " << channel->m_nWriteRetryCount << std::endl
        << "\tread - bad node: " << channel->m_nReadBadNodeCount << std::endl
        << "\tread - bad block: " << channel->m_nReadBadBlockCount << std::endl
        << "\tread - write timeout: " << channel->m_nReadWriteTimeoutCount << std::endl
        << "\tread - check block size failed: " << channel->m_nReadCheckBlockSizeFailedCount << std::endl
        << "\tread - check node count failed: " << channel->m_nReadCheckNodeSizeFailedCount << std::endl
        << "\tread - check hash failed: " << channel->m_nReadCheckHashFailedCount << std::endl
        << std::endl;

    out << "Debug:" << std::endl
        << "\tlast action - channel: " << m_nLastActionChannelPtr << std::endl
        << "\tlast action - begin node index: " << m_nLastActionChannelBeginNodeIndex << std::endl
        << "\tlast action - end node index: " << m_nLastActionChannelEndNodeIndex << std::endl
        << std::endl;

    if (needNodeStatus)
    {
        out << std::endl << "Node head list:" << std::endl;
        for (size_t i = 0; i < channel->m_nNodeCount; ++i)
        {
            void *data_ptr = 0;
            volatile NFShmNodeHead *node_head = GetNodeHead(channel, i, &data_ptr, nullptr);
            bool start_node = CheckFlag(node_head->m_nFlag, MF_START_NODE);

            if (start_node)
            {
                NFShmBlockHead *block_head = GetBlockHead(channel, i, nullptr, nullptr);
                out << "Node index: " << std::setw(10) << i << " => seq=" << node_head->m_nOperationSeq << ", is start node=Yes"
                    << ", Data Length=" << block_head->m_nBufferSize << ", Hash=" << block_head->m_nFastCheck
                    << ", is written=" << (CheckFlag(node_head->m_nFlag, NF_WRITEN) ? "Yes" : "No") << ", data(Hex): ";
            } else
            {
                out << "Node index: " << std::setw(10) << i << " => seq=" << node_head->m_nOperationSeq << ", is start node=No"
                    << ", is written=" << (CheckFlag(node_head->m_nFlag, NF_WRITEN) ? "Yes" : "No") << ", data(Hex): ";
            }

            if (needNodeData < NFShmBlock::NODE_DATA_SIZE)
            {
                //util::string::dumphex(data_ptr, need_node_data, out);
            } else
            {
                //util::string::dumphex(data_ptr, shm_block::node_data_size, out);
            }
            out << std::endl;
        }

        out << "IO (after dump nodes):" << std::endl
            << "\tfirst waiting time: " << channel->m_nFirstFailedWritingTime << std::endl
            << "\tread index: " << channel->m_nAtomicReadCur << std::endl
            << "\twrite index: " << channel->m_nAtomicWriteCur << std::endl
            << "\toperation sequence: " << channel->m_nAtomicOperationSeq << std::endl
            << std::endl;
    }
}

void NFIBusConnection::GetShmStats(NFShmChannel *channel, NFShmStatsBlockError &out)
{
    memset(&out, 0, sizeof(out));
    if (nullptr == channel)
    {
        return;
    }

    out.m_nWriteCheckSequenceFailedCount = channel->m_nWriteCheckSequenceFailedCount;
    out.m_nWriteRetryCount = channel->m_nWriteRetryCount;

    out.m_nReadBadNodeCount = channel->m_nReadBadNodeCount;
    out.m_nReadBadBlockCount = channel->m_nReadBadBlockCount;
    out.m_nReadWriteTimeoutCount = channel->m_nReadWriteTimeoutCount;
    out.m_nReadCheckBlockSizeFailedCount = channel->m_nReadCheckBlockSizeFailedCount;
    out.m_nReadCheckNodeSizeFailedCount = channel->m_nReadCheckNodeSizeFailedCount;
    out.m_nReadCheckHashFailedCount = channel->m_nReadCheckHashFailedCount;
}

std::pair<size_t, size_t> NFIBusConnection::LastAction()
{
    return std::make_pair(m_nLastActionChannelBeginNodeIndex, m_nLastActionChannelEndNodeIndex);
}

int NFIBusConnection::AttachShmCheck(void *buffer, size_t len)
{
    // 缓冲区最小长度为数据头+空洞node的长度
    if (len < sizeof(NFShmChannelHead) + NFShmBlock::NODE_DATA_SIZE + NFShmBlock::NODE_HEAD_SIZE)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_SIZE_TOO_SMALL;
    }

    NFShmChannelHead *head = (NFShmChannelHead *) buffer;

    if (0 != strncmp(SHM_CHANNEL_NAME, head->m_nShmChannel.m_nNodeMagic, strlen(SHM_CHANNEL_NAME)))
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_BUFFER_INVALID;
    }

    if (0 != strncmp(SHM_CHANNEL_NAME, head->m_nConnectChannel.m_nNodeMagic, strlen(SHM_CHANNEL_NAME)))
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_BUFFER_INVALID;
    }

    // check channel version
    if (SHM_CHANNEL_VERSION != head->m_nShmChannel.m_channelVersion)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_UNSUPPORTED_VERSION;
    }

    if (NFBUS_MACRO_DATA_ALIGN_SIZE != head->m_nShmChannel.m_channelAlignSize)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_ALIGN_SIZE_MISMATCH;
    }

    if (sizeof(size_t) != head->m_nShmChannel.m_channelHostSize)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_ARCH_SIZE_T_MISMATCH;
    }

    if (SHM_CHANNEL_VERSION != head->m_nConnectChannel.m_channelVersion)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_UNSUPPORTED_VERSION;
    }

    if (NFBUS_MACRO_DATA_ALIGN_SIZE != head->m_nConnectChannel.m_channelAlignSize)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_ALIGN_SIZE_MISMATCH;
    }

    if (sizeof(size_t) != head->m_nConnectChannel.m_channelHostSize)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_ARCH_SIZE_T_MISMATCH;
    }

    return NFrame::ERR_CODE_SVR_OK;
}

int NFIBusConnection::AttachShm(key_t shmKey, size_t len)
{
    size_t real_size;
    void *buffer;

    int ret = OpenShmBuffer(shmKey, len, &buffer, &real_size, false);
    if (ret < 0) return ret;

    ret = AttachShmCheck(buffer, real_size);
    if (ret < 0)
    {
        CloseShmBuffer();
        return ret;
    }

    return ret;
}

int NFIBusConnection::InitShmBuffer(void *buffer, size_t len)
{
    // 缓冲区最小长度为数据头+空洞node的长度
    if (len < sizeof(NFShmChannelHead) + (NFShmBlock::CONNECT_SHM_SIZE) + (NFShmBlock::NODE_DATA_SIZE + NFShmBlock::NODE_HEAD_SIZE))
        return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_SIZE_TOO_SMALL;
    //if (len < sizeof(NFShmChannelHead) + (NFShmBlock::node_data_size + NFShmBlock::node_head_size))
    //    return NFrame::ERR_CODE_NFBUS_ERR_CHANNEL_SIZE_TOO_SMALL;

    memset(buffer, 0x00, len);
    NFShmChannelHead *head = (NFShmChannelHead *) buffer;
    ////////////////////////////////////m_nConnectChannel//////////////////////////////////////////////////////////
    // 节点计算
    head->m_nConnectChannel.m_nNodeSize = NFShmBlock::NODE_DATA_SIZE;
    {
        head->m_nConnectChannel.m_nNodeSizeBinPower = 0;
        size_t node_size = head->m_nConnectChannel.m_nNodeSize;
        while (node_size > 1)
        {
            node_size >>= 1;
            ++head->m_nConnectChannel.m_nNodeSizeBinPower;
        }
    }
    head->m_nConnectChannel.m_nNodeCount = (NFShmBlock::CONNECT_SHM_SIZE) / (head->m_nConnectChannel.m_nNodeSize + NFShmBlock::NODE_HEAD_SIZE);

    // 偏移位置计算
    head->m_nConnectChannel.m_nAreaChannelOffset = (char *) &head->m_nConnectChannel - (char *) buffer;
    head->m_nConnectChannel.m_nAreaHeadOffset = sizeof(NFShmChannelHead);
    head->m_nConnectChannel.m_nAreaDataOffset = head->m_nConnectChannel.m_nAreaHeadOffset + head->m_nConnectChannel.m_nNodeCount * NFShmBlock::NODE_HEAD_SIZE;
    head->m_nConnectChannel.m_nAreaEndOffset = head->m_nConnectChannel.m_nAreaDataOffset + head->m_nConnectChannel.m_nNodeCount * head->m_nConnectChannel.m_nNodeSize;

    // 配置初始化
    CreateDefaultConf(&head->m_nConnectChannel);

    static_assert(sizeof(head->m_nConnectChannel.m_nNodeMagic) >= (sizeof(SHM_CHANNEL_NAME) - 1), "magic text size error");

    memcpy(head->m_nConnectChannel.m_nNodeMagic, SHM_CHANNEL_NAME, sizeof(head->m_nConnectChannel.m_nNodeMagic));

    head->m_nConnectChannel.m_channelVersion = SHM_CHANNEL_VERSION;
    head->m_nConnectChannel.m_channelAlignSize = NFBUS_MACRO_DATA_ALIGN_SIZE;
    head->m_nConnectChannel.m_channelHostSize = static_cast<uint16_t>(sizeof(size_t));
    ////////////////////////////////////m_nConnectChannel//////////////////////////////////////////////////////////
    ////////////////////////////////////m_nShmChannel//////////////////////////////////////////////////////////
    // 节点计算
    head->m_nShmChannel.m_nNodeSize = NFShmBlock::NODE_DATA_SIZE;
    {
        head->m_nShmChannel.m_nNodeSizeBinPower = 0;
        size_t node_size = head->m_nShmChannel.m_nNodeSize;
        while (node_size > 1)
        {
            node_size >>= 1;
            ++head->m_nShmChannel.m_nNodeSizeBinPower;
        }
    }
    head->m_nShmChannel.m_nNodeCount = (len - NFShmBlock::CHANNEL_HEAD_SIZE - NFShmBlock::CONNECT_SHM_SIZE) / (head->m_nShmChannel.m_nNodeSize + NFShmBlock::NODE_HEAD_SIZE);
    //head->m_nShmChannel.m_nNodeCount = (len - NFShmBlock::channel_head_size) / (head->m_nShmChannel.m_nNodeSize + NFShmBlock::node_head_size);

    // 偏移位置计算
    head->m_nShmChannel.m_nAreaChannelOffset = (char *) &head->m_nShmChannel - (char *) buffer;
    head->m_nShmChannel.m_nAreaHeadOffset = sizeof(NFShmChannelHead) + NFShmBlock::CONNECT_SHM_SIZE;
    //head->m_nShmChannel.m_nAreaHeadOffset = sizeof(NFShmChannelHead);
    head->m_nShmChannel.m_nAreaDataOffset = head->m_nConnectChannel.m_nAreaEndOffset + head->m_nShmChannel.m_nNodeCount * NFShmBlock::NODE_HEAD_SIZE;
    //head->m_nShmChannel.m_nAreaDataOffset = head->m_nShmChannel.m_nAreaHeadOffset + head->m_nShmChannel.m_nNodeCount * NFShmBlock::node_head_size;
    head->m_nShmChannel.m_nAreaEndOffset = head->m_nShmChannel.m_nAreaDataOffset + head->m_nShmChannel.m_nNodeCount * head->m_nShmChannel.m_nNodeSize;

    // 配置初始化
    CreateDefaultConf(&head->m_nShmChannel);

    static_assert(sizeof(head->m_nShmChannel.m_nNodeMagic) >= (sizeof(SHM_CHANNEL_NAME) - 1), "magic text size error");

    memcpy(head->m_nShmChannel.m_nNodeMagic, SHM_CHANNEL_NAME, sizeof(head->m_nShmChannel.m_nNodeMagic));

    head->m_nShmChannel.m_channelVersion = SHM_CHANNEL_VERSION;
    head->m_nShmChannel.m_channelAlignSize = NFBUS_MACRO_DATA_ALIGN_SIZE;
    head->m_nShmChannel.m_channelHostSize = static_cast<uint16_t>(sizeof(size_t));
    ////////////////////////////////////m_nShmChannel//////////////////////////////////////////////////////////

    return NFrame::ERR_CODE_SVR_OK;
}

int NFIBusConnection::InitShm(key_t shmKey, size_t len)
{
    size_t real_size;
    void *buffer;

    int ret = OpenShmBuffer(shmKey, len, &buffer, &real_size, true);
    if (ret < 0) return ret;

    ret = InitShmBuffer(buffer, real_size);
    if (ret < 0) return ret;

    return 0;
}

int NFIBusConnection::CloseShm()
{
    return CloseShmBuffer();
}


/**
 * @brief 通过key找到相应的共享内存
 */
NFShmChannel *NFIBusConnection::GetShmChannel()
{
    CHECK_EXPR(m_pShmRecord, NULL, "");
    NFShmChannelHead *head = (NFShmChannelHead *) m_pShmRecord->m_nBuffer;
    return &head->m_nShmChannel;
}

/**
 * @brief 通过key找到相应的共享内存
 */
NFShmRecordType *NFIBusConnection::GetShmRecord()
{
    CHECK_EXPR(m_pShmRecord, NULL, "");
    return m_pShmRecord;
}

int NFIBusConnection::OpenShmBuffer(key_t shmKey, size_t len, void** data, size_t* realSize, bool create)
{
    NFShmRecordType shm_record;

    // 已经映射则直接返回
    if (m_pShmRecord != nullptr)
    {
        if (data) *data = (void *) m_pShmRecord->m_nBuffer;
        if (realSize) *realSize = m_pShmRecord->m_nSize;
        ++m_pShmRecord->m_nReferenceCount;
        return NFrame::ERR_CODE_SVR_OK;
    }

#ifdef _WIN32
    memset(&shm_record, 0, sizeof(shm_record));
    SYSTEM_INFO si;
    ::GetSystemInfo(&si);
    // size_t page_size = static_cast<std::size_t>(si.dwPageSize);

    char shm_file_name[64] = { 0 };
    // Use Global\\ prefix requires the SeCreateGlobalPrivilege privilege, so we do not use it
    std::string shmFileName = NF_FORMAT("nfbus_{}.bus", NFServerIDUtil::GetBusNameFromBusID(shmKey));
    std::wstring wShmFileName = NFStringUtility::s2ws(shmFileName);

    // 首先尝试直接打开
    shm_record.m_nHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, wShmFileName.c_str());

    if (nullptr != shm_record.m_nHandle) {
        shm_record.m_nBuffer = (LPTSTR)MapViewOfFile(shm_record.m_nHandle,   // handle to map object
            FILE_MAP_ALL_ACCESS, // read/write permission
            0, 0, len);

        if (nullptr == shm_record.m_nBuffer) {
            CloseHandle(shm_record.m_nHandle);
            return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;
        }

        if (data) *data = (void *)shm_record.m_nBuffer;
        if (realSize) *realSize = len;

        shm_record.m_nSize = len;
        shm_record.m_nReferenceCount = 1;

        if (m_pShmRecord == nullptr)
        {
            m_pShmRecord = NF_NEW NFShmRecordType();
            NF_ASSERT(m_pShmRecord);
        }

        *m_pShmRecord = shm_record;

        return NFrame::ERR_CODE_SVR_OK;
    }

    // 如果允许创建则创建
    if (!create) return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;

    HANDLE hFileID = CreateFile(wShmFileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFileID == nullptr)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;
    }

    shm_record.m_nHandle = CreateFileMapping(hFileID, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(len), wShmFileName.c_str());

    if (nullptr == shm_record.m_nHandle) return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;

    shm_record.m_nBuffer = (LPTSTR)MapViewOfFile(shm_record.m_nHandle,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0, 0, len);

    if (nullptr == shm_record.m_nBuffer) return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;

    shm_record.m_nSize = len;
    shm_record.m_nReferenceCount = 1;
    if (data) *data = (void *)shm_record.m_nBuffer;
    if (realSize) *realSize = len;

    if (m_pShmRecord == nullptr)
    {
        m_pShmRecord = NF_NEW NFShmRecordType();
        NF_ASSERT(m_pShmRecord);
    }
    *m_pShmRecord = shm_record;

#else
    // len 长度对齐到分页大小
    size_t page_size = ::sysconf(_SC_PAGESIZE);
    len = (len + page_size - 1) & (~(page_size - 1));

    int shmflag = 0666;
    if (create) shmflag |= IPC_CREAT;

#ifdef __linux__
    // linux下阻止从交换分区分配物理页
    shmflag |= SHM_NORESERVE;

    // 临时关闭大页表功能，等后续增加了以下判定之后再看情况加回来
    // 使用大页表要先判定 /proc/meminfo 内的一些字段内容，再配置大页表
    // -- Hugepagesize: 大页表的分页大小，如果ATBUS_MACRO_HUGETLB_SIZE小于这个值，要对齐到这个值
    // -- HugePages_Total: 大页表总大小
    // -- HugePages_Free: 大页表可用大小，如果可用值小于需要分配的空间，也不能用大页表
    //#ifdef ATBUS_MACRO_HUGETLB_SIZE
    //            // 如果大于4倍的大页表，则对齐到大页表并使用大页表
    //            if (len > (4 * ATBUS_MACRO_HUGETLB_SIZE)) {
    //                len = (len + (ATBUS_MACRO_HUGETLB_SIZE)-1) & (~((ATBUS_MACRO_HUGETLB_SIZE)-1));
    //                shmflag |= SHM_HUGETLB;
    //            }
    //#endif

#endif
    if (false)
    {
        shm_record.m_nShmId = 0;
        int open_flag = O_RDWR;
        if (create)
        {
            open_flag |= O_CREAT;
        }

        auto pServerData = FindModule<NFIConfigModule>()->GetAppConfig(m_serverType);
        CHECK_EXPR_ASSERT(pServerData, -1, "FindModule<NFIConfigModule>()->GetAppConfig(m_serverType) Faileds");
        shm_record.m_nShmPath = NF_FORMAT("./nfbus_{}_{}.bus", pServerData->ServerName, shmKey);
        shm_record.m_nShmFd = shm_open(shm_record.m_nShmPath.c_str(), open_flag, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (-1 == shm_record.m_nShmFd)
        {
            return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;
        }

        struct stat statbuf;
        if (0 != fstat(shm_record.m_nShmFd, &statbuf))
        {
            shm_unlink(shm_record.m_nShmPath.c_str());
            return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;
        }

        if (statbuf.st_size <= 0)
        {
            if (0 != ftruncate(shm_record.m_nShmFd, (off_t) len))
            {
                shm_unlink(shm_record.m_nShmPath.c_str());
                return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;
            }

            if (0 != fstat(shm_record.m_nShmFd, &statbuf))
            {
                shm_unlink(shm_record.m_nShmPath.c_str());
                return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;
            }

            shm_record.m_nSize = static_cast<size_t>(statbuf.st_size);
        } else
        {
            shm_record.m_nSize = static_cast<size_t>(statbuf.st_size);
        }

        int shm_map_flag = MAP_SHARED;
    #    ifdef __linux__
        shm_map_flag |= MAP_NORESERVE;
    #    endif
        shm_record.m_nBuffer = mmap(nullptr, shm_record.m_nSize, PROT_READ | PROT_WRITE, shm_map_flag, shm_record.m_nShmFd, 0);
        if (MAP_FAILED == shm_record.m_nBuffer)
        {
            shm_unlink(shm_record.m_nShmPath.c_str());
            return NFrame::ERR_CODE_NFBUS_ERR_SHM_MAP_FAILED;
        }
    }
    else
    {
        shm_record.m_nShmFd = 0;
        shm_record.m_nShmId = shmget(shmKey, len, shmflag);
        if (-1 == shm_record.m_nShmId) return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;

        // 获取实际长度
        struct shmid_ds shm_info;
        if (shmctl(shm_record.m_nShmId, IPC_STAT, &shm_info)) return NFrame::ERR_CODE_NFBUS_ERR_SHM_GET_FAILED;

        shm_record.m_nSize = shm_info.shm_segsz;

        // 获取地址
        shm_record.m_nBuffer = shmat(shm_record.m_nShmId, NULL, 0);
    }

    shm_record.m_nReferenceCount = 1;

    if (data)
    {
        *data = shm_record.m_nBuffer;
    }

    if (realSize)
    {
        *realSize = shm_record.m_nSize;
    }

    if (m_pShmRecord == NULL)
    {
        m_pShmRecord = NF_NEW NFShmRecordType();
        NF_ASSERT(m_pShmRecord);
    }

    *m_pShmRecord = shm_record;
#endif

    return NFrame::ERR_CODE_SVR_OK;
}

int NFIBusConnection::ShmSend(NFShmChannel *channel, const void *buf, size_t len)
{
    if (nullptr == channel) return NFrame::ERR_CODE_NFBUS_ERR_PARAMS;

    int ret = 0;
    size_t left_try_times = channel->m_nConf.m_nWriteRetryTimes;
    while (left_try_times-- > 0)
    {
        ret = ShmRealSend(channel, buf, len);

        // 原子操作序列冲突，重试
        if (NFrame::ERR_CODE_NFBUS_ERR_NODE_BAD_BLOCK_CSEQ_ID == ret || NFrame::ERR_CODE_NFBUS_ERR_NODE_BAD_BLOCK_WSEQ_ID == ret)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "ShmSend 原子操作序列冲突，重试 ");
            ++channel->m_nWriteRetryCount;
            continue;
        }

        return ret;
    }

    return ret;
}

int NFIBusConnection::ShmRecv(NFShmChannel* channel, void* buf, size_t len, size_t* recvSize)
{
    if (nullptr == channel) return NFrame::ERR_CODE_NFBUS_ERR_PARAMS;

    int ret = NFrame::ERR_CODE_SVR_OK;

    void *buffer_start = nullptr;
    size_t buffer_len = 0;
    NFShmBlockHead *block_head = nullptr;
    size_t read_begin_cur = channel->m_nAtomicReadCur.load();
    const size_t ori_read_cur = read_begin_cur;
    size_t read_end_cur;
    size_t write_cur = channel->m_nAtomicWriteCur.load();
    // std::atomic_thread_fence(std::memory_order_seq_cst);

    uint32_t timeout_operation_seq = 0;

    while (true)
    {
        read_end_cur = read_begin_cur;

        if (read_begin_cur == write_cur)
        {
            ret = ret ? ret : NFrame::ERR_CODE_NFBUS_ERR_NO_DATA;
            break;
        }

        volatile NFShmNodeHead *node_head = GetNodeHead(channel, read_begin_cur, nullptr, nullptr);

        /**
         * 这个时候，可能写出端处于移动了atomic_write_cur，但是还没有写出 MF_START_NODE 的情况。所以情况列举如下:
         *   MF_START_NODE | MF_WRITEN: 数据块已写完
         *   MF_WRITEN: 节点容错
         *   MF_START_NODE: 是起始节点但是数据未写完（也可能是发送端在写出过程中崩溃）
         *   0: 移动游标后尚未设置MF_START_NODE，这个出现概率非常低，但是也会出现。（也可能是发送端在写出过程中崩溃）
         *
         * 由于MF_START_NODE和0都是无法判定是没写完还是写出端崩溃的，所以都要走超时检测逻辑。
         * 但是如果被判定超时并且写出端只写出了部分节点的的 MF_WRITEN 这时候剩下的节点的flag都会是0。
         *   如果这些都通过超时机制判定，则最多可能等待 消息长度*超时判定时长/节点长度，默认设置是最少2秒钟。
         *   所以这里需要特别处理下，当进入超时流程后，所有非 MF_START_NODE 并且operation_seq相等的节点也应该视为错误。
         *   注意上面这个流程只能在超时流程中进行，因为其他错误流程可能第一个数据块错误，但是紧接着的第二个数据块处于正在写出的状态而没有设置
         *   MF_START_NODE和operation_seq。我们的operation_seq取值范围是uint32，所以max(uint32)*节点长度（默认是500GB）以内的通道里operation_seq不会重复
         *   我们的数据通道不可能使用这么大的内存，所以加上operation_seq后能尽可能地消除空数据快的超时影响
         */
        // 容错处理 -- 未写入完成
        if (likely(CheckFlag(node_head->m_nFlag, NF_WRITEN)))
        {
            // 容错处理 -- 不是起始节点
            if (unlikely(!CheckFlag(node_head->m_nFlag, MF_START_NODE)))
            {
                read_begin_cur = GetNextIndex(channel, read_begin_cur, 1);
                node_head->m_nFlag = 0;

                ++channel->m_nReadBadNodeCount;
                continue;
            }

            // 容错处理 -- 如果前面已经发现错误，这里就不能再消耗 MF_START_NODE了
            // 防止后面把block弹出却没有读出数据并返回错误码
            if (unlikely(ret))
            {
                break;
            }

        } else
        {
            uint64_t cnow = (uint64_t) (clock() / (CLOCKS_PER_SEC / 1000)); // 转换到毫秒

            // 上面提到的快速跳过流程
            if (unlikely(timeout_operation_seq && timeout_operation_seq == node_head->m_nOperationSeq &&
                         !CheckFlag(node_head->m_nFlag, MF_START_NODE)))
            {
                read_begin_cur = GetNextIndex(channel, read_begin_cur, 1);
                node_head->m_nFlag = 0;

                ++channel->m_nReadBadNodeCount;
                continue;
            }

            // 初次读取超时
            if (!channel->m_nFirstFailedWritingTime)
            {
                channel->m_nFirstFailedWritingTime = cnow;
                ret = ret ? ret : NFrame::ERR_CODE_NFBUS_ERR_NO_DATA;
                break;
            }

            uint64_t cd = cnow > channel->m_nFirstFailedWritingTime ? cnow - channel->m_nFirstFailedWritingTime
                                                                    : channel->m_nFirstFailedWritingTime - cnow;
            // 写入超时
            if (channel->m_nFirstFailedWritingTime && cd > channel->m_nConf.m_nConfSendTimeoutMs)
            {
                timeout_operation_seq = node_head->m_nOperationSeq;

                read_begin_cur = GetNextIndex(channel, read_begin_cur, 1);
                node_head->m_nFlag = 0;

                ++channel->m_nReadBadNodeCount;
                ++channel->m_nReadBadBlockCount;
                ++channel->m_nReadWriteTimeoutCount;

                channel->m_nFirstFailedWritingTime = 0;
                continue;
            }

            // 未到超时时间
            ret = ret ? ret : NFrame::ERR_CODE_NFBUS_ERR_NO_DATA;
            break;
        }

        // 数据检测
        block_head = GetBlockHead(channel, read_begin_cur, &buffer_start, &buffer_len);

        // 缓冲区长度异常
        if (!block_head->m_nBufferSize ||
            block_head->m_nBufferSize >= channel->m_nAreaEndOffset - channel->m_nAreaDataOffset - channel->m_nConf.m_nProtectMemorySize)
        {
            ret = ret ? ret : NFrame::ERR_CODE_NFBUS_ERR_NODE_BAD_BLOCK_BUFF_SIZE;

            read_begin_cur = GetNextIndex(channel, read_begin_cur, 1);
            node_head->m_nFlag = 0;

            ++channel->m_nReadBadNodeCount;
            ++channel->m_nReadCheckBlockSizeFailedCount;
            continue;
        }

        // 写出的缓冲区不足
        if (block_head->m_nBufferSize > len)
        {
            ret = ret ? ret : NFrame::ERR_CODE_NFBUS_ERR_BUFF_LIMIT;
            if (recvSize) *recvSize = block_head->m_nBufferSize;

            break;
        }


        // 重置操作码（防冲突+读检测）
        uint32_t check_opr_seq = node_head->m_nOperationSeq;
        for (read_end_cur = read_begin_cur; read_end_cur != write_cur; read_end_cur = GetNextIndex(channel, read_end_cur, 1))
        {
            volatile NFShmNodeHead *this_node_head = GetNodeHead(channel, read_end_cur, nullptr, nullptr);
            if (this_node_head->m_nOperationSeq != check_opr_seq)
            {
                break;
            }

            // 如果出现异常了两个连续写入块有相同的operation_seq，会在这里被会切割开
            if (read_end_cur != read_begin_cur && CheckFlag(this_node_head->m_nFlag, MF_START_NODE))
            {
                break;
            }

            // 如果前面触发了超时保护，则会有一批节点的operation_seq未被清空。为保证行为一致，所以这里也不再清空 operation_seq 了
            // this_node_head->operation_seq = 0;
            this_node_head->m_nFlag = 0;
        }

        // 有效的node数量检查
        {
            size_t nodes_num = GetNodeRangeCount(channel, read_begin_cur, read_end_cur);
            if (CalcNodeNum(channel, block_head->m_nBufferSize) != nodes_num)
            {
                ret = ret ? ret : NFrame::ERR_CODE_NFBUS_ERR_NODE_BAD_BLOCK_NODE_NUM;
                read_begin_cur = GetNextIndex(channel, read_begin_cur, 1);
                // 上面的循环已经重置过flag了

                ++channel->m_nReadBadNodeCount;
                ++channel->m_nReadCheckNodeSizeFailedCount;
                continue;
            }
        }

        break;
    }


    do
    {
        // 出错退出, 移动读游标到最后读取位置
        if (ret)
        {
            break;
        }

        // 设置屏障，保证这个执行前数据区和head区内存已被刷入
        std::atomic_thread_fence(std::memory_order_acquire);

        channel->m_nFirstFailedWritingTime = 0;

        // 接收数据 - 无回绕

        if (block_head->m_nBufferSize <= buffer_len)
        {
            memcpy(buf, buffer_start, block_head->m_nBufferSize);

        } else
        { // 接收数据 - 有回绕
            memcpy(buf, buffer_start, buffer_len);

            // 回绕nodes
            GetNodeHead(channel, 0, &buffer_start, nullptr);
            memcpy((char *) buf + buffer_len, buffer_start, block_head->m_nBufferSize - buffer_len);
        }
        NFDataAlignType fast_check = FastCheck(buf, block_head->m_nBufferSize);

        if (recvSize) *recvSize = block_head->m_nBufferSize;

        // 校验不通过
        if (fast_check != block_head->m_nFastCheck)
        {
            ++channel->m_nReadCheckHashFailedCount;
            ret = ret ? ret : NFrame::ERR_CODE_NFBUS_ERR_BAD_DATA;
        }

    } while (false);

    // 设置游标
    if (ori_read_cur != read_end_cur)
    {
        channel->m_nAtomicReadCur.store(read_end_cur);
        // 不再访问数据区和head区了，所以不再需要memory barrier了
    }

    // 用于调试的节点编号信息
    m_nLastActionChannelBeginNodeIndex = ori_read_cur;
    m_nLastActionChannelEndNodeIndex = read_end_cur;
    m_nLastActionChannelPtr = channel;
    return ret;
}


int NFIBusConnection::ShmRealSend(NFShmChannel *channel, const void *buf, size_t len)
{
    if (nullptr == channel) return NFrame::ERR_CODE_NFBUS_ERR_PARAMS;

    if (0 == len) return NFrame::ERR_CODE_SVR_OK;

    size_t nodeCount = CalcNodeNum(channel, len);
    // 要写入的数据比可用的缓冲区还大
    if (nodeCount >= channel->m_nNodeCount - channel->m_nConf.m_nProtectNodeCount)
    {
        return NFrame::ERR_CODE_NFBUS_ERR_BUFF_LIMIT;
    }

    // 获取操作序号
    uint32_t oprSeq = FetchOperationSeq(channel);

    // 游标操作
    size_t readCur = 0;
    size_t newWriteCur, writeCur = channel->m_nAtomicWriteCur.load();
    unsigned char retryTimes = 0;

    while (true)
    {
        readCur = channel->m_nAtomicReadCur.load();
        // std::atomic_thread_fence(std::memory_order_seq_cst);

        // 要留下一个node做tail, 所以多减1
        size_t availableNode = GetAvailableNodeCount(channel, readCur, writeCur);
        if (nodeCount > availableNode)
        {
            return NFrame::ERR_CODE_NFBUS_ERR_BUFF_LIMIT;
        }

        // 新的尾部node游标
        newWriteCur = GetNextIndex(channel, writeCur, nodeCount);

        // @see http://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
        // @see https://en.wikipedia.org/wiki/Load-link/store-conditional
        // CAS, 使用compare_exchange_weak在MIPS、ARM等架构上可能低概率出现可以成功但是走了失败流程，这里会自动重试
        bool f = channel->m_nAtomicWriteCur.compare_exchange_weak(writeCur, newWriteCur);

        if (likely(f)) break;

        // 发现冲突原子操作失败则重试
        // 增加补偿策略(bkoff)，防止高竞争时多个进程/线程之间频繁冲突
        ++retryTimes;
        __UTIL_LOCK_SPIN_LOCK_WAIT(retryTimes);
    }
    m_nLastActionChannelBeginNodeIndex = writeCur;
    m_nLastActionChannelEndNodeIndex = newWriteCur;
    m_nLastActionChannelPtr = channel;

    // 数据缓冲区操作 - 初始化
    void *bufferStart = nullptr;
    size_t bufferLen = 0;
    NFShmBlockHead *blockHead = GetBlockHead(channel, writeCur, &bufferStart, &bufferLen);
    memset(blockHead, 0x00, sizeof(NFShmBlockHead));

    // 数据缓冲区操作 - 要写入的节点
    {
        blockHead->m_nBufferSize = 0;

        volatile NFShmNodeHead *first_node_head = GetNodeHead(channel, writeCur, nullptr, nullptr);
        first_node_head->m_nFlag = SetFlag(0, MF_START_NODE);
        first_node_head->m_nOperationSeq = oprSeq;

        for (size_t i = GetNextIndex(channel, writeCur, 1); i != newWriteCur; i = GetNextIndex(channel, i, 1))
        {
            volatile NFShmNodeHead *thisNodeHead = GetNodeHead(channel, i, nullptr, nullptr);
            assert((char *) thisNodeHead < (char *) channel + channel->m_nAreaDataOffset);

            // 写数据node出现冲突
            // 写超时会导致this_node_head还是之前版本的数据，并不会被清空。所以不再恢复 operation_seq
            // if (this_node_head->operation_seq) {
            //     return EN_ATBUS_ERR_NODE_BAD_BLOCK_WSEQ_ID;
            // }

            thisNodeHead->m_nFlag = SetFlag(0, NF_WRITEN);
            thisNodeHead->m_nOperationSeq = oprSeq;
        }
    }
    blockHead->m_nBufferSize = len;

    // 数据写入
    // fast_memcpy
    // 数据有回绕
    if (newWriteCur && newWriteCur < writeCur)
    {
        size_t copy_len = len > bufferLen ? bufferLen : len;
        memcpy(bufferStart, buf, copy_len);

        // 回绕nodes
        GetNodeHead(channel, 0, &bufferStart, nullptr);
        memcpy(bufferStart, static_cast<const char*>(buf) + copy_len, len - copy_len);
    } else
    {
        memcpy(bufferStart, buf, len);
    }
    blockHead->m_nFastCheck = FastCheck(buf, len);

    // 设置首node header，数据写完标记
    {
        // 设置屏障，先保证数据区和head区内存已被刷入
        std::atomic_thread_fence(std::memory_order_acq_rel);

        volatile NFShmNodeHead *first_node_head = GetNodeHead(channel, writeCur, nullptr, nullptr);
        first_node_head->m_nFlag = SetFlag(first_node_head->m_nFlag, NF_WRITEN);

        // 设置屏障，保证head内存同步，然后复查操作序号，writen标记延迟同步没关系
        std::atomic_thread_fence(std::memory_order_acquire);
        // 再检查一次，以防memcpy时发生写冲突
        if (oprSeq != first_node_head->m_nOperationSeq)
        {
            ++channel->m_nWriteCheckSequenceFailedCount;
            return NFrame::ERR_CODE_NFBUS_ERR_NODE_BAD_BLOCK_CSEQ_ID;
        }
    }

    return NFrame::ERR_CODE_SVR_OK;
}

int NFIBusConnection::SetWriteTimeout(NFShmChannel *channel, uint64_t ms)
{
    if (nullptr == channel) return NFrame::ERR_CODE_NFBUS_ERR_PARAMS;
    channel->m_nConf.m_nConfSendTimeoutMs = ms;
    return NFrame::ERR_CODE_SVR_OK;
}

uint64_t NFIBusConnection::GetWriteTimeout(NFShmChannel *channel)
{
    if (nullptr == channel) return 0;
    return channel->m_nConf.m_nConfSendTimeoutMs;
}

int NFIBusConnection::SetWriteRetryTimes(NFShmChannel *channel, size_t times)
{
    if (nullptr == channel) return NFrame::ERR_CODE_NFBUS_ERR_PARAMS;
    channel->m_nConf.m_nWriteRetryTimes = times;
    return NFrame::ERR_CODE_SVR_OK;
}

size_t NFIBusConnection::GetWriteRetryTimes(NFShmChannel *channel)
{
    if (nullptr == channel) return 0;
    return channel->m_nConf.m_nWriteRetryTimes;
}

void NFIBusConnection::CopyConf(NFShmConf &dst, const NFShmConf &src)
{
    dst.m_nProtectNodeCount = src.m_nProtectNodeCount;
    dst.m_nProtectMemorySize = src.m_nProtectMemorySize;
    dst.m_nConfSendTimeoutMs = src.m_nConfSendTimeoutMs;
    dst.m_nWriteRetryTimes = src.m_nWriteRetryTimes;
    dst.m_nAtomicRecverIdentify = src.m_nAtomicRecverIdentify;
}

/**
 * @brief 生存默认配置
 * @param conf
 */
void NFIBusConnection::CreateDefaultConf(NFShmChannel *channel)
{
    assert(channel);
    if (nullptr == channel)
    {
        return;
    }

    // 根据Jeffrey Dean大神2007年发布的一个数据，内存4ms大约能复制16MB数据
    // 我们实测的每秒大约能传输数据量大于1GB，所以最大消息长度在4MB以内时4ms都足够传输整个消息，但是超出这个数值最好就再设置长一点
    // 这里我们不考虑CPU调度切换，因为这个情况下无法估计时间，所以就让他超时吧
#if NFBUS_MACRO_MSG_LIMIT <= 4 * 1024 * 1024
    channel->m_nConf.m_nConfSendTimeoutMs = 4;
#else
    channel->m_nConf.m_nConfSendTimeoutMs = (NFBUS_MACRO_MSG_LIMIT / (1024 * 1024)) + 1;
#endif
    channel->m_nConf.m_nWriteRetryTimes = 4; // 默认写序列错误重试4次


    if (!channel->m_nConf.m_nProtectNodeCount && channel->m_nConf.m_nProtectMemorySize)
    {
        channel->m_nConf.m_nProtectNodeCount =
            (channel->m_nConf.m_nProtectMemorySize + NFShmBlock::NODE_DATA_SIZE - 1) / NFShmBlock::NODE_DATA_SIZE;
    }
    else if (!channel->m_nConf.m_nProtectNodeCount)
    {
        // 默认留1/128的数据块用于保护缓冲区
        channel->m_nConf.m_nProtectNodeCount = channel->m_nNodeCount >> 7;

        // protect at most 16KB
        if (channel->m_nConf.m_nProtectNodeCount > NFBUS_MACRO_DATA_MAX_PROTECT_SIZE / NFShmBlock::NODE_DATA_SIZE)
        {
            channel->m_nConf.m_nProtectNodeCount = NFBUS_MACRO_DATA_MAX_PROTECT_SIZE / NFShmBlock::NODE_DATA_SIZE;
        }
    }

    if (channel->m_nConf.m_nProtectNodeCount > channel->m_nNodeCount) channel->m_nConf.m_nProtectNodeCount = channel->m_nNodeCount;

    channel->m_nConf.m_nProtectMemorySize = channel->m_nConf.m_nProtectNodeCount * NFShmBlock::NODE_DATA_SIZE;
}

int NFIBusConnection::CloseShmBuffer()
{
    CHECK_NULL(0, m_pShmRecord);

    assert(m_pShmRecord->m_nReferenceCount > 0);
    if (m_pShmRecord->m_nReferenceCount > 1)
    {
        --m_pShmRecord->m_nReferenceCount;
        return NFrame::ERR_CODE_SVR_OK;
    } else
    {
        m_pShmRecord->m_nReferenceCount = 0;
    }

    NFShmRecordType record = *m_pShmRecord;
    NF_SAFE_DELETE(m_pShmRecord);

#ifdef WIN32
    UnmapViewOfFile(record.m_nBuffer);
    CloseHandle(record.m_nHandle);
#else
    if (record.m_nShmId != 0)
    {
        int res = shmdt(record.m_nBuffer);
        if (-1 == res) {
            return NFrame::ERR_CODE_NFBUS_ERR_SHM_CLOSE_FAILED;
        }
    }
    else if (record.m_nShmFd != 0)
    {
        if (0 != munmap(record.m_nBuffer, record.m_nSize)) {
            shm_unlink(record.m_nShmPath.c_str());
            return NFrame::ERR_CODE_NFBUS_ERR_SHM_CLOSE_FAILED;
        }
        shm_unlink(record.m_nShmPath.c_str());
    }
#endif

    return NFrame::ERR_CODE_SVR_OK;
}

void NFIBusConnection::SetMsgPeerCallback(const BusMsgPeerCallback &cb)
{
    m_busMsgPeerCb = cb;
}

int NFIBusConnection::SendBusConnectMsg(uint64_t busId, uint64_t busLength)
{
    NFDataPackage package;
    package.mModuleId = NF_MODULE_FRAME;
    package.nMsgId = NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_REQ;
    package.nParam1 = busId;
    package.nParam2 = busLength;
    package.nSendBusLinkId = m_bindFlag.mLinkId;

    Send(package, nullptr, 0);
    return 0;
}

int NFIBusConnection::SendBusConnectRspMsg(uint64_t busId, uint64_t busLength)
{
    NFDataPackage package;
    package.mModuleId = NF_MODULE_FRAME;
    package.nMsgId = NFrame::NF_SERVER_TO_SERVER_BUS_CONNECT_RSP;
    package.nParam1 = busId;
    package.nParam2 = busLength;
    package.nSendBusLinkId = m_bindFlag.mLinkId;

    Send(package, nullptr, 0);
    return 0;
}

int NFIBusConnection::SendBusHeartBeatMsg(uint64_t busId, uint64_t busLength)
{
    NFDataPackage package;
    package.mModuleId = NF_MODULE_FRAME;
    package.nMsgId = NFrame::NF_SERVER_TO_SERVER_HEART_BEAT;
    package.nParam1 = busId;
    package.nParam2 = busLength;
    package.nSendBusLinkId = m_bindFlag.mLinkId;

    Send(package, nullptr, 0);
    return 0;
}

int NFIBusConnection::SendBusHeartBeatRspMsg(uint64_t busId, uint64_t busLength)
{
    NFDataPackage package;
    package.mModuleId = NF_MODULE_FRAME;
    package.nMsgId = NFrame::NF_SERVER_TO_SERVER_HEART_BEAT_RSP;
    package.nParam1 = busId;
    package.nParam2 = busLength;
    package.nSendBusLinkId = m_bindFlag.mLinkId;

    Send(package, nullptr, 0);
    return 0;
}

const NFMessageFlag &NFIBusConnection::GetBindFlag() const
{
    return m_bindFlag;
}