// -------------------------------------------------------------------------
//    @FileName         :    NFObjPtr.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

/**
 * @file NFObjPtr.h
 * @brief 共享内存对象指针实现文件
 * @details 定义了NFrame框架中的智能对象指针NFObjPtr，专门用于共享内存环境下的
 *          对象指针管理。提供了自动的指针有效性检查、共享内存恢复时的指针适应、
 *          以及对象删除后的空指针保护等功能。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFObjPtr: 共享内存对象智能指针模板类
 *       - 指向NFObject类的智能指针
 *       - 支持共享内存恢复时的自动适应
 *       - 对象删除后指针自动返回空
 *       - 提供安全的解引用操作
 * 
 * @warning 使用对象指针时需要注意：
 *          - 只能指向继承自NFObject的对象
 *          - 在共享内存恢复时会自动适应新的内存地址
 *          - 对象被删除后指针会自动失效
 *          - 使用前应该检查指针的有效性
 *          - 不要直接操作内部的原始指针
 */

#pragma once

#include "NFObject.h"
#include "NFShmMgr.h"

/**
 * @class NFObjPtr
 * @brief 共享内存对象智能指针模板类
 * @details 专门为共享内存环境设计的智能指针，指向继承自NFObject的对象。
 *          提供了对象序列号验证机制，确保指针指向的对象没有被删除。
 *          在共享内存恢复时会自动调整指针地址，支持进程重启后的指针有效性。
 * 
 * @tparam ObjType 对象类型，必须继承自NFObject
 * 
 * @note 主要特性：
 *       - 对象序列号验证：防止野指针问题
 *       - 共享内存地址自动适应：支持进程重启恢复
 *       - 安全的解引用操作：提供多种访问方式
 *       - STL兼容接口：支持标准的智能指针操作
 * 
 * @warning 使用注意事项：
 *          - 对象被删除后，指针会自动失效返回nullptr
 *          - 在使用指针前应该检查其有效性
 *          - 不要直接修改内部的m_pObj和m_iObjSeq成员
 */
template<class ObjType>
class NFObjPtr
{
private:
public:
    /**
     * @brief 带参数构造函数
     * @details 用指定的对象指针初始化智能指针，同时记录对象的序列号
     * @param pObj 要管理的对象指针，可以为nullptr
     */
    explicit NFObjPtr(ObjType *pObj)
        : m_pObj(pObj),
          m_iObjSeq(pObj ? pObj->m_iObjSeq : INVALID_ID)
    {
    }

    /**
     * @brief 默认构造函数
     * @details 根据共享内存管理器的创建模式选择初始化方式。
     *          在创建模式下会调用CreateInit()，在恢复模式下会调用ResumeInit()
     */
    explicit NFObjPtr()
    {
        if (NFShmMgr::Instance()->GetCreateMode() == EN_OBJ_MODE_INIT)
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 析构函数
     * @details 销毁智能指针，将内部指针置为0（注意：不会删除指向的对象）
     */
    ~NFObjPtr()
    {
        if (m_pObj)
        {
            m_pObj = 0;
        }
    }

    /**
     * @brief 创建时初始化
     * @details 第一次创建时的初始化，将指针和序列号都设为无效值
     */
    void CreateInit()
    {
        m_pObj = NULL;
        m_iObjSeq = INVALID_ID;
    }

    /**
     * @brief 恢复时初始化
     * @details 从共享内存恢复时的初始化，调整指针地址以适应新的内存布局
     */
    void ResumeInit()
    {
        if (m_pObj)
        {
            char *m_pChar = (char *) m_pObj;
            m_pChar += NFShmMgr::Instance()->GetAddrOffset();
            m_pObj = (ObjType *) m_pChar;
        }
    }

    /**
     * @brief 拷贝构造函数
     * @details 从另一个智能指针创建新的智能指针，复制对象指针和序列号
     * @param rT 要拷贝的智能指针
     */
    NFObjPtr(const NFObjPtr<ObjType> &rT)
    {
        m_pObj = (ObjType *) rT.GetPoint();
        m_iObjSeq = INVALID_ID;

        if (m_pObj)
        {
            m_iObjSeq = m_pObj->m_iObjSeq;
        }
    }

    /**
     * @brief 赋值操作符（智能指针版本）
     * @details 将另一个智能指针的值赋给当前指针
     * @param rT 要赋值的智能指针
     * @return const NFObjPtr<ObjType>& 返回当前对象的引用
     */
    const NFObjPtr<ObjType> &operator =(const NFObjPtr<ObjType> &rT)
    {
        m_pObj = (ObjType *) rT.GetPoint();
        if (m_pObj)
        {
            m_iObjSeq = m_pObj->m_iObjSeq;
        }
        else
        {
            m_iObjSeq = INVALID_ID;
        }

        return *this;
    }

    /**
     * @brief 赋值操作符（原始指针版本）
     * @details 将原始对象指针赋给智能指针
     * @param pT 要赋值的原始对象指针
     * @return const NFObjPtr<ObjType>& 返回当前对象的引用
     */
    const NFObjPtr<ObjType> &operator =(const ObjType *pT)
    {
        m_pObj = (ObjType *) pT;

        if (m_pObj)
        {
            m_iObjSeq = m_pObj->m_iObjSeq;
        }
        else
        {
            m_iObjSeq = INVALID_ID;
        }

        return *this;
    }

    /**
     * @brief 解引用操作符
     * @details 返回指向对象的引用，用于直接访问对象的成员
     * @return ObjType& 对象引用
     * @warning 使用前应确保指针有效，否则可能导致程序崩溃
     */
    ObjType &operator*()
    {
        return *GetPoint();
    }

    /**
     * @brief 解引用操作符（常量版本）
     * @details 返回指向对象的常量引用
     * @return const ObjType& 对象常量引用
     */
    const ObjType &operator*() const
    {
        return *GetPoint();
    }

    /**
     * @brief 成员访问操作符
     * @details 用于访问对象的成员函数和成员变量
     * @return ObjType* 对象指针
     */
    ObjType *operator->()
    {
        return GetPoint();
    }

    /**
     * @brief 成员访问操作符（常量版本）
     * @details 用于访问对象的成员函数和成员变量（常量版本）
     * @return const ObjType* 对象指针
     */
    const ObjType *operator->() const
    {
        return GetPoint();
    }

    /**
     * @brief 类型转换操作符（常量指针版本）
     * @details 将智能指针隐式转换为常量原始指针
     * @return const ObjType* 常量对象指针
     */
    operator const ObjType *() const
    {
        return GetPoint();
    }

    /**
     * @brief 类型转换操作符（普通指针版本）
     * @details 将智能指针隐式转换为原始指针
     * @return ObjType* 对象指针
     */
    operator ObjType *()
    {
        return GetPoint();
    }

    /**
     * @brief 重置指针为空
     * @details 将智能指针重置为空状态，不指向任何对象
     */
    void Reset()
    {
        if (NULL != m_pObj)
        {
            m_pObj = NULL;
        }
    }

    /**
     * @brief 重置指针指向新对象
     * @details 将智能指针重置为指向新的对象，并更新序列号
     * @param pT 新的对象指针
     */
    void Reset(ObjType *pT)
    {
        m_pObj = pT;

        if (m_pObj)
        {
            m_iObjSeq = m_pObj->m_iObjSeq;
        }
    }

public:
    /**
     * @brief 获取有效的对象指针
     * @details 检查对象序列号的有效性，如果对象已被删除则返回nullptr
     * @return ObjType* 有效的对象指针，如果对象已删除返回nullptr
     */
    ObjType *GetPoint()
    {
        if (!m_pObj)
        {
            return NULL;
        }

        if (m_pObj->m_iObjSeq == m_iObjSeq)
        {
            return m_pObj;
        }
        else
        {
            Reset();
        }

        return NULL;
    }

    /**
     * @brief 获取有效的对象指针（常量版本）
     * @details 检查对象序列号的有效性，如果对象已被删除则返回nullptr
     * @return const ObjType* 有效的对象指针，如果对象已删除返回nullptr
     */
    const ObjType *GetPoint() const
    {
        if (!m_pObj)
        {
            return NULL;
        }

        if (m_pObj->m_iObjSeq == m_iObjSeq)
        {
            return m_pObj;
        }
        else
        {
            const_cast<NFObjPtr<ObjType> *>(this)->Reset();
        }

        return NULL;
    }

    ObjType *m_pObj;        ///< 指向对象的原始指针
    int m_iObjSeq;          ///< 对象序列号，用于验证对象是否还有效
};

/**
 * @brief 智能指针相等比较操作符
 * @tparam ObjType 对象类型
 * @param _Left 左操作数
 * @param _Right 右操作数
 * @return bool true表示两个智能指针指向同一个对象
 */
template <class ObjType>
bool operator==(const NFObjPtr<ObjType>& _Left, const NFObjPtr<ObjType>& _Right) noexcept {
    return _Left.GetPoint() == _Right.GetPoint();
}

/**
 * @brief 智能指针不等比较操作符
 * @tparam ObjType 对象类型
 * @param _Left 左操作数
 * @param _Right 右操作数
 * @return bool true表示两个智能指针指向不同的对象
 */
template <class ObjType>
bool operator!=(const NFObjPtr<ObjType>& _Left, const NFObjPtr<ObjType>& _Right) noexcept {
    return _Left.GetPoint() != _Right.GetPoint();
}

/**
 * @brief 智能指针与nullptr相等比较操作符
 * @tparam ObjType 对象类型
 * @param _Left 智能指针
 * @param nullptr_t null指针
 * @return bool true表示智能指针为空
 */
template <class ObjType>
bool operator==(const NFObjPtr<ObjType>& _Left, nullptr_t) noexcept {
    return _Left.GetPoint() == nullptr;
}

/**
 * @brief nullptr与智能指针相等比较操作符
 * @tparam ObjType 对象类型
 * @param nullptr_t null指针
 * @param _Right 智能指针
 * @return bool true表示智能指针为空
 */
template <class ObjType>
bool operator==(nullptr_t, const NFObjPtr<ObjType>& _Right) noexcept {
    return nullptr == _Right.GetPoint();
}

/**
 * @brief 智能指针与nullptr不等比较操作符
 * @tparam ObjType 对象类型
 * @param _Left 智能指针
 * @param nullptr_t null指针
 * @return bool true表示智能指针不为空
 */
template <class ObjType>
bool operator!=(const NFObjPtr<ObjType>& _Left, nullptr_t) noexcept {
    return _Left.GetPoint() != nullptr;
}

/**
 * @brief nullptr与智能指针不等比较操作符
 * @tparam ObjType 对象类型
 * @param nullptr_t null指针
 * @param _Right 智能指针
 * @return bool true表示智能指针不为空
 */
template <class ObjType>
bool operator!=(nullptr_t, const NFObjPtr<ObjType>& _Right) noexcept {
    return nullptr != _Right.GetPoint();
}

/**
 * @class NFRawShmPtr
 * @brief 不校验对象序列号的共享内存指针
 * @details 简化版的共享内存指针，不进行对象序列号校验。
 *          适用于某些逻辑特性决定了指针不会发生变化的场景，
 *          性能比NFObjPtr稍好，但安全性较低。
 * 
 * @tparam ObjType 对象类型
 * 
 * @note 使用场景：
 *       - 指针生命周期明确且不会失效的情况
 *       - 对性能要求较高且能确保安全性的场景
 *       - 临时使用的指针，不需要长期持有
 * 
 * @warning 使用风险：
 *          - 不会检查对象是否已被删除
 *          - 可能产生野指针问题
 *          - 使用前需要确保对象仍然有效
 */
template<class ObjType>
class NFRawShmPtr
{
public:
    /**
     * @brief 带参数构造函数
     * @param pObj 要管理的对象指针
     */
    explicit NFRawShmPtr(ObjType *pObj)
        : m_pObj(pObj)
    {
    }

    /**
     * @brief 默认构造函数
     * @details 根据共享内存管理器的创建模式选择初始化方式
     */
    explicit NFRawShmPtr()
    {
        if (NFShmMgr::Instance()->GetCreateMode() == EN_OBJ_MODE_INIT)
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    /**
     * @brief 析构函数
     */
    ~NFRawShmPtr()
    {
        if (m_pObj)
        {
            m_pObj = 0;
        }
    }

    /**
     * @brief 创建时初始化
     */
    void CreateInit()
    {
        m_pObj = 0;
    }

    /**
     * @brief 恢复时初始化
     * @details 调整指针地址以适应共享内存的地址偏移
     */
    void ResumeInit()
    {
        if (m_pObj)
        {
            char *m_pChar = (char *) m_pObj;
            m_pChar += NFShmMgr::Instance()->GetAddrOffset();
            m_pObj = (ObjType *) m_pChar;
        }
    }

    /**
     * @brief 拷贝构造函数
     * @param rT 要拷贝的原始共享内存指针
     */
    NFRawShmPtr(const NFRawShmPtr<ObjType> &rT)
    {
        m_pObj = rT.GetPoint();
    }

    /**
     * @brief 赋值操作符（原始共享内存指针版本）
     * @param rT 要赋值的原始共享内存指针
     * @return const NFRawShmPtr<ObjType>& 返回当前对象的引用
     */
    const NFRawShmPtr<ObjType> &operator =(const NFRawShmPtr<ObjType> &rT)
    {
        m_pObj = rT.GetPoint();
        return *this;
    }

    /**
     * @brief 赋值操作符（原始指针版本）
     * @param pT 要赋值的原始对象指针
     * @return const NFRawShmPtr<ObjType>& 返回当前对象的引用
     */
    const NFRawShmPtr<ObjType> &operator =(const ObjType *pT)
    {
        m_pObj = (ObjType *) pT;
        return *this;
    }

    /**
     * @brief 解引用操作符
     * @return ObjType& 对象引用
     */
    ObjType &operator*()
    {
        return *GetPoint();
    }

    /**
     * @brief 解引用操作符（常量版本）
     * @return const ObjType& 对象常量引用
     */
    const ObjType &operator*() const
    {
        return *GetPoint();
    }

    /**
     * @brief 成员访问操作符
     * @return ObjType* 对象指针
     */
    ObjType *operator->()
    {
        return GetPoint();
    }

    /**
     * @brief 成员访问操作符（常量版本）
     * @return const ObjType* 对象指针
     */
    const ObjType *operator->() const
    {
        return GetPoint();
    }

    /**
     * @brief 与NFObjPtr的相等比较操作符
     * @param rT 要比较的NFObjPtr智能指针
     * @return bool true表示指向同一个对象
     */
    bool operator==(const NFObjPtr<ObjType> &rT) const
    {
        return GetPoint() == rT.GetPoint();
    }

    /**
     * @brief 与原始指针的相等比较操作符
     * @param pT 要比较的原始指针
     * @return bool true表示指向同一个对象
     */
    bool operator==(const ObjType *pT) const
    {
        return GetPoint() == pT;
    }

    /**
     * @brief 与原始指针的不等比较操作符
     * @param pT 要比较的原始指针
     * @return bool true表示指向不同的对象
     */
    bool operator!=(const ObjType *pT) const
    {
        return GetPoint() != pT;
    }

    /**
     * @brief 布尔类型转换操作符
     * @details 用于检查指针是否有效（非空）
     * @return bool true表示指针有效
     */
    operator bool() const
    {
        return 0 != GetPoint();
    }

    /**
     * @brief 类型转换操作符（常量指针版本）
     * @return const ObjType* 常量对象指针
     */
    operator const ObjType *() const
    {
        return GetPoint();
    }

    /**
     * @brief 类型转换操作符（普通指针版本）
     * @return ObjType* 对象指针
     */
    operator ObjType *()
    {
        return GetPoint();
    }

    /**
     * @brief 重置指针为空
     */
    void Reset()
    {
        if (0 != m_pObj)
        {
            m_pObj = 0;
        }
    }

    /**
     * @brief 重置指针指向新对象
     * @param pT 新的对象指针
     */
    void Reset(ObjType *pT)
    {
        m_pObj = pT;
    }

public:
    /**
     * @brief 获取对象指针
     * @details 直接返回内部指针，不进行任何有效性检查
     * @return ObjType* 对象指针
     */
    ObjType *GetPoint()
    {
        return m_pObj;
    }

    /**
     * @brief 获取对象指针（常量版本）
     * @details 直接返回内部指针，不进行任何有效性检查
     * @return const ObjType* 对象指针
     */
    const ObjType *GetPoint() const
    {
        return m_pObj;
    }

private:
    ObjType *m_pObj;    ///< 指向对象的原始指针
};
