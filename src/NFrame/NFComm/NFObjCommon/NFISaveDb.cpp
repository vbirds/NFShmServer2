// -------------------------------------------------------------------------
//    @FileName         :    NFISaveDb.cpp
//    @Author           :    gaoyi
//    @Date             :    23-11-17
//    @Email			:    445267987@qq.com
//    @Module           :    NFISaveDb
//
// -------------------------------------------------------------------------

/**
 * @file NFISaveDb.cpp
 * @brief 数据库保存接口实现文件
 * @details 实现了NFrame框架中数据库保存接口NFISaveDb的基础功能，包括保存时间管理、
 *          数据变更检测、保存状态控制等。为数据持久化提供了标准的接口实现，
 *          支持批量保存和增量保存策略。
 * 
 * @author gaoyi
 * @date 23-11-17
 * @email 445267987@qq.com
 * 
 * @note 实现的主要功能：
 *       - 数据库保存时间戳管理
 *       - 保存状态检查和控制
 *       - 数据变更序列号处理
 *       - 保存频率控制逻辑
 *       - 紧急保存状态检测
 *       - 保存完成回调处理
 * 
 * @warning 使用注意事项：
 *          - 保存时间戳用于避免重复保存
 *          - 继承类需要实现具体的保存逻辑
 *          - 保存失败时需要处理回滚逻辑
 *          - 注意控制保存频率避免数据库压力
 */

#include "NFISaveDb.h"

NFISaveDb::NFISaveDb()
{
    if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
    {
        CreateInit();
    }
    else
    {
        ResumeInit();
    }
}

NFISaveDb::~NFISaveDb()
{
}

int NFISaveDb::CreateInit()
{
    m_lastSaveDbTime = 0;
    m_curSavingDbTime = 0;
    m_lastAllSeq = 0;
    return 0;
}


int NFISaveDb::ResumeInit()
{
    return 0;
}


bool NFISaveDb::IsNeedSave() const
{
    if (IsUrgentNeedSave())
    {
        return true;
    }
    return true;
}

bool NFISaveDb::IsSavingDb() const
{
    return (m_curSavingDbTime != 0) && (static_cast<uint64_t>(NF_ADJUST_TIMENOW()) <= m_curSavingDbTime + SAVE_TRANS_ACTIVE_TIMEOUT + 5);
}

int NFISaveDb::SendTransToDb(int iReason)
{
    return 0;
}

bool NFISaveDb::CanSaveDb(uint32_t maxTimes, bool bForce)
{
    if (IsNeedSave() && !IsSavingDb())
    {
        if (bForce || NF_ADJUST_TIMENOW() - m_lastSaveDbTime >= maxTimes)
        {
            return true;
        }
    }
    return false;
}

int NFISaveDb::SaveToDb(uint32_t maxTimes, int iReason, bool bForce)
{
    if (CanSaveDb(maxTimes, bForce))
    {
        SendTransToDb(iReason);
    }
    return 0;
}

int NFISaveDb::OnSaveDb(bool success, uint32_t seq)
{
    m_lastAllSeq = seq;
    m_lastSaveDbTime = NF_ADJUST_TIMENOW();
    m_curSavingDbTime = 0;
    if (success && seq == GetAllSeq())
    {
        ClearAllSeq();
    }
    return 0;
}

uint32_t NFISaveDb::GetAllSeq() const
{
    return GetCurSeq();
}

void NFISaveDb::ClearAllSeq()
{
    ClearUrgent();
}