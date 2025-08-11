// -------------------------------------------------------------------------
//    @FileName         :    NFNodeList.cpp
//    @Author           :    gaoyi
//    @Date             :    22-11-14
//    @Email			:    445267987@qq.com
//    @Module           :    NFNodeList
//
// -------------------------------------------------------------------------

/**
 * @file NFNodeList.cpp
 * @brief 共享内存链表实现文件
 * @details 实现了NFrame框架中共享内存链表的核心功能，主要包括链表节点对象的
 *          检查ID序列管理。为共享内存环境下的双向链表提供了基础的实现支持，
 *          确保链表操作的安全性和正确性。
 * 
 * @author gaoyi
 * @date 22-11-14
 * @email 445267987@qq.com
 * 
 * @note 实现的主要功能：
 *       - 链表检查ID序列的全局管理
 *       - 检查ID的递增和溢出处理
 *       - 链表节点的唯一性标识支持
 *       - 静态成员变量的初始化
 * 
 * @warning 使用注意事项：
 *          - 检查ID用于验证链表节点的有效性
 *          - 序列号溢出时会重置为1
 *          - 这是全局序列，所有链表共享
 *          - 不要手动修改检查ID序列
 */

#include "NFNodeList.h"

int NFIListNodeObj::m_iListCheckIdSeq = 0;

int NFIListNodeObj::GetListCheckIdSeq()
{
    m_iListCheckIdSeq++;

    if (m_iListCheckIdSeq <= 0)
    {
        m_iListCheckIdSeq = 1;
    }

    return m_iListCheckIdSeq;
}

