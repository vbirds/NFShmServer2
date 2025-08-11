// -------------------------------------------------------------------------
//    @FileName         :    NFIDescStoreEx.cpp
//    @Author           :    gaoyi
//    @Date             :    23-9-19
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDescStoreEx
//    @Desc             :    描述存储扩展接口实现文件，提供配置数据存储的扩展功能实现。
//                          该文件实现了描述存储扩展接口类的方法，包括扩展的加载和重载功能、
//                          状态管理、有效性检查、重载准备。
//                          主要功能包括提供配置数据存储的扩展功能实现、支持扩展的加载和重载、
//                          支持状态管理和有效性检查、提供重载准备功能。
//                          描述存储扩展接口实现是NFShmXFrame框架的配置管理扩展组件实现，负责：
//                          - 配置数据存储的扩展功能实现
//                          - 扩展的加载和重载接口实现
//                          - 状态管理和有效性检查实现
//                          - 重载准备和状态维护实现
//                          - 数据有效性验证实现
//                          - 跨服务器配置同步扩展实现
//
// -------------------------------------------------------------------------

#include "NFIDescStoreEx.h"

/**
 * @brief 构造函数
 * 
 * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
 */
NFIDescStoreEx::NFIDescStoreEx()
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
NFIDescStoreEx::~NFIDescStoreEx()
{
}

/**
 * @brief 创建初始化
 * 
 * 初始化所有状态标志为默认值
 * 
 * @return 0表示成功
 */
int NFIDescStoreEx::CreateInit()
{
    m_bValid = false; ///< 初始化为无效状态
    m_bIsLoaded = false; ///< 初始化为未加载状态
    m_bIsChecked = false; ///< 初始化为未检查状态
    m_bIsReLoading = false; ///< 初始化为非重载状态
    return 0;
}

/**
 * @brief 恢复初始化
 * 
 * 从共享内存恢复对象状态
 * 
 * @return 0表示成功
 */
int NFIDescStoreEx::ResumeInit()
{
    return 0;
}

/**
 * @brief 加载数据
 * 
 * 加载配置数据的虚函数，子类需要重写此方法
 * 
 * @return 0表示成功
 */
int NFIDescStoreEx::Load()
{
    return 0;
}

/**
 * @brief 重载数据
 * 
 * 准备重载并执行加载操作
 * 
 * @return 加载操作的返回码
 */
int NFIDescStoreEx::Reload()
{
    PrepareReload(); ///< 准备重载
    int iRetCode = Load(); ///< 执行加载
    return iRetCode;
}

/**
 * @brief 检查所有数据加载完成
 * 
 * 检查所有配置数据是否加载完成的虚函数，子类需要重写此方法
 * 
 * @return 0表示成功
 */
int NFIDescStoreEx::CheckWhenAllDataLoaded()
{
    return 0;
}
