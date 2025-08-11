// -------------------------------------------------------------------------
//    @FileName         :    NFServerBindRpcService.h
//    @Author           :    gaoyi
//    @Date             :    23-3-25
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerBindRpcService
//    @Desc             :    服务器绑定RPC服务头文件，提供服务器间RPC通信的绑定定义。
//                          该文件定义了服务器间RPC通信的绑定宏，包括服务器信息查询、
//                          服务器注册、数据同步、存储服务器操作等RPC服务。
//                          主要功能包括提供服务器间RPC通信的绑定定义、支持服务器信息查询、
//                          支持服务器注册和数据同步、提供存储服务器操作接口。
//                          服务器绑定RPC服务是NFShmXFrame框架的RPC通信基础组件，负责：
//                          - 服务器间RPC通信的绑定定义
//                          - 服务器信息查询和注册
//                          - 数据同步和状态更新
//                          - 存储服务器操作接口
//                          - 跨服务器通信协议定义
//                          - RPC服务路由和分发
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIRpcService.h"
#include "NFServerComm/NFServerMessage/ServerCommon.pb.h"
#include "ServerMsg.pb.h"

/**
 * @brief 获取服务器信息请求RPC绑定
 * 
 * 定义获取服务器信息的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFServer::NF_RPC_SERVICE_GET_SERVER_INFO_REQ, NFServer::RpcRequestGetServerInfo, NFrame::ServerInfoReport)

/**
 * @brief 服务器与服务器直接的注册返回RPC
 * 
 * 定义服务器间注册的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_SERVER_TO_SERVER_REGISTER, NFrame::ServerInfoReportList, NFrame::ServerInfoReportListRespne)

/**
 * @brief 服务器数据同步RPC绑定
 * 
 * 定义服务器间数据同步的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_SERVER_TO_SERVER_SYNC_DATA_RPC, NFrame::Proto_ServerSyncDataReq, NFrame::Proto_ServerSyncDataRsp)

/**
 * @brief StoreServer Rpc Service
 * 
 * 存储服务器相关的RPC服务绑定定义
 */

/**
 * @brief 存储服务器选择对象RPC绑定
 * 
 * 定义存储服务器选择对象的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_SELECTOBJ, NFrame::storesvr_selobj, NFrame::storesvr_selobj_res)

/**
 * @brief 存储服务器选择RPC绑定
 * 
 * 定义存储服务器选择的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_SELECT, NFrame::storesvr_sel, NFrame::storesvr_sel_res)

/**
 * @brief 存储服务器插入对象RPC绑定
 * 
 * 定义存储服务器插入对象的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_INSERTOBJ, NFrame::storesvr_insertobj, NFrame::storesvr_insertobj_res)

/**
 * @brief 存储服务器修改对象RPC绑定
 * 
 * 定义存储服务器修改对象的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_MODIFYOBJ, NFrame::storesvr_modobj, NFrame::storesvr_modobj_res)

/**
 * @brief 存储服务器修改RPC绑定
 * 
 * 定义存储服务器修改的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_MODIFY, NFrame::storesvr_mod, NFrame::storesvr_mod_res)

/**
 * @brief 存储服务器更新RPC绑定
 * 
 * 定义存储服务器更新的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_UPDATE, NFrame::storesvr_update, NFrame::storesvr_update_res)

/**
 * @brief 存储服务器更新对象RPC绑定
 * 
 * 定义存储服务器更新对象的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_UPDATEOBJ, NFrame::storesvr_updateobj, NFrame::storesvr_updateobj_res)

/**
 * @brief 存储服务器执行RPC绑定
 * 
 * 定义存储服务器执行的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_EXECUTE, NFrame::storesvr_execute, NFrame::storesvr_execute_res)

/**
 * @brief 存储服务器批量执行RPC绑定
 * 
 * 定义存储服务器批量执行的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_EXECUTE_MORE, NFrame::storesvr_execute_more, NFrame::storesvr_execute_more_res);

/**
 * @brief 存储服务器删除RPC绑定
 * 
 * 定义存储服务器删除的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_DELETE, NFrame::storesvr_del, NFrame::storesvr_del_res)

/**
 * @brief 存储服务器删除对象RPC绑定
 * 
 * 定义存储服务器删除对象的RPC服务绑定
 */
DEFINE_BIND_RPC_SERVICE(NFrame::NF_STORESVR_C2S_DELETEOBJ, NFrame::storesvr_delobj, NFrame::storesvr_delobj_res)
