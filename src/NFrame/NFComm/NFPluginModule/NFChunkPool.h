// -------------------------------------------------------------------------
//    @FileName         :    NFChunkPool.h
//    @Author           :    gaoyi
//    @Date             :    2022/10/10
//    @Email			:    445267987@qq.com
//    @Module           :    NFChunkPool
//    @Description      :    固定大小内存块池，提供高效的内存分配和管理
//                           基于双向链表实现的内存池，支持快速分配和释放
//
// -------------------------------------------------------------------------

#pragma once

#include <vector>

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFDoubleList.h"

/**
 * @brief 内存块描述结构
 * 
 * NFMemBlock 描述了内存池中的一个内存块的状态和管理信息：
 * 
 * 1. 设计目的：
 *    - 跟踪内存块的使用状态
 *    - 管理内存块内的空闲chunk链表
 *    - 提供内存块的统计信息
 *    - 支持内存块的链表管理
 * 
 * 2. 核心字段：
 *    - m_freeIdx: 下一个可分配chunk的索引
 *    - m_freeNum: 当前可用chunk的数量
 *    - m_freeList: 空闲chunk的链表头
 *    - m_blockList: 内存块在全局链表中的节点
 * 
 * 3. 状态管理：
 *    - 空内存块：m_freeNum等于最大chunk数
 *    - 满内存块：m_freeNum为0
 *    - 部分使用：0 < m_freeNum < 最大chunk数
 * 
 * 4. 链表集成：
 *    - 通过m_blockList集成到NFDoubleList
 *    - 支持O(1)的插入和删除操作
 *    - 便于内存块的分类管理
 */
struct NFMemBlock {
    uint32_t m_freeIdx;         ///< 下一个可分配chunk的索引位置
    uint32_t m_freeNum;         ///< 当前内存块中空闲chunk的数量
    void *m_freeList;           ///< 指向空闲chunk链表的头指针
    NFDoubleNode m_blockList;   ///< 双向链表节点，用于将内存块加入管理链表

    /**
     * @brief 默认构造函数
     * 
     * 初始化内存块为空状态：
     * - 重置所有计数器为0
     * - 清空空闲链表指针
     * - 初始化双向链表节点
     */
    NFMemBlock()
        : m_freeIdx(0),
          m_freeNum(0),
          m_freeList(nullptr),
          m_blockList(reinterpret_cast<void *>(this)) {
    }
};

/**
 * @brief 高性能内存块池类
 * 
 * NFChunkPool 提供了固定大小内存块的高效分配管理：
 * 
 * 1. 核心设计理念：
 *    - 固定大小分配：所有chunk大小相同，消除碎片化
 *    - 预分配策略：启动时预分配内存块，减少运行时开销
 *    - 分层管理：使用多个链表管理不同状态的内存块
 *    - 快速分配：O(1)时间复杂度的分配和释放操作
 * 
 * 2. 内存管理策略：
 *    - 块分组管理：将chunk组织成固定大小的内存块
 *    - 三级链表：空闲、部分使用、完全使用的内存块分类
 *    - 自动扩容：内存不足时自动分配新的内存块
 *    - 内存回收：支持向系统归还不需要的内存
 * 
 * 3. 数据结构组织：
 *    ```
 *    NFChunkPool
 *    ├── m_emptyList    (完全空闲的内存块)
 *    ├── m_blockList    (部分使用的内存块)
 *    └── m_fullList     (完全使用的内存块)
 *    ```
 * 
 * 4. 内存块内部结构：
 *    ```
 *    MemoryBlock
 *    ├── Block Header   (NFMemBlock管理信息)
 *    ├── Chunk[0]      (第一个chunk)
 *    ├── Chunk[1]      (第二个chunk)
 *    ├── ...
 *    └── Chunk[N-1]    (最后一个chunk)
 *    ```
 * 
 * 5. 分配算法：
 *    - 优先从部分使用的内存块分配
 *    - 其次从完全空闲的内存块分配
 *    - 最后分配新的内存块
 *    - 分配后自动更新内存块状态
 * 
 * 6. 释放算法：
 *    - 根据地址定位到所属内存块
 *    - 将chunk加入空闲链表
 *    - 更新内存块状态和统计信息
 *    - 必要时进行内存块状态迁移
 * 
 * 7. 性能优化特性：
 *    - 内存对齐：确保chunk地址按cache line对齐
 *    - 局部性优化：相关chunk在物理上相邻
 *    - 分支预测友好：热路径代码优化
 *    - 无锁设计：单线程环境下避免同步开销
 * 
 * 8. 应用场景：
 *    - 固定大小对象的频繁分配（如网络消息、临时缓冲区）
 *    - 实时系统的确定性内存分配
 *    - 游戏引擎的对象池管理
 *    - 数据库系统的页面管理
 * 
 * 使用示例：
 * @code
 * // 创建内存池：预留10个块，每个chunk 64字节，每块1024个chunk，释放时归还系统
 * NFChunkPool pool(10, 64, 1024, true);
 * 
 * // 分配内存
 * void* ptr1 = pool.AllocChunk();
 * void* ptr2 = pool.AllocChunk();
 * 
 * // 获取统计信息
 * uint32_t total = pool.GetAllCount();    // 总chunk数
 * uint32_t used = pool.GetUsedCount();    // 已使用数
 * uint32_t free = pool.GetFreeCount();    // 可用数
 * 
 * // 释放内存
 * pool.FreeChunk(ptr1);
 * pool.FreeChunk(ptr2);
 * 
 * // 获取chunk信息
 * int index = pool.GetChunkIndex(ptr1);   // 获取chunk索引
 * void* chunk = pool.GetChunkByIndex(index); // 根据索引获取chunk
 * @endcode
 * 
 * 设计优势：
 * - 零内存碎片化问题
 * - 可预测的分配性能
 * - 高效的内存利用率
 * - 简单的内存泄漏检测
 * - 支持内存使用统计
 */
class NFChunkPool {
public:
    /**
     * @brief 构造函数
     * @param reserveSize 预分配的内存块数量
     * @param chunkSize 每个chunk的字节大小
     * @param chunkCount 每个内存块包含的chunk数量
     * @param freeToSys 是否在空闲时将内存归还给系统
     * 
     * 初始化内存池的基本参数：
     * - 设置chunk的大小和每块的chunk数量
     * - 配置内存回收策略
     * - 初始化内部数据结构
     * - 预分配指定数量的内存块
     * 
     * 参数说明：
     * - reserveSize: 影响启动性能和内存占用的平衡
     * - chunkSize: 必须足够容纳目标对象且考虑对齐
     * - chunkCount: 影响内存块的粒度和管理开销
     * - freeToSys: true表示积极回收，false表示保留缓存
     */
    NFChunkPool(uint32_t reserveSize,
                uint32_t chunkSize,
                uint32_t chunkCount,
                bool freeToSys);

    /**
     * @brief 虚析构函数
     * 
     * 清理内存池的所有资源：
     * - 释放所有已分配的内存块
     * - 清理内部管理数据结构
     * - 输出内存泄漏统计信息（调试模式）
     */
    virtual ~NFChunkPool();

    /**
     * @brief 分配一个chunk
     * @return 成功返回chunk指针，失败返回nullptr
     * 
     * 高效的chunk分配算法：
     * 1. 优先从部分使用的内存块分配
     * 2. 如果没有，从完全空闲的内存块分配
     * 3. 如果都没有，分配新的内存块
     * 4. 更新内存块状态和统计信息
     * 
     * 性能特点：
     * - O(1)时间复杂度（除非需要分配新块）
     * - 优化的内存访问模式
     * - 最小化的分支跳转
     * 
     * 失败情况：
     * - 系统内存不足
     * - 达到预设的最大内存限制
     */
    void *AllocChunk();

    /**
     * @brief 释放一个chunk
     * @param chunk 要释放的chunk指针
     * @return 释放成功返回true，失败返回false
     * 
     * 智能的chunk释放算法：
     * 1. 验证chunk指针的有效性
     * 2. 定位chunk所属的内存块
     * 3. 将chunk加入内存块的空闲链表
     * 4. 更新内存块状态和统计信息
     * 5. 必要时进行内存块状态迁移
     * 6. 考虑是否将空闲内存块归还系统
     * 
     * 安全检查：
     * - 验证chunk是否属于当前内存池
     * - 检查double-free错误
     * - 验证内存对齐要求
     * 
     * 状态管理：
     * - 自动在不同状态链表间迁移内存块
     * - 更新全局使用统计
     * - 触发内存回收策略
     */
    bool FreeChunk(void *chunk);

    /**
     * @brief 获取chunk的大小
     * @return chunk的字节大小
     * 
     * 返回每个chunk的实际大小，用于：
     * - 验证分配请求的合理性
     * - 内存使用统计和监控
     * - 调试和诊断工具
     */
    uint32_t ChunkSize() const;

    /**
     * @brief 获取chunk在内存池中的索引
     * @param chunk chunk指针
     * @return chunk的全局索引，失败返回-1
     * 
     * 计算chunk的唯一标识索引：
     * - 用于调试和日志记录
     * - 支持chunk的快速定位
     * - 提供内存使用的可视化
     * - 实现自定义的内存管理策略
     * 
     * 算法特点：
     * - 基于内存地址的数学计算
     * - O(1)时间复杂度
     * - 跨内存块的连续编号
     */
    int GetChunkIndex(void *chunk) const;

    /**
     * @brief 根据索引获取chunk指针
     * @param iIndex chunk的全局索引
     * @return 对应的chunk指针，索引无效返回nullptr
     * 
     * 根据索引快速定位chunk：
     * - 与GetChunkIndex互为逆操作
     * - 支持基于索引的内存访问
     * - 用于实现自定义的分配策略
     * - 调试工具的内存遍历功能
     * 
     * 边界检查：
     * - 验证索引范围的有效性
     * - 检查对应chunk是否已分配
     * - 确保返回指针的安全性
     */
    void *GetChunkByIndex(int iIndex) const;

    /**
     * @brief 获取总chunk数量
     * @return 内存池中所有chunk的总数（包括已用和空闲）
     * 
     * 返回内存池的总容量，用于：
     * - 内存使用率的计算
     * - 容量规划和调优
     * - 性能监控和报告
     */
    uint32_t GetAllCount() const { return m_allCount; }
    
    /**
     * @brief 获取空闲chunk数量
     * @return 当前可用的chunk数量
     * 
     * 返回可立即分配的chunk数量：
     * - 实时反映可用资源
     * - 用于负载均衡决策
     * - 内存告警和监控
     */
    uint32_t GetFreeCount() const { return m_freeCount; }
    
    /**
     * @brief 获取已使用chunk数量
     * @return 当前已分配的chunk数量
     * 
     * 计算当前的内存使用量：
     * - 等于总数减去空闲数
     * - 用于内存泄漏检测
     * - 性能分析和优化
     */
    uint32_t GetUsedCount() const { return m_allCount - m_freeCount; }
    
protected:
    /**
     * @brief 分配一个新的内存块
     * @return 成功返回内存块指针，失败返回nullptr
     * 
     * 从系统分配新的内存块：
     * - 分配包含多个chunk的大块内存
     * - 初始化内存块的管理结构
     * - 建立chunk间的空闲链表
     * - 将内存块加入相应的管理链表
     * 
     * 内存布局：
     * - 先分配NFMemBlock管理结构
     * - 然后分配chunkCount个chunk
     * - 确保适当的内存对齐
     * - 初始化空闲链表连接
     */
    NFMemBlock *AllocBlock();
    
    /**
     * @brief 释放一个内存块
     * @param pBlock 要释放的内存块指针
     * 
     * 将内存块归还给系统：
     * - 验证内存块的完整性
     * - 从管理链表中移除
     * - 更新全局统计信息
     * - 释放底层内存
     * 
     * 安全检查：
     * - 确认内存块完全空闲
     * - 验证管理结构的一致性
     * - 防止double-free错误
     */
    void FreeBlock(NFMemBlock *pBlock);

    /**
     * @brief 从指定内存块分配chunk
     * @param block 内存块指针
     * @return 分配的chunk指针
     * 
     * 内存块级别的chunk分配：
     * - 从内存块的空闲链表获取chunk
     * - 更新内存块的统计信息
     * - 处理内存块状态的变化
     * - 维护空闲链表的完整性
     */
    void *AllocChunk(NFMemBlock *block);

    /**
     * @brief 向指定内存块释放chunk
     * @param block 内存块指针
     * @param chunk 要释放的chunk指针
     * @return 释放成功返回true
     * 
     * 内存块级别的chunk释放：
     * - 将chunk加入内存块的空闲链表
     * - 更新内存块的使用统计
     * - 检查内存块状态变化
     * - 必要时迁移内存块到不同链表
     */
    bool FreeChunk(NFMemBlock *block, void *chunk);

    /**
     * @brief 获取chunk在内存块中的索引
     * @param block 内存块指针
     * @param chunk chunk指针
     * @return chunk在该内存块中的索引
     * 
     * 计算chunk在特定内存块中的位置：
     * - 基于内存地址的偏移计算
     * - 用于验证chunk的归属
     * - 支持调试和诊断功能
     */
    int GetChunkIndex(NFMemBlock *block, void *chunk) const;

    /**
     * @brief 释放链表中的所有内存块
     * @param list 要释放的内存块链表
     * 
     * 批量释放内存块的工具函数：
     * - 遍历链表中的所有内存块
     * - 逐个释放每个内存块
     * - 清空链表结构
     * - 更新全局统计信息
     * 
     * 使用场景：
     * - 析构函数中的资源清理
     * - 内存池的重置操作
     * - 错误恢复中的资源释放
     */
    void FreeBlockList(NFDoubleList &list);

    /**
     * @brief 获取chunk的下一个指针引用
     * @param chunk chunk指针
     * @return chunk中存储的下一个chunk指针的引用
     * 
     * 空闲链表的指针操作辅助函数：
     * - 将chunk的内存空间用作链表指针存储
     * - 提供类型安全的指针访问
     * - 简化空闲链表的维护操作
     * 
     * 实现原理：
     * - 空闲chunk的前几个字节用于存储下一个chunk指针
     * - 通过引用返回支持直接修改
     * - 确保指针对齐要求
     */
    void *&NextOf(void * chunk);

protected:
    bool m_freeToSys;           ///< 是否在空闲时将内存归还系统
    uint32_t m_reserveSize;     ///< 预留的内存块数量
    uint32_t m_chunkSize;       ///< 每个chunk的字节大小
    uint32_t m_chunkCount;      ///< 每个内存块包含的chunk数量
    uint32_t m_freeCount;       ///< 当前空闲的chunk总数
    uint32_t m_allCount;        ///< 所有chunk的总数（已用+空闲）
    uint32_t m_freeThreshold;   ///< 空闲内存块的释放阈值
    size_t m_memSize;           ///< 单个内存块的总字节大小
    NFDoubleList m_blockList;   ///< 部分使用的内存块链表
    NFDoubleList m_emptyList;   ///< 完全空闲的内存块链表
    NFDoubleList m_fullList;    ///< 完全使用的内存块链表
    std::vector<NFMemBlock *> m_vecBlock; ///< 所有内存块的快速索引数组
};

#ifndef  CHECK_MEM_LEAK
#define  CHECK_MEM_LEAK    ///< 内存泄漏检测标志，启用内存分配跟踪
#endif // CHECK_MEM_LEAK

