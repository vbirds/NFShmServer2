// -------------------------------------------------------------------------
//    @FileName         :    NFIAsyModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFIAsyModule
//    @Description      :    异步模块接口定义，提供Actor模式的异步任务处理框架
//                           支持任务组管理、负载均衡和异步数据库操作
//
//
//                    .::::.
//                  .::::::::.
//                 :::::::::::  FUCK YOU
//             ..:::::::::::'
//           '::::::::::::'
//             .::::::::::
//        '::::::::::::::..
//             ..::::::::::::.
//           ``::::::::::::::::
//            ::::``:::::::::'        .:::.
//           ::::'   ':::::'       .::::::::.
//         .::::'      ::::     .:::::::'::::.
//        .:::'       :::::  .:::::::::' ':::::.
//       .::'        :::::.:::::::::'      ':::::.
//      .::'         ::::::::::::::'         ``::::.
//  ...:::           ::::::::::::'              ``::.
// ```` ':.          ':::::::::'                  ::::..
//                    '.:::::'                    ':'````..
//
// -------------------------------------------------------------------------


#pragma once

#include "NFIModule.h"
#include <vector>
#include "NFITaskModule.h"
#include "NFTask.h"

#include "google/protobuf/message.h"
#include "NFComm/NFKernelMessage/FrameSqlData.pb.h"
#include "NFIConfigModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/// @brief 条件查询回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 查询结果数据
using SelectByCondCb = std::function<void(int iRet, NFrame::storesvr_sel_res& stSelectRes)>;

/// @brief 对象查询回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 查询结果数据
using SelectObjCb = std::function<void(int iRet, NFrame::storesvr_selobj_res& stSelectRes)>;

/// @brief 条件删除回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 删除结果数据
using DeleteByCondCb = std::function<void(int iRet, NFrame::storesvr_del_res& stSelectRes)>;

/// @brief 对象删除回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 删除结果数据
using DeleteObjCb = std::function<void(int iRet, NFrame::storesvr_delobj_res& stSelectRes)>;

/// @brief 对象插入回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 插入结果数据
using InsertObjCb = std::function<void(int iRet, NFrame::storesvr_insertobj_res& stSelectRes)>;

/// @brief 条件修改回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 修改结果数据
using ModifyByCondCb = std::function<void(int iRet, NFrame::storesvr_mod_res& stSelectRes)>;

/// @brief 对象修改回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 修改结果数据
using ModifyObjCb = std::function<void(int iRet, NFrame::storesvr_modobj_res& stSelectRes)>;

/// @brief 条件更新回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 更新结果数据
using UpdateByCondCb = std::function<void(int iRet, NFrame::storesvr_update_res& stSelectRes)>;

/// @brief 对象更新回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 更新结果数据
using UpdateObjCb = std::function<void(int iRet, NFrame::storesvr_updateobj_res& stSelectRes)>;

/// @brief SQL执行回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 执行结果数据
using ExecuteCb = std::function<void(int iRet, NFrame::storesvr_execute_res& stSelectRes)>;

/// @brief 批量SQL执行回调函数类型
/// @param iRet 操作结果，0表示成功，非0表示失败
/// @param stSelectRes 执行结果数据
using ExecuteMoreCb = std::function<void(int iRet, NFrame::storesvr_execute_more_res& stSelectRes)>;

/**
 * @brief 异步模块接口类
 * 
 * NFIAsyModule 提供了基于Actor模式的异步任务处理框架：
 * 
 * 1. Actor池管理：
 *    - 多任务组的Actor池初始化
 *    - Actor的创建、分配和管理
 *    - 线程池与Actor的映射关系
 * 
 * 2. 负载均衡机制：
 *    - 基于平衡ID的负载分发
 *    - 随机Actor选择算法
 *    - 避免数据竞争的任务分配
 * 
 * 3. 异步任务处理：
 *    - 任务的异步分发和执行
 *    - 数据库操作的异步化
 *    - 回调机制的支持
 * 
 * 4. 组件管理：
 *    - Actor组件的添加和获取
 *    - 组件生命周期管理
 * 
 * 设计原理：
 * - 采用Actor模式避免锁竞争
 * - 通过负载均衡确保数据一致性
 * - 支持水平扩展的任务处理
 * - 提供异步编程模型简化开发
 * 
 * 使用场景：
 * - 异步数据库操作
 * - 高并发任务处理
 * - 消息队列处理
 * - I/O密集型操作
 * 
 * 关键特性：
 * - 无锁并发处理
 * - 数据隔离保证
 * - 自动负载均衡
 * - 错误恢复机制
 */
class NFIAsyModule : public NFIModule {
public:
	/**
	 * @brief 构造函数
	 * @param pPluginManager 插件管理器指针
	 * 
	 * 初始化异步模块，设置Actor池的初始状态。
	 */
	explicit NFIAsyModule(NFIPluginManager* pPluginManager): NFIModule(pPluginManager)
	{
		m_bInitActor = false;
	}

	/**
	 * @brief 析构函数
	 * 
	 * 清理异步模块资源，释放Actor池。
	 */
	~NFIAsyModule() override = default;

	/**
	 * @brief 初始化Actor池
	 * @param iMaxTaskGroup 最大任务组数量，用于限制任务组的并发数量
	 * @param iMaxActorNum 每个任务组中的最大Actor数量，用于限制Actor池中Actor的并发数量
	 * @return 初始化成功返回true，失败返回false
	 * 
	 * 该函数用于初始化Actor池，设置任务组和Actor的最大数量。
	 * 通过该函数，可以确保Actor池在后续操作中能够正确处理任务和Actor的分配。
	 * 
	 * 初始化过程：
	 * 1. 根据配置确定工作线程数量
	 * 2. 初始化任务模块的Actor线程
	 * 3. 为每个任务组创建指定数量的Actor
	 * 4. 建立Actor池的管理结构
	 * 
	 * 使用示例：
	 * @code
	 * // 初始化4个任务组，每组8个Actor
	 * bool success = InitActorPool(4, 8);
	 * if (success) {
	 *     // Actor池初始化成功，可以开始分配任务
	 * }
	 * @endcode
	 * 
	 * @note 此函数只能调用一次，重复调用将被忽略
	 * @note 如果iMaxActorNum <= 0，将自动设置为线程数的2倍
	 */
	virtual bool InitActorPool(int iMaxTaskGroup, int iMaxActorNum)
	{
		if (!m_bInitActor)
		{
			int iMaxThread = 1;
			/*
			启动多线程任务系统
			*/
			if (m_pObjPluginManager->IsLoadAllServer())
			{
				iMaxThread = 1;
			} else {
				NFServerConfig *pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_NONE);
				NF_ASSERT(pConfig);

				iMaxThread = (int)pConfig->WorkThreadNum;
			}

			FindModule<NFITaskModule>()->InitActorThread(iMaxTaskGroup, iMaxThread);

			if (iMaxActorNum <= 0)
			{
				iMaxActorNum = iMaxThread * 2;
			}

			m_stVecActorGroupPool.resize(iMaxTaskGroup);

			for (int iTaskGroup = 0; iTaskGroup < (int)m_stVecActorGroupPool.size(); iTaskGroup++)
			{
				for (int i = 0; i < iMaxActorNum; i++)
				{
					int iActorId = FindModule<NFITaskModule>()->RequireActor(iTaskGroup);
					if (iActorId <= 0) {
						return false;
					}

					m_stVecActorGroupPool[iTaskGroup].push_back(iActorId);
				}
			}

			m_bInitActor = true;
		}
		return true;
	}

	/**
	 * @brief 检查指定任务组和Actor ID是否存在
	 * @param iTaskGroup 任务组的标识符，用于指定要检查的任务组
	 * @param iActorId Actor的标识符，用于指定要检查的Actor
	 * @return 如果任务组和Actor ID都存在返回true，否则返回false
	 * 
	 * 该函数用于验证给定的任务组和Actor ID是否存在于系统中。
	 * 通常用于任务分配前的有效性验证或Actor管理场景。
	 * 
	 * 使用示例：
	 * @code
	 * if (Exist(taskGroup, actorId)) {
	 *     // Actor存在，可以安全分配任务
	 *     AddTask(taskGroup, pTask);
	 * }
	 * @endcode
	 */
	virtual bool Exist(int iTaskGroup, int iActorId)
	{
		CHECK_EXPR(iTaskGroup >= 0 && iTaskGroup < (int)m_stVecActorGroupPool.size(), false, "taskGroup:{} error", iTaskGroup);
		for (size_t i = 0; i < m_stVecActorGroupPool[iTaskGroup].size(); i++)
		{
			if (m_stVecActorGroupPool[iTaskGroup][i] == iActorId) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief 为Actor添加组件
	 * @param iTaskGroup 任务组ID
	 * @param iActorId Actor ID
	 * @param pComponent 要添加的组件指针
	 * @return 添加成功返回0，失败返回非0值
	 * 
	 * 为指定的Actor添加功能组件，扩展Actor的处理能力。
	 * 组件可以包含特定的业务逻辑或数据处理功能。
	 */
	virtual int AddActorComponent(int iTaskGroup, int iActorId, NFITaskComponent* pComponent)
	{
		return FindModule<NFITaskModule>()->AddActorComponent(iTaskGroup, iActorId, pComponent);
	}

	/**
	 * @brief 获取Actor的组件
	 * @param iTaskGroup 任务组ID
	 * @param iActorId Actor ID
	 * @return 返回组件指针，未找到返回nullptr
	 * 
	 * 获取指定Actor关联的组件实例，用于访问组件提供的功能。
	 */
	virtual NFITaskComponent* GetActorComponent(int iTaskGroup, int iActorId)
	{
		return FindModule<NFITaskModule>()->GetTaskComponent(iTaskGroup, iActorId);
	}

	/**
	 * @brief 通过平衡ID获取负载均衡的Actor
	 * @param iTaskGroup 任务组ID
	 * @param ullBalanceId 动态平衡ID，用于负载均衡算法
	 * @return 返回选中的Actor ID，失败返回-1
	 * 
	 * 为了防止数据库操作冲突，使用动态平衡ID确保同一时刻
	 * 只有一个线程对数据库表中的同一条数据进行读取或写入。
	 * 
	 * 负载均衡策略：
	 * - 如果ullBalanceId为0，则随机选择Actor
	 * - 否则使用模运算确保相同ID总是分配到同一Actor
	 * 
	 * 使用示例：
	 * @code
	 * // 确保同一用户的操作总是在同一Actor中执行
	 * uint64_t userId = 12345;
	 * int actorId = GetBalanceActor(0, userId);
	 * if (actorId > 0) {
	 *     // 分配任务到该Actor
	 * }
	 * @endcode
	 */
	int GetBalanceActor(int iTaskGroup, uint64_t ullBalanceId)
	{
		CHECK_EXPR(iTaskGroup >= 0 && iTaskGroup < (int)m_stVecActorGroupPool.size(), -1, "taskGroup:{} error", iTaskGroup);
		if (ullBalanceId == 0)
		{
			return GetRandActor(iTaskGroup);
		}
		if (m_stVecActorGroupPool[iTaskGroup].empty())
		{
			return -1;
		}
		m_ullSuitIndex = ullBalanceId % m_stVecActorGroupPool[iTaskGroup].size();
		return m_stVecActorGroupPool[iTaskGroup][m_ullSuitIndex];
	}

	/**
	 * @brief 随机获取一个Actor
	 * @param iTaskGroup 任务组ID
	 * @return 返回随机选中的Actor ID，失败返回-1
	 * 
	 * 使用轮询算法随机选择一个可用的Actor，用于没有特定
	 * 平衡要求的任务分配。
	 * 
	 * 算法特点：
	 * - 轮询分配，确保负载均匀
	 * - 无状态依赖，适合一般性任务
	 * - 高效的选择算法
	 */
	int GetRandActor(int iTaskGroup)
	{
		CHECK_EXPR(iTaskGroup >= 0 && iTaskGroup < (int)m_stVecActorGroupPool.size(), -1, "taskGroup:{} error", iTaskGroup);
		if (m_stVecActorGroupPool[iTaskGroup].empty())
		{
			return -1;
		}

		m_ullSuitIndex++;
		m_ullSuitIndex = m_ullSuitIndex % m_stVecActorGroupPool[iTaskGroup].size();

		return m_stVecActorGroupPool[iTaskGroup][m_ullSuitIndex];
	}

	/**
	 * @brief 添加异步任务到指定任务组
	 * @param iTaskGroup 任务组ID
	 * @param pTask 要异步处理的任务对象指针
	 * @return 添加成功返回正值，失败返回0或负值
	 * 
	 * 通过平衡ID将任务分配到合适的Actor进行异步处理。
	 * 系统会根据任务的平衡ID自动选择最优的Actor，
	 * 确保相关任务的执行顺序和数据一致性。
	 * 
	 * 分配策略：
	 * 1. 根据任务的平衡ID选择目标Actor
	 * 2. 将任务添加到该Actor的处理队列
	 * 3. 任务将在该Actor的线程中异步执行
	 * 
	 * 使用示例：
	 * @code
	 * // 创建数据库操作任务
	 * auto pTask = new DatabaseTask(userId, operation);
	 * int result = AddTask(DB_TASK_GROUP, pTask);
	 * if (result > 0) {
	 *     // 任务已成功添加到处理队列
	 * }
	 * @endcode
	 */
	int AddTask(int iTaskGroup, NFTask* pTask)
	{
		if (pTask)
		{
			int iActorId = GetBalanceActor(iTaskGroup, pTask->GetBalanceId());
			if (iActorId > 0)
			{
				return FindModule<NFITaskModule>()->AddTask(iTaskGroup, iActorId, pTask);
			}
		}

		return 0;
	}

protected:
	/// @brief Actor索引数组，按任务组组织的Actor池结构
	/// 第一维是任务组，第二维是该组内的Actor ID列表
	std::vector<std::vector<int> > m_stVecActorGroupPool;

	/// @brief 用于负载均衡的索引计数器，实现轮询分配算法
	size_t m_ullSuitIndex = 0;

	/// @brief Actor池初始化标志，防止重复初始化
	bool m_bInitActor;
};
