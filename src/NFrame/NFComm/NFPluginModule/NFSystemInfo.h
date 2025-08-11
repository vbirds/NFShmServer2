// -------------------------------------------------------------------------
//    @FileName         :    NFSystemInfo.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFSystemInfo
//    @Desc             :    系统信息管理模块，提供系统资源监控和进程信息获取功能
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "common/sigar/sigar.h"

/**
 * @brief 内存信息结构体
 * 
 * 用于存储系统内存的详细信息，包括总内存、可用内存、已使用内存等
 */
class _NFExport NFMemInfo
{
public:
	/**
	 * @brief 构造函数，初始化内存信息
	 */
	NFMemInfo()
	{
		mTotalMem = 0;
		mFreeMem = 0;
		mUsedMem = 0;
		mUsedPercent = 0.0;
		mFreePercent = 0.0;
	}

	uint64_t mTotalMem;		///< 总内存大小（字节）
	uint64_t mFreeMem;		///< 可用内存大小（字节）
	uint64_t mUsedMem;		///< 已使用内存大小（字节）
	double mUsedPercent;		///< 内存使用率（百分比）
	double mFreePercent;		///< 内存可用率（百分比）
};

/**
 * @brief CPU信息结构体
 * 
 * 用于存储CPU相关信息（当前为空，预留扩展）
 */
class NFCpuInfo
{
public:

};

/**
 * @brief 操作系统信息结构体
 * 
 * 用于存储操作系统的详细信息，包括系统名称、版本、架构等
 */
class NFOsInfo
{
public:
	/**
	 * @brief 构造函数
	 */
	NFOsInfo()
	{

	}

	std::string mOsName;			///< 操作系统名称
	std::string mOsVersion;		///< 操作系统版本
	std::string mOsArch;			///< 操作系统架构
	std::string mOsMachine;		///< 机器类型
	std::string mOsDescription;	///< 操作系统描述
	std::string mPatchLevel;		///< 补丁级别
	std::string mVendor;			///< 厂商信息
	std::string mVendorVersion;	///< 厂商版本
	std::string mVerdorName;		///< 厂商名称
	std::string mVendorCodeName;	///< 厂商代码名称
	std::string mLocalIp;			///< 本地IP地址
	std::string mMacAddress;		///< MAC地址
};

/**
 * @brief 进程信息结构体
 * 
 * 用于存储进程的详细信息，包括进程ID、CPU使用率、内存使用量等
 */
class NFProcessInfo
{
public:
	/**
	 * @brief 构造函数，初始化进程信息
	 */
	NFProcessInfo()
	{
		mPid = 0;
		mCpuUsed = 0.0;
		mMemUsed = 0;
		mThreads = 0;
	}

	std::string mName;	///< 进程名称
	std::string mCwd;	///< 进程工作目录

	uint32_t mPid = 0;		///< 进程ID
	double   mCpuUsed = 0.0;	///< CPU使用率（百分比）
	uint64_t mMemUsed = 0;		///< 内存使用量（字节）
	uint32_t mThreads = 0;		///< 线程数量
};

/**
 * @brief 系统信息管理类
 * 
 * 提供系统资源监控、进程信息获取、网络信息获取等功能
 * 使用sigar库获取系统底层信息
 */
class NFSystemInfo
{
public:
	/**
	 * @brief 构造函数
	 */
	NFSystemInfo();
	
	/**
	 * @brief 析构函数
	 */
	virtual ~NFSystemInfo();

	/**
	 * @brief 初始化系统信息模块
	 * 
	 * 初始化sigar库，获取系统基本信息
	 */
	virtual void Init();

	/**
	 * @brief 获取当前进程ID
	 * @return 当前进程ID
	 */
	virtual uint32_t GetProcessPid();

	/**
	 * @brief 获取内存信息
	 * @return 内存信息结构体引用
	 */
	virtual const NFMemInfo& GetMemInfo() const;
	
	/**
	 * @brief 获取进程信息
	 * @return 进程信息结构体引用
	 */
	virtual const NFProcessInfo& GetProcessInfo() const;
	
	/**
	 * @brief 获取操作系统信息
	 * @return 操作系统信息结构体引用
	 */
	virtual const NFOsInfo& GetOsInfo() const;
	
	/**
	 * @brief 获取当前CPU使用率
	 * @return CPU使用率（百分比）
	 */
	virtual float GetCurCpuPer() const;
	
	/**
	 * @brief 获取当前内存使用量
	 * @return 内存使用量（字节）
	 */
	virtual uint64_t GetCurMemPer() const;
	
	/**
	 * @brief 获取CPU核心数
	 * @return CPU核心数
	 */
	virtual uint32_t GetCpuCount() const;

	/**
	 * @brief 计算指定进程的CPU使用率
	 * @param pid 进程ID
	 * @return CPU使用率（百分比）
	 */
	virtual double CountProcessCpuPer(uint32_t pid);
	
	/**
	 * @brief 计算指定进程的内存使用量
	 * @param pid 进程ID
	 * @return 内存使用量（字节）
	 */
	virtual uint64_t CountProcessMemPer(uint32_t pid);

	/**
	 * @brief 计算当前进程基本信息
	 */
	virtual void CountCurProcessBaseInfo();
	
	/**
	 * @brief 计算CPU信息
	 */
	virtual void CountCpu();
	
	/**
	 * @brief 计算内存信息
	 */
	virtual void CountMemInfo();
	
	/**
	 * @brief 计算当前进程信息
	 */
	virtual void CountCurProcessInfo();
	
	/**
	 * @brief 计算系统信息
	 */
	virtual void CountSystemInfo();
	
	/**
	 * @brief 计算操作系统信息
	 */
	virtual void CountOsInfo();

	/**
	 * @brief 获取用户数量
	 * @return 用户数量
	 */
	virtual uint32_t GetUserCount() const;
	
	/**
	 * @brief 设置用户数量
	 * @param userCount 用户数量
	 */
	virtual void SetUserCount(uint32_t userCount);

	/**
	 * @brief 获取本地IP地址
	 * @return 本地IP地址字符串
	 */
	std::string GetLocalIp();
	
	/**
	 * @brief 获取MAC地址
	 * @return MAC地址字符串
	 */
	std::string GetMacAddress();

private:
	NFProcessInfo mCurProcessInfo;	///< 当前进程信息
	uint32_t mCpuCount = 1;			///< CPU核心数
	NFMemInfo mMachineMemInfo;		///< 机器内存信息
	NFOsInfo mMachineOsInfo;		///< 机器操作系统信息
	uint32_t mUserCount = 0;		///< 当前服务器玩家数量
    sigar_t *m_sigar;				///< sigar库句柄
};
