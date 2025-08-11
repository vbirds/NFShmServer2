// -------------------------------------------------------------------------
//    @FileName         :    NFShmEventMgr.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmEventMgr
//    @Desc             :    共享内存事件管理器头文件，提供事件订阅和发布功能。
//                          该文件定义了共享内存事件管理器类，提供事件的订阅和取消订阅、
//                          事件的发布和执行、事件键值管理、订阅信息管理。
//                          主要功能包括事件订阅管理、事件发布机制、事件执行调度、
//                          订阅信息维护
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include <NFComm/NFPluginModule/NFEventTemplate.h>
#include <NFComm/NFShmStl/NFShmHashMap.h>

#include "NFComm/NFObjCommon/NFNodeList.h"
#include "NFShmEventKey.h"
#include "NFShmSubscribeInfo.h"

#define NF_SHM_EVENT_KEY_MAX_NUM 50000   ///< 共享内存事件键最大数量
#define NF_SHM_OBJ_MAX_EVENT_NUM 100     ///< 共享内存对象最大事件数量

/**
 * @brief 共享内存事件管理器
 * 
 * 负责管理共享内存中的事件订阅和发布
 * 提供高效的事件处理机制
 */
class NFShmEventMgr final : public NFObjectGlobalTemplate<NFShmEventMgr, EOT_TYPE_EVENT_MGR, NFObject>
{
public:
    /**
     * @brief 构造函数
     */
    NFShmEventMgr();

    /**
     * @brief 析构函数
     */
    ~NFShmEventMgr() override;

public:
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
     * @brief 订阅事件
     *
     * @param pSink		订阅对象
     * @param serverType 服务器类型
     * @param eventId	事件ID
     * @param srcType	事件源类型，玩家类型，怪物类型之类的
     * @param srcId		事件源ID，一般都是玩家，生物唯一id
     * @param desc		事件描述，用于打印，获取信息，查看BUG之类的
     * @return			订阅事件是否成功
     */
    int Subscribe(NFObject* pSink, NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const std::string& desc);

    /**
     * @brief 取消订阅事件
     *
     * @param pSink		订阅对象
     * @param serverType 服务器类型
     * @param eventId	事件ID
     * @param srcType	事件源类型，玩家类型，怪物类型之类的
     * @param srcId		事件源ID，一般都是玩家，生物唯一id
     * @return			取消订阅事件是否成功
     */
    int UnSubscribe(const NFObject* pSink, NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId);

    /**
     * @brief 取消pSink所有订阅事件
     *
     * @param pSink		订阅对象
     * @return			取消订阅事件是否成功
     */
    int UnSubscribeAll(const NFObject* pSink);

    /**
     * @brief 取消指定全局ID的所有订阅事件
     *
     * @param globalId 全局ID
     * @return 取消订阅事件是否成功
     */
    int UnSubscribeAll(int globalId);

    /**
     * @brief 发送事件,并执行收到事件的对象的对应函数
     *
     * @param serverType 服务器类型
     * @param eventId		事件ID
     * @param srcType		事件源类型，玩家类型，怪物类型之类的
     * @param srcId			事件源ID，一般都是玩家，生物唯一id
     * @param message	事件传输的数据
     * @return				执行是否成功
     */
    int Fire(NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message);

private:
    /**
     * @brief 删除skey的链表里的pSink
     *
     * @param pLastNode 最后一个节点
     * @return			删除skey的链表里的pSink是否成功
     */
    int DelEventKeyListSubscribeInfo(NFShmSubscribeInfo* pLastNode);

    /**
     * @brief 执行所有订阅事件key的函数
     *
     * @param key			事件合成key，skey.nsrcid可能为0，可能=nEventID
     * @param serverType     服务器类型
     * @param eventId		事件ID
     * @param srcType		事件源类型，玩家类型，怪物类型之类的
     * @param srcId			事件源ID，一般都是玩家，生物唯一id
     * @param message	    事件传输的数据
     * @return				执行是否成功
     */
    int Fire(const NFShmEventKey& key, uint32_t serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message);

private:
    /**
     * @brief 事件键值所有订阅信息映射
     * 
     * 存储所有事件键值对应的订阅信息
     */
    NFShmHashMap<NFShmEventKey, NFNodeObjMultiList<NFShmSubscribeInfo>, NF_SHM_EVENT_KEY_MAX_NUM> m_eventKeyAllSubscribe;

    /**
     * @brief 共享内存对象所有订阅信息映射
     * 
     * 存储所有共享内存对象对应的订阅信息
     */
    NFShmHashMap<int, NFNodeObjMultiList<NFShmSubscribeInfo>, NF_SHM_EVENT_KEY_MAX_NUM> m_shmObjAllSubscribe;

    /**
     * @brief 事件触发层数
     * 
     * 用于控制事件触发的深度
     */
    int32_t m_nFireLayer;
};
