// -------------------------------------------------------------------------
//    @FileName         :    NFCStoreServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCStoreServerModule
//    @Desc             :    NFShmXFrame存储服务器模块接口定义
//                          提供数据库操作和存储管理功能
//                          支持RPC服务，处理数据库查询、插入、修改、删除等操作
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFCore/NFCommMap.hpp"
#include "NFServerComm/NFServerCommon/NFIStoreServerModule.h"

/**
 * @brief 存储服务器模块实现类
 * 
 * NFCStoreServerModule是NFIStoreServerModule接口的具体实现，
 * 负责处理数据库操作和存储服务功能。
 * 
 * 该模块提供以下主要功能：
 * - 数据库查询、插入、修改、删除操作
 * - RPC服务接口，支持远程数据库操作
 * - 数据库连接管理和事务处理
 * - 缓存机制支持
 * - 动态加载Protobuf信息和数据库检查
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCStoreServerModule : public NFIStoreServerModule
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化存储服务器模块，设置插件管理器
	 * 
	 * @param p 插件管理器指针
	 */
	explicit NFCStoreServerModule(NFIPluginManager* p);
	
	/**
	 * @brief 析构函数
	 * 
	 * 清理存储服务器模块资源
	 */
	virtual ~NFCStoreServerModule();

	/**
	 * @brief 模块唤醒
	 * 
	 * 在模块初始化完成后调用，进行必要的初始化工作
	 * 
	 * @return 初始化结果状态码
	 */
	virtual int Awake() override;

	/**
	 * @brief 模块初始化
	 * 
	 * 初始化存储服务器模块，包括数据库连接等
	 * 
	 * @return 初始化结果状态码
	 */
	virtual int Init() override;

	/**
	 * @brief 模块定时更新
	 * 
	 * 每帧调用，处理模块的定时任务
	 * 
	 * @return 处理结果状态码
	 */
	virtual int Tick() override;

	/**
	 * @brief 动态插件处理
	 * 
	 * 处理动态插件的加载和卸载
	 * 
	 * @return 处理结果状态码
	 */
	virtual int OnDynamicPlugin() override;

    /**
     * @brief 重新加载配置
     * 
     * 当配置文件发生变化时，重新加载配置
     * 
     * @return 重载结果状态码
     */
    virtual int OnReloadConfig() override;

    /**
     * @brief 定时器回调
     * 
     * 处理定时器事件
     * 
     * @param nTimerID 定时器ID
     * @return 处理结果状态码
     */
    virtual int OnTimer(uint32_t nTimerID) override;

    /**
     * @brief 处理来自服务器的信息
     * 
     * 处理来自其他服务器的消息请求
     * 
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
    virtual int OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet) override;

    /**
     * @brief 动态加载protobuf信息，并检查数据库，可能的话建立数据库，表格，列
     * 
     * 动态加载Protobuf定义文件，检查并创建数据库结构
     * 
     * @return 加载结果状态码
     */
    virtual int LoadPbAndCheckDB();
public:
    /**
     * @brief 处理数据库请求
     * 
     * 处理来自客户端的数据库操作请求
     * 
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
	int OnHandleStoreReq(uint64_t unLinkId, NFDataPackage& packet);
public:
    /**
     * @brief Select Obj Rpc Service
     * 
     * 处理对象查询RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleSelectObjRpc(NFrame::storesvr_selobj& request, NFrame::storesvr_selobj_res& respone);

    /**
     * @brief Select Rpc Service
     * 
     * 处理查询RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @param cb 回调函数
     * @return 处理结果状态码
     */
    int OnHandleSelectRpc(NFrame::storesvr_sel& request, NFrame::storesvr_sel_res& respone, const std::function<void()>& cb);

    /**
     * @brief Insert Obj Rpc Service
     * 
     * 处理对象插入RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleInsertObjRpc(NFrame::storesvr_insertobj& request, NFrame::storesvr_insertobj_res& respone);

    /**
     * @brief Modify Obj Rpc Service
     * 
     * 处理对象修改RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleModifyObjRpc(NFrame::storesvr_modobj& request, NFrame::storesvr_modobj_res& respone);

    /**
     * @brief Modify Rpc Service
     * 
     * 处理修改RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleModifyRpc(NFrame::storesvr_mod& request, NFrame::storesvr_mod_res& respone);

    /**
     * @brief Update Rpc Service
     * 
     * 处理更新RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleUpdateRpc(NFrame::storesvr_update& request, NFrame::storesvr_update_res& respone);

    /**
     * @brief Update Obj Rpc Service
     * 
     * 处理对象更新RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleUpdateObjRpc(NFrame::storesvr_updateobj& request, NFrame::storesvr_updateobj_res& respone);

    /**
     * @brief Execute Rpc Service
     * 
     * 处理执行RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleExecuteRpc(NFrame::storesvr_execute& request, NFrame::storesvr_execute_res& respone);

    /**
     * @brief Execute More Rpc Service
     * 
     * 处理批量执行RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @param cb 回调函数
     * @return 处理结果状态码
     */
    int OnHandleExecuteMoreRpc(NFrame::storesvr_execute_more& request, NFrame::storesvr_execute_more_res& respone, const std::function<void()>& cb);

    /**
     * @brief Delete Rpc Service
     * 
     * 处理删除RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleDeleteRpc(NFrame::storesvr_del& request, NFrame::storesvr_del_res& respone);

    /**
     * @brief Delete Obj Rpc Service
     * 
     * 处理对象删除RPC请求
     * 
     * @param request 请求数据
     * @param respone 响应数据
     * @return 处理结果状态码
     */
    int OnHandleDeleteObjRpc(NFrame::storesvr_delobj& request, NFrame::storesvr_delobj_res& respone);

public:
    bool m_useCache; ///< 是否使用缓存
};
