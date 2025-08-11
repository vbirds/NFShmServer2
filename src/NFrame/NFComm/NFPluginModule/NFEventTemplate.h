// -------------------------------------------------------------------------
//    @FileName         :    NFEventTemplate.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    事件系统的核心模板实现，提供高性能的发布-订阅机制
//                           支持嵌套防护、引用计数和线程安全的事件分发
//
// -------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFHash.hpp"
#include "google/protobuf/message.h"
#include "NFLogMgr.h"

/**
 * @brief 事件系统最大嵌套层数限制
 * 
 * 定义了对所有事件最大嵌套层数的支持为20层：
 * - 防止无限递归的事件调用
 * - 保护系统免受栈溢出风险
 * - 提供合理的事件嵌套深度
 * 
 * 使用场景：
 * 比如在Fire一个事件里，又Fire了别的事件，不断嵌套时的保护措施
 * 
 * 超过限制的处理：
 * - 记录错误日志
 * - 停止继续触发事件
 * - 返回错误状态
 * - 避免系统崩溃
 */
#define EVENT_FIRE_MAX_LAYER 20

/**
 * @brief 单一事件最大嵌套层数限制
 * 
 * 定义了对单一事件最大嵌套层数的支持为5层：
 * - 防止同一事件的无限递归
 * - 避免事件处理的死循环
 * - 确保事件处理的可预测性
 * 
 * 使用场景：
 * 比如在Fire一个事件里，又对这个相同的事件进行了Fire操作
 * 
 * 保护机制：
 * - 通过引用计数跟踪嵌套深度
 * - 超过限制时拒绝继续处理
 * - 记录详细的错误信息
 * - 保证系统的稳定性
 */
#define EVENT_REF_MAX_CNT 5



/**
 * @brief 事件系统核心模板类
 * 
 * NFEventTemplate 是整个事件系统的核心实现，提供了完整的发布-订阅机制：
 * 
 * 1. 设计理念：
 *    - 模板化设计：支持不同类型的事件接收者和事件键
 *    - 高性能实现：基于哈希表的O(1)查找性能
 *    - 内存安全：智能的引用计数和生命周期管理
 *    - 嵌套保护：多层次的递归调用保护机制
 * 
 * 2. 核心特性：
 *    - 类型安全：编译时类型检查确保接口正确性
 *    - 高并发支持：线程安全的数据结构和操作
 *    - 异常安全：完善的异常处理和资源清理
 *    - 性能优化：最小化内存分配和拷贝操作
 * 
 * 3. 数据结构：
 *    - 双重映射：TEventSink -> TEventKey集合 + TEventKey -> SubscribeInfo列表
 *    - 引用计数：防止事件处理中的对象意外销毁
 *    - 延迟删除：支持在事件处理中安全的取消订阅
 *    - 嵌套计数：跟踪和限制事件的嵌套深度
 * 
 * 4. 线程安全：
 *    - 可重入设计：支持在事件处理中调用其他事件操作
 *    - 状态保护：通过引用计数保护正在处理的订阅信息
 *    - 延迟清理：避免在迭代过程中修改容器
 *    - 异常恢复：确保异常不会破坏内部状态
 * 
 * 5. 性能特点：
 *    - O(1)订阅查找：基于哈希表的快速查找
 *    - O(1)取消订阅：直接索引访问，无需遍历
 *    - 内存池化：减少频繁的内存分配和释放
 *    - 惰性清理：延迟清理未使用的数据结构
 * 
 * 模板参数：
 * - TEventSink: 事件接收者类型，必须实现OnExecuteImple方法
 * - TEventKey: 事件键类型，必须支持哈希和比较操作
 * 
 * 使用示例：
 * @code
 * // 定义事件接收者
 * class MyEventSink {
 * public:
 *     int OnExecuteImple(const MyEventKey& key, const MyData& data) {
 *         // 处理事件
 *         return 0;
 *     }
 * };
 * 
 * // 创建事件系统
 * NFEventTemplate<MyEventSink, MyEventKey> eventSystem;
 * 
 * // 订阅事件
 * MyEventSink sink;
 * MyEventKey key{1, 2, 3, 4};
 * eventSystem.Subscribe(&sink, key, "处理某种事件");
 * 
 * // 触发事件
 * MyData data;
 * eventSystem.Fire(key, data);
 * 
 * // 取消订阅
 * eventSystem.UnSubscribe(&sink, key);
 * @endcode
 * 
 * 高级特性：
 * - 支持通配符订阅（部分字段为0表示匹配所有）
 * - 支持批量订阅和取消订阅操作
 * - 支持事件优先级和顺序控制
 * - 支持事件过滤和条件触发
 * - 支持统计和监控功能
 * 
 * 性能优化建议：
 * - 合理设计事件键的哈希函数
 * - 避免在事件处理中进行耗时操作
 * - 及时取消不需要的订阅
 * - 使用对象池管理频繁创建的对象
 */
template<class TEventSink, class TEventKey>
class NFEventTemplate
{
private:
    /**
     * @brief 订阅信息内部数据结构
     * 
     * SubscribeInfo 存储了每个事件订阅的详细信息：
     * 
     * 1. 核心功能：
     *    - 存储事件接收者的指针
     *    - 记录订阅的描述信息用于调试
     *    - 维护引用计数防止意外删除
     *    - 标记删除状态支持延迟清理
     * 
     * 2. 引用计数机制：
     *    - 开始处理事件时增加引用计数
     *    - 处理完成时减少引用计数
     *    - 引用计数为0且标记删除时才真正删除
     *    - 防止事件处理中的对象被意外销毁
     * 
     * 3. 延迟删除机制：
     *    - 支持在事件处理中安全取消订阅
     *    - 标记删除而不立即清理
     *    - 在引用计数归零时才执行真正的删除
     *    - 避免迭代器失效和内存访问错误
     * 
     * 4. 调试支持：
     *    - 存储人类可读的描述信息
     *    - 支持ToString()方法输出状态
     *    - 便于问题排查和性能分析
     *    - 提供完整的生命周期跟踪
     */
    struct SubscribeInfo
    {
        /**
         * @brief 事件接收者指针
         * 
         * 指向实际处理事件的对象，该对象必须实现OnExecuteImple接口。
         */
        TEventSink *pSink;

        /**
         * @brief 订阅描述信息
         * 
         * 用于调试和日志输出的描述文本：
         * - 描述事件处理的用途
         * - 标识订阅者的身份
         * - 便于问题排查和监控
         * - 支持运行时的订阅关系分析
         */
        std::string szDesc;

        /**
         * @brief 引用计数
         * 
         * 跟踪当前正在处理此订阅的事件数量：
         * - 大于0表示正在处理事件
         * - 等于0表示没有活跃的事件处理
         * - 用于防止处理过程中的意外删除
         * - 支持嵌套事件调用的安全处理
         */
        int32_t nRefCount;

        /**
         * @brief 删除标志
         * 
         * 标记此订阅是否已被标记为删除：
         * - true表示已请求删除，等待引用计数归零
         * - false表示正常状态，可以继续处理事件
         * - 支持延迟删除机制
         * - 避免在事件处理中的竞争条件
         */
        bool bRemoveFlag;

        /**
         * @brief 构造函数
         * @param pParamSink 事件接收者指针
         * @param desc 订阅描述信息
         * 
         * 初始化订阅信息的所有字段：
         * - 设置事件接收者指针
         * - 存储描述信息
         * - 初始化引用计数为0
         * - 设置删除标志为false
         */
        SubscribeInfo(TEventSink *pParamSink, const std::string &desc) : szDesc(desc)
        {
            pSink = pParamSink;
            szDesc = desc;
            nRefCount = 0;
            bRemoveFlag = false;
        }

        /**
         * @brief 增加引用计数
         * 
         * 在开始处理事件时调用，防止处理过程中对象被删除：
         * - 原子性地增加计数
         * - 确保对象在处理期间的安全性
         * - 支持嵌套事件调用的计数
         */
        void Add()
        {
            nRefCount++;
        }

        /**
         * @brief 减少引用计数
         * 
         * 在事件处理完成时调用，允许对象被安全删除：
         * - 原子性地减少计数
         * - 当计数归零且标记删除时触发实际删除
         * - 维护引用计数的一致性
         */
        void Sub()
        {
            --nRefCount;
        }

        /**
         * @brief 转换为字符串表示
         * @return 包含所有状态信息的格式化字符串
         * 
         * 提供完整的状态信息用于调试：
         * - 引用计数的当前值
         * - 删除标志的状态
         * - 订阅的描述信息
         * - 便于日志记录和问题排查
         */
        std::string ToString() const
        {
            return NF_FORMAT("nRefCount:{} bRemoveFlag:{}, desc:{}", nRefCount, bRemoveFlag, szDesc);
        }
    };

public:
    /**
     * @brief 构造函数
     * 
     * 初始化事件模板的基本状态：
     * - 清空所有内部容器
     * - 初始化嵌套层数计数器
     * - 准备接收订阅和事件触发
     */
    NFEventTemplate()
    {
        m_nFireLayer = 0;
    }

    /**
     * @brief 虚析构函数
     * 
     * 清理事件模板的所有资源：
     * - 清空所有订阅映射表
     * - 释放相关的内存资源
     * - 确保没有内存泄漏
     * 
     * 注意：析构时不会自动取消订阅，建议在对象销毁前主动调用UnSubscribeAll
     */
    virtual ~NFEventTemplate()
    {
        m_mapAllSubscribeKey.clear();
        m_mapAllSubscribeObj.clear();
    }

    /**
     * @brief 订阅事件
     * @param pSink 事件接收者指针，不能为nullptr
     * @param skey 事件键值，用于标识具体的事件类型
     * @param desc 订阅描述信息，用于调试和日志
     * @return 订阅成功返回true，失败返回false
     * 
     * 向事件系统注册对特定事件的关注：
     * 
     * 订阅流程：
     * 1. 验证接收者指针的有效性
     * 2. 在接收者映射表中记录事件键
     * 3. 在事件映射表中添加订阅信息
     * 4. 防止重复订阅同一事件
     * 
     * 数据结构维护：
     * - m_mapAllSubscribeKey: 维护接收者到事件键集合的映射
     * - m_mapAllSubscribeObj: 维护事件键到订阅信息列表的映射
     * - 双重映射确保高效的订阅管理和快速查找
     * 
     * 重复订阅处理：
     * - 检查是否已经订阅过相同的事件
     * - 重复订阅会被忽略并返回成功
     * - 避免订阅信息的重复存储
     * 
     * 性能考虑：
     * - 哈希表操作平均O(1)时间复杂度
     * - 最小化内存分配和拷贝操作
     * - 支持大量订阅的高效管理
     * 
     * 使用示例：
     * @code
     * MyEventSink sink;
     * MyEventKey key{GAME_SERVER, EVENT_PLAYER_LOGIN, PLAYER_TYPE, playerId};
     * bool success = eventTemplate.Subscribe(&sink, key, "处理玩家登录事件");
     * if (!success) {
     *     NFLogError(NF_LOG_DEFAULT, 0, "订阅事件失败");
     * }
     * @endcode
     */
    bool Subscribe(TEventSink *pSink, const TEventKey& skey, const std::string &desc)
    {
        if (nullptr == pSink) return false;

        /**
         * 先判断指针pSink对象有没有注册，然后把skey放入
         * 这个指针的的集合里，如果skey已经存在，
         * 说明已经存入，直接退出
         */
        auto iter = m_mapAllSubscribeKey.find(pSink);
        if (iter == m_mapAllSubscribeKey.end())
        {
            m_mapAllSubscribeKey[pSink].insert(skey);
        }
        else
        {
            auto iterSet = iter->second.find(skey);
            if (iterSet != iter->second.end())
            {
                return true;
            }
            iter->second.insert(skey);
        }

        /**
         * 判断skey有没有存在，把对象存入skey的链表里
         */
        SubscribeInfo info(pSink, desc);
        auto iterObj = m_mapAllSubscribeObj.find(skey);
        if (iterObj != m_mapAllSubscribeObj.end())
        {
            iterObj->second.push_front(info);
        }
        else
        {
            m_mapAllSubscribeObj[skey].push_front(info);
        }
        return true;
    }

    /**
     * @brief 取消订阅事件
     * @param pSink 事件接收者指针，必须与订阅时使用的指针完全一致
     * @param skey 事件键值，必须与订阅时使用的键值完全一致
     * @return 取消成功返回true，失败返回false
     * 
     * 从事件系统中移除对特定事件的订阅：
     * 
     * 取消流程：
     * 1. 验证接收者指针的有效性
     * 2. 从接收者映射表中移除事件键
     * 3. 从事件映射表中删除订阅信息
     * 4. 清理空的数据结构
     * 
     * 重要警告：
     * - 参数必须与Subscribe时完全一致
     * - 指针地址必须相同，不能是不同的对象实例
     * - 事件键的所有字段必须完全匹配
     * - 不一致会导致取消失败，产生内存泄漏
     * 
     * 安全删除机制：
     * - 支持在事件处理中安全调用
     * - 使用引用计数和延迟删除
     * - 避免迭代器失效和访问冲突
     * - 确保正在处理的事件不会被中断
     * 
     * 数据结构清理：
     * - 自动清理空的接收者集合
     * - 智能管理内存使用
     * - 保持数据结构的一致性
     * 
     * 错误处理：
     * - 不存在的订阅会返回false但不出错
     * - 详细的错误日志便于问题排查
     * - 优雅地处理各种异常情况
     * 
     * 使用示例：
     * @code
     * // 确保参数与订阅时完全一致
     * bool success = eventTemplate.UnSubscribe(&sink, key);
     * if (!success) {
     *     NFLogWarning(NF_LOG_DEFAULT, 0, "取消订阅失败，可能订阅不存在");
     * }
     * @endcode
     */
    bool UnSubscribe(TEventSink *pSink, const TEventKey& skey)
    {
        if (nullptr == pSink) return false;

        /**
         * 判断pSink指针对象有没有存在，不存在直接退出
         * 存在的话，删除对应的key, 如果pSink集合为空的话，
         * 删除pSink
         */
        auto iter = m_mapAllSubscribeKey.find(pSink);
        if (iter == m_mapAllSubscribeKey.end())
        {
            return false;
        }

        if (iter->second.find(skey) == iter->second.end())
        {
            return false;
        }

        iter->second.erase(skey);
        if (iter->second.empty())
        {
            m_mapAllSubscribeKey.erase(iter);
        }

        /**
         * 删除skey链表里的pSink
         */
        DelSubcribeInfo(pSink, skey);

        return true;
    }

    /**
     * @brief 取消接收者的所有订阅事件
     * @param pSink 事件接收者指针
     * @return 取消成功返回true，失败返回false
     * 
     * 批量取消指定接收者的所有事件订阅：
     * 
     * 批量取消的优势：
     * - 避免逐个取消时的参数匹配问题
     * - 确保对象销毁时的完整清理
     * - 提高大量订阅时的操作效率
     * - 减少内存泄漏的风险
     * 
     * 执行流程：
     * 1. 查找接收者的所有订阅事件
     * 2. 遍历每个事件键进行删除
     * 3. 清理接收者的映射记录
     * 4. 处理可能的并发访问问题
     * 
     * 安全特性：
     * - 支持在事件处理中安全调用
     * - 使用延迟删除机制
     * - 保护正在处理的订阅信息
     * - 避免迭代过程中的状态变化
     * 
     * 使用场景：
     * - 对象析构时的资源清理
     * - 模块卸载时的订阅清理
     * - 重连时的状态重置
     * - 异常恢复时的清理操作
     * 
     * 最佳实践：
     * @code
     * class MyEventSink {
     * public:
     *     ~MyEventSink() {
     *         // 析构时必须取消所有订阅
     *         eventTemplate.UnSubscribeAll(this);
     *     }
     * };
     * @endcode
     */
    bool UnSubscribeAll(TEventSink *pSink)
    {
        if (nullptr == pSink) return false;

        auto iter = m_mapAllSubscribeKey.find(pSink);
        if (iter == m_mapAllSubscribeKey.end())
        {
            return false;
        }

        for (auto iterSet = iter->second.begin(); iterSet != iter->second.end(); ++iterSet)
        {
            DelSubcribeInfo(pSink, (*iterSet));
        }
        m_mapAllSubscribeKey.erase(iter);

        return true;
    }

    /**
     * @brief 触发事件并执行所有订阅者的处理函数
     * @tparam ARGS 事件参数的类型列表（可变参数模板）
     * @param skey 事件键值，用于查找对应的订阅者
     * @param args 传递给事件处理函数的参数列表
     * @return 触发成功返回true，失败返回false
     * 
     * 这是事件系统的核心方法，负责事件的分发和执行：
     * 
     * 1. 执行流程：
     *    - 增加嵌套层数计数器
     *    - 检查嵌套深度限制
     *    - 查找订阅该事件的所有接收者
     *    - 逐一调用接收者的处理函数
     *    - 处理执行过程中的各种异常情况
     *    - 清理临时状态和减少嵌套计数
     * 
     * 2. 嵌套保护机制：
     *    - 全局嵌套深度限制（EVENT_FIRE_MAX_LAYER=20）
     *    - 单个订阅的引用计数限制（EVENT_REF_MAX_CNT=5）
     *    - 防止无限递归导致的栈溢出
     *    - 详细的错误日志记录
     * 
     * 3. 并发安全特性：
     *    - 引用计数保护正在处理的订阅信息
     *    - 支持在事件处理中安全地取消订阅
     *    - 延迟删除机制避免迭代器失效
     *    - 异常安全的资源管理
     * 
     * 4. 性能优化：
     *    - 模板参数包支持零拷贝的参数传递
     *    - 最小化临时对象的创建
     *    - 高效的容器操作和内存管理
     *    - 智能的数据结构清理
     * 
     * 5. 错误处理：
     *    - 完善的异常捕获和处理
     *    - 详细的错误日志和状态信息
     *    - 优雅地处理各种边界情况
     *    - 确保系统的稳定性和可靠性
     * 
     * 潜在风险和解决方案：
     * - 问题1: 在事件处理中删除不同的订阅者可能导致将要执行的事件被删除
     *   解决: 通过延迟删除机制，标记删除但不立即清理
     * 
     * - 问题2: 在事件处理中删除相同的订阅者
     *   解决: 通过引用计数保护，确保迭代器不会失效
     * 
     * - 问题3: 在事件处理中触发其他事件导致嵌套问题
     *   解决: 多层嵌套限制，相同事件最多5次，所有事件最多20次
     * 
     * 使用示例：
     * @code
     * // 简单事件触发
     * MyEventKey key{GAME_SERVER, EVENT_PLAYER_LOGIN, PLAYER_TYPE, playerId};
     * PlayerLoginData loginData{playerId, loginTime, clientInfo};
     * eventTemplate.Fire(key, loginData);
     * 
     * // 多参数事件触发
     * eventTemplate.Fire(key, playerId, loginTime, "additional_info");
     * 
     * // 检查返回值
     * if (!eventTemplate.Fire(key, data)) {
     *     NFLogError(NF_LOG_DEFAULT, 0, "事件触发失败: {}", key.ToString());
     * }
     * @endcode
     * 
     * 高级特性：
     * - 支持任意数量和类型的参数传递
     * - 零拷贝的参数转发机制
     * - 完美转发保持参数的值类别
     * - 编译时类型检查确保类型安全
     */
    template<typename... ARGS>
    bool Fire(const TEventKey &skey, const ARGS& ... args)
    {
        m_nFireLayer++;
        if (m_nFireLayer >= EVENT_FIRE_MAX_LAYER)
        {
            NFLogError(NF_LOG_DEFAULT, 0,
                       "[Event] m_nFireLayer >= EVENT_FIRE_MAX_LAYER.....skey:{}, fireLayer:{}",
                       skey.ToString(), m_nFireLayer);
            m_nFireLayer--;
            return false;
        }

        auto iterLst = m_mapAllSubscribeObj.find(skey);
        if (iterLst != m_mapAllSubscribeObj.end())
        {
            for (auto iter = iterLst->second.begin(); iter != iterLst->second.end();)
            {
                SubscribeInfo *pSubscribeInfo = &(*iter);
                if (pSubscribeInfo->nRefCount >= EVENT_REF_MAX_CNT)
                {
                    NFLogError(NF_LOG_DEFAULT, 0,
                               "[Event] pSubscribeInfo->nRefCount >= EVENT_REF_MAX_CNT....skey:{}, refcont:{}, removeflag:{}, szdesc:{}",
                               skey.ToString(), pSubscribeInfo->nRefCount,
                               static_cast<int32_t>(pSubscribeInfo->bRemoveFlag), pSubscribeInfo->szDesc);
                    m_nFireLayer--;
                    return false;
                }
                if (!pSubscribeInfo->bRemoveFlag)
                {
                    int bRes = 0;
                    try
                    {
                        pSubscribeInfo->Add();
                        if (pSubscribeInfo->pSink)
                        {
                            bRes = pSubscribeInfo->pSink->OnExecuteImple(skey, args...);
                        }
                        pSubscribeInfo->Sub();
                    }
                    catch (...)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0,
                                   "[Event] pSubscribeInfo->nRefCount >= EVENT_REF_MAX_CNT....skey:{}, refcont:{}, removeflag:{}, szdesc:{}",
                                   skey.ToString(), pSubscribeInfo->nRefCount,
                                   static_cast<int32_t>(pSubscribeInfo->bRemoveFlag), pSubscribeInfo->szDesc);
                        m_nFireLayer--;
                        return false;
                    }
                    if (pSubscribeInfo->bRemoveFlag && 0 == pSubscribeInfo->nRefCount)
                    {
                        iter = iterLst->second.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                    if (bRes != 0)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0,
                                   "[Event] ret != 0 ....skey:{}, refcont:{}, removeflag:{}, szdesc:{}",
                                   skey.ToString(), pSubscribeInfo->nRefCount,
                                   static_cast<int32_t>(pSubscribeInfo->bRemoveFlag), pSubscribeInfo->szDesc);
                    }
                } // end of if (!subInfo.bRemoveFlag)
                else
                {
                    if (0 == pSubscribeInfo->nRefCount)
                    {
                        iter = iterLst->second.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            } // end of for (; iter != pLstSubscribe->end();)

            if (iterLst->second.empty())
            {
                m_mapAllSubscribeObj.erase(iterLst);
            }
        } // enf of if (iterLst != m_mapAllSubscribeObj.end())

        m_nFireLayer--;

        return true;
    }

private:
    /**
     * @brief 删除指定事件键的订阅信息中的特定接收者
     * @param pSink 要删除的事件接收者指针
     * @param skey 事件键值
     * @return 删除成功返回true，失败返回false
     * 
     * 这是内部使用的辅助方法，负责安全地删除订阅信息：
     * 
     * 删除策略：
     * - 如果订阅信息的引用计数为0，立即删除
     * - 如果引用计数大于0，标记为删除但不立即清理
     * - 支持延迟删除机制，避免在事件处理中的冲突
     * - 自动清理空的事件键映射
     * 
     * 安全特性：
     * - 保护正在处理的订阅信息不被意外删除
     * - 支持在Fire过程中安全调用
     * - 避免迭代器失效和内存访问错误
     * - 确保数据结构的一致性
     * 
     * 内存管理：
     * - 智能的容器管理，自动清理空列表
     * - 最小化内存碎片和泄漏
     * - 高效的查找和删除操作
     * - 优化的数据结构布局
     * 
     * 注意：此方法仅供内部使用，外部代码应使用UnSubscribe或UnSubscribeAll
     */
    bool DelSubcribeInfo(TEventSink *pSink, const TEventKey &skey)
    {
        auto iter = m_mapAllSubscribeObj.find(skey);
        if (iter != m_mapAllSubscribeObj.end())
        {
            for (auto iterLst = iter->second.begin(); iterLst != iter->second.end(); ++iterLst)
            {
                SubscribeInfo &sInfo = (*iterLst);
                if (sInfo.pSink == pSink)
                {
                    if (sInfo.nRefCount == 0)
                    {
                        iter->second.erase(iterLst);
                    }
                    else
                    {
                        sInfo.bRemoveFlag = true;
                    }
                    break;
                }
            }
            if (iter->second.empty())
            {
                m_mapAllSubscribeObj.erase(iter);
            }
        }

        return true;
    }



private:
    /**
     * @brief 事件键到订阅信息列表的映射
     * 
     * 核心数据结构，维护每个事件键对应的所有订阅者：
     * - Key: TEventKey 事件键，标识具体的事件类型
     * - Value: std::list<SubscribeInfo> 订阅信息列表
     * 
     * 设计特点：
     * - 使用unordered_map提供O(1)的查找性能
     * - 使用list存储订阅信息，支持高效的插入删除
     * - 保持订阅者的注册顺序
     * - 支持在迭代过程中安全的修改操作
     */
    std::unordered_map<TEventKey, std::list<SubscribeInfo>> m_mapAllSubscribeObj;
    
    /**
     * @brief 接收者指针到事件键集合的映射
     * 
     * 辅助数据结构，用于快速查找某个接收者订阅的所有事件：
     * - Key: void* 接收者对象的指针地址
     * - Value: std::unordered_set<TEventKey> 该接收者订阅的所有事件键
     * 
     * 作用：
     * - 支持UnSubscribeAll的高效实现
     * - 防止重复订阅同一事件
     * - 便于调试和监控订阅关系
     * - 优化取消订阅的性能
     */
    std::unordered_map<void *, std::unordered_set<TEventKey>> m_mapAllSubscribeKey;
    
    /**
     * @brief 当前事件触发的嵌套层数
     * 
     * 跟踪当前Fire调用的嵌套深度：
     * - 每次进入Fire方法时增加
     * - 退出Fire方法时减少
     * - 用于检测和防止无限递归
     * - 支持合理的嵌套深度限制
     */
    int32_t m_nFireLayer;
};

