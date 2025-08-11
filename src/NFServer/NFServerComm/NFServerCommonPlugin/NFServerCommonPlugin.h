// -------------------------------------------------------------------------
//    @FileName         :    NFServerCommonPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerCommonPlugin
//    @Desc             :    NFShmXFrame服务器通用插件头文件
//    
//    该文件定义了NFShmXFrame框架中服务器通用插件的接口类。
//    服务器通用插件负责提供服务器间通信、消息处理、数据同步等核心功能。
//    该插件是NFShmXFrame框架的重要组成部分，为所有服务器类型提供统一的消息处理能力。
//    
//    主要功能包括：
//    - 服务器间消息传递和路由
//    - 数据同步和事务处理
//    - 跨服务器通信支持
//    - 统一的插件管理接口
//    - 动态插件加载和管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 服务器通用插件类
 * 
 * NFServerCommonPlugin是NFShmXFrame框架中的服务器通用插件，
 * 负责提供服务器间通信、消息处理、数据同步等核心功能。
 * 该插件实现了NFIPlugin接口，可以被插件管理器动态加载和管理。
 * 
 * 主要功能包括：
 * - 服务器间消息传递
 * - 数据同步和事务处理
 * - 跨服务器通信支持
 * - 统一的插件管理接口
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFServerCommonPlugin : public NFIPlugin
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化服务器通用插件，设置插件管理器
     * 
     * @param p 插件管理器指针
     */
    explicit NFServerCommonPlugin(NFIPluginManager* p):NFIPlugin(p)
    {

    }

    /**
     * @brief 获取插件版本号
     * 
     * 返回当前插件的版本号，用于版本管理和兼容性检查
     * 
     * @return 插件版本号
     */
    virtual int GetPluginVersion() override;

    /**
     * @brief 获取插件名称
     * 
     * 返回插件的唯一标识名称，用于插件管理和识别
     * 
     * @return 插件名称字符串
     */
    virtual std::string GetPluginName() override;

    /**
     * @brief 安装插件
     * 
     * 在插件管理器中注册该插件提供的所有模块和服务。
     * 包括服务器消息模块等核心功能模块的注册。
     */
    virtual void Install() override;

    /**
     * @brief 卸载插件
     * 
     * 从插件管理器中注销该插件提供的所有模块和服务。
     * 清理插件相关的资源和注册信息。
     */
    virtual void Uninstall() override;
};

