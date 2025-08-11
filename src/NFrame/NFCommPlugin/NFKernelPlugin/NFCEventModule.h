// -------------------------------------------------------------------------
//    @FileName         :    NFCEventModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCEventModule
//    @Desc             :    事件模块头文件，提供事件驱动的消息传递和订阅机制。
//                          该文件定义了NFShmXFrame框架的事件模块，提供完整的事件驱动系统，
//                          包括事件发布、事件订阅、事件取消订阅、批量取消订阅等功能。
//                          主要功能包括基于事件ID的多维度事件分类、支持protobuf消息作为事件数据载体、
//                          线程安全的事件处理机制、智能的事件嵌套防护、引用计数管理防止迭代器失效
//    @Description      :    事件模块头文件，提供事件驱动的消息传递和订阅机制
//
// -------------------------------------------------------------------------

#ifndef NFC_EVENT_MODULE_H
#define NFC_EVENT_MODULE_H

#include <iostream>
#include "NFComm/NFPluginModule/NFIEventModule.h"

#include "NFComm/NFPluginModule/NFEventTemplate.h"
#include "NFComm/NFPluginModule/NFEventObj.h"

/**
 * @brief 事件模块使用注意事项
 * 
 * 重要事项：
 * 1. 取消订阅事件时传入的参数一定要和订阅事件时传入的参数一致
 *    事件内部是以订阅事件传入的参数（包括回调指针）组合为事件key的，
 *    如果取消和订阅时参数不一致，取消订阅就会失败，就会导致有事件残留（包括野指针），
 *    下次该事件发生的时候触发回调就会异常
 * 
 * 2. 事件嵌套层数不能太多，如果可以，尽量不要使用事件嵌套，主要是为了避免造成死循环，
 *    目前事件最大嵌套层数支持5层
 * 
 * 潜在问题说明（可能导致问题，但不会导致崩溃，可能与你预想的不一样）：
 * - 问题1：假设我在Fire事件里，相同的key，删除不同的pSink，
 *          可能导致将要执行的事件被删除，这可能与你预想的设计不一样
 * - 问题2：假设我在Fire事件里，相同的key，删除相同的pSink，由于事件系统利用SubscribeInfo的Add,Sub引用计数做了预防，
 *          迭代器不会立马被删除，不会导致std::list迭代器失效，这样删除不会导致问题
 * - 问题3：假设我在Fire事件里，Fire了别的事件，会导致迭代问题，事件系统已经了做了预付，相同的事件，最多迭代5次，
 *          所有的Fire事件最多迭代20次
 */

class NFIKernelModule;

/**
 * @class NFCEventModule
 * @brief 事件模块实现类
 *
 * NFCEventModule提供了完整的事件驱动系统，支持观察者模式的消息传递机制：
 * 
 * 事件系统核心功能：
 * - 事件发布(Fire)：向所有订阅者广播事件
 * - 事件订阅(Subscribe)：注册对特定事件的监听
 * - 事件取消订阅(UnSubscribe)：移除事件监听
 * - 批量取消订阅：一次性移除对象的所有事件监听
 * 
 * 事件模型特点：
 * - 基于事件ID、源类型、源ID的多维度事件分类
 * - 支持protobuf消息作为事件数据载体
 * - 线程安全的事件处理机制
 * - 智能的事件嵌套防护（最大5层嵌套）
 * - 引用计数管理防止迭代器失效
 * 
 * 应用场景：
 * - 游戏内系统间解耦通信
 * - 玩家行为事件通知
 * - 状态变化广播
 * - 模块间异步消息传递
 * - 业务逻辑触发器系统
 * 
 * 安全保障：
 * - 防止事件循环嵌套导致的死循环
 * - 安全的订阅者生命周期管理
 * - 事件处理中的异常隔离
 * - 内存泄漏预防机制
 * 
 * @note 事件系统是框架内模块解耦的核心机制
 * @note 订阅和取消订阅时参数必须完全一致
 * @warning 避免在事件处理中进行深度嵌套的事件触发
 */
class NFCEventModule
	: public NFIEventModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFCEventModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 */
	virtual ~NFCEventModule();

	/**
	 * @brief 关闭前处理
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int BeforeShut() override;
	
	/**
	 * @brief 关闭事件模块
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Shut() override;
	
	/**
	 * @brief 事件模块定时更新
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Tick() override;

public:
	/**
	 * @brief 发送事件并执行收到事件的对象的对应函数
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID，用于标识特定类型的事件
	 * @param bySrcType 事件源类型，如玩家类型、怪物类型等
	 * @param nSrcID 事件源ID，一般是玩家或生物的唯一ID
	 * @param message 事件传输的protobuf数据
	 * @return 返回0表示执行成功，非0表示失败
	 * 
	 * 向所有订阅了指定事件的对象发送事件通知。
	 * 事件会按照订阅顺序依次触发各个处理函数。
	 * 
	 * @note 事件处理过程中如果发生异常，会被隔离以保护其他订阅者
	 * @note 支持事件嵌套，但有最大层数限制（5层）
	 */
	virtual int FireExecute(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message& message) override;

	/**
	 * @brief 订阅事件
	 * @param pSink 事件处理对象，必须继承自NFEventObjBase
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID，要监听的事件类型
	 * @param bySrcType 事件源类型，如玩家类型、怪物类型等
	 * @param nSrcID 事件源ID，一般是玩家或生物的唯一ID
	 * @param desc 事件描述，用于调试和日志记录
	 * @return 返回true表示订阅成功，false表示失败
	 * 
	 * 注册对特定事件的监听。当匹配的事件发生时，
	 * 会调用pSink对象的OnEvent方法。
	 * 
	 * @note 同一个对象可以订阅多个不同的事件
	 * @note 重复订阅相同事件会覆盖之前的订阅
	 * @warning 取消订阅时必须使用完全相同的参数
	 */
	virtual bool Subscribe(NFEventObjBase* pSink, NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const std::string& desc) override;

	/**
	 * @brief 取消订阅事件
	 * @param pSink 事件处理对象
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 事件源类型
	 * @param nSrcID 事件源ID
	 * @return 返回true表示取消订阅成功，false表示未找到对应订阅
	 * 
	 * 移除对特定事件的监听。
	 * 
	 * @warning 参数必须与订阅时完全一致，否则取消订阅会失败
	 * @note 取消订阅失败可能导致野指针，在对象销毁前务必正确取消所有订阅
	 */
	virtual bool UnSubscribe(NFEventObjBase* pSink, NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID) override;

	/**
	 * @brief 取消指定对象的所有订阅事件
	 * @param pSink 事件处理对象
	 * @return 返回true表示取消订阅成功，false表示未找到任何订阅
	 * 
	 * 批量移除某个对象注册的所有事件监听。
	 * 通常在对象销毁时调用，确保不会有野指针残留。
	 * 
	 * @note 这是推荐的清理方式，比逐个取消订阅更安全
	 */
	virtual bool UnSubscribeAll(NFEventObjBase* pSink) override;

private:
	/**
	 * @brief 事件执行中心
	 * 
	 * 基于事件模板的执行中心，负责管理事件的订阅、取消订阅和分发。
	 * 使用SEventKey作为事件的唯一标识。
	 */
	NFEventTemplate<NFEventObjBase, SEventKey> m_ExecuteCenter;
};

#endif

