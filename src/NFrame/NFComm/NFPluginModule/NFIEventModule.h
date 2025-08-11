// -------------------------------------------------------------------------
//    @FileName         :    NFIEventModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFIEventModule
//    @Description      :    事件模块接口定义，提供系统事件的发布订阅机制
//                           支持跨模块、跨服务器的事件通信和处理
//
// -------------------------------------------------------------------------

#pragma once

#include <iostream>
#include "NFIModule.h"

class NFEventObjBase;

/**
 * @brief 事件模块接口类
 * 
 * NFIEventModule 提供了完整的事件系统功能，实现了发布-订阅模式：
 * 
 * 1. 事件发布机制：
 *    - 支持带参数的事件发布
 *    - 自动通知所有订阅者
 *    - 支持跨服务器类型的事件传递
 * 
 * 2. 事件订阅机制：
 *    - 灵活的事件订阅和取消订阅
 *    - 支持基于源ID和源类型的精确订阅
 *    - 批量取消订阅功能
 * 
 * 3. 事件分类：
 *    - 基于服务器类型的事件分类
 *    - 基于事件ID的事件标识
 *    - 基于源类型和源ID的事件过滤
 * 
 * 事件系统特性：
 * - 解耦的模块间通信
 * - 类型安全的参数传递
 * - 高效的事件分发机制
 * - 便于调试的事件跟踪
 * 
 * 使用场景：
 * - 模块间的松耦合通信
 * - 游戏逻辑事件处理
 * - 系统状态变化通知
 * - 跨服务器的事件同步
 */
class NFIEventModule
	: public NFIModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFIEventModule(NFIPluginManager* p) :NFIModule(p)
	{

	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构，清理所有事件订阅关系。
	 */
	virtual ~NFIEventModule()
	{

	}

	/**
	 * @brief 发送事件并执行收到事件的对象的对应函数
	 * @param serverType 服务器类型，指定事件的适用范围
	 * @param nEventID 事件ID，唯一标识事件类型
	 * @param bySrcType 事件源类型（如玩家类型、怪物类型等）
	 * @param nSrcID 事件源ID，一般是玩家或生物的唯一标识
	 * @param message 事件传输的数据，使用protobuf消息格式
	 * @return 执行结果，0表示成功，非0表示失败
	 * 
	 * 向系统中发布一个事件，所有订阅了该事件的对象都会收到通知
	 * 并执行相应的处理函数。事件数据通过protobuf消息传递，保证
	 * 类型安全和跨语言兼容性。
	 * 
	 * 使用示例：
	 * @code
	 * // 发布玩家登录事件
	 * PlayerLoginEvent loginEvent;
	 * loginEvent.set_player_id(playerId);
	 * loginEvent.set_login_time(currentTime);
	 * FireExecute(NF_ST_GAME, EVENT_PLAYER_LOGIN, ENTITY_TYPE_PLAYER, playerId, loginEvent);
	 * @endcode
	 */
	virtual int FireExecute(NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message& message) = 0;

	/**
	 * @brief 订阅事件
	 * @param pSink 事件处理对象，必须继承自NFEventObjBase
	 * @param serverType 服务器类型，指定订阅的事件范围
	 * @param nEventID 事件ID，指定要订阅的事件类型
	 * @param bySrcType 事件源类型，可用于过滤特定类型的事件源
	 * @param nSrcID 事件源ID，可用于订阅特定对象的事件，0表示订阅所有
	 * @param desc 事件描述，用于调试和日志记录
	 * @return 订阅成功返回true，失败返回false
	 * 
	 * 订阅指定条件的事件。当匹配的事件发生时，会调用pSink对象的
	 * OnExecute方法。支持精确订阅（指定源ID）和模糊订阅（源ID为0）。
	 * 
	 * 使用示例：
	 * @code
	 * // 订阅特定玩家的登录事件
	 * Subscribe(this, NF_ST_GAME, EVENT_PLAYER_LOGIN, ENTITY_TYPE_PLAYER, playerId, "监听玩家登录");
	 * 
	 * // 订阅所有玩家的登录事件
	 * Subscribe(this, NF_ST_GAME, EVENT_PLAYER_LOGIN, ENTITY_TYPE_PLAYER, 0, "监听所有玩家登录");
	 * @endcode
	 */
	virtual bool Subscribe(NFEventObjBase* pSink, NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const std::string& desc) = 0;

	/**
	 * @brief 取消订阅事件
	 * @param pSink 事件处理对象
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 事件源类型
	 * @param nSrcID 事件源ID
	 * @return 取消订阅成功返回true，失败返回false
	 * 
	 * 取消之前订阅的特定事件。参数必须与订阅时完全一致才能成功取消。
	 * 
	 * 使用示例：
	 * @code
	 * // 取消订阅特定玩家的登录事件
	 * UnSubscribe(this, NF_ST_GAME, EVENT_PLAYER_LOGIN, ENTITY_TYPE_PLAYER, playerId);
	 * @endcode
	 */
	virtual bool UnSubscribe(NFEventObjBase* pSink, NF_SERVER_TYPE serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID) = 0;

	/**
	 * @brief 取消指定对象的所有订阅事件
	 * @param pSink 事件处理对象
	 * @return 取消订阅成功返回true，失败返回false
	 * 
	 * 一次性取消指定对象订阅的所有事件，通常在对象销毁时调用，
	 * 确保不会产生悬挂的事件订阅关系。
	 * 
	 * 使用示例：
	 * @code
	 * // 在对象析构函数中取消所有订阅
	 * UnSubscribeAll(this);
	 * @endcode
	 */
	virtual bool UnSubscribeAll(NFEventObjBase* pSink) = 0;
};


