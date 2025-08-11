// -------------------------------------------------------------------------
//    @FileName         :    NFShmSubcribeInfo.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmSubcribeInfo.h
//    @Desc             :    共享内存订阅信息实现文件，提供事件订阅信息的管理功能。
//                          该文件实现了订阅信息对象的核心功能，包括订阅信息对象的创建和初始化、
//                          引用计数管理、订阅状态管理、订阅信息字符串化。
//                          主要功能包括订阅信息初始化、引用计数管理、状态标志管理、
//                          信息格式化输出。
//                          设计特点包括基于共享内存支持跨进程、引用计数机制、
//                          状态标志管理、信息完整性保证
//
// -------------------------------------------------------------------------

#include "NFShmSubscribeInfo.h"
#include "NFComm/NFObjCommon/NFTypeDefines.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"

/**
 * @brief 构造函数
 * 
 * 初始化订阅信息对象，根据共享内存模式选择初始化方式
 */
NFShmSubscribeInfo::NFShmSubscribeInfo()
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
 * 在创建时进行初始化，设置所有成员变量为默认值
 * 
 * @return 0 成功，其他值表示失败
 */
int NFShmSubscribeInfo::CreateInit()
{
    // 初始化订阅对象指针
    m_pSink = NULL;
    // 初始化引用计数
    m_refCount = 0;
    // 初始化移除标志
    m_removeFlag = false;
    // 初始化共享内存对象ID
    m_shmObjId = INVALID_ID;
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 在恢复时进行初始化
 * 
 * @return 0 成功，其他值表示失败
 */
int NFShmSubscribeInfo::ResumeInit()
{
    return 0;
}

/**
 * @brief 增加引用计数
 * 
 * 增加订阅信息的引用计数
 */
void NFShmSubscribeInfo::Add()
{
    m_refCount++;
}

/**
 * @brief 减少引用计数
 * 
 * 减少订阅信息的引用计数
 */
void NFShmSubscribeInfo::Sub()
{
    --m_refCount;
}

/**
 * @brief 转换为字符串
 * 
 * 将订阅信息对象转换为字符串格式，用于调试和日志
 * 
 * @return 格式化的字符串
 */
std::string NFShmSubscribeInfo::ToString() const
{
    return NF_FORMAT("refCount:{},removeFlag:{},desc:{},eventKey:{},shmObjId:{}", m_refCount, m_removeFlag, m_szDesc.ToString(), m_eventKey.ToString(), m_shmObjId);
}
