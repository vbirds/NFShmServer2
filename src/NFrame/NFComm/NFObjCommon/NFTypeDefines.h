// -------------------------------------------------------------------------
//    @FileName         :    NFTypeDefines.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

/**
 * @file NFTypeDefines.h
 * @brief 框架类型定义文件
 * @details 定义了NFrame框架中的核心类型枚举和宏定义，包括共享内存对象类型枚举、
 *          对象注册宏、以及对象管理相关的标识符。这些定义为整个框架的对象系统
 *          提供了统一的类型标识和注册机制。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - EN_FRAME_SHMOBJ_TYPE: 框架共享内存对象类型枚举
 *       - REGISTER_SHM_OBJ: 共享内存对象注册宏
 *       - REGISTER_SINGLETON_SHM_OBJ: 单例对象注册宏
 *       - REGISTER_SHM_OBJ_WITH_HASH: 带哈希的对象注册宏
 *       - UNREGISTER_SHM_OBJ: 对象反注册宏
 * 
 * @warning 使用类型定义时需要注意：
 *          - 对象类型ID范围已预分配，不要冲突
 *          - 基础引擎ID: 0-100，服务器架构ID: 100-999
 *          - 描述存储ID: 1000-2000，最大类型: 2000
 *          - 注册对象时要确保类型ID唯一
 *          - 反注册时确保没有对象实例在使用
 */

#pragma once
#include "NFComm/NFCore/NFPlatform.h"

/**
 * @enum EN_FRAME_SHMOBJ_TYPE
 * @brief NFrame框架共享内存对象类型枚举
 * @details 定义了NFrame框架中所有共享内存对象的类型标识符，用于对象注册、创建、管理等。
 *          类型ID按照不同的功能模块进行了分段，确保不同模块的对象类型不会冲突。
 * 
 * @note ID分配规则：
 *       - 0-100: 基础引擎共享内存对象ID
 *       - 100-999: 服务器架构共享内存对象ID
 *       - 1000-2000: 描述存储对象ID
 *       - 最大类型ID: 2000
 * 
 * @warning 注意事项：
 *          - 位置标记为"位置不可修改"的枚举值不能改变其数值
 *          - 新增类型时要注意不要超出各段的范围
 *          - 确保每个类型ID在整个系统中唯一
 */
typedef enum
{
	/**
	 * @brief 基础对象类型
	 * @details 框架基础对象NFObject的类型ID，所有对象的基类
	 * @note 0-100的是基础引擎共享内存ID
	 */
	EOT_OBJECT = 0,
	
	/**
	 * @brief 全局ID对象类型
	 * @details 全局ID管理对象的类型ID
	 * @warning 位置不可修改，必须保持为1
	 */
	EOT_GLOBAL_ID = 1,
	
    /**
     * @brief 定时器对象类型
     * @details 定时器对象的类型ID
     * @warning 位置不可修改，必须保持为2
     */
    EOT_TYPE_TIMER_OBJ = 2,
    
    /**
     * @brief 定时器管理器类型
     * @details 定时器管理器对象的类型ID
     * @warning 位置不可修改，必须保持为3
     */
    EOT_TYPE_TIMER_MNG = 3,
    
    /**
     * @brief 订阅信息对象类型
     * @details 事件订阅信息对象的类型ID
     * @warning 位置不可修改，必须保持为4
     */
    EOT_TYPE_SUBSCRIBEINFO_OBJ = 4,
    
    /**
     * @brief 事件管理器类型
     * @details 事件管理器对象的类型ID
     * @warning 位置不可修改，必须保持为5
     */
    EOT_TYPE_EVENT_MGR = 5,
    
    /**
     * @brief 事务基类类型
     * @details 事务处理基类NFTransBase的类型ID
     */
    EOT_TRANS_BASE = 6,
    
    /**
     * @brief 事务管理器类型
     * @details 事务管理器对象的类型ID
     */
    EOT_TRANS_MNG = 7,
    
    /**
     * @brief 基础数据库对象类型
     * @details 基础数据库对象的类型ID
     */
    EOT_BASE_DB_OBJ = 8,
    
    /**
     * @brief 事务数据库对象类型
     * @details 事务处理中的数据库对象类型ID
     */
    EOT_TRANS_DB_OBJ = 9,
    
    /**
     * @brief 事务数据库对象管理器类型
     * @details 事务数据库对象管理器的类型ID
     */
    EOT_TRANS_DB_OBJ_MGR = 10,
    
    /**
     * @brief RPC事务ID类型
     * @details RPC调用中的事务标识对象类型ID
     */
    EOT_RPC_TRANS_ID = 11,
    
    /**
     * @brief 服务器同步数据对象类型
     * @details 服务器间数据同步对象的类型ID
     */
    EOT_SERVER_SYNC_DATA_OBJ = 12,
    
    /**
     * @brief 服务器同步数据对象管理器类型
     * @details 服务器同步数据对象管理器的类型ID
     */
    EOT_SERVER_SYNC_DATA_OBJ_MGR = 13,
    
    /**
     * @brief 事务服务器同步数据对象类型
     * @details 事务处理中的服务器同步数据对象类型ID
     */
    EOT_TRANS_SERVER_SYNC_DATA_OBJ = 14,

	/**
	 * @brief 服务器架构对象ID范围开始
	 * @details 服务器架构共享内存对象ID的起始值
	 * @note 100-999的是服务器架构共享内存ID
	 */
	EOT_SERVER_FRAME_BEGIN_ID = 100,
	
	/**
	 * @brief 服务器架构对象ID范围结束
	 * @details 服务器架构共享内存对象ID的结束值
	 */
	EOT_SERVER_FRAME_END_ID = 999,

	/**
	 * @brief 描述存储对象ID范围开始
	 * @details 描述存储对象ID的起始值，用于配置数据等静态数据对象
	 */
	EOT_DESC_STORE_BEGIN_ID = 1000,
	
    /**
     * @brief 描述存储对象ID范围结束
     * @details 描述存储对象ID的结束值
     */
    EOT_DESC_STORE_END_ID = 2000,

	/**
	 * @brief 最大对象类型值
	 * @details 定义对象类型的最大值，不能超过此值
	 * @warning 新增对象类型时不能超过此值
	 */
	EOT_MAX_TYPE = 2000,
} EN_FRAME_SHMOBJ_TYPE;

/**
 * @brief 注册共享内存对象宏
 * @details 用于注册普通共享内存对象到对象管理系统
 * @param ClassName 要注册的类名
 * @param ObjNum 对象池中该类型对象的数量
 * 
 * @note 使用方式：
 * @code
 * REGISTER_SHM_OBJ(MyClass, 1000);
 * @endcode
 * 
 * @warning 注意事项：
 *          - 必须在对象使用前注册
 *          - 对象数量要根据实际需求合理设置
 *          - 类必须已经定义了GetStaticClassType()等静态方法
 */
#define REGISTER_SHM_OBJ(ClassName, ObjNum ) do{\
        ClassName::RegisterClassToObjSeg(ClassName::GetStaticClassType(), sizeof(ClassName), (ObjNum), std::string(#ClassName), false);\
    }while(0)

/**
 * @brief 注册单例共享内存对象宏
 * @details 用于注册单例模式的共享内存对象到对象管理系统
 * @param ClassName 要注册的单例类名
 * 
 * @note 使用方式：
 * @code
 * REGISTER_SINGLETON_SHM_OBJ(MySingletonClass);
 * @endcode
 * 
 * @warning 注意事项：
 *          - 单例对象在整个系统中只有一个实例
 *          - 类必须已经定义了GetStaticClassType()等静态方法
 *          - 适用于管理器、配置等全局唯一的对象
 */
#define REGISTER_SINGLETON_SHM_OBJ(ClassName) do{\
        ClassName::RegisterClassToObjSeg(ClassName::GetStaticClassType(), sizeof(ClassName), 1, std::string(#ClassName), false, true);\
    }while(0)

/**
 * @brief 注册带哈希的共享内存对象宏
 * @details 用于注册支持哈希查找的共享内存对象到对象管理系统
 * @param ClassName 要注册的类名
 * @param ObjNum 对象池中该类型对象的数量
 * 
 * @note 使用方式：
 * @code
 * REGISTER_SHM_OBJ_WITH_HASH(MyHashClass, 1000);
 * @endcode
 * 
 * @warning 注意事项：
 *          - 带哈希的对象支持通过哈希键快速查找
 *          - 对象必须正确设置和管理哈希键
 *          - 哈希键必须在对象池中唯一
 *          - 适用于需要频繁查找的对象类型
 */
#define REGISTER_SHM_OBJ_WITH_HASH( ClassName, ObjNum ) do{\
        ClassName::RegisterClassToObjSeg(ClassName::GetStaticClassType(), sizeof(ClassName), (ObjNum), std::string(#ClassName), true);\
    }while(0)

/**
 * @brief 反注册共享内存对象宏
 * @details 用于从共享内存管理器中移除对象类型的注册信息
 * @param ClassName 要反注册的类名
 * @param ObjNum 对象数量（当前实现中未使用）
 * 
 * @note 使用方式：
 * @code
 * UNREGISTER_SHM_OBJ(MyClass, 1000);
 * @endcode
 * 
 * @warning 使用注意事项：
 *          - 必须在确保没有该类型对象被使用时调用
 *          - 反注册后不能再创建该类型的对象
 *          - 通常在系统关闭或模块卸载时使用
 *          - 错误的反注册可能导致系统不稳定
 */
#define UNREGISTER_SHM_OBJ(ClassName, ObjNum ) do{\
        ClassName::UnRegisterClassToObjSeg(ClassName::GetStaticClassType());\
    }while(0)


