// -------------------------------------------------------------------------
//    @FileName         :    NFObjectPool.hpp
//    @Author           :    gaoyi
//    @Date             :    2022/10/10
//    @Email			:    445267987@qq.com
//    @Module           :    NFObjectPool
//    @Description      :    高性能对象池模板，提供类型安全的对象分配和释放
//
// -------------------------------------------------------------------------

/**
 * @file NFObjectPool.hpp
 * @brief 高性能对象池模板实现
 * @details 该文件实现了一个基于内存池的高性能对象池，主要特性包括：
 * 
 * **核心优势：**
 * - **高性能**：基于内存池，避免频繁的系统内存分配
 * - **类型安全**：模板实现，编译期类型检查
 * - **内存跟踪**：可选的内存泄漏检测支持
 * - **自动构造**：支持带参数的对象构造
 * - **零碎片**：预分配连续内存，减少内存碎片
 * 
 * **技术实现：**
 * - 继承自NFChunkPool，复用内存池基础设施
 * - 使用placement new进行就地构造
 * - 支持可变参数模板，灵活的对象初始化
 * - 条件编译支持，开发和生产环境差异化
 * 
 * **适用场景：**
 * - 频繁创建和销毁的对象
 * - 游戏中的实体、消息、缓冲区等
 * - 网络编程中的数据包对象
 * - 需要性能优化的数据结构
 * 
 * **使用模式：**
 * @code
 * // 创建对象池
 * NFObjectPool<MyClass> pool(1000);  // 预分配1000个对象
 * 
 * // 分配对象
 * MyClass* obj = pool.MallocObj();
 * 
 * // 使用对象
 * obj->DoSomething();
 * 
 * // 释放对象
 * pool.FreeObj(obj);
 * @endcode
 * 
 * @author gaoyi
 * @date 2022/10/10
 * @version 1.0
 */

#pragma once

#include "NFChunkPool.h"
#include "NFMemTracker.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @class NFObjectPool
 * @brief 高性能对象池模板类
 * @tparam TYPE 要管理的对象类型
 * @details 基于内存池技术的对象池实现，提供高效的对象分配和释放机制。
 * 
 * **核心特性：**
 * - **内存池基础**：继承NFChunkPool，获得高效的内存管理能力
 * - **类型安全**：模板设计确保类型安全和编译期优化
 * - **对象生命周期**：自动调用构造函数和析构函数
 * - **内存跟踪**：可选的内存泄漏检测和调试支持
 * - **性能优化**：最小化内存分配开销和内存碎片
 * 
 * **设计原理：**
 * - 预分配大块连续内存
 * - 使用链表管理空闲内存块
 * - placement new实现就地对象构造
 * - 支持对象池的自动扩展
 * 
 * **内存布局：**
 * - 连续的内存块用于存储对象
 * - 每个块大小等于sizeof(TYPE)
 * - 空闲块通过链表连接
 * - 支持多个内存页的管理
 * 
 * **线程安全：**
 * @warning 该类不是线程安全的，多线程使用需要外部同步
 * 
 * **性能特点：**
 * - 分配：O(1)时间复杂度
 * - 释放：O(1)时间复杂度
 * - 内存使用：接近理论最优
 * - CPU缓存友好的内存访问模式
 */
template<class TYPE>
class NFObjectPool : public NFChunkPool
{
public:
    /**
     * @brief 构造函数
     * @param count_in_block 每个内存块中对象的数量，0表示使用默认值
     * @param auto_free 是否自动释放内存，默认为true
     * 
     * **功能说明：**
     * 初始化对象池，设置内存分配策略和管理参数。
     * 
     * **参数说明：**
     * - count_in_block：控制内存分配粒度，影响性能和内存使用
     * - auto_free：控制析构时是否自动释放所有内存
     * 
     * **初始化过程：**
     * - 计算对象大小和对齐要求
     * - 设置内存块分配策略
     * - 初始化空闲链表管理
     * - 准备内存跟踪机制
     */
    NFObjectPool(uint32_t count_in_block = 0, bool auto_free = true);

    /**
     * @brief 析构函数
     * 
     * 清理对象池资源，确保所有分配的对象都被正确析构。
     * 
     * **清理过程：**
     * - 调用所有活跃对象的析构函数
     * - 释放所有内存池资源
     * - 清理内存跟踪记录
     * - 重置内部状态
     * 
     * @warning 确保所有对象都已正确释放，否则可能导致资源泄漏
     */
    virtual ~NFObjectPool();

    /**
     * @brief 分配对象（默认构造）
     * @return 指向新分配对象的指针，失败时返回nullptr
     * 
     * **功能说明：**
     * 从对象池中分配一个对象，并使用默认构造函数进行初始化。
     * 
     * **分配过程：**
     * 1. 从内存池获取空闲内存块
     * 2. 使用placement new调用默认构造函数
     * 3. 返回构造完成的对象指针
     * 4. 更新内存池统计信息
     * 
     * **性能特点：**
     * - O(1)时间复杂度
     * - 无系统调用开销
     * - 内存局部性好，缓存友好
     * 
     * **错误处理：**
     * - 内存不足时返回nullptr
     * - 构造函数异常时安全处理
     * 
     * @note 返回的对象已完全初始化，可以直接使用
     */
    TYPE *Alloc();

    /**
     * @brief 释放对象
     * @param obj 要释放的对象指针
     * @return 释放是否成功
     * 
     * **功能说明：**
     * 将对象归还给对象池，并调用其析构函数进行清理。
     * 
     * **释放过程：**
     * 1. 验证对象指针的有效性
     * 2. 调用对象的析构函数
     * 3. 将内存块归还给内存池
     * 4. 更新统计信息
     * 
     * **安全检查：**
     * - 空指针检查
     * - 重复释放检测
     * - 内存边界验证
     * 
     * **性能特点：**
     * - O(1)时间复杂度
     * - 无系统调用开销
     * - 立即可重用内存
     * 
     * @warning 释放后不得再使用该对象指针
     * @note 析构函数会被自动调用
     */
    bool Free(TYPE *obj);

    /**
     * @brief 就地构造对象
     * @tparam U 构造的对象类型（通常与TYPE相同）
     * @tparam Args 构造函数参数类型包
     * @param p 要构造对象的内存位置
     * @param args 构造函数参数
     * 
     * **功能说明：**
     * 在指定内存位置使用给定参数构造对象，支持完美转发。
     * 
     * **技术实现：**
     * - 使用placement new进行就地构造
     * - 完美转发构造参数，避免不必要的拷贝
     * - 支持任意构造函数签名
     * 
     * **适用场景：**
     * - 自定义对象初始化逻辑
     * - 带参数的对象构造
     * - 复杂对象的分步构造
     * 
     * **使用示例：**
     * @code
     * void* mem = pool.AllocChunk();
     * pool.Construct<MyClass>(mem, arg1, arg2, arg3);
     * @endcode
     */
    template<class U, class... Args>
    void Construct(U *p, Args &&... args);

public:
    // ========================================================================
    // 内存跟踪版本的接口
    // ========================================================================
    
    /**
     * @brief 分配对象并跟踪内存（带参数构造）
     * @tparam Args 构造函数参数类型包
     * @param file 调用所在文件名
     * @param func 调用所在函数名
     * @param line 调用所在行号
     * @param args 构造函数参数
     * @return 指向新分配对象的指针
     * 
     * **功能说明：**
     * 结合内存跟踪功能的对象分配方法，用于开发阶段的内存泄漏检测。
     * 
     * **跟踪信息：**
     * - 分配位置的源码信息
     * - 对象类型和大小
     * - 分配时间戳
     * - 调用堆栈（如果启用）
     * 
     * **使用场景：**
     * - 开发阶段的内存调试
     * - 内存泄漏检测
     * - 性能分析和优化
     * 
     * **条件编译：**
     * 只在CHECK_MEM_LEAK宏定义时生效，release版本会被替换为普通分配。
     * 
     * @note 该方法会自动记录内存分配信息
     * @see MallocObj宏提供了更便捷的调用方式
     */
    template<class... Args>
    TYPE *AllocTrack(const char *file,
                     const char *func,
                     uint32_t line, Args &&... args);

    /**
     * @brief 释放对象并移除跟踪记录
     * @param obj 要释放的对象指针
     * @return 释放是否成功
     * 
     * **功能说明：**
     * 释放对象并从内存跟踪系统中移除相应的记录。
     * 
     * **跟踪更新：**
     * - 验证释放操作的有效性
     * - 从跟踪系统移除分配记录
     * - 检测潜在的内存问题
     * - 更新泄漏统计信息
     * 
     * **错误检测：**
     * - 重复释放检测
     * - 未分配指针检测
     * - 类型不匹配检测
     * 
     * @note 与AllocTrack成对使用
     * @see FreeObj宏提供了更便捷的调用方式
     */
    bool FreeTrack(TYPE *obj);

    /**
     * @brief 获取当前内存使用量
     * @return 当前使用的内存字节数
     * 
     * **统计信息：**
     * - 包括所有已分配的对象内存
     * - 包括内存池的管理开销
     * - 实时更新的准确数值
     * 
     * **用途：**
     * - 内存使用监控
     * - 性能分析和优化
     * - 资源管理决策
     */
    uint32_t MemSize()
    {
        return m_memSize;
    }
};

// ========================================================================
// 便捷宏定义 - 根据编译模式选择不同的实现
// ========================================================================

#ifdef CHECK_MEM_LEAK
    /// @brief 分配对象宏（内存跟踪版本），自动传递源码位置信息
    #define  MallocObj()        AllocTrack(__FILE__, __FUNCTION__, __LINE__)
    /// @brief 带参数分配对象宏（内存跟踪版本），支持构造函数参数
    #define  MallocObjWithArgs(...)        AllocTrack(__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    /// @brief 释放对象宏（内存跟踪版本），自动记录释放操作
    #define  FreeObj            FreeTrack
#else
    /// @brief 分配对象宏（高性能版本），直接调用分配函数
    #define  MallocObj()        Alloc()
    /// @brief 释放对象宏（高性能版本），直接调用释放函数
    #define  FreeObj            Free
#endif  // CHECK_MEM_LEAK

// ========================================================================
// 模板方法实现
// ========================================================================

/**
 * @brief 构造函数实现
 * @details 初始化对象池，继承自NFChunkPool的基础功能
 */
template<class TYPE>
inline
NFObjectPool<TYPE>::NFObjectPool(uint32_t count_in_block,
                             bool auto_free /*= true*/)
        :NFChunkPool(0, sizeof(TYPE), count_in_block, auto_free)
{

}

/**
 * @brief 析构函数实现
 * @details 清理所有对象池资源
 */
template<class TYPE>
inline NFObjectPool<TYPE>::~NFObjectPool(void) {}

/**
 * @brief 对象分配实现
 * @details 从内存池分配内存并构造对象
 */
template<class TYPE>
inline TYPE *NFObjectPool<TYPE>::Alloc()
{
    TYPE *obj = (TYPE *) AllocChunk();
    if (obj)
    {
        new(obj) TYPE();
    }
    return obj;
}

/**
 * @brief 对象释放实现
 * @details 析构对象并归还内存给池
 */
template<class TYPE>
inline bool NFObjectPool<TYPE>::Free(TYPE *obj)
{
    NF_ASSERT(obj);
    obj->~TYPE();
    return FreeChunk(obj);
}

/**
 * @brief 就地构造实现
 * @details 使用placement new和完美转发构造对象
 */
template<typename T>
template<class U, class... Args>
inline void
NFObjectPool<T>::Construct(U *p, Args &&... args)
{
    new(p) U(std::forward<Args>(args)...);
}

/**
 * @brief 跟踪分配实现
 * @details 分配对象并记录到内存跟踪系统
 */
template<class TYPE>
template<class... Args>
inline TYPE *NFObjectPool<TYPE>::AllocTrack(const char *file,
                                          const char *func,
                                          uint32_t line, Args &&... args)
{
    TYPE *obj = (TYPE *) AllocChunk();
    if (obj)
    {
        Construct<TYPE>(obj, std::forward<Args>(args)...);
        NFMemTracker::Instance()->TrackMalloc(obj, file, func, line);
    }
    return obj;
}

/**
 * @brief 跟踪释放实现
 * @details 析构对象、移除跟踪记录并归还内存
 */
template<class TYPE>
inline bool NFObjectPool<TYPE>::FreeTrack(TYPE *obj)
{
    NF_ASSERT(obj);
    obj->~TYPE();
    NFMemTracker::Instance()->TrackFree(obj);
    return FreeChunk(obj);
}



