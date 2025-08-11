// -------------------------------------------------------------------------
//    @FileName         :    NFIPluginManager.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFIPluginManager
//    @Description      :    插件管理器接口定义，提供插件系统的核心管理接口
//                           负责插件和模块的生命周期管理、动态加载、配置管理等功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFPluginModule/NFGlobalSystem.h"
#include "NFSystemInfo.h"
#include <functional>
#include <list>

/// @brief 表示所有服务器的常量字符串
#define ALL_SERVER ("AllServer")

class NFIModule;

class NFIPlugin;

class NFIPluginManager;

/// @brief 用于创建独立模块的函数类型定义
typedef std::function<NFIModule *(NFIPluginManager* pMan)> CREATE_ALONE_MODULE;

/// @brief 用于创建插件的函数类型定义
typedef std::function<NFIPlugin *(NFIPluginManager* pMan)> CREATE_PLUGIN_FUNCTION;

/**
 * @brief 全局模块查找模板函数
 * @tparam T 模块类型，必须继承自NFIModule
 * @return 返回找到的模块指针，未找到返回nullptr
 * 
 * 该函数通过全局系统查找指定类型的模块，支持多服务器模式和单服务器模式。
 * 在macOS平台上，由于dynamic_cast的兼容性问题，使用静态转换。
 */
template <typename T>
static T* FindModule()
{
	static T* pStaticModule = NULL;
	if (NFGlobalSystem::Instance()->IsMoreServer())
	{
		NFIModule* pLogicModule = NFGlobalSystem::Instance()->FindModule(typeid(T).name());
		if (pLogicModule)
		{
			if (!TIsDerived<T, NFIModule>::Result)
			{
				return nullptr;
			}

			//OSX上dynamic_cast返回了NULL
#if NF_PLATFORM == NF_PLATFORM_APPLE
			T* pT = (T*)pLogicModule;
#else
			T* pT = dynamic_cast<T*>(pLogicModule);
#endif

			pStaticModule = pT;
			return pT;
		}
		return nullptr;
	}
	if (pStaticModule == NULL)
	{
		NFIModule* pLogicModule = NFGlobalSystem::Instance()->FindModule(typeid(T).name());
		if (pLogicModule)
		{
			if (!TIsDerived<T, NFIModule>::Result)
			{
				return nullptr;
			}

			//OSX上dynamic_cast返回了NULL
#if NF_PLATFORM == NF_PLATFORM_APPLE
			T* pT = (T*)pLogicModule;
#else
			T* pT = dynamic_cast<T*>(pLogicModule);
#endif

			pStaticModule = pT;
			return pT;
		}
		return nullptr;
	}
	return pStaticModule;
}

/**
 * @brief 插件管理器接口类
 * 
 * NFIPluginManager 是整个插件系统的核心管理接口，负责：
 * 
 * 1. 插件生命周期管理：
 *    - 插件的加载、注册、初始化
 *    - 插件的运行时管理和调度
 *    - 插件的卸载和资源清理
 * 
 * 2. 模块管理：
 *    - 模块的注册和查找
 *    - 模块间的依赖管理
 *    - 模块的生命周期调度
 * 
 * 3. 系统配置管理：
 *    - 应用程序配置参数管理
 *    - 路径和环境设置
 *    - 运行时参数控制
 * 
 * 4. 服务器管理：
 *    - 多服务器模式支持
 *    - 服务器初始化阶段控制
 *    - 服务器停止和重启管理
 * 
 * 5. 性能和监控：
 *    - 性能分析器管理
 *    - 运行时统计信息
 *    - 调试和诊断功能
 * 
 * 6. 高级功能：
 *    - 热更新支持
 *    - 动态插件加载
 *    - 共享内存管理
 *    - 进程管理
 */
class NFIPluginManager
{
public:
	/**
	 * @brief 默认构造函数
	 * 
	 * 使用默认实现，不执行任何特殊操作。
	 */
	NFIPluginManager() = default;

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类对象能够正确释放资源。
	 */
	virtual ~NFIPluginManager() = default;

	/**
	 * @brief 所有插件加载完成后调用
	 * 
	 * 提供一个钩子，允许在所有插件加载完成后执行自定义操作，
	 * 如跨插件的依赖关系建立、全局初始化等。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterLoadAllPlugin()
	{
		return 0;
	}

	/**
	 * @brief 配置加载阶段
	 * 
	 * 在共享内存初始化完成后调用，用于加载系统配置文件，
	 * 包括Excel配置、XML配置等。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int LoadConfig()
	{
		return 0;
	}

	/**
	 * @brief 系统唤醒阶段
	 * 
	 * 在对象初始化之前执行准备工作，建立基础的系统环境。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int Awake()
	{
		return 0;
	}

	/**
	 * @brief 系统初始化阶段
	 * 
	 * 执行系统的主要初始化工作，创建必要的数据结构和服务。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int Init()
	{
		return 0;
	}

	/**
	 * @brief 准备运行阶段
	 * 
	 * 在进入主运行循环前进行最后的准备工作。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int ReadyTick()
	{
		return 0;
	}

	/**
	 * @brief 主运行循环函数
	 * 
	 * 执行系统的主要逻辑或任务，定期被调用以处理业务逻辑。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int Tick()
	{
		return 0;
	}

	/**
	 * @brief 关闭前准备阶段
	 * 
	 * 在系统关闭之前执行准备工作，如保存重要数据、断开连接等。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int BeforeShut()
	{
		return 0;
	}

	/**
	 * @brief 系统关闭阶段
	 * 
	 * 执行实际的关闭操作，停止服务和释放资源。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int Shut()
	{
		return 0;
	}

	/**
	 * @brief 系统最终化阶段
	 * 
	 * 执行关闭后的清理工作，确保所有资源都被正确释放。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int Finalize()
	{
		return 0;
	}

	/**
	 * @brief 配置重新加载处理
	 * 
	 * 当配置需要重新加载时调用，处理配置更新的逻辑。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int OnReloadConfig()
	{
		return 0;
	}

	/**
	 * @brief 配置重新加载后处理
	 * 
	 * 在配置重新加载完成后执行后续操作。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterOnReloadConfig()
	{
		return 0;
	}

	/**
	 * @brief 服务器连接完成回调
	 * 
	 * 当所有必要的服务器连接建立完成后调用。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAllConnectFinish()
	{
		return 0;
	}

	/**
	 * @brief 服务器数据加载完成回调
	 * 
	 * 当服务器数据（包括Excel配置和数据库数据）加载完成后调用。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAllDescStoreLoaded()
	{
		return 0;
	}

	/**
	 * @brief 连接和数据加载都完成回调
	 * 
	 * 当服务器连接和数据加载都完成后调用。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAllConnectAndAllDescStore()
	{
		return 0;
	}

	/**
	 * @brief 指定类型服务器连接完成回调
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAllConnectFinish(NF_SERVER_TYPE serverType)
	{
		return 0;
	}

	/**
	 * @brief 指定类型服务器数据加载完成回调
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAllDescStoreLoaded(NF_SERVER_TYPE serverType)
	{
		return 0;
	}

	/**
	 * @brief 指定类型服务器连接和数据都完成回调
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAllConnectAndAllDescStore(NF_SERVER_TYPE serverType)
	{
		return 0;
	}

	/**
	 * @brief 从数据库加载全局数据回调
	 * 
	 * 从数据库加载全局数据完成后调用，这个加载一定在完成连接后执行，
	 * 有可能依赖数据存储数据，也可能不依赖。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterObjFromDBLoaded()
	{
		return 0;
	}

	/**
	 * @brief 服务器数据同步完成回调
	 * 
	 * 完成服务器之间的数据同步和注册后调用。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterServerSyncData()
	{
		return 0;
	}

	/**
	 * @brief 应用程序初始化完成回调
	 * 
	 * 服务器完成所有初始化工作后调用。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAppInitFinish()
	{
		return 0;
	}

	/**
	 * @brief 指定类型服务器从数据库加载全局数据回调
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterObjFromDBLoaded(NF_SERVER_TYPE serverType)
	{
		return 0;
	}

	/**
	 * @brief 指定类型服务器数据同步完成回调
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterServerSyncData(NF_SERVER_TYPE serverType)
	{
		return 0;
	}

	/**
	 * @brief 指定类型服务器初始化完成回调
	 * @param serverType 服务器类型
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int AfterAppInitFinish(NF_SERVER_TYPE serverType)
	{
		return 0;
	}

	/**
	 * @brief 查找指定类型的模块
	 * @tparam T 模块类型，必须继承自NFIModule
	 * @return 返回找到的模块指针，未找到返回nullptr
	 * 
	 * 该模板函数用于类型安全地查找指定类型的模块，支持多服务器模式和单服务器模式。
	 * 在macOS平台上使用静态转换以避免dynamic_cast的兼容性问题。
	 */
	template <typename T>
	T* FindModule()
	{
		static T* pStaticModule = NULL;
		if (NFGlobalSystem::Instance()->IsMoreServer())
		{
			NFIModule* pLogicModule = FindModule(typeid(T).name());
			if (pLogicModule)
			{
				if (!TIsDerived<T, NFIModule>::Result)
				{
					return nullptr;
				}

				//OSX上dynamic_cast返回了NULL
#if NF_PLATFORM == NF_PLATFORM_APPLE
                T* pT = (T*)pLogicModule;
#else
				T* pT = dynamic_cast<T*>(pLogicModule);
#endif

				pStaticModule = pT;
				return pT;
			}
			return nullptr;
		}
		if (pStaticModule == nullptr)
		{
			NFIModule* pLogicModule = FindModule(typeid(T).name());
			if (pLogicModule)
			{
				if (!TIsDerived<T, NFIModule>::Result)
				{
					return nullptr;
				}

				//OSX上dynamic_cast返回了NULL
#if NF_PLATFORM == NF_PLATFORM_APPLE
                    T* pT = (T*)pLogicModule;
#else
				T* pT = dynamic_cast<T*>(pLogicModule);
#endif

				pStaticModule = pT;
				return pT;
			}
			return nullptr;
		}
		return pStaticModule;
	}


	/**
	 * @brief 开始插件管理器的运行
	 * 
	 * 初始化插件管理器并开始运行，通常在系统启动时调用。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int Begin() = 0;

	/**
	 * @brief 结束插件管理器的运行
	 * 
	 * 停止插件管理器的运行并进行清理工作。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int End() = 0;

	/**
	 * @brief 注册静态插件
	 * @param strPluginName 插件名称
	 * @param createFunc 插件创建函数
	 * 
	 * 注册一个静态链接的插件，提供插件的创建函数以便后续实例化。
	 */
	virtual void RegisteredStaticPlugin(const std::string& strPluginName, const CREATE_PLUGIN_FUNCTION& createFunc) = 0;

	/**
	 * @brief 注册插件实例
	 * @param plugin 插件实例指针
	 * 
	 * 将一个已创建的插件实例注册到插件管理器中。
	 */
	virtual void Registered(NFIPlugin* plugin) = 0;

	/**
	 * @brief 反注册插件实例
	 * @param plugin 插件实例指针
	 * 
	 * 从插件管理器中移除指定的插件实例。
	 */
	virtual void UnRegistered(NFIPlugin* plugin) = 0;

	/**
	 * @brief 查找插件
	 * @param strPluginName 插件名称
	 * @return 返回找到的插件指针，未找到返回nullptr
	 */
	virtual NFIPlugin* FindPlugin(const std::string& strPluginName) = 0;

	/**
	 * @brief 添加模块到系统
	 * @param strModuleName 模块名称
	 * @param pModule 模块实例指针
	 * @return 成功返回模块ID，失败返回-1
	 */
	virtual int AddModule(const std::string& strModuleName, NFIModule* pModule) = 0;

	/**
	 * @brief 从系统中移除模块
	 * @param strModuleName 模块名称
	 */
	virtual void RemoveModule(const std::string& strModuleName) = 0;

	/**
	 * @brief 查找模块
	 * @param strModuleName 模块名称
	 * @return 返回找到的模块指针，未找到返回nullptr
	 */
	virtual NFIModule* FindModule(const std::string& strModuleName) = 0;

	/**
	 * @brief 加载所有插件
	 * 
	 * 加载系统中已注册的所有插件。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int LoadAllPlugin() = 0;

	/**
	 * @brief 加载指定的插件库
	 * @param strPluginDLLName 插件库文件名或路径
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int LoadPluginLibrary(const std::string& strPluginDLLName) = 0;

	/**
	 * @brief 卸载指定的插件库
	 * @param strPluginDLLName 插件库文件名或路径
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int UnLoadPluginLibrary(const std::string& strPluginDLLName) = 0;

	/**
	 * @brief 动态加载指定的插件库
	 * @param strPluginDLLName 插件库文件名或路径
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int DynamicLoadPluginLibrary(const std::string& strPluginDLLName) = 0;

	/**
	 * @brief 获取应用程序完整路径
	 * @return 返回应用程序的完整路径字符串
	 */
	virtual const std::string& GetFullPath() const = 0;

	/**
	 * @brief 设置应用程序完整路径
	 * @param strFullPath 要设置的完整路径
	 */
	virtual void SetFullPath(const std::string& strFullPath) = 0;

	/**
	 * @brief 获取应用程序ID
	 * @return 返回当前应用程序的唯一标识ID
	 */
	virtual int GetAppID() const = 0;

	/**
	 * @brief 设置应用程序ID
	 * @param nAppID 要设置的应用程序唯一标识ID
	 */
	virtual void SetAppID(int nAppID) = 0;

	/**
	 * @brief 获取世界ID
	 * @return 返回当前所属的世界ID
	 */
	virtual int GetWorldID() const = 0;

	/**
	 * @brief 获取区域ID
	 * @return 返回当前所属的区域ID
	 */
	virtual int GetZoneID() const = 0;

	/**
	 * @brief 获取区域内的子区域ID
	 * @return 返回区域内的子区域标识ID
	 */
	virtual int GetZoneAreaID() const = 0;

	/**
	 * @brief 获取配置文件路径
	 * @return 返回当前对象的配置文件路径
	 */
	virtual const std::string& GetConfigPath() const = 0;

	/**
	 * @brief 设置配置文件路径
	 * @param strPath 要设置的配置文件路径
	 */
	virtual void SetConfigPath(const std::string& strPath) = 0;

	/**
	 * @brief 获取插件路径
	 * @return 返回当前对象的插件路径
	 */
	virtual const std::string& GetPluginPath() const = 0;

	/**
	 * @brief 设置插件路径
	 * @param strPath 要设置的插件路径
	 */
	virtual void SetPluginPath(const std::string& strPath) = 0;

	/**
	 * @brief 获取应用程序名称
	 * @return 返回当前对象的应用程序名称
	 */
	virtual const std::string& GetAppName() const = 0;

	/**
	 * @brief 设置应用程序名称
	 * @param strAppName 要设置的应用程序名称
	 */
	virtual void SetAppName(const std::string& strAppName) = 0;

	/**
	 * @brief 获取字符串参数
	 * @return 返回当前对象的字符串参数
	 */
	virtual const std::string& GetStrParam() const = 0;

	/**
	 * @brief 设置字符串参数
	 * @param strAppName 要设置的字符串参数
	 */
	virtual void SetStrParam(const std::string& strAppName) = 0;

	/**
	 * @brief 获取日志路径
	 * @return 返回当前对象的日志路径
	 */
	virtual const std::string& GetLogPath() const = 0;

	/**
	 * @brief 设置日志路径
	 * @param strName 要设置的日志路径
	 */
	virtual void SetLogPath(const std::string& strName) = 0;

	/**
	 * @brief 判断是否加载所有服务器
	 * @return 返回是否加载所有服务器的布尔值
	 */
	virtual bool IsLoadAllServer() const = 0;

	/**
	 * @brief 设置是否加载所有服务器
	 * @param b 要设置的布尔值
	 */
	virtual void SetLoadAllServer(bool b) = 0;

	/**
	 * @brief 设置Lua脚本路径
	 * @param luaScriptPath 要设置的Lua脚本路径
	 */
	virtual void SetLuaScriptPath(const std::string& luaScriptPath) = 0;

	/**
	 * @brief 设置游戏名称
	 * @param game 要设置的游戏名称
	 */
	virtual void SetGame(const std::string& game) = 0;

	/**
	 * @brief 获取Lua脚本路径
	 * @return 返回当前对象的Lua脚本路径
	 */
	virtual const std::string& GetLuaScriptPath() const = 0;

	/**
	 * @brief 获取游戏名称
	 * @return 返回当前对象的游戏名称
	 */
	virtual const std::string& GetGame() const = 0;

	/**
	 * @brief 获取初始化时间
	 * @return 返回当前对象的初始化时间
	 */
	virtual uint64_t GetInitTime() const = 0;

	/**
	 * @brief 获取当前时间
	 * @return 返回当前对象的当前时间
	 */
	virtual uint64_t GetNowTime() const = 0;

	/**
	 * @brief 判断是否为守护进程
	 * @return 返回是否为守护进程的布尔值
	 */
	virtual bool IsDaemon() const = 0;

	/**
	 * @brief 设置为守护进程
	 */
	virtual void SetDaemon() = 0;

	/**
	 * @brief 设置是否开启性能分析器
	 * @param b 要设置的布尔值
	 */
	virtual void SetOpenProfiler(bool b) = 0;

	/**
	 * @brief 判断是否开启性能分析器
	 * @return 返回是否开启性能分析器的布尔值
	 */
	virtual bool IsOpenProfiler() = 0;

	/**
	 * @brief 开始性能分析
	 * @param funcName 要分析的函数名称
	 */
	virtual void BeginProfiler(const std::string& funcName) = 0;

	/**
	 * @brief 结束性能分析
	 * @return 返回一个64位无符号整数，表示从开始性能分析到结束所经过的时间或其他性能指标。
	 */
	virtual uint64_t EndProfiler() = 0;

	/**
	 * @brief 检查服务器是否正在停止
	 * 
	 * 检查服务器是否处于停服状态。停服意味着需要保存重要数据，
	 * 共享内存可能会被清理，服务器将走正常的停服流程。
	 * 
	 * @return 服务器正在停止返回true，否则返回false
	 */
	virtual bool IsServerStopping() const = 0;

	/**
	 * @brief 设置服务器停止状态
	 * 
	 * 设置服务器的停服状态。当设置为停服时，服务器将开始执行
	 * 停服流程，保存数据并准备关闭。
	 * 
	 * @param exitApp 是否停止服务器
	 */
	virtual void SetServerStopping(bool exitApp) = 0;

	/**
	 * @brief 停止服务器
	 * 
	 * 执行服务器停止操作。这将触发正常的停服流程，
	 * 保存需要的数据，清理资源。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int StopServer() = 0;

	/**
	 * @brief 检查服务器停止条件
	 * 
	 * 在服务器停止之前，检查是否满足停服条件。
	 * 如玩家是否下线、数据是否保存完成等。
	 * 
	 * @return 满足停止条件返回0，否则返回非0值
	 */
	virtual int CheckStopServer() = 0;

	/**
	 * @brief 停服前的处理操作
	 * 
	 * 在停服之前执行一些操作以满足停服条件，
	 * 如强制玩家下线、保存关键数据等。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int OnStopServer() = 0;

	/**
	 * @brief 服务器被强制终止前的处理
	 * 
	 * 当服务器进程即将被强制终止时调用，
	 * 用于执行最后的数据保存操作。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int OnServerKilling() = 0;

	/**
	 * @brief 检查服务器是否需要重新加载
	 * 
	 * 检查服务器是否处于配置重新加载状态。
	 * 
	 * @return 需要重新加载返回true，否则返回false
	 */
	virtual bool IsReloadServer() const = 0;

	/*
	 * SetReloadServer - 重新加载服务器的配置数据
	 *
	 * 该函数是一个纯虚函数，用于通知服务器重新加载其配置数据。具体实现由派生类提供。
	 *
	 * @param exitApp: 布尔值，指示是否在重新加载配置后退出应用程序。
	 *                 - true: 重新加载配置后退出应用程序。
	 *                 - false: 仅重新加载配置，不退出应用程序。
	 *
	 * @return: 无返回值。
	 */
	virtual void SetReloadServer(bool exitApp) = 0;

	/**
	 * @brief 获取是否在更改配置文件时退出应用程序。
	 *
	 * 该函数用于查询当前是否在更改配置文件时退出应用程序。
	 *
	 * @return bool 返回值为 true 表示在更改配置文件时退出应用程序，false 表示不退出。
	 */
	virtual bool GetChangeProfileApp() const = 0;

	/**
	 * @brief 设置是否在更改配置文件时退出应用程序。
	 *
	 * 该函数用于设置是否在更改配置文件时退出应用程序。
	 *
	 * @param exitApp 如果为 true，表示在更改配置文件时退出应用程序；如果为 false，表示不退出。
	 */
	virtual void SetChangeProfileApp(bool exitApp) = 0;

	/**
	 * @brief 获取是否在启动新应用程序时终止前一个应用程序。
	 *
	 * 该函数用于查询当前是否在启动新应用程序时终止前一个应用程序。
	 *
	 * @return bool 返回值为 true 表示在启动新应用程序时终止前一个应用程序，false 表示不终止。
	 */
	virtual bool GetKillPreApp() const = 0;

	/**
	 * @brief 设置是否在启动新应用程序时终止前一个应用程序。
	 *
	 * 该函数用于设置是否在启动新应用程序时终止前一个应用程序。
	 *
	 * @param exitApp 如果为 true，表示在启动新应用程序时终止前一个应用程序；如果为 false，表示不终止。
	 */
	virtual void SetKillPreApp(bool exitApp) = 0;

	/**
	 * @brief 检查服务器是否需要热更新
	 * 
	 * 检查服务器是否处于热更新状态。热更新用于服务器需要
	 * 更新应用程序代码的情况，这时会终止当前应用程序并重启新版本。
	 * 
	 * @return 需要热更新返回true，否则返回false
	 */
	virtual bool IsHotfixServer() const = 0;

	/**
	 * @brief 设置服务器热更新状态
	 * 
	 * 设置服务器的热更新状态。当设置为热更新时，
	 * 服务器将准备终止当前进程并启动新版本。
	 * 
	 * @param exitApp 是否执行热更新
	 */
	virtual void SetHotfixServer(bool exitApp) = 0;

	/**
	 * @brief 执行服务器热更新
	 * 
	 * 执行服务器热更新操作。这将终止当前运行的应用程序，
	 * 并重启新版本的服务器应用程序。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int HotfixServer() = 0;

	/**
	 * @brief 检查共享内存是否已初始化。
	 *
	 * 该函数用于判断共享内存是否已经成功初始化。
	 *
	 * @return bool 返回true表示共享内存已初始化，false表示未初始化。
	 */
	virtual bool IsInitShm() const = 0;

	/**
	 * @brief 设置共享内存为已初始化状态。
	 *
	 * 该函数用于标记共享内存已经成功初始化。
	 */
	virtual void SetInitShm() = 0;

	/**
	 * @brief 设置总线名称。
	 *
	 * 该函数用于设置当前对象关联的总线名称。
	 *
	 * @param busName 总线名称，类型为std::string。
	 */
	virtual void SetBusName(const std::string& busName) = 0;

	/**
	 * @brief 获取总线名称。
	 *
	 * 该函数用于获取当前对象关联的总线名称。
	 *
	 * @return const std::string& 返回总线名称的引用。
	 */
	virtual const std::string& GetBusName() const = 0;

	/**
	 * @brief 设置PID文件名。
	 *
	 * 该函数用于设置当前对象关联的PID文件名。
	 */
	virtual void SetPidFileName() = 0;

	/**
	 * @brief 获取PID文件名。
	 *
	 * 该函数用于获取当前对象关联的PID文件名。
	 *
	 * @return const std::string& 返回PID文件名的引用。
	 */
	virtual const std::string& GetPidFileName() = 0;

#if NF_PLATFORM == NF_PLATFORM_LINUX
	/**
	 * @brief Linux版本的带超时等待子进程结束
	 * @param pid 子进程ID
	 * @param sec 超时时间（秒）
	 * @return 返回子进程退出状态码
	 */
	virtual int TimedWait(pid_t pid, int sec) = 0;
#else
	/**
	 * @brief Windows版本的带超时等待进程结束
	 * @param proc_id Windows进程ID
	 * @param sec 超时时间（秒）
	 * @return 返回进程退出状态码
	 */
	virtual int TimedWait(DWORD proc_id, int sec) = 0;
#endif

	/**
	 * @brief 检查PID文件是否存在
	 * 
	 * 检查当前应用程序的PID文件是否已经存在，
	 * 用于防止同一应用程序的多重启动。
	 * 
	 * @return 文件存在返回0，不存在返回非0值
	 */
	virtual int CheckPidFile() = 0;

	/**
	 * @brief 创建PID文件
	 * 
	 * 创建包含当前进程ID的PID文件，
	 * 用于进程管理和防止重复启动。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int CreatePidFile() = 0;

	/**
	 * @brief 终止之前的应用程序实例
	 * 
	 * 查找并终止之前运行的同名应用程序实例，
	 * 确保只有一个实例在运行。
	 * 
	 * @return 成功返回0，失败返回非0值
	 */
	virtual int KillPreApp() = 0;

	/**
	 * @brief 停止应用程序
	 * 
	 * 发送停止信号给应用程序，触发正常的关闭流程。
	 */
	virtual void StopApp() = 0;

	/**
	 * @brief 重新加载应用程序
	 * 
	 * 发送重新加载信号给应用程序，触发配置重新加载。
	 */
	virtual void ReloadApp() = 0;

	/**
	 * @brief 退出应用程序
	 * 
	 * 发送退出信号给应用程序，触发立即退出。
	 */
	virtual void QuitApp() = 0;

	/**
	 * @brief 获取当前帧的编号。
	 * @return 返回当前帧的编号。
	 */
	virtual uint32_t GetFrame() const = 0;

	/**
	 * @brief 获取当前帧的时间。
	 * @return 返回当前帧的时间，单位为毫秒。
	 */
	virtual uint32_t GetFrameTime() const = 0;

	/**
	 * @brief 获取当前帧的计数。
	 * @return 返回当前帧的计数。
	 */
	virtual uint32_t GetCurFrameCount() const = 0;

	/**
	 * @brief 判断当前帧是否为固定帧。
	 * @return 返回true表示当前帧为固定帧，否则返回false。
	 */
	virtual bool IsFixedFrame() const = 0;

	/**
	 * @brief 设置当前帧是否为固定帧。
	 * @param frame 设置为true表示当前帧为固定帧，false表示非固定帧。
	 */
	virtual void SetFixedFrame(bool frame) = 0;

	/**
	 * @brief 获取空闲时的睡眠时间。
	 * @return 返回空闲时的睡眠时间，单位为微秒。
	 */
	virtual uint32_t GetIdleSleepUs() const = 0;

	/**
	 * @brief 设置空闲时的睡眠时间。
	 * @param time 空闲时的睡眠时间，单位为微秒。
	 */
	virtual void SetIdleSleepUs(uint32_t time) = 0;

	/**
	 * @brief 判断是否已经初始化。
	 * @return 返回true表示已经初始化，否则返回false。
	 */
	virtual bool IsInited() const = 0;

	/**
	 * @brief 判断指定服务器类型是否已经初始化。
	 * @param eServerType 服务器类型。
	 * @return 返回true表示指定服务器类型已经初始化，否则返回false。
	 */
	virtual bool IsInited(NF_SERVER_TYPE eServerType) const = 0;

	/**
	 * @brief 设置初始化状态。
	 * @param b 设置为true表示已初始化，false表示未初始化。
	 */
	virtual void SetIsInited(bool b) = 0;

	/**
	 * @brief 注册应用程序任务。
	 * @param eServerType 服务器类型。
	 * @param taskType 任务类型。
	 * @param desc 任务描述。
	 * @param taskGroup 任务组。
	 * @return 返回注册结果，0表示成功，非0表示失败。
	 */
	virtual int RegisterAppTask(NF_SERVER_TYPE eServerType, uint32_t taskType, const std::string& desc,
	                            uint32_t taskGroup) = 0;

	/**
	 * @brief 完成应用程序任务。
	 * @param eServerType 服务器类型。
	 * @param taskType 任务类型。
	 * @param taskGroup 任务组。
	 * @return 返回完成结果，0表示成功，非0表示失败。
	 */
	virtual int FinishAppTask(NF_SERVER_TYPE eServerType, uint32_t taskType,
	                          uint32_t taskGroup) = 0;

	/**
	 * @brief 判断指定服务器类型的任务组是否已完成。
	 * @param eServerType 服务器类型。
	 * @param taskGroup 任务组。
	 * @return 返回true表示任务组已完成，否则返回false。
	 */
	virtual bool IsFinishAppTask(NF_SERVER_TYPE eServerType, uint32_t taskGroup) const = 0;

	/**
	 * @brief 判断指定服务器类型的任务组是否存在。
	 * @param eServerType 服务器类型。
	 * @param taskGroup 任务组。
	 * @return 返回true表示任务组存在，否则返回false。
	 */
	virtual bool IsHasAppTask(NF_SERVER_TYPE eServerType, uint32_t taskGroup) const = 0;

	/**
	 * @brief 判断指定服务器类型的任务组中是否存在指定类型的任务。
	 * @param eServerType 服务器类型。
	 * @param taskGroup 任务组。
	 * @param taskType 任务类型。
	 * @return 返回true表示任务存在，否则返回false。
	 */
	virtual bool IsHasAppTask(NF_SERVER_TYPE eServerType, uint32_t taskGroup, uint32_t taskType) const = 0;

	/**
	 * @brief 发送转储信息。
	 * @param dmpInfo 转储信息。
	 * @return 返回发送结果，0表示成功，非0表示失败。
	 */
	virtual int SendDumpInfo(const std::string& dmpInfo) = 0;

	/**
	 * @brief 获取插件列表
	 *
	 * 该函数是一个纯虚函数，用于获取当前系统中所有插件的列表。
	 * 子类必须实现该函数以返回具体插件列表。
	 *
	 * @return std::list<NFIPlugin*> 返回一个包含所有插件的列表，列表中的每个元素都是指向NFIPlugin对象的指针。
	 */
	virtual std::list<NFIPlugin*> GetListPlugin() = 0;

	/**
	 * @brief 获取机器地址的MD5值
	 *
	 * 该函数是一个纯虚函数，用于获取当前机器地址的MD5哈希值。
	 * 子类必须实现该函数以返回具体的MD5值。
	 *
	 * @return std::string 返回一个字符串，表示当前机器地址的MD5哈希值。
	 */
	virtual std::string GetMachineAddrMD5() = 0;
};

