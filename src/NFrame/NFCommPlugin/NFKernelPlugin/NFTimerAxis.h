// -------------------------------------------------------------------------
//    @FileName         :    NFTimerAxis.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFTimerAxis
//    @Description      :    时间轴头文件，提供高精度定时器的底层实现
//
// -------------------------------------------------------------------------
#pragma once
#include <list>
#include <vector>
#include "NFComm/NFCore/NFSingleton.hpp"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"

class NFTimerObjBase;

/**
 * @class NFTimerAxis
 * @brief 时间轴定时器实现类
 *
 * NFTimerAxis是定时器系统的核心实现，提供了高精度、高效率的定时器管理：
 * 
 * 时间轴设计：
 * - 双时间轴：毫秒级时间轴和秒级时间轴
 * - 时间分片：将时间划分为固定大小的时间片
 * - 快速定位：O(1)复杂度的定时器插入和删除
 * - 内存高效：基于时间轮算法的内存优化
 * 
 * 定时器功能：
 * - 毫秒定时器：支持毫秒级精度的定时任务
 * - 秒定时器：优化的秒级定时器，减少检查频率
 * - 重复定时器：支持指定次数的重复执行
 * - 一次性定时器：单次执行的延时任务
 * - 时钟定时器：基于绝对时间的定时器
 * 
 * 高级特性：
 * - 时间校正：自动处理系统时间调整
 * - 负载均衡：智能分散定时器触发时间
 * - 快速查找：基于时间片的快速定时器定位
 * - 内存复用：定时器对象的内存池管理
 * 
 * 性能优化：
 * - 时间轮算法：减少定时器检查的时间复杂度
 * - 批量处理：批量处理到期的定时器
 * - 缓存友好：优化的数据结构布局
 * - 最小堆：对于长期定时器使用堆结构
 * 
 * 应用场景：
 * - 游戏定时任务：技能冷却、状态效果等
 * - 网络超时：连接超时、请求超时检测
 * - 系统维护：定期清理、数据同步等
 * - 业务逻辑：定时活动、定时推送等
 * 
 * @note 基于时间轮算法实现，提供O(1)的插入和删除性能
 * @note 支持系统时间调整和夏令时变化
 * @warning 定时器回调不应执行长时间阻塞操作
 */
class NFTimerAxis
{
public:
	/**
	 * @brief 构造函数
	 */
	NFTimerAxis();
	
	/**
	 * @brief 析构函数
	 */
	virtual ~NFTimerAxis();

	/**
	 * @brief 初始化时间轴
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 初始化毫秒和秒级时间轴，创建时间片数组。
	 */
	bool Init();
	
	/**
	 * @brief 反初始化时间轴
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 清理所有定时器，释放时间轴资源。
	 */
	bool UnInit();

	/**
	 * @brief 设置定时器
	 * @param nTimerID 定时器唯一标识ID
	 * @param nInterVal 执行间隔时间（毫秒）
	 * @param handler 定时器处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 返回true表示设置成功，false表示失败
	 * 
	 * 创建一个按指定间隔重复执行的定时器。
	 * 根据间隔时间自动选择毫秒或秒级时间轴。
	 */
	bool SetTimer(uint32_t nTimerID, uint64_t nInterVal, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);
	
	/**
	 * @brief 关闭指定定时器
	 * @param nTimerID 定时器ID
	 * @param handler 定时器处理对象
	 * @return 返回true表示关闭成功，false表示定时器不存在
	 */
	bool KillTimer(uint32_t nTimerID, NFTimerObjBase* handler);
	
	/**
	 * @brief 关闭指定对象的所有定时器
	 * @param handler 定时器处理对象
	 * @return 返回true表示关闭成功
	 * 
	 * 批量移除某个对象注册的所有定时器。
	 */
	bool KillAllTimer(NFTimerObjBase* handler);
	
	/**
	 * @brief 更新时间轴，检查并触发到期定时器
	 * 
	 * 这是时间轴的核心驱动函数，需要在主循环中定期调用。
	 * 检查毫秒和秒级时间轴，触发所有到期的定时器。
	 */
	void Update();

	/**
	 * @brief 设置固定时间定时器（时钟模式）
	 * @param nTimerID 定时器唯一标识ID
	 * @param nStartTime 开始时间戳（Unix时间戳）
	 * @param nInterSec 间隔秒数，0表示每天执行
	 * @param handler 定时器处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 返回true表示设置成功，false表示失败
	 * 
	 * 创建在每天固定时间或按天数间隔执行的定时器。
	 */
	bool SetClocker(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterSec, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);
	
	/**
	 * @brief 设置日历定时器（字符串时间模式）
	 * @param nTimerID 定时器唯一标识ID
	 * @param timeStr 时间字符串，支持复杂的时间表达式
	 * @param handler 定时器处理对象
	 * @param nCallCount 调用次数，默认为INFINITY_CALL（无限次）
	 * @return 返回true表示设置成功，false表示时间格式错误
	 * 
	 * 基于时间字符串创建复杂的调度定时器。
	 */
	bool SetCalender(uint32_t nTimerID, const std::string& timeStr, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);

private:
	/**
	 * @brief 设置秒级定时器
	 * @param nTimerID 定时器ID
	 * @param nInterVal 间隔时间（秒）
	 * @param handler 定时器处理对象
	 * @param nCallCount 调用次数
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 内部方法，用于创建秒级精度的定时器。
	 */
	bool SetTimerSec(uint32_t nTimerID, uint64_t nInterVal, NFTimerObjBase* handler, uint32_t nCallCount = INFINITY_CALL);
	
	/**
	 * @brief 检查毫秒级时间轴
	 * 
	 * 检查毫秒级时间轴上的定时器，触发到期的定时器。
	 */
	void CheckTick();
	
	/**
	 * @brief 更新秒级时间轴
	 * 
	 * 检查秒级时间轴上的定时器，触发到期的定时器。
	 */
	void UpdateSec();

protected:
	/**
	 * @brief 获取当前毫秒时间戳
	 * @return 返回当前毫秒时间戳
	 */
	static uint64_t GetTick()
	{
		return NF_ADJUST_TIMENOW_MS();
	}

	/**
	 * @brief 获取当前Unix秒时间戳
	 * @return 返回当前Unix秒时间戳
	 */
	static uint64_t GetUnixSec()
	{
		return NF_ADJUST_TIMENOW();
	}

	/**
	 * @struct Timer
	 * @brief 定时器数据结构
	 * 
	 * 存储单个定时器的所有信息，包括时间参数、回调对象、
	 * 在时间轴中的位置等。
	 */
	struct Timer
	{
		uint32_t nTimerID;      ///< 定时器ID
		uint64_t nInterVal;     ///< 间隔时间
		uint32_t nCallCount;    ///< 调用次数
		uint64_t nLastTick;     ///< 最后一次触发时间
		NFTimerObjBase* pHandler; ///< 回调处理对象指针
		uint8_t byType;         ///< 类型：0-毫秒定时器，1-秒定时器
		uint32_t nGridIndex;    ///< 所在的时间刻度索引
		std::list<Timer*>::iterator pos; ///< 在时间轴中的位置迭代器，便于快速定位
		
		/**
		 * @brief 构造函数
		 * 
		 * 初始化所有成员变量为默认值。
		 */
		Timer()
		{
			nTimerID = 0;
			nInterVal = 0;
			nCallCount = 0;
			nLastTick = 0;
			pHandler = nullptr;
			byType = 0;
			nGridIndex = 0;
		}
	};

	/**
	 * @brief 重置定时器在时间轴上的位置
	 * @param pTimer 定时器指针
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 当定时器的触发时间发生变化时，重新计算并设置其在时间轴上的位置。
	 */
	static bool ResetTimerPos(Timer* pTimer);

	typedef std::list<Timer*> TIMER_LIST;      ///< 某个时间刻度中的定时器列表
	typedef std::vector<TIMER_LIST*> TIMER_AXIS; ///< 整个时间轴的定时器数组

	/**
	 * @brief 毫秒级时间轴
	 * 
	 * 用于管理毫秒精度的定时器，提供高精度的定时功能。
	 */
	TIMER_AXIS m_TimerAxis;
	
	/**
	 * @brief 毫秒时间轴初始Tick
	 * 
	 * 记录毫秒时间轴初始化时的时间戳，用于相对时间计算。
	 */
	uint64_t m_nInitTick;
	
	/**
	 * @brief 毫秒时间轴最后检查的Tick
	 * 
	 * 记录上次检查毫秒时间轴的时间戳，用于增量更新。
	 */
	uint64_t m_nLastTick;

	/**
	 * @brief 秒级时间轴
	 * 
	 * 用于管理秒精度的定时器，减少长期定时器的检查开销。
	 */
	TIMER_AXIS m_TimerAxisSec;
	
	/**
	 * @brief 秒时间轴初始时间
	 * 
	 * 记录秒时间轴初始化时的Unix时间戳。
	 */
	uint64_t m_nInitSec;
	
	/**
	 * @brief 秒时间轴最后检查的时间
	 * 
	 * 记录上次检查秒时间轴的Unix时间戳。
	 */
	uint64_t m_nLastSec;
};


