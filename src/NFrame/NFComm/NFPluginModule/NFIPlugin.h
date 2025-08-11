// -------------------------------------------------------------------------
//    @FileName         :    NFIPlugin.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFIPlugin
//    @Description      :    插件接口定义，提供插件系统的基础接口和宏定义
//                           用于实现模块化的插件架构，支持动态加载和管理
//
// -------------------------------------------------------------------------

#pragma once


#include <iostream>
#include <map>
#include <vector>
#include "NFComm/NFCore/NFPlatform.h"
#include "NFIModule.h"

/**
 * @brief 注册模块的宏定义
 * @param pManager 插件管理器指针
 * @param classBaseName 基类名称
 * @param className 具体类名称
 * 
 * 该宏用于在插件中注册一个模块，会创建模块实例并添加到管理器中。
 * 使用模板元编程确保类型继承关系的正确性。
 */
#define REGISTER_MODULE(pManager, classBaseName, className)  \
    assert((TIsDerived<classBaseName, NFIModule>::Result)); \
    assert((TIsDerived<className, classBaseName>::Result)); \
    NFIModule* pRegisterModule##className= new className(pManager); \
    pRegisterModule##className->m_strName = (#className);             \
    pManager->AddModule(#classBaseName, pRegisterModule##className );AddModule(#classBaseName, pRegisterModule##className );

/**
 * @brief 注册需要参与Tick循环的模块的宏定义
 * @param pManager 插件管理器指针
 * @param classBaseName 基类名称
 * @param className 具体类名称
 * 
 * 该宏用于注册需要定期执行Tick函数的模块，这些模块会被加入到Tick循环中。
 */
#define REGISTER_MODULE_TICK(pManager, classBaseName, className)  \
    assert((TIsDerived<classBaseName, NFIModule>::Result)); \
    assert((TIsDerived<className, classBaseName>::Result)); \
    NFIModule* pRegisterModule##className= new className(pManager); \
    pRegisterModule##className->m_strName = (#className);             \
    pManager->AddModule(#classBaseName, pRegisterModule##className );AddModuleTick(#classBaseName, pRegisterModule##className );

/**
 * @brief 反注册模块的宏定义
 * @param pManager 插件管理器指针
 * @param classBaseName 基类名称
 * @param className 具体类名称
 * 
 * 该宏用于从插件管理器中移除并销毁指定的模块实例。
 */
#define UNREGISTER_MODULE(pManager, classBaseName, className) \
   NFIModule* pUnRegisterModule##className =  dynamic_cast<NFIModule*>(pManager->FindModule(typeid(classBaseName).name())); \
	pManager->RemoveModule( #classBaseName );RemoveModule(#classBaseName); delete pUnRegisterModule##className;

/**
 * @brief 注册静态插件的宏定义
 * @param pManager 插件管理器指针
 * @param className 插件类名称
 * 
 * 该宏用于注册静态链接的插件，通过lambda表达式提供插件创建函数。
 */
#define REGISTER_STATIC_PLUGIN(pManager, className)  pManager->RegisteredStaticPlugin(#className, [] (NFIPluginManager* pMan) ->NFIPlugin* { return NF_NEW className(pMan);});

/**
 * @brief 创建插件实例的宏定义
 * @param pManager 插件管理器指针
 * @param className 插件类名称
 * 
 * 该宏用于创建插件实例并将其注册到插件管理器中。
 */
#define CREATE_PLUGIN(pManager, className)  NFIPlugin* pCreatePlugin##className = new className(pManager); pManager->Registered( pCreatePlugin##className );

/**
 * @brief 销毁插件实例的宏定义
 * @param pManager 插件管理器指针
 * @param className 插件类名称
 * 
 * 该宏用于从插件管理器中反注册并销毁指定的插件实例。
 */
#define DESTROY_PLUGIN(pManager, className) pManager->UnRegistered( pManager->FindPlugin((#className)) );

class NFIPluginManager;

/**
 * @brief 插件接口基类
 * 
 * NFIPlugin 是所有插件的基类，继承自 NFIModule，提供了插件系统的核心接口。
 * 插件是模块的容器，负责管理多个相关模块的生命周期，并提供插件级别的管理功能。
 * 
 * 主要功能包括：
 * - 插件的安装和卸载
 * - 模块的注册和管理
 * - 插件生命周期的管理
 * - 支持动态加载和热更新
 * - 提供服务器初始化各阶段的回调接口
 */
class NFIPlugin : public NFIModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFIPlugin(NFIPluginManager* p):NFIModule(p)
	{
	}

	/**
	 * @brief 获取插件版本号
	 * @return 返回插件的版本号
	 */
	virtual int GetPluginVersion() = 0;

	/**
	 * @brief 获取插件名称
	 * @return 返回插件的名称字符串
	 */
	virtual std::string GetPluginName() = 0;

	/**
	 * @brief 安装插件
	 * 
	 * 在插件加载时调用，用于注册插件包含的所有模块。
	 * 派生类需要在此函数中实现具体的模块注册逻辑。
	 */
	virtual void Install() = 0;

	/**
	 * @brief 卸载插件
	 * 
	 * 在插件卸载时调用，用于清理插件注册的所有模块和资源。
	 * 派生类需要在此函数中实现具体的清理逻辑。
	 */
	virtual void Uninstall() = 0;
public:
	/**
	 * @brief 所有插件加载完成后调用
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterLoadAllPlugin() override;

	/**
	 * @brief 加载配置文件阶段
	 * @return 成功返回0，失败返回非0值
	 */
	int LoadConfig() override;

	/**
	 * @brief 插件唤醒阶段
	 * @return 成功返回0，失败返回非0值
	 */
	int Awake() override;

	/**
	 * @brief 插件初始化阶段
	 * @return 成功返回0，失败返回非0值
	 */
	int Init() override;

	/**
	 * @brief 准备进入Tick循环阶段
	 * @return 成功返回0，失败返回非0值
	 */
	int ReadyTick() override;

	/**
	 * @brief 插件Tick函数，定期执行
	 * @return 成功返回0，失败返回非0值
	 */
	int Tick() override;

	/**
	 * @brief 关闭前的准备工作
	 * @return 成功返回0，失败返回非0值
	 */
	int BeforeShut() override;

	/**
	 * @brief 关闭插件
	 * @return 成功返回0，失败返回非0值
	 */
	int Shut() override;

	/**
	 * @brief 最终化阶段，释放资源
	 * @return 成功返回0，失败返回非0值
	 */
	int Finalize() override;

	/**
	 * @brief 重新加载配置时调用
	 * @return 成功返回0，失败返回非0值
	 */
	int OnReloadConfig() override;

	/**
	 * @brief 重新加载配置后的处理
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterOnReloadConfig() override;
    /**
     * @brief 热更新服务器
     * 
     * 用于服务器需要热更新应用代码的情况，这时候会杀掉正在运行的应用程序，
     * 重启新的服务器应用程序。
     * 
     * @return 成功返回0，失败返回非0值
     */
	int HotfixServer() override;

    /**
     * @brief 检查服务器停止条件
     * 
     * 在服务器停止之前，检查服务器是否满足停止条件。
     * 可以在此函数中实现优雅停机的条件检查。
     * 
     * @return 成功返回0，失败返回非0值
     */
	int CheckStopServer() override;

    /**
     * @brief 停止服务器前的处理
     * 
     * 在服务器停止之前，做一些操作以满足停止条件。
     * 可以在此函数中实现停服前的数据保存、连接断开等操作。
     * 
     * @return 成功返回0，失败返回非0值
     */
	int StopServer() override;

    /**
     * @brief 服务器被杀死前的数据保存
     * 
     * 在服务器进程被杀死之前保存需要的数据。
     * 这是最后的数据保存机会，应该保存关键数据。
     * 
     * @return 成功返回0，失败返回非0值
     */
	int OnServerKilling() override;

	/**
	 * @brief 动态插件加载时调用
	 * @return 成功返回0，失败返回非0值
	 */
	int OnDynamicPlugin() override;
public:
    /**
     * @brief 初始化共享内存对象注册
     * 
     * 用于注册需要在共享内存中管理的对象类型。
     * 
     * @return 成功返回true，失败返回false
     */
    virtual bool InitShmObjectRegister();

	/**
	 * @brief 检查插件是否支持动态加载
	 * @return 支持动态加载返回true，否则返回false
	 */
	virtual bool IsDynamicLoad();

	/**
	 * @brief 添加模块到插件
	 * @param moduleName 模块名称
	 * @param pModule 模块实例指针
	 * 
	 * 将指定的模块添加到插件的模块管理列表中。
	 */
	virtual void AddModule(const std::string& moduleName, NFIModule* pModule);
	
	/**
	 * @brief 添加需要Tick的模块到插件
	 * @param moduleName 模块名称
	 * @param pModule 模块实例指针
	 * 
	 * 将指定的模块添加到插件的Tick模块列表中，这些模块会参与定期的Tick调用。
	 */
	virtual void AddModuleTick(const std::string& moduleName, NFIModule* pModule);

	/**
	 * @brief 从插件中移除模块
	 * @param moduleName 模块名称
	 * 
	 * 从插件的模块管理列表中移除指定的模块。
	 */
	virtual void RemoveModule(const std::string& moduleName);

    /**
     * @brief 服务器连接完成后调用
     * @return 成功返回0，失败返回非0值
     */
	int AfterAllConnectFinish() override;

    /**
     * @brief 加载完服务器数据后调用
     * 
     * 包括Excel配置数据以及从数据库拉取的数据加载完成后调用。
     * 
     * @return 成功返回0，失败返回非0值
     */
	int AfterAllDescStoreLoaded() override;

	/**
	 * @brief 连接和数据存储都完成后调用
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterAllConnectAndAllDescStore() override;

    /**
     * @brief 从数据库加载全局数据后调用
     * 
     * 这个加载一定在完成连接后执行，有可能依赖数据存储数据，也可能不依赖。
     * 
     * @return 成功返回0，失败返回非0值
     */
	int AfterObjFromDBLoaded() override;

    /**
     * @brief 完成服务器之间的数据同步后调用
     * @return 成功返回0，失败返回非0值
     */
	int AfterServerSyncData() override;

    /**
     * @brief 服务器完成初始化后调用
     * @return 成功返回0，失败返回非0值
     */
	int AfterAppInitFinish() override;

	/**
	 * @brief 指定类型服务器连接完成后调用
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterAllConnectFinish(NF_SERVER_TYPE serverType) override;

	/**
	 * @brief 指定类型服务器数据加载完成后调用
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterAllDescStoreLoaded(NF_SERVER_TYPE serverType) override;

	/**
	 * @brief 指定类型服务器连接和数据都完成后调用
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterAllConnectAndAllDescStore(NF_SERVER_TYPE serverType) override;

	/**
	 * @brief 指定类型服务器从数据库加载全局数据后调用
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterObjFromDBLoaded(NF_SERVER_TYPE serverType) override;

	/**
	 * @brief 指定类型服务器完成数据同步后调用
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterServerSyncData(NF_SERVER_TYPE serverType) override;

	/**
	 * @brief 指定类型服务器完成初始化后调用
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	int AfterAppInitFinish(NF_SERVER_TYPE serverType) override;
protected:
	/// @brief 插件包含的模块映射表，键为模块名称，值为模块实例指针
	std::map<std::string, NFIModule*> m_mapModule;
	/// @brief 插件包含的所有模块列表，用于顺序遍历
	std::vector<NFIModule*> m_vecModule;
	/// @brief 需要参与Tick循环的模块列表
	std::vector<NFIModule*> m_vecTickModule;
};



