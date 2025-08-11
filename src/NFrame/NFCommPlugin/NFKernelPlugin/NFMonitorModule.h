// -------------------------------------------------------------------------
//    @FileName         :    NFMonitorModule.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFCMonitorModule
//    @Desc             :    监控模块头文件，提供系统性能和状态监控功能。
//                          该文件定义了NFShmXFrame框架的监控模块，提供全面的系统监控和性能统计功能，
//                          包括系统资源监控、业务监控指标、监控功能特性、性能优化等功能。
//                          主要功能包括CPU使用率监控、内存使用量监控、系统信息获取、进程状态监控、
//                          用户数量统计、连接数量监控、处理性能统计、服务状态监控、定时采集、
//                          事件驱动、数据统计、报警机制、低开销监控、智能采样、数据缓存、异步处理
//    @Description      :    监控模块头文件，提供系统性能和状态监控功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIMonitorModule.h"
#include "NFComm/NFPluginModule/NFSystemInfo.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFComm/NFPluginModule/NFEventObj.h"

/**
 * @class NFCMonitorModule
 * @brief 服务器监控模块实现类
 *
 * NFCMonitorModule提供了全面的系统监控和性能统计功能：
 * 
 * 系统资源监控：
 * - CPU使用率：实时监控CPU占用情况
 * - 内存使用量：监控内存消耗和内存泄漏
 * - 系统信息：获取硬件和操作系统信息
 * - 进程状态：监控进程运行状态和资源占用
 * 
 * 业务监控指标：
 * - 用户数量：实时统计在线用户数
 * - 连接数量：监控网络连接状态
 * - 处理性能：统计请求处理效率
 * - 服务状态：监控各个服务模块的健康状态
 * 
 * 监控功能特性：
 * - 定时采集：定期收集系统和业务指标
 * - 事件驱动：响应系统事件进行监控
 * - 数据统计：提供统计数据的计算和分析
 * - 报警机制：支持阈值检测和报警通知
 * 
 * 性能优化：
 * - 低开销监控：最小化监控对系统性能的影响
 * - 智能采样：根据系统负载调整监控频率
 * - 数据缓存：缓存监控数据减少重复计算
 * - 异步处理：非阻塞的监控数据处理
 * 
 * 应用场景：
 * - 性能调优：识别系统性能瓶颈
 * - 容量规划：提供系统容量规划数据
 * - 故障诊断：快速定位系统问题
 * - 运维监控：实时监控系统运行状态
 * - 报表统计：生成系统运行报表
 * 
 * @note 监控数据可用于外部监控系统集成
 * @note 支持多种监控指标的扩展和自定义
 */
class NFCMonitorModule : public NFIMonitorModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFCMonitorModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 */
	virtual ~NFCMonitorModule();

public:
	/**
	 * @brief 初始化监控模块
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 初始化系统信息收集器和监控定时器。
	 */
	virtual int Init() override;

	/**
	 * @brief 监控模块定时更新
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 定期收集系统性能数据和业务监控指标。
	 */
	virtual int Tick() override;

	/**
	 * @brief 最终化处理
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 清理监控资源，输出最终统计报告。
	 */
	virtual int Finalize() override;

	/**
	 * @brief 定时器回调处理
	 * @param nTimerID 定时器ID
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 处理定时器触发的监控任务，如定期数据收集和统计。
	 */
	virtual int OnTimer(uint32_t nTimerID);

	/**
	 * @brief 事件处理回调
	 * @param serverType 服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 事件源类型
	 * @param nSrcID 事件源ID
	 * @param pMessage 事件消息
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 处理系统事件，收集相关的监控数据。
	 */
	virtual int OnExecute(uint32_t serverType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message* pMessage);

	/**
	 * @brief 获取系统信息
	 * @return 返回系统信息对象的常量引用
	 * 
	 * 获取硬件配置、操作系统版本、进程信息等系统基础信息。
	 */
	virtual const NFSystemInfo& GetSystemInfo() const;

	/**
	 * @brief 获取当前用户数量
	 * @return 返回当前在线用户数
	 * 
	 * 获取实时的在线用户统计数据。
	 */
	virtual uint32_t GetUserCount() const;
	
	/**
	 * @brief 设置当前用户数量
	 * @param count 用户数量
	 * 
	 * 更新当前在线用户数统计。
	 */
	virtual void SetUserCount(uint32_t count);

	/**
	 * @brief 获取CPU使用率
	 * @return 返回CPU使用率百分比（0.0-100.0）
	 * 
	 * 实时获取系统CPU使用率，用于性能监控和负载评估。
	 */
	virtual double GetCpuUsed();

	/**
	 * @brief 获取内存使用量
	 * @return 返回内存使用量（字节）
	 * 
	 * 获取当前进程的内存使用量，包括物理内存和虚拟内存。
	 */
	virtual uint64_t GetMemUsed();

	/**
	 * @brief 统计并打印监控数据
	 * 
	 * 计算各项监控指标的统计数据并输出到日志，
	 * 包括性能趋势、峰值数据、平均值等。
	 */
    virtual void CountAndPrint();

private:
	/**
	 * @brief 系统信息对象
	 * 
	 * 存储系统的基础信息，包括：
	 * - 硬件配置信息（CPU、内存、磁盘等）
	 * - 操作系统信息（版本、架构等）
	 * - 进程信息（PID、启动时间等）
	 * - 网络配置信息
	 */
	NFSystemInfo mSystemInfo;
};
