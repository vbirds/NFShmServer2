// -------------------------------------------------------------------------
//    @FileName         :    NFShmIdx.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    共享内存索引头文件，提供对象索引的管理功能。
//                          该文件定义了共享内存索引类，提供索引对象的创建和初始化、
//                          索引与对象的关联管理、索引状态的管理、内存缓冲区的管理。
//                          主要功能包括索引初始化、对象关联管理、缓冲区管理、状态维护。
//                          设计特点包括基于共享内存支持跨进程、高效的对象关联、
//                          灵活的内存管理、状态一致性保证
//
// -------------------------------------------------------------------------
#pragma once

#define MAXDSINFOITEM  2
#define IDXNULL			-1

class NFObject;

/**
 * @brief 共享内存索引类
 * 
 * 负责管理共享内存中的对象索引，包括：
 * - 索引对象的创建和初始化
 * - 索引与对象的关联管理
 * - 索引状态的管理
 * - 内存缓冲区的管理
 * 
 * 主要功能：
 * - 索引初始化
 * - 对象关联管理
 * - 缓冲区管理
 * - 状态维护
 * 
 * 设计特点：
 * - 基于共享内存，支持跨进程
 * - 高效的对象关联
 * - 灵活的内存管理
 * - 状态一致性保证
 */
class NFShmIdx
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化索引对象，根据共享内存模式选择初始化方式
     */
	NFShmIdx();
	
	/**
	 * @brief 析构函数
	 * 
	 * 清理索引对象资源，重新初始化
	 */
	~NFShmIdx();
	
	/**
	 * @brief 创建初始化
	 * 
	 * 在创建时进行初始化
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
	 * @brief 初始化索引
	 * 
	 * 重置索引对象的所有成员变量为初始状态
	 */
	void Initialize();
	
	/**
	 * @brief 获取关联对象
	 * 
	 * 获取索引关联的对象指针
	 * 
	 * @return 关联的对象指针
	 */
	NFObject* GetAttachedObj();
	
	/**
	 * @brief 获取关联对象（常量版本）
	 * 
	 * 获取索引关联的对象指针（常量版本）
	 * 
	 * @return 关联的对象指针
	 */
	const NFObject* GetAttachedObj() const;
	
	/**
	 * @brief 设置关联对象
	 * 
	 * 设置索引关联的对象指针
	 * 
	 * @param pObj 对象指针
	 */
	void SetAttachedObj(NFObject* pObj);
	
	/**
	 * @brief 获取对象缓冲区
	 * 
	 * 获取索引关联的对象缓冲区指针
	 * 
	 * @return 缓冲区指针
	 */
	void* GetObjBuf() const;
	
	/**
	 * @brief 设置对象缓冲区
	 * 
	 * 设置索引关联的对象缓冲区指针
	 * 
	 * @param pBuf 缓冲区指针
	 */
	void SetObjBuf(void* pBuf);
	
	/**
	 * @brief 获取索引值
	 * 
	 * @return 索引值
	 */
	int GetIndex() const { return m_iIndex; }
	
	/**
	 * @brief 设置索引值
	 * 
	 * @param iIndex 索引值
	 */
	void SetIndex(int iIndex) { m_iIndex = iIndex; }

	/**
	 * @brief 赋值操作符
	 * 
	 * 复制另一个索引对象的内容
	 * 
	 * @param idx 源索引对象
	 * @return 当前对象的引用
	 */
	NFShmIdx& operator=(const NFShmIdx& idx)
	{
		if (this != &idx)
		{
			m_uEntity.m_pBuf = idx.m_uEntity.m_pBuf;
			m_iIndex = idx.m_iIndex;
		}

		return *this;
	}

private:
	/**
	 * @brief 实体联合体
	 * 
	 * 用于存储对象指针或缓冲区指针
	 */
	union
	{
		NFObject* m_pAttachedObj;  ///< 关联的对象指针
		void* m_pBuf;              ///< 缓冲区指针
	} m_uEntity;

	int m_iIndex;                 ///< 索引值
};
