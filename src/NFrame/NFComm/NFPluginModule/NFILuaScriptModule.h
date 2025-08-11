// -------------------------------------------------------------------------
//    @FileName         :    NFILuaScriptModule.h
//    @Author           :    GaoYi
//    @Date             :    2018/05/25
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    Lua脚本模块接口定义，提供Lua脚本的加载、重载、
//                           执行和会话管理功能
//
// -------------------------------------------------------------------------

/**
 * @file NFILuaScriptModule.h
 * @brief Lua脚本模块接口定义
 * @details 该文件定义了Lua脚本模块的接口，提供了Lua脚本的动态加载、重载、
 *          GM函数执行以及玩家会话管理等功能。支持运行时脚本更新和调试。
 * @author GaoYi
 * @date 2018/05/25
 * @version 1.0
 */

#pragma once

#include <vector>
#include <map>
#include <memory>

#include "NFITimerModule.h"
#include "NFServerDefine.h"
#include "NFIDynamicModule.h"

/**
 * @class NFILuaScriptModule
 * @brief Lua脚本模块接口类
 * @details 该接口定义了Lua脚本引擎的基础功能，包括：
 *          - 脚本文件的动态加载和重载
 *          - GM命令和调试函数的执行
 *          - 玩家会话的管理和报告
 *          - 脚本运行时的热更新支持
 */
class NFILuaScriptModule : public NFIDynamicModule
{
public:
	/**
	 * @brief 构造函数
	 * @param pPluginManager 插件管理器指针
	 */
	NFILuaScriptModule(NFIPluginManager* pPluginManager) : NFIDynamicModule(pPluginManager)
    {

    }

	/**
	 * @brief 析构函数
	 */
    virtual ~NFILuaScriptModule()
    {

    }

public:
	/**
	 * @brief 会话报告处理
	 * @param playerId 玩家ID
	 * @param report 报告内容
	 * @details 处理来自客户端的会话报告信息，用于统计和监控
	 */
	virtual void SessionReport(uint64_t playerId, const std::string& report) = 0;

	/**
	 * @brief 关闭玩家会话
	 * @param playerId 玩家ID
	 * @details 清理指定玩家的会话数据和相关资源
	 */
	virtual void SessionClose(uint64_t playerId) = 0;

	/**
	 * @brief 重新加载所有Lua脚本文件
	 * @details 重新加载系统中所有的Lua脚本文件，用于热更新
	 */
	virtual void ReloadAllLuaFiles() = 0;

	/**
	 * @brief 重新加载Lua脚本文件
	 * @details 重新加载已注册的Lua脚本文件
	 */
	virtual void ReloadLuaFiles() = 0;

	/**
	 * @brief 重新加载指定的Lua脚本文件
	 * @param vecStr 要重新加载的脚本文件路径列表
	 * @details 按照指定的文件列表重新加载Lua脚本，支持选择性更新
	 */
	virtual void ReloadLuaFiles(const std::vector<std::string>& vecStr) = 0;

	/**
	 * @brief 执行GM（游戏管理）函数
	 * @param luaFunc Lua函数名称
	 * @param vecStr 函数参数列表
	 * @details 执行指定的Lua GM函数，用于游戏调试和管理操作
	 */
	virtual void RunGmFunction(const std::string& luaFunc, const std::vector<std::string>& vecStr) = 0;
};

