// -------------------------------------------------------------------------
//    @FileName         :    NFMemTransMng.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMemTransMng.cpp
//    @Desc             :    内存事务管理器实现文件，提供事务对象的统一管理和生命周期控制。
//                          该文件实现了NFShmXFrame框架的内存事务管理器，负责事务对象的创建和销毁、
//                          事务生命周期管理、事务状态监控、服务器停止检查等。
//                          主要功能包括事务对象池管理、事务执行调度、事务完成检查、
//                          服务器状态管理、事务对象查找与回收
//
// -------------------------------------------------------------------------

#include "NFMemTransMng.h"
#include "NFComm/NFObjCommon/NFTransBase.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"

NFMemTransMng::NFMemTransMng()
{
    if (NFShmMgr::Instance()->GetCreateMode() == EN_OBJ_MODE_INIT)
    {
        CreateInit();
    }
    else
    {
        ResumeInit();
    }
}

NFMemTransMng::~NFMemTransMng()
{
}

int NFMemTransMng::CreateInit()
{
    return 0;
}

int NFMemTransMng::ResumeInit()
{
    return 0;
}

/**
 * @brief 创建事务对象
 * 
 * 该函数用于创建指定类型的事务对象。
 * 创建过程包括：
 * 1. 检查事务对象列表是否已满
 * 2. 创建事务对象
 * 3. 将事务对象的全局ID添加到列表中
 * 4. 记录创建日志
 * 
 * @param bTransObjType 事务对象类型
 * @return 事务对象指针，失败返回nullptr
 */
NFTransBase* NFMemTransMng::CreateTrans(uint32_t bTransObjType)
{
    // 检查事务对象列表是否已满
    if (m_aiTransObjIdList.size() >= m_aiTransObjIdList.max_size())
    {
        NFLogFatal(NF_LOG_DEFAULT, 0, "TransMng is FULL err, objType:{}, TotalNum:{}, MaxNum:{}", bTransObjType, m_aiTransObjIdList.size(), m_aiTransObjIdList.max_size());
        return nullptr;
    }

    // 创建事务对象
    NFTransBase* pTransBase = CreateTransObj(bTransObjType);
    CHECK_EXPR(pTransBase, NULL, "CreateTransObj Failed, TransObjType:{}", bTransObjType);

    // 将事务对象的全局ID添加到列表中
    m_aiTransObjIdList.push_back(pTransBase->GetGlobalId());
    NFLogDebug(NF_LOG_DEFAULT, 0, "Create Trans TotalNum:{} Info:{} Pointer:{}", m_aiTransObjIdList.size(), pTransBase->DebugString(), static_cast<void*>(pTransBase));

    return pTransBase;
}

/**
 * @brief 根据事务ID获取事务对象
 * 
 * 该函数根据事务ID获取对应的事务对象。
 * 
 * @param ullTransId 事务ID
 * @return 事务对象指针，失败返回nullptr
 */
NFTransBase* NFMemTransMng::GetTransBase(uint64_t ullTransId) const
{
    CHECK_EXPR(ullTransId < INT_MAX, NULL, "TrandID Max:{} IntMax:{}", ullTransId, INT_MAX);
    return NFTransBase::GetObjByGlobalId(ullTransId, true);
}

/**
 * @brief 创建事务对象
 * 
 * 该函数通过内存管理模块创建指定类型的事务对象。
 * 
 * @param bTransObjType 事务对象类型
 * @return 事务对象指针
 */
NFTransBase* NFMemTransMng::CreateTransObj(uint32_t bTransObjType) const
{
    return dynamic_cast<NFTransBase*>(FindModule<NFIMemMngModule>()->CreateObj(static_cast<int>(bTransObjType)));
}

/**
 * @brief 根据索引获取事务对象
 * 
 * 该函数根据索引从事务对象列表中获取对应的事务对象。
 * 
 * @param iIndex 事务对象索引
 * @return 事务对象指针，失败返回nullptr
 */
NFTransBase* NFMemTransMng::GetTransObj(int iIndex) const
{
    NFTransBase* pTransBase = nullptr;
    if (iIndex >= 0 && iIndex < static_cast<int>(m_aiTransObjIdList.size()))
    {
        pTransBase = GetTransBase(m_aiTransObjIdList[iIndex]);
        if (nullptr == pTransBase)
        {
            LOG_ERR(0, -1, "This Trans  %d Is Invalid", m_aiTransObjIdList[iIndex]);
        }
    }

    return pTransBase;
}

/**
 * @brief 检查所有事务是否完成
 * 
 * 该函数检查所有事务的状态，判断是否全部完成。
 * 检查过程包括：
 * 1. 遍历所有事务对象ID
 * 2. 获取每个事务对象
 * 3. 记录事务状态信息
 * 4. 更新完成状态标志
 * 
 * @param bAllTransFinished 输出参数，表示是否所有事务都已完成
 * @return 检查结果，0表示成功
 */
int NFMemTransMng::CheckAllTransFinished(bool& bAllTransFinished) const
{
    bAllTransFinished = true;
    int iTransIndex = 0;
    while (iTransIndex < static_cast<int>(m_aiTransObjIdList.size()))
    {
        bAllTransFinished = false;
        NFTransBase* pTransBase = GetTransBase(m_aiTransObjIdList[iTransIndex]);
        if (pTransBase)
        {
            LOG_INFO(0, "Exist: TransID %d %d, Index %d, Class Type %d",
                     pTransBase->GetGlobalId(), m_aiTransObjIdList[iTransIndex], iTransIndex, pTransBase->GetClassType());
        }
        else
        {
            LOG_INFO(0, "NonExist: TransID %d Index %d.", m_aiTransObjIdList[iTransIndex], iTransIndex);
        }
        iTransIndex++;
    }
    return 0;
}

/**
 * @brief 执行事务心跳处理
 * 
 * 该函数是事务管理器的核心处理函数，负责：
 * 1. 按顺序处理事务对象
 * 2. 检查事务超时状态
 * 3. 释放已完成的事务
 * 4. 处理未完成的事务
 * 5. 更新处理索引
 * 
 * @param dwCurRunIndex 当前运行索引
 * @param bIsTickAll 是否处理所有事务
 * @return 处理结果
 */
int NFMemTransMng::DoTick(uint32_t dwCurRunIndex, bool bIsTickAll)
{
    int iPerTickNumThisTime = m_iNumPerTick;

    while (m_iTickedNum < iPerTickNumThisTime || bIsTickAll)
    {
        // 检查是否已处理完所有事务
        if (m_iLastTickIndex >= static_cast<int>(m_aiTransObjIdList.size()))
        {
            m_iLastTickIndex = 0;
            m_bIsTickFinished = true;
            break;
        }

        // 获取当前索引的事务对象
        NFTransBase* pTransBase = GetTransObj(m_iLastTickIndex);

        if (pTransBase)
        {
            if (pTransBase->GetGlobalId() == m_aiTransObjIdList[m_iLastTickIndex])
            {
                // 检查事务超时
                if (pTransBase->IsTimeOut())
                {
                    pTransBase->SetFinished(NFrame::ERR_CODE_SVR_SYSTEM_TIMEOUT); //time out
                }

                // 检查事务是否可以释放
                if (pTransBase->IsCanRelease())
                {
                    NFLogDebug(NF_LOG_DEFAULT, 0, "Free Trans END Index:{} Pointer:{} Info:{}", m_iLastTickIndex, static_cast<void*>(pTransBase), pTransBase->DebugString());
                    FindModule<NFIMemMngModule>()->DestroyObj(pTransBase);
                    m_aiTransObjIdList[m_iLastTickIndex] = m_aiTransObjIdList.back();
                    m_aiTransObjIdList.back() = 0;
                    m_aiTransObjIdList.pop_back();
                }
                else
                {
                    // 处理未完成的事务
                    if (!pTransBase->IsFinished())
                    {
                        pTransBase->ProcessTick();
                    }
                }
            }
            else
            {
                NFLogFatal(NF_LOG_DEFAULT, 0, "Trans Index Err ObjGlobalID:{} != IndexGlobalID:{} ObjPointer:{} Info:{}", pTransBase->GetGlobalId(), m_aiTransObjIdList[m_iLastTickIndex], static_cast<void*>(pTransBase), pTransBase->DebugString());
            }

            m_iTickedNum++;
        }

        m_iLastTickIndex++;
    }

    return 0;
}

int NFMemTransMng::CheckStopServer()
{
    bool bAllTransFinished;
    CheckAllTransFinished(bAllTransFinished);
    return bAllTransFinished ? 0 : -1;
}

int NFMemTransMng::StopServer()
{
    return 0;
}
