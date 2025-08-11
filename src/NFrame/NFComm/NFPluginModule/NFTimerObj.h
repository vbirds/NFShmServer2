// -------------------------------------------------------------------------
//    @FileName         :    NFTimerObj.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    定时器对象基类定义，提供统一的定时器管理接口
//
// -------------------------------------------------------------------------

/**
 * @file NFTimerObj.h
 * @brief 定时器对象基类定义
 * @details 该文件定义了定时器功能的基础架构，主要包含：
 *          - 定时器基类接口（NFTimerObjBase）
 *          - 具体定时器对象实现（NFTimerObj）
 *          - 支持普通定时器和固定时间定时器
 *          - 与插件管理器集成，提供模块化的定时器服务
 * 
 * **设计特点：**
 * - 纯虚接口设计，支持多态和可扩展性
 * - 内存管理优化，避免重复分配
 * - 支持一次性和循环定时器
 * - 线程安全的定时器操作
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

#pragma once
#include <stdint.h>
#include "NFComm/NFCore/NFPlatform.h"
#include "NFBaseObj.h"

class NFIPluginManager;

/**
 * @class NFTimerObjBase
 * @brief 定时器对象基类
 * @details 定义了定时器功能的核心接口，所有需要定时器功能的类都应该继承此基类。
 * 
 * **核心功能：**
 * - **定时器管理**：创建、删除、查询定时器
 * - **多种定时器**：支持普通定时器和固定时间定时器
 * - **内存优化**：内置定时器信息指针，避免重复分配
 * - **生命周期管理**：自动清理定时器资源
 * 
 * **设计模式：**
 * - 使用纯虚函数定义接口，强制子类实现
 * - 提供内部指针管理，优化内存使用
 * - 支持多态调用，便于统一管理
 * 
 * **使用场景：**
 * - 游戏业务逻辑的定时任务
 * - 系统监控和状态检查
 * - 网络超时和重连机制
 * - 资源定期清理和维护
 * 
 * @note 这是一个抽象基类，不能直接实例化，需要子类实现具体功能
 */
class NFTimerObjBase
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化定时器基类，将内部指针设置为nullptr。
     * 子类应在此基础上进行进一步初始化。
     */
	NFTimerObjBase():m_pTimerInfoPtr(nullptr), m_pFixTimerInfoPtr(nullptr)
    {

    }

    /**
     * @brief 析构函数
     * 
     * 清理定时器基类资源，将内部指针重置为nullptr。
     * 子类应确保在析构前正确清理所有定时器。
     * 
     * @note 虚析构函数确保多态删除的正确性
     */
	virtual ~NFTimerObjBase()
    {
        m_pTimerInfoPtr = nullptr;
        m_pFixTimerInfoPtr = nullptr;
    }

public:
    /**
     * @brief 定时器回调函数（纯虚函数）
     * @param nTimerID 触发的定时器ID
     * @return 处理结果，0表示成功，其他值表示错误
     * 
     * **功能说明：**
     * 当定时器到期时，系统会自动调用此函数。
     * 子类必须实现此函数来处理具体的定时器逻辑。
     * 
     * **实现要求：**
     * - 处理速度要快，避免阻塞定时器系统
     * - 异常安全，不应抛出未捕获的异常
     * - 根据TimerID区分不同的定时器任务
     * 
     * **注意事项：**
     * @warning 在回调中删除定时器是安全的
     * @warning 避免在回调中执行耗时操作
     * @warning 回调函数应该是线程安全的
     */
	virtual int OnTimer(uint32_t nTimerID) = 0;

    /**
     * @brief 设置定时器（纯虚函数）
     * @param nTimerID 定时器ID，用于标识和管理定时器
     * @param nInterVal 时间间隔（毫秒），定时器触发的间隔时间
     * @param nCallCount 调用次数，0表示无限循环，>0表示指定次数后停止
     * @return 设置是否成功
     * 
     * **功能说明：**
     * 创建一个新的定时器或更新现有定时器的配置。
     * 
     * **参数说明：**
     * - nTimerID：唯一标识符，同一对象内不能重复
     * - nInterVal：毫秒级精度，最小间隔建议不少于10ms
     * - nCallCount：0=永久循环，1=只执行一次，N=执行N次后停止
     * 
     * **使用示例：**
     * @code
     * // 创建一个每秒触发的永久定时器
     * SetTimer(1001, 1000, 0);
     * 
     * // 创建一个5秒后只执行一次的定时器
     * SetTimer(1002, 5000, 1);
     * @endcode
     */
	virtual bool SetTimer(uint32_t nTimerID, uint64_t nInterVal, uint32_t nCallCount = 0) = 0;

    /**
     * @brief 删除指定定时器（纯虚函数）
     * @param nTimerID 要删除的定时器ID
     * @return 删除是否成功
     * 
     * **功能说明：**
     * 删除指定ID的定时器，停止其继续触发。
     * 
     * **行为特点：**
     * - 立即停止定时器的触发
     * - 清理相关资源和内存
     * - 删除不存在的定时器返回false但不会出错
     * 
     * **线程安全：**
     * 在定时器回调函数内调用此方法是安全的。
     */
	virtual bool KillTimer(uint32_t nTimerID) = 0;

    /**
     * @brief 删除所有定时器（纯虚函数）
     * @return 删除是否成功
     * 
     * **功能说明：**
     * 一次性删除当前对象的所有定时器，通常在对象销毁时调用。
     * 
     * **清理过程：**
     * - 停止所有定时器的触发
     * - 清理所有定时器相关资源
     * - 重置内部状态为初始状态
     * 
     * **使用场景：**
     * - 对象析构时的清理
     * - 重置对象状态
     * - 紧急停止所有定时任务
     * 
     * @note 建议在析构函数中调用此方法
     */
	virtual bool KillAllTimer() = 0;

    /**
     * @brief 设置固定时间定时器（纯虚函数）
     * @param nTimerID 定时器ID
     * @param nStartTime 开始时间（Unix时间戳，秒）
     * @param nInterSec 间隔时间（秒）
     * @param nCallCount 调用次数，0表示无限循环
     * @return 设置是否成功
     * 
     * **功能说明：**
     * 创建一个在指定时间点开始执行的定时器，适用于需要精确时间控制的场景。
     * 
     * **时间精度：**
     * - 开始时间：秒级精度，使用Unix时间戳
     * - 间隔时间：秒级精度，不支持毫秒
     * 
     * **应用场景：**
     * - 每日定时任务（如0:00执行）
     * - 活动开始和结束时间控制
     * - 定期数据备份和维护
     * - 跨时区的时间同步
     * 
     * **注意事项：**
     * @warning 如果开始时间已过，定时器会立即开始执行
     * @note 建议配合时区转换使用，确保时间准确性
     */
	virtual bool SetFixTimer(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterSec, uint32_t nCallCount = 0) = 0;

public:
    /**
     * @brief 获取定时器信息指针的地址
     * @return 指向定时器信息指针的指针
     * 
     * **功能说明：**
     * 返回内部定时器信息指针的地址，供定时器管理系统使用。
     * 这是一个内部接口，用于优化内存管理和提高性能。
     * 
     * **设计目的：**
     * - 避免重复分配定时器管理结构
     * - 提供统一的内存管理接口
     * - 支持定时器系统的内部优化
     * 
     * @note 这是一个内部接口，一般不需要直接调用
     */
	virtual void** GetTimerInfoPtr()
	{
		return &m_pTimerInfoPtr;
	}

    /**
     * @brief 获取固定时间定时器信息指针的地址
     * @return 指向固定时间定时器信息指针的指针
     * 
     * **功能说明：**
     * 返回内部固定时间定时器信息指针的地址，供定时器管理系统使用。
     * 与普通定时器分开管理，优化不同类型定时器的处理。
     * 
     * @note 这是一个内部接口，一般不需要直接调用
     */
	virtual void** GetFixTimerInfoPtr()
	{
		return &m_pFixTimerInfoPtr;
	}

private:
	void* m_pTimerInfoPtr;        ///< 普通定时器信息指针，由定时器管理系统维护
	void* m_pFixTimerInfoPtr;     ///< 固定时间定时器信息指针，由定时器管理系统维护
};

/**
 * @struct NFTimerObj
 * @brief 具体的定时器对象实现
 * @details 继承自NFTimerObjBase和NFBaseObj，提供完整的定时器功能实现。
 * 
 * **继承关系：**
 * - NFTimerObjBase：提供定时器接口定义
 * - NFBaseObj：提供插件管理器访问能力
 * 
 * **实现特点：**
 * - 与插件管理器深度集成
 * - 自动注册到全局定时器管理系统
 * - 支持所有基类定义的定时器功能
 * - 提供默认的资源清理机制
 * 
 * **使用方式：**
 * 通常作为基类被具体的业务类继承，业务类只需实现OnTimer方法。
 * 
 * @code
 * class MyBusinessClass : public NFTimerObj {
 * public:
 *     MyBusinessClass(NFIPluginManager* pMgr) : NFTimerObj(pMgr) {}
 *     
 *     virtual int OnTimer(uint32_t nTimerID) override {
 *         // 处理定时器逻辑
 *         return 0;
 *     }
 * };
 * @endcode
 */
struct NFTimerObj : public NFTimerObjBase, public NFBaseObj
{
public:
    /**
     * @brief 构造函数
     * @param pPluginManager 插件管理器指针
     * 
     * 初始化定时器对象，建立与插件管理器的关联。
     * 通过插件管理器可以访问定时器管理模块和其他系统服务。
     */
    NFTimerObj(NFIPluginManager* pPluginManager);
    
    /**
     * @brief 析构函数
     * 
     * 清理定时器对象资源，自动删除所有关联的定时器。
     * 确保对象销毁时不会有遗留的定时器继续触发。
     */
    virtual ~NFTimerObj();

public:
    /**
     * @brief 定时器回调函数（纯虚函数）
     * @param nTimerID 触发的定时器ID
     * @return 处理结果，0表示成功
     * 
     * 子类必须实现此方法来处理具体的定时器逻辑。
     * 实现要求和注意事项同基类说明。
     */
    virtual int OnTimer(uint32_t nTimerID) = 0;

    /**
     * @brief 设置定时器
     * @param nTimerID 定时器ID
     * @param nInterVal 时间间隔（毫秒）
     * @param nCallCount 调用次数，0表示无限循环
     * @return 设置是否成功
     * 
     * 具体实现会通过插件管理器访问定时器模块，
     * 在全局定时器系统中注册定时器。
     */
    virtual bool SetTimer(uint32_t nTimerID, uint64_t nInterVal, uint32_t nCallCount = 0);

    /**
     * @brief 删除指定定时器
     * @param nTimerID 要删除的定时器ID
     * @return 删除是否成功
     * 
     * 从全局定时器系统中移除指定的定时器。
     */
    virtual bool KillTimer(uint32_t nTimerID);

    /**
     * @brief 删除所有定时器
     * @return 删除是否成功
     * 
     * 删除当前对象的所有定时器，释放相关资源。
     */
    virtual bool KillAllTimer();

    /**
     * @brief 设置固定时间定时器
     * @param nTimerID 定时器ID
     * @param nStartTime 开始时间（Unix时间戳，秒）
     * @param nInterSec 间隔时间（秒）
     * @param nCallCount 调用次数，0表示无限循环
     * @return 设置是否成功
     * 
     * 在指定的绝对时间点开始执行的定时器。
     */
    virtual bool SetFixTimer(uint32_t nTimerID, uint64_t nStartTime, uint32_t nInterSec, uint32_t nCallCount = 0);
};

