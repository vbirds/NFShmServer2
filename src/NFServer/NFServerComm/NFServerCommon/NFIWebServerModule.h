// -------------------------------------------------------------------------
//    @FileName         :    NFIWebServerModule.h
//    @Author           :    gaoyi
//    @Date             :   2023-07-10
//    @Email			:    445267987@qq.com
//    @Module           :    NFIWebServerModule.h
//    @Desc             :    Web服务器模块接口头文件，提供Web服务器的通用功能接口。
//                          该文件定义了Web服务器模块接口类，包括Web服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供Web服务器的通用功能接口、支持Web服务器连接管理、
//                          支持消息处理和路由分发、提供网络事件处理。
//                          Web服务器模块接口是NFShmXFrame框架的Web服务器组件，负责：
//                          - Web服务器的通用功能接口
//                          - Web服务器连接管理和状态维护
//                          - 消息处理和路由分发
//                          - 服务器注册和发现
//                          - 网络事件处理和回调
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFWorkServerModule.h"

/**
 * @brief Web服务器模块接口类
 * 
 * 该类是Web服务器的通用功能接口，提供了：
 * - Web服务器连接管理和状态维护
 * - 消息处理和路由分发
 * - 服务器注册和发现
 * - 网络事件处理和回调
 * 
 * 主要功能：
 * - 提供Web服务器的通用功能接口
 * - 支持Web服务器连接管理和状态维护
 * - 支持消息处理和路由分发
 * - 提供服务器注册和发现
 * - 支持网络事件处理和回调
 * - 支持跨服务器通信
 * 
 * 使用方式：
 * @code
 * class MyWebServerModule : public NFIWebServerModule {
 * public:
 *     virtual int OnExecute() override;
 *     virtual int OnTimer() override;
 * };
 * @endcode
 */
class NFIWebServerModule : public NFWorkServerModule
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化Web服务器模块，设置服务器类型为Web服务器
     * 
     * @param p 插件管理器指针
     */
    NFIWebServerModule(NFIPluginManager* p) :NFWorkServerModule(p,NF_ST_WEB_SERVER)
    {

    }

    /**
     * @brief 析构函数
     */
    virtual ~NFIWebServerModule()
    {

    }
};