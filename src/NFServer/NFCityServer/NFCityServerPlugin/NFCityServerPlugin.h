// -------------------------------------------------------------------------
//    @FileName         :    NFCityServerPlugin.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFCityServerPlugin
//    @Desc             :    NFShmXFrame城市服务器插件接口定义
//                          提供城市服务器功能的插件接口，负责城市管理和区域服务
//                          支持城市数据管理、区域划分和城市相关业务逻辑
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 城市服务器插件类
 *
 * NFCityServerPlugin是NFShmXFrame框架的城市服务器插件，
 * 负责管理城市相关的功能和业务逻辑。
 *
 * 该插件提供以下主要功能：
 * - 城市数据管理和维护
 * - 区域划分和城市边界管理
 * - 城市相关业务逻辑处理
 * - 城市服务器间通信协调
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFCityServerPlugin : public NFIPlugin
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化城市服务器插件，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCityServerPlugin(NFIPluginManager* p): NFIPlugin(p)
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
     * 注册城市服务器模块到插件管理器
     */
    virtual void Install() override;

    /**
     * @brief 卸载插件
     *
     * 从插件管理器中注销城市服务器模块
     */
    virtual void Uninstall() override;

    /**
     * @brief 初始化共享内存对象注册
     *
     * 注册城市服务器相关的共享内存对象
     *
     * @return 注册结果，true表示成功
     */
    virtual bool InitShmObjectRegister() override;
};