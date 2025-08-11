// -------------------------------------------------------------------------
//    @FileName         :    NFObjDefine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFObjDefine.h
 * @brief 对象基础定义文件
 * @details 定义了NFrame框架中对象系统的基础常量、枚举类型、类型别名等。
 *          包括对象魔数、无效ID、路径长度限制、对象模式枚举、以及各种
 *          基础类型定义。这些定义为整个对象系统提供了基础的类型支持。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - 对象魔数检查常量（用于内存完整性检查）
 *       - 无效ID和最大值常量定义
 *       - 对象段数量限制常量
 *       - EN_OBJ_MODE: 对象模式枚举（初始化/恢复）
 *       - NFObjectHashKey: 对象哈希键类型别名
 *       - 调试断言函数和销毁回调函数类型
 * 
 * @warning 使用这些定义时需要注意：
 *          - INVALID_ID用于表示无效的对象ID
 *          - 对象魔数用于内存安全检查，不要修改
 *          - 对象模式决定了对象的初始化方式
 *          - 销毁回调函数用于自动清理机制
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include <functional>

/**
 * @brief 对象内存完整性检查魔数
 * @details 用于检查对象内存是否被破坏的魔数常量，值为1234567890
 * @note 在调试模式下，对象会使用此魔数进行内存完整性验证
 * @warning 不要修改此值，否则会影响内存完整性检查
 */
#define OBJECT_MAGIC_CHECK_NUMBER 1234567890

/**
 * @brief 无效ID常量
 * @details 表示无效的ID值，值为-1，用于标识无效的对象ID、索引等
 * @note 当函数返回此值时，通常表示操作失败或对象不存在
 */
const int INVALID_ID = -1;

/**
 * @brief 最大路径长度常量
 * @details 定义路径字符串的最大长度，值为192字节
 * @note 用于限制文件路径、配置路径等字符串的长度
 */
const int MAX_PATH_LEN = 192;

/**
 * @brief 64位无符号整数最大值
 * @details 定义32位范围内的最大值，值为0xffffffff
 * @note 实际上这是32位无符号整数的最大值，命名可能有误导性
 */
const uint64_t ULL_MAX = 0xffffffff;

/**
 * @brief 对象段最大数量常量
 * @details 定义对象段的最大数量，值为255
 * @note 用于限制共享内存中对象段的数量
 */
const int C_I_MAX_OBJSEG_COUNT = 255;

/**
 * @enum EN_OBJ_MODE
 * @brief 对象模式枚举
 * @details 定义对象的创建和运行模式，决定对象的初始化方式
 */
typedef enum
{
	/**
	 * @brief 初始化模式
	 * @details 对象第一次创建时的模式，需要进行完整的初始化
	 * @note 在此模式下，对象会调用CreateInit()函数进行初始化
	 */
	EN_OBJ_MODE_INIT = 1,
	
	/**
	 * @brief 恢复模式
	 * @details 对象从共享内存恢复时的模式，需要恢复运行时状态
	 * @note 在此模式下，对象会调用ResumeInit()函数进行恢复
	 */
	EN_OBJ_MODE_RECOVER = 2,
} EN_OBJ_MODE;

/**
 * @brief 调试断言函数
 * @details 在调试模式下触发断言，用于调试时检查错误条件
 * @note 仅在NF_DEBUG_MODE宏定义时有效，发布版本中此函数为空
 * @warning 调用此函数会导致程序中断（调试模式下）
 */
inline void debug_assert()
{
#ifdef NF_DEBUG_MODE
	assert(0);
#endif
}

/**
 * @brief NFObject类前向声明
 * @details 声明NFObject类，避免头文件循环依赖
 */
class NFObject;

/**
 * @typedef DESTROY_OBJECT_AUTO_ERASE_FUNCTION
 * @brief 对象自动清理函数类型
 * @details 定义对象自动清理时使用的回调函数类型
 * @param pObj 要检查的对象指针
 * @return bool 返回true表示对象应该被清理，false表示保留对象
 * @note 用于对象池的自动清理机制，允许自定义清理条件
 */
typedef std::function<bool(NFObject* pObj)> DESTROY_OBJECT_AUTO_ERASE_FUNCTION;

/**
 * @typedef NFObjectHashKey
 * @brief 对象哈希键类型别名
 * @details 定义对象哈希键的数据类型为64位无符号整数
 * @note 用于在哈希表中快速定位和管理对象
 */
typedef uint64_t NFObjectHashKey;
