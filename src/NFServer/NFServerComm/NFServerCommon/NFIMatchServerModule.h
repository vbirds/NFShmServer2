// -------------------------------------------------------------------------
//    @FileName         :    NFIMatchServerModule.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFIMatchServerModule
//    @Desc             :    匹配服务器模块接口头文件，提供匹配服务器的通用功能接口。
//                          该文件定义了匹配服务器模块接口类，包括匹配服务器连接管理、
//                          消息处理接口、服务器注册功能、网络事件处理。
//                          主要功能包括提供匹配服务器的通用功能接口、支持匹配服务器连接管理、
//                          支持消息处理和路由分发、提供网络事件处理。
//                          匹配服务器模块接口是NFShmXFrame框架的匹配服务器组件，负责：
//                          - 匹配服务器的通用功能接口
//                          - 匹配服务器连接管理和状态维护
//                          - 消息处理和路由分发
//                          - 服务器注册和发现
//                          - 网络事件处理和回调
//                          - 跨服务器通信支持
//
// -------------------------------------------------------------------------

#pragma once
#include "NFWorkServerModule.h"

/**
 * @brief 匹配服务器模块接口类
 * 
 * 该类是匹配服务器的通用功能接口，提供了：
 * - 匹配服务器连接管理和状态维护
 * - 消息处理和路由分发
 * - 服务器注册和发现
 * - 网络事件处理和回调
 * 
 * 主要功能：
 * - 提供匹配服务器的通用功能接口
 * - 支持匹配服务器连接管理和状态维护
 * - 支持消息处理和路由分发
 * - 提供服务器注册和发现
 * - 支持网络事件处理和回调
 * - 支持跨服务器通信
 * 
 * 使用方式：
 * @code
 * class MyMatchServerModule : public NFIMatchServerModule {
 * public:
 *     virtual int OnExecute() override;
 *     virtual int OnTimer() override;
 * };
 * @endcode
 */
class NFIMatchServerModule : public NFWorkServerModule
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化匹配服务器模块，设置服务器类型为匹配服务器
     * 
     * @param p 插件管理器指针
     */
    NFIMatchServerModule(NFIPluginManager* p) : NFWorkServerModule(p, NF_ST_MATCH_SERVER)
    {

    }

    /**
     * @brief 析构函数
     */
    virtual ~NFIMatchServerModule()
    {

    }
};