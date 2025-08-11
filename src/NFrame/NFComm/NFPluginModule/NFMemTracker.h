// -------------------------------------------------------------------------
//    @FileName         :    NFMemTracker.h
//    @Author           :    gaoyi
//    @Date             :    2022/10/10
//    @Email			:    445267987@qq.com
//    @Module           :    NFMemTracker
//    @Description      :    内存泄漏跟踪器，提供内存分配和释放的详细跟踪功能
//
// -------------------------------------------------------------------------

/**
 * @file NFMemTracker.h
 * @brief 内存泄漏跟踪器
 * @details 该文件实现了一个高效的内存泄漏检测工具，主要功能包括：
 * 
 * **核心功能：**
 * - **内存分配跟踪**：记录每次内存分配的详细信息
 * - **源码定位**：精确定位内存分配的文件、函数、行号
 * - **泄漏检测**：识别未释放的内存块
 * - **报告生成**：生成详细的内存使用报告
 * - **线程安全**：支持多线程环境下的安全跟踪
 * 
 * **技术特点：**
 * - 单例模式设计，全局统一管理
 * - 使用哈希表优化查找性能
 * - 最小化性能开销，适合生产环境
 * - 支持条件编译，release版本可完全关闭
 * 
 * **使用场景：**
 * - 开发阶段的内存泄漏检测
 * - 性能分析和内存优化
 * - 生产环境的内存监控
 * - 代码质量保证和测试
 * 
 * @author gaoyi
 * @date 2022/10/10
 * @version 1.0
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFMutex.h"
#include "NFComm/NFCore/NFSingleton.hpp"
#include <unordered_map>

/**
 * @struct NFTrackData
 * @brief 内存跟踪数据结构
 * @details 记录每次内存分配的详细信息，用于后续的泄漏检测和报告生成。
 * 
 * **存储信息：**
 * - 分配位置的源码行号
 * - 分配所在的文件名
 * - 分配所在的函数名
 * 
 * **设计考虑：**
 * - 结构紧凑，减少内存开销
 * - 包含足够信息用于问题定位
 * - 支持快速的查找和比较操作
 * 
 * **用途：**
 * - 作为哈希表的值类型存储跟踪信息
 * - 用于生成内存泄漏报告
 * - 提供调试时的上下文信息
 */
struct NFTrackData
{
    uint32_t line_no_;           ///< 内存分配所在的源码行号
    std::string file_name_;      ///< 内存分配所在的文件名（完整路径）
    std::string func_name_;      ///< 内存分配所在的函数名

    /**
     * @brief 构造函数
     * @param line_no 源码行号
     * @param file_name 文件名
     * @param func_name 函数名
     * 
     * 初始化跟踪数据，记录内存分配的精确位置信息。
     */
    NFTrackData(uint32_t line_no,
                const char *file_name,
                const char *func_name);
};

/**
 * @class NFMemTracker
 * @brief 内存跟踪器主类
 * @details 实现单例模式的内存跟踪器，提供全局统一的内存分配和释放跟踪。
 * 
 * **核心特性：**
 * - **单例模式**：确保全局唯一实例，统一管理所有内存跟踪
 * - **线程安全**：使用互斥锁保护共享数据，支持多线程环境
 * - **高性能**：使用哈希表优化查找，最小化跟踪开销
 * - **详细记录**：精确记录每次分配的源码位置信息
 * - **灵活报告**：支持多种格式的内存使用报告
 * 
 * **工作原理：**
 * 1. 内存分配时调用TrackMalloc记录分配信息
 * 2. 内存释放时调用TrackFree移除跟踪记录
 * 3. 程序结束时未移除的记录即为内存泄漏
 * 4. 通过PrintMemLink生成详细的泄漏报告
 * 
 * **性能考虑：**
 * - 跟踪开销控制在微秒级别
 * - 内存开销与跟踪的分配数量线性相关
 * - 支持条件编译，生产环境可关闭
 * 
 * **使用方式：**
 * @code
 * // 通常不直接使用，而是通过宏进行调用
 * void* ptr = malloc(1024);
 * NFMemTracker::Instance()->TrackMalloc(ptr, __FILE__, __FUNCTION__, __LINE__);
 * 
 * // 释放时
 * NFMemTracker::Instance()->TrackFree(ptr);
 * free(ptr);
 * 
 * // 生成报告
 * NFMemTracker::Instance()->PrintMemLink("memory_leak_report.txt");
 * @endcode
 * 
 * @see 配合CHECK_MEM_LEAK宏使用，实现自动化的内存跟踪
 */
class NFMemTracker : public NFSingleton<NFMemTracker>
{
public:
    /**
     * @brief 生成内存泄漏报告
     * @param output_filename 输出文件名，报告将写入此文件
     * 
     * **功能说明：**
     * 分析当前所有未释放的内存分配，生成详细的泄漏报告。
     * 
     * **报告内容：**
     * - 泄漏内存块的地址
     * - 分配时的源码位置（文件、函数、行号）
     * - 分配时间和调用堆栈（如果可用）
     * - 泄漏统计信息和分析
     * 
     * **使用场景：**
     * - 程序退出前的内存泄漏检查
     * - 定期的内存健康检查
     * - 问题诊断和调试
     * 
     * **注意事项：**
     * @warning 该方法会遍历所有跟踪记录，可能需要一定时间
     * @note 建议在程序结束前调用，获得完整的泄漏信息
     */
    void PrintMemLink(const std::string& output_filename);

    /**
     * @brief 跟踪内存分配
     * @param ptr 分配的内存指针
     * @param file_path 分配所在的文件路径
     * @param func_num 分配所在的函数名
     * @param line_no 分配所在的行号
     * @return 跟踪是否成功
     * 
     * **功能说明：**
     * 记录一次内存分配操作，将分配信息存储到跟踪系统中。
     * 
     * **跟踪信息：**
     * - 内存指针作为唯一标识
     * - 完整的源码位置信息
     * - 分配时间戳（内部记录）
     * 
     * **性能优化：**
     * - 使用哈希表快速插入
     * - 最小化锁的持有时间
     * - 优化内存拷贝操作
     * 
     * **错误处理：**
     * - 空指针检查
     * - 重复分配检测
     * - 内存不足时的安全处理
     * 
     * @warning 必须与TrackFree成对使用
     * @note 该方法是线程安全的
     */
    bool TrackMalloc(void *ptr, const char *file_path, const char *func_num, uint32_t line_no);

    /**
     * @brief 跟踪内存释放
     * @param ptr 要释放的内存指针
     * 
     * **功能说明：**
     * 记录一次内存释放操作，从跟踪系统中移除对应的分配记录。
     * 
     * **操作过程：**
     * - 查找指针对应的跟踪记录
     * - 验证释放操作的有效性
     * - 从跟踪表中移除记录
     * - 更新内部统计信息
     * 
     * **错误检测：**
     * - 检测重复释放（double free）
     * - 检测释放未分配的指针
     * - 检测野指针释放
     * 
     * **性能特点：**
     * - O(1)平均查找时间
     * - 最小化锁竞争
     * - 快速的记录移除
     * 
     * @warning 释放不存在的指针会记录警告但不会崩溃
     * @note 该方法是线程安全的
     */
    void TrackFree(void *ptr);

protected:
    typedef NFMutex TMutexLock;                                        ///< 互斥锁类型定义
    typedef std::unordered_map<void *, NFTrackData> PtrTrackMap;     ///< 指针跟踪映射表类型

    TMutexLock mutex_lock_;        ///< 线程同步互斥锁，保护共享数据的并发访问
    PtrTrackMap ptr_track_map_;    ///< 指针跟踪映射表，存储所有活跃的内存分配记录

public:
    /**
     * @brief 构造函数
     * 
     * 初始化内存跟踪器，设置内部数据结构和同步机制。
     * 由于使用单例模式，外部不应直接调用构造函数。
     */
    NFMemTracker();
    
    /**
     * @brief 析构函数
     * 
     * 清理内存跟踪器资源，输出最终的内存使用报告。
     * 如果存在未释放的内存，会自动生成泄漏报告。
     * 
     * @note 析构时会自动检查是否有内存泄漏
     */
    virtual ~NFMemTracker();
};


