// -------------------------------------------------------------------------
//    @FileName         :    NFITimerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    定时器模块接口定义，提供灵活的定时任务管理功能
//                           支持普通定时器、固定时间定时器和日历定时器等多种类型
//
// -------------------------------------------------------------------------
#pragma once

#include "NFIModule.h"

class NFTimerObjBase;

/**
 * @brief 定时器模块接口类
 * 
 * NFITimerModule 提供了完整的定时器管理功能，支持多种类型的定时任务：
 * 
 * 1. 普通定时器：
 *    - 基于时间间隔的重复执行
 *    - 支持指定执行次数或无限循环
 * 
 * 2. 固定时间定时器（Clocker）：
 *    - 在指定的绝对时间点触发
 *    - 支持按天重复执行
 * 
 * 3. 日历定时器：
 *    - 基于日历时间的复杂定时
 *    - 支持时间字符串格式配置
 * 
 * 定时器特性：
 * - 每个定时器具有唯一ID
 * - 支持定时器的创建、删除和批量管理
 * - 通过回调对象处理定时器事件
 * - 线程安全的定时器管理
 */
class NFITimerModule : public NFIModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFITimerModule(NFIPluginManager* p) :NFIModule(p)
	{

	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构，清理所有定时器资源。
	 */
	virtual ~NFITimerModule()
	{

	}
	
	/**
	 * @brief 设置定时器
	 * @param nTimerID 定时器唯一标识ID
	 * @param nInterVal 定时器间隔时间（毫秒）
	 * @param handler 定时器事件处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 成功返回true，失败返回false
	 * 
	 * 创建一个按固定间隔重复执行的定时器。当定时器触发时，
	 * 会调用handler对象的相应回调方法。
	 * 
	 * 使用示例：
	 * @code
	 * // 设置一个每秒执行一次，总共执行10次的定时器
	 * SetTimer(1001, 1000, this, 10);
	 * 
	 * // 设置一个无限循环的定时器
	 * SetTimer(1002, 5000, this);
	 * @endcode
	 */
	virtual bool SetTimer(uint32_t nTimerID, uint64_t nInterVal, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL) = 0;

	/**
	 * @brief 关闭指定定时器
	 * @param nTimerID 要关闭的定时器ID
	 * @param handler 定时器关联的处理对象
	 * @return 成功返回true，失败返回false
	 * 
	 * 根据定时器ID和处理对象关闭指定的定时器。
	 * 需要同时匹配ID和处理对象才能成功关闭。
	 */
	virtual bool KillTimer(uint32_t nTimerID, NFTimerObjBase* handler) = 0;

	/**
	 * @brief 关闭指定对象的所有定时器
	 * @param handler 定时器处理对象
	 * @return 成功返回true，失败返回false
	 * 
	 * 关闭与指定处理对象关联的所有定时器。
	 * 通常在对象销毁时调用，确保不会产生悬挂的定时器。
	 */
	virtual bool KillAllTimer(NFTimerObjBase* handler) = 0;

	/**
	 * @brief 设置固定时间定时器（时钟定时器）
	 * @param nTimerID 定时器唯一标识ID
	 * @param nStartTime 起始时间（Unix时间戳，毫秒）
	 * @param nInterDays 间隔天数
	 * @param handler 定时器事件处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 成功返回true，失败返回false
	 * 
	 * 创建一个在固定时间点触发的定时器，支持按天重复。
	 * 适用于需要在特定时间执行的任务，如每日重置、周期性维护等。
	 * 
	 * 使用示例：
	 * @code
	 * // 从明天开始，每天的某个时间点执行
	 * uint64_t tomorrow = GetTomorrowTimestamp();
	 * SetClocker(2001, tomorrow, 1, this);
	 * @endcode
	 */
	virtual bool SetClocker(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterDays, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL) = 0;

	/**
	 * @brief 设置日历定时器
	 * @param nTimerID 定时器唯一标识ID
	 * @param timeStr 时间字符串（格式如"2023-12-25 10:30:00"）
	 * @param handler 定时器事件处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 成功返回true，失败返回false
	 * 
	 * 根据时间字符串创建定时器，支持复杂的时间表达式。
	 * 适用于需要基于日历时间的定时任务。
	 * 
	 * 使用示例：
	 * @code
	 * // 在指定的日期时间执行
	 * SetCalender(3001, "2023-12-25 00:00:00", this, 1);
	 * @endcode
	 */
	virtual bool SetCalender(uint32_t nTimerID, const std::string& timeStr, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL) = 0;
};

