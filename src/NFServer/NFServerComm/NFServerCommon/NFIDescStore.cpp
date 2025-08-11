// -------------------------------------------------------------------------
//    @FileName         :    NFIDescStore.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDescStore.cpp
//    @Desc             :    描述存储接口实现文件，提供配置数据存储的基础功能实现。
//                          该文件实现了描述存储接口类的方法，包括对象初始化、
//                          数据库操作、定时器处理、保存机制。
//                          主要功能包括提供配置数据存储的基础功能实现、支持数据库操作、
//                          支持定时器处理和保存机制、提供对象生命周期管理。
//                          描述存储接口实现是NFShmXFrame框架的配置管理基础组件实现，负责：
//                          - 配置数据存储的基础功能实现
//                          - 数据库操作接口实现
//                          - 定时器处理和保存机制实现
//                          - 对象生命周期管理实现
//                          - 配置数据的加载和保存实现
//                          - 跨服务器配置数据同步实现
//
// -------------------------------------------------------------------------

#include "NFIDescStore.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFIDescStoreModule.h"
#include "NFComm/NFCore/NFRandom.hpp"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFIDescStore::NFIDescStore():NFObject()
{
    if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode()) {
        CreateInit();
    }
    else {
        ResumeInit();
    }
}

/**
 * @brief 析构函数
 * 
 * 清理定时器资源
 */
NFIDescStore::~NFIDescStore()
{
    if (m_bSaveTimer != INVALID_ID)
    {
        DeleteTimer(m_bSaveTimer);
        m_bSaveTimer = INVALID_ID;
    }
}

/**
 * @brief 创建初始化
 * 
 * 在对象创建时调用，初始化描述存储对象的所有成员变量
 * 
 * @return 初始化结果
 */
int NFIDescStore::CreateInit()
{
    m_bValid = false;         // 对象无效
    m_bIsLoaded = false;      // 数据未加载
    m_bIsChecked = false;     // 数据未检查
    m_bSaveTimer = INVALID_ID; // 保存定时器为无效ID
    m_bIsReLoading = false;   // 未重新加载
    m_bIsDBLoaded = false;    // 数据库未加载
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 在对象恢复时调用，恢复描述存储对象状态
 * 
 * @return 初始化结果
 */
int NFIDescStore::ResumeInit()
{
    return 0;
}

/**
 * @brief 保存描述存储到数据库
 * 
 * 如果不是文件加载模式，则调用描述存储模块保存数据到数据库
 * 
 * @param pMessage 消息对象
 * @return 保存结果
 */
int NFIDescStore::SaveDescStoreToDB(const google::protobuf::Message *pMessage)
{
    if (!IsFileLoad())
    {
        return FindModule<NFIDescStoreModule>()->SaveDescStoreByFileName(GetDBName(), GetFileName(), pMessage);
    }
    return 0;
}

/**
 * @brief 插入描述存储到数据库
 * 
 * 如果不是文件加载模式，则调用描述存储模块插入数据到数据库
 * 
 * @param pMessage 消息对象
 * @return 插入结果
 */
int NFIDescStore::InsertDescStoreToDB(const google::protobuf::Message *pMessage)
{
    if (!IsFileLoad())
    {
        return FindModule<NFIDescStoreModule>()->InsertDescStoreByFileName(GetDBName(), GetFileName(), pMessage);
    }
    return 0;
}

/**
 * @brief 从数据库删除描述存储
 * 
 * 如果不是文件加载模式，则调用描述存储模块从数据库删除数据
 * 
 * @param pMessage 消息对象
 * @return 删除结果
 */
int NFIDescStore::DeleteDescStoreToDB(const google::protobuf::Message *pMessage)
{
    if (!IsFileLoad())
    {
        return FindModule<NFIDescStoreModule>()->DeleteDescStoreByFileName(GetDBName(), GetFileName(), pMessage);
    }
    return 0;
}

/**
 * @brief 启动保存定时器
 * 
 * 启动自动保存定时器，使用随机延迟避免同时保存
 * 
 * @return 启动结果
 */
int NFIDescStore::StartSaveTimer()
{
    int rand = NFRandInt(1000, 10000);
    m_bSaveTimer = SetTimer(1*1000, 0, 0, 0, 0, rand);
    return 0;
}

/**
 * @brief 定时器回调
 * 
 * 处理定时器事件，包括保存定时器回调
 * 
 * @param timeId 定时器ID
 * @param callcount 调用次数
 * @return 处理结果
 */
//must be virtual
int NFIDescStore::OnTimer(int timeId, int callcount)
{
    if (m_bSaveTimer == timeId)
    {
        SaveDescStore();
    }
    return 0;
}



