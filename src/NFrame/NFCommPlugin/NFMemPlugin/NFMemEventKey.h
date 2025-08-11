// -------------------------------------------------------------------------
//    @FileName         :    NFMemEventKey
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFMemEventKey
//    @Desc             :    内存事件键值头文件，提供事件键值管理功能，包括事件键值的创建和初始化、
//                          事件键值的比较和哈希、事件键值的序列化。
//                          该文件定义了NFShmXFrame框架的内存事件键值类，提供事件键值管理、事件键值比较、
//                          事件键值哈希计算、事件键值字符串表示等功能。
//                          主要功能包括事件键值管理、事件键值比较、事件键值哈希计算、事件键值字符串表示
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
 * @brief 内存事件键值类，提供事件键值管理功能
 * 
 * 该类提供事件键值管理功能，包括事件键值的创建和初始化、
 * 事件键值的比较和哈希、事件键值的序列化。
 * 提供事件键值管理、事件键值比较、事件键值哈希计算、事件键值字符串表示等功能。
 */
class NFMemEventKey
{
public:
    /**
     * @brief 构造函数
     * 
     * 根据共享内存管理器的创建模式进行相应的初始化
     */
    NFMemEventKey()
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
     * @param eventKey 要比较的事件键值
     * @return 是否相等
     */
    bool operator==(const NFMemEventKey& eventKey) const
    {
        return m_serverType == eventKey.m_serverType &&
            m_eventId == eventKey.m_eventId &&
            m_srcType == eventKey.m_srcType &&
            m_srcId == eventKey.m_srcId;
    }

    /**
     * @brief 转换为字符串
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
 * 为NFMemEventKey类提供哈希函数，用于在哈希容器中使用
 */
namespace std
{
    template <>
    struct hash<NFMemEventKey>
    {
        size_t operator()(const NFMemEventKey& eventKey) const noexcept
        {
            return NFHash::hash_combine(eventKey.m_serverType, eventKey.m_eventId, eventKey.m_srcType, eventKey.m_srcId);
        }
    };
}
