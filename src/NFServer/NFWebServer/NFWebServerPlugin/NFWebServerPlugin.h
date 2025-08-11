// -------------------------------------------------------------------------
//    @FileName         :    NFWebServerPlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFWebServerPlugin
//    @Desc             :    NFShmXFrame Web服务器插件接口定义
//                          提供Web服务器功能的插件接口，负责HTTP服务和Web管理
//                          支持HTTP请求处理、Web界面管理和API服务
// -------------------------------------------------------------------------


#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief Web服务器插件类
 *
 * NFWebServerPlugin是NFShmXFrame框架的Web服务器插件，
 * 负责管理HTTP服务和Web管理相关的功能和业务逻辑。
 *
 * 该插件提供以下主要功能：
 * - HTTP请求处理
 * - Web界面管理
 * - API服务提供
 * - 静态资源服务
 * - Web安全控制
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFWebServerPlugin : public NFIPlugin
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化Web服务器插件，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFWebServerPlugin(NFIPluginManager* p):NFIPlugin(p)
    {
    }

    /**
     * @brief 获取插件版本号
     *
     * @return 插件版本号
     */
    virtual int GetPluginVersion() override;

    /**
     * @brief 获取插件名称
     *
     * @return 插件名称字符串
     */
    virtual std::string GetPluginName() override;

    /**
     * @brief 安装插件
     *
     * 注册Web服务器模块到插件管理器
     */
    virtual void Install() override;

    /**
     * @brief 卸载插件
     *
     * 从插件管理器中注销Web服务器模块
     */
    virtual void Uninstall() override;
};

