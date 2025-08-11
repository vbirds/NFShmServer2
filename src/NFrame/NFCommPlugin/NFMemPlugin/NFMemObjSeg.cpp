// -------------------------------------------------------------------------
//    @FileName         :    NFMemObjSeg.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    内存对象段实现文件，负责管理内存中的对象段，包括对象段的内存分配和管理、
//                          对象索引的创建和维护、哈希表的管理和查找、对象的创建和销毁、内存池的管理。
//                          该文件实现了NFShmXFrame框架的内存对象段类，提供对象段初始化、内存分配和释放、
//                          索引管理、哈希表操作、对象生命周期管理等功能。
//                          主要功能包括高效的内存管理、灵活的索引机制、可选的哈希表支持、自动内存回收
//
// -------------------------------------------------------------------------

#include "NFMemObjSeg.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFMemIdx.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFCMemMngModule.h"

NFMemObjSeg::NFMemObjSeg()
{
    m_nObjSize = 0;
    m_iUseHash = false;
    m_pCreateFn = nullptr;
    m_pObjectPool = nullptr;
}

int NFMemObjSeg::SetAndInitObj(NFCMemMngModule* pShmModule, size_t nObjSize, int iItemCount, NFObject*(*pfCreateObj)(void*), bool iUseHash)
{
    m_pShmModule = pShmModule;
    m_nObjSize = nObjSize;
    m_iUseHash = iUseHash;
    m_pCreateFn = pfCreateObj;

    int chunkCount = iItemCount > 500 ? 500 : iItemCount;
    chunkCount = chunkCount <= 0 ? 1 : chunkCount;
    m_pObjectPool = new NFChunkPool(sizeof(int), nObjSize, chunkCount, true);

    NFShmMgr::Instance()->SetCreateMode(EN_OBJ_MODE_INIT);
    m_pShmModule->SetCreateMode(EN_OBJ_MODE_INIT);

    return 0;
}

NFMemObjSeg::~NFMemObjSeg()
{
    NF_SAFE_DELETE(m_pObjectPool);
}

/**
 * @brief 根据索引获取对象
 * 
 * 该函数根据对象索引获取对应的对象指针。
 * 获取过程包括：
 * 1. 验证对象池的有效性
 * 2. 根据索引从对象池获取内存块
 * 3. 从内存块头部获取索引信息
 * 4. 验证索引的有效性
 * 5. 从索引向量中获取对象索引
 * 6. 验证索引匹配
 * 7. 返回关联的对象指针
 * 
 * @param iIdx 对象索引
 * @return 对象指针，失败返回nullptr
 */
NFObject* NFMemObjSeg::GetObj(int iIdx)
{
    CHECK_NULL_RE_NULL(0, m_pObjectPool, "");
    // 根据索引从对象池获取内存块
    void *p = m_pObjectPool->GetChunkByIndex(iIdx);
    auto pHead = static_cast<int*>(p);
    CHECK_EXPR(pHead, nullptr, "pHead == NULL, iIdx:{} stack:{}", iIdx, TRACE_STACK());
    // 从内存块头部获取索引信息
    int iHeadIndex = *pHead;
    CHECK_EXPR(iHeadIndex >= 0 && iHeadIndex < (int)m_objIndexVec.size(), nullptr, "iHeadIndex:{} not valid, stack:{}", iHeadIndex, TRACE_STACK());
    // 从索引向量中获取对象索引
    NFMemIdx& idx = m_objIndexVec[iHeadIndex];
    CHECK_EXPR(iIdx == idx.GetIndex(), nullptr, "iIndex == idx.GetIndex(), stack:{}", TRACE_STACK());
    return idx.GetAttachedObj();
}

/**
 * @brief 向哈希表中插入键值对
 * 
 * 该函数用于向哈希表中插入键值对，建立哈希键与对象ID的映射关系。
 * 如果哈希键已存在，则插入失败。
 * 
 * @param key 哈希键
 * @param objId 对象ID
 * @return 插入结果，0表示成功，INVALID_ID表示失败
 */
int NFMemObjSeg::HashInsert(NFObjectHashKey key, int objId)
{
    auto iter = m_hashMgr.find(key);
    if (iter == m_hashMgr.end())
    {
        m_hashMgr.insert(std::make_pair(key, objId));
        return 0;
    }

    return INVALID_ID;
}

/**
 * @brief 在哈希表中查找对象
 * 
 * 该函数根据哈希键在哈希表中查找对应的对象。
 * 查找过程包括：
 * 1. 在哈希表中查找哈希键
 * 2. 如果找到，则根据对象ID获取对象指针
 * 3. 返回对象指针
 * 
 * @param key 哈希键
 * @return 对象指针，未找到返回nullptr
 */
NFObject* NFMemObjSeg::HashFind(NFObjectHashKey key)
{
    auto iter = m_hashMgr.find(key);
    if (iter != m_hashMgr.end())
    {
        int objId = iter->second;
        return GetObj(objId);
    }

    return nullptr;
}

/**
 * @brief 从哈希表中删除键值对
 * 
 * 该函数用于从哈希表中删除指定的键值对。
 * 
 * @param key 要删除的哈希键
 * @return 删除结果，0表示成功，-1表示失败
 */
int NFMemObjSeg::HashErase(NFObjectHashKey key)
{
    size_t count = m_hashMgr.erase(key);
    if (count > 0)
    {
        return 0;
    }

    return -1;
}

/**
 * @brief 为对象分配内存
 * 
 * 该函数用于为对象分配内存空间。
 * 分配过程包括：
 * 1. 验证对象池的有效性
 * 2. 从对象池分配内存块
 * 3. 获取内存块的索引
 * 4. 创建对象索引并设置相关属性
 * 5. 将对象索引添加到索引向量
 * 6. 设置内存块头部的索引信息
 * 7. 返回对象缓冲区指针
 * 
 * @return 分配的内存指针，失败返回nullptr
 */
void* NFMemObjSeg::AllocMemForObject()
{
    CHECK_NULL_RE_NULL(0, m_pObjectPool, "m_pObjectPool == NULL");
    // 从对象池分配内存块
    void* p = m_pObjectPool->AllocChunk();
    CHECK_EXPR(p, nullptr, "p == NULL, statck:{}", TRACE_STACK());
    // 获取内存块的索引
    int iIndex = m_pObjectPool->GetChunkIndex(p);
    CHECK_EXPR(iIndex >= 0, nullptr, "iIndex:{} not valid, stack:{}", iIndex, TRACE_STACK());
    // 创建对象索引并设置相关属性
    NFMemIdx idx;
    idx.SetObjBuf(reinterpret_cast<void*>(reinterpret_cast<size_t>(p)+sizeof(int)));
    idx.SetIndex(iIndex);
    // 将对象索引添加到索引向量
    m_objIndexVec.push_back(idx);
    auto pHead = static_cast<int*>(p);
    CHECK_EXPR(pHead, nullptr, "pHead == NULL, statck:{}", TRACE_STACK());
    // 设置内存块头部的索引信息
    *pHead = static_cast<int>(m_objIndexVec.size()) - 1;
    return idx.GetObjBuf();
}

/**
 * @brief 释放对象内存
 * 
 * 该函数用于释放对象占用的内存空间。
 * 释放过程包括：
 * 1. 验证内存指针和对象池的有效性
 * 2. 计算内存块的起始地址
 * 3. 获取内存块的索引
 * 4. 从内存块头部获取索引信息
 * 5. 验证索引的有效性
 * 6. 从索引向量中获取对象索引
 * 7. 验证内存地址和索引的匹配
 * 8. 从索引向量中移除对象索引
 * 9. 更新内存块头部的索引信息
 * 10. 释放内存块
 * 
 * @param pMem 要释放的内存指针
 * @return 释放结果，0表示成功，-1表示失败
 */
int NFMemObjSeg::FreeMemForObject(void* pMem)
{
    CHECK_NULL(0, pMem);
    CHECK_NULL(0, m_pObjectPool);
    // 计算内存块的起始地址
    auto p = reinterpret_cast<void*>(reinterpret_cast<size_t>(pMem) - sizeof(int));
    CHECK_EXPR(p, -1, "p == NULL, stack:{}", TRACE_STACK());
    // 获取内存块的索引
    int iIndex = m_pObjectPool->GetChunkIndex(p);
    CHECK_EXPR(iIndex >= 0, -1, "iIndex:{} not valid, stack:{}", iIndex, TRACE_STACK());
    // 从内存块头部获取索引信息
    auto pHead = static_cast<int*>(p);
    CHECK_EXPR(pHead, -1, "pHead == NULL, stack:{}", TRACE_STACK());
    int iHeadIndex = *pHead;
    CHECK_EXPR(iHeadIndex >= 0 && iHeadIndex < (int)m_objIndexVec.size(), -1, "iHeadIndex:{} not valid, stack:{}", iHeadIndex, TRACE_STACK());
    // 从索引向量中获取对象索引
    NFMemIdx& idx = m_objIndexVec[iHeadIndex];
    CHECK_EXPR(pMem == idx.GetObjBuf(), -1, "pMem != idx.GetObjBuf(), stack:{}", TRACE_STACK());
    CHECK_EXPR(iIndex == idx.GetIndex(), -1, "iIndex == idx.GetIndex(), stack:{}", TRACE_STACK());
    // 从索引向量中移除对象索引
    if (iHeadIndex == static_cast<int>(m_objIndexVec.size()) - 1)
    {
        m_objIndexVec.pop_back();
    }
    else
    {
        // 更新内存块头部的索引信息
        NFMemIdx& backIdx = m_objIndexVec.back();
        void* pBack = backIdx.GetObjBuf();
        CHECK_EXPR(pBack, -1, "pBack == NULL, stack:{}", TRACE_STACK());
        auto pBackHead = reinterpret_cast<void*>(reinterpret_cast<size_t>(pBack) - sizeof(int));
        CHECK_EXPR(pBackHead, -1, "pBackHead == NULL, stack:{}", TRACE_STACK());
        int iBackIndex = m_pObjectPool->GetChunkIndex(pBackHead);
        CHECK_EXPR(iBackIndex >= 0 && iBackIndex == backIdx.GetIndex(), -1, "iBackIndex:{} not valid, stack:{}", iBackIndex, TRACE_STACK());
        auto pIntBackHead = static_cast<int*>(pBackHead);
        CHECK_EXPR(pIntBackHead, -1, "pBackHead == NULL, stack:{}", TRACE_STACK());
        CHECK_EXPR(*pIntBackHead == static_cast<int>(m_objIndexVec.size()) -1, -1, "stack:{}", TRACE_STACK());
        *pIntBackHead = iHeadIndex;

        m_objIndexVec[iHeadIndex] = backIdx;
        m_objIndexVec.pop_back();
    }

    m_pObjectPool->FreeChunk(p);
    return 0;
}

NFObject* NFMemObjSeg::GetHeadObj()
{
    if (m_objIndexVec.empty())
    {
        return nullptr;
    }

    return m_objIndexVec.front().GetAttachedObj();
}

NFObject* NFMemObjSeg::GetNextObj(NFObject* pObj)
{
    CHECK_NULL_RET_VAL(0, m_pObjectPool, nullptr);
    auto p = reinterpret_cast<void*>(reinterpret_cast<size_t>(pObj) - sizeof(int));
    CHECK_EXPR(p, nullptr, "p == NULL, stack:{}", TRACE_STACK());
    int iIndex = m_pObjectPool->GetChunkIndex(p);
    CHECK_EXPR(iIndex >= 0, nullptr, "iIndex:{} not valid, stack:{}", iIndex, TRACE_STACK());
    auto pHead = static_cast<int*>(p);
    CHECK_EXPR(pHead, nullptr, "pHead == NULL, stack:{}", TRACE_STACK());
    int iHeadIndex = *pHead;
    CHECK_EXPR(iHeadIndex >= 0 && iHeadIndex < (int)m_objIndexVec.size(), nullptr, "iHeadIndex:{} not valid, stack:{}", iHeadIndex, TRACE_STACK());
    NFMemIdx& idx = m_objIndexVec[iHeadIndex];
    CHECK_EXPR(pObj == idx.GetAttachedObj(), pObj, "pMem != idx.GetObjBuf(), stack:{}", TRACE_STACK());
    CHECK_EXPR(iIndex == idx.GetIndex(), pObj, "iIndex == idx.GetIndex(), stack:{}", TRACE_STACK());
    ++iHeadIndex;
    if (iHeadIndex >= 0 && iHeadIndex < static_cast<int>(m_objIndexVec.size()))
    {
        NFMemIdx& idxNext = m_objIndexVec[iHeadIndex];
        return idxNext.GetAttachedObj();
    }
    return nullptr;
}

bool NFMemObjSeg::IsEnd(int iIndex)
{
    if (iIndex >= static_cast<int>(m_objIndexVec.size()))
        return true;
    return false;
}

int NFMemObjSeg::GetObjId(void* pstObj) const
{
    CHECK_NULL_RET_VAL(0, m_pObjectPool, INVALID_ID);
    auto p = reinterpret_cast<void*>(reinterpret_cast<size_t>(pstObj) - sizeof(int));
    CHECK_EXPR(p, -1, "p == NULL, stack:{}", TRACE_STACK());
    return m_pObjectPool->GetChunkIndex(p);
}
