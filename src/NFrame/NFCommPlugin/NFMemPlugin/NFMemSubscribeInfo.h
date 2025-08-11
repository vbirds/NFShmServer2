// -------------------------------------------------------------------------
//    @FileName         :    NFShmSubcribeInfo.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmSubcribeInfo.h
//    @Desc             :    内存订阅信息头文件，提供事件订阅信息管理功能，包括事件订阅信息的创建和初始化、
//                          订阅信息的引用计数管理、订阅信息的生命周期管理。
//                          该文件定义了NFShmXFrame框架的内存订阅信息类，提供订阅信息管理、引用计数控制、
//                          事件键值关联、订阅描述信息等功能。
//                          主要功能包括订阅信息管理、引用计数控制、事件键值关联、订阅描述信息
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
#include "NFMemEventKey.h"

/**
 * @brief 订阅信息索引枚举
 */
enum
{
    NF_SHM_SUBSCRIBEINFO_EVENT_KEY_INDEX_0 = 0, ///< 事件键值索引
    NF_SHM_SUBSCRIBEINFO_SHM_OBJ_INDEX_1 = 1,   ///< 共享内存对象索引
    NF_SHM_SUBSCRIBEINFO_MAX_INDEX_NUM = 2,     ///< 最大索引数量
};

/**
 * @brief 内存订阅信息类，提供事件订阅信息管理功能
 * 
 * 该类继承自NFObjectTemplate和NFMultiListNodeObjWithGlobalId，提供事件订阅信息管理功能，
 * 包括事件订阅信息的创建和初始化、订阅信息的引用计数管理、订阅信息的生命周期管理。
 * 提供订阅信息管理、引用计数控制、事件键值关联、订阅描述信息等功能。
 */
class NFMemSubscribeInfo final : public NFObjectTemplate<NFMemSubscribeInfo, EOT_TYPE_SUBSCRIBEINFO_OBJ, NFObject>, public NFMultiListNodeObjWithGlobalId<NFMemSubscribeInfo, NF_SHM_SUBSCRIBEINFO_MAX_INDEX_NUM>
{
public:
    /**
     * @brief 构造函数
     */
    NFMemSubscribeInfo();

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
     * @brief 增加引用计数
     */
    void Add();

    /**
     * @brief 减少引用计数
     */
    void Sub();

    /**
     * @brief 转换为字符串
     * @return 字符串表示
     */
    std::string ToString() const;

public:
    /**
     * @brief 事件对象指针
     */
    NFObjPtr<NFObject> m_pSink;

    /**
     * @brief 引用次数
     */
    int32_t m_refCount;

    /**
     * @brief 移除标志
     */
    bool m_removeFlag;

    /**
     * @brief 描述信息
     */
    NFShmString<32> m_szDesc;

    /**
     * @brief 事件键值
     */
    NFMemEventKey m_eventKey;

    /**
     * @brief 共享内存对象ID
     */
    int m_shmObjId;
};
