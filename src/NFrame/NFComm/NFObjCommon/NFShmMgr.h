// -------------------------------------------------------------------------
//    @FileName         :    NFShmMgr.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

/**
 * @file NFShmMgr.h
 * @brief 共享内存管理器定义文件
 * @details 定义了NFrame框架中的共享内存管理器NFShmMgr，作为单例类管理整个框架的
 *          共享内存模式和状态。主要用于解决C++类构造函数、析构函数调用虚函数的问题，
 *          同时提供共享内存对象创建模式的全局控制。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFShmMgr: 共享内存管理器单例类
 *       - 对象创建模式管理（初始化/恢复）
 *       - 对象运行模式管理
 *       - 地址偏移量管理（用于指针恢复）
 * 
 * @warning 使用时需要注意：
 *          - 这是一个单例类，全局只有一个实例
 *          - 创建模式和运行模式需要正确设置
 *          - 地址偏移量用于共享内存恢复时的指针修正
 *          - 不要在构造函数中调用虚函数
 */

#pragma once

#include "NFComm/NFCore/NFSingleton.hpp"
#include "NFObjDefine.h"

/**
 * @class NFShmMgr
 * @brief 共享内存管理器单例类
 * @details 负责管理整个框架的共享内存对象创建模式和运行模式。由于C++类构造函数、析构函数
 *          调用虚函数的问题，以及不希望NFShmObj类的构造函数包含太多参数，所以使用这个
 *          管理器来传递一些临时参数和全局状态。
 * 
 * @note 关键功能：
 *       - 管理对象创建模式（初始化模式或恢复模式）
 *       - 管理对象运行模式
 *       - 管理地址偏移量用于共享内存恢复
 *       - 单例模式确保全局状态一致性
 * 
 * @warning 注意事项：
 *          - 全局单例，影响所有共享内存对象的创建
 *          - 模式设置必须在对象创建前完成
 *          - 地址偏移量用于共享内存恢复时的指针修正
 */
class NFShmMgr final : public NFSingleton<NFShmMgr>
{
public:
	/**
	 * @brief 构造函数
	 * @details 初始化共享内存管理器，设置默认的创建模式和运行模式
	 */
	NFShmMgr();
	
	/**
	 * @brief 析构函数
	 * @details 清理共享内存管理器资源
	 */
	~NFShmMgr() override;

public:
	/**
	 * @brief 获取当前的对象创建模式
	 * @details 返回当前共享内存对象的创建模式，决定对象是进行初始化还是恢复
	 * @return EN_OBJ_MODE 当前的对象创建模式
	 *         - EN_OBJ_MODE_INIT: 初始化模式，新创建对象
	 *         - EN_OBJ_MODE_RECOVER: 恢复模式，从共享内存恢复对象
	 */
	EN_OBJ_MODE	GetCreateMode() const;

	/**
	 * @brief 设置对象创建模式
	 * @details 设置共享内存对象的创建模式，影响后续创建的所有对象
	 * @param mode 要设置的对象创建模式
	 *        - EN_OBJ_MODE_INIT: 初始化模式
	 *        - EN_OBJ_MODE_RECOVER: 恢复模式
	 * @warning 必须在对象创建前设置，否则可能导致对象状态不一致
	 */
	void SetCreateMode(EN_OBJ_MODE mode);

	/**
	 * @brief 获取当前的对象运行模式
	 * @details 返回当前共享内存对象的运行模式
	 * @return EN_OBJ_MODE 当前的对象运行模式
	 */
	EN_OBJ_MODE	GetRunMode() const;

	/**
	 * @brief 设置对象运行模式
	 * @details 设置共享内存对象的运行模式
	 * @param mode 要设置的对象运行模式
	 */
	void SetRunMode(EN_OBJ_MODE mode);

    /**
     * @brief 获取地址偏移量
     * @details 获取相对于上次共享内存地址恢复后的偏移量，用于恢复指针对象
     * @return size_t 当前的地址偏移量
     * @note 主要用于共享内存恢复时修正指针地址
     */
    size_t GetAddrOffset() const;

    /**
     * @brief 设置地址偏移量
     * @details 设置地址偏移量，用于共享内存恢复时的指针修正
     * @param offset 要设置的地址偏移量
     * @note 这个偏移量在共享内存恢复时用于修正所有指针的地址
     */
    void SetAddrOffset(size_t offset);

public:
    /**
     * @brief 对象创建模式
     * @details 控制共享内存对象的创建行为
     * @note 可能的值：
     *       - EN_OBJ_MODE_INIT: 初始化模式
     *       - EN_OBJ_MODE_RECOVER: 恢复模式
     */
    EN_OBJ_MODE m_objCreateMode;
    
    /**
     * @brief 对象运行模式
     * @details 控制共享内存对象的运行行为
     */
    EN_OBJ_MODE m_objRunMode;
    
    /**
     * @brief 地址偏移量
     * @details 相对于上次共享内存地址恢复后的偏移量，用来恢复指针对象
     * @note 在共享内存重新映射时，所有指针都需要根据这个偏移量进行调整
     */
    size_t m_siAddrOffset;

public:
    /**
     * @brief 类型标识
     * @details 用于标识管理器的类型或状态
     */
    int m_iType;
};

