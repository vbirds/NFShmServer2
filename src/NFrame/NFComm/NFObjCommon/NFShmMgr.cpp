// -------------------------------------------------------------------------
//    @FileName         :    NFShmMgr.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

/**
 * @file NFShmMgr.cpp
 * @brief 共享内存管理器实现文件
 * @details 实现了NFrame框架中共享内存管理器NFShmMgr的核心功能，提供了共享内存
 *          对象创建模式和运行模式的全局管理。作为单例类，负责协调整个框架的
 *          共享内存对象的生命周期和状态管理。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 实现的主要功能：
 *       - 共享内存对象创建模式管理
 *       - 共享内存对象运行模式管理
 *       - 地址偏移量管理（用于指针恢复）
 *       - 单例模式的生命周期管理
 *       - 全局状态的访问接口
 * 
 * @warning 使用注意事项：
 *          - 这是全局单例，影响所有对象的创建
 *          - 创建模式决定对象是初始化还是恢复
 *          - 地址偏移量用于共享内存恢复时修正指针
 *          - 模式设置应该在对象创建前完成
 */

#include "NFShmMgr.h"

NFShmMgr::NFShmMgr()
{
    m_objCreateMode = EN_OBJ_MODE_INIT;
    m_objRunMode = EN_OBJ_MODE_RECOVER;
    m_siAddrOffset = 0;
}

NFShmMgr::~NFShmMgr()
{
}

/**
 * @brief 获取当前的对象创建模式
 * @details 返回当前共享内存对象的创建模式，决定对象是进行初始化还是恢复
 * @return EN_OBJ_MODE 当前的对象创建模式
 *         - EN_OBJ_MODE_INIT: 初始化模式，新创建对象
 *         - EN_OBJ_MODE_RECOVER: 恢复模式，从共享内存恢复对象
 */
EN_OBJ_MODE NFShmMgr::GetCreateMode() const
{
    return m_objCreateMode;
}

/**
 * @brief 设置对象创建模式
 * @details 设置共享内存对象的创建模式，影响后续创建的所有对象
 * @param mode 要设置的对象创建模式
 *        - EN_OBJ_MODE_INIT: 初始化模式
 *        - EN_OBJ_MODE_RECOVER: 恢复模式
 * @warning 必须在对象创建前设置，否则可能导致对象状态不一致
 */
void NFShmMgr::SetCreateMode(EN_OBJ_MODE mode)
{
    m_objCreateMode = mode;
}

/**
 * @brief 获取当前的对象运行模式
 * @details 返回当前共享内存对象的运行模式
 * @return EN_OBJ_MODE 当前的对象运行模式
 */
EN_OBJ_MODE NFShmMgr::GetRunMode() const
{
    return m_objRunMode;
}

/**
 * @brief 设置对象运行模式
 * @details 设置共享内存对象的运行模式
 * @param mode 要设置的对象运行模式
 */
void NFShmMgr::SetRunMode(EN_OBJ_MODE mode)
{
    m_objRunMode = mode;
}

/**
 * @brief 获取地址偏移量
 * @details 返回共享内存恢复时使用的地址偏移量，用于指针修正
 * @return size_t 地址偏移量（字节数）
 * @note 在共享内存恢复时，所有指针都需要加上这个偏移量来适应新的内存布局
 */
size_t NFShmMgr::GetAddrOffset() const
{
    return m_siAddrOffset;
}

/**
 * @brief 设置地址偏移量
 * @details 设置共享内存恢复时使用的地址偏移量
 * @param offset 地址偏移量（字节数）
 * @note 这个值用于共享内存恢复时修正所有指针地址
 */
void NFShmMgr::SetAddrOffset(size_t offset)
{
    m_siAddrOffset = offset;
}
