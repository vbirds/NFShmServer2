// -------------------------------------------------------------------------
//    @FileName         :    NFNavMeshServerPlugin.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFNavMeshServerPlugin
//    @Desc             :    NFShmXFrame导航网格服务器插件接口定义
//                          提供导航网格服务器功能的插件接口，负责路径查找和导航计算
//                          支持A*算法、寻路优化和导航网格管理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"


#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 导航网格服务器插件类
 *
 * NFNavMeshServerPlugin是NFShmXFrame框架的导航网格服务器插件，
 * 负责管理路径查找和导航计算相关的功能和业务逻辑。
 *
 * 该插件提供以下主要功能：
 * - 路径查找和导航计算
 * - A*算法实现
 * - 导航网格管理
 * - 寻路优化
 * - 动态障碍物处理
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFNavMeshServerPlugin : public NFIPlugin
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化导航网格服务器插件，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFNavMeshServerPlugin(NFIPluginManager* p): NFIPlugin(p)
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
     * 注册导航网格服务器模块到插件管理器
     */
    virtual void Install() override;

    /**
     * @brief 卸载插件
     *
     * 从插件管理器中注销导航网格服务器模块
     */
    virtual void Uninstall() override;

    /**
     * @brief 初始化共享内存对象注册
     *
     * 注册导航网格服务器相关的共享内存对象
     *
     * @return 注册结果，true表示成功
     */
    virtual bool InitShmObjectRegister() override;
};