// -------------------------------------------------------------------------
//    @FileName         :    NFIMonitorModule.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFPluginModule
//    @Description      :    监控模块接口定义，提供系统性能监控和用户统计功能
//                           支持CPU、内存使用率监控和实时数据统计
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIDynamicModule.h"
#include <vector>
#include <string>
#include "NFSystemInfo.h"

/**
 * @brief 监控模块接口类
 * 
 * NFIMonitorModule 提供了系统监控和性能统计功能：
 * 
 * 1. 系统信息监控：
 *    - 系统硬件信息获取
 *    - 系统运行状态监控
 *    - 系统资源信息查询
 * 
 * 2. 性能指标监控：
 *    - CPU使用率实时监控
 *    - 内存使用量统计
 *    - 资源使用趋势分析
 * 
 * 3. 用户统计功能：
 *    - 在线用户数量统计
 *    - 用户连接状态监控
 *    - 用户活跃度分析
 * 
 * 4. 监控数据输出：
 *    - 定期统计数据打印
 *    - 监控报告生成
 *    - 性能日志记录
 * 
 * 监控系统特性：
 * - 实时性能数据采集
 * - 低开销的监控实现
 * - 可配置的监控频率
 * - 多维度数据统计
 * - 异常状态告警
 * 
 * 使用场景：
 * - 服务器性能监控
 * - 系统资源管理
 * - 运维数据统计
 * - 性能瓶颈分析
 * - 容量规划支持
 */
class NFIMonitorModule : public NFIDynamicModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    NFIMonitorModule(NFIPluginManager *p) : NFIDynamicModule(p)
    {

    }

    /**
     * @brief 虚析构函数
     * 
     * 确保派生类能够正确析构，清理监控资源。
     */
    virtual ~NFIMonitorModule()
    {

    }

    /**
     * @brief 获取系统信息
     * @return 返回系统信息对象的常量引用
     * 
     * 获取当前系统的详细信息，包括硬件配置、操作系统版本、
     * 网络配置等系统基础信息。
     */
    virtual const NFSystemInfo &GetSystemInfo() const = 0;

    /**
     * @brief 获取当前用户数量
     * @return 返回当前在线用户数量
     * 
     * 获取系统中当前活跃的用户连接数量，用于负载监控和
     * 容量管理。
     */
    virtual uint32_t GetUserCount() const = 0;

    /**
     * @brief 设置用户数量
     * @param count 要设置的用户数量
     * 
     * 手动设置当前用户数量，通常在用户连接状态发生变化时调用。
     * 
     * 使用示例：
     * @code
     * // 用户登录时增加计数
     * uint32_t currentCount = GetUserCount();
     * SetUserCount(currentCount + 1);
     * 
     * // 用户离线时减少计数
     * SetUserCount(currentCount - 1);
     * @endcode
     */
    virtual void SetUserCount(uint32_t count) = 0;

    /**
     * @brief 获取CPU使用率
     * @return 返回CPU使用率百分比（0.0-100.0）
     * 
     * 获取当前系统的CPU使用率，返回值为百分比形式。
     * 用于监控系统负载和性能状态。
     * 
     * @note 返回值范围为0.0到100.0，表示CPU使用百分比
     */
    virtual double GetCpuUsed() = 0;

    /**
     * @brief 获取内存使用量
     * @return 返回当前使用的内存大小（字节）
     * 
     * 获取当前进程或系统使用的内存总量，用于内存使用监控
     * 和内存泄漏检测。
     */
    virtual uint64_t GetMemUsed() = 0;

    /**
     * @brief 统计并打印监控数据
     * 
     * 执行一次完整的系统监控数据统计，并将结果输出到日志。
     * 包括CPU使用率、内存使用量、用户数量等关键指标。
     * 
     * 通常在定时器中定期调用，用于生成监控报告和性能日志。
     * 输出的信息可用于：
     * - 性能趋势分析
     * - 系统状态监控
     * - 运维决策支持
     * - 问题诊断参考
     */
    virtual void CountAndPrint() = 0;
};

