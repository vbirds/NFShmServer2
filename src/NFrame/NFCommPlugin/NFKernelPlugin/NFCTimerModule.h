// -------------------------------------------------------------------------
//    @FileName         :    NFCTimerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCTimerModule
//    @Desc             :    定时器模块头文件，提供高精度定时任务和时间调度功能。
//                          该文件定义了NFShmXFrame框架的定时器模块，提供完整的定时器和时间调度解决方案，
//                          包括间隔定时器、一次性定时器、有限次数定时器、固定时间定时器、日历定时器等功能。
//                          主要功能包括高精度时间轴管理、定时器启动和停止、批量定时器管理、
//                          线程安全的定时器操作、自动清理过期定时器、支持无限次调用
//    @Description      :    定时器模块头文件，提供高精度定时任务和时间调度功能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFITimerModule.h"
#include "NFTimerAxis.h"

/**
 * @class NFCTimerModule
 * @brief 定时器模块实现类
 *
 * NFCTimerModule提供了完整的定时器和时间调度解决方案：
 * 
 * 定时器功能：
 * - 间隔定时器：按指定时间间隔重复执行
 * - 一次性定时器：只执行一次的延时任务
 * - 有限次数定时器：执行指定次数后自动停止
 * - 定时器管理：启动、停止、批量管理定时器
 * 
 * 时钟功能：
 * - 固定时间定时器(Clocker)：每日固定时间执行
 * - 日历定时器(Calendar)：基于时间字符串的复杂调度
 * - 支持跨天定时任务
 * - 支持多天间隔的周期性任务
 * 
 * 高级特性：
 * - 高精度时间轴管理
 * - 支持无限次调用(INFINITY_CALL)
 * - 内存高效的定时器管理
 * - 线程安全的定时器操作
 * - 自动清理过期定时器
 * 
 * 应用场景：
 * - 游戏内定时任务（刷怪、重置等）
 * - 心跳检测和状态同步
 * - 延时消息处理
 * - 定时数据保存
 * - 活动时间管理
 * 
 * @note 定时器基于系统时间轴实现，具有毫秒级精度
 * @note 支持大量并发定时器而不影响性能
 */
class NFCTimerModule : public NFITimerModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFCTimerModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 */
	virtual ~NFCTimerModule();

public:
	/**
	 * @brief 初始化定时器模块
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Init() override;

	/**
	 * @brief 定时器模块主循环更新
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 在每一帧中检查和触发到期的定时器，这是定时器系统的核心驱动函数。
	 */
	virtual int Tick() override;

	/**
	 * @brief 关闭前处理
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int BeforeShut() override;

	/**
	 * @brief 关闭定时器模块
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 清理所有定时器资源并停止定时器服务。
	 */
	virtual int Shut() override;

public:
	/**
	 * @brief 设置间隔定时器
	 * @param nTimerID 定时器唯一标识ID
	 * @param nInterVal 执行间隔时间（毫秒）
	 * @param handler 定时器处理对象，必须继承自NFTimerObjBase
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 返回true表示设置成功，false表示失败
	 * 
	 * 创建一个按指定间隔重复执行的定时器。当定时器触发时，
	 * 会调用handler对象的OnTimer方法。
	 * 
	 * @note 同一个handler的相同TimerID会覆盖之前的定时器
	 * @note 如果nCallCount为1，则等同于一次性延时任务
	 */
	virtual bool SetTimer(uint32_t nTimerID, uint64_t nInterVal, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL) override;

	/**
	 * @brief 关闭指定定时器
	 * @param nTimerID 定时器ID
	 * @param handler 定时器处理对象
	 * @return 返回true表示关闭成功，false表示定时器不存在
	 * 
	 * 停止并移除指定的定时器。
	 */
	virtual bool KillTimer(uint32_t nTimerID, NFTimerObjBase* handler) override;

	/**
	 * @brief 关闭指定对象的所有定时器
	 * @param handler 定时器处理对象
	 * @return 返回true表示关闭成功，false表示没有找到相关定时器
	 * 
	 * 批量停止并移除某个对象注册的所有定时器，
	 * 通常在对象销毁时调用以避免野指针。
	 */
	virtual bool KillAllTimer(NFTimerObjBase* handler) override;

	/**
	 * @brief 设置固定时间定时器（时钟模式）
	 * @param nTimerID 定时器唯一标识ID
	 * @param nStartTime 开始时间戳（Unix时间戳）
	 * @param nInterDays 间隔天数，0表示每天执行
	 * @param handler 定时器处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 返回true表示设置成功，false表示失败
	 * 
	 * 创建在每天固定时间或按天数间隔执行的定时器。
	 * 适用于每日重置、周期性活动等场景。
	 * 
	 * @note nStartTime应该是完整的时间戳，系统会提取其中的时分秒作为每日执行时间
	 * @note nInterDays=0表示每天执行，nInterDays=1表示隔天执行，以此类推
	 */
	virtual bool SetClocker(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterDays, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL) override;

	/**
	 * @brief 设置日历定时器（字符串时间模式）
	 * @param nTimerID 定时器唯一标识ID
	 * @param timeStr 时间字符串，支持复杂的时间表达式
	 * @param handler 定时器处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 返回true表示设置成功，false表示时间字符串格式错误
	 * 
	 * 基于时间字符串创建复杂的调度定时器。
	 * 支持Cron表达式风格的时间配置。
	 * 
	 * @note timeStr的具体格式取决于NFTimerAxis的实现
	 * @note 适用于复杂的业务调度需求
	 */
	virtual bool SetCalender(uint32_t nTimerID, const std::string& timeStr, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL) override;

protected:
	/**
	 * @brief 时间轴管理器
	 * 
	 * 负责底层的时间轴管理和定时器调度，
	 * 提供高精度的时间计算和定时器触发机制。
	 */
	NFTimerAxis mTimerAxis;
};

