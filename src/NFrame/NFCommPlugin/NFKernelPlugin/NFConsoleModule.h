// -------------------------------------------------------------------------
//    @FileName         :    NFConsoleModule.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFConsoleModule
//    @Desc             :    控制台模块头文件，提供命令行交互和动态加载功能。
//                          该文件定义了NFShmXFrame框架的控制台模块，提供完整的服务器运行时控制和管理功能，
//                          包括命令行解析、交互式控制、多线程处理、消息队列、配置热重载、插件热更新、
//                          Lua热更新、文件监控、优雅关闭、性能分析、系统时间设置等功能。
//                          主要功能包括线程安全的命令消息传递、自动监控插件文件变化、
//                          后台线程处理控制台输入和命令、支持复杂的命令行参数解析
//    @Description      :    控制台模块头文件，提供命令行交互和动态加载功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIConsoleModule.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"
#include "NFComm/NFCore/NFCmdLine.h"
#include "NFComm/NFCore/NFQueue.hpp"

#include <thread>
#include <future>

/**
 * @enum NFConsoleMsgEnum
 * @brief 控制台消息类型枚举
 *
 * 定义了控制台支持的各种命令类型，用于运行时的动态控制和管理。
 */
enum NFConsoleMsgEnum
{
    NFConsoleMsg_Null = 0,          ///< 空消息
    NFConsoleMsg_Exit = 1,          ///< 退出命令
    NFConsoleMsg_Profiler = 2,      ///< 性能分析命令
    NFConsoleMsg_Reload = 3,        ///< 重载引擎配置
    NFConsoleMsg_Dynamic = 4,       ///< 动态加载引擎
    NFConsoleMsg_ProductFile = 5,   ///< 生成节点头文件
    NFConsoleMsg_HotfixLua = 6,     ///< 热更新单个Lua文件
    NFConsoleMsg_HotfixAllLua = 7,  ///< 热更新所有Lua文件
    NFConsoleMsg_SetTime = 8,       ///< 设置系统时间
};

/**
 * @struct NFConsoleMsg
 * @brief 控制台消息结构体
 *
 * 封装控制台命令和相关参数，支持最多5个字符串参数。
 */
struct NFConsoleMsg
{
    NFConsoleMsgEnum mMsgType;      ///< 消息类型
    std::string mParam1;            ///< 参数1
    std::string mParam2;            ///< 参数2
    std::string mParam3;            ///< 参数3
    std::string mParam4;            ///< 参数4
    std::string mParam5;            ///< 参数5
};

/**
 * @struct NFConsolePluginFile
 * @brief 控制台插件文件信息结构体
 *
 * 用于跟踪插件文件的修改状态，支持热更新功能。
 */
struct NFConsolePluginFile
{
    /**
     * @brief 构造函数
     * 
     * 初始化修改时间为0。
     */
    NFConsolePluginFile()
    {
        mModifyTime = 0;
        mTempModifyTime = 0;
    }

    uint32_t mTempModifyTime;       ///< 临时修改时间戳
    uint32_t mModifyTime;           ///< 文件修改时间戳
    std::string mPluginFile;        ///< 插件文件路径
    std::string mPluginName;        ///< 插件名称
    std::string mPluginMd5;         ///< 插件文件MD5校验值
};

/**
 * @class NFCConsoleModule
 * @brief 控制台模块实现类
 *
 * NFCConsoleModule提供了完整的服务器运行时控制和管理功能：
 * 
 * 控制台交互功能：
 * - 命令行解析：支持复杂的命令行参数解析
 * - 交互式控制：提供运行时的控制台交互界面
 * - 多线程处理：后台线程处理控制台输入和命令
 * - 消息队列：线程安全的命令消息传递
 * 
 * 动态管理功能：
 * - 配置热重载：运行时重新加载配置文件
 * - 插件热更新：动态加载和卸载插件模块
 * - Lua热更新：支持Lua脚本的热更新功能
 * - 文件监控：自动监控插件文件变化
 * 
 * 系统控制功能：
 * - 优雅关闭：安全的服务器关闭流程
 * - 性能分析：启动和停止性能分析器
 * - 时间设置：运行时调整系统时间
 * - 代码生成：动态生成节点头文件
 * 
 * 高级特性：
 * - 多线程架构：分离用户交互和业务逻辑
 * - 文件变化监控：自动检测插件文件修改
 * - MD5校验：确保文件完整性
 * - 命令历史：支持命令历史记录
 * 
 * 应用场景：
 * - 开发调试：开发过程中的快速调试和测试
 * - 运维管理：生产环境的运行时管理
 * - 热更新：无停机的代码和配置更新
 * - 监控控制：实时的系统状态监控和控制
 * 
 * @note 控制台模块运行在独立线程中，不会阻塞主业务逻辑
 * @note 支持跨平台的控制台交互功能
 * @warning 热更新操作需要谨慎使用，确保不影响正在运行的业务
 */
class NFCConsoleModule : public NFIConsoleModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    NFCConsoleModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     */
    virtual ~NFCConsoleModule();

public:
    /**
     * @brief 模块觉醒初始化
     * @return 返回0表示成功，非0表示失败
     * 
     * 创建后台线程，初始化命令解析器和消息队列。
     */
    virtual int Awake();

    /**
     * @brief 模块定时更新
     * @return 返回0表示成功，非0表示失败
     * 
     * 处理来自控制台的命令消息，执行相应的操作。
     */
    virtual int Tick();

    /**
     * @brief 关闭模块
     * @return 返回0表示成功，非0表示失败
     * 
     * 停止后台线程，清理资源。
     */
    virtual int Shut();

    /**
     * @brief 配置重载处理
     * @return 返回0表示成功，非0表示失败
     * 
     * 响应配置重载命令，更新相关设置。
     */
    virtual int OnReloadConfig();

    /**
     * @brief 定时器回调处理
     * @param nTimerID 定时器ID
     * @return 返回0表示成功，非0表示失败
     * 
     * 处理定时器触发的事件，如定期检查插件文件。
     */
    virtual int OnTimer(uint32_t nTimerID);

    /**
     * @brief 添加动态插件文件到监控列表
     * @param file 插件文件路径
     * 
     * 将插件文件加入监控列表，自动检测文件修改并支持热更新。
     */
    virtual void AddDynamicPluginFile(const std::string &file);

    /**
     * @brief 检查插件文件修改
     * 
     * 遍历监控的插件文件，检测是否有文件被修改，
     * 如果发现修改则触发相应的热更新操作。
     */
    virtual void CheckPluginFileModify();

public:
    /**
     * @brief 创建后台处理线程
     * 
     * 创建用于处理控制台输入和命令执行的后台线程。
     * 该线程独立于主业务逻辑线程运行。
     */
    virtual void CreateBackThread();

    /**
     * @brief 检查插件线程
     * 
     * 创建专门用于监控插件文件变化的线程。
     */
    virtual void CheckPluginThread();

    /**
     * @brief 后台线程主循环
     * 
     * 后台线程的主要工作循环，负责：
     * - 读取控制台输入
     * - 解析用户命令
     * - 将命令加入消息队列
     * - 处理线程间通信
     */
    virtual void BackThreadLoop();

    /**
     * @brief 插件检查线程主循环
     * 
     * 插件监控线程的主要工作循环，负责：
     * - 定期检查插件文件修改时间
     * - 计算文件MD5值变化
     * - 触发热更新操作
     * - 处理文件系统事件
     */
    virtual void CheckPluginThreadLoop();

private:
    /**
     * @brief 命令行解析器
     * 
     * 用于解析控制台输入的命令和参数，
     * 支持复杂的命令行语法和参数验证。
     */
    NFCmdLine::NFParser mCmdParser;
    
    /**
     * @brief 后台处理线程
     * 
     * 负责控制台交互和命令处理的后台线程。
     */
    std::shared_ptr<std::thread> mBackThread;
    
    /**
     * @brief 插件监控线程
     * 
     * 负责监控插件文件变化的专用线程。
     */
    std::shared_ptr<std::thread> mPluginThread;
    
    /**
     * @brief 控制台消息队列
     * 
     * 线程安全的消息队列，用于在后台线程和主线程间传递命令。
     */
    NFQueueVector<NFConsoleMsg> mQueueMsg;

    /**
     * @brief 插件文件监控列表
     * 
     * 存储所有需要监控的插件文件信息，
     * 包括文件路径、修改时间、MD5值等。
     */
    std::vector<NFConsolePluginFile> mPluginFileList;
};
