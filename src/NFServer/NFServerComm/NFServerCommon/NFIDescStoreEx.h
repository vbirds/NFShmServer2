// -------------------------------------------------------------------------
//    @FileName         :    NFIDescStoreEx.h
//    @Author           :    gaoyi
//    @Date             :    23-9-19
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDescStoreEx
//    @Desc             :    描述存储扩展接口头文件，提供配置数据存储的扩展功能。
//                          该文件定义了描述存储扩展接口类，包括扩展的加载和重载功能、
//                          状态管理、有效性检查、重载准备。
//                          主要功能包括提供配置数据存储的扩展功能、支持扩展的加载和重载、
//                          支持状态管理和有效性检查、提供重载准备功能。
//                          描述存储扩展接口是NFShmXFrame框架的配置管理扩展组件，负责：
//                          - 配置数据存储的扩展功能
//                          - 扩展的加载和重载接口
//                          - 状态管理和有效性检查
//                          - 重载准备和状态维护
//                          - 数据有效性验证
//                          - 跨服务器配置同步扩展
//
// -------------------------------------------------------------------------

#pragma once


#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"

/**
 * @brief 描述存储扩展接口类
 * 
 * 该类是配置数据存储的扩展接口，提供了：
 * - 扩展的加载和重载功能
 * - 状态管理和有效性检查
 * - 重载准备和状态维护
 * - 数据有效性验证
 * 
 * 主要功能：
 * - 提供配置数据存储的扩展功能
 * - 支持扩展的加载和重载接口
 * - 支持状态管理和有效性检查
 * - 提供重载准备和状态维护
 * - 支持数据有效性验证
 * - 支持跨服务器配置同步扩展
 * 
 * 使用方式：
 * @code
 * class MyDescStoreEx : public NFIDescStoreEx {
 * public:
 *     virtual int Load() override;
 *     virtual int Reload() override;
 *     virtual int CheckWhenAllDataLoaded() override;
 * };
 * @endcode
 */
class NFIDescStoreEx : public NFObject
{
public:
    /**
     * @brief 构造函数
     */
    NFIDescStoreEx();

    /**
     * @brief 析构函数
     */
    virtual ~NFIDescStoreEx();

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化描述存储扩展对象
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复描述存储扩展对象状态
     * 
     * @return 初始化结果
     */
    int ResumeInit();
public:
    /**
     * @brief 加载数据
     * 
     * 加载配置数据
     * 
     * @return 加载结果
     */
    virtual int Load();

    /**
     * @brief 重新加载数据
     * 
     * 重新加载配置数据
     * 
     * @return 重新加载结果
     */
    virtual int Reload();

    /**
     * @brief 检查所有数据加载完成
     * 
     * 检查所有配置数据是否已经加载完成
     * 
     * @return 检查结果
     */
    virtual int CheckWhenAllDataLoaded();
public:
    /**
     * @brief 设置有效状态
     * 
     * 将对象设置为有效状态
     */
    void SetValid()
    {
        m_bValid = true;
    }

    /**
     * @brief 检查是否有效
     * 
     * 检查对象是否处于有效状态
     * 
     * @return 是否有效
     */
    bool IsValid()
    {
        return m_bValid;
    }

    /**
     * @brief 设置加载状态
     * 
     * 设置对象的加载状态
     * 
     * @param bIsLoaded 是否已加载
     */
    void SetLoaded(bool bIsLoaded)
    {
        m_bIsLoaded = bIsLoaded;
    }

    /**
     * @brief 检查是否已加载
     * 
     * 检查对象是否已经加载完成
     * 
     * @return 是否已加载
     */
    bool IsLoaded()
    {
        return m_bIsLoaded;
    }

    /**
     * @brief 设置检查状态
     * 
     * 设置对象的检查状态
     * 
     * @param bIsChecked 是否已检查
     */
    void SetChecked(bool bIsChecked)
    {
        m_bIsChecked = bIsChecked;
    }

    /**
     * @brief 检查是否已检查
     * 
     * 检查对象是否已经检查完成
     * 
     * @return 是否已检查
     */
    bool IsChecked()
    {
        return m_bIsChecked;
    }

    /**
     * @brief 准备重载
     * 
     * 准备重新加载数据
     * 
     * @return 准备结果
     */
    virtual int PrepareReload()
    {
        return 0;
    }

    /**
     * @brief 设置重载状态
     * 
     * 设置对象的重载状态
     * 
     * @param bIsLoaded 是否正在重载
     */
    void SetReLoading(bool bIsLoaded)
    {
        m_bIsReLoading = bIsLoaded;
    }

    /**
     * @brief 检查是否正在重载
     * 
     * 检查对象是否正在重载
     * 
     * @return 是否正在重载
     */
    bool IsReloading()
    {
        return m_bIsReLoading;
    }

    /**
     * @brief 检查是否需要重载
     * 
     * 检查对象是否需要重载
     * 
     * @return 是否需要重载
     */
    virtual bool IsNeedReload()
    {
        return IsReloading();
    }
private:
    bool m_bValid; ///< 是否有效
    bool m_bIsLoaded; ///< 是否已加载
    bool m_bIsChecked; ///< 是否已检查
    bool m_bIsReLoading; ///< 是否正在重载
};