// -------------------------------------------------------------------------
//    @FileName         :    NFDoubleList.h
//    @Author           :    gaoyi
//    @Date             :    2022/10/10
//    @Email			:    445267987@qq.com
//    @Module           :    NFDoubleList
//
// -------------------------------------------------------------------------

/**
 * @file NFDoubleList.h
 * @brief 双向链表实现
 * 
 * 此文件提供了高效的双向链表数据结构实现。
 * 支持在链表头部和尾部快速插入节点，以及删除任意节点。
 * 适用于需要双向遍历和频繁插入删除的场景。
 */

#pragma once

#include "NFPlatform.h"
#include "NFLikely.h"

/**
 * @brief 双向链表节点
 * 
 * NFDoubleNode表示双向链表中的一个节点，包含指向前后节点的指针
 * 和一个通用的宿主对象指针。
 * 
 * 设计特点：
 * - 通用性：host指针可以指向任意类型的对象
 * - 双向链接：同时维护前向和后向指针
 * - 生命周期管理：提供完整的构造、析构和重置功能
 * - 内存安全：所有指针操作都有合理的默认值和重置机制
 * 
 * 使用方法：
 * @code
 * // 创建节点
 * MyObject* obj = new MyObject();
 * NFDoubleNode* node = new NFDoubleNode(obj);
 * 
 * // 获取宿主对象
 * MyObject* retrieved = static_cast<MyObject*>(node->GetHost());
 * 
 * // 重置节点
 * node->Reset();
 * @endcode
 */
struct NFDoubleNode
{
    /** @brief 宿主对象指针，指向实际存储的数据 */
    void*       host;
    /** @brief 前一个节点指针 */
    NFDoubleNode* prev;
    /** @brief 后一个节点指针 */
    NFDoubleNode* next;
    
    /**
     * @brief 默认构造函数
     * 
     * 创建一个空节点，所有指针都初始化为NULL。
     */
    NFDoubleNode()
    {
        Reset();
    }

    /**
     * @brief 析构函数
     * 
     * 销毁节点时重置所有指针，但不释放宿主对象。
     * 
     * @note 宿主对象的生命周期由调用者管理
     */
    ~NFDoubleNode()
    {
        Reset();
    }

    /**
     * @brief 带宿主对象的构造函数
     * 
     * 创建一个包含指定宿主对象的节点。
     * 
     * @param h 宿主对象指针
     */
    explicit NFDoubleNode(void* h)
    {
        host = h;
        ResetNode();
    }

    /**
     * @brief 完全重置节点
     * 
     * 将所有指针（包括宿主对象指针）重置为NULL。
     */
    void Reset()
    {
        prev = NULL;
        next = NULL;
        host = NULL;
    }

    /**
     * @brief 重置链表指针
     * 
     * 仅重置前后节点指针，保留宿主对象指针。
     * 
     * @return bool 总是返回true
     */
    bool ResetNode()
    {
        prev = NULL;
        next = NULL;
        return true;
    }

    /**
     * @brief 获取宿主对象指针
     * 
     * @return void* 宿主对象指针
     */
    void* GetHost()
    {
        return host;
    }

    /**
     * @brief 设置宿主对象指针
     * 
     * @param h 要设置的宿主对象指针
     */
    void SetHost(void* h)
    {
        host = h;
    }
};

/**
 * @brief 双向链表容器
 * 
 * NFDoubleList提供了双向链表的完整实现，支持在头部和尾部快速插入，
 * 以及删除任意节点的操作。
 * 
 * 主要特性：
 * - 双向遍历：支持从头到尾和从尾到头的双向遍历
 * - 快速插入：在头部和尾部插入节点的时间复杂度为O(1)
 * - 灵活删除：删除任意节点的时间复杂度为O(1)
 * - 通用存储：可以存储任意类型的对象指针
 * - 轻量设计：最小化内存开销和操作复杂度
 * 
 * 设计原理：
 * - 维护头尾指针：快速访问链表两端
 * - 计数器：实时跟踪链表中的节点数量
 * - 双向连接：每个节点都知道前后节点的位置
 * 
 * 适用场景：
 * - LRU缓存实现
 * - 任务队列管理
 * - 事件处理链
 * - 需要频繁插入删除的数据结构
 * - 双向遍历的需求
 * 
 * 使用方法：
 * @code
 * NFDoubleList list;
 * 
 * // 创建并插入节点
 * MyObject* obj1 = new MyObject(1);
 * MyObject* obj2 = new MyObject(2);
 * 
 * NFDoubleNode* node1 = new NFDoubleNode(obj1);
 * NFDoubleNode* node2 = new NFDoubleNode(obj2);
 * 
 * list.InsertTail(node1);
 * list.InsertHead(node2);
 * 
 * // 遍历链表
 * NFDoubleNode* current = list.Head();
 * while (current != nullptr) {
 *     MyObject* obj = static_cast<MyObject*>(current->GetHost());
 *     // 处理对象
 *     current = list.Next(current);
 * }
 * 
 * // 删除节点
 * list.Delete(node1);
 * 
 * // 清空链表
 * list.Clear();
 * @endcode
 * 
 * @note 节点的内存管理由调用者负责
 * @note 宿主对象的生命周期由调用者管理
 * @note 此实现不是线程安全的，多线程使用时需要外部同步
 */
class NFDoubleList
{
private:
    /** @brief 链表中节点的数量 */
    uint32_t    count_;
    /** @brief 头节点指针 */
    NFDoubleNode* head_;
    /** @brief 尾节点指针 */
    NFDoubleNode* tail_;

public:
    /**
     * @brief 默认构造函数
     * 
     * 创建一个空的双向链表。
     */
    NFDoubleList()
    {
        count_ = 0;
        head_  = NULL;
        tail_  = NULL;
    }
    
    /**
     * @brief 析构函数
     * 
     * 销毁链表，但不释放节点或宿主对象的内存。
     * 
     * @note 调用者需要在销毁链表前手动清理所有节点
     */
    ~NFDoubleList() {}

    /**
     * @brief 清空链表
     * 
     * 将链表重置为空状态，不释放节点内存。
     * 
     * @note 调用者需要在调用此方法前手动释放所有节点的内存
     */
    void Clear()
    {
        count_ = 0;
        head_  = NULL;
        tail_  = NULL;
    }

    /**
     * @brief 获取节点数量
     * 
     * @return uint32_t 链表中节点的数量
     */
    uint32_t Count()
    {
        return count_;
    }

    /**
     * @brief 在链表尾部插入节点
     * 
     * 将节点插入到链表的尾部，成为新的尾节点。
     * 
     * @param elem 要插入的节点指针，不能为NULL
     * 
     * @pre elem不能为NULL
     * @post 链表计数器递增1
     * @post elem成为新的尾节点
     */
    void InsertTail(NFDoubleNode* elem)
    {
        NF_COMM_ASSERT(elem);
        if (LIKELY(tail_ != NULL))
        {
            Insert(tail_, elem);
            tail_ = elem;
        }
        else
        {
            head_ = tail_ = elem;
            elem->next = nullptr;
        }
        ++count_;
    }

    /**
     * @brief 在链表头部插入节点
     * 
     * 将节点插入到链表的头部，成为新的头节点。
     * 
     * @param elem 要插入的节点指针，不能为NULL
     * 
     * @pre elem不能为NULL
     * @post 链表计数器递增1
     * @post elem成为新的头节点
     */
    void InsertHead(NFDoubleNode* elem)
    {
        NF_COMM_ASSERT(elem);
        if (LIKELY(head_ != NULL))
        {
            elem->next = head_;
            head_->prev = elem;
            head_ = elem;
        }
        else
        {
            head_ = tail_ = elem;
        }
        ++count_;
    }

    /**
     * @brief 删除指定节点
     * 
     * 从链表中删除指定的节点，但不释放节点内存。
     * 
     * @param elem 要删除的节点指针
     * @return bool 删除成功返回true，节点不在链表中返回false
     * 
     * @note 此方法只从链表中移除节点，不释放节点内存
     * @note 调用者需要自行管理节点的内存释放
     */
    bool Delete(NFDoubleNode* elem);
    
    /**
     * @brief 检查链表是否为空
     * 
     * @return bool 链表为空返回true，否则返回false
     */
    bool IsEmpty() {return head_ == NULL;}

    /**
     * @brief 获取头节点
     * 
     * @return NFDoubleNode* 头节点指针，链表为空时返回NULL
     */
    NFDoubleNode* Head() {return head_;}
    
    /**
     * @brief 获取尾节点
     * 
     * @return NFDoubleNode* 尾节点指针，链表为空时返回NULL
     */
    NFDoubleNode* Tail() {return tail_;}
    
    /**
     * @brief 获取指定节点的前一个节点
     * 
     * @param cur 当前节点指针
     * @return NFDoubleNode* 前一个节点指针，如果没有前节点或cur为NULL则返回NULL
     */
    NFDoubleNode* Prev(NFDoubleNode* cur)
    {
        NF_COMM_ASSERT(cur);
        if (cur)
        {
            return cur->prev;
        }
        else
        {
            return NULL;
        }
    }
    
    /**
     * @brief 获取指定节点的后一个节点
     * 
     * @param cur 当前节点指针
     * @return NFDoubleNode* 后一个节点指针，如果没有后节点或cur为NULL则返回NULL
     */
    NFDoubleNode* Next(NFDoubleNode* cur)
    {
        NF_COMM_ASSERT(cur);
        if (cur)
        {
            return cur->next;
        }
        else
        {
            return NULL;
        }
    }

protected:
    /**
     * @brief 在指定节点后插入新节点
     * 
     * 这是一个内部辅助方法，用于在指定节点后插入新节点。
     * 
     * @param prev 前一个节点
     * @param elem 要插入的新节点
     * 
     * @note 这是受保护的方法，仅供内部使用
     */
    void Insert(NFDoubleNode* prev, NFDoubleNode* elem);
};


