// -------------------------------------------------------------------------
//    @FileName         :    NFObjSeg.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    内存对象段头文件，负责管理内存中的对象段，包括对象段的内存分配和管理、
//                          对象索引的创建和维护、哈希表的管理和查找、对象的创建和销毁、内存池的管理。
//                          该文件定义了NFShmXFrame框架的内存对象段类，提供对象段初始化、内存分配和释放、
//                          索引管理、哈希表操作、对象生命周期管理等功能。
//                          主要功能包括高效的内存管理、灵活的索引机制、可选的哈希表支持、自动内存回收
//
// -------------------------------------------------------------------------
#pragma once

#include <NFComm/NFPluginModule/NFChunkPool.h>

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFShmStl/NFShmDyList.h"
#include "NFComm/NFShmStl/NFShmDyVector.h"
#include "NFComm/NFShmStl/NFShmDyHashMapWithList.h"
#include "NFMemIdx.h"

class NFObject;
class NFCMemMngModule;
class NFMemObjSeg;

/**
 * @brief 内存对象段类，负责管理内存中的对象段
 * 
 * 该类负责管理内存中的对象段，包括对象段的内存分配和管理、
 * 对象索引的创建和维护、哈希表的管理和查找、对象的创建和销毁、内存池的管理。
 * 提供高效的内存管理、灵活的索引机制、可选的哈希表支持、自动内存回收等功能。
 */
class NFMemObjSeg
{
    friend class NFCMemMngModule;
public:
    /**
     * @brief 模板方法，用于查找指定类型的模块
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
     * @brief 为对象分配内存
     * @return 分配的内存指针，失败返回nullptr
     */
    void *AllocMemForObject();

    /**
     * @brief 释放对象内存
     * @param pMem 要释放的内存指针
     * @return 释放结果，0表示成功
     */
    int FreeMemForObject(void *pMem);
public:
    /**
     * @brief 向哈希表中插入对象
     * @param key 对象哈希键
     * @param objId 对象ID
     * @return 插入结果，0表示成功
     */
    int HashInsert(NFObjectHashKey key, int objId);

    /**
     * @brief 在哈希表中查找对象
     * @param key 对象哈希键
     * @return 找到的对象指针，未找到返回nullptr
     */
    NFObject* HashFind(NFObjectHashKey key);

    /**
     * @brief 从哈希表中删除对象
     * @param key 对象哈希键
     * @return 删除结果，0表示成功
     */
    int HashErase(NFObjectHashKey key);
public:
    /**
     * @brief 根据索引获取对象
     * @param iIdx 对象索引
     * @return 对象指针，失败返回nullptr
     */
    NFObject* GetObj(int iIdx);
    
    /**
     * @brief 根据对象指针获取对象ID
     * @param pstObj 对象指针
     * @return 对象ID
     */
    int GetObjId(void *pstObj) const;
public:
    /**
     * @brief 获取第一个对象
     * @return 第一个对象指针
     */
    NFObject* GetHeadObj();

    /**
     * @brief 获取下一个对象
     * @param pObj 当前对象指针
     * @return 下一个对象指针
     */
    NFObject* GetNextObj(NFObject* pObj);

    /**
     * @brief 判断是否到达末尾
     * @param iIndex 对象索引
     * @return 是否到达末尾
     */
    bool IsEnd(int iIndex);
public:
    /**
     * @brief 获取对象总数
     * @return 对象总数
     */
    int GetItemCount() const { return m_pObjectPool->GetAllCount(); }

    /**
     * @brief 获取已使用对象数量
     * @return 已使用对象数量
     */
    int GetUsedCount() const { return m_pObjectPool->GetUsedCount(); }

    /**
     * @brief 获取空闲对象数量
     * @return 空闲对象数量
     */
    int GetFreeCount() const { return m_pObjectPool->GetFreeCount(); }

    /**
     * @brief 获取对象大小
     * @return 对象大小
     */
    int GetObjSize() const { return m_nObjSize; }

    /**
     * @brief 获取内存总大小
     * @return 内存总大小
     */
    int GetMemSize() const { return GetItemCount() * m_nObjSize; }

    /**
     * @brief 获取哈希表大小
     * @param itemCount 对象数量
     * @return 哈希表大小
     */
    static int GetHashSize(int itemCount) { return NFShmDyHashMapWithList<uint64_t, int>::CountSize(itemCount); }
public:
    /**
     * @brief 迭代器递增操作
     * @param iPos 当前位置
     * @return 递增后的位置
     */
    virtual size_t IterIncr(size_t iPos)
    {
        ++iPos;
        return iPos;
    }

    /**
     * @brief 迭代器递减操作
     * @param iPos 当前位置
     * @return 递减后的位置
     */
    virtual size_t IterDecr(size_t iPos)
    {
        --iPos;
        return iPos;
    }

    /**
     * @brief 获取迭代器起始位置
     * @return 起始位置
     */
    virtual size_t IterBegin()
    {
        return 0;
    }

    /**
     * @brief 获取迭代器结束位置
     * @return 结束位置
     */
    virtual size_t IterEnd()
    {
        return m_objIndexVec.size();
    }

    virtual size_t IterNext(size_t iPos)
    {
        ++iPos;
        return iPos;
    }

    virtual NFObject* GetIterObj(size_t iPos)
    {
        if (iPos < m_objIndexVec.size())
        {
            return m_objIndexVec[iPos].GetAttachedObj();
        }
        return nullptr;
    }

    virtual const NFObject* GetIterObj(size_t iPos) const
    {
        if (iPos < m_objIndexVec.size())
        {
            return m_objIndexVec[iPos].GetAttachedObj();
        }
        return nullptr;
    }

    std::vector<NFMemIdx>& GetObjIndexVec() { return m_objIndexVec; }
    const std::vector<NFMemIdx>& GetObjIndexVec() const { return m_objIndexVec; }
private:
    NFMemObjSeg();

    ~NFMemObjSeg();

    int SetAndInitObj(NFCMemMngModule* pShmModule, size_t nObjSize, int iItemCount, NFObject*(*pfCreateObj)(void*), bool iUseHash = false);

    size_t m_nObjSize;
    std::unordered_map<NFObjectHashKey, int> m_hashMgr;
    bool m_iUseHash;

    NFObject*(*m_pCreateFn)(void*);
    NFCMemMngModule* m_pShmModule;

    NFChunkPool* m_pObjectPool;
    std::vector<NFMemIdx> m_objIndexVec;
};


