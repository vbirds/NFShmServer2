// -------------------------------------------------------------------------
//    @FileName         :    NFDescStorePlugin.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDescStorePlugin
//    @Desc             :    NFShmXFrame描述存储插件接口定义
//                          提供描述存储功能的插件接口，支持配置表数据的存储和管理
//                          包括数据库存储和文件存储两种方式，支持动态加载和热更新
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include <string>

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 描述存储插件类
 * 
 * NFDescStorePlugin是NFShmXFrame框架的描述存储插件，
 * 负责管理配置表数据的存储、加载和访问功能。
 * 
 * 该插件提供以下主要功能：
 * - 配置表数据的数据库存储和文件存储
 * - 支持动态加载和热更新配置表
 * - 提供统一的配置表访问接口
 * - 支持多种数据格式和存储方式
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFDescStorePlugin : public NFIPlugin
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化描述存储插件，设置插件管理器
     * 
     * @param p 插件管理器指针
     */
    explicit NFDescStorePlugin(NFIPluginManager* p) : NFIPlugin(p)
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
     * 注册描述存储模块到插件管理器
     */
    virtual void Install() override;

    /**
     * @brief 卸载插件
     * 
     * 从插件管理器中注销描述存储模块
     */
    virtual void Uninstall() override;

    /**
     * @brief 初始化共享内存对象注册
     * 
     * 注册描述存储相关的共享内存对象
     * 
     * @return 注册是否成功
     */
    virtual bool InitShmObjectRegister() override;
};


