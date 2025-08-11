// -------------------------------------------------------------------------
//    @FileName         :    NFSchedule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFSchedule
//    @Description      :    协程调度器头文件，提供底层协程调度和上下文切换功能
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFCore/NFPlatform.h"

#include <functional>
#include <list>
#include <set>
#include <string.h>
#include <stdint.h>
#include <unordered_map>

#if NF_PLATFORM == NF_PLATFORM_LINUX
#include <sys/poll.h>
#include <ucontext.h>
#else

#endif

#include "NFCoroutine.h"
#include "google/protobuf/message.h"

#ifdef Yield
#undef Yield
#endif

/**
 * @class NFSchedule
 * @brief 协程调度器核心实现类
 *
 * NFSchedule是协程系统的底层调度器，提供了协程的基础调度和上下文切换功能：
 * 
 * 调度器核心功能：
 * - 协程创建：动态创建和管理协程实例
 * - 上下文切换：高效的协程上下文保存和恢复
 * - 调度管理：协程的运行状态管理和调度
 * - 内存管理：协程栈内存的分配和回收
 * 
 * 跨平台支持：
 * - Linux平台：基于ucontext的协程实现
 * - Windows平台：基于Fiber的协程实现
 * - 统一接口：提供一致的跨平台协程API
 * - 性能优化：针对不同平台的性能优化
 * 
 * 内存优化：
 * - 栈复用：协程结束后栈内存可复用
 * - 空闲池：维护空闲协程对象池
 * - 懒删除：延迟删除机制减少内存碎片
 * - 自定义栈大小：支持配置协程栈大小
 * 
 * 状态管理：
 * - 运行状态跟踪：实时跟踪协程运行状态
 * - 生命周期管理：完整的协程生命周期控制
 * - 错误处理：协程异常的捕获和处理
 * - 资源清理：自动清理协程相关资源
 * 
 * 高级特性：
 * - 用户数据：协程可携带自定义protobuf数据
 * - 执行统计：协程执行次数和时间统计
 * - 调试支持：协程调试和性能分析
 * - 优雅关闭：服务器停止时的协程清理
 * 
 * @note 这是协程系统的底层实现，通常通过NFCoroutineSchedule使用
 * @note 支持Linux ucontext和Windows Fiber两种实现
 * @warning 协程栈大小需要根据实际使用情况合理配置
 */
class NFSchedule {
public:
	/**
	 * @brief 构造函数
	 * @param stackSize 协程栈大小（字节），默认值需外部指定
	 * 
	 * 初始化协程调度器，设置栈大小并准备主上下文。
	 */
	NFSchedule(uint32_t stackSize)
	{
	    running = -1;
	    co_free_num = 0;
	    stack_size = stackSize;
#if NF_PLATFORM == NF_PLATFORM_LINUX
        memset(&main, 0, sizeof(ucontext_t));
#else
		main = ConvertThreadToFiber(nullptr);
#endif
	}

	/**
	 * @brief 析构函数
	 * 
	 * 清理所有协程资源，包括运行中的协程和空闲协程池。
	 */
	~NFSchedule()
	{
        for(auto iter = co_hash_map.begin(); iter != co_hash_map.end(); iter++)
        {
            if (iter->second)
            {
                NF_SAFE_DELETE(iter->second);
            }
        }

        for(auto iter = co_free_list.begin(); iter != co_free_list.end(); iter++)
        {
           NF_SAFE_DELETE(*iter);
        }
#if NF_PLATFORM == NF_PLATFORM_LINUX
        memset(&main, 0, sizeof(ucontext_t));
#else
        ConvertFiberToThread();
		main = NULL;
#endif
	}

	/**
	 * @brief 检查服务器停止状态
	 * @return 返回0表示可以停止，非0表示还有协程在运行
	 * 
	 * 检查是否还有未完成的协程，用于优雅停机。
	 */
	int CheckStopServer();

	/**
	 * @brief 创建一个协程（std::function版本）
	 * @param std_func 协程执行体的std::function对象
	 * @return 返回协程对象指针，失败返回nullptr
	 * 
	 * @note 只能够在主线程调用
	 */
	NFCoroutine *NewCoroutine(const std::function<void()>& std_func);

	/**
	 * @brief 创建一个协程（函数指针版本）
	 * @param func 协程执行体的函数指针
	 * @param ud 传递给协程函数的用户数据
	 * @return 返回协程对象指针，失败返回nullptr
	 * 
	 * @note 只能够在主线程调用
	 */
	NFCoroutine *NewCoroutine(NFCoroutineFunc func, void *ud);

	/**
	 * @brief 创建一个协程并返回ID（std::function版本）
	 * @param func 协程执行体的std::function对象
	 * @return 返回协程ID，失败返回-1
	 * 
	 * @note 只能够在主线程调用
	 */
	int64_t CreateCoroutine(const std::function<void()>& func);

	/**
	 * @brief 创建一个协程并返回ID（函数指针版本）
	 * @param func 协程执行体的函数指针
	 * @param ud 传递给协程函数的用户数据
	 * @return 返回协程ID，失败返回-1
	 * 
	 * @note 只能够在主线程调用
	 */
	int64_t CreateCoroutine(NFCoroutineFunc func, void *ud);

	/**
	 * @brief 恢复一个协程的运行
	 * @param id 协程ID
	 * @param result Resume时传递的结果，默认为0
	 * @return 返回处理结果，参见CoroutineErrorCode
	 * 
	 * 将指定协程从挂起状态恢复到运行状态。
	 * 
	 * @note 只能够在主线程调用
	 */
	int32_t CoroutineResume(int64_t id, int32_t result = 0);

	/**
	 * @brief 获取协程当前状态
	 * @param id 协程ID
	 * @return 返回协程运行状态，参见协程状态枚举
	 */
	int CoroutineStatus(int64_t id);

	/**
	 * @brief 获取协程的用户数据
	 * @param id 协程ID
	 * @return 返回协程携带的protobuf消息指针，不存在返回nullptr
	 */
    google::protobuf::Message *CoroutineUserData(int64_t id);
    
    /**
     * @brief 设置协程的用户数据
     * @param id 协程ID
     * @param pUserData protobuf消息指针
     * @return 返回0表示成功，非0表示失败
     */
    int CoroutineSetUserData(int64_t id, google::protobuf::Message *pUserData);

	/**
	 * @brief 获取当前正在运行的协程ID
	 * @return 返回正在运行的协程ID，-1表示在主线程中
	 */
	int64_t CoroutineRunning();

	/**
	 * @brief 挂起当前协程的运行
	 * @return 返回处理结果，参见CoroutineErrorCode
	 * 
	 * 暂停当前协程的执行，将控制权返回给主线程。
	 * 
	 * @note 只能够在协程内调用
	 */
	int32_t CoroutineYield();

public:
#if NF_PLATFORM == NF_PLATFORM_LINUX
    /**
     * @brief 主线程上下文（Linux平台）
     * 
     * 存储主线程的ucontext，用于协程和主线程间的上下文切换。
     */
    ucontext_t main;
#else
	/**
	 * @brief 主线程Fiber句柄（Windows平台）
	 * 
	 * 存储主线程的Fiber句柄，用于协程和主线程间的切换。
	 */
	void *main;
#endif

    /**
     * @brief 当前正在运行的协程ID
     * 
     * -1表示当前在主线程中执行，其他值表示正在运行的协程ID。
     */
    int64_t running;
    
    /**
     * @brief 协程哈希映射表
     * 
     * 以协程ID为键，协程对象指针为值的映射表，
     * 用于快速查找和管理协程实例。
     */
    std::unordered_map<int64_t, NFCoroutine*> co_hash_map;
    
    /**
     * @brief 空闲协程对象列表
     * 
     * 存储已结束但未删除的协程对象，用于对象复用，
     * 减少频繁的内存分配和释放开销。
     */
    std::list<NFCoroutine*> co_free_list;
    
    /**
     * @brief 空闲协程对象数量
     * 
     * 记录空闲协程池中的对象数量，用于内存管理优化。
     */
    int32_t co_free_num;
    
    /**
     * @brief 协程栈大小
     * 
     * 每个协程分配的栈空间大小（字节），
     * 影响协程的内存使用和嵌套调用深度。
     */
    uint32_t stack_size;
};
