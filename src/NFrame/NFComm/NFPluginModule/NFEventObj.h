// -------------------------------------------------------------------------
//    @FileName         :    NFEventObj.h
//    @Author           :    gaoyi
//    @Date             :    22-11-15
//    @Email			:    445267987@qq.com
//    @Module           :    NFEventObj
//    @Description      :    事件驱动系统的核心对象和接口定义
//                           提供发布-订阅模式的事件处理机制和跨服务器事件广播
//
// -------------------------------------------------------------------------

#pragma once

#include <string>
#include <stdint.h>
#include "NFComm/NFCore/NFPlatform.h"
#include "google/protobuf/message.h"
#include "NFBaseObj.h"
#include "NFEventTemplate.h"

class NFIPluginManager;

/**
 * @brief 事件处理宏 - 带日志输出版本
 * @param serverType 服务器类型
 * @param nEventID 事件ID
 * @param bySrcType 事件源类型
 * @param nSrcID 事件源ID
 * @param pMessage 事件消息指针
 * @param eventMsg 期望的消息类型
 * @param pEventMsg 转换后的消息指针变量名
 * 
 * 这个宏用于事件处理函数中的消息类型检查和转换：
 * 
 * 功能特点：
 * - 自动进行Protocol Buffer消息的类型转换
 * - 提供详细的调试日志输出
 * - 包含完整的错误检查和处理
 * - 适用于开发和调试阶段
 * 
 * 使用场景：
 * - 开发阶段需要详细日志的事件处理
 * - 调试复杂的事件流程
 * - 需要跟踪事件传递过程的场合
 * 
 * 处理流程：
 * 1. 检查消息指针的有效性
 * 2. 尝试动态转换为指定的消息类型
 * 3. 转换失败时输出错误日志并返回-1
 * 4. 转换成功时输出追踪日志
 * 
 * 使用示例：
 * @code
 * int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, 
 *               uint64_t nSrcID, const google::protobuf::Message* pMessage) {
 *     EVENT_PROCESS_WITH_PRINTF(serverType, nEventID, bySrcType, nSrcID, 
 *                               pMessage, UserLoginEvent, pLoginEvent);
 *     // 现在可以安全使用pLoginEvent了
 *     ProcessLogin(pLoginEvent->user_id(), pLoginEvent->login_time());
 *     return 0;
 * }
 * @endcode
 */
#define EVENT_PROCESS_WITH_PRINTF(serverType, nEventID, bySrcType, nSrcID, pMessage, eventMsg, pEventMsg) \
    CHECK_NULL(0, pMessage);\
    const eventMsg* pEventMsg = dynamic_cast<const eventMsg*>(pMessage);\
    if (!pEventMsg)                \
    {                                                    \
        NFLogError(NF_LOG_DEFAULT, 0, "Protobuf dynamic_cast Failed, serverType:{},nEventID:{},bySrcType:{}, nSrcID:{} message:{} messageClass:{}", \
        serverType, nEventID, bySrcType, nSrcID, pMessage->DebugString(), pMessage->GetTypeName());\
        return -1;\
    }\
    else {\
        NFLogTrace(NF_LOG_DEFAULT, 0, "serverType:{},nEventID:{},bySrcType:{}, nSrcID:{} message:{} messageClass:{}", \
        serverType, nEventID, bySrcType, nSrcID, pMessage->DebugString(), pMessage->GetTypeName());\
    }

/**
 * @brief 事件处理宏 - 静默版本
 * @param serverType 服务器类型
 * @param nEventID 事件ID
 * @param bySrcType 事件源类型
 * @param nSrcID 事件源ID
 * @param pMessage 事件消息指针
 * @param eventMsg 期望的消息类型
 * @param pEventMsg 转换后的消息指针变量名
 * 
 * 这个宏提供了与EVENT_PROCESS_WITH_PRINTF相同的功能，但不输出追踪日志：
 * 
 * 功能特点：
 * - 高性能的消息类型检查和转换
 * - 只在错误时输出日志
 * - 适用于生产环境
 * - 减少日志输出的性能开销
 * 
 * 使用场景：
 * - 生产环境的事件处理
 * - 高频率事件的处理
 * - 性能敏感的代码路径
 * - 不需要详细日志的稳定功能
 * 
 * 性能优势：
 * - 避免了成功时的字符串格式化开销
 * - 减少了日志I/O操作
 * - 降低了CPU和内存使用率
 * - 提高了事件处理的吞吐量
 */
#define EVENT_PROCESS_NO_PRINTF(serverType, nEventID, bySrcType, nSrcID, pMessage, eventMsg, pEventMsg) \
    CHECK_NULL(0, pMessage);\
    const eventMsg* pEventMsg = dynamic_cast<const eventMsg*>(pMessage);\
    if (!pEventMsg)                \
    {                                                    \
        NFLogError(NF_LOG_DEFAULT, 0, "Protobuf dynamic_cast Failed, serverType:{},nEventID:{},bySrcType:{}, nSrcID:{} message:{} messageClass:{}", \
        serverType, nEventID, bySrcType, nSrcID, pMessage->DebugString(), pMessage->GetTypeName());\
        return -1;\
    }


/**
 * @brief 事件使用注意事项和最佳实践
 * 
 * 重要提醒：
 * 取消订阅事件时传入的参数一定要和订阅事件时传入的参数一致，
 * 事件内部是以订阅事件传入的参数（包括回调指针）组合为事件key的，
 * 如果取消和订阅时参数不一致，取消订阅就会失败，就会导致有事件残留（包括野指针），
 * 下次该事件发生的时候触发回调就会异常
 * 
 * 事件嵌套限制：
 * 事件嵌套层数不能太多，如果可以，尽量不要使用事件嵌套，主要是为了避免造成死循环，
 * 目前事件最大嵌套层数支持5层
 * 
 * 安全使用指南：
 * 1. 订阅和取消订阅参数必须完全一致
 * 2. 对象销毁前必须调用UnSubscribeAll()
 * 3. 避免深层次的事件嵌套调用
 * 4. 在事件处理中谨慎修改事件订阅关系
 * 5. 使用智能指针管理事件对象的生命周期
 */

/**
 * @brief 事件键值结构体
 * 
 * SEventKey 定义了事件系统中用于唯一标识事件的复合键：
 * 
 * 1. 设计目的：
 *    - 提供事件的唯一标识机制
 *    - 支持多维度的事件分类
 *    - 实现高效的事件路由和查找
 *    - 支持跨服务器的事件处理
 * 
 * 2. 键值组成：
 *    - nServerType: 服务器类型，区分不同服务器的事件
 *    - nEventID: 事件ID，标识具体的事件类型
 *    - bySrcType: 事件源类型，区分玩家、NPC、系统等
 *    - nSrcID: 事件源ID，具体的对象实例ID
 * 
 * 3. 使用场景：
 *    - 事件订阅和取消订阅的键值匹配
 *    - 事件触发时的路由查找
 *    - 事件系统的内部索引和管理
 *    - 跨服务器事件的识别和分发
 * 
 * 4. 哈希优化：
 *    - 实现了高效的哈希函数
 *    - 支持在STL容器中的快速查找
 *    - 确保键值的唯一性和分布均匀性
 */
struct SEventKey
{
    /**
     * @brief 事件源ID
     * 
     * 事件的主要标识，通常是触发事件的对象的唯一ID：
     * - 玩家事件：玩家的角色ID
     * - NPC事件：NPC的实例ID  
     * - 系统事件：系统模块的标识
     * - 全局事件：设置为0表示全局范围
     */
    uint64_t nSrcID;

    /**
     * @brief 事件ID
     * 
     * 具体的事件类型标识：
     * - 由事件定义文件统一分配
     * - 每种事件类型对应唯一的ID
     * - 支持自定义事件的动态分配
     * - 范围通常在1-65535之间
     */
    uint32_t nEventID;

    /**
     * @brief 事件源类型
     * 
     * 用来区别不同类型的事件源：
     * - 玩家类型：PLAYER_TYPE
     * - 怪物类型：MONSTER_TYPE
     * - NPC类型：NPC_TYPE
     * - 系统类型：SYSTEM_TYPE
     * - 可扩展支持更多自定义类型
     */
    uint32_t bySrcType;

    /**
     * @brief 服务器类型
     * 
     * 用来区分AllServer模式下不同服务器的事件：
     * - 游戏服务器：GAME_SERVER
     * - 登录服务器：LOGIN_SERVER
     * - 世界服务器：WORLD_SERVER
     * - 支持分布式部署的事件隔离
     */
    uint32_t nServerType;

    /**
     * @brief 默认构造函数
     * 
     * 初始化所有字段为0，创建一个无效的事件键。
     */
    SEventKey()
    {
        nSrcID = 0;
        nEventID = 0;
        bySrcType = 0;
        nServerType = 0;
    }

    /**
     * @brief 相等比较操作符
     * @param eventKey 要比较的另一个事件键
     * @return 如果所有字段都相等返回true，否则返回false
     * 
     * 用于事件键的精确匹配：
     * - STL容器中的查找操作
     * - 事件订阅和取消订阅的匹配
     * - 事件触发时的路由确定
     */
    bool operator==(const SEventKey &eventKey) const
    {
        return ((nServerType == eventKey.nServerType) &&
                (nEventID == eventKey.nEventID) &&
                (bySrcType == eventKey.bySrcType) &&
                (nSrcID == eventKey.nSrcID));
    }

    /**
     * @brief 转换为字符串表示
     * @return 包含所有字段信息的格式化字符串
     * 
     * 用于调试和日志输出：
     * - 提供人类可读的事件键表示
     * - 便于问题排查和调试
     * - 支持日志记录和监控
     */
    std::string ToString() const
    {
        return NF_FORMAT("nServerType:{} nEventID:{}, nSrcID:{}, bySrcType:{}", nServerType, nEventID, nSrcID, bySrcType);
    }
};

/**
 * @brief 为SEventKey提供哈希函数支持
 * 
 * 这个特化的哈希函数使得SEventKey可以用作STL无序容器的键：
 * 
 * 实现特点：
 * - 使用组合哈希算法确保分布均匀
 * - 避免哈希冲突的发生
 * - 提供O(1)的查找性能
 * - 支持高并发的事件处理
 */
namespace std
{
    template<>
    struct hash<SEventKey>
    {
        /**
         * @brief 计算事件键的哈希值
         * @param eventKey 要计算哈希的事件键
         * @return 哈希值
         * 
         * 使用NFHash::hash_combine函数组合多个字段的哈希：
         * - 确保不同字段组合产生不同哈希值
         * - 提供良好的哈希分布特性
         * - 支持高效的容器操作
         */
        size_t operator()(const SEventKey &eventKey) const
        {
            return NFHash::hash_combine(eventKey.nServerType, eventKey.nEventID, eventKey.bySrcType, eventKey.nSrcID);
        }
    };
}

/**
 * @brief 事件系统基础对象类
 * 
 * NFEventObjBase 提供了事件系统的核心接口：
 * 
 * 1. 设计理念：
 *    - 纯接口设计：定义事件处理的标准契约
 *    - 多态支持：通过虚函数实现不同的事件处理策略
 *    - 生命周期管理：提供完整的订阅生命周期接口
 *    - 扩展性：支持自定义事件类型和处理逻辑
 * 
 * 2. 核心功能：
 *    - 事件接收和处理：OnExecute接口
 *    - 事件发送和广播：Fire系列接口
 *    - 事件订阅管理：Subscribe/UnSubscribe接口
 *    - 跨服务器事件：支持服务器间的事件传播
 * 
 * 3. 使用模式：
 *    - 所有需要使用事件系统的类都应该继承此类
 *    - 实现OnExecute方法处理感兴趣的事件
 *    - 在构造时订阅需要的事件
 *    - 在析构时取消所有订阅
 * 
 * 4. 安全特性：
 *    - 引用计数保护：防止事件处理中的对象销毁
 *    - 嵌套限制：避免无限递归的事件调用
 *    - 异常处理：确保事件处理异常不会影响系统稳定性
 */
class _NFExport NFEventObjBase
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化事件对象的基本状态。
     */
    NFEventObjBase()
    {

    }

    /**
     * @brief 虚析构函数
     * 
     * 确保派生类的正确析构，建议在派生类析构函数中调用UnSubscribeAll()。
     */
    virtual ~NFEventObjBase()
    {

    }

public:
    /**
     * @brief 事件执行接口的内部实现
     * @param skey 事件键值结构
     * @param message 事件消息对象
     * @return 处理结果，0表示成功，非0表示错误
     * 
     * 这是事件系统内部使用的接口，将结构化的事件键转换为标准的参数格式：
     * - 解构事件键为独立参数
     * - 调用标准的OnExecute接口
     * - 保持接口的一致性和兼容性
     * 
     * 注意：用户代码通常不需要直接调用此方法。
     */
    int OnExecuteImple(const SEventKey skey, const google::protobuf::Message& message)
    {
        return OnExecute(skey.nServerType, skey.nEventID, skey.bySrcType, skey.nSrcID, &message);
    }

    /**
     * @brief 事件处理接口（纯虚函数）
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param pMessage 事件消息指针
     * @return 处理结果，0表示成功，非0表示错误
     * 
     * 这是事件系统的核心接口，所有继承此类的对象都必须实现：
     * 
     * 实现要求：
     * - 根据事件类型进行相应的业务处理
     * - 使用EVENT_PROCESS_*宏进行消息类型转换
     * - 返回0表示处理成功，非0表示处理失败
     * - 避免在处理中进行耗时操作
     * 
     * 参数说明：
     * - serverType: 区分不同服务器的事件
     * - nEventID: 具体的事件类型标识
     * - bySrcType: 事件源的类型分类
     * - nSrcID: 触发事件的具体对象ID
     * - pMessage: Protocol Buffer格式的事件数据
     * 
     * 安全考虑：
     * - 事件处理中可能发生其他事件的Fire操作
     * - 可能在处理过程中有其他对象的UnSubscribe操作
     * - 需要注意事件嵌套的深度限制
     * - 异常处理要充分，避免影响其他事件处理
     * 
     * 使用示例：
     * @code
     * virtual int OnExecute(uint32_t serverType, uint32_t nEventID, 
     *                      uint32_t bySrcType, uint64_t nSrcID, 
     *                      const google::protobuf::Message *pMessage) override {
     *     switch(nEventID) {
     *         case EVENT_PLAYER_LOGIN:
     *             EVENT_PROCESS_NO_PRINTF(serverType, nEventID, bySrcType, nSrcID, 
     *                                    pMessage, PlayerLoginEvent, pLoginEvent);
     *             return HandlePlayerLogin(pLoginEvent);
     *         case EVENT_PLAYER_LOGOUT:
     *             // 处理其他事件...
     *             break;
     *     }
     *     return 0;
     * }
     * @endcode
     */
    virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage) = 0;

public:
    /**
     * @brief 触发本地事件执行
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型  
     * @param nSrcID 事件源ID
     * @param message 事件消息对象
     * @return 执行结果，0表示成功
     * 
     * 在本地服务器内触发事件，通知所有订阅者：
     * 
     * 执行流程：
     * 1. 构造事件键值
     * 2. 查找所有订阅该事件的对象
     * 3. 按订阅顺序依次调用OnExecute接口
     * 4. 处理可能的异常和嵌套情况
     * 
     * 潜在风险和解决方案：
     * - 问题1: 在事件处理中删除不同的订阅者可能导致将要执行的事件被删除
     * - 问题2: 在事件处理中删除相同的订阅者，通过引用计数防护不会立即删除
     * - 问题3: 在事件处理中触发其他事件会导致嵌套，系统限制最多5层相同事件，总共20层事件
     * 
     * 使用场景：
     * - 系统内部状态变化的通知
     * - 业务逻辑触发的事件传播
     * - 模块间的解耦通信
     */
    virtual int FireExecute(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message) = 0;
    
    /**
     * @brief 向指定服务器类型广播事件
     * @param nServerType 发送方服务器类型
     * @param nRecvServerType 接收方服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件消息
     * @param self 是否包含自己，默认false
     * @return 执行结果
     * 
     * 向指定类型的服务器广播事件：
     * - 支持跨服务器的事件传播
     * - 可以选择是否包含发送方自己
     * - 适用于服务器集群的状态同步
     */
    virtual int FireBroadcast(NF_SERVER_TYPE nServerType, NF_SERVER_TYPE nRecvServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) = 0;
    
    /**
     * @brief 向指定服务器实例广播事件
     * @param nServerType 发送方服务器类型
     * @param nRecvServerType 接收方服务器类型
     * @param busId 目标服务器的总线ID
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件消息
     * @param self 是否包含自己，默认false
     * @return 执行结果
     * 
     * 向特定的服务器实例发送事件：
     * - 精确控制事件的目标服务器
     * - 支持负载均衡的事件分发
     * - 实现点对点的服务器通信
     */
    virtual int FireBroadcast(NF_SERVER_TYPE nServerType, NF_SERVER_TYPE nRecvServerType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) = 0;
    
    /**
     * @brief 向所有服务器广播事件
     * @param nServerType 发送方服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件消息
     * @param self 是否包含自己，默认false
     * @return 执行结果
     * 
     * 向整个服务器集群广播事件：
     * - 全网范围的事件通知
     * - 适用于全局状态变更
     * - 系统级别的紧急通知
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE nServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) = 0;
    
    /**
     * @brief 向指定总线的所有服务器广播事件
     * @param nServerType 发送方服务器类型
     * @param busId 目标总线ID
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件消息
     * @param self 是否包含自己，默认false
     * @return 执行结果
     * 
     * 向特定总线的所有服务器广播：
     * - 按总线分组的事件广播
     * - 支持分区域的服务器管理
     * - 实现局部范围的事件通知
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE nServerType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) = 0;

    /**
     * @brief 订阅事件
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param desc 事件描述，用于调试和日志
     * @return 订阅成功返回true，失败返回false
     * 
     * 向事件系统注册对特定事件的关注：
     * 
     * 订阅机制：
     * - 基于事件键的精确匹配订阅
     * - 支持通配符模式的订阅（nSrcID=0表示关注所有源）
     * - 自动管理订阅关系的生命周期
     * - 防止重复订阅同一事件
     * 
     * 参数说明：
     * - serverType: 指定关注的服务器类型
     * - nEventID: 具体的事件类型
     * - bySrcType: 事件源的类型过滤
     * - nSrcID: 具体的事件源ID，0表示关注所有
     * - desc: 描述信息，便于调试和监控
     * 
     * 使用注意：
     * - 订阅参数必须与后续的取消订阅参数完全一致
     * - 建议在对象构造时进行订阅
     * - 重复订阅同一事件会被忽略
     * - 订阅过多会影响事件触发的性能
     * 
     * 最佳实践：
     * @code
     * // 在构造函数中订阅感兴趣的事件
     * MyClass::MyClass() {
     *     Subscribe(NF_ST_GAME, EVENT_PLAYER_LOGIN, PLAYER_TYPE, 0, "处理玩家登录");
     *     Subscribe(NF_ST_GAME, EVENT_PLAYER_LOGOUT, PLAYER_TYPE, playerId, "处理特定玩家退出");
     * }
     * @endcode
     */
    virtual bool Subscribe(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const std::string &desc) = 0;

    /**
     * @brief 取消订阅事件
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @return 取消成功返回true，失败返回false
     * 
     * 从事件系统中移除对特定事件的订阅：
     * 
     * 取消机制：
     * - 必须与订阅时的参数完全一致
     * - 支持引用计数保护，避免事件处理中的意外删除
     * - 自动清理相关的内部数据结构
     * - 不存在的订阅会返回false但不会出错
     * 
     * 重要警告：
     * - 参数必须与Subscribe时完全一致，否则取消失败
     * - 取消失败会导致事件残留，可能产生野指针
     * - 建议使用UnSubscribeAll()进行安全的批量取消
     * 
     * 安全使用：
     * @code
     * // 确保参数与订阅时一致
     * Subscribe(NF_ST_GAME, EVENT_PLAYER_LOGIN, PLAYER_TYPE, 0, "desc");
     * // ...
     * UnSubscribe(NF_ST_GAME, EVENT_PLAYER_LOGIN, PLAYER_TYPE, 0); // 参数必须一致
     * @endcode
     */
    virtual bool UnSubscribe(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID) = 0;

    /**
     * @brief 取消所有订阅事件
     * @return 取消成功返回true，失败返回false
     * 
     * 批量取消当前对象的所有事件订阅：
     * 
     * 安全特性：
     * - 自动遍历并清理所有订阅关系
     * - 避免参数不匹配的问题
     * - 确保对象销毁时的资源清理
     * - 支持在事件处理中安全调用
     * 
     * 使用场景：
     * - 对象析构时的资源清理
     * - 模块重载时的状态重置
     * - 异常恢复时的订阅清理
     * 
     * 最佳实践：
     * @code
     * MyClass::~MyClass() {
     *     // 析构时务必取消所有订阅
     *     UnSubscribeAll();
     * }
     * @endcode
     */
    virtual bool UnSubscribeAll() = 0;
};


/**
 * @brief 完整的事件系统对象类
 * 
 * NFEventObj 继承了事件基类和框架基础对象，提供了完整的事件系统实现：
 * 
 * 1. 设计特点：
 *    - 多重继承：同时继承事件接口和框架基础功能
 *    - 完整实现：提供了所有事件接口的具体实现
 *    - 框架集成：与插件管理器和其他系统无缝集成
 *    - 生产就绪：经过充分测试的稳定实现
 * 
 * 2. 核心功能：
 *    - 本地事件处理：高效的本地事件分发机制
 *    - 跨服务器事件：完整的分布式事件广播支持
 *    - 订阅管理：自动化的订阅生命周期管理
 *    - 错误恢复：健壮的异常处理和恢复机制
 * 
 * 3. 使用方式：
 *    - 直接继承：继承此类获得完整的事件能力
 *    - 组合使用：将此类作为成员变量使用
 *    - 模块集成：在插件模块中使用事件功能
 * 
 * 4. 性能特点：
 *    - 高效路由：基于哈希表的快速事件路由
 *    - 内存优化：智能的内存管理和回收机制
 *    - 并发友好：支持多线程环境下的安全使用
 *    - 低延迟：最小化事件传播的开销
 * 
 * 使用示例：
 * @code
 * class GameLogic : public NFEventObj {
 * public:
 *     GameLogic(NFIPluginManager* mgr) : NFEventObj(mgr) {
 *         // 订阅感兴趣的事件
 *         Subscribe(NF_ST_GAME, EVENT_PLAYER_LOGIN, PLAYER_TYPE, 0, "处理玩家登录");
 *     }
 *     
 *     virtual int OnExecute(uint32_t serverType, uint32_t nEventID, 
 *                          uint32_t bySrcType, uint64_t nSrcID, 
 *                          const google::protobuf::Message *pMessage) override {
 *         // 处理事件逻辑
 *         return 0;
 *     }
 * };
 * @endcode
 */
class _NFExport NFEventObj : public NFEventObjBase, public NFBaseObj
{
public:
    /**
     * @brief 构造函数
     * @param pPluginManager 插件管理器指针
     * 
     * 初始化事件对象并注册到框架系统：
     * - 设置插件管理器引用
     * - 初始化事件处理状态
     * - 注册到全局事件管理器
     * - 准备接收和发送事件
     */
    NFEventObj(NFIPluginManager *pPluginManager);

    /**
     * @brief 虚析构函数
     * 
     * 清理事件对象的所有资源：
     * - 自动取消所有事件订阅
     * - 从全局事件管理器注销
     * - 清理相关的内部数据结构
     * - 确保没有内存泄漏
     */
    virtual ~NFEventObj();

public:
    /**
     * @brief 事件处理接口（纯虚函数）
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param pMessage 事件消息指针
     * @return 处理结果，0表示成功，非0表示错误
     * 
     * 继承类必须实现此接口来处理具体的事件逻辑。
     * 具体的实现要求和注意事项请参考基类的文档说明。
     */
    virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message *pMessage) = 0;

public:
    /**
     * @brief 触发本地事件执行
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件消息对象
     * @return 执行结果，0表示成功
     * 
     * 实现本地事件的触发和分发，通过插件管理器调用事件系统。
     */
    virtual int FireExecute(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message) override;
    
    /**
     * @brief 向指定服务器类型广播事件
     * 
     * 具体实现请参考基类接口说明。
     */
    virtual int FireBroadcast(NF_SERVER_TYPE nServerType, NF_SERVER_TYPE nRecvServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;
    
    /**
     * @brief 向指定服务器实例广播事件
     * 
     * 具体实现请参考基类接口说明。
     */
    virtual int FireBroadcast(NF_SERVER_TYPE nServerType, NF_SERVER_TYPE nRecvServerType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;
    
    /**
     * @brief 向所有服务器广播事件
     * 
     * 具体实现请参考基类接口说明。
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE nServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;
    
    /**
     * @brief 向指定总线的所有服务器广播事件
     * 
     * 具体实现请参考基类接口说明。
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE nServerType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;
    
    /**
     * @brief 订阅事件
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param desc 事件描述
     * @return 订阅成功返回true，失败返回false
     * 
     * 通过插件管理器注册事件订阅，具体实现请参考基类接口说明。
     */
    virtual bool Subscribe(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const std::string &desc) override;

    /**
     * @brief 取消订阅事件
     * @param serverType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @return 取消成功返回true，失败返回false
     * 
     * 通过插件管理器取消事件订阅，具体实现请参考基类接口说明。
     */
    virtual bool UnSubscribe(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID) override;

    /**
     * @brief 取消所有订阅事件
     * @return 取消成功返回true，失败返回false
     * 
     * 批量取消所有事件订阅，通过插件管理器执行，具体实现请参考基类接口说明。
     */
    virtual bool UnSubscribeAll() override;
};


