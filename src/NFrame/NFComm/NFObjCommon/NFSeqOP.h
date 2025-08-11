// -------------------------------------------------------------------------
//    @FileName         :    NFSeqOP.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFSeqOP.h
//
// -------------------------------------------------------------------------

/**
 * @file NFSeqOP.h
 * @brief 序列操作类定义文件
 * @details 定义了NFrame框架中的序列操作基类，提供了数据变更跟踪、脏数据标记、
 *          紧急保存标记等功能。主要用于数据持久化场景，通过序列号来跟踪数据
 *          的变更状态，支持增量保存和批量保存策略。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFSeqOP: 序列操作基类
 *       - NFDescStoreSeqOP: 描述存储序列操作类
 *       - 数据变更序列号管理
 *       - 脏数据标记机制
 *       - 紧急保存状态管理
 * 
 * @warning 使用序列操作时需要注意：
 *          - 每次数据变更都应该调用MarkDirty()
 *          - 紧急数据应该调用MarkUrgent()
 *          - 保存完成后应该调用ClearUrgent()
 *          - 序列号用于跟踪数据版本，不要手动修改
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFShmMgr.h"

/**
 * @class NFSeqOP
 * @brief 序列操作基类
 * @details 提供数据变更跟踪、脏数据标记、紧急保存标记等功能。主要用于数据持久化场景，
 *          通过序列号来跟踪数据的变更状态，支持增量保存和批量保存策略。当数据发生变更时，
 *          会自动递增序列号并标记为需要保存状态。
 * 
 * @note 核心功能：
 *       - 数据变更序列号自动管理
 *       - 脏数据状态跟踪
 *       - 紧急保存状态标记
 *       - 支持数据保存状态重置
 * 
 * @warning 使用注意事项：
 *          - 每次数据变更都应该调用MarkDirty()
 *          - 紧急数据变更应该调用MarkUrgent()
 *          - 保存完成后应该调用ClearUrgent()
 *          - 不要手动修改序列号
 */
class NFSeqOP
{
public:
    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式，选择调用CreateInit()或ResumeInit()
     */
    NFSeqOP()
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
     * @brief 虚析构函数
     * @details 确保派生类能够正确析构
     */
    virtual ~NFSeqOP()
    {
    }

    /**
     * @brief 创建初始化函数
     * @details 当对象第一次创建时调用，初始化序列号和状态标志
     * @return int 初始化结果，0表示成功
     */
    int CreateInit()
    {
        m_dwCurSeq = 0;
        m_bIsUrgentNeedSave = false;
        return 0;
    }

    /**
     * @brief 重置当前序列号
     * @details 将序列号重置为0，并清除紧急保存标志
     * @note 通常在数据保存完成后调用
     */
    void ResetCurSeq()
    {
        m_dwCurSeq = 0;
        m_bIsUrgentNeedSave = false;
    }

    /**
     * @brief 恢复初始化函数
     * @details 当对象从共享内存恢复时调用，不需要重新初始化状态
     * @return int 恢复结果，0表示成功
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 检查是否紧急需要保存
     * @details 返回当前是否处于紧急保存状态
     * @return bool true表示需要紧急保存，false表示不需要
     * @note 紧急保存通常用于重要数据的及时持久化
     */
    virtual bool IsUrgentNeedSave() const
    {
        return m_bIsUrgentNeedSave;
    }

    /**
     * @brief 获取当前序列号
     * @details 返回当前的数据变更序列号
     * @return uint32_t 当前序列号，用于跟踪数据版本
     * @note 序列号在每次数据变更时自动递增
     */
    virtual uint32_t GetCurSeq() const
    {
        return m_dwCurSeq;
    }

    /**
     * @brief 标记数据为脏状态
     * @details 当数据发生变更时调用，递增序列号并标记为紧急保存
     * @note 这是最常用的标记函数，大部分数据变更都应该调用此函数
     */
    virtual void MarkDirty()
    {
        m_dwCurSeq++;
        m_bIsUrgentNeedSave = true;
    }

    /**
     * @brief 标记为紧急状态
     * @details 标记数据为紧急需要保存状态，递增序列号
     * @note 与MarkDirty()功能相同，用于强调数据的重要性
     */
    virtual void MarkUrgent()
    {
        m_dwCurSeq++;
        m_bIsUrgentNeedSave = true;
    }

    /**
     * @brief 清除紧急保存标志
     * @details 清除紧急保存状态，通常在数据保存完成后调用
     * @note 仅清除紧急标志，不影响序列号
     */
    virtual void ClearUrgent()
    {
        m_bIsUrgentNeedSave = false;
    }

protected:
    /**
     * @brief 当前序列号
     * @details 数据变更的序列号，每次变更时自动递增
     * @note 用于跟踪数据的版本变化，支持增量保存
     */
    uint32_t m_dwCurSeq;
    
    /**
     * @brief 紧急保存标志
     * @details 标识数据是否需要紧急保存
     * @note true表示需要尽快保存，false表示可以延迟保存
     */
    bool m_bIsUrgentNeedSave;
};

/**
 * @class NFDescStoreSeqOP
 * @brief 描述存储序列操作类
 * @details 专门用于描述存储（配置数据）的序列操作管理，提供与NFSeqOP相同的功能，
 *          但是专门针对描述存储场景进行了优化。主要用于配置数据的变更跟踪和保存管理。
 * 
 * @note 与NFSeqOP的区别：
 *       - 专门用于描述存储场景
 *       - 可能有不同的保存策略
 *       - 功能接口与NFSeqOP保持一致
 * 
 * @warning 使用注意事项与NFSeqOP相同
 */
class NFDescStoreSeqOP
{
public:
    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式，选择调用CreateInit()或ResumeInit()
     */
    NFDescStoreSeqOP()
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
     * @brief 创建初始化函数
     * @details 当对象第一次创建时调用，初始化序列号和状态标志
     * @return int 初始化结果，0表示成功
     */
    int CreateInit()
    {
        m_dwCurSeq = 0;
        m_bIsUrgentNeedSave = false;
        return 0;
    }

    /**
     * @brief 重置当前序列号
     * @details 将序列号重置为0，并清除紧急保存标志
     * @note 通常在数据保存完成后调用
     */
    void ResetCurSeq()
    {
        m_dwCurSeq = 0;
        m_bIsUrgentNeedSave = false;
    }

    /**
     * @brief 恢复初始化函数
     * @details 当对象从共享内存恢复时调用，不需要重新初始化状态
     * @return int 恢复结果，0表示成功
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 检查是否紧急需要保存
     * @details 返回当前是否处于紧急保存状态
     * @return bool true表示需要紧急保存，false表示不需要
     * @note 紧急保存通常用于重要配置数据的及时持久化
     */
    virtual bool IsUrgentNeedSave() const
    {
        return m_bIsUrgentNeedSave;
    }

    /**
     * @brief 获取当前序列号
     * @details 返回当前的数据变更序列号
     * @return uint32_t 当前序列号，用于跟踪配置数据版本
     * @note 序列号在每次配置数据变更时自动递增
     */
    virtual uint32_t GetCurSeq() const
    {
        return m_dwCurSeq;
    }

    /**
     * @brief 标记配置数据为脏状态
     * @details 当配置数据发生变更时调用，递增序列号并标记为紧急保存
     * @note 这是最常用的标记函数，大部分配置数据变更都应该调用此函数
     */
    virtual void MarkDirty()
    {
        m_dwCurSeq++;
        m_bIsUrgentNeedSave = true;
    }

    /**
     * @brief 标记为紧急状态
     * @details 标记配置数据为紧急需要保存状态，递增序列号
     * @note 与MarkDirty()功能相同，用于强调配置数据的重要性
     */
    virtual void MarkUrgent()
    {
        m_dwCurSeq++;
        m_bIsUrgentNeedSave = true;
    }

    /**
     * @brief 清除紧急保存标志
     * @details 清除紧急保存状态，通常在配置数据保存完成后调用
     * @note 仅清除紧急标志，不影响序列号
     */
    virtual void ClearUrgent()
    {
        m_bIsUrgentNeedSave = false;
    }

protected:
    /**
     * @brief 当前序列号
     * @details 配置数据变更的序列号，每次变更时自动递增
     * @note 用于跟踪配置数据的版本变化，支持增量保存
     */
    uint32_t m_dwCurSeq;
    
    /**
     * @brief 紧急保存标志
     * @details 标识配置数据是否需要紧急保存
     * @note true表示需要尽快保存，false表示可以延迟保存
     */
    bool m_bIsUrgentNeedSave;
};
