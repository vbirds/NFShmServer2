// -------------------------------------------------------------------------
//    @FileName         :    NFServerStatus.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerStatus
//
// -------------------------------------------------------------------------

/**
 * @file NFServerStatus.h
 * @brief 服务器状态管理类
 * 
 * 此文件提供了服务器生命周期状态的管理功能，使用原子操作确保线程安全。
 * 主要用于跟踪服务器的启动、运行、停止等状态变化。
 */

#pragma once

#include <atomic>
#include <string>
#include "NFComm/NFCore/NFPlatform.h"

/**
 * @brief 服务器状态管理类
 * 
 * NFServerStatus提供了服务器生命周期状态的管理和查询功能。
 * 使用原子操作确保多线程环境下的状态一致性。
 * 
 * 主要特性：
 * - 线程安全：使用std::atomic确保状态读写的原子性
 * - 状态机：提供完整的服务器生命周期状态转换
 * - 查询接口：提供便捷的状态查询方法
 * - 子状态：支持细粒度的子状态管理
 * - 无锁设计：基于原子操作，避免锁开销
 * 
 * 状态流转：
 * kNull -> kInitializing -> kInitialized -> kStarting -> kRunning -> kStopping -> kStopped
 * 
 * 适用场景：
 * - 服务器启动流程控制
 * - 优雅关闭实现
 * - 多线程状态同步
 * - 服务健康检查
 * - 监控和诊断
 * 
 * 使用方法：
 * @code
 * class MyServer : public NFServerStatus {
 * public:
 *     void Start() {
 *         m_playerStatus = kInitializing;
 *         // 初始化逻辑...
 *         
 *         m_playerStatus = kInitialized;
 *         // 准备启动...
 *         
 *         m_playerStatus = kStarting;
 *         // 启动服务...
 *         
 *         m_playerStatus = kRunning;
 *         std::cout << "服务器已启动" << std::endl;
 *     }
 *     
 *     void Stop() {
 *         if (IsRunning()) {
 *             m_playerStatus = kStopping;
 *             m_substatus = kStoppingListener;
 *             // 停止监听器...
 *             
 *             m_substatus = kStoppingThreadPool;
 *             // 停止线程池...
 *             
 *             m_playerStatus = kStopped;
 *             std::cout << "服务器已停止" << std::endl;
 *         }
 *     }
 *     
 *     void StatusCheck() {
 *         if (IsRunning()) {
 *             std::cout << "服务器正在运行" << std::endl;
 *         } else if (IsStopping()) {
 *             std::cout << "服务器正在停止" << std::endl;
 *         } else if (IsStopped()) {
 *             std::cout << "服务器已停止" << std::endl;
 *         }
 *     }
 * };
 * @endcode
 * 
 * @note 这是一个基类，通常被服务器类继承使用
 * @note 状态变更应该按照预定义的流程进行，避免无效转换
 */
class _NFExport NFServerStatus {
public:
	/**
	 * @brief 服务器主状态枚举
	 * 
	 * 定义了服务器生命周期的主要状态。
	 */
	enum Status {
		/** @brief 空状态，初始状态 */
		kNull = 0,
		/** @brief 正在初始化，加载配置和资源 */
		kInitializing = 1,
		/** @brief 已初始化完成，准备启动 */
		kInitialized = 2,
		/** @brief 正在启动，开启各种服务 */
		kStarting = 3,
		/** @brief 正在运行，提供正常服务 */
		kRunning = 4,
		/** @brief 正在停止，准备关闭服务 */
		kStopping = 5,
		/** @brief 已停止，所有服务已关闭 */
		kStopped = 6,
	};

	/**
	 * @brief 服务器子状态枚举
	 * 
	 * 提供更细粒度的状态信息，特别是在停止过程中的详细状态。
	 */
	enum SubStatus {
		/** @brief 空子状态 */
		kSubStatusNull = 0,
		/** @brief 正在停止监听器 */
		kStoppingListener = 1,
		/** @brief 正在停止线程池 */
		kStoppingThreadPool = 2,
	};

	/**
	 * @brief 检查服务器是否正在运行
	 * 
	 * 判断服务器是否处于运行状态，可以正常提供服务。
	 * 
	 * @return bool 服务器正在运行时返回true，否则返回false
	 * 
	 * @note 这是线程安全的原子操作
	 * @note 只有当状态为kRunning时才返回true
	 */
	bool IsRunning() const {
		return m_playerStatus.load() == kRunning;
	}

	/**
	 * @brief 检查服务器是否已停止
	 * 
	 * 判断服务器是否已完全停止，所有服务都已关闭。
	 * 
	 * @return bool 服务器已停止时返回true，否则返回false
	 * 
	 * @note 这是线程安全的原子操作
	 * @note 只有当状态为kStopped时才返回true
	 */
	bool IsStopped() const {
		return m_playerStatus.load() == kStopped;
	}

	/**
	 * @brief 检查服务器是否正在停止
	 * 
	 * 判断服务器是否正在执行停止流程，但尚未完全停止。
	 * 
	 * @return bool 服务器正在停止时返回true，否则返回false
	 * 
	 * @note 这是线程安全的原子操作
	 * @note 只有当状态为kStopping时才返回true
	 * @note 可以配合子状态获取更详细的停止进度
	 */
	bool IsStopping() const {
		return m_playerStatus.load() == kStopping;
	}

protected:
	/** @brief 服务器主状态，使用原子操作确保线程安全 */
	std::atomic<Status> m_playerStatus = { kNull };
	/** @brief 服务器子状态，提供更细粒度的状态信息 */
	std::atomic<SubStatus> m_substatus = { kSubStatusNull };
};
