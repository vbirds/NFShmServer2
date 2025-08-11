// -------------------------------------------------------------------------
//    @FileName         :    NFDBObjMgr.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDBObjMgr.h
//    @Desc             :    数据库对象管理器头文件，提供数据库对象的管理功能。
//                          该文件定义了数据库对象管理器类，包括对象管理接口、数据加载接口、
//                          数据保存接口、事务处理接口。
//                          主要功能包括提供数据库对象的管理、支持数据加载和保存、
//                          支持事务处理和状态管理、提供对象生命周期管理。
//                          数据库对象管理器是NFShmXFrame框架的数据库管理核心组件，负责：
//                          - 数据库对象的管理和调度
//                          - 数据加载和保存的协调
//                          - 事务处理和状态管理
//                          - 对象生命周期管理
//                          - 数据库操作的批量处理
//                          - 错误处理和重试机制
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFShmStl/NFShmHashSet.h>
#include <NFComm/NFShmStl/NFShmList.h>

#include "NFComm/NFObjCommon/NFObject.h"

class NFBaseDBObj;
class NFDBObjTrans;

/**
 * @brief 数据库对象管理器类
 * 
 * 该类是数据库对象的管理器，提供了：
 * - 数据库对象的管理和调度
 * - 数据加载和保存的协调
 * - 事务处理和状态管理
 * - 对象生命周期管理
 * - 数据库操作的批量处理
 * 
 * 主要功能：
 * - 管理所有数据库对象
 * - 协调数据加载和保存操作
 * - 处理事务和状态管理
 * - 提供对象生命周期管理
 * - 支持批量数据库操作
 * - 提供错误处理和重试机制
 * 
 * 使用方式：
 * @code
 * // 获取数据库对象
 * NFBaseDBObj* pObj = pDBObjMgr->GetObj(objId);
 * 
 * // 从数据库加载对象
 * int result = pDBObjMgr->LoadFromDB(pObj);
 * 
 * // 保存对象到数据库
 * int result = pDBObjMgr->SaveToDB(pObj);
 * @endcode
 */
class NFDBObjMgr : public NFObjectGlobalTemplate<NFDBObjMgr, EOT_TRANS_DB_OBJ_MGR, NFObject>
{
public:
    /**
     * @brief 构造函数
     */
    NFDBObjMgr();

    /**
     * @brief 析构函数
     */
    virtual ~NFDBObjMgr();

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化数据库对象管理器
     * 
     * @return 初始化结果
     */
    //非继承函数, 不要加virtual
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复数据库对象管理器状态
     * 
     * @return 初始化结果
     */
    //非继承函数, 不要加virtual
    int ResumeInit();

    /**
     * @brief 定时器回调
     * 
     * 处理定时器事件，包括对象调度、状态更新等
     * 
     * @param timeId 定时器ID
     * @param callcount 调用次数
     * @return 处理结果
     */
    //must be virtual
    virtual int OnTimer(int timeId, int callcount) override;
public:
    /**
     * @brief 获取数据库对象
     * 
     * 根据对象ID获取数据库对象
     * 
     * @param iObjID 对象ID
     * @return 数据库对象指针，如果未找到返回nullptr
     */
    NFBaseDBObj* GetObj(int iObjID);

    /**
     * @brief 从数据库加载对象
     * 
     * 从数据库加载指定对象的数据
     * 
     * @param pObj 数据库对象指针
     * @return 加载结果
     */
    int LoadFromDB(NFBaseDBObj* pObj);

    /**
     * @brief 数据加载完成回调
     * 
     * 当数据加载完成时调用，处理加载结果
     * 
     * @param iObjID 对象ID
     * @param err_code 错误代码
     * @param pData 加载的数据
     * @return 处理结果
     */
    int OnDataLoaded(int iObjID, int32_t err_code, const google::protobuf::Message* pData);

    /**
     * @brief 数据插入完成回调
     * 
     * 当数据插入完成时调用，处理插入结果
     * 
     * @param trans 事务对象
     * @param success 是否成功
     * @return 处理结果
     */
    int OnDataInserted(NFDBObjTrans* trans, bool success);

    /**
     * @brief 数据保存完成回调
     * 
     * 当数据保存完成时调用，处理保存结果
     * 
     * @param trans 事务对象
     * @param success 是否成功
     * @return 处理结果
     */
    int OnDataSaved(NFDBObjTrans* trans, bool success);

    /**
     * @brief 保存对象到数据库
     * 
     * 将指定对象保存到数据库
     * 
     * @param pObj 数据库对象指针
     * @return 保存结果
     */
    int SaveToDB(NFBaseDBObj* pObj);

    /**
     * @brief 心跳处理
     * 
     * 处理数据库对象管理器的心跳，包括对象调度、状态更新等
     * 
     * @return 处理结果
     */
    int Tick();

    /**
     * @brief 检查所有数据加载完成
     * 
     * 检查所有数据库对象是否已经加载完成
     * 
     * @return 检查结果
     */
    int CheckWhenAllDataLoaded();
public:
    /**
     * @brief 检查停服条件
     * 
     * 停服之前，检查服务器是否满足停服条件
     * 
     * @return 检查结果
     */
    /*
     * 停服之前，检查服务器是否满足停服条件
     * */
    int CheckStopServer() override;

    /**
     * @brief 停服处理
     * 
     * 停服之前，做一些操作，满足停服条件
     * 
     * @return 处理结果
     */
    /*
     * 停服之前，做一些操作，满足停服条件
     * */
    int StopServer() override;
private:
    int m_iLastSavingObjIndex; ///< 最后保存的对象索引
    uint32_t m_iLastTickTime; ///< 最后心跳时间
    int m_iTransMngObjID; ///< 事务管理器对象ID
    int m_iTimer; ///< 定时器ID
    NFShmList<int, 1024> m_runningObjList; ///< 运行中的对象列表
    NFShmList<int, 1024> m_failedObjList; ///< 失败的对象列表
    NFShmHashSet<int, 1024> m_loadDBList; ///< 正在加载数据库的对象列表
    NFShmHashSet<int, 1024> m_loadDBFinishList; ///< 数据库加载完成的对象列表
};
