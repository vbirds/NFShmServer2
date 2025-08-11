// -------------------------------------------------------------------------
//    @FileName         :    NFShmGlobalId.h
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFPluginModule
//    @Desc             :    共享内存全局ID管理器头文件，提供全局唯一ID的分配和回收功能。
//                          该文件定义了共享内存全局ID管理器类，提供全局ID的分配和回收、
//                          ID与对象的映射管理、轮次管理和文件持久化、统计信息收集、
//                          版本兼容性检查。主要功能包括全局ID分配和释放、对象ID映射表管理、
//                          轮次统计和文件更新、性能监控和统计、内存池管理。
//                          设计特点包括基于共享内存支持跨进程、队列管理高效分配、
//                          轮次机制防止ID溢出、文件持久化支持恢复、统计信息收集
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFShmStl/NFShmQueue.h>
#include <NFComm/NFShmStl/NFShmString.h>

#include "NFComm/NFObjCommon/NFObjPtr.h"
#include "NFComm/NFObjCommon/NFTypeDefines.h"

// 2的指数
#ifndef MAX_GLOBALID_NUM
#define MAX_GLOBALID_NUM 1048576
#endif

#define MAX_GLOBALID_NUM_MASK (MAX_GLOBALID_NUM-1)
#define MAX_GLOBALID_QUEUE_SIZE (MAX_GLOBALID_NUM+1)  //MAX_GLOBALID_NUM 的数值+1


#define BUFFSIZE 1024
#define GLOBALID_LOOP_BACK 2000

class NFObject;
class NFShmObjSeg;

/**
 * @brief 共享内存ID索引结构体
 * 
 * 用于存储全局ID与对象的映射关系
 */
struct NFShmIdIndex
{
	int m_iId;                           ///< 全局ID
	int m_iIndex;                        ///< 对象索引
	int m_iType;                         ///< 对象类型
	NFRawShmPtr<NFObject> m_pObjPtr;    ///< 对象指针
};

/**
 * @brief 共享内存全局ID管理器类
 * 
 * 负责管理共享内存中的全局唯一ID，包括：
 * - 全局ID的分配和回收
 * - ID与对象的映射管理
 * - 轮次管理和文件持久化
 * - 统计信息收集
 * - 版本兼容性检查
 * 
 * 主要功能：
 * - 全局ID分配和释放
 * - 对象ID映射表管理
 * - 轮次统计和文件更新
 * - 性能监控和统计
 * - 内存池管理
 * 
 * 设计特点：
 * - 基于共享内存，支持跨进程
 * - 队列管理，高效分配
 * - 轮次机制，防止ID溢出
 * - 文件持久化，支持恢复
 * - 统计信息收集
 */
class NFShmGlobalId final : public NFObject
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化全局ID管理器，设置文件名和基本参数
     * 根据共享内存模式选择初始化方式
     */
	NFShmGlobalId();
	
	/**
	 * @brief 析构函数
	 * 
	 * 清理全局ID管理器资源
	 */
	~NFShmGlobalId() override;

	/**
	 * @brief 创建初始化
	 * 
	 * 在创建时进行初始化，设置所有成员变量为默认值
	 * 
	 * @return 0 成功，其他值表示失败
	 */
	int CreateInit();
	
	/**
	 * @brief 恢复初始化
	 * 
	 * 在恢复时进行初始化
	 * 
	 * @return 0 成功，其他值表示失败
	 */
	int ResumeInit();

	/**
	 * @brief 获取全局ID
	 * 
	 * 为指定对象分配全局唯一ID
	 * 
	 * @param iType 对象类型
	 * @param iIndex 对象索引
	 * @param pObj 对象指针
	 * @return 分配的全局ID，失败返回-1
	 */
	int GetGlobalId(int iType, int iIndex, const NFObject* pObj);
	
	/**
	 * @brief 释放ID
	 * 
	 * 释放指定的全局ID
	 * 
	 * @param iId 全局ID
	 * @return 0 成功，其他值表示失败
	 */
	int ReleaseId(int iId);
	
	/**
	 * @brief 获取对象
	 * 
	 * 根据全局ID获取对象指针
	 * 
	 * @param iId 全局ID
	 * @return 对象指针，失败返回nullptr
	 */
	NFObject* GetObj(int iId);

public:
    /**
     * @brief 注册类到对象段
     * 
     * 静态方法，注册对象类型到对象段管理器
     * 
     * @param bType 对象类型
     * @param iObjSize 对象大小
     * @param iObjCount 对象数量
     * @param className 类名
     * @param useHash 是否使用哈希
     * @param singleton 是否单例
     * @return 注册结果
     */
	static int RegisterClassToObjSeg(int bType, int iObjSize, int iObjCount, const std::string& className, bool useHash, bool singleton);
	
	/**
	 * @brief 恢复对象
	 * 
	 * 从缓冲区恢复对象
	 * 
	 * @param pBuffer 缓冲区指针
	 * @return 恢复的对象指针
	 */
	static NFObject* ResumeObject(void* pBuffer);
	
	/**
	 * @brief 创建对象
	 * 
	 * 创建新的全局ID管理器对象
	 * 
	 * @return 创建的对象指针
	 */
	static NFObject* CreateObject();
	
	/**
	 * @brief 销毁对象
	 * 
	 * 销毁指定的对象
	 * 
	 * @param pObj 对象指针
	 */
	static void DestroyObject(NFObject* pObj);
	
	/**
	 * @brief 获取静态类名
	 * 
	 * @return 类名字符串
	 */
	static std::string GetStaticClassName() { return "NFShmGlobalId"; }
	
	/**
	 * @brief 获取静态类类型
	 * 
	 * @return 类类型
	 */
	static int GetStaticClassType() { return EOT_GLOBAL_ID; }
	
	/**
	 * @brief 重载new操作符
	 * 
	 * 在指定缓冲区上创建对象
	 * 
	 * @param nSize 对象大小
	 * @param pBuffer 缓冲区指针
	 * @return 对象指针
	 */
	void* operator new(size_t nSize, void* pBuffer) throw();
	
public:
    /**
     * @brief 增加秒数
     * 
     * 增加时间偏移量
     * 
     * @param iSecond 增加的秒数
     * @return 操作结果
     */
	int AddSecond(int iSecond);
	
	/**
	 * @brief 设置秒偏移量
	 * 
	 * @param iSecOffSet 秒偏移量
	 * @return 操作结果
	 */
	int SetSecOffSet(int iSecOffSet);
	
	/**
	 * @brief 获取秒偏移量
	 * 
	 * @return 秒偏移量
	 */
	int GetSecOffSet() const;
	
	/**
	 * @brief 获取使用计数
	 * 
	 * @return 使用计数
	 */
	int GetUseCount() const;

	/**
	 * @brief 获取操作统计信息
	 * 
	 * @return 统计信息字符串
	 */
	std::string GetOperatingStatistic() const;
	
	/**
	 * @brief 设置运行时文件ID
	 * 
	 * @param iId 文件ID
	 */
	void SetRunTimeFileId(int iId);
	
private:
    /**
     * @brief 恢复文件更新数据
     * 
     * @return 是否成功
     */
	bool ResumeFileUpdateData();
	
	/**
	 * @brief 计算轮次更新文件
	 * 
	 * @return 是否成功
	 */
	bool CalcRoundUpdateFile();
	
	/**
	 * @brief 写入轮次信息
	 * 
	 * @return 是否成功
	 */
	bool WriteRound() const;

protected:
	int m_iThisRoundCountMax;                                    ///< 当前轮次最大计数
	int m_iUseCount;                                             ///< 使用计数
	int m_iThisRoundCount;                                       ///< 当前轮次计数
	int m_iRoundMultiple;                                        ///< 轮次倍数
	int m_iGlobalIdAppendNum;                                    ///< 全局ID追加数量
	NFShmString<32> m_szFileName;                               ///< 文件名
	NFShmQueue<int,MAX_GLOBALID_QUEUE_SIZE> m_stQueue;          ///< ID队列
	NFShmIdIndex m_stIdTable[MAX_GLOBALID_NUM];                 ///< ID索引表
	time_t m_tRoundBegin;                                        ///< 轮次开始时间
	int m_iSecOffSet;                                            ///< 秒偏移量
};

