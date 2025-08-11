// -------------------------------------------------------------------------
//    @FileName         :    NFObjSeg.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    共享内存对象段头文件，提供对象段的内存管理和索引功能。
//                          该文件定义了共享内存对象段类，提供对象段的内存分配和管理、
//                          对象索引的创建和维护、哈希表的管理和查找、对象的创建和销毁、
//                          内存池的管理。主要功能包括对象段初始化、内存分配和释放、
//                          索引管理、哈希表操作、对象生命周期管理。
//                          设计特点包括基于共享内存支持跨进程、高效的内存管理、
//                          灵活的索引机制、可选的哈希表支持、自动内存回收
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFShmStl/NFShmDyList.h"
#include "NFComm/NFShmStl/NFShmDyVector.h"
#include "NFComm/NFShmStl/NFShmDyHashMapWithList.h"
#include "NFShmIdx.h"

class NFObject;
class NFShmObjSeg;
class NFCShmMngModule;

/**
    vec  m_createUseTime:9304 ms m_deleteUseTime:3557 ms m_getUseTime:38 ms
    vec m_createUseTime:9457 ms m_deleteUseTime:3496 ms m_getUseTime:35 ms
    list m_createUseTime:9627 ms m_deleteUseTime:3483 ms m_getUseTime:32 ms
    list m_createUseTime:9441 ms m_deleteUseTime:3449 ms m_getUseTime:31 ms
    分别测试了vector索引和list索引在10万次CreateObj,DestroyObj,GetObjByObjId下的效率，发现他们的效率几乎差不多,
    但在随机删除轮询tick的情况下，vector更加的平均，所以优先选择最新的vector索引方案
 */
#define SHM_OBJ_SEQ_USE_VECTOR_INDEX

/**
 * @brief 共享内存对象段类
 * 
 * 负责管理共享内存中的对象段，包括：
 * - 对象段的内存分配和管理
 * - 对象索引的创建和维护
 * - 哈希表的管理和查找
 * - 对象的创建和销毁
 * - 内存池的管理
 * 
 * 主要功能：
 * - 对象段初始化
 * - 内存分配和释放
 * - 索引管理
 * - 哈希表操作
 * - 对象生命周期管理
 * 
 * 设计特点：
 * - 基于共享内存，支持跨进程
 * - 高效的内存管理
 * - 灵活的索引机制
 * - 可选的哈希表支持
 * - 自动内存回收
 */
class NFShmObjSeg
{
    friend class NFCShmMngModule;
public:
    /**
     * @brief 重载new操作符
     * 
     * 在指定缓冲区上创建对象段
     * 
     * @param nSize 对象大小
     * @param pBuffer 缓冲区指针
     * @return 对象指针
     */
    void *operator new(size_t nSize, void *pBuffer) throw();

    /**
     * @brief 创建对象段
     * 
     * 在共享内存中创建对象段
     * 
     * @param pShmModule 共享内存管理模块指针
     * @return 创建的对象段指针
     */
    static NFShmObjSeg* CreateObjSeg(NFCShmMngModule* pShmModule);

    /**
     * @brief 重载delete操作符
     * 
     * 释放对象段内存
     * 
     * @param pMem 内存指针
     * @param ptr 缓冲区指针
     */
    void operator delete(void* pMem, void* ptr);
public:
    /**
     * @brief 查找模块
     * 
     * 模板函数，用于查找指定类型的模块
     * 
     * @tparam T 模块类型
     * @return 模块指针
     */
    template <typename T>
    T* FindModule() const
    {
        return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<T>();
    }
public:
    /**
     * @brief 格式化对象区
     * 
     * 初始化对象区域的内存布局
     * 
     * @return 格式化结果
     */
    int FormatObj();

    /**
     * @brief 为对象分配内存
     * 
     * 从对象段中分配一块内存给对象使用
     * 
     * @return 分配的内存指针
     */
    void *AllocMemForObject();

    /**
     * @brief 释放对象内存
     * 
     * 释放对象占用的内存
     * 
     * @param pMem 内存指针
     * @return 释放结果
     */
    int FreeMemForObject(void *pMem);

    /**
     * @brief 创建对象
     * 
     * 在对象段中创建一个新对象
     * 
     * @return 创建结果
     */
    int CreateObject();
public:
    /**
     * @brief 哈希表插入
     * 
     * 向哈希表中插入键值对
     * 
     * @param key 哈希键
     * @param objId 对象ID
     * @return 插入结果
     */
    int HashInsert(NFObjectHashKey key, int objId);

    /**
     * @brief 哈希表查找
     * 
     * 根据哈希键查找对象
     * 
     * @param key 哈希键
     * @return 找到的对象指针
     */
    NFObject *HashFind(NFObjectHashKey key);

    /**
     * @brief 哈希表删除
     * 
     * 从哈希表中删除指定键值对
     * 
     * @param key 哈希键
     * @return 删除结果
     */
    int HashErase(NFObjectHashKey key);
public:
    /**
     * @brief 获取对象
     * 
     * 根据索引获取对象
     * 
     * @param iIdx 对象索引
     * @return 对象指针
     */
    NFObject *GetObj(int iIdx);

    /**
     * @brief 获取索引
     * 
     * 根据索引获取索引对象
     * 
     * @param iIdx 索引值
     * @return 索引对象指针
     */
    NFShmIdx *GetIdx(int iIdx);

    /**
     * @brief 获取对象ID
     * 
     * 根据对象指针获取对象ID
     * 
     * @param pstObj 对象指针
     * @return 对象ID
     */
    int GetObjId(void *pstObj) const;
public:
    /**
     * @brief 获取头对象
     * 
     * 获取对象段的第一个对象
     * 
     * @return 头对象指针
     */
    NFObject *GetHeadObj();

    /**
     * @brief 获取下一个对象
     * 
     * 获取指定对象的下一个对象
     * 
     * @param pObj 当前对象指针
     * @return 下一个对象指针
     */
    NFObject *GetNextObj(NFObject *pObj);

    /**
     * @brief 检查是否结束
     * 
     * 检查指定索引是否到达末尾
     * 
     * @param iIndex 索引值
     * @return 是否结束
     */
    bool IsEnd(int iIndex);
public:
    /**
     * @brief 获取项目总数
     * 
     * @return 项目总数
     */
    int GetItemCount() const { return m_iItemCount; }
    
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
    /**
     * @brief 获取已使用数量（向量索引）
     * 
     * @return 已使用数量
     */
    int GetUsedCount() const { return m_idxVec.size(); }
#else
    /**
     * @brief 获取已使用数量（链表索引）
     * 
     * @return 已使用数量
     */
    int GetUsedCount() const { return m_idxLst.size(); }
#endif

    /**
     * @brief 获取空闲数量
     * 
     * @return 空闲数量
     */
    int GetFreeCount() const { return m_iItemCount - GetUsedCount(); }

    /**
     * @brief 获取对象大小
     * 
     * @return 对象大小
     */
    int GetObjSize() const { return m_nObjSize; }

    /**
     * @brief 获取内存大小
     * 
     * @return 内存大小
     */
    int GetMemSize() const { return m_nMemSize; }

    /**
     * @brief 获取总大小
     * 
     * @return 总大小
     */
    int GetAllSize() const { return m_nMemSize + sizeof(NFShmObjSeg); }

    /**
     * @brief 获取哈希表大小
     * 
     * @param itemCount 项目数量
     * @return 哈希表大小
     */
    static int GetHashSize(int itemCount) { return NFShmDyHashMapWithList<uint64_t, int>::CountSize(itemCount); }
public:
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
    /**
     * @brief ShmObj类链表迭代器+1
     * @param iPos
     * @return
     */
    virtual size_t IterIncr(size_t iPos)
    {
        ++iPos;
        return iPos;
    }

    /**
     * @brief ShmObj类链表迭代器-1
     * @param iPos
     * @return
     */
    virtual size_t IterDecr(size_t iPos)
    {
        --iPos;
        return iPos;
    }

    virtual size_t IterBegin()
    {
        return 0;
    }

    virtual size_t IterEnd()
    {
        return m_idxVec.size();
    }

    virtual size_t IterNext(size_t iPos)
    {
        ++iPos;
        return iPos;
    }

    virtual NFObject* GetIterObj(size_t iPos)
    {
        if (iPos < m_idxVec.size())
        {
            return m_idxVec[iPos].GetAttachedObj();
        }
        return nullptr;
    }

    virtual const NFObject* GetIterObj(size_t iPos) const
    {
        if (iPos < m_idxVec.size())
        {
            return m_idxVec[iPos].GetAttachedObj();
        }
        return nullptr;
    }
#else
    /**
     * @brief ShmObj类链表迭代器+1
     * @param iPos
     * @return
     */
    virtual size_t IterIncr(size_t iPos)
    {
        auto iter = m_idxLst.GetIterator(iPos);
        ++iter;
        return iter.m_node->m_self;
    }

    /**
     * @brief ShmObj类链表迭代器-1
     * @param iPos
     * @return
     */
    virtual size_t IterDecr(size_t iPos)
    {
        auto iter = m_idxLst.GetIterator(iPos);
        --iter;
        return iter.m_node->m_self;
    }

    virtual size_t IterBegin()
    {
        auto iter = m_idxLst.begin();
        return iter.m_node->m_self;
    }

    virtual size_t IterEnd()
    {
        auto iter = m_idxLst.end();
        return iter.m_node->m_self;
    }

    virtual size_t IterNext(size_t iPos)
    {
        auto iter = m_idxLst.GetIterator(iPos);
        ++iter;
        return iter.m_node->m_self;
    }

    virtual NFObject* GetIterObj(size_t iPos)
    {
        auto iter = m_idxLst.GetIterator(iPos);
        return iter->GetAttachedObj();
    }

    virtual const NFObject* GetIterObj(size_t iPos) const
    {
        auto iter = m_idxLst.GetIterator(iPos);
        return iter->GetAttachedObj();
    }
#endif

protected:
    NFShmIdx *CreateIdx();

    int DestroyIdx(int iIdx);
private:
    NFShmObjSeg();

    ~NFShmObjSeg();

    int SetAndInitObj(NFCShmMngModule* pShmModule, size_t nObjSize, int iItemCount, NFObject*(*pfCreateObj)(void*), bool iUseHash = false);

    size_t m_nObjSize;
    size_t m_nMemSize;
#ifdef SHM_OBJ_SEQ_USE_VECTOR_INDEX
    NFShmDyVector<NFShmIdx> m_idxVec;
    NFShmDyList<int> m_idxQueue;
#else
    NFShmDyList<NFShmIdx> m_idxLst;
#endif
    char* m_pObjs;
    NFShmDyHashMapWithList<NFObjectHashKey, int> m_hashMgr;
    int m_iItemCount;
    bool m_iUseHash;

    NFObject *(*m_pCreateFn)(void *);
    NFCShmMngModule* m_pShmModule;
};


