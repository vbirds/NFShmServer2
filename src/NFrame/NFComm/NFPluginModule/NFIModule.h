// -------------------------------------------------------------------------
//    @FileName         :    NFIModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFIModule
//    @Description      :    模块接口基类定义，提供插件系统中模块的基础接口
//                           所有业务模块都需要继承此接口，实现模块化架构
//
// -------------------------------------------------------------------------

#pragma once

#include <string>
#include "NFBaseObj.h"

/**
 * @brief 模块接口基类
 * 
 * NFIModule 是插件系统中所有模块的基类，继承自 NFBaseObj。
 * 它定义了模块生命周期的标准接口，包括初始化、运行、关闭等各个阶段。
 * 
 * 模块生命周期包括以下阶段：
 * 1. AfterLoadAllPlugin - 所有插件加载完成后
 * 2. LoadConfig - 加载配置文件
 * 3. Awake - 模块唤醒
 * 4. Init - 模块初始化
 * 5. ReadyTick - 准备进入运行循环
 * 6. Tick - 运行时循环调用
 * 7. BeforeShut - 关闭前准备
 * 8. Shut - 关闭模块
 * 9. Finalize - 最终清理
 * 
 * 同时提供了服务器初始化各阶段的回调接口，以及热更新、停服等特殊操作的接口。
 */
class NFIModule : public NFBaseObj
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * 
     * 创建模块实例时需要传入插件管理器指针，用于模块与管理器的交互。
     */
    NFIModule(NFIPluginManager *p) : NFBaseObj(p)
    {

    }

    /**
     * @brief 虚析构函数
     * 
     * 确保派生类能够正确析构，释放模块占用的资源。
     */
    virtual ~NFIModule()
    {
    }

    /**
     * @brief 所有插件加载完成后调用
     * 
     * 在系统所有插件都加载完成后调用此函数，此时可以进行跨插件的模块依赖初始化。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterLoadAllPlugin()
    {
        return 0;
    }

    /**
     * @brief 加载配置文件阶段
     * 
     * 在此阶段加载模块相关的配置文件，包括Excel配置、XML配置等。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int LoadConfig()
    {
        return 0;
    }

    /**
     * @brief 模块唤醒阶段
     * 
     * 模块的早期初始化阶段，通常用于创建基础数据结构和依赖关系。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int Awake()
    {
        return 0;
    }

    /**
     * @brief 模块初始化阶段
     * 
     * 模块的主要初始化阶段，在此阶段完成模块的完整初始化工作。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int Init()
    {
        return 0;
    }

    /**
     * @brief 准备进入Tick循环阶段
     * 
     * 在模块开始接收Tick调用之前的最后准备工作。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int ReadyTick()
    {
        return 0;
    }

    /**
     * @brief 模块Tick函数
     * 
     * 运行时定期调用的函数，用于处理需要周期性执行的逻辑。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int Tick()
    {
        return 0;
    }

    /**
     * @brief 关闭前的准备工作
     * 
     * 在模块关闭之前的准备阶段，可以进行资源释放的准备工作。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int BeforeShut()
    {
        return 0;
    }

    /**
     * @brief 关闭模块
     * 
     * 关闭模块并释放相关资源，停止模块的所有功能。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int Shut()
    {
        return 0;
    }

    /**
     * @brief 最终化阶段
     * 
     * 模块生命周期的最后阶段，进行最终的资源清理和释放。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int Finalize()
    {
        return 0;
    }

    /**
     * @brief 热更新配置时调用
     * 
     * 当系统进行配置热更新时，模块调用此函数重新加载配置。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int OnReloadConfig()
    {
        return 0;
    }

    /**
     * @brief 配置热更新完成后调用
     * 
     * 在所有配置都热更新完成后，模块调用此函数进行后续处理。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterOnReloadConfig()
    {
        return 0;
    }

    /**
     * @brief 动态插件热更新后调用
     * 
     * 在动态加载DLL/SO文件后，模块调用此函数进行相应处理。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int OnDynamicPlugin()
    {
        return 0;
    }

    /**
     * @brief 热更新服务器应用程序
     * 
     * 用于服务器需要热更新应用程序代码的情况，这时候会杀掉正在运行的应用程序，
     * 重启新的服务器应用程序。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int HotfixServer()
    {
        return 0;
    }

    /**
     * @brief 检查服务器停止条件
     * 
     * 在服务器停止之前，检查是否满足停止条件。如果不满足条件，应该执行StopServer()。
     * 
     * @return 满足停止条件返回0，不满足返回非0值
     */
    virtual int CheckStopServer()
    {
        return 0;
    }

    /**
     * @brief 停止服务器前的处理
     * 
     * 在服务器停止之前，如果不满足停止条件，执行一些操作以满足停止条件。
     * 可以在此函数中实现优雅停机的逻辑。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int StopServer()
    {
        return 0;
    }

    /**
     * @brief 服务器被杀死前的处理
     * 
     * 在服务器进程被强制杀死之前执行的函数，用于保存关键数据。
     * 这是最后的数据保存机会。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int OnServerKilling()
    {
        return 0;
    }

    /**
     * @brief 服务器连接完成后调用
     * 
     * 当服务器完成所有必要的网络连接后调用此函数。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAllConnectFinish()
    {
        return 0;
    }

    /**
     * @brief 服务器数据加载完成后调用
     * 
     * 包括Excel配置数据以及从数据库拉取的数据加载完成后调用。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAllDescStoreLoaded()
    {
        return 0;
    }

    /**
     * @brief 连接和数据存储都完成后调用
     * 
     * 当AfterAllConnectFinish和AfterAllDescStoreLoaded都完成后调用。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAllConnectAndAllDescStore()
    {
        return 0;
    }

    /**
     * @brief 从数据库加载全局数据后调用
     * 
     * 这个加载一定在完成连接后执行，有可能依赖数据存储数据，也可能不依赖。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterObjFromDBLoaded()
    {
        return 0;
    }

    /**
     * @brief 完成服务器之间的数据同步后调用
     * 
     * 当服务器之间的数据同步和注册完成后调用此函数。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterServerSyncData()
    {
        return 0;
    }

    /**
     * @brief 服务器完成初始化后调用
     * 
     * 当服务器完成所有初始化工作后调用此函数。
     * 
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAppInitFinish()
    {
        return 0;
    }

    /**
     * @brief 指定类型服务器连接完成后调用
     * @param serverType 服务器类型
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAllConnectFinish(NF_SERVER_TYPE serverType)
    {
        return 0;
    }

    /**
     * @brief 指定类型服务器数据加载完成后调用
     * @param serverType 服务器类型
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAllDescStoreLoaded(NF_SERVER_TYPE serverType)
    {
        return 0;
    }

    /**
     * @brief 指定类型服务器连接和数据都完成后调用
     * @param serverType 服务器类型
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAllConnectAndAllDescStore(NF_SERVER_TYPE serverType)
    {
        return 0;
    }

    /**
     * @brief 指定类型服务器从数据库加载全局数据后调用
     * @param serverType 服务器类型
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterObjFromDBLoaded(NF_SERVER_TYPE serverType)
    {
        return 0;
    }

    /**
     * @brief 指定类型服务器完成数据同步后调用
     * @param serverType 服务器类型
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterServerSyncData(NF_SERVER_TYPE serverType)
    {
        return 0;
    }

    /**
     * @brief 指定类型服务器完成初始化后调用
     * @param serverType 服务器类型
     * @return 成功返回0，失败返回非0值
     */
    virtual int AfterAppInitFinish(NF_SERVER_TYPE serverType)
    {
        return 0;
    }

    /// @brief 模块名称，用于标识和查找模块
    std::string m_strName;
};

