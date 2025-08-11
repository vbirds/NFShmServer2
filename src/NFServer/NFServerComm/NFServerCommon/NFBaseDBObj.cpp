// -------------------------------------------------------------------------
//    @FileName         :    NFBaseDBObj.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFBaseDBObj.cpp
//    @Desc             :    基础数据库对象实现文件，提供数据库对象的基础功能实现。
//                          该文件实现了基础数据库对象类的方法，包括对象初始化、
//                          数据加载和保存、重试机制、定时器处理。
//                          主要功能包括提供数据库对象的基础功能实现、支持数据加载和保存、
//                          支持重试机制和错误处理、提供定时器处理。
//                          基础数据库对象实现是NFShmXFrame框架的数据库操作基础组件实现，负责：
//                          - 数据库对象的基础功能实现
//                          - 数据加载和保存接口实现
//                          - 重试机制和错误处理实现
//                          - 定时器处理和状态管理实现
//                          - 数据库操作的生命周期管理实现
//                          - 跨服务器数据库同步实现
//
// -------------------------------------------------------------------------

#include "NFBaseDBObj.h"

#include "NFDBObjMgr.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFBaseDBObj::NFBaseDBObj()
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
NFBaseDBObj::~NFBaseDBObj()
{
}

/**
 * @brief 创建初始化
 * 
 * 在对象创建时调用，初始化数据库对象的所有成员变量
 * 
 * @return 初始化结果
 */
int NFBaseDBObj::CreateInit()
{
    m_bDataInited = false;        // 数据未初始化
    m_iTransID = 0;               // 事务ID为0
    m_iLastDBOpTime = 0;          // 最后数据库操作时间为0
    m_uModKey = 0;                // 模块键值为0
    m_iRetryTimes = 0;            // 重试次数为0
    m_bNeedInsertDB = false;      // 不需要插入数据库
    m_iServerType = NF_ST_NONE;   // 服务器类型为无
    m_iRetryDelayTimer = INVALID_ID; // 重试延迟定时器为无效ID
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 在对象恢复时调用，恢复数据库对象状态
 * 
 * @return 初始化结果
 */
int NFBaseDBObj::ResumeInit()
{
    return 0;
}

/**
 * @brief 定时器回调
 * 
 * 处理定时器事件，包括重试延迟等
 * 
 * @param timeId 定时器ID
 * @param callCount 调用次数
 * @return 处理结果
 */
int NFBaseDBObj::OnTimer(int timeId, int callCount)
{
    if (timeId == m_iRetryDelayTimer)
    {
        SyncReqRetry();
    }
    return 0;
}

/**
 * @brief 同步请求重试
 * 
 * 执行同步请求重试操作，清除重试延迟定时器并从数据库加载对象
 * 
 * @return 重试结果
 */
int NFBaseDBObj::SyncReqRetry()
{
    m_iRetryDelayTimer = INVALID_ID;
    int iRet = NFDBObjMgr::Instance()->LoadFromDB(this);
    CHECK_ERR(0, iRet, "LoadFromDB Failed, class:{}", GetClassName());
    return 0;
}

/**
 * @brief 延迟同步请求重试
 * 
 * 执行延迟同步请求重试操作，设置重试延迟定时器
 * 
 * @return 重试结果
 */
int NFBaseDBObj::DelaySyncReqRetry()
{
    // 如果已有重试延迟定时器，先删除
    if (m_iRetryDelayTimer != INVALID_ID)
    {
        DeleteTimer(m_iRetryDelayTimer);
        m_iRetryDelayTimer = INVALID_ID;
    }

    // 设置重试延迟定时器，GetRetryDis()秒后执行
    m_iRetryDelayTimer = SetTimer(0, 0, GetRetryDis(), 0);
    if (m_iRetryDelayTimer == INVALID_ID)
    {
        CHECK_ERR(0, -1, "SetTimer Failed");
    }
    return 0;
}
