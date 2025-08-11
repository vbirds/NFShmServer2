// -------------------------------------------------------------------------
//    @FileName         :    NFCpu.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFCpu.h
 * @brief CPU使用率监控工具类
 * 
 * 此文件提供了跨平台的CPU使用率监控功能，主要用于系统性能监控。
 * 当前主要实现了Windows平台的CPU使用率计算，Linux平台待扩展。
 */

#pragma once

#include "NFPlatform.h"

#if NF_PLATFORM == NF_PLATFORM_WIN
#include <windows.h>
#include <Psapi.h>
#include <conio.h>
#include <stdio.h>

/**
 * @brief Windows平台CPU使用率监控类
 * 
 * CpuUsage提供了在Windows平台下获取当前进程CPU使用率的功能。
 * 通过比较系统时间和进程时间来计算CPU使用率百分比。
 * 
 * 实现原理：
 * - 获取系统总的CPU时间（内核态+用户态）
 * - 获取当前进程的CPU时间（内核态+用户态）
 * - 计算两次测量之间的时间差
 * - CPU使用率 = (进程时间差 / 系统时间差) * 100%
 * 
 * 主要特性：
 * - 线程安全：使用原子操作保证多线程安全
 * - 准确计算：基于系统API获取精确时间信息
 * - 自动管理：首次运行标志和历史数据维护
 * - 高效实现：避免重复计算，提供缓存结果
 * 
 * 使用方法：
 * @code
 * CpuUsage cpuMonitor;
 * 
 * // 定期查询CPU使用率
 * while (running) {
 *     short usage = cpuMonitor.GetUsage();
 *     std::cout << "CPU使用率: " << usage << "%" << std::endl;
 *     Sleep(1000);  // 每秒查询一次
 * }
 * @endcode
 * 
 * @note 第一次调用GetUsage()时会初始化基准数据，返回值可能不准确
 * @note 建议间隔一定时间（如1秒）再次调用以获得准确的使用率
 */
class CpuUsage
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化CPU使用率监控对象，清零所有时间记录和状态标志。
	 */
	CpuUsage() : IsFirstRun(true), m_nCpuUsage(-1), m_lRunCount(0)
	{
		ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
		ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));

		ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
		ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
	}

	/**
	 * @brief 获取当前CPU使用率
	 * 
	 * 计算并返回当前进程的CPU使用率百分比。
	 * 基于系统时间和进程时间的差值计算得出。
	 * 
	 * 计算过程：
	 * 1. 获取当前系统时间（空闲、内核、用户）
	 * 2. 获取当前进程时间（内核、用户）
	 * 3. 与上次记录的时间进行比较
	 * 4. 计算CPU使用率 = (进程时间差/系统时间差) * 100
	 * 
	 * @return short CPU使用率百分比（0-100）
	 *               - 首次调用返回-1或缓存值
	 *               - 获取失败时返回上次的缓存值
	 *               - 正常情况返回当前CPU使用率
	 * 
	 * @note 此方法是线程安全的，使用原子操作避免并发问题
	 * @note 首次调用会建立基准，建议忽略首次结果
	 */
	short GetUsage()
	{
		short nCpuCopy = m_nCpuUsage;

		if (::InterlockedIncrement(&m_lRunCount) == 1)
		{
			FILETIME ftSysIdle, ftSysKernel, ftSysUser;
			FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

			if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) ||
				!GetProcessTimes(GetCurrentProcess(), &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser))
			{
				::InterlockedDecrement(&m_lRunCount);
				return nCpuCopy;
			}

			if (!IsFirstRun)
			{
				/*
				CPU使用率计算方法：
				获取系统从上次测量以来运行的总时间（内核态+用户态）
				以及进程运行的总时间（内核态+用户态）
				*/
				ULONGLONG ftSysKernelDiff = SubtractTimes(ftSysKernel, m_ftPrevSysKernel);
				ULONGLONG ftSysUserDiff = SubtractTimes(ftSysUser, m_ftPrevSysUser);

				ULONGLONG ftProcKernelDiff = SubtractTimes(ftProcKernel, m_ftPrevProcKernel);
				ULONGLONG ftProcUserDiff = SubtractTimes(ftProcUser, m_ftPrevProcUser);

				ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;
				ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;

				if (nTotalSys > 0)
				{
					m_nCpuUsage = static_cast<short>((100.0 * nTotalProc) / nTotalSys);
				}
			}

			m_ftPrevSysKernel = ftSysKernel;
			m_ftPrevSysUser = ftSysUser;
			m_ftPrevProcKernel = ftProcKernel;
			m_ftPrevProcUser = ftProcUser;

			nCpuCopy = m_nCpuUsage;
		}

		IsFirstRun = false;
		::InterlockedDecrement(&m_lRunCount);

		return nCpuCopy;
	}

private:
	/**
	 * @brief 计算两个FILETIME之间的差值
	 * 
	 * 将FILETIME结构转换为64位整数并计算差值。
	 * FILETIME表示自1601年1月1日以来的100纳秒间隔数。
	 * 
	 * @param ftA 较新的时间
	 * @param ftB 较旧的时间
	 * @return ULONGLONG 时间差（100纳秒为单位）
	 */
	static ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB)
	{
		LARGE_INTEGER a, b;
		a.LowPart = ftA.dwLowDateTime;
		a.HighPart = ftA.dwHighDateTime;

		b.LowPart = ftB.dwLowDateTime;
		b.HighPart = ftB.dwHighDateTime;

		return a.QuadPart - b.QuadPart;
	}

	/** @brief 是否为首次运行标志，用于跳过首次不准确的计算 */
	bool IsFirstRun;
	/** @brief 当前CPU使用率缓存值（百分比） */
	short m_nCpuUsage;

	/** @brief 上次记录的系统内核态时间 */
	FILETIME m_ftPrevSysKernel;
	/** @brief 上次记录的系统用户态时间 */
	FILETIME m_ftPrevSysUser;

	/** @brief 上次记录的进程内核态时间 */
	FILETIME m_ftPrevProcKernel;
	/** @brief 上次记录的进程用户态时间 */
	FILETIME m_ftPrevProcUser;

	/** @brief 运行计数器，用于多线程同步 */
	volatile LONG m_lRunCount;
};
#endif

/**
 * @brief 跨平台CPU和内存监控工具类
 * 
 * NFCpu提供了跨平台的系统性能监控功能，包括CPU使用率和内存使用情况的获取。
 * 支持Windows和Linux平台，提供统一的API接口。
 * 
 * 主要功能：
 * - CPU时间获取：支持进程和系统级别的CPU时间统计
 * - CPU使用率计算：基于时间差计算准确的CPU使用率
 * - 内存监控：获取进程的虚拟内存和物理内存使用情况
 * - 跨平台支持：统一的API，自动适配不同操作系统
 * 
 * 使用场景：
 * - 系统性能监控和报警
 * - 应用程序性能分析
 * - 资源使用情况统计
 * - 负载均衡决策支持
 * 
 * 使用方法：
 * @code
 * // 获取CPU使用率
 * int cpuUsage = NFCpu::GetCurCpuUseage();
 * std::cout << "CPU使用率: " << cpuUsage << "%" << std::endl;
 * 
 * // 获取内存使用情况
 * int vmSize, rssSize;
 * NFCpu::GetCurMemoryUsage(&vmSize, &rssSize);
 * std::cout << "虚拟内存: " << vmSize << "KB, 物理内存: " << rssSize << "KB" << std::endl;
 * 
 * // 计算时间段内的CPU使用率
 * long long startCpu = NFCpu::GetCurCpuTime();
 * long long startTotal = NFCpu::GetTotalCpuTime();
 * // ... 执行一些操作 ...
 * long long endCpu = NFCpu::GetCurCpuTime();
 * long long endTotal = NFCpu::GetTotalCpuTime();
 * float usage = NFCpu::CalculateCurCpuUseage(startCpu, endCpu, startTotal, endTotal);
 * @endcode
 * 
 * @note 所有方法都是静态方法，不需要创建实例
 * @note CPU使用率的计算基于时间间隔，建议间隔至少1秒以获得准确结果
 */
class NFCpu
{
public:
	/**
	 * @brief 获取当前进程的CPU使用时间
	 * 
	 * 返回当前进程从启动以来消耗的总CPU时间（用户态+内核态）。
	 * 时间单位通常为毫秒或微秒，具体取决于平台实现。
	 * 
	 * @return long long 进程CPU使用时间
	 * 
	 * @note 这是累积时间，不是瞬时使用率
	 * @note 可以通过两次调用的差值计算时间段内的CPU使用情况
	 */
	static long long GetCurCpuTime();

	/**
	 * @brief 获取整个系统的CPU使用时间
	 * 
	 * 返回系统从启动以来的总CPU时间，包括所有进程的CPU使用时间。
	 * 用于计算系统整体的CPU活动水平。
	 * 
	 * @return long long 系统总CPU使用时间
	 * 
	 * @note 这是所有CPU核心、所有进程的累积时间
	 * @note 通常用作CPU使用率计算的分母
	 */
	static long long GetTotalCpuTime();

	/**
	 * @brief 计算进程在指定时间段内的CPU使用率
	 * 
	 * 基于提供的进程CPU时间和系统CPU时间的起止值，
	 * 计算进程在该时间段内的CPU使用率百分比。
	 * 
	 * @param cur_cpu_time_start 时间段开始时的进程CPU时间
	 * @param cur_cpu_time_stop 时间段结束时的进程CPU时间
	 * @param total_cpu_time_start 时间段开始时的系统总CPU时间
	 * @param total_cpu_time_stop 时间段结束时的系统总CPU时间
	 * @return float CPU使用率百分比（0.0-100.0）
	 * 
	 * @note 计算公式：(进程CPU时间差 / 系统CPU时间差) * 100
	 * @note 返回值范围通常在0-100之间，但多核系统可能超过100
	 */
	static float CalculateCurCpuUseage(long long cur_cpu_time_start, long long cur_cpu_time_stop,
	                                   long long total_cpu_time_start, long long total_cpu_time_stop);

	/**
	 * @brief 获取进程当前内存使用情况
	 * 
	 * 获取当前进程的内存使用统计信息，包括虚拟内存和物理内存。
	 * 
	 * @param vm_size_kb [out] 输出参数，虚拟内存大小，单位为KB
	 * @param rss_size_kb [out] 输出参数，物理内存大小（RSS），单位为KB
	 * @return int 操作结果码，0表示成功，非0表示失败
	 * 
	 * @note 虚拟内存（VM）是进程地址空间的总大小
	 * @note 物理内存（RSS）是实际占用的物理RAM大小
	 * @note RSS通常小于VM，因为虚拟内存可能包含未分配的页面
	 */
	static int GetCurMemoryUsage(int* vm_size_kb, int* rss_size_kb);

	/**
	 * @brief 获取进程当前CPU使用率
	 * 
	 * 获取当前进程的实时CPU使用率百分比。
	 * 这是一个便捷方法，内部自动处理时间采样和计算。
	 * 
	 * @return int CPU使用率百分比（0-100）
	 * 
	 * @note 此方法可能需要一定的采样时间才能返回准确结果
	 * @note 建议不要过于频繁调用，以免影响性能
	 */
	static int GetCurCpuUseage();

	/**
	 * @brief 获取进程当前内存使用大小
	 * 
	 * 获取当前进程使用的内存总量的简化版本。
	 * 返回值通常为物理内存使用量。
	 * 
	 * @return int 内存使用大小，具体单位取决于实现（通常为KB或MB）
	 * 
	 * @note 这是GetCurMemoryUsage的简化版本
	 * @note 具体返回的是虚拟内存还是物理内存取决于平台实现
	 */
	static int GetCurMemorySize();
};

