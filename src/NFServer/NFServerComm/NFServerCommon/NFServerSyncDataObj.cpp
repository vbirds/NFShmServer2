// -------------------------------------------------------------------------
//    @FileName         :    NFServerSyncDataObj.cpp
//    @Author           :    gaoyi
//    @Date             :    2025-07-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerSyncDataObj
//    @Desc             :    服务器同步数据对象实现文件，提供服务器间数据同步的对象功能实现。
//                          该文件实现了服务器同步数据对象类的方法，包括对象初始化、
//                          同步请求处理、重试机制、定时器处理。
//                          主要功能包括提供服务器间数据同步的对象功能实现、支持对象初始化、
//                          支持同步请求处理和重试机制、提供定时器处理。
//                          服务器同步数据对象实现是NFShmXFrame框架的服务器同步组件实现，负责：
//                          - 服务器间数据同步的对象功能实现
//                          - 对象初始化和状态管理实现
//                          - 同步请求处理和重试机制实现
//                          - 定时器处理和错误处理实现
//                          - 数据转换和消息处理实现
//                          - 跨服务器数据一致性实现
//
// -------------------------------------------------------------------------

#include "NFServerSyncDataObj.h"

#include "NFServerSyncDataObjMgr.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFServerSyncDataObj::NFServerSyncDataObj()
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

/**
 * @brief 析构函数
 */
NFServerSyncDataObj::~NFServerSyncDataObj()
{
}

/**
 * @brief 创建初始化
 * 
 * 初始化所有成员变量为默认值
 * 
 * @return 0表示成功
 */
int NFServerSyncDataObj::CreateInit()
{
    m_bDataInited = false; ///< 数据未初始化
    m_iTransID = 0; ///< 事务ID为0
    m_iLastSyncOpTime = 0; ///< 最后同步操作时间为0
    m_iRetryTimes = 0; ///< 重试次数为0
    m_iServerType = NF_ST_NONE; ///< 服务器类型为无
    m_iDstServerType = NF_ST_NONE; ///< 目标服务器类型为无
    m_iDstBusId = 0; ///< 目标总线ID为0
    m_iMsgId = 0; ///< 消息ID为0
    m_iRetryDelayTimer = INVALID_ID; ///< 重试延迟定时器为无效ID
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 从共享内存恢复对象状态
 * 
 * @return 0表示成功
 */
int NFServerSyncDataObj::ResumeInit()
{
    return 0;
}

/**
 * @brief 处理失败情况
 * 
 * 根据数据是否已初始化决定处理方式：
 * - 如果数据未初始化，则重试
 * - 如果数据已初始化，则记录错误
 * 
 * @return 失败处理方式
 */
eDealWithSyncFailed NFServerSyncDataObj::DealWithFailed()
{
    if (!m_bDataInited)
    {
        return EN_DW_SYNC_RETRY; ///< 数据未初始化，重试
    }
    else
    {
        return EN_DW_SYNC_LOG_FAIL; ///< 数据已初始化，记录错误
    }
}

/**
 * @brief 定时器回调
 * 
 * 处理重试延迟定时器事件
 * 
 * @param timeId 定时器ID
 * @param callCount 调用次数
 * @return 0表示成功
 */
int NFServerSyncDataObj::OnTimer(int timeId, int callCount)
{
    if (timeId == m_iRetryDelayTimer)
    {
        SyncReqRetry(); ///< 执行重试同步请求
    }
    return 0;
}

/**
 * @brief 同步请求重试
 * 
 * 清除重试延迟定时器，重新发送同步请求
 * 
 * @return 0表示成功
 */
int NFServerSyncDataObj::SyncReqRetry()
{
    m_iRetryDelayTimer = INVALID_ID; ///< 清除重试延迟定时器
    int iRet = NFServerSyncDataObjMgr::Instance()->SyncReq(this); ///< 重新发送同步请求
    CHECK_ERR(0, iRet, "SyncReq Failed, class:{}", GetClassName()); ///< 检查错误
    return 0;
}

/**
 * @brief 延迟同步请求重试
 * 
 * 设置延迟定时器，在指定时间后重试同步请求
 * 
 * @return 0表示成功
 */
int NFServerSyncDataObj::DelaySyncReqRetry()
{
    if (m_iRetryDelayTimer != INVALID_ID)
    {
        DeleteTimer(m_iRetryDelayTimer); ///< 删除现有定时器
        m_iRetryDelayTimer = INVALID_ID; ///< 清除定时器ID
    }

    //sec秒后，执行定时器
    m_iRetryDelayTimer = SetTimer(0, 0, GetRetryDis(), 0); ///< 设置重试延迟定时器
    if (m_iRetryDelayTimer == INVALID_ID)
    {
        CHECK_ERR(0, -1, "SetTimer Failed"); ///< 检查定时器设置失败
    }
    return 0;
}
