// -------------------------------------------------------------------------
//    @FileName         :    NFISnsServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFISnsServerModule.h
//    @Desc             :    社交服务器模块接口头文件，提供社交服务器的通用功能接口。
//                          该文件定义了社交服务器模块接口类，包括社交服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供社交服务器的通用功能接口、支持社交服务器连接管理、
//                          支持消息处理和路由分发、提供网络事件处理。
//                          社交服务器模块接口是NFShmXFrame框架的社交服务器组件，负责：
//                          - 社交服务器的通用功能接口
//                          - 社交服务器连接管理和状态维护
//                          - 消息处理和路由分发
//                          - 服务器注册和发现
//                          - 网络事件处理和回调
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFWorkServerModule.h"

/**
 * @brief 社交服务器模块接口类
 * 
 * 该类是社交服务器的通用功能接口，提供了：
 * - 社交服务器连接管理和状态维护
 * - 消息处理和路由分发
 * - 服务器注册和发现
 * - 网络事件处理和回调
 * 
 * 主要功能：
 * - 提供社交服务器的通用功能接口
 * - 支持社交服务器连接管理和状态维护
 * - 支持消息处理和路由分发
 * - 提供服务器注册和发现
 * - 支持网络事件处理和回调
 * - 支持跨服务器通信
 * 
 * 使用方式：
 * @code
 * class MySnsServerModule : public NFISnsServerModule {
 * public:
 *     virtual int OnExecute() override;
 *     virtual int OnTimer() override;
 * };
 * @endcode
 */
class NFISnsServerModule : public NFWorkServerModule
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化社交服务器模块，设置服务器类型为社交服务器
     * 
     * @param p 插件管理器指针
     */
    NFISnsServerModule(NFIPluginManager* p) :NFWorkServerModule(p, NF_ST_SNS_SERVER)
    {

    }

    /**
     * @brief 析构函数
     */
    virtual ~NFISnsServerModule()
    {

    }
};