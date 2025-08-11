// -------------------------------------------------------------------------
//    @FileName         :    NFIMsgModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    服务器消息模块接口头文件，提供基于消息的通讯接口。
//                          该文件定义了服务器消息模块的接口类，包括消息发送接口、
//                          服务器间通信接口、事务处理接口、数据库操作接口。
//                          主要功能包括提供服务器间消息通信、支持多种服务器类型、
//                          支持事务处理、支持数据库操作、提供RPC服务调用。
//                          服务器消息模块是NFShmXFrame框架的核心通信组件，负责：
//                          - 服务器间消息通信管理
//                          - 跨服务器消息路由
//                          - 事务处理和状态管理
//                          - 数据库操作接口
//                          - RPC服务调用
//                          - 消息队列管理
//                          - 负载均衡支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFError.h"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFComm/NFPluginModule/NFIDynamicModule.h"
#include "NFComm/NFPluginModule/NFStoreProtoCommon.h"
#include "NFServerBindRpcService.h"
#include "NFComm/NFPluginModule/NFProtobufCommon.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include <map>
#include <unordered_map>
#include <list>
#include <string>
#include <map>
#include <functional>
#include <unordered_set>

//#define TEST_SERVER_SEND_MSG
#define TEST_SERVER_SEND_MSG_FRAME_COUNT 1

/**
 * @brief 基于消息的通讯接口类
 * 
 * 该类是服务器消息模块的核心接口，提供了：
 * - 服务器间消息通信功能
 * - 跨服务器消息路由
 * - 事务处理和状态管理
 * - 数据库操作接口
 * - RPC服务调用
 * - 消息队列管理
 * - 负载均衡支持
 * 
 * 支持多种服务器类型：
 * - Master Server (主服务器)
 * - World Server (世界服务器)
 * - Game Server (游戏服务器)
 * - Logic Server (逻辑服务器)
 * - Proxy Server (代理服务器)
 * - Store Server (存储服务器)
 * - Online Server (在线服务器)
 * - SNS Server (社交服务器)
 * 
 * 使用方式：
 * @code
 * // 发送消息到主服务器
 * pServerMessageModule->SendMsgToMasterServer(NF_ST_GAME_SERVER, msgId, data);
 * 
 * // 发送事务到存储服务器
 * pServerMessageModule->SendTransToStoreServer(NF_ST_GAME_SERVER, busId, cmd, tableId, dbname, tableName, data);
 * @endcode
 */
class NFIServerMessageModule : public NFIDynamicModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFIServerMessageModule(NFIPluginManager* p) : NFIDynamicModule(p)
	{
	}

	/**
	 * @brief 析构函数
	 */
	virtual ~NFIServerMessageModule()
	{
	}

public:
	/**
	 * @brief 发送消息到主服务器
	 * 
	 * 向主服务器发送消息，支持Protobuf消息格式
	 * 
	 * @param eSendTyp 发送服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToMasterServer(NF_SERVER_TYPE eSendTyp, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                  uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到主服务器（带模块ID）
	 * 
	 * 向主服务器发送消息，支持指定模块ID
	 * 
	 * @param eSendTyp 发送服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToMasterServer(NF_SERVER_TYPE eSendTyp, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                  uint64_t nParam2 = 0) = 0;

public:
	////////////////////////////send proxy msg to other serer//////////////////////////////////
	/**
	 * @brief 通过Bus ID发送代理消息
	 * 
	 * 向指定Bus ID的服务器发送代理消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 通过Bus ID发送代理消息（带模块ID）
	 * 
	 * 向指定Bus ID的服务器发送代理消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
	                                uint64_t nParam1 = 0, uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 通过Bus ID发送代理消息（原始数据）
	 * 
	 * 向指定Bus ID的服务器发送代理消息，支持原始数据格式
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param msg 消息数据指针
	 * @param nLen 消息数据长度
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const char* msg, uint32_t nLen, uint64_t nParam1 = 0,
	                                uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 通过Bus ID发送代理消息（原始数据，带模块ID）
	 * 
	 * 向指定Bus ID的服务器发送代理消息，支持原始数据格式和模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param msg 消息数据指针
	 * @param nLen 消息数据长度
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const char* msg, uint32_t nLen,
	                                uint64_t nParam1 = 0, uint64_t nParam2 = 0) = 0;

	///////////////////////////other server send msg to proxy server/////////////////////////////
	/**
	 * @brief 发送重定向消息到代理服务器
	 * 
	 * 向指定的代理服务器发送重定向消息，支持指定多个客户端ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param ids 客户端ID集合
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @return 发送结果
	 */
	virtual int SendRedirectMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, const std::unordered_set<uint64_t>& ids, uint32_t nMsgId,
	                                         const google::protobuf::Message& xData) = 0;

	/**
	 * @brief 发送重定向消息到所有代理服务器
	 * 
	 * 向所有代理服务器发送重定向消息，支持指定多个客户端ID
	 * 
	 * @param eType 目标服务器类型
	 * @param ids 客户端ID集合
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @return 发送结果
	 */
	virtual int SendRedirectMsgToAllProxyServer(NF_SERVER_TYPE eType, const std::unordered_set<uint64_t>& ids, uint32_t nMsgId,
	                                            const google::protobuf::Message& xData) = 0;

	/**
	 * @brief 发送重定向消息到所有代理服务器（无客户端ID）
	 * 
	 * 向所有代理服务器发送重定向消息，不指定具体客户端ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @return 发送结果
	 */
	virtual int SendRedirectMsgToAllProxyServer(NF_SERVER_TYPE eType, uint32_t nMsgId,
	                                            const google::protobuf::Message& xData) = 0;

	/**
	 * @brief 发送消息到代理服务器
	 * 
	 * 向指定的代理服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                 uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到代理服务器（带模块ID）
	 * 
	 * 向指定的代理服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
	                                 uint64_t nParam1 = 0, uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到代理服务器（字符串数据）
	 * 
	 * 向指定的代理服务器发送消息，支持字符串数据格式
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据字符串
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const std::string& xData, uint64_t nParam1 = 0,
	                                 uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到代理服务器（字符串数据，带模块ID）
	 * 
	 * 向指定的代理服务器发送消息，支持字符串数据格式和模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据字符串
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const std::string& xData,
	                                 uint64_t nParam1 = 0, uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到代理服务器（原始数据）
	 * 
	 * 向指定的代理服务器发送消息，支持原始数据格式
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param pData 消息数据指针
	 * @param dataLen 消息数据长度
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const char* pData, int dataLen, uint64_t nParam1 = 0,
	                                 uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到代理服务器（原始数据，带模块ID）
	 * 
	 * 向指定的代理服务器发送消息，支持原始数据格式和模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param pData 消息数据指针
	 * @param dataLen 消息数据长度
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const char* pData, int dataLen,
	                                 uint64_t nParam1 = 0, uint64_t nParam2 = 0) = 0;

	///////////////////////////other server send msg to world server/////////////////////////////
	/**
	 * @brief 发送消息到世界服务器
	 * 
	 * 向世界服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToWorldServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                 uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到世界服务器（带模块ID）
	 * 
	 * 向世界服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToWorldServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                 uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送事务到世界服务器
	 * 
	 * 向世界服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @return 发送结果
	 */
	virtual int SendTransToWorldServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0,
	                                   uint32_t rsp_trans_id = 0) = 0;

	///////////////////////////other server send msg to center server/////////////////////////////
	/**
	 * @brief 发送消息到中心服务器
	 * 
	 * 向中心服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                  uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到中心服务器（带模块ID）
	 * 
	 * 向中心服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToCenterServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                  uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送事务到中心服务器
	 * 
	 * 向中心服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @return 发送结果
	 */
	virtual int SendTransToCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0,
	                                    uint32_t rsp_trans_id = 0) = 0;

	///////////////////////////other server send msg to cross center server/////////////////////////////
	/**
	 * @brief 发送消息到跨服中心服务器
	 * 
	 * 向跨服中心服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                       uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到跨服中心服务器（带模块ID）
	 * 
	 * 向跨服中心服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                       uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送事务到跨服中心服务器
	 * 
	 * 向跨服中心服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @return 发送结果
	 */
	virtual int SendTransToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0,
	                                         uint32_t rsp_trans_id = 0) = 0;
	///////////////////////////other server send msg to game server/////////////////////////////
	/**
	 * @brief 发送消息到游戏服务器
	 * 
	 * 向指定的游戏服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到游戏服务器（带模块ID）
	 * 
	 * 向指定的游戏服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
	                                uint64_t nParam1 = 0, uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送事务到游戏服务器
	 * 
	 * 向指定的游戏服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @return 发送结果
	 */
	virtual int SendTransToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0,
	                                  uint32_t rsp_trans_id = 0) = 0;

	///////////////////////////other server send msg to logic server/////////////////////////////
	/**
	 * @brief 发送消息到逻辑服务器
	 * 
	 * 向指定的逻辑服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                 uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到逻辑服务器（带模块ID）
	 * 
	 * 向指定的逻辑服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData,
	                                 uint64_t nParam1 = 0, uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送事务到逻辑服务器
	 * 
	 * 向指定的逻辑服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @return 发送结果
	 */
	virtual int SendTransToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0,
	                                   uint32_t rsp_trans_id = 0) = 0;

	///////////////////////////other server send msg to sns server/////////////////////////////
	/**
	 * @brief 根据SNS类型发送消息到社交服务器
	 * 
	 * 根据SNS服务器类型向指定的社交服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param snsType SNS服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToSnsServerBySnsType(NF_SERVER_TYPE eType, NF_SNS_SERVER_TYPE snsType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                        uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到社交服务器
	 * 
	 * 向社交服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToSnsServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                               uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到社交服务器（带模块ID）
	 * 
	 * 向社交服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToSnsServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                               uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送事务到社交服务器
	 * 
	 * 向社交服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @return 发送结果
	 */
	virtual int SendTransToSnsServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0,
	                                 uint32_t rsp_trans_id = 0) = 0;
	///////////////////////////other server send msg to online server/////////////////////////////
	/**
	 * @brief 发送消息到在线服务器
	 * 
	 * 向在线服务器发送消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToOnlineServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                  uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送消息到在线服务器（带模块ID）
	 * 
	 * 向在线服务器发送消息，支持指定模块ID
	 * 
	 * @param eType 目标服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 发送结果
	 */
	virtual int SendMsgToOnlineServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0,
	                                  uint64_t nParam2 = 0) = 0;

	/**
	 * @brief 发送事务到在线服务器
	 * 
	 * 向在线服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @return 发送结果
	 */
	virtual int SendTransToOnlineServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0,
	                                    uint32_t rsp_trans_id = 0) = 0;

public:
	/**
	 * @brief 发送事务到存储服务器
	 * 
	 * 向存储服务器发送事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令
	 * @param table_id 表ID
	 * @param dbname 数据库名
	 * @param table_name 表名
	 * @param xData 消息数据
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键，默认为0
	 * @param cls_name 类名
	 * @param packet_type 数据包类型
	 * @return 发送结果
	 */
	virtual int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
	                                   const std::string& dbname, const std::string& table_name, const google::protobuf::Message& xData, int trans_id = 0,
	                                   uint32_t seq = 0,
	                                   uint64_t mod_key = 0, const std::string& cls_name = "", uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) = 0;

	/**
	 * @brief 发送事务到存储服务器（批量）
	 * 
	 * 向存储服务器发送批量事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令
	 * @param table_id 表ID
	 * @param dbname 数据库名
	 * @param table_name 表名
	 * @param vecFileds 字段列表
	 * @param vk_list 键值对列表
	 * @param where_addtional_conds 附加条件
	 * @param max_records 最大记录数，默认为100
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键，默认为0
	 * @param cls_name 类名
	 * @param packet_type 数据包类型
	 * @return 发送结果
	 */
	virtual int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
	                                   const std::string& dbname, const std::string& table_name, const std::vector<std::string>& vecFileds,
	                                   const std::vector<NFrame::storesvr_vk>& vk_list,
	                                   const std::string& where_addtional_conds, int max_records = 100, int trans_id = 0, uint32_t seq = 0,
	                                   uint64_t mod_key = 0, const std::string& cls_name = "", uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) = 0;

	/**
	 * @brief 发送事务到存储服务器（单条）
	 * 
	 * 向存储服务器发送单条事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令
	 * @param table_id 表ID
	 * @param dbname 数据库名
	 * @param table_name 表名
	 * @param xData 消息数据
	 * @param vk_list 键值对列表
	 * @param where_addtional_conds 附加条件
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键，默认为0
	 * @param cls_name 类名
	 * @param packet_type 数据包类型
	 * @return 发送结果
	 */
	virtual int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
	                                   const std::string& dbname, const std::string& table_name, const google::protobuf::Message& xData,
	                                   const std::vector<NFrame::storesvr_vk>& vk_list,
	                                   const std::string& where_addtional_conds, int trans_id = 0, uint32_t seq = 0,
	                                   uint64_t mod_key = 0, const std::string& cls_name = "", uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) = 0;

	/**
	 * @brief 发送事务到存储服务器（字符串）
	 * 
	 * 向存储服务器发送字符串事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令
	 * @param table_id 表ID
	 * @param dbname 数据库名
	 * @param table_name 表名
	 * @param xData 消息数据字符串
	 * @param max_records 最大记录数，默认为100
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键，默认为0
	 * @param cls_name 类名
	 * @param packet_type 数据包类型
	 * @return 发送结果
	 */
	virtual int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
	                                   const std::string& dbname, const std::string& table_name, const std::string& xData, int max_records, int trans_id = 0,
	                                   uint32_t seq = 0,
	                                   uint64_t mod_key = 0, const std::string& cls_name = "", uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) = 0;

	/**
	 * @brief 发送事务到存储服务器（单条，无事务）
	 * 
	 * 向存储服务器发送单条事务消息，不包含事务ID
	 * 
	 * @param eType 目标服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令
	 * @param table_id 表ID
	 * @param dbname 数据库名
	 * @param table_name 表名
	 * @param xData 消息数据
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键，默认为0
	 * @param cls_name 类名
	 * @param packet_type 数据包类型
	 * @return 发送结果
	 */
	virtual int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
	                                   const std::string& dbname, const std::string& table_name, const std::string& xData, int trans_id = 0, uint32_t seq = 0,
	                                   uint64_t mod_key = 0, const std::string& cls_name = "", uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) = 0;

public:
	///////////////////////store server select obj////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 调用StoreServer的selectobj rpc， 查询数据库里的一条数据,  通过protobuf结构data传数据的key, 并通过protobuf结构data存储返回来的结果。这个函数必须在携程里调用数。
	 * @tparam DataType     代表要查询的表的protobuf结构,
	 * @param eType         服务器类型
	 * @param mod_key       用来作为多线程查询的哈希一致性的key,0表示随机
	 * @param data          作为输入存储查询的key, 作为输出存储查询的结果
	 * @param vecFields     要查询的列，不填意味着查询所有的列
	 * @param dstBusId      指定负责查询的storeserver
	 * @param dbname        指定要查询的数据库
	 * @return
	 */
	template <typename DataType>
	int GetRpcSelectObjService(NF_SERVER_TYPE eType, uint64_t mod_key, DataType& data,
	                           const std::vector<std::string>& vecFields = std::vector<std::string>(), uint32_t dstBusId = 0, const std::string& tbname = "",
	                           const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_selobj selobj;
		std::string clsname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		std::string tempTbName;
		if (tbname.empty())
		{
			tempTbName = clsname;
		}
		else
		{
			tempTbName = tbname;
		}

		CHECK_EXPR(!tempTbName.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_selectobj(selobj, tempDBName, tempTbName, mod_key, data, clsname, packageName, vecFields);

		NFrame::storesvr_selobj_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_SELECTOBJ>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                             selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			data.ParsePartialFromString(selobjRes.record());
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				if (iRet != NFrame::ERR_CODE_STORESVR_ERRCODE_SELECT_EMPTY)
				{
					NFLogError(NF_LOG_DEFAULT, 0, "NFrame::E_STORESVR_C2S_SELECTOBJ Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
					           opres.errmsg());
				}
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC选择对象服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器查询对象数据，支持回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @tparam ResponFunc 响应函数类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 数据对象，用于存储查询结果
	 * @param func 回调函数，用于处理操作结果
	 * @param vecFields 要查询的字段列表，默认为空（查询所有字段）
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param tbname 表名，默认为空（使用默认表名）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType, typename ResponFunc>
	int64_t GetRpcSelectObjService(NF_SERVER_TYPE eType, uint64_t mod_key, DataType& data, const ResponFunc& func,
	                               const std::vector<std::string>& vecFields = std::vector<std::string>(), uint32_t dstBusId = 0,
	                               const std::string& tbname = "", const std::string& dbname = "")
	{
		return GetRpcSelectObjServiceInner(eType, mod_key, data, func, &ResponFunc::operator(), vecFields, dstBusId, tbname, dbname);
	}

	/**
	 * @brief 发送选择对象事务
	 * 
	 * 向存储服务器发送选择对象事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键
	 * @param data 数据对象
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param vecFields 字段列表，默认为空
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名
	 * @return
	 */
	virtual int SendSelectObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0,
	                               uint32_t seq = 0,
	                               const std::vector<std::string>& vecFields = std::vector<std::string>(), uint32_t dstBusId = 0,
	                               const std::string& dbname = "") = 0;

private:
	template <class DataType, typename ResponFunc>
	int64_t GetRpcSelectObjServiceInner(NF_SERVER_TYPE eType, uint64_t mod_key, DataType& data, const ResponFunc& responFunc,
	                                    void (ResponFunc::*pf)(int rpcRetCode, DataType& respone) const,
	                                    const std::vector<std::string>& vecFields = std::vector<std::string>(), uint32_t dstBusId = 0,
	                                    const std::string& tbname = "", const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			DataType* pRespone = data.New();
			pRespone->CopyFrom(data);
			int rpcRetCode = GetRpcSelectObjService(eType, mod_key, *pRespone, vecFields,
			                                        dstBusId, tbname, dbname);
			(responFunc.*pf)(rpcRetCode, *pRespone);
			NF_SAFE_DELETE(pRespone);
		});
		return iRet;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	///////////////////////store server select////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC描述存储服务
	 * 
	 * 通过RPC调用获取描述存储服务的数据，支持字段过滤和条件查询
	 * 
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param pDescStoreMessage 描述存储消息指针，用于存储查询结果
	 * @param vecFields 要查询的字段列表，默认为空（查询所有字段）
	 * @param where_addtional_conds 附加查询条件，默认为空
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	int GetRpcDescStoreService(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message* pDescStoreMessage,
	                           const std::vector<std::string>& vecFields = std::vector<std::string>(), const std::string& where_addtional_conds = "",
	                           int max_records = 100, uint32_t dstBusId = 0,
	                           const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_sel sel;
		std::string tbname = NFProtobufCommon::GetDescStoreClsName(*pDescStoreMessage);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(*pDescStoreMessage);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

		std::vector<NFrame::storesvr_vk> vk_list;
		NFStoreProtoCommon::storesvr_selectbycond(sel, tempDBName, tbname, mod_key, vecFields, vk_list, where_addtional_conds, max_records,
		                                          tbname, packageName);

		NFrame::storesvr_sel_res selRes;
		STATIC_ASSERT_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_SELECT, NFrame::storesvr_sel, NFrame::storesvr_sel_res);
		NF_ASSERT_MSG(FindModule<NFICoroutineModule>()->IsInCoroutine(), "Call GetRpcService Must Int the Coroutine");
		NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
		CHECK_EXPR(pConfig, -1, "can't find server config! servertype:{}", GetServerName(eType));

		NFrame::Proto_FramePkg svrPkg;
		svrPkg.set_module_id(NF_MODULE_FRAME);
		svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_SELECT);
		svrPkg.set_msg_data(sel.SerializePartialAsString());
		auto pRpcInfo = svrPkg.mutable_rpc_info();
		pRpcInfo->set_req_rpc_id(FindModule<NFICoroutineModule>()->CurrentTaskId());
		pRpcInfo->set_req_rpc_hash(NFHash::hash<std::string>()(sel.GetTypeName()));
		pRpcInfo->set_rsp_rpc_hash(NFHash::hash<std::string>()(selRes.GetTypeName()));
		pRpcInfo->set_req_server_type(eType);
		pRpcInfo->set_req_bus_id(pConfig->BusId);

		FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, pConfig->BusId, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD,
		                                                svrPkg);

		int iRet = FindModule<NFICoroutineModule>()->SetUserData(&selRes);
		CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));

		do
		{
			iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS);
			CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));
			auto& opres = selRes.opres();
			if (iRet == 0 && opres.err_code() == 0)
			{
				const google::protobuf::Descriptor* pSheetFieldDesc = pDescStoreMessage->GetDescriptor();
				CHECK_EXPR(pSheetFieldDesc, -1, "pSheetFieldDesc == NULL");
				const google::protobuf::Reflection* pSheetReflect = pDescStoreMessage->GetReflection();
				CHECK_EXPR(pSheetReflect, -1, "pSheetFieldDesc == NULL");

				if (pSheetFieldDesc->field_count() > 0)
				{
					/*  比如 message Sheet_GameRoomDesc
					*		{
					*			repeated GameRoomDesc GameRoomDesc_List = 1  [(nanopb).max_count=100];
					*		}
					*		获得上面GameRoomDesc_List信息
					*/
					const google::protobuf::FieldDescriptor* pSheetRepeatedFieldDesc = pSheetFieldDesc->field(0);
					if (pSheetRepeatedFieldDesc->is_repeated() &&
						pSheetRepeatedFieldDesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
					{
						//如果is_repeated 开始处理
						for (int i = 0; i < (int)selRes.record_size(); i++)
						{
							::google::protobuf::Message* pSheetRepeatedMessageObject = pSheetReflect->AddMessage(
								pDescStoreMessage, pSheetRepeatedFieldDesc);
							pSheetRepeatedMessageObject->ParsePartialFromString(selRes.record(i));
						}
					}
				}

				if (selRes.is_lastbatch())
				{
					break;
				}
			}
			else
			{
				if (iRet == 0)
				{
					iRet = opres.err_code();
					NFLogError(NF_LOG_DEFAULT, 0, "GetRpcDescStoreService Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
					           opres.errmsg());
				}
				else
				{
					NFLogError(NF_LOG_DEFAULT, 0, "GetRpcDescStoreService Failed, iRet:{}", GetErrorStr(iRet));
				}

				break;
			}
		}
		while (true);

		FindModule<NFICoroutineModule>()->SetUserData(NULL);

		return iRet;
	}

	/**
	 * @brief 获取RPC选择服务（模板版本）
	 * 
	 * 通过RPC调用获取选择服务的数据，支持字段过滤、键值对查询和条件查询
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 数据对象，用于获取表名和包名
	 * @param respone 响应数据列表，用于存储查询结果
	 * @param vecFields 要查询的字段列表，默认为空（查询所有字段）
	 * @param vk_list 键值对列表，用于条件查询，默认为空
	 * @param where_addtional_conds 附加查询条件，默认为空
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcSelectService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, std::vector<DataType>& respone,
	                        const std::vector<std::string>& vecFields = std::vector<std::string>(),
	                        const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                        const std::string& where_addtional_conds = "",
	                        int max_records = 100, uint32_t dstBusId = 0,
	                        const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_sel sel;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

		NFStoreProtoCommon::storesvr_selectbycond(sel, tempDBName, tbname, mod_key, vecFields, vk_list, where_addtional_conds, max_records,
		                                          tbname, packageName);

		NFrame::storesvr_sel_res selRes;
		STATIC_ASSERT_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_SELECT, NFrame::storesvr_sel, NFrame::storesvr_sel_res);
		NF_ASSERT_MSG(FindModule<NFICoroutineModule>()->IsInCoroutine(), "Call GetRpcService Must Int the Coroutine");
		NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
		CHECK_EXPR(pConfig, -1, "can't find server config! servertype:{}", GetServerName(eType));

		NFrame::Proto_FramePkg svrPkg;
		svrPkg.set_module_id(NF_MODULE_FRAME);
		auto pRpcInfo = svrPkg.mutable_rpc_info();
		svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_SELECT);
		svrPkg.set_msg_data(sel.SerializePartialAsString());
		pRpcInfo->set_req_rpc_id(FindModule<NFICoroutineModule>()->CurrentTaskId());
		pRpcInfo->set_req_rpc_hash(NFHash::hash<std::string>()(sel.GetTypeName()));
		pRpcInfo->set_rsp_rpc_hash(NFHash::hash<std::string>()(selRes.GetTypeName()));
		pRpcInfo->set_req_server_type(eType);
		pRpcInfo->set_req_bus_id(pConfig->BusId);

		FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, pConfig->BusId, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD,
		                                                svrPkg);

		int iRet = FindModule<NFICoroutineModule>()->SetUserData(&selRes);
		CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));

		do
		{
			iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS);
			CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));
			auto& opres = selRes.opres();
			if (iRet == 0 && opres.err_code() == 0)
			{
				for (int i = 0; i < (int)selRes.record_size(); i++)
				{
					DataType result;
					result.ParsePartialFromString(selRes.record(i));
					respone.push_back(result);
				}

				if (selRes.is_lastbatch())
				{
					break;
				}
			}
			else
			{
				if (iRet == 0)
				{
					iRet = opres.err_code();
					NFLogError(NF_LOG_DEFAULT, 0, "NFrame::E_STORESVR_C2S_SELECT Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
					           opres.errmsg());
				}
				else
				{
					NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
				}

				break;
			}
		}
		while (true);

		FindModule<NFICoroutineModule>()->SetUserData(NULL);

		return iRet;
	}

	/**
	 * @brief 获取RPC选择服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器查询数据，支持回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @tparam ResponFunc 响应函数类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 数据对象，用于获取表名和包名
	 * @param func 回调函数，用于处理操作结果
	 * @param vecFields 要查询的字段列表，默认为空（查询所有字段）
	 * @param vk_list 键值对列表，用于条件查询，默认为空
	 * @param where_addtional_conds 附加查询条件，默认为空
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType, typename ResponFunc>
	int64_t GetRpcSelectService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const ResponFunc& func,
	                            const std::vector<std::string>& vecFields = std::vector<std::string>(),
	                            const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                            const std::string& where_addtional_conds = "",
	                            int max_records = 100, uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		return GetRpcSelectServiceInner(eType, mod_key, data, func, &ResponFunc::operator(), vecFields, vk_list, where_addtional_conds, max_records, dstBusId,
		                                dbname);
	}

	/**
	 * @brief 发送选择事务
	 * 
	 * 向存储服务器发送选择事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键
	 * @param data 数据对象
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param vecFields 字段列表，默认为空
	 * @param vk_list 键值对列表，默认为空
	 * @param where_addtional_conds 附加条件，默认为空
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名
	 * @return
	 */
	virtual int SendSelectTrans(NF_SERVER_TYPE eType, uint64_t mod_key, const google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0,
	                            const std::vector<std::string>& vecFields = std::vector<std::string>(), const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                            const std::string& where_addtional_conds = "", int max_records = 100, uint32_t dstBusId = 0, const std::string& dbname = "") = 0;

	/**
	 * @brief 发送选择事务（带私钥）
	 * 
	 * 向存储服务器发送选择事务消息，支持私钥
	 * 
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键
	 * @param data 数据对象
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param vecFields 字段列表，默认为空
	 * @param privateKeys 私钥列表，默认为空
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名
	 * @return
	 */
	virtual int SendSelectTrans(NF_SERVER_TYPE eType, uint64_t mod_key, const google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0,
	                            const std::vector<std::string>& vecFields = std::vector<std::string>(), const std::vector<std::string>& privateKeys = std::vector<std::string>(),
	                            int max_records = 100, uint32_t dstBusId = 0, const std::string& dbname = "") = 0;

private:
	template <class DataType, typename ResponFunc>
	int64_t GetRpcSelectServiceInner(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const ResponFunc& responFunc,
	                                 void (ResponFunc::*pf)(int rpcRetCode, std::vector<DataType>& respone) const,
	                                 const std::vector<std::string>& vecFields = std::vector<std::string>(),
	                                 const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                                 const std::string& where_addtional_conds = "", int max_records = 100, uint32_t dstBusId = 0,
	                                 const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=]()
		{
			std::vector<DataType> respone;
			int rpcRetCode = GetRpcSelectService(eType, mod_key, data, respone, vecFields, vk_list, where_addtional_conds, max_records,
			                                     dstBusId, dbname);

			(responFunc.*pf)(rpcRetCode, respone);
		});
		return iRet;
	}

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server insert////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC插入对象服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器插入对象数据
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要插入的数据对象
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcInsertObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_insertobj selobj;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_insertobj(selobj, tempDBName, tbname, mod_key, data, tbname, packageName);

		NFrame::storesvr_insertobj_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_INSERTOBJ>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                             selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			LOG_TRACE(0, "NFrame::NF_STORESVR_C2S_INSERTOBJ Success");
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_INSERTOBJ Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC插入对象服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器插入对象数据，支持回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要插入的数据对象
	 * @param func 回调函数，用于处理操作结果
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType>
	int64_t GetRpcInsertObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const std::function<void(int)>& func,
	                               uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			int rpcRetCode = GetRpcInsertObjService(eType, mod_key, data, dstBusId, dbname);
			if (func)
			{
				func(rpcRetCode);
			}
		});
		return iRet;
	}

	/**
	 * @brief 发送插入对象事务
	 * 
	 * 向存储服务器发送插入对象事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键
	 * @param data 数据对象
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名
	 * @return
	 */
	virtual int SendInsertObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0, uint32_t seq = 0, uint32_t dstBusId = 0, const std::string& dbname = "") = 0;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server modifyobj////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC修改对象服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器修改对象数据
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要修改的数据对象
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcModifyObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_modobj selobj;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_modifyobj(selobj, tempDBName, tbname, mod_key, data, tbname, packageName);

		NFrame::storesvr_modobj_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_MODIFYOBJ>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                             selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			LOG_TRACE(0, "NFrame::NF_STORESVR_C2S_MODIFYOBJ Success");
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_MODIFYOBJ Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC修改对象服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器修改对象数据，支持回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要修改的数据对象
	 * @param func 回调函数，用于处理操作结果
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType>
	int64_t GetRpcModifyObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const std::function<void(int)>& func,
	                               uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			int rpcRetCode = GetRpcModifyObjService(eType, mod_key, data, dstBusId, dbname);
			if (func)
			{
				func(rpcRetCode);
			}
		});
		return iRet;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server UpdateObj////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC更新对象服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器更新对象数据
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要更新的数据对象
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcUpdateObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_updateobj selobj;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_updateobj(selobj, tempDBName, tbname, mod_key, data, tbname, packageName);

		NFrame::storesvr_updateobj_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_UPDATEOBJ>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                             selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			LOG_TRACE(0, "NFrame::NF_STORESVR_C2S_UPDATEOBJ Success");
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_UPDATEOBJ Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC更新对象服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器更新对象数据，支持回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要更新的数据对象
	 * @param func 回调函数，用于处理操作结果
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType>
	int64_t GetRpcUpdateObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const std::function<void(int)>& func,
	                               uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			int rpcRetCode = GetRpcUpdateObjService(eType, mod_key, data, dstBusId, dbname);
			if (func)
			{
				func(rpcRetCode);
			}
		});
		return iRet;
	}

	/**
	 * @brief 发送修改对象事务
	 * 
	 * 向存储服务器发送修改对象事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键
	 * @param data 数据对象
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名
	 * @return
	 */
	virtual int SendModifyObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0, uint32_t seq = 0, uint32_t dstBusId = 0, const std::string& dbname = "") = 0;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server delete obj////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC删除对象服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器删除对象数据
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要删除的数据对象
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcDeleteObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_delobj selobj;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_deleteobj(selobj, tempDBName, tbname, mod_key, data, tbname, packageName);

		NFrame::storesvr_delobj_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_DELETEOBJ>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                             selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			LOG_TRACE(0, "NFrame::NF_STORESVR_C2S_DELETEOBJ Success");
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_DELETEOBJ Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC删除对象服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器删除对象数据，支持回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要删除的数据对象
	 * @param func 回调函数，用于处理操作结果
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType>
	int64_t GetRpcDeleteObjService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const std::function<void(int)>& func,
	                               uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			int rpcRetCode = GetRpcDeleteObjService(eType, mod_key, data, dstBusId, dbname);
			if (func)
			{
				func(rpcRetCode);
			}
		});
		return iRet;
	}

	/**
	 * @brief 发送删除对象事务
	 * 
	 * 向存储服务器发送删除对象事务消息
	 * 
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键
	 * @param data 数据对象
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名
	 * @return
	 */
	virtual int SendDeleteObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0, uint32_t seq = 0, uint32_t dstBusId = 0, const std::string& dbname = "") = 0;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server delete////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC删除服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器删除数据，支持条件删除
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 数据对象，用于获取表名和包名
	 * @param vk_list 键值对列表，用于条件删除，默认为空
	 * @param where_addtional_conds 附加删除条件，默认为空
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcDeleteService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data,
	                        const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                        const std::string& where_addtional_conds = "",
	                        uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_del selobj;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_deletebycond(selobj, tempDBName, tbname, mod_key, vk_list, where_addtional_conds, tbname, packageName);

		NFrame::storesvr_del_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_DELETE>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                          selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			LOG_TRACE(0, "NFrame::NF_STORESVR_C2S_DELETE Success");
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_DELETE Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC删除服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器删除数据，支持条件删除和回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 数据对象，用于获取表名和包名
	 * @param func 回调函数，用于处理操作结果
	 * @param vk_list 键值对列表，用于条件删除，默认为空
	 * @param where_addtional_conds 附加删除条件，默认为空
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType>
	int64_t GetRpcDeleteService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const std::function<void(int)>& func,
	                            const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                            const std::string& where_addtional_conds = "",
	                            uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			int rpcRetCode = GetRpcDeleteService(eType, mod_key, data, vk_list, where_addtional_conds, dstBusId, dbname);
			if (func)
			{
				func(rpcRetCode);
			}
		});
		return iRet;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server modify////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC修改服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器修改数据，支持条件修改
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要修改的数据对象
	 * @param vk_list 键值对列表，用于条件修改，默认为空
	 * @param where_addtional_conds 附加修改条件，默认为空
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcModifyService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data,
	                        const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                        const std::string& where_addtional_conds = "", uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_mod selobj;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_modifybycond(selobj, tempDBName, tbname, mod_key, data, vk_list, where_addtional_conds, tbname, packageName);

		NFrame::storesvr_mod_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_MODIFY>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                          selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			LOG_TRACE(0, "NFrame::NF_STORESVR_C2S_INSERTOBJ Success");
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_MODIFY Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC修改服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器修改数据，支持条件修改和回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要修改的数据对象
	 * @param func 回调函数，用于处理操作结果
	 * @param vk_list 键值对列表，用于条件修改，默认为空
	 * @param where_addtional_conds 附加修改条件，默认为空
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType>
	int64_t GetRpcModifyService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const std::function<void(int)>& func,
	                            const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                            const std::string& where_addtional_conds = "",
	                            uint32_t
	                            dstBusId = 0,
	                            const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			int rpcRetCode = GetRpcModifyService(eType, mod_key, data, vk_list, where_addtional_conds, dstBusId, dbname);
			if (func)
			{
				func(rpcRetCode);
			}
		});
		return iRet;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server select////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC更新服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器更新数据，支持条件更新
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要更新的数据对象
	 * @param vk_list 键值对列表，用于条件更新，默认为空
	 * @param where_addtional_conds 附加更新条件，默认为空
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcUpdateService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data,
	                        const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                        const std::string& where_addtional_conds = "", uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_update selobj;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");
		NFStoreProtoCommon::storesvr_updatebycond(selobj, tempDBName, tbname, mod_key, data, vk_list, where_addtional_conds, tbname, packageName);

		NFrame::storesvr_update_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_UPDATE>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                          selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			LOG_TRACE(0, "NFrame::NF_STORESVR_C2S_MODINS Success");
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_MODINS Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC更新服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器更新数据，支持条件更新和回调函数
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 要更新的数据对象
	 * @param func 回调函数，用于处理操作结果
	 * @param vk_list 键值对列表，用于条件更新，默认为空
	 * @param where_addtional_conds 附加更新条件，默认为空
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <class DataType>
	int64_t GetRpcUpdateService(NF_SERVER_TYPE eType, uint64_t mod_key, const DataType& data, const std::function<void(int)>& func,
	                            const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                            const std::string& where_addtional_conds = "",
	                            uint32_t
	                            dstBusId = 0,
	                            const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=, &data]()
		{
			int rpcRetCode = GetRpcUpdateService(eType, mod_key, data, vk_list, where_addtional_conds, dstBusId, dbname);
			if (func)
			{
				func(rpcRetCode);
			}
		});
		return iRet;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server update////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC执行服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器执行SQL语句
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param data 数据对象，用于存储执行结果
	 * @param sql SQL语句
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <class DataType>
	int GetRpcExecuteService(NF_SERVER_TYPE eType, uint64_t mod_key, DataType& data, const std::string& sql,
	                         uint32_t dstBusId = 0,
	                         const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		NFrame::storesvr_execute selobj;
		std::string clsname = NFProtobufCommon::GetProtoBaseName(data);
		//std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!clsname.empty(), -1, "no clsname ........");
		NFStoreProtoCommon::storesvr_execute(selobj, tempDBName, clsname, mod_key, sql);
		NFrame::storesvr_execute_res selobjRes;
		int iRet = FindModule<NFIMessageModule>()->GetRpcService<NF_MODULE_FRAME, NFrame::NF_STORESVR_C2S_EXECUTE>(eType, NF_ST_STORE_SERVER, dstBusId, selobj,
		                                                                                                           selobjRes);
		auto& opres = selobjRes.opres();
		if (iRet == 0 && opres.err_code() == 0)
		{
			data.ParsePartialFromString(selobjRes.record());
		}
		else
		{
			if (iRet == 0)
			{
				iRet = opres.err_code();
				NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_EXECUTE Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
				           opres.errmsg());
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
			}
		}
		return iRet;
	}

	/**
	 * @brief 获取RPC执行服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器执行SQL语句，支持回调函数
	 * 
	 * @tparam ResponFunc 响应函数类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param sql SQL语句
	 * @param func 回调函数，用于处理操作结果
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <typename ResponFunc>
	int GetRpcExecuteService(NF_SERVER_TYPE eType, uint64_t mod_key, const std::string& sql,
	                         const ResponFunc& func, uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		return GetRpcExecuteServiceInner(eType, mod_key, sql, func, &ResponFunc::operator(), dstBusId, dbname);
	}

private:
	template <class DataType, typename ResponFunc>
	int64_t GetRpcExecuteServiceInner(NF_SERVER_TYPE eType, uint64_t mod_key, const std::string& sql,
	                                  const ResponFunc& responFunc, void (ResponFunc::*pf)(int rpcRetCode, DataType& respone) const,
	                                  uint32_t dstBusId = 0,
	                                  const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=]()
		{
			DataType respone;
			int rpcRetCode = GetRpcExecuteService(eType, mod_key, respone, sql, dstBusId, dbname);
			(responFunc.*pf)(rpcRetCode, respone);
		});
		return iRet;
	}

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////store server select////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 获取RPC执行多条记录服务（模板版本）
	 * 
	 * 通过RPC调用向存储服务器执行SQL语句，返回多条记录
	 * 
	 * @tparam DataType 数据类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param respone 响应数据列表，用于存储执行结果
	 * @param sql SQL语句
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 操作结果，0表示成功
	 */
	template <typename DataType>
	int GetRpcExecuteMoreService(NF_SERVER_TYPE eType, uint64_t mod_key, std::vector<DataType>& respone, const std::string& sql, int max_records = 100,
	                             uint32_t dstBusId = 0,
	                             const std::string& dbname = "")
	{
		std::string tempDBName = dbname;
		if (dbname.empty())
		{
			NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
			if (pConfig)
			{
				tempDBName = pConfig->DefaultDBName;
			}
		}
		CHECK_EXPR(!tempDBName.empty(), -1, "no dbname ........");

		if (dstBusId == 0)
		{
			auto pDbServer = FindModule<NFIMessageModule>()->GetSuitDbServer(eType, tempDBName, mod_key);
			if (pDbServer)
			{
				dstBusId = pDbServer->mServerInfo.bus_id();
			}
		}

		DataType data;
		NFrame::storesvr_execute_more sel;
		std::string tbname = NFProtobufCommon::GetProtoBaseName(data);
		std::string packageName = NFProtobufCommon::GetProtoPackageName(data);
		CHECK_EXPR(!tbname.empty(), -1, "no tbname ........");

		NFStoreProtoCommon::storesvr_execute_more(sel, tempDBName, tbname, mod_key, sql, max_records, tbname, packageName);
		NFrame::storesvr_execute_more_res selRes;
		STATIC_ASSERT_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_EXECUTE_MORE, NFrame::storesvr_execute_more,
		                               NFrame::storesvr_execute_more_res);
		NF_ASSERT_MSG(FindModule<NFICoroutineModule>()->IsInCoroutine(), "Call GetRpcService Must Int the Coroutine");
		NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(eType);
		CHECK_EXPR(pConfig, -1, "can't find server config! servertype:{}", GetServerName(eType));

		NFrame::Proto_FramePkg svrPkg;
		svrPkg.set_module_id(NF_MODULE_FRAME);
		svrPkg.set_msg_id(NFrame::NF_STORESVR_C2S_EXECUTE_MORE);
		svrPkg.set_msg_data(sel.SerializePartialAsString());
		auto pRpcInfo = svrPkg.mutable_rpc_info();
		pRpcInfo->set_req_rpc_id(FindModule<NFICoroutineModule>()->CurrentTaskId());
		pRpcInfo->set_req_rpc_hash(NFHash::hash<std::string>()(sel.GetTypeName()));
		pRpcInfo->set_rsp_rpc_hash(NFHash::hash<std::string>()(selRes.GetTypeName()));
		pRpcInfo->set_req_server_type(eType);
		pRpcInfo->set_req_bus_id(pConfig->BusId);

		FindModule<NFIMessageModule>()->SendMsgToServer(eType, NF_ST_STORE_SERVER, pConfig->BusId, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD,
		                                                svrPkg);

		int iRet = FindModule<NFICoroutineModule>()->SetUserData(&selRes);
		CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));

		do
		{
			iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS);
			CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));
			auto& opres = selRes.opres();
			if (iRet == 0 && opres.err_code() == 0)
			{
				for (int i = 0; i < selRes.record_size(); i++)
				{
					DataType result;
					result.ParsePartialFromString(selRes.record(i));
					respone.push_back(result);
				}

				if (selRes.is_lastbatch())
				{
					break;
				}
			}
			else
			{
				if (iRet == 0)
				{
					iRet = opres.err_code();
					NFLogError(NF_LOG_DEFAULT, 0, "NFrame::NF_STORESVR_C2S_EXECUTE_MORE Failed, iRet:{} errMsg:{}", GetErrorStr(iRet),
					           opres.errmsg());
				}
				else
				{
					NFLogError(NF_LOG_DEFAULT, 0, "GetRpcService Failed, iRet:{}", GetErrorStr(iRet));
				}

				break;
			}
		}
		while (true);

		FindModule<NFICoroutineModule>()->SetUserData(NULL);

		return iRet;
	}

	/**
	 * @brief 获取RPC执行多条记录服务（回调版本）
	 * 
	 * 通过RPC调用向存储服务器执行SQL语句，返回多条记录，支持回调函数
	 * 
	 * @tparam ResponFunc 响应函数类型模板参数
	 * @param eType 目标服务器类型
	 * @param mod_key 模块键，用于哈希一致性
	 * @param sql SQL语句
	 * @param func 回调函数，用于处理操作结果
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0（自动选择）
	 * @param dbname 数据库名，默认为空（使用默认数据库）
	 * @return 协程ID
	 */
	template <typename ResponFunc>
	int64_t GetRpcExecuteMoreService(NF_SERVER_TYPE eType, uint64_t mod_key, const std::string& sql, const ResponFunc& func,
	                                 int max_records = 100, uint32_t dstBusId = 0, const std::string& dbname = "")
	{
		return GetRpcExecuteMoreServiceInner(eType, mod_key, sql, func, &ResponFunc::operator(), max_records, dstBusId,
		                                     dbname);
	}

private:
	template <class DataType, typename ResponFunc>
	int64_t GetRpcExecuteMoreServiceInner(NF_SERVER_TYPE eType, uint64_t mod_key, const std::string& sql, const ResponFunc& responFunc,
	                                      void (ResponFunc::*pf)(int rpcRetCode, std::vector<DataType>& respone) const,
	                                      int max_records = 100, uint32_t dstBusId = 0,
	                                      const std::string& dbname = "")
	{
		int64_t iRet = FindModule<NFICoroutineModule>()->MakeCoroutine
		([=]()
		{
			std::vector<DataType> respone;
			int rpcRetCode = GetRpcExecuteMoreService(eType, mod_key, respone, sql, max_records,
			                                          dstBusId, dbname);

			(responFunc.*pf)(rpcRetCode, respone);
		});
		return iRet;
	}
};
