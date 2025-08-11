// -------------------------------------------------------------------------
//    @FileName         :    NFDefine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    插件模块的基础定义文件，包含对象事件、数据事件等核心枚举和类型定义
//                           为插件系统提供统一的事件和数据类型标准
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include <functional>
#include <stdint.h>
#include <string.h>

/**
 * @brief 对象事件类型枚举
 * 
 * 定义了对象生命周期中的各种事件类型，用于对象的创建、销毁、数据加载等过程的管理。
 * 这些事件按照对象生命周期的顺序依次触发，为系统提供完整的对象状态管理。
 */
enum NF_OBJECT_EVENT
{
	OBJECT_EVT_NONE = 0,            ///< 无事件
	OBJECT_EVT_DESTROY,             ///< 对象销毁事件
	OBJECT_EVT_PRE_DESTROY,         ///< 对象销毁前事件
	OBJECT_EVT_PRE_LOAD_DATA,       ///< 数据加载前事件
	OBJECT_EVT_LOAD_DATA,           ///< 数据加载事件
	OBJECT_EVT_PRE_EFFECT_DATA,     ///< 数据生效前事件
	OBJECT_EVT_EFFECT_DATA,         ///< 数据生效事件
	OBJECT_EVT_POST_EFFECT_DATA,    ///< 数据生效后事件
	OBJECT_EVT_DATA_FINISHED,       ///< 数据处理完成事件
	OBJECT_EVT_ALL_FINISHED,        ///< 所有处理完成事件（创建实体时需要手动调用）
};

class NFCData;

/**
 * @brief 数据表事件数据类
 * 
 * 用于描述数据表操作事件的详细信息，包括操作类型、行列位置、名称等信息。
 * 当数据表发生变化时，会通过此类传递事件的具体信息。
 */
class DATA_TABLE_EVENT_DATA
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化数据表事件数据，设置默认值。
	 */
	DATA_TABLE_EVENT_DATA()
		: nOpType(0), nRow(-1), nCol(-1)
	{
	}

	uint8_t nOpType;      ///< 操作类型（插入、删除、更新等）
	int16_t nRow;         ///< 行索引，-1表示无效行
	int16_t nCol;         ///< 列索引，-1表示无效列  
	std::string strName;  ///< 相关名称（字段名、表名等）
};

/// @brief 数据节点事件函数类型定义，用于处理数据节点变化事件
/// @param 参数1 对象ID
/// @param 参数2 属性名称
/// @param 参数3 旧数据值
/// @param 参数4 新数据值
/// @return 处理结果，0表示成功
using DATA_NODE_EVENT_FUNCTOR = std::function<int(const uint64_t, const std::string&, const NFCData&, const NFCData&)>;

/// @brief 数据节点事件函数智能指针类型
using DATA_NODE_EVENT_FUNCTOR_PTR = NF_SHARE_PTR<DATA_NODE_EVENT_FUNCTOR>;

/// @brief 数据表事件函数类型定义，用于处理数据表变化事件
/// @param 参数1 对象ID
/// @param 参数2 事件数据详情
/// @param 参数3 旧数据值
/// @param 参数4 新数据值
/// @return 处理结果，0表示成功
using DATA_TABLE_EVENT_FUNCTOR = std::function<int(const uint64_t, const DATA_TABLE_EVENT_DATA&, const NFCData&, const NFCData&)>;

/// @brief 数据表事件函数智能指针类型
using DATA_TABLE_EVENT_FUNCTOR_PTR = NF_SHARE_PTR<DATA_TABLE_EVENT_FUNCTOR>;

/// @brief 类事件函数类型定义，用于处理对象类级别的事件
/// @param 参数1 对象ID
/// @param 参数2 类名称
/// @param 参数3 对象事件类型
/// @param 参数4 事件数据
/// @return 处理结果，true表示成功
using CLASS_EVENT_FUNCTOR = std::function<bool(const uint64_t&, const std::string&, const NF_OBJECT_EVENT, const NFCData&)>;

/// @brief 类事件函数智能指针类型
using CLASS_EVENT_FUNCTOR_PTR = NF_SHARE_PTR<CLASS_EVENT_FUNCTOR>;

