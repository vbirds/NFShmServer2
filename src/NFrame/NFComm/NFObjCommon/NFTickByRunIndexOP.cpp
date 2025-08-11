// -------------------------------------------------------------------------
//    @FileName         :    NFTickByRunIndexOP.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTickByRunIndexOP.cpp
//
// -------------------------------------------------------------------------

/**
 * @file NFTickByRunIndexOP.cpp
 * @brief 按运行索引分批处理操作实现文件
 * @details 实现了NFrame框架中分批处理操作类NFTickByRunIndexOP的核心功能，提供了
 *          基于运行索引的分批处理机制。通过控制处理频率和批次大小，避免单次
 *          处理过多数据造成的性能问题，适用于大量数据的分批处理场景。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 实现的主要功能：
 *       - 基于运行索引的定时触发机制
 *       - 分批处理的执行控制
 *       - 处理状态的跟踪和重置
 *       - 停止时的全量处理支持
 *       - 处理频率的动态调整机制
 * 
 * @warning 使用注意事项：
 *          - 继承类必须实现DoTick()抽象方法
 *          - 处理失败会记录错误日志但不中断流程
 *          - 停止处理时会执行所有剩余数据
 *          - 频率调整需要通过DoChangeTickGap()实现
 */

#include "NFTickByRunIndexOP.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"

int NFTickByRunIndexOP::TickNow(uint32_t dwCurRunIndex)
{
    int iRetCode = 0;

    if (IsNeedTick(dwCurRunIndex) == false)
    {
        return 0;
    }

    m_dwLastTickRunIndex=dwCurRunIndex;
    m_iTickedNum = 0;

    iRetCode = DoTick(dwCurRunIndex);
    if (iRetCode != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "DoTick Failed");
    }

    ResetPerTick(dwCurRunIndex);

    return 0;
}

int NFTickByRunIndexOP::TickAllWhenStop(uint32_t dwCurRunIndex)
{
    int iRetCode = 0;
    iRetCode = DoTick(dwCurRunIndex, true);
    if (iRetCode != 0)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "DoTick Failed");
    }

    m_iTickedNum = 0;

    ResetPerTick(dwCurRunIndex);

    return 0;
}

int NFTickByRunIndexOP::DoChangeTickGap()
{
    return 0;
}