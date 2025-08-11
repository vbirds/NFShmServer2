// -------------------------------------------------------------------------
//    @FileName         :    NFEventDefine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    系统事件ID定义文件，包含所有预定义的系统级事件标识
//                           为事件系统提供统一的事件ID常量定义
//
// -------------------------------------------------------------------------
#pragma once

#include "NFEventObj.h"

/**
 * @brief 系统事件定义枚举
 * 
 * 定义了系统中使用的各种事件ID常量，用于事件的发布和订阅。
 * 这些事件ID保证了事件类型的唯一性和一致性。
 * 
 * 事件ID分类：
 * - 0-9: 基础系统事件（测试、GM等）
 * - 10-19: 数据库相关事件
 * - 更多分类可按需扩展
 */
enum NFEventDefine
{
	NFEVENT_TEST = 0,                 ///< 测试事件，用于系统测试和调试
	NFEVENT_GM = 1,                   ///< GM命令事件，用于游戏管理员指令处理
	NFEVENT_LUA_ERROR_LOG = 2,        ///< Lua系统错误日志事件，Lua脚本运行错误时触发
	NFEVENT_LUA_FINISH_LOAD = 3,      ///< Lua系统加载完成事件，所有Lua脚本加载完毕时触发

	NFEVENT_MYSQL_UPDATE_MESSAGE = 10, ///< MySQL数据库更新操作事件，数据库update操作时触发
};

