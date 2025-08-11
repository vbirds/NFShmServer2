// -------------------------------------------------------------------------
//    @FileName         :    NFSystemInfo.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFSystemInfo
//    @Desc             :    系统信息管理模块实现，提供系统资源监控和进程信息获取功能
// -------------------------------------------------------------------------

#include "NFSystemInfo.h"
#include <iostream>
#include "NFComm/NFCore/NFMD5.h"

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @brief sigar库函数声明：将网络地址转换为字符串
 * @param sigar sigar库句柄
 * @param address 网络地址结构
 * @param addr_str 输出字符串缓冲区
 * @return 操作结果
 */
int sigar_net_address_to_string(sigar_t *sigar,
                                               sigar_net_address_t *address,
                                               char *addr_str);
#ifdef __cplusplus
}
#endif

/**
 * @brief 构造函数
 * 
 * 初始化系统信息管理对象，设置默认值
 */
NFSystemInfo::NFSystemInfo()
{
    m_sigar = NULL;
	mCpuCount = 1;
}

/**
 * @brief 初始化系统信息模块
 * 
 * 初始化sigar库，获取系统基本信息包括CPU、操作系统和当前进程信息
 */
void NFSystemInfo::Init()
{
	sigar_open(&m_sigar);

	CountCpu();
	CountOsInfo();
	CountCurProcessBaseInfo();
}

/**
 * @brief 析构函数
 * 
 * 清理sigar库资源
 */
NFSystemInfo::~NFSystemInfo()
{
	if (m_sigar)
	{
		sigar_close(m_sigar);
	}
}

/**
 * @brief 获取当前进程ID
 * @return 当前进程ID
 */
uint32_t NFSystemInfo::GetProcessPid()
{
	return (uint32_t)getpid();
}

/**
 * @brief 计算当前进程基本信息
 * 
 * 获取当前进程的ID、名称和工作目录信息
 */
void NFSystemInfo::CountCurProcessBaseInfo()
{
	if (m_sigar)
	{
		mCurProcessInfo.mPid = GetProcessPid();

		sigar_proc_exe_t exestate;
		if (sigar_proc_exe_get(m_sigar, mCurProcessInfo.mPid, &exestate) != SIGAR_OK)
		{
			//NFLogError("sigar_proc_state_get error, cur_pid:{}", mCurProcessInfo.mPid);
		}

		mCurProcessInfo.mName = exestate.name;
		mCurProcessInfo.mCwd = exestate.cwd;
	}
}

/**
 * @brief 计算CPU信息
 * 
 * 获取系统CPU核心数量
 */
void NFSystemInfo::CountCpu()
{
	if (m_sigar)
	{
		int status = SIGAR_OK;

		sigar_cpu_list_t cpulist;

		status = sigar_cpu_list_get(m_sigar, &cpulist);

		if (status != SIGAR_OK)
		{
			mCpuCount = 1;
		}
		else
		{
			mCpuCount = cpulist.number;
			sigar_cpu_list_destroy(m_sigar, &cpulist);
		}
	}
}

/**
 * @brief 获取CPU核心数
 * @return CPU核心数
 */
uint32_t NFSystemInfo::GetCpuCount() const
{
	return mCpuCount;
}

/**
 * @brief 获取当前CPU使用率
 * @return CPU使用率（百分比）
 */
float NFSystemInfo::GetCurCpuPer() const
{
	return mCurProcessInfo.mCpuUsed;
}

/**
 * @brief 获取当前内存使用量
 * @return 内存使用量（字节）
 */
uint64_t NFSystemInfo::GetCurMemPer() const
{
	return mCurProcessInfo.mMemUsed;
}

/**
 * @brief 计算指定进程的CPU使用率
 * @param pid 进程ID
 * @return CPU使用率（百分比）
 */
double NFSystemInfo::CountProcessCpuPer(uint32_t pid)
{
	if (m_sigar)
	{
		sigar_proc_cpu_t cpu;
		double percent = 0;

		int status = sigar_proc_cpu_get(m_sigar, pid, &cpu);
		if (status == SIGAR_OK)
		{
			percent = double(cpu.percent) * 100.f;

#if NF_PLATFORM == NF_PLATFORM_WIN
			// Windows平台需要除以CPU核心数来获得准确的百分比
			percent /= double(mCpuCount);
#endif
		}
		else
		{
			//NFLogError("sigar_proc_cpu_get cpu error, cur_pid:{}", pid);
		}

		return percent;
	}

	return 0;
}

/**
 * @brief 计算指定进程的内存使用量
 * @param pid 进程ID
 * @return 内存使用量（字节）
 */
uint64_t NFSystemInfo::CountProcessMemPer(uint32_t pid)
{
	int status;
	uint64_t mem = 0;

	if (m_sigar)
	{
		sigar_proc_mem_t proc_mem;
		status = sigar_proc_mem_get(m_sigar, pid, &proc_mem);

		if (status == SIGAR_OK)
		{
			mem = proc_mem.resident;
		}
		else
		{
			//NFLogError("sigar_proc_mem_get error, cur_pid:{}", pid);
		}
	}

	return mem;
}

/**
 * @brief 获取内存信息
 * @return 内存信息结构体引用
 */
const NFMemInfo& NFSystemInfo::GetMemInfo() const
{
	return mMachineMemInfo;
}

/**
 * @brief 获取进程信息
 * @return 进程信息结构体引用
 */
const NFProcessInfo& NFSystemInfo::GetProcessInfo() const
{
	return mCurProcessInfo;
}

/**
 * @brief 获取操作系统信息
 * @return 操作系统信息结构体引用
 */
const NFOsInfo& NFSystemInfo::GetOsInfo() const
{
	return mMachineOsInfo;
}

/**
 * @brief 计算操作系统信息
 * 
 * 获取操作系统的详细信息，包括系统名称、版本、架构等
 */
void NFSystemInfo::CountOsInfo()
{
	if (m_sigar)
	{
		sigar_sys_info_t sysInfo;
		if (sigar_sys_info_get(m_sigar, &sysInfo) != SIGAR_OK)
		{
			//NFLogError("sigar_sys_info_get error");
		}

		mMachineOsInfo.mOsName = sysInfo.name;
		mMachineOsInfo.mOsVersion = sysInfo.version;
		mMachineOsInfo.mOsArch = sysInfo.arch;
		mMachineOsInfo.mOsMachine = sysInfo.machine;
		mMachineOsInfo.mOsDescription = sysInfo.description;
		mMachineOsInfo.mPatchLevel = sysInfo.patch_level;
		mMachineOsInfo.mVendor = sysInfo.vendor;
		mMachineOsInfo.mVendorVersion = sysInfo.vendor_version;
		mMachineOsInfo.mVerdorName = sysInfo.vendor_name;
		mMachineOsInfo.mVendorCodeName = sysInfo.vendor_code_name;
		mMachineOsInfo.mLocalIp = GetLocalIp();
		mMachineOsInfo.mMacAddress = GetMacAddress();
	}
}

/**
 * @brief 获取MAC地址
 * @return MAC地址字符串
 */
std::string NFSystemInfo::GetMacAddress()
{
	if (m_sigar)
	{
		sigar_net_interface_config_t netConfig;
		if (sigar_net_interface_config_get(m_sigar, NULL, &netConfig) != SIGAR_OK)
		{
			return std::string();
		}

		char name[SIGAR_FQDN_LEN];
		sigar_net_address_to_string(m_sigar, &netConfig.hwaddr, name);
		return std::string(name);
	}
	return std::string();
}

/**
 * @brief 获取本地IP地址
 * @return 本地IP地址字符串
 */
std::string NFSystemInfo::GetLocalIp()
{
	if (m_sigar)
	{
		char name[SIGAR_FQDN_LEN];
		sigar_fqdn_get(m_sigar, name, SIGAR_FQDN_LEN);
		return std::string(name);
	}
	return std::string();
}

/**
 * @brief 计算系统信息
 * 
 * 更新内存信息和当前进程信息
 */
void NFSystemInfo::CountSystemInfo()
{
	CountMemInfo();
	CountCurProcessInfo();
}

/**
 * @brief 计算当前进程信息
 * 
 * 更新当前进程的CPU使用率、内存使用量和线程数
 */
void NFSystemInfo::CountCurProcessInfo()
{
	mCurProcessInfo.mCpuUsed = CountProcessCpuPer(mCurProcessInfo.mPid);
	mCurProcessInfo.mMemUsed = CountProcessMemPer(mCurProcessInfo.mPid);

	if (m_sigar)
	{
		sigar_proc_state_t procstate;
		if (sigar_proc_state_get(m_sigar, mCurProcessInfo.mPid, &procstate) != SIGAR_OK)
		{
			//NFLogError("sigar_proc_state_get error, cur_pid:{}", mCurProcessInfo.mPid);
		}
		mCurProcessInfo.mThreads = (uint32_t)procstate.threads;
	}
}

/**
 * @brief 计算内存信息
 * 
 * 获取系统内存的详细信息，包括总内存、可用内存、已使用内存等
 */
void NFSystemInfo::CountMemInfo()
{
	if (m_sigar)
	{
		sigar_mem_t sigar_mem;
		sigar_mem_get(m_sigar, &sigar_mem);

		mMachineMemInfo.mTotalMem = sigar_mem.total;
		mMachineMemInfo.mFreeMem = sigar_mem.actual_free;
		mMachineMemInfo.mUsedMem = sigar_mem.actual_used;
		mMachineMemInfo.mUsedPercent = sigar_mem.used_percent;
		mMachineMemInfo.mFreePercent = sigar_mem.free_percent;
	}
}

/**
 * @brief 获取用户数量
 * @return 用户数量
 */
uint32_t NFSystemInfo::GetUserCount() const
{
	return mUserCount;
}

/**
 * @brief 设置用户数量
 * @param userCount 用户数量
 */
void NFSystemInfo::SetUserCount(uint32_t userCount)
{
	mUserCount = userCount;
}

