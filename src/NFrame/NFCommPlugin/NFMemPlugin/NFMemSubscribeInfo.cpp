// -------------------------------------------------------------------------
//    @FileName         :    NFShmSubcribeInfo.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmSubcribeInfo.h
//    @Desc             :    内存订阅信息实现文件，提供事件订阅信息管理功能，包括事件订阅信息的创建和初始化、
//                          订阅信息的引用计数管理、订阅信息的生命周期管理。
//                          该文件实现了NFShmXFrame框架的内存订阅信息类，提供订阅信息管理、引用计数控制、
//                          事件键值关联、订阅描述信息等功能。
//                          主要功能包括订阅信息管理、引用计数控制、事件键值关联、订阅描述信息
//
// -------------------------------------------------------------------------

#include "NFMemSubscribeInfo.h"
#include "NFComm/NFObjCommon/NFTypeDefines.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式进行相应的初始化
 */
NFMemSubscribeInfo::NFMemSubscribeInfo()
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
 * @brief 创建初始化
 * 
 * 该函数在对象创建时被调用，负责初始化订阅信息对象的成员变量。
 * 
 * @return 初始化结果，0表示成功
 */
int NFMemSubscribeInfo::CreateInit()
{
    m_pSink = NULL;
    m_refCount = 0;
    m_removeFlag = false;
    m_shmObjId = INVALID_ID;
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 该函数在对象恢复时被调用，负责恢复订阅信息对象的状态。
 * 
 * @return 初始化结果，0表示成功
 */
int NFMemSubscribeInfo::ResumeInit()
{
    return 0;
}

/**
 * @brief 增加引用计数
 * 
 * 该函数用于增加订阅信息的引用计数。
 * 引用计数用于跟踪订阅信息的使用情况，防止过早释放。
 */
void NFMemSubscribeInfo::Add()
{
    m_refCount++;
}

/**
 * @brief 减少引用计数
 * 
 * 该函数用于减少订阅信息的引用计数。
 * 当引用计数为0时，可以考虑释放订阅信息对象。
 */
void NFMemSubscribeInfo::Sub()
{
    --m_refCount;
}

/**
 * @brief 获取字符串表示
 * 
 * 该函数返回订阅信息对象的字符串表示，包括：
 * - 引用计数
 * - 移除标志
 * - 订阅描述
 * - 事件键值
 * - 共享内存对象ID
 * 
 * @return 字符串表示
 */
std::string NFMemSubscribeInfo::ToString() const
{
    return NF_FORMAT("refCount:{},removeFlag:{},desc:{},eventKey:{},shmObjId:{}", m_refCount, m_removeFlag, m_szDesc.ToString(), m_eventKey.ToString(), m_shmObjId);
}
