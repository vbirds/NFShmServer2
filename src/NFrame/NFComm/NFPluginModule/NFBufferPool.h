// -------------------------------------------------------------------------
//    @FileName         :    NFBufferPool.h
//    @Author           :    gaoyi
//    @Date             :    2022/10/10
//    @Email			:    445267987@qq.com
//    @Module           :    NFBufferPool
//    @Description      :    高性能内存缓冲池，提供分块内存管理和内存泄漏检测
//                           基于不同大小的内存块池实现零碎片、高效率的内存分配
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFChunkPool.h"
#include "NFComm/NFCore/NFSingleton.hpp"

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

/**
 * @brief 高性能内存缓冲池类
 * 
 * NFBufferPool 提供了基于分块的高效内存管理系统：
 * 
 * 1. 核心设计特点：
 *    - 分层内存池架构：根据请求大小自动选择合适的内存池
 *    - 零碎片化设计：预分配固定大小的内存块，避免内存碎片
 *    - 高效的分配算法：O(1)时间复杂度的内存分配和释放
 *    - 内存对齐优化：确保内存访问的最佳性能
 * 
 * 2. 内存管理策略：
 *    - 自适应大小选择：根据请求大小选择最合适的内存块
 *    - 池化复用机制：释放的内存块返回池中重复使用
 *    - 动态扩容支持：内存不足时自动分配新的内存块
 *    - 内存统计监控：实时跟踪内存使用情况
 * 
 * 3. 调试和诊断功能：
 *    - 内存泄漏检测：跟踪每个内存分配的文件名、函数名和行号
 *    - 分配历史记录：维护详细的内存分配日志
 *    - 使用统计报告：提供内存使用情况的详细统计
 *    - 调试模式支持：开发时启用额外的检查和日志
 * 
 * 4. 性能优化特性：
 *    - 无锁设计：在单线程环境下避免锁开销
 *    - 预分配策略：启动时预分配常用大小的内存块
 *    - 局部性优化：相关内存块在物理上相邻分配
 *    - 快速查找：使用索引数组快速定位合适的内存池
 * 
 * 5. 使用场景：
 *    - 游戏服务器的高频内存分配
 *    - 网络数据包的缓冲管理
 *    - 临时对象的快速分配
 *    - 大量小对象的内存池化
 * 
 * 6. 配置参数：
 *    - block_size: 单个内存块的大小，影响分配效率
 *    - max_size: 最大支持的单次分配大小
 *    - 池数量: 根据常用大小配置不同的内存池
 * 
 * 内存池架构：
 * ```
 * NFBufferPool
 * ├── ChunkPool[0]    (1-32 bytes)
 * ├── ChunkPool[1]    (33-64 bytes)
 * ├── ChunkPool[2]    (65-128 bytes)
 * ├── ...
 * └── ChunkPool[N]    (large blocks)
 * ```
 * 
 * 使用示例：
 * @code
 * // 创建缓冲池
 * NFBufferPool pool(1024*1024, 32*1024);  // 1MB块大小，32KB最大分配
 * 
 * // 分配内存
 * void* ptr = pool.Alloc(256);
 * 
 * // 重新分配
 * ptr = pool.Realloc(ptr, 512);
 * 
 * // 释放内存
 * pool.Free(ptr);
 * 
 * // 调试模式下的内存跟踪
 * #ifdef CHECK_MEM_LEAK
 * void* debugPtr = pool.MallocBuf(128);  // 自动记录文件名和行号
 * pool.FreeBuf(debugPtr);
 * #endif
 * @endcode
 * 
 * 设计优势：
 * - 消除内存碎片化问题
 * - 提供可预测的内存分配性能
 * - 简化内存泄漏检测和调试
 * - 支持高并发的内存分配需求
 * - 提供详细的内存使用统计
 */
class NFBufferPool
{
public:
    /**
     * @brief 构造函数
     * @param block_size 内存块大小，默认1MB，影响内存池的粒度和效率
     * @param max_size 最大分配大小，默认32KB，超过此大小的分配将失败
     * 
     * 初始化缓冲池并设置基本参数：
     * - 根据max_size计算需要的内存池数量
     * - 为每个大小级别创建对应的ChunkPool
     * - 设置内存对齐和分配策略
     * 
     * 设计考虑：
     * - block_size越大，内存浪费可能越多，但减少了内存池数量
     * - max_size限制了单次分配的最大内存，有助于防止内存滥用
     * - 内存池数量通过2的幂次方划分，便于快速定位
     */
    explicit NFBufferPool(uint32_t block_size = 1024 * 1024,
                          uint32_t max_size   = 32*1024);
    
    /**
     * @brief 析构函数
     * 
     * 清理所有内存池资源：
     * - 释放所有已分配的内存块
     * - 销毁所有ChunkPool对象
     * - 在调试模式下检查内存泄漏
     * - 输出内存使用统计信息
     */
    ~NFBufferPool();

    /**
     * @brief 分配指定大小的内存
     * @param size 需要分配的字节数
     * @return 分配成功返回内存指针，失败返回nullptr
     * 
     * 内存分配流程：
     * 1. 检查请求大小是否超过max_size限制
     * 2. 根据请求大小计算应该使用的内存池索引
     * 3. 从对应的ChunkPool中分配内存块
     * 4. 如果当前池没有可用内存，尝试扩容或分配新池
     * 5. 返回对齐后的内存地址
     * 
     * 性能特点：
     * - O(1)时间复杂度
     * - 内存自动对齐
     * - 支持内存池的动态扩容
     * 
     * 注意事项：
     * - 分配的内存已经初始化为0
     * - 返回的指针需要调用Free释放
     * - 大于max_size的请求会失败
     */
    void* Alloc(size_t size);
    
    /**
     * @brief 重新分配内存
     * @param ptr 原有内存指针，可以为nullptr
     * @param size 新的内存大小
     * @return 重新分配成功返回新内存指针，失败返回nullptr
     * 
     * 重新分配策略：
     * 1. 如果ptr为nullptr，等同于Alloc(size)
     * 2. 如果size为0，等同于Free(ptr)
     * 3. 获取原内存块的大小信息
     * 4. 如果新大小适合原内存池，直接返回原指针
     * 5. 否则分配新内存，复制数据，释放原内存
     * 
     * 优化策略：
     * - 尽量避免数据拷贝
     * - 复用原有内存空间
     * - 保持数据的完整性
     * 
     * 安全保证：
     * - 数据拷贝时确保不会溢出
     * - 原内存在拷贝完成后才释放
     * - 失败时原内存保持不变
     */
    void* Realloc(void* ptr, size_t size);
    
    /**
     * @brief 释放内存
     * @param ptr 要释放的内存指针
     * 
     * 内存释放流程：
     * 1. 检查指针的有效性
     * 2. 根据内存地址确定所属的内存池
     * 3. 将内存块返回给对应的ChunkPool
     * 4. 更新内存使用统计
     * 
     * 设计特点：
     * - 快速的内存池定位算法
     * - 内存块的自动归还和复用
     * - 调试模式下的双重释放检测
     * 
     * 注意事项：
     * - 不要释放同一指针多次
     * - 只能释放通过本池分配的内存
     * - 释放后不要再使用该指针
     */
    void  Free(void* ptr);

public:
    /**
     * @brief 带调试信息的内存分配
     * @param size 需要分配的字节数
     * @param file 调用文件名
     * @param func 调用函数名
     * @param line 调用行号
     * @return 分配成功返回内存指针，失败返回nullptr
     * 
     * 调试模式下的增强功能：
     * - 记录每次分配的详细调用信息
     * - 建立内存地址到调用栈的映射
     * - 支持内存泄漏的精确定位
     * - 提供分配历史的查询接口
     * 
     * 调试信息包含：
     * - 分配时的文件名和行号
     * - 调用函数的名称
     * - 分配的内存大小
     * - 分配的时间戳
     * 
     * 使用场景：
     * - 开发阶段的内存泄漏检测
     * - 性能分析和内存使用优化
     * - 问题复现和调试支持
     */
    void* AllocTrack(size_t size,
                     const char* file,
                     const char* func,
                     uint32_t line);
                     
    /**
     * @brief 带调试信息的内存重新分配
     * @param ptr 原有内存指针
     * @param size 新的内存大小
     * @param file 调用文件名
     * @param func 调用函数名
     * @param line 调用行号
     * @return 重新分配成功返回新内存指针，失败返回nullptr
     * 
     * 调试版本的重新分配功能：
     * - 更新内存分配的调试信息
     * - 跟踪内存大小的变化历史
     * - 记录重新分配的调用栈
     * - 检测内存使用模式的异常
     */
    void* ReallocTrack(void* ptr,
                       size_t size,
                       const char* file,
                       const char* func,
                       uint32_t line);
                       
    /**
     * @brief 带调试信息的内存释放
     * @param ptr 要释放的内存指针
     * 
     * 调试版本的内存释放功能：
     * - 验证释放地址的合法性
     * - 检测双重释放的问题
     * - 清理调试信息和跟踪记录
     * - 统计内存的生命周期信息
     * 
     * 安全检查：
     * - 确认指针确实由本池分配
     * - 检测是否已经被释放过
     * - 验证内存块的完整性
     * - 清除敏感数据防止泄露
     */
    void  FreeTrack(void* ptr);

private:
    /**
     * @brief 分配内存池数组
     * 
     * 初始化内存池管理结构：
     * - 根据max_size计算需要的池数量
     * - 为每个大小级别创建ChunkPool
     * - 设置池之间的大小递增关系
     * - 初始化池的统计和管理信息
     */
    void   AllocChunkPool();
    
    /**
     * @brief 释放所有内存池
     * 
     * 清理所有内存池资源：
     * - 释放每个ChunkPool的内存
     * - 销毁池管理结构
     * - 清理统计信息
     * - 检查未释放的内存块
     */
    void   ReleaseChunkPool();
    
    /**
     * @brief 获取内存块的长度信息
     * @param ptr 内存块指针
     * @return 内存块的实际大小
     * 
     * 通过内存块的头部信息获取其大小：
     * - 每个分配的内存块都包含大小信息
     * - 支持快速的大小查询
     * - 用于重新分配时的大小比较
     * - 提供内存使用统计的基础数据
     */
    size_t GetBufferLength(void* ptr);

public:
    typedef uint16_t IndexType;  ///< 内存池索引类型，支持65536个不同大小的池

private:
    IndexType   array_num_;      ///< 内存池数组的数量
    size_t      max_size_;       ///< 支持的最大分配大小
    uint32_t    block_size_;     ///< 单个内存块的大小
    NFChunkPool** mini_mem_alloc_; ///< 内存池指针数组，每个元素管理特定大小的内存块
};

/// @brief 内存分配宏定义 - 根据是否开启内存泄漏检测选择不同的实现

#ifdef CHECK_MEM_LEAK
/// @brief 调试模式下的内存分配宏，自动记录文件名、函数名和行号
#define  MallocBuf(SIZET)         AllocTrack(SIZET, __FILE__, __FUNCTION__, __LINE__)
/// @brief 调试模式下的内存重新分配宏，自动记录调试信息
#define  ReallocBuf(PTRT, SIZET)  ReallocTrack(PTRT, SIZET, __FILE__, __FUNCTION__, __LINE__)
/// @brief 调试模式下的内存释放宏
#define  FreeBuf                  FreeTrack
#else
/// @brief 发布模式下的内存分配宏，直接调用高效版本
#define  MallocBuf(SIZET)         Alloc(SIZET)
/// @brief 发布模式下的内存重新分配宏
#define  ReallocBuf(PTRT, SIZET)  Realloc(PTRT, SIZET)
/// @brief 发布模式下的内存释放宏
#define  FreeBuf                  Free
#endif  // CHECK_MEM_LEAK


