// -------------------------------------------------------------------------
//    @FileName         :    NFIOnlineServerModule.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFIOnlineServerModule
//    @Desc             :    在线服务器模块接口头文件，提供在线服务器的通用功能接口。
//                          该文件定义了在线服务器模块接口类，包括在线服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供在线服务器的通用功能接口、支持在线服务器连接管理、
//                          支持消息处理和路由分发、提供网络事件处理。
//                          在线服务器模块接口是NFShmXFrame框架的在线服务器组件，负责：
//                          - 在线服务器的通用功能接口
//                          - 在线服务器连接管理和状态维护
//                          - 消息处理和路由分发
//                          - 服务器注册和发现
//                          - 网络事件处理和回调
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#pragma once
#include "NFWorkServerModule.h"

/**
 * @brief 在线服务器模块接口类
 * 
 * 该类是在线服务器的通用功能接口，提供了：
 * - 在线服务器连接管理和状态维护
 * - 消息处理和路由分发
 * - 服务器注册和发现
 * - 网络事件处理和回调
 * 
 * 主要功能：
 * - 提供在线服务器的通用功能接口
 * - 支持在线服务器连接管理和状态维护
 * - 支持消息处理和路由分发
 * - 提供服务器注册和发现
 * - 支持网络事件处理和回调
 * - 支持跨服务器通信
 * 
 * 使用方式：
 * @code
 * class MyOnlineServerModule : public NFIOnlineServerModule {
 * public:
 *     virtual int OnExecute() override;
 *     virtual int OnTimer() override;
 * };
 * @endcode
 */
class NFIOnlineServerModule : public NFWorkServerModule
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化在线服务器模块，设置服务器类型为在线服务器
     * 
     * @param p 插件管理器指针
     */
    NFIOnlineServerModule(NFIPluginManager* p) : NFWorkServerModule(p, NF_ST_ONLINE_SERVER)
    {

    }

    /**
     * @brief 析构函数
     */
    virtual ~NFIOnlineServerModule()
    {

    }
};