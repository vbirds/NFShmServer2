// -------------------------------------------------------------------------
//    @FileName         :    NFShmTransMng.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmTransMng.h
//    @Desc             :    共享内存事务管理器头文件，提供事务管理和生命周期控制。
//                          该文件定义了共享内存事务管理器类，提供事务对象的创建和销毁、
//                          事务生命周期管理、事务状态监控、服务器停止检查。
//                          主要功能包括事务对象池管理、事务执行调度、事务完成检查、
//                          服务器状态管理
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFShmStl/NFShmVector.h>

#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFTickByRunIndexOP.h"

#define MAX_TOTAL_TRANS_NUM 500000  ///< 最大事务总数

class NFTransBase;

/**
 * @brief 共享内存事务管理器
 * 
 * 负责管理共享内存中的事务对象，包括：
 * - 事务的创建和销毁
 * - 事务执行调度
 * - 事务完成状态检查
 * - 服务器停止条件检查
 * 
 * 继承自NFObjectGlobalTemplate和NFTickByRunIndexOP
 */
class NFShmTransMng final : public NFObjectGlobalTemplate<NFShmTransMng, EOT_TRANS_MNG, NFObject>, public NFTickByRunIndexOP
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化事务管理器
     */
    NFShmTransMng();

    /**
     * @brief 析构函数
     * 
     * 清理事务管理器资源
     */
    ~NFShmTransMng() override;

    /**
     * @brief 创建初始化
     * 
     * 新创建时的初始化
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 从共享内存恢复时的初始化
     * 
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 创建事务
     * 
     * 根据事务类型创建新的事务对象
     * 
     * @param bTransObjType 事务对象类型
     * @return 事务对象指针
     */
    NFTransBase* CreateTrans(uint32_t bTransObjType);

    /**
     * @brief 获取事务对象
     * 
     * 根据事务ID获取事务对象
     * 
     * @param ullTransId 事务ID
     * @return 事务对象指针
     */
    NFTransBase* GetTransBase(uint64_t ullTransId) const;

    /**
     * @brief 检查所有事务是否完成
     * 
     * @param bAllTransFinished 输出参数，所有事务是否完成
     * @return 检查结果
     */
    int CheckAllTransFinished(bool& bAllTransFinished) const;

    /**
     * @brief 执行心跳
     * 
     * 处理事务的定时任务
     * 
     * @param dwCurRunIndex 当前运行索引
     * @param bIsTickAll 是否处理所有事务
     * @return 处理结果
     */
    int DoTick(uint32_t dwCurRunIndex, bool bIsTickAll = false) override;

    /**
     * @brief 获取总事务数量
     * 
     * @return 总事务数量
     */
    int GetTotalTransNum() const { return m_aiTransObjIdList.size(); }

    /**
     * @brief 停服之前，检查服务器是否满足停服条件
     * 
     * @return 检查结果
     */
    int CheckStopServer() override;

    /**
     * @brief 停服之前，做一些操作，满足停服条件
     * 
     * @return 操作结果
     */
    int StopServer() override;

protected:
    /**
     * @brief 创建事务对象
     * 
     * @param bTransObjType 事务对象类型
     * @return 事务对象指针
     */
    NFTransBase* CreateTransObj(uint32_t bTransObjType) const;

    /**
     * @brief 获取事务对象
     * 
     * @param iIndex 索引
     * @return 事务对象指针
     */
    NFTransBase* GetTransObj(int iIndex) const;

protected:
    NFShmVector<int, MAX_TOTAL_TRANS_NUM> m_aiTransObjIdList;  ///< 事务对象ID列表
};


