// -------------------------------------------------------------------------
//    @FileName         :    NFObjectIterator.h
//    @Author           :    gaoyi
//    @Date             :    23-6-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFObjectIterator
//
// -------------------------------------------------------------------------

/**
 * @file NFObjectIterator.h
 * @brief 对象迭代器实现文件
 * @details 定义了NFrame框架中的对象迭代器模板类NFObjectIterator，提供了STL兼容的
 *          双向迭代器实现。支持对象容器的遍历访问，包括前向和后向迭代、
 *          解引用操作、迭代器比较等标准STL迭代器功能。
 *
 * @author gaoyi
 * @date 23-6-13
 * @email 445267987@qq.com
 *
 * @note 该文件包含了：
 *       - NFObjectIterator: 对象迭代器模板类
 *       - STL兼容的双向迭代器接口
 *       - 支持const和非const迭代器
 *       - 提供解引用和成员访问操作
 *       - 支持前缀和后缀递增递减操作
 *
 * @warning 使用迭代器时需要注意：
 *          - 迭代器依赖于容器的有效性
 *          - 在容器修改期间不要使用迭代器
 *          - 迭代器失效后需要重新获取
 *          - 注意迭代器的类型转换安全性
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"

class NFObject;

/**
 * @struct NFObjectIterator
 * @brief 对象迭代器模板类
 * @details 实现了STL兼容的双向迭代器，用于遍历对象容器。
 *          支持前向和后向迭代，提供解引用、成员访问、比较等标准迭代器操作。
 *          该迭代器与STL算法兼容，可以用于标准算法和范围遍历。
 *
 * @tparam Tp 迭代器指向的对象类型
 * @tparam Ref 引用类型，通常为Tp&或const Tp&
 * @tparam Ptr 指针类型，通常为Tp*或const Tp*
 * @tparam Container 容器类型，必须提供GetIterObj、IterIncr、IterDecr等方法
 *
 * @note 迭代器特性：
 *       - 迭代器类别：bidirectional_iterator_tag（双向迭代器）
 *       - 支持前缀和后缀的++、--操作
 *       - 支持解引用操作符*和->
 *       - 支持迭代器比较操作==、!=
 *       - 支持const和非const版本的迭代器
 *
 * @warning 迭代器使用注意事项：
 *          - 迭代器的有效性依赖于容器的存在
 *          - 容器结构改变时迭代器可能失效
 *          - 不要在迭代过程中修改容器结构
 *          - 确保不越过容器边界
 */
template<class Tp, class Ref, class Ptr, class Container>
struct NFObjectIterator
{
    typedef size_t size_type;                           ///< 大小类型定义
    typedef ptrdiff_t difference_type;                  ///< 差值类型定义
    typedef std::bidirectional_iterator_tag iterator_category; ///< 迭代器类别：双向迭代器

    typedef NFObjectIterator<Tp, Tp &, Tp *, Container> iterator;       ///< 普通迭代器类型
    typedef NFObjectIterator<Tp, const Tp &, const Tp *, Container> const_iterator; ///< 常量迭代器类型
    typedef NFObjectIterator self;                      ///< 自身类型定义

    typedef Tp value_type;      ///< 值类型定义
    typedef Ptr pointer;        ///< 指针类型定义
    typedef Ref reference;      ///< 引用类型定义

    Container *m_pContainer;    ///< 指向容器的指针
    int m_type;                 ///< 对象类型标识
    size_t m_pos;               ///< 当前迭代器位置

    /**
     * @brief 带参数的构造函数
     * @details 使用容器指针、对象类型和位置初始化迭代器
     * @param pContainer 容器指针，不能为nullptr
     * @param type 对象类型标识
     * @param pos 迭代器初始位置
     */
    explicit NFObjectIterator(const Container *pContainer, int type, size_t pos)
            : m_pContainer(const_cast<Container *>(pContainer)), m_type(type), m_pos(pos)
    {
    }

    /**
     * @brief 默认构造函数
     * @details 创建一个无效的迭代器，所有成员都初始化为默认值
     */
    NFObjectIterator() : m_pContainer(nullptr), m_type(0), m_pos(0) {}

    /**
     * @brief 拷贝构造函数（从普通迭代器）
     * @details 从同类型的迭代器创建新的迭代器
     * @param x 要拷贝的迭代器
     */
    NFObjectIterator(const iterator& x) : NFObjectIterator(x.m_pContainer, x.m_type, x.m_pos)
    {

    }

    /**
     * @brief 模板拷贝构造函数
     * @details 支持从兼容类型的迭代器创建新的迭代器，用于const转换等
     * @tparam Tpx 源迭代器的值类型
     * @param x 要拷贝的迭代器
     */
    template<class Tpx>
    NFObjectIterator(const NFObjectIterator<Tpx, Tpx&, Tpx*, Container>& x) : NFObjectIterator(x.m_pContainer, x.m_type, x.m_pos)
    {

    }

    /**
     * @brief 解引用操作符
     * @details 返回迭代器指向对象的引用，用于直接访问对象
     * @return reference 对象引用
     * @warning 使用前确保迭代器有效，否则可能导致程序崩溃
     */
    reference operator*() const { return *(dynamic_cast<pointer>(m_pContainer->GetIterObj(m_type, m_pos))); }

    /**
     * @brief 成员访问操作符
     * @details 返回迭代器指向对象的指针，用于访问对象的成员
     * @return pointer 对象指针
     * @warning 使用前确保迭代器有效
     */
    pointer operator->() const { return dynamic_cast<pointer>(m_pContainer->GetIterObj(m_type, m_pos)); }

    /**
     * @brief 获取对象指针
     * @details 直接返回迭代器指向的对象指针
     * @return pointer 对象指针，如果迭代器无效可能返回nullptr
     */
    pointer GetObj() const { return dynamic_cast<pointer>(m_pContainer->GetIterObj(m_type, m_pos)); }

    /**
     * @brief 前缀递增操作符
     * @details 将迭代器向前移动一个位置，返回移动后的迭代器引用
     * @return self& 移动后的迭代器引用
     */
    self& operator++()
    {
        this->Incr();
        return *this;
    }

    /**
     * @brief 后缀递增操作符
     * @details 将迭代器向前移动一个位置，返回移动前的迭代器副本
     * @param int 后缀递增标识参数（未使用）
     * @return self 移动前的迭代器副本
     */
    self operator++(int)
    {
        self tmp = *this;
        this->Incr();
        return tmp;
    }

    /**
     * @brief 前缀递减操作符
     * @details 将迭代器向后移动一个位置，返回移动后的迭代器引用
     * @return self& 移动后的迭代器引用
     */
    self& operator--()
    {
        this->Decr();
        return *this;
    }

    /**
     * @brief 后缀递减操作符
     * @details 将迭代器向后移动一个位置，返回移动前的迭代器副本
     * @param int 后缀递减标识参数（未使用）
     * @return self 移动前的迭代器副本
     */
    self operator--(int)
    {
        self tmp = *this;
        this->Decr();
        return tmp;
    }

    /**
     * @brief 迭代器递增实现
     * @details 通过容器的IterIncr方法将迭代器位置向前移动
     */
    void Incr() { m_pos = m_pContainer->IterIncr(m_type, m_pos); }

    /**
     * @brief 迭代器递减实现
     * @details 通过容器的IterDecr方法将迭代器位置向后移动
     */
    void Decr() { m_pos = m_pContainer->IterDecr(m_type, m_pos); }

    /**
     * @brief 迭代器相等比较操作符
     * @details 比较两个迭代器是否指向相同的容器、类型和位置
     * @param x 要比较的迭代器
     * @return bool true表示两个迭代器相等
     */
    bool operator==(const NFObjectIterator& x) const
    {
        return m_pContainer == x.m_pContainer && m_type == x.m_type && m_pos == x.m_pos;
    }

    /**
     * @brief 迭代器不等比较操作符
     * @details 比较两个迭代器是否指向不同的容器、类型或位置
     * @param x 要比较的迭代器
     * @return bool true表示两个迭代器不相等
     */
    bool operator!=(const NFObjectIterator& x) const
    {
        return !(m_pContainer == x.m_pContainer && m_type == x.m_type && m_pos == x.m_pos);
    }

    /**
     * @brief 迭代器赋值操作符
     * @details 将另一个迭代器的值赋给当前迭代器
     * @param x 要赋值的迭代器
     * @return NFObjectIterator& 当前迭代器的引用
     */
    NFObjectIterator& operator=(const NFObjectIterator& x)
    {
        if (this != &x)
        {
            m_pContainer = x.m_pContainer;
            m_type = x.m_type;
            m_pos = x.m_pos;
        }

        return *this;
    }
};