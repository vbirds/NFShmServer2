// -------------------------------------------------------------------------
//    @FileName         :    NFISaveDb.h
//    @Author           :    gaoyi
//    @Date             :    23-11-17
//    @Email			:    445267987@qq.com
//    @Module           :    NFISaveDb
//
// -------------------------------------------------------------------------

/**
 * @file NFISaveDb.h
 * @brief 数据库保存接口定义文件
 * @details 定义了NFrame框架中数据库保存操作的接口类NFISaveDb，继承自NFSeqOP序列操作类，
 *          提供了数据持久化到数据库的标准接口。包括保存时间跟踪、脏数据检测、
 *          批量保存控制、保存状态管理等功能。
 * 
 * @author gaoyi
 * @date 23-11-17
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFISaveDb: 数据库保存接口类
 *       - 保存时间戳管理
 *       - 数据变更序列号跟踪
 *       - 保存状态检查接口
 *       - 数据库事务发送接口
 * 
 * @warning 实现该接口时需要注意：
 *          - 正确管理保存时间戳避免重复保存
 *          - 实现合适的脏数据检测逻辑
 *          - 处理保存失败的回滚机制
 *          - 控制保存频率避免数据库压力过大
 */

#pragma once

#include <NFComm/NFObjCommon/NFSeqOP.h>
#include <NFComm/NFObjCommon/NFTransBase.h>
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFObjCommon/NFObjectTemplate.h"
#include "NFComm/NFObjCommon/NFObject.h"

/**
 * @class NFISaveDb
 * @brief 数据库保存接口类
 * @details 继承自NFSeqOP序列操作类，提供了数据持久化到数据库的标准接口。包括保存时间跟踪、
 *          脏数据检测、批量保存控制、保存状态管理等功能。是所有需要数据库保存功能的对象的基类。
 * 
 * @note 核心功能：
 *       - 保存时间戳管理（最后保存时间、当前保存时间）
 *       - 数据变更序列号跟踪（继承自NFSeqOP）
 *       - 保存状态检查和控制
 *       - 批量保存频率控制
 *       - 保存完成回调处理
 * 
 * @warning 实现注意事项：
 *          - 继承类需要实现具体的保存逻辑
 *          - 正确管理保存时间戳避免重复保存
 *          - 实现合适的脏数据检测逻辑
 *          - 处理保存失败的回滚机制
 *          - 控制保存频率避免数据库压力过大
 */
class NFISaveDb : public NFSeqOP
{
public:
    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式，选择调用CreateInit()或ResumeInit()
     */
    NFISaveDb();

    /**
     * @brief 析构函数
     * @details 清理数据库保存接口相关资源
     */
    ~NFISaveDb() override;

    /**
     * @brief 创建初始化函数
     * @details 当对象第一次创建时调用，初始化保存时间戳等状态
     * @return int 初始化结果，0表示成功
     */
    int CreateInit();
    
    /**
     * @brief 恢复初始化函数
     * @details 当对象从共享内存恢复时调用，恢复保存状态
     * @return int 恢复结果，0表示成功
     */
    int ResumeInit();

public:
    /**
     * @brief 获取最后保存数据库的时间
     * @details 返回上次成功保存到数据库的时间戳
     * @return uint64_t 最后保存时间戳（毫秒）
     * @note 用于判断数据保存的时效性和保存间隔
     */
    uint64_t GetLastSaveDbTime() const { return m_lastSaveDbTime; }

    /**
     * @brief 设置最后保存数据库的时间
     * @details 设置上次保存到数据库的时间戳
     * @param saveTime 保存时间戳（毫秒）
     * @note 通常在保存成功后调用，更新最后保存时间
     */
    void SetLastSaveDbTime(uint64_t saveTime) { m_lastSaveDbTime = saveTime; }

    /**
     * @brief 设置当前正在保存的时间
     * @details 设置当前开始保存操作的时间戳
     * @param saveTime 保存开始时间戳（毫秒）
     * @note 用于跟踪保存操作的进行状态，避免重复保存
     */
    void SetCurSavingDbTime(uint64_t saveTime) { m_curSavingDbTime = saveTime; }

    /**
     * @brief 获取当前正在保存的时间
     * @details 返回当前保存操作开始的时间戳
     * @return uint64_t 当前保存开始时间戳（毫秒）
     * @note 用于检查保存操作是否在进行中
     */
    uint64_t GetCurSaveingDbTime() const { return m_curSavingDbTime; }

    /**
     * @brief 检查是否需要保存
     * @details 虚函数，由继承类实现具体的保存需求判断逻辑
     * @return bool true表示需要保存，false表示不需要保存
     * @note 继承类应该实现具体的判断逻辑，如检查数据是否有变更、
     *       是否达到保存间隔等条件
     */
    virtual bool IsNeedSave() const;

    /**
     * @brief 检查是否正在保存
     * @details 虚函数，判断当前是否有保存操作在进行中
     * @return bool true表示正在保存，false表示没有保存操作
     * @note 用于避免并发保存操作，确保数据一致性
     */
    virtual bool IsSavingDb() const;

    /**
     * @brief 发送事务到数据库
     * @details 虚函数，发送数据库保存事务
     * @param iReason 保存原因代码，用于标识保存的触发原因
     * @return int 发送结果，0表示成功，非0表示失败
     * @note 继承类应该实现具体的事务发送逻辑
     */
    virtual int SendTransToDb(int iReason);

    /**
     * @brief 保存数据到数据库
     * @details 虚函数，执行数据库保存操作
     * @param maxTimes 最大保存次数限制
     * @param iReason 保存原因代码
     * @param bForce 是否强制保存，默认为false
     * @return int 保存结果，0表示成功，非0表示失败
     * @note 继承类应该实现具体的保存逻辑，包括数据验证、
     *       事务处理、错误处理等
     */
    virtual int SaveToDb(uint32_t maxTimes, int iReason, bool bForce = false);

    /**
     * @brief 检查是否可以保存到数据库
     * @details 虚函数，检查当前是否满足保存条件
     * @param maxTimes 最大保存次数限制
     * @param bForce 是否强制保存，默认为false
     * @return bool true表示可以保存，false表示不能保存
     * @note 检查条件可能包括：保存频率限制、数据有效性、
     *       系统状态等
     */
    virtual bool CanSaveDb(uint32_t maxTimes, bool bForce = false);

    /**
     * @brief 数据库保存完成回调
     * @details 虚函数，当数据库保存操作完成时调用
     * @param success 保存是否成功
     * @param seq 保存操作的序列号
     * @return int 回调处理结果，0表示成功，非0表示失败
     * @note 继承类可以在此函数中处理保存完成后的逻辑，
     *       如更新状态、清理资源、通知其他模块等
     */
    virtual int OnSaveDb(bool success, uint32_t seq);

    /**
     * @brief 获取所有序列号
     * @details 虚函数，获取当前所有相关的序列号
     * @return uint32_t 序列号值
     * @note 用于跟踪数据的完整性和一致性
     */
    virtual uint32_t GetAllSeq() const;

    /**
     * @brief 清除所有序列号
     * @details 虚函数，清除所有相关的序列号
     * @note 通常在数据重置或初始化时调用
     */
    virtual void ClearAllSeq();

protected:
    /**
     * @brief 最后保存数据库的时间
     * @details 记录上次成功保存到数据库的时间戳（毫秒）
     * @note 用于计算保存间隔，避免过于频繁的保存操作
     */
    uint64_t m_lastSaveDbTime;
    
    /**
     * @brief 当前正在保存的时间
     * @details 记录当前保存操作开始的时间戳（毫秒）
     * @note 用于检测保存操作的进行状态，避免并发保存
     */
    uint64_t m_curSavingDbTime;
    
    /**
     * @brief 所有序列号
     * @details 记录所有相关数据的序列号
     * @note 用于跟踪数据的版本和一致性
     */
    uint64_t m_lastAllSeq;
};