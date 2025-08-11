// -------------------------------------------------------------------------
//    @FileName         :    NFCoroutine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCoroutine
//    @Description      :    协程基础头文件，定义协程的核心数据结构和状态常量
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFPluginModule/NFError.h"
#include "google/protobuf/message.h"

#include <functional>
#include <list>
#include <set>
#include <string.h>
#include <stdint.h>

#if NF_PLATFORM == NF_PLATFORM_LINUX
#include <sys/poll.h>
#include <ucontext.h>
#else
#include <WinSock2.h>
#include <Windows.h>
#endif

/**
 * @defgroup CoroutineStates 协程状态常量
 * @brief 定义协程的各种运行状态
 * @{
 */
#define NF_COROUTINE_DEAD 0      ///< 协程已死亡状态
#define NF_COROUTINE_READY 1     ///< 协程就绪状态，等待执行
#define NF_COROUTINE_RUNNING 2   ///< 协程正在运行状态
#define NF_COROUTINE_SUSPEND 3   ///< 协程挂起状态
/** @} */

/**
 * @defgroup CoroutineConstants 协程系统常量
 * @brief 定义协程系统的重要常量
 * @{
 */
#define MAX_FREE_CO_NUM     1024    ///< 最大空闲协程数量
#define INVALID_CO_ID       -1      ///< 无效协程ID
/** @} */

class NFSchedule;

/**
 * @typedef NFCoroutineFunc
 * @brief 协程函数指针类型定义
 * @param sch 协程调度器指针
 * @param ud 用户数据指针
 * 
 * 定义了协程执行函数的标准签名，所有协程函数都必须符合此接口。
 */
typedef void (*NFCoroutineFunc)(NFSchedule *, void *ud);

/**
 * @class NFCoroutine
 * @brief 协程基础数据结构类
 *
 * NFCoroutine是协程系统的基础数据结构，包含了协程的所有核心信息：
 * 
 * 协程基础组件：
 * - 执行函数：支持函数指针和std::function两种方式
 * - 上下文：保存协程的CPU寄存器状态和栈指针
 * - 协程栈：独立的执行栈空间
 * - 状态管理：协程的运行状态跟踪
 * 
 * 跨平台支持：
 * - Linux平台：使用ucontext_t进行上下文切换
 * - Windows平台：使用Fiber API进行上下文切换
 * - 统一接口：提供一致的协程操作接口
 * 
 * 数据携带功能：
 * - 用户数据：支持void*类型的自定义数据
 * - Protobuf数据：支持结构化的protobuf消息数据
 * - 执行结果：Resume时的结果传递机制
 * 
 * 内存管理：
 * - 栈分配：动态分配协程执行栈
 * - 自动释放：析构时自动释放栈内存
 * - 栈大小：支持自定义栈空间大小
 * 
 * 高级特性：
 * - Hook机制：支持系统调用的Hook功能
 * - 结果传递：协程间的数据传递机制
 * - 状态跟踪：实时跟踪协程的执行状态
 * 
 * @note 这是协程系统的核心数据结构，通常不直接使用
 * @note 协程栈大小的选择影响内存使用和嵌套调用深度
 * @warning 协程栈溢出可能导致程序崩溃，需要合理设置栈大小
 */
class NFCoroutine {
public:
	/**
	 * @brief 构造函数
	 * @param stackSize 协程栈大小（字节）
	 * 
	 * 初始化协程对象，分配执行栈空间并设置初始状态。
	 */
	NFCoroutine(uint32_t stackSize) {
		func = NULL;
		ud = NULL;
		sch = NULL;
		status = NF_COROUTINE_DEAD;
		enable_hook = false;
		stack = new char[stackSize];
		result = 0;
#if NF_PLATFORM == NF_PLATFORM_LINUX
		memset(&ctx, 0, sizeof(ucontext_t));
#else
		ctx = NULL;
#endif
        userData = NULL;
	}

	/**
	 * @brief 析构函数
	 * 
	 * 释放协程栈内存和相关资源。
	 */
	virtual ~NFCoroutine()
	{
		delete [] stack;
	}

public:
	/**
	 * @brief 协程执行函数指针
	 * 
	 * 指向协程要执行的C风格函数，使用NFCoroutineFunc类型。
	 */
	NFCoroutineFunc func;
	
	/**
	 * @brief C++风格协程执行函数
	 * 
	 * 协程要执行的std::function对象，支持lambda表达式和函数对象。
	 */
	std::function<void()> std_func;
	
	/**
	 * @brief 用户数据指针
	 * 
	 * 传递给协程函数的自定义数据，可以是任意类型的指针。
	 */
	void *ud;

#if NF_PLATFORM == NF_PLATFORM_LINUX
	/**
	 * @brief 协程上下文（Linux平台）
	 * 
	 * 保存协程的CPU寄存器状态和栈指针，用于上下文切换。
	 */
	ucontext_t ctx;
#else
	/**
	 * @brief 协程Fiber句柄（Windows平台）
	 * 
	 * Windows平台下的Fiber句柄，用于协程切换。
	 */
	void* ctx;
#endif

	/**
	 * @brief 所属调度器指针
	 * 
	 * 指向管理此协程的调度器实例。
	 */
	NFSchedule * sch;
	
	/**
	 * @brief 协程状态
	 * 
	 * 当前协程的运行状态，可能的值：
	 * - NF_COROUTINE_DEAD: 协程已死亡
	 * - NF_COROUTINE_READY: 协程就绪，等待执行
	 * - NF_COROUTINE_RUNNING: 协程正在运行
	 * - NF_COROUTINE_SUSPEND: 协程已挂起
	 */
	int status;
	
	/**
	 * @brief Hook机制启用标志
	 * 
	 * 是否启用系统调用的Hook功能，用于在协程中进行非阻塞的系统调用。
	 */
	bool enable_hook;
	
	/**
	 * @brief 协程栈指针
	 * 
	 * 指向协程独立的执行栈空间，栈大小在构造时指定。
	 */
	char* stack;
	
	/**
	 * @brief Resume结果
	 * 
	 * 协程被Resume时携带的结果值，用于协程间的数据传递。
	 */
	int32_t result;
	
	/**
	 * @brief 用户数据
	 * 
	 * 协程可携带的protobuf消息数据，用于结构化的数据传递。
	 */
    google::protobuf::Message *userData;
};
