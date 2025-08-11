// -------------------------------------------------------------------------
//    @FileName         :    NFITimerEventModule.h
//    @Author           :    gaoyi
//    @Date             :    22-11-15
//    @Email			:    445267987@qq.com
//    @Module           :    NFITimerEventModule
//    @Description      :    定时器事件模块接口定义，提供事件发送、订阅和定时器管理功能
//
// -------------------------------------------------------------------------

/**
 * @file NFITimerEventModule.h
 * @brief 定时器事件模块接口定义
 * @details 该文件定义了定时器事件模块的核心接口，集成了事件系统和定时器功能：
 *          - 事件的发送、订阅和取消订阅机制
 *          - 本地和跨服务器的事件广播
 *          - 灵活的定时器管理（一次性、循环、固定时间）
 *          - 线程安全的事件处理架构
 * @author gaoyi
 * @date 22-11-15
 * @version 1.0
 */

#pragma once

#include "NFIModule.h"
#include "NFEventObj.h"
#include "NFTimerObj.h"

/**
 * @class NFITimerEventModule
 * @brief 定时器事件模块接口类
 * @details 该接口类结合了事件系统和定时器功能，提供了完整的异步事件处理解决方案：
 * 
 * **核心功能：**
 * - **事件系统**：支持本地和跨服务器的事件发送与订阅
 * - **定时器管理**：提供多种类型的定时器（一次性、循环、固定时间）
 * - **线程安全**：所有操作都是线程安全的，支持多线程环境
 * - **性能优化**：使用引用计数和迭代保护机制，避免常见的并发问题
 * 
 * **继承关系：**
 * - 继承自NFIModule：获得模块生命周期管理能力
 * - 继承自NFEventObjBase：获得事件对象的基础功能
 * - 继承自NFTimerObjBase：获得定时器对象的基础功能
 * 
 * **设计特点：**
 * - 采用发布-订阅模式，实现松耦合的事件通信
 * - 支持事件嵌套和循环检测，最大迭代深度保护
 * - 提供灵活的定时器配置选项
 * - 支持分布式环境下的事件广播
 * 
 * **使用场景：**
 * - 游戏服务器的业务逻辑事件处理
 * - 定时任务和周期性操作
 * - 跨服务器的状态同步和通知
 * - 异步消息处理和延迟执行
 * 
 * **注意事项：**
 * @warning 在事件处理函数中修改订阅关系可能导致迭代器失效
 * @warning 事件嵌套深度有限制，避免无限循环
 * @see NFEventObjBase 了解事件系统的基础功能
 * @see NFTimerObjBase 了解定时器系统的基础功能
 */
class NFITimerEventModule : public NFIModule, public NFEventObjBase, public NFTimerObjBase
{
public:
    /**
     * @brief 构造函数
     * @param pPluginManager 插件管理器指针
     * 
     * 初始化定时器事件模块，设置插件管理器关联。
     */
    NFITimerEventModule(NFIPluginManager* pPluginManager);

    /**
     * @brief 析构函数
     * 
     * 清理定时器事件模块资源，确保所有事件订阅和定时器都被正确清理。
     */
    virtual ~NFITimerEventModule();

public:
    /**
     * @brief 发送事件并执行收到事件的对象的对应函数
     * @param nServerType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型，玩家类型、怪物类型之类的
     * @param nSrcID 事件源ID，一般都是玩家、生物唯一id
     * @param message 事件传输的数据（Protocol Buffers消息）
     * @return 执行是否成功，0表示成功
     * 
     * **功能说明：**
     * 向本服务器内的所有订阅者发送指定事件，这是事件系统的核心功能。
     * 
     * **重要提醒：**
     * 存在几个可能导致问题的威胁，但不会导致崩溃，可能与预想的不一样：
     * 
     * **问题1：**假设在Fire事件里，相同的key，删除不同的pSink，
     *         可能导致将要执行的事件被删除，这可能与你预想的设计不一样
     * 
     * **问题2：**假设在Fire事件里，相同的key，删除相同的pSink，由于事件系统利用SubscribeInfo的Add,Sub引用计数做了预防，
     *         迭代器不会立马被删除，不会导致std::list迭代器失效，这样删除不会导致问题
     * 
     * **问题3：**假设在Fire事件里，Fire了别的事件，会导致迭代问题，事件系统已经做了预防，相同的事件最多迭代5次，
     *         所有的Fire事件最多迭代20次
     * 
     * @warning 在事件处理过程中修改订阅关系需要特别小心
     * @note 该方法会触发所有匹配条件的事件订阅者
     */
    virtual int FireExecute(NF_SERVER_TYPE nServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message& message) override;

    /**
     * @brief 向指定类型的服务器广播事件
     * @param nServerType 发送方服务器类型
     * @param nRecvServerType 接收方服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件数据
     * @param self 是否包含自己，默认false
     * @return 执行结果，0表示成功
     * 
     * 向指定类型的所有服务器实例广播事件，用于跨服务器的事件通知。
     */
    virtual int FireBroadcast(NF_SERVER_TYPE nServerType, NF_SERVER_TYPE nRecvServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;

    /**
     * @brief 向指定服务器广播事件
     * @param nServerType 发送方服务器类型
     * @param nRecvServerType 接收方服务器类型
     * @param busId 目标服务器总线ID
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件数据
     * @param self 是否包含自己，默认false
     * @return 执行结果，0表示成功
     * 
     * 向指定总线ID的服务器广播事件，支持精确的服务器定位。
     */
    virtual int FireBroadcast(NF_SERVER_TYPE nServerType, NF_SERVER_TYPE nRecvServerType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;

    /**
     * @brief 向所有服务器广播事件
     * @param nServerType 发送方服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件数据
     * @param self 是否包含自己，默认false
     * @return 执行结果，0表示成功
     * 
     * 向集群中的所有服务器广播事件，用于全局事件通知。
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE nServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;

    /**
     * @brief 向所有服务器广播事件（带总线ID）
     * @param nServerType 发送方服务器类型
     * @param busId 总线ID
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型
     * @param nSrcID 事件源ID
     * @param message 事件数据
     * @param self 是否包含自己，默认false
     * @return 执行结果，0表示成功
     * 
     * 向集群中的所有服务器广播事件，包含总线ID信息。
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE nServerType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message, bool self = false) override;

    /**
     * @brief 订阅事件
     * @param nServerType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型，玩家类型、怪物类型之类的
     * @param nSrcID 事件源ID，一般都是玩家、生物唯一id
     * @param desc 事件描述，用于打印、获取信息、查看BUG之类的
     * @return 订阅事件是否成功
     * 
     * **功能说明：**
     * 订阅指定的事件，当事件被触发时，订阅者会收到通知。
     * 
     * **订阅机制：**
     * - 支持基于事件ID、源类型、源ID的精确订阅
     * - 使用引用计数机制，同一订阅可以多次注册
     * - 提供描述信息，便于调试和监控
     * 
     * @note 订阅关系会一直保持，直到显式取消订阅或对象销毁
     * @see UnSubscribe 取消订阅事件
     */
    virtual bool Subscribe(NF_SERVER_TYPE nServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const std::string& desc) override;

    /**
     * @brief 取消订阅事件
     * @param nServerType 服务器类型
     * @param nEventID 事件ID
     * @param bySrcType 事件源类型，玩家类型、怪物类型之类的
     * @param nSrcID 事件源ID，一般都是玩家、生物唯一id
     * @return 取消订阅事件是否成功
     * 
     * **功能说明：**
     * 取消对指定事件的订阅，停止接收该事件的通知。
     * 
     * **取消机制：**
     * - 精确匹配订阅条件进行取消
     * - 使用引用计数，需要调用相同次数才能完全取消
     * - 安全的迭代器处理，避免并发问题
     * 
     * @note 如果多次订阅同一事件，需要调用相同次数的取消订阅
     * @see Subscribe 订阅事件
     */
    virtual bool UnSubscribe(NF_SERVER_TYPE nServerType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID) override;

    /**
     * @brief 取消所有订阅事件
     * @return 取消订阅事件是否成功
     * 
     * **功能说明：**
     * 取消当前对象的所有事件订阅，通常在对象销毁时调用。
     * 
     * **清理机制：**
     * - 一次性清理所有订阅关系
     * - 确保没有遗留的事件订阅
     * - 防止对象销毁后仍收到事件通知
     * 
     * @warning 调用后将无法接收任何事件通知
     * @note 建议在对象析构函数中调用此方法
     */
    virtual bool UnSubscribeAll() override;

public:
    /**
     * @brief 设置定时器
     * @param nTimerID 定时器ID，用于标识和管理定时器
     * @param nInterVal 时间间隔（毫秒），定时器触发的间隔时间
     * @param nCallCount 调用次数，0表示无限循环，>0表示指定次数后停止
     * @return 设置是否成功
     * 
     * **功能说明：**
     * 创建一个定时器，按指定间隔触发回调函数。
     * 
     * **定时器特性：**
     * - 支持一次性和循环定时器
     * - 毫秒级精度的时间控制
     * - 自动管理定时器生命周期
     * - 线程安全的定时器操作
     * 
     * **使用场景：**
     * - 周期性的状态检查和更新
     * - 定时的数据保存和同步
     * - 游戏中的Buff效果和冷却时间
     * - 心跳检测和连接维护
     * 
     * @note 定时器ID应该是唯一的，重复ID会覆盖原有定时器
     * @see KillTimer 删除定时器
     */
    virtual bool SetTimer(uint32_t nTimerID, uint64_t nInterVal, uint32_t nCallCount = 0) override;

    /**
     * @brief 删除定时器
     * @param nTimerID 要删除的定时器ID
     * @return 删除是否成功
     * 
     * **功能说明：**
     * 删除指定ID的定时器，停止其继续触发。
     * 
     * **删除机制：**
     * - 立即停止定时器的触发
     * - 清理定时器相关资源
     * - 安全处理正在执行的定时器回调
     * 
     * @note 删除不存在的定时器ID会返回失败，但不会产生错误
     * @see SetTimer 设置定时器
     */
    virtual bool KillTimer(uint32_t nTimerID) override;

    /**
     * @brief 删除所有定时器
     * @return 删除是否成功
     * 
     * **功能说明：**
     * 删除当前对象的所有定时器，通常在对象销毁时调用。
     * 
     * **清理机制：**
     * - 一次性清理所有定时器
     * - 确保没有遗留的定时器回调
     * - 防止对象销毁后仍有定时器触发
     * 
     * @warning 调用后所有定时器都将停止工作
     * @note 建议在对象析构函数中调用此方法
     * @see KillTimer 删除单个定时器
     */
    virtual bool KillAllTimer() override;

    /**
     * @brief 设置固定时间定时器
     * @param nTimerID 定时器ID
     * @param nStartTime 开始时间（Unix时间戳，秒）
     * @param nInterSec 间隔时间（秒）
     * @param nCallCount 调用次数，0表示无限循环
     * @return 设置是否成功
     * 
     * **功能说明：**
     * 创建一个在固定时间点开始执行的定时器。
     * 
     * **固定时间特性：**
     * - 指定具体的开始时间点
     * - 适用于需要精确时间控制的场景
     * - 支持跨时区的时间设置
     * - 自动计算首次触发的延迟时间
     * 
     * **使用场景：**
     * - 每日、每周的定时任务
     * - 活动开始和结束时间控制
     * - 数据备份和维护窗口
     * - 定时公告和系统通知
     * 
     * @note 如果开始时间已过，定时器会立即开始执行
     * @warning 时间精度为秒级，不支持毫秒级控制
     * @see SetTimer 设置普通定时器
     */
    virtual bool SetFixTimer(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterSec, uint32_t nCallCount = 0) override;
};
