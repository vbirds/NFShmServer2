// -------------------------------------------------------------------------
//    @FileName         :    NFTickByRunIndexOP.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTickByRunIndexOP.h
//
// -------------------------------------------------------------------------

/**
 * @file NFTickByRunIndexOP.h
 * @brief 按运行索引分批处理操作类定义文件
 * @details 定义了NFrame框架中的分批处理基类NFTickByRunIndexOP，提供了大量数据的
 *          分批处理机制。通过控制每次处理的数量和间隔，避免单次处理过多数据
 *          导致的性能问题。支持动态调整处理频率和批次大小。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFTickByRunIndexOP: 分批处理基类
 *       - 处理频率控制（等待间隔）
 *       - 批次大小控制（每次处理数量）
 *       - 处理状态跟踪和完成检测
 *       - 动态频率调整机制
 * 
 * @warning 使用分批处理时需要注意：
 *          - 必须实现DoTick()虚函数
 *          - 处理时间不要超过预设的基础tick时间
 *          - 合理设置批次大小避免单次处理时间过长
 *          - 注意处理的数据一致性问题
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"

/**
 * @class NFTickByRunIndexOP
 * @brief 按运行索引分批处理操作类
 * @details 提供了基于运行索引的分批处理机制，通过控制处理频率和批次大小，避免单次处理
 *          过多数据造成的性能问题。适用于大量数据的分批处理场景，如批量更新、批量计算等。
 *          支持动态调整处理频率和批次大小以适应不同的性能需求。
 * 
 * @note 核心功能：
 *       - 基于运行索引的定时触发机制
 *       - 分批处理的执行控制
 *       - 处理状态的跟踪和重置
 *       - 停止时的全量处理支持
 *       - 处理频率的动态调整机制
 * 
 * @warning 使用注意事项：
 *          - 继承类必须实现DoTick()抽象方法
 *          - 处理时间不要超过预设的基础tick时间
 *          - 合理设置批次大小避免单次处理时间过长
 *          - 注意处理的数据一致性问题
 */
class NFTickByRunIndexOP
{
public:
	/**
	 * @brief 默认构造函数
	 * @details 根据共享内存管理器的创建模式，选择调用CreateInit()或ResumeInit()
	 */
	NFTickByRunIndexOP()
	{
		if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
		{
			CreateInit();
		}
		else
		{
			ResumeInit();
		}
	}

	/**
	 * @brief 参数化构造函数
	 * @details 使用指定参数初始化分批处理操作
	 * @param dwWaitTickIndex 等待tick间隔，外层调用多少次tick才真正执行一次内层tick
	 * @param iNumPerTick 每次tick处理的对象数量
	 */
	NFTickByRunIndexOP(uint32_t dwWaitTickIndex, int iNumPerTick)
	{
		InitTick(dwWaitTickIndex, iNumPerTick);
	}

	/**
	 * @brief 创建初始化函数
	 * @details 当对象第一次创建时调用，初始化所有控制参数
	 * @return int 初始化结果，0表示成功
	 */
	int CreateInit()
	{
		m_dwLastTickRunIndex = 0;
		m_iLastTickIndex     = 0;
		m_iTickedNum         = 0;
		m_dwWaitTickIndex    = 0;
		m_iNumPerTick        = 0;
		m_bIsTickFinished    = false;
		return 0;
	}

	/**
	 * @brief 恢复初始化函数
	 * @details 当对象从共享内存恢复时调用
	 * @return int 恢复结果，0表示成功
	 */
	int ResumeInit()
	{
		return 0;
	}

	/**
	 * @brief 初始化tick参数
	 * @details 设置分批处理的控制参数
	 * @param dwWaitTickIndex 等待tick间隔，外层调用多少次tick才真正执行一次内层tick
	 * @param iNumPerTick 每次tick处理的对象数量
	 * @note 此函数会重置所有状态，包括频率调整相关的参数
	 */
	void InitTick(uint32_t dwWaitTickIndex, int iNumPerTick)
	{
		m_dwLastTickRunIndex = 0;
		m_iLastTickIndex     = 0;
		m_iTickedNum         = 0;
		m_dwWaitTickIndex    = dwWaitTickIndex;
		m_iNumPerTick        = iNumPerTick;
		m_bIsTickFinished    = false;
		m_bNeedChangeGap = false;
		m_iDestTimePerRound = 0;
		m_iBaseTickMS = 10;
	}

	/**
	 * @brief 虚析构函数
	 * @details 确保派生类能够正确析构
	 */
	virtual ~NFTickByRunIndexOP()
	{
	}

	/**
	 * @brief 执行一次tick处理
	 * @details 根据当前运行索引判断是否需要执行处理，如果需要则调用DoTick()
	 * @param dwCurRunIndex 当前运行索引
	 * @return int 处理结果，0表示成功，非0表示失败
	 * @note 只有满足时间间隔条件时才会真正执行处理
	 */
	int TickNow(uint32_t dwCurRunIndex);

	/**
	 * @brief 停止时处理所有剩余数据
	 * @details 在系统停止时调用，处理所有剩余的数据，确保不遗漏
	 * @param dwCurRunIndex 当前运行索引
	 * @return int 处理结果，0表示成功，非0表示失败
	 * @note 此函数会强制处理所有剩余数据，忽略批次大小限制
	 */
	int TickAllWhenStop(uint32_t dwCurRunIndex);

protected:
	/**
	 * @brief 执行具体的tick处理逻辑
	 * @details 纯虚函数，继承类必须实现具体的处理逻辑
	 * @param dwCurRunIndex 当前运行索引
	 * @param bIsTickAll 是否处理所有数据，true表示忽略批次限制
	 * @return int 处理结果，0表示成功，非0表示失败
	 * @note 这是核心处理函数，继承类应该在此实现具体的业务逻辑
	 */
	virtual int DoTick(uint32_t dwCurRunIndex, bool bIsTickAll = false) = 0;

	/**
	 * @brief 改变tick间隔的处理函数
	 * @details 虚函数，用于处理tick频率调整，默认实现返回0
	 * @return int 处理结果，0表示成功，非0表示失败
	 * @note 继承类可以重写此函数来实现自定义的频率调整逻辑
	 */
	virtual int DoChangeTickGap();

	/**
	 * @brief 设置tick频率调整参数
	 * @details 设置需要调整的目标执行时间和基础tick时间
	 * @param iTimeMS tick一轮需要的时间（毫秒）
	 * @param iBaseTickMS 游戏服务器基础tick时间，一般为10ms
	 * @return int 设置结果，0表示成功
	 * @note 设置后会标记需要频率调整，在下次DoChangeTickGap()时生效
	 */
	int SetChangeTickGap(int iTimeMS, int iBaseTickMS)
	{
		m_bNeedChangeGap = true;
		m_iDestTimePerRound = iTimeMS;
		m_iBaseTickMS = iBaseTickMS;
		return 0;
	}

	/**
	 * @brief 重置每次tick的状态
	 * @details 在tick完成后调用，重置相关状态准备下次处理
	 * @param dwCurRunIndex 当前运行索引
	 * @note 如果当前轮次已完成，会重置tick索引和状态
	 */
	void ResetPerTick(uint32_t dwCurRunIndex)
	{
		if (m_bIsTickFinished)
		{
			m_iLastTickIndex     = 0;
			m_dwLastTickRunIndex = dwCurRunIndex;
			m_bIsTickFinished    = false;
		}

		m_iTickedNum = 0;
	}

	/**
	 * @brief 检查是否需要执行tick
	 * @details 根据运行索引和等待间隔判断是否需要执行处理
	 * @param dwCurRunIndex 当前运行索引
	 * @return bool true表示需要执行，false表示不需要执行
	 * @note 考虑了索引溢出的情况，确保逻辑正确性
	 */
	bool IsNeedTick(uint32_t dwCurRunIndex) const
	{
		return ((m_dwLastTickRunIndex + m_dwWaitTickIndex) <= dwCurRunIndex || (dwCurRunIndex < m_dwLastTickRunIndex));
	}

protected:
	/**
	 * @brief 上次执行tick的运行索引
	 * @details 记录外层tick上一次传入的索引值，用于计算时间间隔
	 */
	uint32_t m_dwLastTickRunIndex;
	
	/**
	 * @brief 内层tick已执行的对象个数
	 * @details 在当前处理轮次中，已经处理的对象数量
	 */
	int m_iLastTickIndex;
	
	/**
	 * @brief 本次tick执行的对象个数
	 * @details 当前tick调用中实际处理的对象数量
	 */
	int m_iTickedNum;
	
	/**
	 * @brief 等待tick间隔
	 * @details 外层调用多少次tick才真正执行一次内层的tick，控制处理频率
	 */
	uint32_t m_dwWaitTickIndex;
	
	/**
	 * @brief 每次tick处理的对象数量
	 * @details 每次执行时需要操作的对象个数，控制批次大小
	 */
	int m_iNumPerTick;
	
	/**
	 * @brief tick完成标志
	 * @details 标识本次处理轮次是否已经完成
	 */
	bool m_bIsTickFinished;

	/**
	 * @brief 是否需要调整tick频率
	 * @details 标识是否需要调整tick的执行频率
	 */
	bool m_bNeedChangeGap;
	
	/**
	 * @brief 目标执行时间
	 * @details 调整后希望达到的每轮执行时间（毫秒）
	 */
	int m_iDestTimePerRound;
	
	/**
	 * @brief 基础tick时间
	 * @details 服务器基础tick时间，一般为10ms，用于频率调整计算
	 */
	int m_iBaseTickMS;
};
