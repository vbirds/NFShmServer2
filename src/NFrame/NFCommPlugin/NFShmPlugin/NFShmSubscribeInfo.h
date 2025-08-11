// -------------------------------------------------------------------------
//    @FileName         :    NFShmSubscribeInfo.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmSubscribeInfo
//    @Desc             :    共享内存订阅信息头文件，提供事件订阅信息管理功能。
//                          该文件定义了共享内存订阅信息类，提供事件订阅信息的创建和初始化、
//                          订阅信息的引用计数管理、订阅信息的生命周期管理。
//                          主要功能包括订阅信息管理、引用计数控制、事件键值关联、
//                          订阅描述信息
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include <NFComm/NFObjCommon/NFTypeDefines.h>
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFObjPtr.h"
#include "NFComm/NFShmStl/NFShmString.h"
#include "NFComm/NFObjCommon/NFNodeList.h"
#include "NFShmEventKey.h"

enum
{
    NF_SHM_SUBSCRIBEINFO_EVENT_KEY_INDEX_0 = 0,  ///< 事件键值索引0
    NF_SHM_SUBSCRIBEINFO_SHM_OBJ_INDEX_1 = 1,    ///< 共享内存对象索引1
    NF_SHM_SUBSCRIBEINFO_MAX_INDEX_NUM = 2,      ///< 最大索引数量
};

/**
 * @brief 事件描述信息
 *
 * 用于管理共享内存中的事件订阅信息
 * 包含订阅对象、引用计数、事件键值等信息
 */
class NFShmSubscribeInfo final : public NFObjectTemplate<NFShmSubscribeInfo, EOT_TYPE_SUBSCRIBEINFO_OBJ, NFObject>, public NFMultiListNodeObjWithGlobalId<NFShmSubscribeInfo, NF_SHM_SUBSCRIBEINFO_MAX_INDEX_NUM>
{
public:
    /**
     * @brief 构造函数
     */
    NFShmSubscribeInfo();

    /**
     * @brief 创建初始化
     *
     * @return 初始化结果
     */
    int CreateInit();

    /**
     * @brief 恢复初始化
     *
     * @return 初始化结果
     */
    int ResumeInit();

    /**
     * @brief 增加引用
     *
     * 增加订阅信息的引用计数
     */
    void Add();

    /**
     * @brief 减少引用
     *
     * 减少订阅信息的引用计数
     */
    void Sub();

    /**
     * @brief 转换为字符串
     *
     * @return 字符串表示
     */
    std::string ToString() const;

public:
    /**
     * @brief 事件对象
     *
     * 指向订阅事件的共享内存对象
     */
    NFObjPtr<NFObject> m_pSink;

    /**
     * @brief 引用次数
     *
     * 当前订阅信息的引用计数
     */
    int32_t m_refCount;

    /**
     * @brief 移除标志
     *
     * 标记是否已移除订阅
     */
    bool m_removeFlag;

    /**
     * @brief 描述信息
     *
     * 订阅事件的描述信息
     */
    NFShmString<32> m_szDesc;

    /**
     * @brief 事件键值
     *
     * 关联的事件键值信息
     */
    NFShmEventKey m_eventKey;

    /**
     * @brief 共享内存对象ID
     *
     * 关联的共享内存对象ID
     */
    int m_shmObjId;
};
