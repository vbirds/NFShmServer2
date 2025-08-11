// -------------------------------------------------------------------------
//    @FileName         :    NFMemGlobalId.cpp
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFPluginModule
//    @Desc             :    内存全局ID实现文件，负责管理内存中的全局唯一ID，包括全局ID的分配和回收、
//                          ID与对象的映射管理、轮次管理和文件持久化、统计信息收集、版本兼容性检查。
//                          该文件实现了NFShmXFrame框架的内存全局ID管理器类，提供全局ID分配和释放、
//                          对象ID映射表管理、轮次统计和文件更新、性能监控和统计、内存池管理等功能。
//                          主要功能包括队列管理高效分配、轮次机制防止ID溢出、文件持久化支持恢复、
//                          统计信息收集
//
// -------------------------------------------------------------------------

#include "NFMemGlobalId.h"

#include <NFComm/NFCore/NFCommon.h>
#include <NFComm/NFCore/NFFileUtility.h>
#include <NFComm/NFCore/NFServerTime.h>

#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFCore/NFDateTime.hpp"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"

NFMemGlobalId::NFMemGlobalId() : m_iThisRoundCountMax(0), m_iThisRoundCount(0),
                                 m_iGlobalIdAppendNum(0)
{
    NF_FORMAT_EXPR(m_szFileName, "{}_globalid", NFServerIDUtil::GetBusNameFromBusID(NFGlobalSystem::Instance()->GetGlobalPluginManager()->GetAppID()));
    //校验
    COMPILE_TIME_ASSERT(static_cast<int64_t>(MAX_GLOBALID_NUM * GLOBALID_LOOP_BACK) < INT_MAX);

    if (NFShmMgr::Instance()->GetCreateMode() == EN_OBJ_MODE_INIT)
    {
        CreateInit();
    }
    else
    {
        ResumeInit();
    }
}

NFMemGlobalId::~NFMemGlobalId()
{
}

int NFMemGlobalId::CreateInit()
{
    m_iUseCount = 0;
    //初始化的时候可以保证转完一圈不重复
    m_iThisRoundCountMax = MAX_GLOBALID_NUM;

    //memset(m_stIDTable, -1, sizeof(m_stIDTable));
    for (int i = 1; i < static_cast<int>(ARRAYSIZE(m_stIdTable)); i++)
    {
        m_stIdTable[i].m_iId = -1;
        m_stIdTable[i].m_iIndex = -1;
        m_stIdTable[i].m_iType = -1;
        m_stIdTable[i].m_pObjPtr = nullptr;
    }

    for (int i = 0; i < MAX_GLOBALID_NUM; i++)
    {
        m_stQueue.push(i);
    }

    if (!ResumeFileUpdateData())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "ResumeFileUpdateData error in CGlobalID");
        return -1;
    }

    m_tRoundBegin = time(nullptr);

    m_iSecOffSet = 0;
    return 0;
}

int NFMemGlobalId::ResumeInit()
{
    return 0;
}

/**
 * @brief 分配全局ID
 * 
 * 该函数用于分配一个新的全局ID。
 * 分配过程包括：
 * 1. 验证对象指针的有效性
 * 2. 检查当前轮次是否需要更新文件
 * 3. 如果队列为空，则记录错误并返回
 * 4. 从队列中取出下一个可用的ID
 * 5. 验证ID的有效性
 * 6. 计算ID在表中的索引位置
 * 7. 验证索引位置的有效性
 * 8. 更新ID表中的相关数据
 * 9. 增加使用计数
 * 10. 返回分配的ID
 * 
 * @param iType 对象类型
 * @param iIndex 对象索引
 * @param pObj 对象指针
 * @return 分配的全局ID，失败返回-1
 */
int NFMemGlobalId::GetGlobalId(int iType, int iIndex, const NFObject* pObj)
{
    CHECK_NULL(0, pObj);
    if (!CalcRoundUpdateFile())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "CalcRoundUpdateFile error in CGlobalID");
        return -1;
    }

    if (m_stQueue.empty())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "Global id work out! m_stQueue empty");
        return -1;
    }

    int iQueueIndex = m_stQueue.front();
    m_stQueue.pop();
    if (iQueueIndex < 0 || iQueueIndex >= MAX_GLOBALID_NUM)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "iQueueIndex error, iQueueIndex:{}", iQueueIndex);
        return -1;
    }

    NFMemIdIndex* pIdIndex = &m_stIdTable[iQueueIndex];
    pIdIndex->m_iId = iQueueIndex + m_iGlobalIdAppendNum;
    pIdIndex->m_iType = iType;
    pIdIndex->m_iIndex = iIndex;
    pIdIndex->m_pObjPtr = pObj;
    m_iUseCount++;
    return pIdIndex->m_iId;
}

/**
 * @brief 释放全局ID
 * 
 * 该函数用于释放已分配的全局ID，将其重新放回队列中供后续使用。
 * 释放过程包括：
 * 1. 验证ID的有效性
 * 2. 计算ID在表中的索引位置
 * 3. 验证索引位置的有效性
 * 4. 将索引重新放回队列
 * 5. 清空ID表中的相关数据
 * 
 * @param iId 要释放的全局ID
 * @return 释放结果，0表示成功，-1表示失败
 */
int NFMemGlobalId::ReleaseId(int iId)
{
    // 检查ID的有效性
    if (iId < 0)
    {
        return -1;
    }

    // 计算ID在表中的索引位置
    int iIdIndex = iId & MAX_GLOBALID_NUM_MASK;
    if (iIdIndex < 0)
    {
        return -1;
    }

    // 获取ID索引对象并验证ID匹配
    NFMemIdIndex* pIdIndex = &m_stIdTable[iIdIndex];
    if (pIdIndex->m_iId != iId)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "Release Index ID:{} ID:{}", pIdIndex->m_iId, iId);
        return -1;
    }

    // 将索引重新放回队列
    m_stQueue.push(iIdIndex);

    // 清空ID表中的相关数据
    pIdIndex->m_iId = -1;
    pIdIndex->m_iIndex = -1;
    pIdIndex->m_iType = -1;
    pIdIndex->m_pObjPtr = nullptr;
    m_iUseCount--;
    return 0;
}

/**
 * @brief 根据全局ID获取对象
 * 
 * 该函数根据全局ID查找并返回对应的对象指针。
 * 查找过程包括：
 * 1. 验证ID的有效性
 * 2. 计算ID在表中的索引位置
 * 3. 验证索引位置的有效性
 * 4. 验证ID匹配
 * 5. 返回对象指针
 * 
 * @param iId 全局ID
 * @return 对象指针，失败返回nullptr
 */
NFObject* NFMemGlobalId::GetObj(int iId)
{
    if (iId < 0) return nullptr;

    int iIdIndex = iId & MAX_GLOBALID_NUM_MASK;

    if (likely(iIdIndex >= 0 && iIdIndex < MAX_GLOBALID_NUM))
    {
        if (likely(m_stIdTable[iIdIndex].m_iId == iId))
        {
            NFObject* pObj = m_stIdTable[iIdIndex].m_pObjPtr;

#ifdef NF_DEBUG_MODE
            NFObject* pObjGetObjFromTypeIndex = FindModule<NFIMemMngModule>()->GetObjByObjId(m_stIdTable[iIdIndex].m_iType, m_stIdTable[iIdIndex].m_iIndex);
            NF_ASSERT(pObjGetObjFromTypeIndex == pObj);
#endif
            //理论上还是存在这种可能性，只要服务器运行时间足够久
            if (pObj && pObj->GetGlobalId() != iId)
            {
                /*发生这种情况说明上层逻辑不对，对象不存在已经很久了，为什么还去Get呢.*/
                NFLogError(NF_LOG_DEFAULT, 0, "id {} in({},{},{})", pObj->GetGlobalId(), m_stIdTable[iIdIndex].m_iId, m_stIdTable[iIdIndex].m_iIndex,
                           m_stIdTable[iIdIndex].m_iType);
                debug_assert();
                return nullptr;
            }

            return pObj;
        }
        return nullptr;
    }

    return nullptr;
}

NFObject* NFMemGlobalId::ResumeObject(void* pBuffer)
{
    NFObject* pTmp = nullptr;
    pTmp = new(pBuffer) NFMemGlobalId();
    return pTmp;
}

NFObject* NFMemGlobalId::CreateObject()
{
    NFMemGlobalId* pTmp = nullptr;
    void* pVoid = NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->AllocMemForObject(EOT_GLOBAL_ID);
    if (!pVoid)
    {
        return nullptr;
    }

    NFShmMgr::Instance()->m_iType = EOT_GLOBAL_ID;
    pTmp = new(pVoid) NFMemGlobalId();
    NFShmMgr::Instance()->m_iType = INVALID_ID;
    return pTmp;
}

void NFMemGlobalId::DestroyObject(NFObject* pObj)
{
    auto pTmp = dynamic_cast<NFMemGlobalId*>(pObj);
    pTmp->NFMemGlobalId::~NFMemGlobalId();
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->FreeMemForObject(EOT_GLOBAL_ID, pTmp);
}

void* NFMemGlobalId::operator new(size_t, void* pBuffer) throw()
{
    return pBuffer;
}

int NFMemGlobalId::AddSecond(int iSecond)
{
    m_iSecOffSet += iSecond;
    return 0;
}

int NFMemGlobalId::SetSecOffSet(int iSecOffSet)
{
    m_iSecOffSet = iSecOffSet;
    return 0;
}

int NFMemGlobalId::GetSecOffSet() const
{
    return m_iSecOffSet;
}

int NFMemGlobalId::RegisterClassToObjSeg(int bType, int iObjSize, int iObjCount, const std::string& className, bool useHash,
                                         bool singleton)
{
    NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->RegisterClassToObjSeg(bType, iObjSize, iObjCount, ResumeObject,
                                                                                                               CreateObject,
                                                                                                               DestroyObject, -1, className, useHash,
                                                                                                               singleton);
    return 0;
}

bool NFMemGlobalId::ResumeFileUpdateData()
{
    //该程序的作用是如果Round_time存在，读取其中的数字，如果不存在，则创建之
    std::string content;
    if (NFFileUtility::IsFileExist(m_szFileName.ToString()))
    {
        NFFileUtility::ReadFileContent(m_szFileName.ToString(), content);
    }
    else
    {
        content = "0";
        NFFileUtility::WriteFile(m_szFileName.ToString(), content);
    }

    m_iRoundMultiple = NFCommon::strto<int>(content) + 1; //加一保证ID和上次不重复
    m_iRoundMultiple = m_iRoundMultiple % GLOBALID_LOOP_BACK;
    m_iGlobalIdAppendNum = m_iRoundMultiple * MAX_GLOBALID_NUM;

    if (!WriteRound())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "WriteRound round_time error ");
        return false;
    }

    return true;
}

bool NFMemGlobalId::CalcRoundUpdateFile()
{
    //该函数实现的功能是计算循环次数是不是达到m_iThisRoundCount，如果达到则RoundTimes+1并写到文件中
    if (m_iThisRoundCount >= m_iThisRoundCountMax)
    {
        m_iThisRoundCountMax = MAX_GLOBALID_NUM - m_iUseCount;
        m_iThisRoundCount = 0;
        m_iRoundMultiple++;
        m_iRoundMultiple = m_iRoundMultiple % GLOBALID_LOOP_BACK;
        m_iGlobalIdAppendNum = m_iRoundMultiple * MAX_GLOBALID_NUM;
        m_tRoundBegin = time(nullptr);

        if (!WriteRound())
        {
            NFLogError(NF_LOG_DEFAULT, 0, "WriteRound round_time error ");
            return false;
        }
    }

    m_iThisRoundCount++;
    return true;
}

bool NFMemGlobalId::WriteRound() const
{
    //该函数的作用是把循环次数RoundTimes写到文件pszFileName中
    std::string content = NFCommon::tostr(m_iRoundMultiple);
    return NFFileUtility::WriteFile(m_szFileName.ToString(), content);
}

int NFMemGlobalId::GetUseCount() const
{
    return m_iUseCount;
}

std::string NFMemGlobalId::GetOperatingStatistic() const
{
    std::string szStatistic;
    NF_FORMAT_EXPR(szStatistic, "GlobalID: total {}, used {}, round {}, round begin {}",
                   MAX_GLOBALID_NUM, m_iUseCount, m_iRoundMultiple, NFDateTime(m_tRoundBegin).GetDbTimeString());
    return szStatistic;
}
