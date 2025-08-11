// -------------------------------------------------------------------------
//    @FileName         :    NFShmTransMng.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmTransMng.cpp
//    @Desc             :    共享内存事务管理器实现文件，提供事务对象的生命周期管理。
//                          该文件实现了事务管理器的核心功能，包括事务对象的创建和销毁、
//                          事务状态管理、事务超时检测、事务处理调度、事务生命周期管理。
//                          主要功能包括事务对象管理、事务状态检查、事务超时处理、
//                          事务处理调度、服务器停止检查。
//                          设计特点包括基于共享内存支持跨进程、批量处理机制、
//                          超时自动处理、状态一致性检查、内存自动回收
//
// -------------------------------------------------------------------------

#include "NFShmTransMng.h"
#include "NFComm/NFObjCommon/NFTransBase.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @brief 构造函数
 * 
 * 初始化事务管理器，根据共享内存模式选择初始化方式
 */
NFShmTransMng::NFShmTransMng()
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

/**
 * @brief 析构函数
 * 
 * 清理事务管理器资源
 */
NFShmTransMng::~NFShmTransMng()
{
}

/**
 * @brief 创建初始化
 * 
 * 在创建时进行初始化
 * 
 * @return 0 成功，其他值表示失败
 */
int NFShmTransMng::CreateInit()
{
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 在恢复时进行初始化
 * 
 * @return 0 成功，其他值表示失败
 */
int NFShmTransMng::ResumeInit()
{
    return 0;
}

/**
 * @brief 创建事务对象
 * 
 * 创建指定类型的事务对象并添加到管理列表
 * 
 * @param bTransObjType 事务对象类型
 * @return 创建的事务对象指针，失败返回nullptr
 */
NFTransBase* NFShmTransMng::CreateTrans(uint32_t bTransObjType)
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

    // 添加到事务对象ID列表
    m_aiTransObjIdList.push_back(pTransBase->GetGlobalId());
    NFLogDebug(NF_LOG_DEFAULT, 0, "Create Trans TotalNum:{} Info:{} Pointer:{}", m_aiTransObjIdList.size(), pTransBase->DebugString(), static_cast<void*>(pTransBase));

    return pTransBase;
}

/**
 * @brief 获取事务对象
 * 
 * 根据事务ID获取事务对象
 * 
 * @param ullTransId 事务ID
 * @return 事务对象指针，失败返回nullptr
 */
NFTransBase* NFShmTransMng::GetTransBase(uint64_t ullTransId) const
{
    CHECK_EXPR(ullTransId < INT_MAX, NULL, "TrandID Max:{} IntMax:{}", ullTransId, INT_MAX);
    return NFTransBase::GetObjByGlobalId(ullTransId, true);
}

/**
 * @brief 创建事务对象
 * 
 * 通过内存管理模块创建指定类型的事务对象
 * 
 * @param bTransObjType 事务对象类型
 * @return 创建的事务对象指针
 */
NFTransBase* NFShmTransMng::CreateTransObj(uint32_t bTransObjType) const
{
    return dynamic_cast<NFTransBase*>(FindModule<NFIMemMngModule>()->CreateObj(static_cast<int>(bTransObjType)));
}

/**
 * @brief 根据索引获取事务对象
 * 
 * 根据索引从事务对象ID列表中获取事务对象
 * 
 * @param iIndex 索引位置
 * @return 事务对象指针，失败返回nullptr
 */
NFTransBase* NFShmTransMng::GetTransObj(int iIndex) const
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
 * 遍历所有事务对象，检查是否都已完成
 * 
 * @param bAllTransFinished 输出参数，true表示所有事务都已完成
 * @return 0 成功，其他值表示失败
 */
int NFShmTransMng::CheckAllTransFinished(bool& bAllTransFinished) const
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
 * @brief 处理事务心跳
 * 
 * 批量处理事务对象，包括超时检查、状态更新和内存回收
 * 
 * @param dwCurRunIndex 当前运行索引
 * @param bIsTickAll 是否处理所有事务
 * @return 0 成功，其他值表示失败
 */
int NFShmTransMng::DoTick(uint32_t dwCurRunIndex, bool bIsTickAll)
{
    int iPerTickNumThisTime = m_iNumPerTick;

    // 批量处理事务对象
    while (m_iTickedNum < iPerTickNumThisTime || bIsTickAll)
    {
        // 检查是否已处理完所有事务
        if (m_iLastTickIndex >= static_cast<int>(m_aiTransObjIdList.size()))
        {
            m_iLastTickIndex = 0;
            m_bIsTickFinished = true;
            break;
        }

        NFTransBase* pTransBase = GetTransObj(m_iLastTickIndex);

        if (pTransBase)
        {
            // 验证事务对象ID一致性
            if (pTransBase->GetGlobalId() == m_aiTransObjIdList[m_iLastTickIndex])
            {
                // 检查事务是否超时
                if (pTransBase->IsTimeOut())
                {
                    pTransBase->SetFinished(NFrame::ERR_CODE_SVR_SYSTEM_TIMEOUT); //time out
                }

                // 检查事务是否可以释放
                if (pTransBase->IsCanRelease())
                {
                    NFLogDebug(NF_LOG_DEFAULT, 0, "Free Trans END Index:{} Pointer:{} Info:{}", m_iLastTickIndex, static_cast<void*>(pTransBase), pTransBase->DebugString());
                    FindModule<NFIMemMngModule>()->DestroyObj(pTransBase);
                    // 从列表中移除已释放的事务
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

/**
 * @brief 检查是否可以停止服务器
 * 
 * 检查所有事务是否都已完成，如果都已完成则可以安全停止服务器
 * 
 * @return 0 可以停止服务器，-1 还有未完成的事务
 */
int NFShmTransMng::CheckStopServer()
{
    bool bAllTransFinished;
    CheckAllTransFinished(bAllTransFinished);
    return bAllTransFinished ? 0 : -1;
}

/**
 * @brief 停止服务器
 * 
 * 执行服务器停止操作
 * 
 * @return 0 成功，其他值表示失败
 */
int NFShmTransMng::StopServer()
{
    return 0;
}
