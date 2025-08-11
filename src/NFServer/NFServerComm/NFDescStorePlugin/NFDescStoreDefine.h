// -------------------------------------------------------------------------
//    @FileName         :    NFDescStoreDefine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFDescStoreDefine.h
//    @Desc             :    NFShmXFrame描述存储相关定义
//                          定义描述存储模块中使用的常量和枚举值
//                          包括哈希表大小、最大ID值等配置参数
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"

/**
 * @brief 最大物品哈希表数量
 * 
 * 定义物品哈希表的最大数量，用于优化查找性能
 */
const int MAX_ITEM_HASH_NUM = 8887;

/**
 * @brief 描述存储相关枚举定义
 */
enum {
	/**
	 * @brief 最大物品描述ID值
	 * 
	 * 定义物品描述ID的最大值，用于ID范围限制
	 */
	MAX_ITEM_DESC_ID_VALUE = 100000,
};
