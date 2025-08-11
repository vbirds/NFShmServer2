// -------------------------------------------------------------------------
//    @FileName         :    NFServerSyncDataObjMgr.h
//    @Author           :    gaoyi
//    @Date             :    2025-07-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerSyncDataObjMgr
//    @Desc             :    服务器同步数据对象管理器头文件，提供服务器间数据同步的管理功能。
//                          该文件定义了服务器同步数据对象管理器类，包括同步对象管理、
//                          数据同步请求、同步状态管理、停服检查。
//                          主要功能包括提供服务器间数据同步的管理、支持同步对象管理、
//                          支持数据同步请求和状态管理、提供停服检查功能。
//                          服务器同步数据对象管理器是NFShmXFrame框架的服务器同步组件，负责：
//                          - 服务器间数据同步的管理
//                          - 同步对象管理和状态维护
//                          - 数据同步请求和响应处理
//                          - 同步进度监控和错误处理
//                          - 停服条件检查和准备
//                          - 跨服务器数据一致性保证
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFShmStl/NFShmHashSet.h>
#include <NFComm/NFShmStl/NFShmList.h>
#include "NFComm/NFObjCommon/NFObject.h"

class NFServerSyncDataObj;

/**
 * @brief 服务器同步数据对象管理器类
 * 
 * 该类是服务器间数据同步的管理器，提供了：
 * - 同步对象管理和状态维护
 * - 数据同步请求和响应处理
 * - 同步进度监控和错误处理
 * - 停服条件检查和准备
 * 
 * 主要功能：
 * - 提供服务器间数据同步的管理
 * - 支持同步对象管理和状态维护
 * - 支持数据同步请求和响应处理
 * - 提供同步进度监控和错误处理
 * - 支持停服条件检查和准备
 * - 支持跨服务器数据一致性保证
 * 
 * 使用方式：
 * @code
 * NFServerSyncDataObjMgr* pMgr = NFServerSyncDataObjMgr::Instance();
 * NFServerSyncDataObj* pObj = pMgr->GetObj(objId);
 * pMgr->SyncReq(pObj);
 * @endcode
 */
class NFServerSyncDataObjMgr : public NFObjectGlobalTemplate<NFServerSyncDataObjMgr, EOT_SERVER_SYNC_DATA_OBJ_MGR, NFObject>
{
public:
    /**
     * @brief 构造函数
     */
    NFServerSyncDataObjMgr();

    /**
     * @brief 析构函数
     */
    virtual ~NFServerSyncDataObjMgr();

    /**
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化服务器同步数据对象管理器
     * 
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复服务器同步数据对象管理器状态
     * 
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 定时器回调
     * 
     * 处理定时器事件
     * 
     * @param timeId 定时器ID
     * @param callcount 调用次数
     * @return 处理结果
     */
    int OnTimer(int timeId, int callcount) override;
public:
    /**
     * @brief 获取同步对象
     * 
     * 根据对象ID获取同步数据对象
     * 
     * @param iObjID 对象ID
     * @return 同步数据对象指针
     */
    NFServerSyncDataObj* GetObj(int iObjID);

    /**
     * @brief 同步请求
     * 
     * 发送数据同步请求
     * 
     * @param pObj 同步数据对象
     * @return 请求结果
     */
    int SyncReq(NFServerSyncDataObj* pObj);

    /**
     * @brief 数据同步完成回调（数据包版本）
     * 
     * 当数据同步完成时调用，处理同步结果
     * 
     * @param iObjID 对象ID
     * @param packet 数据包
     * @return 处理结果
     */
    int OnDataSynced(int iObjID, const NFDataPackage& packet);

    /**
     * @brief 数据同步完成回调（消息版本）
     * 
     * 当数据同步完成时调用，处理同步结果
     * 
     * @param iObjID 对象ID
     * @param err_code 错误码
     * @param pData 数据消息
     * @return 处理结果
     */
    int OnDataSynced(int iObjID, int32_t err_code, const google::protobuf::Message* pData);

    /**
     * @brief 心跳处理
     * 
     * 处理同步管理器的定时心跳
     * 
     * @return 处理结果
     */
    int Tick();

    /**
     * @brief 检查所有数据加载完成
     * 
     * 检查所有同步数据是否已经加载完成
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
    int CheckStopServer() override;

    /**
     * @brief 停服处理
     * 
     * 停服之前，做一些操作，满足停服条件
     * 
     * @return 处理结果
     */
    int StopServer() override;
private:
    int m_iLastSyncObjIndex; ///< 最后同步对象索引
    uint32_t m_iLastTickTime; ///< 最后心跳时间
    int m_iTransMngObjID; ///< 事务管理器对象ID
    NFShmList<int, 1024> m_runningObjList; ///< 运行中的对象列表
    NFShmList<int, 1024> m_failedObjList; ///< 失败的对象列表
    NFShmHashSet<int, 1024> m_syncDataList; ///< 同步数据列表
    NFShmHashSet<int, 1024> m_syncDataFinishList; ///< 同步完成数据列表
};
