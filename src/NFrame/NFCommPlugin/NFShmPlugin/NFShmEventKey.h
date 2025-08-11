// -------------------------------------------------------------------------
//    @FileName         :    NFShmEventKey
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFShmEventKey
//    @Desc             :    共享内存事件键值头文件，提供事件键值管理功能。
//                          该文件定义了共享内存事件键值类，提供事件键值的创建和初始化、
//                          事件键值的比较和哈希、事件键值的序列化。
//                          主要功能包括事件键值管理、事件键值比较、事件键值哈希计算、
//                          事件键值字符串表示
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include <NFComm/NFCore/NFHash.hpp>
#include "NFComm/NFObjCommon/NFShmMgr.h"

#pragma pack(push)
#pragma pack(1)

/**
 * @brief 共享内存事件键值类
 * 
 * 用于标识和管理共享内存中的事件
 * 包含事件源ID、事件ID、源类型和服务器类型信息
 */
class NFShmEventKey
{
public:
    /**
     * @brief 构造函数
     * 
     * 根据共享内存模式进行初始化
     */
    NFShmEventKey()
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
     * 
     * 初始化所有成员变量为默认值
     * 
     * @return 初始化结果
     */
    int CreateInit()
    {
        m_srcId = 0;
        m_eventId = 0;
        m_srcType = 0;
        m_serverType = 0;
        return 0;
    }

    /**
     * @brief 恢复初始化
     * 
     * 从共享内存恢复对象状态
     * 
     * @return 初始化结果
     */
    int ResumeInit()
    {
        return 0;
    }

    /**
     * @brief 事件主要的key，主要指玩家，生物唯一id
     */
    uint64_t m_srcId;

    /**
     * @brief 事件Id
     */
    uint32_t m_eventId;

    /**
     * @brief src类型, 用来区别玩家，怪物的类型
     */
    uint32_t m_srcType;

    /**
     * @brief 服务器类型，用来区分AllServer模式下，不同服务器的事件
     */
    uint32_t m_serverType;

    /**
     * @brief 判断是否相等
     * 
     * 比较两个事件键值是否完全相等
     * 
     * @param eventKey 要比较的事件键值
     * @return 是否相等
     */
    bool operator==(const NFShmEventKey& eventKey) const
    {
        return m_serverType == eventKey.m_serverType &&
            m_eventId == eventKey.m_eventId &&
            m_srcType == eventKey.m_srcType &&
            m_srcId == eventKey.m_srcId;
    }

    /**
     * @brief 转换为字符串
     * 
     * 将事件键值转换为可读的字符串格式
     * 
     * @return 字符串表示
     */
    std::string ToString() const
    {
        return NF_FORMAT("nServerType:{} nEventID:{}, nSrcID:{}, bySrcType:{}", m_serverType, m_eventId, m_srcId, m_srcType);
    }
};

#pragma pack(pop)

/**
 * @brief 求hash值
 * 
 * 为NFShmEventKey提供哈希函数支持
 * 用于在哈希容器中存储事件键值
 */
namespace std
{
    template <>
    struct hash<NFShmEventKey>
    {
        /**
         * @brief 计算哈希值
         * 
         * 使用组合哈希算法计算事件键值的哈希值
         * 
         * @param eventKey 事件键值
         * @return 哈希值
         */
        size_t operator()(const NFShmEventKey& eventKey) const noexcept
        {
            return NFHash::hash_combine(eventKey.m_serverType, eventKey.m_eventId, eventKey.m_srcType, eventKey.m_srcId);
        }
    };
}
