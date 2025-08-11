// -------------------------------------------------------------------------
//    @FileName         :    NFMatchServerPlugin.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFMatchServerPlugin
//    @Desc             :    NFShmXFrame匹配服务器插件接口定义
//                          提供匹配服务器功能的插件接口，负责玩家匹配和队列管理
//                          支持匹配算法、队列管理和匹配结果处理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"


#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 匹配服务器插件类
 *
 * NFMatchServerPlugin是NFShmXFrame框架的匹配服务器插件，
 * 负责玩家匹配和队列管理相关的功能和业务逻辑。
 *
 * 该插件提供以下主要功能：
 * - 玩家匹配算法
 * - 匹配队列管理
 * - 匹配结果处理
 * - 匹配规则配置
 * - 匹配状态跟踪
 * - 共享内存对象管理
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFMatchServerPlugin : public NFIPlugin
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化匹配服务器插件，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFMatchServerPlugin(NFIPluginManager* p): NFIPlugin(p)
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
     * 注册匹配服务器模块到插件管理器
     */
    virtual void Install() override;

    /**
     * @brief 卸载插件
     *
     * 从插件管理器中注销匹配服务器模块
     */
    virtual void Uninstall() override;

    /**
     * @brief 初始化共享内存对象注册
     *
     * 注册匹配服务器相关的共享内存对象，包括：
     * - 匹配队列对象
     * - 匹配规则对象
     * - 匹配结果对象
     *
     * @return 注册结果，true表示成功
     */
    virtual bool InitShmObjectRegister() override;
};