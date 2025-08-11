// -------------------------------------------------------------------------
//    @FileName         :    NFNodeList.h
//    @author           :    gaoyi
//    @Date             :    22-11-14
//    @Email			:    445267987@qq.com
//    @Module           :    NFNodeList
//
// -------------------------------------------------------------------------

/**
 * @file NFNodeList.h
 * @brief 共享内存链表实现文件
 * @details 定义了NFrame框架中的共享内存双向链表实现，提供了链表节点对象接口、
 *          链表管理器等功能。专门为共享内存环境设计，支持对象的链表管理、
 *          快速插入删除、迭代访问等操作。
 * 
 * @author gaoyi
 * @date 22-11-14
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFIListNodeObj: 链表节点对象接口
 *       - NFListMgr: 链表管理器模板类
 *       - 支持头插、尾插、任意位置插入
 *       - 支持节点删除和链表清空
 *       - 提供链表大小和空状态检查
 * 
 * @warning 使用链表时需要注意：
 *          - 节点对象必须继承NFIListNodeObj接口
 *          - 链表在共享内存环境中使用，需要注意内存安全
 *          - 删除节点时需要确保没有其他地方引用该节点
 *          - 链表操作不是线程安全的，需要外部同步
 */

#pragma once

#include "NFRawObject.h"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @class NFIListNodeObj
 * @brief 链表节点对象接口类
 * @details 提供双向链表节点的基本接口，包含前驱和后继节点的指针管理。
 *          该类专为共享内存环境设计，支持节点的创建初始化和恢复初始化。
 *          所有需要作为链表节点的对象都必须继承此接口。
 * 
 * @note 此类需要被继承使用，不能直接实例化
 * @warning 链表操作不是线程安全的，需要外部同步控制
 */
class NFIListNodeObj
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~NFIListNodeObj()
    {
    }

    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式选择初始化方式
     */
    NFIListNodeObj()
    {
        if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 创建初始化
     * @details 第一次创建时的初始化操作，将所有节点指针设为无效值
     * @return int 成功返回0
     */
    int CreateInit()
    {
        m_iPrevNode = INVALID_ID;
        m_iNextNode = INVALID_ID;
        m_iListCheckId = INVALID_ID;
        return 0;
    }

    /**
     * @brief 恢复初始化
     * @details 从共享内存恢复时的初始化操作，当前为空实现
     * @return int 成功返回0
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 获取前驱节点ID
     * @return int 前驱节点的ID，如果没有则返回INVALID_ID
     */
    int GetPrevNode() const
    {
        return m_iPrevNode;
    }

    /**
     * @brief 设置前驱节点ID
     * @param iNode 前驱节点的ID
     */
    void SetPrevNode(int iNode)
    {
        m_iPrevNode = iNode;
    }

    /**
     * @brief 获取后继节点ID
     * @return int 后继节点的ID，如果没有则返回INVALID_ID
     */
    int GetNextNode() const
    {
        return m_iNextNode;
    }

    /**
     * @brief 设置后继节点ID
     * @param iNode 后继节点的ID
     */
    void SetNextNode(int iNode)
    {
        m_iNextNode = iNode;
    }

    /**
     * @brief 获取链表检查ID
     * @details 用于验证节点是否属于某个特定的链表
     * @return int 链表检查ID
     */
    int GetListCheckId() const
    {
        return m_iListCheckId;
    }

    /**
     * @brief 设置链表检查ID
     * @param iListCheckId 要设置的链表检查ID
     */
    void SetListCheckId(int iListCheckId)
    {
        m_iListCheckId = iListCheckId;
    }

    /**
     * @brief 获取链表检查ID序列号
     * @details 静态方法，用于生成唯一的链表检查ID
     * @return int 新的链表检查ID序列号
     */
    static int GetListCheckIdSeq();

public:
    static int m_iListCheckIdSeq;  ///< 链表检查ID序列号，用于生成唯一ID
    int m_iListCheckId;            ///< 链表检查ID，标识节点属于哪个链表
    int m_iPrevNode;               ///< 前驱节点ID
    int m_iNextNode;               ///< 后继节点ID
};

/**
 * @class NFListNodeObjWithObjectId
 * @brief 基于对象ID的链表节点模板类
 * @details 继承自NFIListNodeObj，使用对象的ObjId作为链表节点ID。
 *          适用于需要使用对象内部ID进行链表管理的场景。
 * 
 * @tparam NodeObjType 节点对象类型，必须提供GetObjByObjId和GetObjId方法
 */
template <class NodeObjType>
class NFListNodeObjWithObjectId final : public NFIListNodeObj
{
public:
    /**
     * @brief 析构函数
     */
    ~NFListNodeObjWithObjectId() override
    {
    }

    /**
     * @brief 根据链表节点ID获取对象指针
     * @param iListNodeId 链表节点ID
     * @return NodeObjType* 对象指针，如果找不到返回nullptr
     */
    static NodeObjType* GetObjByListNodeId(int iListNodeId)
    {
        return NodeObjType::GetObjByObjId(iListNodeId);
    }

    /**
     * @brief 获取当前对象的链表节点ID
     * @return int 当前对象的ObjId
     */
    int GetListNodeId()
    {
        return dynamic_cast<NodeObjType*>(this)->GetObjId();
    }
};

/**
 * @class NFListNodeObjWithGlobalId
 * @brief 基于全局ID的链表节点模板类
 * @details 继承自NFIListNodeObj，使用对象的GlobalId作为链表节点ID。
 *          适用于需要使用全局唯一ID进行链表管理的场景。
 * 
 * @tparam NodeObjType 节点对象类型，必须提供GetObjByGlobalId和GetGlobalId方法
 */
template <class NodeObjType>
class NFListNodeObjWithGlobalId : public NFIListNodeObj
{
public:
    /**
     * @brief 析构函数
     */
    ~NFListNodeObjWithGlobalId() override
    {
    }

    /**
     * @brief 根据链表节点ID获取对象指针
     * @param iListNodeId 链表节点ID
     * @return NodeObjType* 对象指针，如果找不到返回nullptr
     */
    static NodeObjType* GetObjByListNodeId(int iListNodeId)
    {
        return NodeObjType::GetObjByGlobalId(iListNodeId, true);
    }

    /**
     * @brief 获取当前对象的链表节点ID
     * @return int 当前对象的GlobalId
     */
    int GetListNodeId()
    {
        return dynamic_cast<NodeObjType*>(this)->GetGlobalId();
    }
};

/**
 * @class NFNodeObjList
 * @brief 双向链表模板类
 * @details 实现了一个完整的双向链表，支持头插、尾插、删除等操作。
 *          专为共享内存环境设计，提供了节点计数、空状态检查等功能。
 *          链表通过节点对象的ID进行管理，支持快速的插入和删除操作。
 * 
 * @tparam NodeObjType 节点对象类型，必须继承自相应的链表节点接口
 * 
 * @note 链表操作的时间复杂度：
 *       - 头插/尾插：O(1)
 *       - 删除指定节点：O(1)
 *       - 查找节点：O(n)
 * 
 * @warning 链表不是线程安全的，多线程使用时需要外部同步
 */
template <class NodeObjType>
class NFNodeObjList final : public NFRawObject
{
public:
    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式选择初始化方式
     */
    NFNodeObjList()
    {
        if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 创建初始化
     * @return int 成功返回0
     */
    int CreateInit()
    {
        InitNodeList();
        return 0;
    }

    /**
     * @brief 恢复初始化
     * @return int 成功返回0
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 初始化链表
     * @details 将链表重置为空状态，并生成新的检查ID
     */
    void InitNodeList()
    {
        m_iNodeCount = 0;
        m_iHeadNode = INVALID_ID;
        m_iTailNode = INVALID_ID;
        m_iListCheckId = NFIListNodeObj::GetListCheckIdSeq();
    }

    /**
     * @brief 检查链表是否为空
     * @return bool true表示链表为空，false表示链表不为空
     */
    bool IsEmpty() const
    {
        return m_iNodeCount <= 0;
    }

    /**
     * @brief 检查链表是否为空（STL风格接口）
     * @return bool true表示链表为空，false表示链表不为空
     */
    bool Empty() const
    {
        return IsEmpty();
    }

    /**
     * @brief 获取链表节点数量
     * @return int 链表中的节点数量
     */
    int GetNodeCount() const
    {
        return m_iNodeCount;
    }

    /**
     * @brief 获取头节点ID
     * @return int 头节点的ID，如果链表为空返回INVALID_ID
     */
    int GetHeadNodeId() const
    {
        return m_iHeadNode;
    }

    /**
     * @brief 获取尾节点ID
     * @return int 尾节点的ID，如果链表为空返回INVALID_ID
     */
    int GetTailNodeId() const
    {
        return m_iTailNode;
    }

    /**
     * @brief 获取链表检查ID
     * @return int 当前链表的检查ID
     */
    int GetLastCheckId() const
    {
        return m_iListCheckId;
    }

    /**
     * @brief 获取头节点对象指针
     * @return NodeObjType* 头节点对象指针，如果链表为空返回nullptr
     */
    NodeObjType* GetHeadNodeObj()
    {
        if (m_iHeadNode != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(m_iHeadNode);
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed, m_iHeadNode:{}", m_iHeadNode);
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 获取尾节点对象指针
     * @return NodeObjType* 尾节点对象指针，如果链表为空返回nullptr
     */
    NodeObjType* GetTailNodeObj()
    {
        if (m_iTailNode != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(m_iTailNode);
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed, m_iTailNode:{}", m_iTailNode);
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 获取指定节点的前驱节点
     * @param pNode 指定的节点对象指针
     * @return NodeObjType* 前驱节点对象指针，如果没有前驱节点返回nullptr
     */
    NodeObjType* GetPrevNodeObj(NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, NULL, "");
        if (pNode->GetPrevNode() != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(pNode->GetPrevNode());
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed");
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 获取指定节点的后继节点
     * @param pNode 指定的节点对象指针
     * @return NodeObjType* 后继节点对象指针，如果没有后继节点返回nullptr
     */
    NodeObjType* GetNextNodeObj(NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, NULL, "");
        if (pNode->GetNextNode() != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(pNode->GetNextNode());
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed");
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 添加节点到链表头部
     * @details 将新节点插入到链表的头部，成为新的头节点
     * @param pNode 要添加的节点对象指针
     * @return int 成功返回0，失败返回-1
     * @pre pNode不能为nullptr，且其前驱和后继节点必须为INVALID_ID
     */
    int AddNode(NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetPrevNode() == INVALID_ID, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetNextNode() == INVALID_ID, -1, "");
        //assert(pNode->GetListCheckId() == INVALID_ID);
        NodeObjType* pHead = GetHeadNodeObj();

        if (pHead)
        {
            pNode->SetNextNode(pHead->GetListNodeId());
            pHead->SetPrevNode(pNode->GetListNodeId());
        }
        else
        {
            CHECK_EXPR_ASSERT(0 == m_iNodeCount, -1, "");
            m_iTailNode = pNode->GetListNodeId();
        }

        pNode->SetListCheckId(m_iListCheckId);
        m_iHeadNode = pNode->GetListNodeId();
        m_iNodeCount++;
        return 0;
    }

    /**
     * @brief 添加节点到链表尾部
     * @details 将新节点插入到链表的尾部，成为新的尾节点
     * @param pNode 要添加的节点对象指针
     * @return int 成功返回0，失败返回-1
     * @pre pNode不能为nullptr，且其前驱和后继节点必须为INVALID_ID
     */
    int AddNodeToTail(NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetPrevNode() == INVALID_ID, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetNextNode() == INVALID_ID, -1, "");
        //assert(pNode->GetListCheckId() == INVALID_ID);
        NodeObjType* pTail = GetTailNodeObj();

        if (pTail)
        {
            pTail->SetNextNode(pNode->GetListNodeId());
            pNode->SetPrevNode(pTail->GetListNodeId());
        }
        else
        {
            CHECK_EXPR_ASSERT(0 == m_iNodeCount, -1, "");
            m_iHeadNode = pNode->GetListNodeId();
        }

        pNode->SetListCheckId(m_iListCheckId);
        m_iTailNode = pNode->GetListNodeId();
        m_iNodeCount++;
        return 0;
    }

    /**
     * @brief 从链表中移除指定节点
     * @details 从链表中删除指定的节点，并重新连接前驱和后继节点
     * @param pNode 要移除的节点对象指针
     * @return int 成功返回0，失败返回-1
     * @pre pNode必须是链表中的有效节点
     * @post 节点的前驱、后继和检查ID都会被重置为INVALID_ID
     */
    int RemoveNode(NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, -1, "");
        CHECK_EXPR_ASSERT(m_iNodeCount > 0, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetListCheckId() == m_iListCheckId, -1, "");
        NodeObjType* pPrevNode = GetPrevNodeObj(pNode);

        if (pPrevNode)
        {
            pPrevNode->SetNextNode(pNode->GetNextNode());
        }

        NodeObjType* pNextNode = GetNextNodeObj(pNode);

        if (pNextNode)
        {
            pNextNode->SetPrevNode(pNode->GetPrevNode());
        }

        if (pNode->GetListNodeId() == m_iHeadNode)
        {
            m_iHeadNode = pNode->GetNextNode();
        }

        if (pNode->GetListNodeId() == m_iTailNode)
        {
            m_iTailNode = pNode->GetPrevNode();
        }

        pNode->SetNextNode(INVALID_ID);
        pNode->SetPrevNode(INVALID_ID);
        pNode->SetListCheckId(INVALID_ID);
        m_iNodeCount--;
        return 0;
    }

    /**
     * @brief 检查节点是否存在于链表中
     * @param pNode 要检查的节点对象指针
     * @return bool true表示节点在链表中，false表示不在
     */
    bool IsExistNode(NodeObjType* pNode)
    {
        return pNode->GetListCheckId() == m_iListCheckId;
    }

private:
    int m_iListCheckId;  ///< 链表检查ID，用于验证节点归属
    int m_iNodeCount;    ///< 链表节点数量
    int m_iHeadNode;     ///< 头节点ID
    int m_iTailNode;     ///< 尾节点ID
};

/**
 * @class NFIMultiListNodeObj
 * @brief 多链表节点对象接口类
 * @details 支持一个对象同时存在于多个链表中的节点接口。
 *          每个节点可以维护MaxType个不同的链表连接信息。
 * 
 * @tparam MaxType 支持的最大链表类型数量
 * 
 * @note 该接口允许一个对象同时参与多个不同类型的链表管理
 * @warning 每个类型索引必须在[0, MaxType)范围内
 */
template <size_t MaxType>
class NFIMultiListNodeObj
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~NFIMultiListNodeObj()
    {
    }

    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式选择初始化方式
     */
    NFIMultiListNodeObj()
    {
        if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 创建初始化
     * @details 初始化所有类型的链表节点信息
     * @return int 成功返回0
     */
    int CreateInit()
    {
        for (int i = 0; i < static_cast<int>(MaxType); i++)
        {
            m_objList[i].CreateInit();
        }
        return 0;
    }

    /**
     * @brief 恢复初始化
     * @return int 成功返回0
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 获取指定类型的前驱节点ID
     * @param typeIndex 链表类型索引，必须在[0, MaxType)范围内
     * @return int 前驱节点ID，如果没有或索引无效返回-1
     */
    int GetPrevNode(int typeIndex) const
    {
        CHECK_EXPR_ASSERT(typeIndex >= 0 && typeIndex < (int)MaxType, -1, "typeIndex:{} max:{}", typeIndex, MaxType);
        return m_objList[typeIndex].m_iPrevNode;
    }

    /**
     * @brief 设置指定类型的前驱节点ID
     * @param typeIndex 链表类型索引，必须在[0, MaxType)范围内
     * @param iNode 前驱节点ID
     */
    void SetPrevNode(int typeIndex, int iNode)
    {
        CHECK_EXPR_ASSERT(typeIndex >= 0 && typeIndex < (int)MaxType, , "typeIndex:{} max:{}", typeIndex, MaxType);
        m_objList[typeIndex].m_iPrevNode = iNode;
    }

    /**
     * @brief 获取指定类型的后继节点ID
     * @param typeIndex 链表类型索引，必须在[0, MaxType)范围内
     * @return int 后继节点ID，如果没有或索引无效返回-1
     */
    int GetNextNode(int typeIndex) const
    {
        CHECK_EXPR_ASSERT(typeIndex >= 0 && typeIndex < (int)MaxType, -1, "typeIndex:{} max:{}", typeIndex, MaxType);
        return m_objList[typeIndex].m_iNextNode;
    }

    /**
     * @brief 设置指定类型的后继节点ID
     * @param typeIndex 链表类型索引，必须在[0, MaxType)范围内
     * @param iNode 后继节点ID
     */
    void SetNextNode(int typeIndex, int iNode)
    {
        CHECK_EXPR_ASSERT(typeIndex >= 0 && typeIndex < (int)MaxType, , "typeIndex:{} max:{}", typeIndex, MaxType);
        m_objList[typeIndex].m_iNextNode = iNode;
    }

    /**
     * @brief 获取指定类型的链表检查ID
     * @param typeIndex 链表类型索引，必须在[0, MaxType)范围内
     * @return int 链表检查ID，如果索引无效返回-1
     */
    int GetListCheckId(int typeIndex) const
    {
        CHECK_EXPR_ASSERT(typeIndex >= 0 && typeIndex < (int)MaxType, -1, "typeIndex:{} max:{}", typeIndex, MaxType);
        return m_objList[typeIndex].m_iListCheckId;
    }

    /**
     * @brief 设置指定类型的链表检查ID
     * @param typeIndex 链表类型索引，必须在[0, MaxType)范围内
     * @param iListCheckId 链表检查ID
     */
    void SetListCheckId(int typeIndex, int iListCheckId)
    {
        CHECK_EXPR_ASSERT(typeIndex >= 0 && typeIndex < (int)MaxType, , "typeIndex:{} max:{}", typeIndex, MaxType);
        m_objList[typeIndex].m_iListCheckId = iListCheckId;
    }

protected:
    NFIListNodeObj m_objList[MaxType];  ///< 不同类型链表的节点信息数组
};

/**
 * @class NFMultiListNodeObjWithGlobalId
 * @brief 基于全局ID的多链表节点模板类
 * @details 继承自NFIMultiListNodeObj，使用对象的GlobalId作为链表节点ID。
 *          支持一个对象同时存在于多个不同类型的链表中。
 * 
 * @tparam NodeObjType 节点对象类型，必须提供GetObjByGlobalId和GetGlobalId方法
 * @tparam MaxType 支持的最大链表类型数量
 */
template <class NodeObjType, size_t MaxType>
class NFMultiListNodeObjWithGlobalId : public NFIMultiListNodeObj<MaxType>
{
public:
    /**
     * @brief 析构函数
     */
    ~NFMultiListNodeObjWithGlobalId() override
    {
    }

    /**
     * @brief 根据链表节点ID获取对象指针
     * @param iListNodeId 链表节点ID
     * @return NodeObjType* 对象指针，如果找不到返回nullptr
     */
    static NodeObjType* GetObjByListNodeId(int iListNodeId)
    {
        return NodeObjType::GetObjByGlobalId(iListNodeId, true);
    }

    /**
     * @brief 获取当前对象的链表节点ID
     * @return int 当前对象的GlobalId
     */
    int GetListNodeId()
    {
        return dynamic_cast<NodeObjType*>(this)->GetGlobalId();
    }
};

/**
 * @class NFNodeObjMultiList
 * @brief 多类型双向链表模板类
 * @details 实现了支持多种类型节点的双向链表。
 *          每个链表可以管理同一类型的不同节点，支持头插、尾插、删除等操作。
 *          适用于需要按不同类型分类管理对象的场景。
 * 
 * @tparam NodeObjType 节点对象类型，必须继承自相应的多链表节点接口
 * 
 * @note 链表操作的时间复杂度：
 *       - 头插/尾插：O(1)
 *       - 删除指定节点：O(1)
 *       - 查找节点：O(n)
 * 
 * @warning 链表不是线程安全的，多线程使用时需要外部同步
 */
template <class NodeObjType>
class NFNodeObjMultiList final : public NFRawObject
{
public:
    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式选择初始化方式
     */
    NFNodeObjMultiList()
    {
        if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 创建初始化
     * @return int 成功返回0
     */
    int CreateInit()
    {
        InitNodeList();
        return 0;
    }

    /**
     * @brief 恢复初始化
     * @return int 成功返回0
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 拷贝构造函数
     * @param list 要拷贝的链表对象
     */
    NFNodeObjMultiList(const NFNodeObjMultiList& list)
    {
        if (this != &list)
        {
            m_iListCheckId = list.m_iListCheckId;
            m_iNodeCount = list.m_iNodeCount;
            m_iHeadNode = list.m_iHeadNode;
            m_iTailNode = list.m_iTailNode;
        }
    }

    /**
     * @brief 初始化链表
     * @details 将链表重置为空状态，并生成新的检查ID
     */
    void InitNodeList()
    {
        m_iNodeCount = 0;
        m_iHeadNode = INVALID_ID;
        m_iTailNode = INVALID_ID;
        m_iListCheckId = NFIListNodeObj::GetListCheckIdSeq();
    }

    /**
     * @brief 获取链表节点数量
     * @return int 链表中的节点数量
     */
    int GetNodeCount() const
    {
        return m_iNodeCount;
    }

    /**
     * @brief 检查链表是否为空
     * @return bool true表示链表为空，false表示链表不为空
     */
    bool IsEmpty() const
    {
        return m_iNodeCount <= 0;
    }

    /**
     * @brief 检查链表是否为空（STL风格接口）
     * @return bool true表示链表为空，false表示链表不为空
     */
    bool Empty() const
    {
        return IsEmpty();
    }

    /**
     * @brief 获取头节点ID
     * @return int 头节点的ID，如果链表为空返回INVALID_ID
     */
    int GetHeadNodeId() const
    {
        return m_iHeadNode;
    }

    /**
     * @brief 获取尾节点ID
     * @return int 尾节点的ID，如果链表为空返回INVALID_ID
     */
    int GetTailNodeId() const
    {
        return m_iTailNode;
    }

    /**
     * @brief 获取链表检查ID
     * @return int 当前链表的检查ID
     */
    int GetLastCheckId() const
    {
        return m_iListCheckId;
    }

    /**
     * @brief 获取头节点对象指针
     * @param typeIndex 链表类型索引（当前版本中未使用）
     * @return NodeObjType* 头节点对象指针，如果链表为空返回nullptr
     */
    NodeObjType* GetHeadNodeObj(int typeIndex)
    {
        if (m_iHeadNode != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(m_iHeadNode);
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed, m_iHeadNode:{}", m_iHeadNode);
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 获取尾节点对象指针
     * @param typeIndex 链表类型索引（当前版本中未使用）
     * @return NodeObjType* 尾节点对象指针，如果链表为空返回nullptr
     */
    NodeObjType* GetTailNodeObj(int typeIndex)
    {
        if (m_iTailNode != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(m_iTailNode);
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed, m_iTailNode:{}", m_iTailNode);
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 获取指定节点的前驱节点
     * @param typeIndex 链表类型索引
     * @param pNode 指定的节点对象指针
     * @return NodeObjType* 前驱节点对象指针，如果没有前驱节点返回nullptr
     */
    NodeObjType* GetPrevNodeObj(int typeIndex, NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, NULL, "");
        if (pNode->GetPrevNode(typeIndex) != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(pNode->GetPrevNode(typeIndex));
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed");
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 获取指定节点的后继节点
     * @param typeIndex 链表类型索引
     * @param pNode 指定的节点对象指针
     * @return NodeObjType* 后继节点对象指针，如果没有后继节点返回nullptr
     */
    NodeObjType* GetNextNodeObj(int typeIndex, NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, NULL, "");
        if (pNode->GetNextNode(typeIndex) != INVALID_ID)
        {
            NodeObjType* pObj = NodeObjType::GetObjByListNodeId(pNode->GetNextNode(typeIndex));
            CHECK_EXPR_ASSERT(pObj, NULL, "GetObjByListNodeId Failed");
            return pObj;
        }

        return nullptr;
    }

    /**
     * @brief 添加节点到指定类型链表头部
     * @details 将新节点插入到指定类型链表的头部，成为新的头节点
     * @param typeIndex 链表类型索引
     * @param pNode 要添加的节点对象指针
     * @return int 成功返回0，失败返回-1
     * @pre pNode不能为nullptr，且在指定类型链表中的前驱和后继节点必须为INVALID_ID
     */
    int AddNode(int typeIndex, NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetPrevNode(typeIndex) == INVALID_ID, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetNextNode(typeIndex) == INVALID_ID, -1, "");
        //assert(pNode->GetListCheckId() == INVALID_ID);
        NodeObjType* pHead = GetHeadNodeObj(typeIndex);

        if (pHead)
        {
            pNode->SetNextNode(typeIndex, pHead->GetListNodeId());
            pHead->SetPrevNode(typeIndex, pNode->GetListNodeId());
        }
        else
        {
            CHECK_EXPR_ASSERT(0 == m_iNodeCount, -1, "");
            m_iTailNode = pNode->GetListNodeId();
        }

        pNode->SetListCheckId(typeIndex, m_iListCheckId);
        m_iHeadNode = pNode->GetListNodeId();
        m_iNodeCount++;
        return 0;
    }

    /**
     * @brief 添加节点到指定类型链表尾部
     * @details 将新节点插入到指定类型链表的尾部，成为新的尾节点
     * @param typeIndex 链表类型索引
     * @param pNode 要添加的节点对象指针
     * @return int 成功返回0，失败返回-1
     * @pre pNode不能为nullptr，且在指定类型链表中的前驱和后继节点必须为INVALID_ID
     */
    int AddNodeToTail(int typeIndex, NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetPrevNode(typeIndex) == INVALID_ID, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetNextNode(typeIndex) == INVALID_ID, -1, "");
        //assert(pNode->GetListCheckId() == INVALID_ID);
        NodeObjType* pTail = GetTailNodeObj(typeIndex);

        if (pTail)
        {
            pTail->SetNextNode(typeIndex, pNode->GetListNodeId());
            pNode->SetPrevNode(typeIndex, pTail->GetListNodeId());
        }
        else
        {
            CHECK_EXPR_ASSERT(0 == m_iNodeCount, -1, "");
            m_iHeadNode = pNode->GetListNodeId();
        }

        pNode->SetListCheckId(typeIndex, m_iListCheckId);
        m_iTailNode = pNode->GetListNodeId();
        m_iNodeCount++;
        return 0;
    }

    /**
     * @brief 从指定类型链表中移除节点
     * @details 从指定类型的链表中删除指定的节点，并重新连接前驱和后继节点
     * @param typeIndex 链表类型索引
     * @param pNode 要移除的节点对象指针
     * @return int 成功返回0，失败返回-1
     * @pre pNode必须是指定类型链表中的有效节点
     * @post 节点在指定类型链表中的前驱、后继和检查ID都会被重置为INVALID_ID
     */
    int RemoveNode(int typeIndex, NodeObjType* pNode)
    {
        CHECK_EXPR_ASSERT(pNode, -1, "");
        CHECK_EXPR_ASSERT(m_iNodeCount > 0, -1, "");
        CHECK_EXPR_ASSERT(pNode->GetListCheckId(typeIndex) == m_iListCheckId, -1, "");
        NodeObjType* pPrevNode = GetPrevNodeObj(typeIndex, pNode);

        if (pPrevNode)
        {
            pPrevNode->SetNextNode(typeIndex, pNode->GetNextNode(typeIndex));
        }

        NodeObjType* pNextNode = GetNextNodeObj(typeIndex, pNode);

        if (pNextNode)
        {
            pNextNode->SetPrevNode(typeIndex, pNode->GetPrevNode(typeIndex));
        }

        if (pNode->GetListNodeId() == m_iHeadNode)
        {
            m_iHeadNode = pNode->GetNextNode(typeIndex);
        }

        if (pNode->GetListNodeId() == m_iTailNode)
        {
            m_iTailNode = pNode->GetPrevNode(typeIndex);
        }

        pNode->SetNextNode(typeIndex, INVALID_ID);
        pNode->SetPrevNode(typeIndex, INVALID_ID);
        pNode->SetListCheckId(typeIndex, INVALID_ID);
        m_iNodeCount--;
        return 0;
    }

    /**
     * @brief 检查节点是否存在于指定类型链表中
     * @param pNode 要检查的节点对象指针
     * @param typeIndex 链表类型索引
     * @return bool true表示节点在指定类型链表中，false表示不在
     */
    bool IsExistNode(NodeObjType* pNode, int typeIndex)
    {
        return pNode->GetListCheckId(typeIndex) == m_iListCheckId;
    }

private:
    int m_iListCheckId;  ///< 链表检查ID，用于验证节点归属
    int m_iNodeCount;    ///< 链表节点数量
    int m_iHeadNode;     ///< 头节点ID
    int m_iTailNode;     ///< 尾节点ID
};
