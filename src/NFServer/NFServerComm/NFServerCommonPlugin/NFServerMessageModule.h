// -------------------------------------------------------------------------
//    @FileName         :    NFServerMessageModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerMessageModule
//    @Desc             :    NFShmXFrame服务器消息模块头文件
//    
//    该文件定义了NFShmXFrame框架中服务器消息处理模块的具体实现类。
//    NFServerMessageModule是NFIServerMessageModule接口的具体实现，
//    负责处理服务器间的消息传递、数据同步、事务处理等核心功能。
//    
//    主要功能包括：
//    - 服务器间消息传递和路由
//    - 跨服务器数据同步
//    - 事务处理和状态管理
//    - 多种消息格式支持（Protobuf、字符串、二进制）
//    - 支持多种服务器类型（Master、World、Center、Game、Logic等）
//    - 代理服务器消息转发
//    - 存储服务器数据操作
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFSingleton.hpp"
#include "NFComm/NFCore/NFPlatform.h"
#include "common/spdlog/fmt/fmt.h"
#include "NFServerComm/NFServerCommon/NFIServerMessageModule.h"

/**
 * @brief 服务器消息模块实现类
 * 
 * NFServerMessageModule是NFIServerMessageModule接口的具体实现，
 * 负责处理NFShmXFrame框架中所有服务器间的消息传递和通信功能。
 * 
 * 该类提供了完整的服务器间通信解决方案，包括：
 * - 向主服务器发送消息
 * - 代理服务器消息传递
 * - 跨服务器消息路由
 * - 事务处理和状态同步
 * - 多种数据格式支持
 * 
 * 支持的主要服务器类型：
 * - Master Server: 主服务器
 * - World Server: 世界服务器
 * - Center Server: 中心服务器
 * - Game Server: 游戏服务器
 * - Logic Server: 逻辑服务器
 * - SNS Server: 社交服务器
 * - Online Server: 在线服务器
 * - Store Server: 存储服务器
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFServerMessageModule : public NFIServerMessageModule
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化服务器消息模块，设置插件管理器
	 * 
	 * @param pPluginManager 插件管理器指针
	 */
	NFServerMessageModule(NFIPluginManager* pPluginManager);

	/**
	 * @brief 析构函数
	 * 
	 * 清理服务器消息模块资源
	 */
	~NFServerMessageModule() override;
public:
	/**
	 * @brief 向主服务器发送消息
	 * 
	 * 通过消息模块向主服务器发送Protobuf消息
	 * 
	 * @param eSendTyp 发送服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToMasterServer(NF_SERVER_TYPE eSendTyp, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向主服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向主服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eSendTyp 发送服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToMasterServer(NF_SERVER_TYPE eSendTyp, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;
public:
	////////////////////////////send proxy msg to other serer//////////////////////////////////
	/**
	 * @brief 通过Bus ID向代理服务器发送消息
	 * 
	 * 通过消息模块向指定Bus ID的代理服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1
	 * @param nParam2 参数2
	 * @return 操作结果，0表示成功
	 */
	int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1, uint64_t nParam2) override;

	/**
	 * @brief 通过Bus ID向代理服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向指定Bus ID的代理服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 通过Bus ID向代理服务器发送二进制消息
	 * 
	 * 通过消息模块向指定Bus ID的代理服务器发送二进制消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nMsgId 消息ID
	 * @param msg 消息数据
	 * @param nLen 消息长度
	 * @param nParam1 参数1
	 * @param nParam2 参数2
	 * @return 操作结果，0表示成功
	 */
	int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const char* msg, uint32_t nLen, uint64_t nParam1, uint64_t nParam2) override;

	/**
	 * @brief 通过Bus ID向代理服务器发送二进制消息（带模块ID）
	 * 
	 * 通过消息模块向指定Bus ID的代理服务器发送二进制消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标Bus ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param msg 消息数据
	 * @param nLen 消息长度
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendProxyMsgByBusId(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const char* msg, uint32_t nLen, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;
public:
	///////////////////////////other server send msg to proxy msg/////////////////////////////
	/**
	 * @brief 向代理服务器发送重定向消息
	 * 
	 * 向指定代理服务器发送重定向消息，支持指定多个目标ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标代理服务器ID
	 * @param ids 目标ID集合
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @return 操作结果，0表示成功
	 */
	int SendRedirectMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, const std::unordered_set<uint64_t>& ids, uint32_t nMsgId, const google::protobuf::Message& xData) override;

	/**
	 * @brief 向所有代理服务器发送重定向消息
	 * 
	 * 向指定服务器类型的所有代理服务器发送重定向消息，支持指定多个目标ID
	 * 
	 * @param eType 服务器类型
	 * @param ids 目标ID集合
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @return 操作结果，0表示成功
	 */
	int SendRedirectMsgToAllProxyServer(NF_SERVER_TYPE eType, const std::unordered_set<uint64_t>& ids, uint32_t nMsgId, const google::protobuf::Message& xData) override;

	/**
	 * @brief 向所有代理服务器发送重定向消息（指定所有）
	 * 
	 * 向指定服务器类型的所有代理服务器发送重定向消息，并设置为所有代理服务器
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @return 操作结果，0表示成功
	 */
	int SendRedirectMsgToAllProxyServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData);

	/**
	 * @brief 向代理服务器发送消息
	 * 
	 * 通过消息模块向代理服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标代理服务器ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向代理服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向代理服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标代理服务器ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向代理服务器发送消息（字符串）
	 * 
	 * 通过消息模块向代理服务器发送字符串消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标代理服务器ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const std::string& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向代理服务器发送消息（带模块ID，字符串）
	 * 
	 * 通过消息模块向代理服务器发送字符串消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标代理服务器ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const std::string& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向代理服务器发送消息（二进制）
	 * 
	 * 通过消息模块向代理服务器发送二进制消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标代理服务器ID
	 * @param nMsgId 消息ID
	 * @param pData 消息数据
	 * @param dataLen 消息长度
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const char* pData, int dataLen, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向代理服务器发送消息（带模块ID，二进制）
	 * 
	 * 通过消息模块向代理服务器发送二进制消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标代理服务器ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param pData 消息数据
	 * @param dataLen 消息长度
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToProxyServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const char* pData, int dataLen, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;
	///////////////////////////other server send msg to world msg/////////////////////////////
	/**
	 * @brief 向世界服务器发送消息
	 * 
	 * 通过消息模块向世界服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToWorldServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向世界服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向世界服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToWorldServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向世界服务器发送事务消息
	 * 
	 * 通过消息模块向世界服务器发送事务消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID
	 * @param rsp_trans_id 响应事务ID
	 * @return 操作结果，0表示成功
	 */
	int SendTransToWorldServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	///////////////////////////other server send msg to center server/////////////////////////////
	/**
	 * @brief 向中心服务器发送消息
	 * 
	 * 通过消息模块向中心服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向中心服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向中心服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToCenterServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向中心服务器发送事务消息
	 * 
	 * 通过消息模块向中心服务器发送事务消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID
	 * @param rsp_trans_id 响应事务ID
	 * @return 操作结果，0表示成功
	 */
	int SendTransToCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	/**
	 * @brief 向跨中心服务器发送消息
	 * 
	 * 通过消息模块向跨中心服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向跨中心服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向跨中心服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向跨中心服务器发送事务消息
	 * 
	 * 通过消息模块向跨中心服务器发送事务消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID
	 * @param rsp_trans_id 响应事务ID
	 * @return 操作结果，0表示成功
	 */
	int SendTransToCrossCenterServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;
	///////////////////////////other server send msg to game msg/////////////////////////////
	/**
	 * @brief 向游戏服务器发送消息
	 * 
	 * 通过消息模块向指定游戏服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标游戏服务器ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向游戏服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向指定游戏服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标游戏服务器ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向游戏服务器发送事务消息
	 * 
	 * 通过消息模块向指定游戏服务器发送事务消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标游戏服务器ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID
	 * @param rsp_trans_id 响应事务ID
	 * @return 操作结果，0表示成功
	 */
	int SendTransToGameServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	///////////////////////////other server send msg to logic server/////////////////////////////
	/**
	 * @brief 向逻辑服务器发送消息
	 * 
	 * 通过消息模块向指定逻辑服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标逻辑服务器ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向逻辑服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向指定逻辑服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标逻辑服务器ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向逻辑服务器发送事务消息
	 * 
	 * 通过消息模块向指定逻辑服务器发送事务消息
	 * 
	 * @param eType 服务器类型
	 * @param nDstId 目标逻辑服务器ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID
	 * @param rsp_trans_id 响应事务ID
	 * @return 操作结果，0表示成功
	 */
	int SendTransToLogicServer(NF_SERVER_TYPE eType, uint32_t nDstId, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	///////////////////////////other server send msg to sns server/////////////////////////////
	/**
	 * @brief 根据SNS类型向SNS服务器发送消息
	 * 
	 * 通过消息模块根据SNS服务器类型向SNS服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param snsType SNS服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToSnsServerBySnsType(NF_SERVER_TYPE eType, NF_SNS_SERVER_TYPE snsType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向SNS服务器发送消息
	 * 
	 * 通过消息模块向SNS服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToSnsServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向SNS服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向SNS服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToSnsServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向SNS服务器发送事务消息
	 * 
	 * 通过消息模块向SNS服务器发送事务消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID
	 * @param rsp_trans_id 响应事务ID
	 * @return 操作结果，0表示成功
	 */
	int SendTransToSnsServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	/**
	 * @brief 向在线服务器发送消息
	 * 
	 * 通过消息模块向在线服务器发送Protobuf消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToOnlineServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向在线服务器发送消息（带模块ID）
	 * 
	 * 通过消息模块向在线服务器发送Protobuf消息，指定模块ID
	 * 
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param nParam1 参数1，默认为0
	 * @param nParam2 参数2，默认为0
	 * @return 操作结果，0表示成功
	 */
	int SendMsgToOnlineServer(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t nParam1 = 0, uint64_t nParam2 = 0) override;

	/**
	 * @brief 向在线服务器发送事务消息
	 * 
	 * 通过消息模块向在线服务器发送事务消息
	 * 
	 * @param eType 服务器类型
	 * @param nMsgId 消息ID
	 * @param xData 消息数据
	 * @param req_trans_id 请求事务ID
	 * @param rsp_trans_id 响应事务ID
	 * @return 操作结果，0表示成功
	 */
	int SendTransToOnlineServer(NF_SERVER_TYPE eType, uint32_t nMsgId, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

public:
	/**
	 * @brief 向存储服务器发送事务消息（字符串数据，带最大记录数）
	 * 
	 * 通过消息模块向存储服务器发送事务消息，使用字符串数据格式
	 * 
	 * @param eType 服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令类型
	 * @param table_id 表ID
	 * @param dbname 数据库名称
	 * @param table_name 表名称
	 * @param xData 数据内容（字符串格式）
	 * @param max_records 最大记录数，默认为100
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键值，默认为0
	 * @param cls_name 类名称，默认为空
	 * @param packet_type 数据包类型，默认为事务类型
	 * @return 操作结果，0表示成功
	 */
	int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
									   const std::string& dbname, const std::string& table_name, const std::string& xData, int max_records = 100,
									   int trans_id = 0, uint32_t seq = 0,
									   uint64_t mod_key = 0, const std::string& cls_name = "",
									   uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) override;

	/**
	 * @brief 向存储服务器发送事务消息（字符串数据）
	 * 
	 * 通过消息模块向存储服务器发送事务消息，使用字符串数据格式
	 * 
	 * @param eType 服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令类型
	 * @param table_id 表ID
	 * @param dbname 数据库名称
	 * @param table_name 表名称
	 * @param xData 数据内容（字符串格式）
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键值，默认为0
	 * @param cls_name 类名称，默认为空
	 * @param packet_type 数据包类型，默认为事务类型
	 * @return 操作结果，0表示成功
	 */
	int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
									   const std::string& dbname, const std::string& table_name, const std::string& xData, int trans_id = 0, uint32_t seq = 0,
									   uint64_t mod_key = 0, const std::string& cls_name = "",
									   uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) override;

	/**
	 * @brief 向存储服务器发送事务消息（Protobuf数据）
	 * 
	 * 通过消息模块向存储服务器发送事务消息，使用Protobuf数据格式
	 * 
	 * @param eType 服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令类型
	 * @param table_id 表ID
	 * @param dbname 数据库名称
	 * @param table_name 表名称
	 * @param xData 数据内容（Protobuf格式）
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键值，默认为0
	 * @param cls_name 类名称，默认为空
	 * @param packet_type 数据包类型，默认为事务类型
	 * @return 操作结果，0表示成功
	 */
	int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
									   const std::string& dbname, const std::string& table_name, const google::protobuf::Message& xData, int trans_id = 0,
									   uint32_t seq = 0,
									   uint64_t mod_key = 0, const std::string& cls_name = "",
									   uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) override;

	/**
	 * @brief 向存储服务器发送事务消息（字段列表和虚拟键）
	 * 
	 * 通过消息模块向存储服务器发送事务消息，使用字段列表和虚拟键
	 * 
	 * @param eType 服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令类型
	 * @param table_id 表ID
	 * @param dbname 数据库名称
	 * @param table_name 表名称
	 * @param vecFileds 字段列表
	 * @param vk_list 虚拟键列表
	 * @param where_addtional_conds 额外条件
	 * @param max_records 最大记录数，默认为100
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键值，默认为0
	 * @param cls_name 类名称，默认为空
	 * @param packet_type 数据包类型，默认为事务类型
	 * @return 操作结果，0表示成功
	 */
	int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
									   const std::string& dbname, const std::string& table_name, const std::vector<std::string>& vecFileds,
									   const std::vector<NFrame::storesvr_vk>& vk_list,
									   const std::string& where_addtional_conds, int max_records = 100, int trans_id = 0, uint32_t seq = 0,
									   uint64_t mod_key = 0, const std::string& cls_name = "",
									   uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) override;

	/**
	 * @brief 向存储服务器发送事务消息（Protobuf数据和虚拟键）
	 * 
	 * 通过消息模块向存储服务器发送事务消息，使用Protobuf数据格式和虚拟键
	 * 
	 * @param eType 服务器类型
	 * @param dstBusId 目标Bus ID
	 * @param cmd 命令类型
	 * @param table_id 表ID
	 * @param dbname 数据库名称
	 * @param table_name 表名称
	 * @param xData 数据内容（Protobuf格式）
	 * @param vk_list 虚拟键列表
	 * @param where_addtional_conds 额外条件
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param mod_key 模块键值，默认为0
	 * @param cls_name 类名称，默认为空
	 * @param packet_type 数据包类型，默认为事务类型
	 * @return 操作结果，0表示成功
	 */
	int SendTransToStoreServer(NF_SERVER_TYPE eType, uint32_t dstBusId, uint32_t cmd, uint32_t table_id,
									   const std::string& dbname, const std::string& table_name, const google::protobuf::Message& xData,
									   const std::vector<NFrame::storesvr_vk>& vk_list,
									   const std::string& where_addtional_conds, int trans_id = 0, uint32_t seq = 0,
									   uint64_t mod_key = 0, const std::string& cls_name = "",
									   uint8_t packet_type = NFrame::E_DISP_TYPE_BY_TRANSACTION) override;
public:
	///////////////////////store server select obj////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief 发送查询事务（虚拟键版本）
	 * 
	 * 向存储服务器发送查询事务，使用虚拟键进行查询
	 * 
	 * @param eType 服务器类型
	 * @param mod_key 模块键值
	 * @param data 查询数据
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param vecFields 字段列表，默认为空
	 * @param vk_list 虚拟键列表，默认为空
	 * @param where_addtional_conds 额外查询条件，默认为空
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名称，默认为空
	 * @return 操作结果，0表示成功
	 */
	int SendSelectTrans(NF_SERVER_TYPE eType, uint64_t mod_key, const google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0,
	                            const std::vector<std::string>& vecFields = std::vector<std::string>(), const std::vector<NFrame::storesvr_vk>& vk_list = std::vector<NFrame::storesvr_vk>(),
	                            const std::string& where_addtional_conds = "", int max_records = 100, uint32_t dstBusId = 0, const std::string& dbname = "") override;

	/**
	 * @brief 发送查询事务（私有键版本）
	 * 
	 * 向存储服务器发送查询事务，使用私有键进行查询
	 * 
	 * @param eType 服务器类型
	 * @param mod_key 模块键值
	 * @param data 查询数据
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param vecFields 字段列表，默认为空
	 * @param privateKeys 私有键列表，默认为空
	 * @param max_records 最大记录数，默认为100
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名称，默认为空
	 * @return 操作结果，0表示成功
	 */
	int SendSelectTrans(NF_SERVER_TYPE eType, uint64_t mod_key, const google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0,
	                            const std::vector<std::string>& vecFields = std::vector<std::string>(), const std::vector<std::string>& privateKeys = std::vector<std::string>(),
	                            int max_records = 100, uint32_t dstBusId = 0, const std::string& dbname = "") override;

	/**
	 * @brief 发送对象查询事务
	 * 
	 * 向存储服务器发送对象查询事务
	 * 
	 * @param eType 服务器类型
	 * @param mod_key 模块键值
	 * @param data 查询数据（引用，用于返回结果）
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param vecFields 字段列表，默认为空
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名称，默认为空
	 * @return 操作结果，0表示成功
	 */
	int SendSelectObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message& data, uint32_t table_id = 0, int trans_id = 0, uint32_t seq = 0,
	                               const std::vector<std::string>& vecFields = std::vector<std::string>(), uint32_t dstBusId = 0,
	                               const std::string& dbname = "") override;

	/**
	 * @brief 发送对象插入事务
	 * 
	 * 向存储服务器发送对象插入事务
	 * 
	 * @param eType 服务器类型
	 * @param mod_key 模块键值
	 * @param data 插入数据
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名称，默认为空
	 * @return 操作结果，0表示成功
	 */
	int SendInsertObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message &data, uint32_t table_id = 0, int trans_id = 0, uint32_t seq = 0, uint32_t dstBusId = 0, const std::string &dbname = "") override;

	/**
	 * @brief 发送对象修改事务
	 * 
	 * 向存储服务器发送对象修改事务
	 * 
	 * @param eType 服务器类型
	 * @param mod_key 模块键值
	 * @param data 修改数据
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名称，默认为空
	 * @return 操作结果，0表示成功
	 */
	int SendModifyObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message &data, uint32_t table_id = 0, int trans_id = 0, uint32_t seq = 0, uint32_t dstBusId = 0, const std::string &dbname = "") override;

	/**
	 * @brief 发送对象删除事务
	 * 
	 * 向存储服务器发送对象删除事务
	 * 
	 * @param eType 服务器类型
	 * @param mod_key 模块键值
	 * @param data 删除数据
	 * @param table_id 表ID，默认为0
	 * @param trans_id 事务ID，默认为0
	 * @param seq 序列号，默认为0
	 * @param dstBusId 目标Bus ID，默认为0
	 * @param dbname 数据库名称，默认为空
	 * @return 操作结果，0表示成功
	 */
	int SendDeleteObjTrans(NF_SERVER_TYPE eType, uint64_t mod_key, google::protobuf::Message &data, uint32_t table_id = 0, int trans_id = 0, uint32_t seq = 0, uint32_t dstBusId = 0, const std::string &dbname = "") override;
};
