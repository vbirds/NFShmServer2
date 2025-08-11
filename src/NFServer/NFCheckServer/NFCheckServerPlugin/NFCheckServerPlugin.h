// -------------------------------------------------------------------------
//    @FileName         :    NFCheckServerPlugin.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFCheckServerPlugin
//    @Desc             :    NFShmXFrame检查服务器插件接口定义
//                          提供检查服务器功能的插件接口，负责系统健康检查和监控
//                          支持系统状态检查、性能监控和故障诊断
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 检查服务器插件类
 *
 * NFCheckServerPlugin是NFShmXFrame框架的检查服务器插件，
 * 负责系统健康检查和监控相关的功能和业务逻辑。
 *
 * 该插件提供以下主要功能：
 * - 系统健康状态检查
 * - 性能监控和统计
 * - 故障诊断和报告
 * - 系统资源监控
 * - 服务可用性检查
 * - 共享内存对象管理
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFCheckServerPlugin : public NFIPlugin
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化检查服务器插件，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCheckServerPlugin(NFIPluginManager* p): NFIPlugin(p)
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
     * 注册检查服务器模块到插件管理器
     */
    virtual void Install() override;

    /**
     * @brief 卸载插件
     *
     * 从插件管理器中注销检查服务器模块
     */
    virtual void Uninstall() override;

    /**
     * @brief 初始化共享内存对象注册
     *
     * 注册检查服务器相关的共享内存对象，包括：
     * - 健康检查对象
     * - 监控数据对象
     * - 诊断信息对象
     *
     * @return 注册结果，true表示成功
     */
    virtual bool InitShmObjectRegister() override;
};