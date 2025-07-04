// -------------------------------------------------------------------------
//    @FileName         :    NFCKernelModule.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFCKernelModule
//    @Desc             :
// -------------------------------------------------------------------------

#include "NFCKernelModule.h"

#include <NFComm/NFPluginModule/NFIPluginManager.h>
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFCore/NFMD5.h"
#include "NFComm/NFCore/NFCRC32.h"
#include "NFComm/NFCore/NFCRC16.h"
#include "NFComm/NFCore/NFBase64.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFComm/NFPluginModule/NFIEventModule.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"

#define NF_GUID_POWER 100000
#define NF_EPOCH 1288834974657

#include <NFComm/NFCore/NFServerTime.h>
#include <NFComm/NFPluginModule/NFIMemMngModule.h>

#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFCore/NFFileUtility.h"
#include "NFComm/NFCore/NFCommon.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFComm/NFCore/NFIniReader.h"
#include "NFComm/NFCore/NFStringUtility.h"

#include "NFComm/NFPluginModule/NFCheck.h"

NFCKernelModule::NFCKernelModule(NFIPluginManager *p) : NFIKernelModule(p)
{
    mLastGuidTimeStamp = 0;
    szUniqIDFile = m_pObjPluginManager->GetAppName() + "_" + m_pObjPluginManager->GetBusName() + ".uid";
    uint32_t busId = m_pObjPluginManager->GetAppID();
    m_iWorldId = NFServerIDUtil::GetWorldID(busId);
    m_iZoneId = NFServerIDUtil::GetZoneID(busId);
    m_iAdaptiveTime = (int) NFGetSecondTime() - UNIQUE_ID_START_TIME;
    m_ushSequence = 0;


    m_ucCheckSeq = UpdateCheckSeq(szUniqIDFile);

    /*
        63-60 4b  worldid
        59-48 12b zoneid
        47-44 4b  checkseq
        43-32 12b seq
        31-0  32b time
    */
    m_ullMask = (((m_iWorldId << 60) & WORLDID_MASK) /*63-60 世界ID */
                 | ((m_iZoneId << 48) & ZONEID_MASK) /*59-48 区服ID*/
                 | ((m_ucCheckSeq << 44) & CHECK_SEQ_MASK)); /*47-44 校正序号 */

    NFLogInfo(NF_LOG_DEFAULT, 0, "m_ullMask:{}", m_ullMask);
}

NFCKernelModule::~NFCKernelModule()
{
}

uint8_t NFCKernelModule::UpdateCheckSeq(const std::string &szCheckSeqFile)
{
    uint8_t cCheckSeq = 0;
    bool iFileExists = NFFileUtility::IsFileExist(szCheckSeqFile);
    if (iFileExists)
    {
        std::string context;
        NFFileUtility::ReadFileContent(szCheckSeqFile, context);
        cCheckSeq = NFCommon::strto<uint8_t>(context);
        cCheckSeq += 1;
        if (cCheckSeq >= 16)
        {
            cCheckSeq = 0;
        }
    }

    NFFileUtility::WriteFile(szCheckSeqFile, &cCheckSeq, sizeof(cCheckSeq));

    return cCheckSeq;
}

bool NFCKernelModule::Execute()
{
    return true;
}

bool NFCKernelModule::BeforeShut()
{
    return true;
}

bool NFCKernelModule::Init()
{
    auto &allServerType = NFGlobalSystem::Instance()->GetAllServerType();
    for (auto iter = allServerType.begin(); iter != allServerType.end(); iter++)
    {
        NF_SERVER_TYPE serverType = (NF_SERVER_TYPE) *iter;
        NFrame::NFEventNoneData event;
        event.set_param1(NFServerTime::Instance()->GetSecOffSet());
        Subscribe(serverType, NFrame::NF_EVENT_SERVER_SET_TIME_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, __FUNCTION__);

        FindModule<NFIMessageModule>()->AddMessageCallBack((NF_SERVER_TYPE) serverType, NFrame::NF_STS_KILL_ALL_SERVER_NTF, this,
                                                           &NFCKernelModule::OnKillServerProcess);
    }

    return true;
}

bool NFCKernelModule::Shut()
{
    return true;
}

bool NFCKernelModule::Finalize()
{
    return true;
}

int NFCKernelModule::Next(int iNowSec)
{
    uint64_t iNowTime = iNowSec - UNIQUE_ID_START_TIME;

    if (iNowTime <= m_iAdaptiveTime)
    {
        m_ushSequence++;

        if (m_ushSequence >= ONE_SECOND_SEQ_NUM)
        {
            m_iAdaptiveTime++;
            m_ushSequence = 0;
        }
    }
    else
    {
        m_iAdaptiveTime = iNowTime;
        m_ushSequence = 0;
    }

    return 0;
}

int NFCKernelModule::Next53(int iNowSec)
{
    uint64_t iNowTime = iNowSec - UNIQUE_ID_START_TIME;

    if (iNowTime <= m_iAdaptiveTime)
    {
        m_iAdaptiveTime++;
    }
    else
    {
        m_iAdaptiveTime = iNowTime;
    }

    return 0;
}

/*
    63-60 4b  worldid
    59-48 12b zoneid
    47-44 4b  checkseq
    43-32 12b seq
    31-0  32b time
*/
uint64_t NFCKernelModule::Get64UUID()
{
    Next(NFGetSecondTime());

    /*    m_ullMask = (((m_iWorldId << 60) & WORLDID_MASK)     *//*63-60 世界ID *//*
                 | ((m_iZoneId << 48) & ZONEID_MASK)      *//*59-48 区服ID*//*
                 | ((m_ucCheckSeq << 44) & CHECK_SEQ_MASK)); */ /*47-44 校正序号 */

    uint64_t ullUniqueID = (m_ullMask
                            | ((m_ushSequence << 32) & ONE_SECOND_SEQ_MASK) /*独立序号*/
                            | (m_iAdaptiveTime & ADAPTIVE_TIME_MASK)); /*系统时间*/


    NFLogTrace(NF_LOG_DEFAULT, 0, "gen uuid::{}", ullUniqueID);

    uint64_t worldId = (ullUniqueID & WORLDID_MASK) >> 60;
    uint64_t zoneId = (ullUniqueID & ZONEID_MASK) >> 48;
    uint64_t checkSeq = (ullUniqueID & CHECK_SEQ_MASK) >> 44;
    uint64_t seq = (ullUniqueID & ONE_SECOND_SEQ_MASK) >> 32;
    uint64_t adaptiveTime = (ullUniqueID & ADAPTIVE_TIME_MASK);

    CHECK_EXPR_MSG(worldId == m_iWorldId, "Get64UUID error, worldId:{} == m_iWorldType:{}", worldId, m_iWorldId);
    CHECK_EXPR_MSG(zoneId == m_iZoneId, "Get64UUID error, zoneId:{} == m_iZoneId:{}", zoneId, m_iZoneId);
    CHECK_EXPR_MSG(checkSeq == m_ucCheckSeq, "Get64UUID error, checkSeq:{} == m_ucCheckSeq:{}", checkSeq, m_ucCheckSeq);
    CHECK_EXPR_MSG(seq == m_ushSequence, "Get64UUID error, seq:{} == m_ushSequence:{}", seq, m_ushSequence);
    CHECK_EXPR_MSG(adaptiveTime == m_iAdaptiveTime, "Get64UUID error, adaptiveTime:{} == m_iAdaptiveTime:{}", adaptiveTime, m_iAdaptiveTime);

    return ullUniqueID;
}

uint64_t NFCKernelModule::Get53UUID()
{
    Next53(NFGetSecondTime());
    /*
        51-48 4b  worldid
        47-36 12b zoneid
        35-32 4b  checkseq
        31-0  32b time
    */
    uint64_t ullUniqueID = (((m_iWorldId << 48) & WORLDID_53_MASK)
                            | ((m_iZoneId << 36) & ZONEID_53_MASK)
                            | ((m_ucCheckSeq << 32) & CHECK_SEQ_53_MASK)
                            | (m_iAdaptiveTime & ADAPTIVE_TIME_MASK));


    NFLogTrace(NF_LOG_DEFAULT, 0, "gen uuid::{}", ullUniqueID);

    uint64_t worldId = (ullUniqueID & WORLDID_53_MASK) >> 48;
    uint64_t zoneId = (ullUniqueID & ZONEID_53_MASK) >> 36;
    uint64_t checkSeq = (ullUniqueID & CHECK_SEQ_53_MASK) >> 32;
    uint64_t adaptiveTime = (ullUniqueID & ADAPTIVE_TIME_MASK);

    CHECK_EXPR_MSG(worldId == m_iWorldId, "Get53UUID error, worldId:{} == m_iWorldId:{}", worldId, m_iWorldId);
    CHECK_EXPR_MSG(zoneId == m_iZoneId, "Get53UUID error, zoneId:{} == m_iZoneId:{}", zoneId, m_iZoneId);
    CHECK_EXPR_MSG(checkSeq == m_ucCheckSeq, "Get53UUID error, checkSeq:{} == m_ucCheckSeq:{}", checkSeq, m_ucCheckSeq);
    CHECK_EXPR_MSG(adaptiveTime == m_iAdaptiveTime, "Get53UUID error, adaptiveTime:{} == m_iAdaptiveTime:{}", adaptiveTime, m_iAdaptiveTime);

    return ullUniqueID;
}

uint64_t NFCKernelModule::Get32UUID()
{
    Next(NFGetSecondTime());
    uint64_t ullUniqueID = (((uint64_t) m_iAdaptiveTime & ADAPTIVE_TIME_MASK));

    NFLogTrace(NF_LOG_DEFAULT, 0, "gen 32 uuid::{}", ullUniqueID);

    return ullUniqueID;
}

uint64_t NFCKernelModule::GetUUID()
{
    return Get64UUID();
}

std::string NFCKernelModule::GetMD5(const std::string &str)
{
    return NFMD5::md5str(str);
}

uint32_t NFCKernelModule::GetCRC32(const std::string &s)
{
    return NFCRC32::Sum(s);
}

uint16_t NFCKernelModule::GetCRC16(const std::string &s)
{
    return NFCRC16::Sum(s);
}

std::string NFCKernelModule::Base64Encode(const std::string &s)
{
    return NFBase64::Encode(s);
}

std::string NFCKernelModule::Base64Decode(const std::string &s)
{
    return NFBase64::Decode(s);
}

int NFCKernelModule::OnTimer(uint32_t nTimerID)
{
    return 0;
}

int NFCKernelModule::OnKillServerProcess(uint64_t unLinkId, NFDataPackage &packet)
{
    NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
    CHECK_NULL(0, pConfig);

    NFrame::NFEventNoneData xMsg;
    FindModule<NFIEventModule>()->FireExecute((NF_SERVER_TYPE)pConfig->ServerType, NFrame::NF_EVENT_SERVER_DEAD_EVENT, NFrame::NF_EVENT_SERVER_TYPE, 0, xMsg);
    return 0;
}

int NFCKernelModule::OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage)
{
    if (nEventID == NFrame::NF_EVENT_SERVER_SET_TIME_EVENT && bySrcType == NFrame::NF_EVENT_SERVER_TYPE)
    {
        const NFrame::NFEventNoneData *pNone = dynamic_cast<const NFrame::NFEventNoneData *>(pMessage);
        if (pNone)
        {
            if (pNone->param1() > 0)
            {
                NFServerTime::Instance()->GmSetTime(NFServerTime::Instance()->UnixSec() + pNone->param1());
                auto pModule = FindModule<NFIMemMngModule>();
                if (pModule)
                {
                    pModule->SetSecOffSet(pNone->param1());
                }
            }
        }
    }

    return 0;
}
