// -------------------------------------------------------------------------
//    @FileName         :    NFMemTransMng.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMemTransMng.h
//    @Desc             :    内存事务管理器头文件，提供事务对象的统一管理和生命周期控制。
//                          该文件定义了NFShmXFrame框架的内存事务管理器，负责事务对象的创建和销毁、
//                          事务生命周期管理、事务状态监控、服务器停止检查等。
//                          主要功能包括事务对象池管理、事务执行调度、事务完成检查、
//                          服务器状态管理、事务对象查找与回收
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFShmStl/NFShmVector.h>

#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFTickByRunIndexOP.h"
#define MAX_TOTAL_TRANS_NUM 500000

class NFTransBase;

/**
 * @brief 内存事务管理器类，提供事务对象的统一管理和生命周期控制
 * 
 * 该类继承自NFObjectGlobalTemplate和NFTickByRunIndexOP，负责事务对象的创建和销毁、
 * 事务生命周期管理、事务状态监控、服务器停止检查等。
 * 提供事务对象池管理、事务执行调度、事务完成检查、
 * 服务器状态管理、事务对象查找与回收等功能。
 */
class NFMemTransMng final : public NFObjectGlobalTemplate<NFMemTransMng, EOT_TRANS_MNG, NFObject>, public NFTickByRunIndexOP
{
public:
    /**
     * @brief 构造函数
     */
    NFMemTransMng();
    
    /**
     * @brief 析构函数
     */
    ~NFMemTransMng() override;

    /**
     * @brief 创建初始化
     * @return 初始化结果
     */
    int CreateInit();
    
    /**
     * @brief 恢复初始化
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 创建事务对象
     * @param bTransObjType 事务对象类型
     * @return 创建的事务对象指针
     */
    NFTransBase* CreateTrans(uint32_t bTransObjType);
    
    /**
     * @brief 根据事务ID获取事务对象
     * @param ullTransId 事务ID
     * @return 事务对象指针
     */
    NFTransBase* GetTransBase(uint64_t ullTransId) const;

    /**
     * @brief 检查所有事务是否完成
     * @param bAllTransFinished 输出参数，表示是否所有事务都完成
     * @return 检查结果
     */
    int CheckAllTransFinished(bool& bAllTransFinished) const;
    
    /**
     * @brief 执行定时器处理
     * @param dwCurRunIndex 当前运行索引
     * @param bIsTickAll 是否处理所有
     * @return 处理结果
     */
    int DoTick(uint32_t dwCurRunIndex, bool bIsTickAll = false) override;

    /**
     * @brief 获取事务总数
     * @return 事务总数
     */
    int GetTotalTransNum() const { return m_aiTransObjIdList.size(); }

    /**
     * @brief 停服之前，检查服务器是否满足停服条件
     * @return 检查结果
     */
    int CheckStopServer() override;

    /**
     * @brief 停服之前，做一些操作，满足停服条件
     * @return 操作结果
     */
    int StopServer() override;

protected:
    /**
     * @brief 创建事务对象（内部方法）
     * @param bTransObjType 事务对象类型
     * @return 创建的事务对象指针
     */
    NFTransBase* CreateTransObj(uint32_t bTransObjType) const;
    
    /**
     * @brief 根据索引获取事务对象（内部方法）
     * @param iIndex 索引
     * @return 事务对象指针
     */
    NFTransBase* GetTransObj(int iIndex) const;

protected:
    std::vector<int> m_aiTransObjIdList; ///< 事务对象ID列表
};


