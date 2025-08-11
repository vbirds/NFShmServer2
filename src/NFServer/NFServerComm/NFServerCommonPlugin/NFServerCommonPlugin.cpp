// -------------------------------------------------------------------------
//    @FileName         :    NFServerCommonPlugin.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerCommonPlugin
//    @Desc             :    NFShmXFrame服务器通用插件实现文件
//    
//    该文件实现了NFShmXFrame框架中服务器通用插件的具体功能。
//    包括插件的初始化、模块注册、动态加载支持等核心实现。
//    该插件为所有服务器类型提供统一的消息处理和通信能力。
//    
//    主要实现内容：
//    - 插件生命周期管理
//    - 动态加载接口实现
//    - 模块注册和注销
//    - 版本和名称管理
//    - 服务器消息模块注册
//
// -------------------------------------------------------------------------

#include "NFServerCommonPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIConfigModule.h"
#include "NFServerMessageModule.h"


#ifdef NF_DYNAMIC_PLUGIN

/**
 * @brief 动态插件启动函数
 * 
 * 当插件以动态库形式加载时，该函数会被调用。
 * 负责创建插件实例并注册到插件管理器中。
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStartPlugin(NFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, NFServerCommonPlugin)

};

/**
 * @brief 动态插件停止函数
 * 
 * 当插件以动态库形式卸载时，该函数会被调用。
 * 负责销毁插件实例并从插件管理器中注销。
 * 
 * @param pm 插件管理器指针
 */
NF_EXPORT void DllStopPlugin(NFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFServerCommonPlugin)
};

#endif


//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获取插件版本号
 * 
 * 返回当前插件的版本号，用于版本管理和兼容性检查。
 * 当前版本返回0，表示初始版本。
 * 
 * @return 插件版本号
 */
int NFServerCommonPlugin::GetPluginVersion()
{
    return 0;
}

/**
 * @brief 获取插件名称
 * 
 * 返回插件的唯一标识名称，用于插件管理和识别。
 * 使用GET_CLASS_NAME宏获取类名作为插件名称。
 * 
 * @return 插件名称字符串
 */
std::string NFServerCommonPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFServerCommonPlugin);
}

/**
 * @brief 安装插件
 * 
 * 在插件管理器中注册该插件提供的所有模块和服务。
 * 当前注册的模块包括：
 * - NFIServerMessageModule: 服务器消息处理模块
 * 
 * 该函数在插件加载时被调用，负责初始化所有功能模块。
 */
void NFServerCommonPlugin::Install()
{
    REGISTER_MODULE(m_pObjPluginManager, NFIServerMessageModule, NFServerMessageModule);
}

/**
 * @brief 卸载插件
 * 
 * 从插件管理器中注销该插件提供的所有模块和服务。
 * 清理插件相关的资源和注册信息，确保插件可以安全卸载。
 * 
 * 该函数在插件卸载时被调用，负责清理所有已注册的模块。
 */
void NFServerCommonPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFIServerMessageModule, NFServerMessageModule);
}