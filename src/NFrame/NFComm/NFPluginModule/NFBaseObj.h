// -------------------------------------------------------------------------
//    @FileName         :    NFBaseObj.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//    @Description      :    基础对象类定义，提供插件系统中所有对象的基类
//                           为所有对象提供插件管理器访问和模块查找功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFIPluginManager.h"
#include <stddef.h>
#include <string.h>

/**
 * @brief 基础对象类
 * 
 * NFBaseObj 是插件系统中所有对象的基类，提供了与插件管理器交互的基础功能。
 * 
 * 主要功能：
 * - 提供插件管理器的访问接口
 * - 提供类型安全的模块查找功能
 * - 为所有派生类提供统一的基础服务
 * 
 * 设计理念：
 * - 所有需要访问插件系统功能的对象都应该继承此类
 * - 通过组合方式持有插件管理器的引用
 * - 提供模板化的模块查找功能，确保类型安全
 */
class NFBaseObj
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * 
     * 创建基础对象实例，必须提供有效的插件管理器指针。
     * 插件管理器是所有对象访问系统功能的入口。
     */
    NFBaseObj(NFIPluginManager *p): m_pObjPluginManager(p)
    {
        //NF_ASSERT_MSG(m_pObjPluginManager != NULL, "m_pObjPluginManager == nullptr")
    }

    /**
     * @brief 虚析构函数
     * 
     * 确保派生类能够正确析构，释放所有占用的资源。
     */
    virtual ~NFBaseObj()
    {

    }

    /**
     * @brief 查找指定类型的模块
     * @tparam T 模块类型，必须继承自NFIModule
     * @return 返回找到的模块指针，未找到返回nullptr
     * 
     * 该模板函数提供类型安全的模块查找功能，通过插件管理器
     * 查找指定类型的模块实例。这是对象访问系统功能的主要途径。
     * 
     * 使用示例：
     * @code
     * auto pLogModule = FindModule<NFILogModule>();
     * if (pLogModule) {
     *     pLogModule->LogInfo("Hello World");
     * }
     * @endcode
     */
    template <typename T>
    T* FindModule() const
    {
        if (m_pObjPluginManager)
        {
            return m_pObjPluginManager->FindModule<T>();
        }
        return nullptr;
    }
public:
    /// @brief 插件管理器指针，提供对插件系统所有功能的访问
    NFIPluginManager* m_pObjPluginManager;
};