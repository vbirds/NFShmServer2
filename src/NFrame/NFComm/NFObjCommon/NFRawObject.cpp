// -------------------------------------------------------------------------
//    @FileName         :    NFRawObject.cpp
//    @Author           :    gaoyi
//    @Date             :    23-9-25
//    @Email			:    445267987@qq.com
//    @Module           :    NFRawObject
//
// -------------------------------------------------------------------------

/**
 * @file NFRawObject.cpp
 * @brief 原始对象类实现文件
 * @details 实现了NFrame框架中原始对象类NFRawObject的核心功能，主要提供共享内存
 *          对象的封装和代理服务。通过内部持有共享内存对象指针，为普通对象提供
 *          定时器等共享内存对象的服务，是连接普通对象和共享内存对象的桥梁。
 * 
 * @author gaoyi
 * @date 23-9-25
 * @email 445267987@qq.com
 * 
 * @note 实现的主要功能：
 *       - 原始对象的生命周期管理
 *       - 共享内存对象的初始化和绑定
 *       - 定时器服务的代理接口
 *       - 对象销毁时的资源清理
 *       - 共享内存模式的适配处理
 * 
 * @warning 使用注意事项：
 *          - 必须调用InitShmObj()绑定共享内存对象
 *          - 定时器功能依赖于共享内存对象的有效性
 *          - 对象销毁时会自动清理所有定时器
 *          - 不要在未绑定共享内存对象时使用定时器功能
 */

#include "NFRawObject.h"

NFRawObject::NFRawObject()
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

NFRawObject::~NFRawObject()
{
    NFRawObject::DeleteAllTimer();
}

int NFRawObject::CreateInit()
{
    m_pShmObj = nullptr;
    return 0;
}

int NFRawObject::ResumeInit()
{
    return 0;
}

int NFRawObject::InitShmObj(const NFObject* pShmObj)
{
    m_pShmObj = pShmObj;
    return 0;
}

NFObject* NFRawObject::GetShmObj()
{
    return m_pShmObj.GetPoint();
}

int NFRawObject::OnTimer(int timeId, int callCount)
{
    return 0;
}