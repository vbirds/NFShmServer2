// -------------------------------------------------------------------------
//    @FileName         :    NFObject.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

/**
 * @file NFObject.h
 * @brief NFrame框架基础对象类定义文件
 * @details 定义了NFrame框架中所有对象的基类NFObject，提供了对象的基本生命周期管理、
 *          定时器功能、模块查找、事件处理、序列化等核心功能。所有框架内的对象都应该
 *          直接或间接继承自这个基类。
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFObject: 框架基础对象类
 *       - 对象生命周期管理（创建、初始化、销毁）
 *       - 定时器相关功能
 *       - 对象回收机制
 *       - 内存魔数检查（调试模式）
 * 
 * @warning 继承NFObject的类需要：
 *          - 实现CreateInit()和ResumeInit()方法
 *          - 在析构函数中正确清理资源
 *          - 注意对象的共享内存模式
 */

#pragma once

#include "NFObjDefine.h"
#include "NFTypeDefines.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFComm/NFObjCommon/NFObjectTemplate.h"

class NFRawObject;

/**
 * @class NFObject
 * @brief NFrame框架基础对象类
 * @details 所有框架内对象的基类，提供了对象的基本生命周期管理、定时器功能、模块查找、
 *          事件处理、序列化等核心功能。继承自NFObjectTemplateNoParent模板类，
 *          为整个框架提供统一的对象管理机制。
 * 
 * @note 核心功能：
 *       - 对象生命周期管理（创建、初始化、销毁）
 *       - 定时器系统（一次性、循环、日历定时器）
 *       - 事件系统（订阅、发布、广播）
 *       - 对象回收机制
 *       - 模块查找和访问
 *       - 内存完整性检查（调试模式）
 * 
 * @warning 继承注意事项：
 *          - 必须实现CreateInit()和ResumeInit()方法
 *          - 析构函数中应该清理所有资源
 *          - 注意对象的共享内存模式
 *          - 定时器回调中要检查对象有效性
 */
class NFObject : public NFObjectTemplateNoParent<NFObject, EOT_OBJECT>
{
public:
    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式，选择调用CreateInit()或ResumeInit()
     *          进行对象初始化。这种设计避免了在构造函数中调用虚函数的问题。
     * @note 构造过程：
     *       - 检查NFShmMgr的创建模式
     *       - EN_OBJ_MODE_INIT模式下调用CreateInit()
     *       - 其他模式下调用ResumeInit()
     */
    NFObject();

    /**
     * @brief 析构函数
     * @details 清理对象资源，包括删除所有相关定时器、取消事件订阅等
     * @note 析构过程会自动：
     *       - 删除所有注册的定时器
     *       - 取消所有事件订阅
     *       - 清理内部状态
     */
    ~NFObject() override;

    /**
     * @brief 对象创建时的初始化函数
     * @details 当对象第一次创建时调用，用于初始化对象的默认状态
     * @return int 初始化结果，0表示成功，非0表示失败
     * @note 这是非虚函数，不要添加virtual关键字
     * @warning 继承类必须实现此函数，并在其中初始化所有成员变量
     */
    int CreateInit();

    /**
     * @brief 对象从共享内存恢复时的初始化函数
     * @details 当对象从共享内存恢复时调用，用于重新建立对象的状态
     * @return int 恢复结果，0表示成功，非0表示失败
     * @note 这是非虚函数，不要添加virtual关键字
     * @warning 继承类必须实现此函数，并在其中恢复必要的运行时状态
     */
    int ResumeInit();

    /**
     * @brief 模块查找模板函数
     * @details 查找指定类型的模块对象，通过全局插件管理器获取
     * @tparam T 要查找的模块类型
     * @return T* 找到的模块指针，如果未找到则返回nullptr
     * @note 这是一个模板函数，支持任何继承自NFIModule的模块类型
     * @example
     * @code
     * auto* pDbModule = FindModule<NFIKernelModule>();
     * if (pDbModule) {
     *     // 使用模块
     * }
     * @endcode
     */
    template <typename T>
    T* FindModule() const
    {
        return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<T>();
    }

    /**
     * @brief 定时器回调函数
     * @details 当注册的定时器触发时调用此函数，必须是虚函数以支持多态
     * @param timeId 定时器ID，用于标识具体的定时器
     * @param callCount 调用次数，对于循环定时器表示第几次调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 这必须是虚函数，继承类可以重写以处理具体的定时器逻辑
     * @warning 在回调中要检查对象状态，避免在对象销毁后继续处理
     */
    virtual int OnTimer(int timeId, int callCount);

#ifdef NF_DEBUG_MODE
    /**
     * @brief 内存魔数检查函数（仅调试模式）
     * @details 检查对象内存的完整性，通过验证魔数来确保对象没有被破坏
     * @note 仅在NF_DEBUG_MODE宏定义时有效
     * @warning 如果魔数检查失败，说明对象内存可能已被破坏
     */
    void CheckMemMagicNum() const;
#endif

    /**
     * @brief 设置对象回收状态
     * @details 标记对象是否处于回收状态，回收状态的对象不应该被继续使用
     * @param bRet true表示设置为回收状态，false表示清除回收状态
     * @note 回收状态主要用于对象池管理，避免使用已回收的对象
     */
    void SetInRecycle(bool bRet);
    
    /**
     * @brief 检查对象是否处于回收状态
     * @details 检查当前对象是否已被标记为回收状态
     * @return bool true表示对象处于回收状态，false表示对象可正常使用
     * @note 在使用对象前应该检查此状态，避免使用已回收的对象
     */
    bool IsInRecycle() const;

    /**
     * @brief 获取对象ID
     * @details 返回对象的唯一标识符，用于在对象池中标识具体的对象实例
     * @return int 对象的ID，如果为INVALID_ID则表示无效对象
     * @note 对象ID在对象生命周期内保持不变
     */
    int GetObjId() const;

    /**
     * @brief 获取对象哈希键
     * @details 返回对象的哈希键，用于快速查找和索引
     * @return NFObjectHashKey 对象的哈希键值
     * @note 哈希键通常用于在哈希表中快速定位对象
     */
    NFObjectHashKey GetHashKey() const;
    
    /**
     * @brief 设置对象哈希键
     * @details 设置对象的哈希键，用于快速查找和索引
     * @param id 要设置的哈希键值
     * @note 哈希键应该是唯一的，避免冲突
     */
    void SetHashKey(NFObjectHashKey id);

    /**
     * @brief 获取全局ID
     * @details 返回对象的全局唯一标识符，跨服务器时保持唯一性
     * @return int 对象的全局ID
     * @note 全局ID在分布式系统中用于标识对象
     */
    int GetGlobalId() const;
    
    /**
     * @brief 设置全局ID
     * @details 设置对象的全局唯一标识符
     * @param iId 要设置的全局ID
     * @note 全局ID应该在分布式系统中保持唯一性
     */
    void SetGlobalId(int iId);

    /**
     * @brief 获取类型索引ID
     * @details 返回对象类型的索引ID，用于标识对象的具体类型
     * @return int 对象类型的索引ID
     * @note 类型索引ID用于对象类型识别和管理
     */
    int GetTypeIndexId() const;

    /**
     * @brief 获取杂项ID
     * @details 返回对象的杂项ID，用于特殊场景的对象标识
     * @return int 对象的杂项ID
     * @note 杂项ID的具体用途由具体的业务逻辑决定
     */
    int GetMiscId() const;

public:
    /**
     * @brief 获取类名称
     * @details 返回对象的类名称字符串，用于调试和日志记录
     * @return std::string 对象的类名称
     * @note 通常返回C++类的名称，用于运行时类型识别
     */
    virtual std::string GetClassName() const;
    
    /**
     * @brief 获取对象池中该类型的总数量
     * @details 返回对象池中当前类型对象的总容量
     * @return int 对象池中该类型的总数量
     * @note 包括已使用和未使用的对象总数
     */
    virtual int GetItemCount() const;
    
    /**
     * @brief 获取对象池中已使用的对象数量
     * @details 返回对象池中当前正在使用的对象数量
     * @return int 已使用的对象数量
     * @note 用于监控对象池的使用情况
     */
    virtual int GetUsedCount() const;
    
    /**
     * @brief 获取对象池中空闲的对象数量
     * @details 返回对象池中当前空闲可用的对象数量
     * @return int 空闲的对象数量
     * @note 等于总数量减去已使用数量
     */
    virtual int GetFreeCount() const;

public:
    /**
     * @brief 执行事件处理
     * @details 处理来自指定服务器的特定事件消息
     * @param serverType 发送事件的服务器类型
     * @param eventId 事件ID，标识具体的事件类型
     * @param srcType 源类型，标识事件来源的类型
     * @param srcId 源ID，标识事件来源的具体对象
     * @param pMessage 事件携带的消息数据
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 这是虚函数，继承类可以重写以处理特定的事件
     */
    virtual int OnExecute(uint32_t serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message* pMessage);
    
    /**
     * @brief 触发事件执行
     * @details 向指定服务器发送事件执行请求
     * @param serverType 目标服务器类型
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @param message 要发送的消息
     * @return int 发送结果，0表示成功，非0表示失败
     * @note 用于主动触发事件的执行
     */
    virtual int FireExecute(NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message);
    
    /**
     * @brief 广播事件到指定类型的服务器
     * @details 向指定类型的所有服务器广播事件消息
     * @param serverType 发送方服务器类型
     * @param recvServerType 接收方服务器类型
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @param message 要广播的消息
     * @param self 是否包含自己，默认为false
     * @return int 广播结果，0表示成功，非0表示失败
     * @note 会发送给所有指定类型的服务器实例
     */
    virtual int FireBroadcast(NF_SERVER_TYPE serverType, NF_SERVER_TYPE recvServerType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message, bool self = false);
    
    /**
     * @brief 广播事件到指定业务ID的服务器
     * @details 向指定业务ID和类型的服务器广播事件消息
     * @param serverType 发送方服务器类型
     * @param recvServerType 接收方服务器类型
     * @param busId 业务ID，用于进一步筛选目标服务器
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @param message 要广播的消息
     * @param self 是否包含自己，默认为false
     * @return int 广播结果，0表示成功，非0表示失败
     * @note 比普通广播更精确，可以指定具体的业务ID
     */
    virtual int FireBroadcast(NF_SERVER_TYPE serverType, NF_SERVER_TYPE recvServerType, uint32_t busId, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message, bool self = false);
    
    /**
     * @brief 向所有服务器广播事件
     * @details 向所有服务器广播事件消息，不区分服务器类型
     * @param serverType 发送方服务器类型
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @param message 要广播的消息
     * @param self 是否包含自己，默认为false
     * @return int 广播结果，0表示成功，非0表示失败
     * @note 这是最广泛的广播，会发送给所有连接的服务器
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message, bool self = false);
    
    /**
     * @brief 向指定业务ID的所有服务器广播事件
     * @details 向指定业务ID的所有服务器广播事件消息
     * @param serverType 发送方服务器类型
     * @param busId 业务ID
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @param message 要广播的消息
     * @param self 是否包含自己，默认为false
     * @return int 广播结果，0表示成功，非0表示失败
     * @note 可以向特定业务组的所有服务器广播
     */
    virtual int FireAllBroadcast(NF_SERVER_TYPE serverType, uint32_t busId, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message, bool self = false);

    /**
     * @brief 订阅事件
     * @details 订阅来自指定服务器和源的特定事件
     * @param serverType 服务器类型
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @param desc 订阅描述信息
     * @return int 订阅结果，0表示成功，非0表示失败
     * @note 订阅后，当匹配的事件发生时会调用OnExecute函数
     */
    virtual int Subscribe(NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const std::string& desc);
    
    /**
     * @brief 取消特定事件订阅
     * @details 取消对指定事件的订阅
     * @param serverType 服务器类型
     * @param eventId 事件ID
     * @param srcType 源类型
     * @param srcId 源ID
     * @return int 取消结果，0表示成功，非0表示失败
     * @note 取消后不再接收匹配的事件
     */
    virtual int UnSubscribe(NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId);
    
    /**
     * @brief 取消所有事件订阅
     * @details 取消当前对象的所有事件订阅
     * @return int 取消结果，0表示成功，非0表示失败
     * @note 通常在对象销毁时调用，确保不再接收任何事件
     */
    virtual int UnSubscribeAll();
public:
    /**
     * @brief 删除指定的定时器
     * @details 根据定时器ID删除对应的定时器
     * @param timeObjId 要删除的定时器ID
     * @return int 删除结果，0表示成功，非0表示失败
     * @note 删除后定时器将不再触发
     */
    virtual int DeleteTimer(int timeObjId);
    
    /**
     * @brief 删除当前对象的所有定时器
     * @details 删除当前对象注册的所有定时器
     * @return int 删除结果，0表示成功，非0表示失败
     * @note 通常在对象销毁时调用，确保清理所有定时器资源
     */
    virtual int DeleteAllTimer();
    
    /**
     * @brief 删除指定原始对象的所有定时器
     * @details 删除与指定原始对象关联的所有定时器
     * @param pRawShmObj 原始共享内存对象指针
     * @return int 删除结果，0表示成功，非0表示失败
     * @note 用于清理特定原始对象的定时器资源
     */
    virtual int DeleteAllTimer(NFRawObject* pRawShmObj);

    /**
     * @brief 注册一次性定时器（相对时间）
     * @details 注册一个在指定时间后执行一次的定时器
     * @param hour 小时数，距离现在多少小时后执行
     * @param minutes 分钟数，距离现在多少分钟后执行
     * @param second 秒数，距离现在多少秒后执行
     * @param microSec 毫秒数，距离现在多少毫秒后执行
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 总延迟时间 = hour*3600 + minutes*60 + second + microSec/1000（秒）
     * @example
     * @code
     * // 10秒后执行一次
     * int timerId = SetTimer(0, 0, 10, 0);
     * @endcode
     */
    virtual int SetTimer(int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册日历定时器（绝对时间点）
     * @details 注册一个在今天指定时间点执行一次的定时器
     * @param hour 小时，0-23
     * @param minutes 分钟，0-59
     * @param second 秒，0-59
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 如果指定时间已过，将在明天的该时间执行
     * @example
     * @code
     * // 今天15:30:00执行一次
     * int timerId = SetCalender(15, 30, 0);
     * @endcode
     */
    virtual int SetCalender(int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册时间戳定时器（绝对时间戳）
     * @details 注册一个在指定时间戳时执行一次的定时器
     * @param timestamp 执行时间的时间戳（秒）
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 时间戳必须是未来的时间，否则可能立即执行
     * @example
     * @code
     * // 在指定时间戳执行一次
     * uint64_t futureTime = time(nullptr) + 3600; // 1小时后
     * int timerId = SetCalender(futureTime);
     * @endcode
     */
    virtual int SetCalender(uint64_t timestamp, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册循环定时器（相对时间开始）
     * @details 注册一个循环执行的定时器，首次在指定时间后执行
     * @param interval 循环间隔时间（毫秒）
     * @param callcount 执行次数，-1表示无限循环
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 首次延迟后，每隔interval毫秒执行一次
     * @example
     * @code
     * // 10秒后开始，每5秒执行一次，共执行10次
     * int timerId = SetTimer(5000, 10, 0, 0, 10, 0);
     * @endcode
     */
    virtual int SetTimer(int interval, int callcount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册每日定时器（相对时间开始）
     * @details 注册一个每日执行的定时器，首次在指定时间后执行
     * @param callcount 执行天数，-1表示每天都执行
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 首次执行后，每隔24小时执行一次
     */
    virtual int SetDayTime(int callcount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册每日日历定时器（绝对时间点）
     * @details 注册一个每日在指定时间点执行的定时器
     * @param callcount 执行天数，-1表示每天都执行
     * @param hour 执行的小时，0-23
     * @param minutes 执行的分钟，0-59
     * @param second 执行的秒，0-59
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 每天在指定的时间点执行，如23:30:00
     * @example
     * @code
     * // 每天23点30分执行，共执行30天
     * int timerId = SetDayCalender(30, 23, 30, 0);
     * @endcode
     */
    virtual int SetDayCalender(int callcount, int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册每周定时器（相对时间开始）
     * @details 注册一个每周执行的定时器，首次在指定时间后执行
     * @param callcount 执行周数，-1表示每周都执行
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 首次执行后，每隔7天执行一次
     */
    virtual int SetWeekTime(int callcount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册每周日历定时器（绝对时间点）
     * @details 注册一个每周在指定星期和时间点执行的定时器
     * @param callcount 执行周数，-1表示每周都执行
     * @param weekDay 星期几，0-6（0表示星期日）
     * @param hour 执行的小时，0-23
     * @param minutes 执行的分钟，0-59
     * @param second 执行的秒，0-59
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 每周在指定的星期和时间点执行
     * @example
     * @code
     * // 每周一上午9点执行，共执行4周
     * int timerId = SetWeekCalender(4, 1, 9, 0, 0);
     * @endcode
     */
    virtual int SetWeekCalender(int callcount, int weekDay, int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册每月定时器（相对时间开始）
     * @details 注册一个每月执行的定时器，首次在指定时间后执行
     * @param callcount 执行月数，-1表示每月都执行
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 首次执行后，每隔30天执行一次（建议在同一天执行）
     */
    virtual int SetMonthTime(int callcount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr);

    /**
     * @brief 注册每月日历定时器（绝对时间点）
     * @details 注册一个每月在指定日期和时间点执行的定时器
     * @param callcount 执行月数，-1表示每月都执行
     * @param day 执行的日期，1-31
     * @param hour 执行的小时，0-23
     * @param minutes 执行的分钟，0-59
     * @param second 执行的秒，0-59
     * @param pRawShmObj 关联的原始对象，默认为nullptr
     * @return int 定时器ID，用于后续操作；失败返回负值
     * @note 每月在指定的日期和时间点执行，如每月15号10点
     * @warning 对于没有指定日期的月份（如2月30日），行为未定义
     * @example
     * @code
     * // 每月15号上午10点执行，共执行12个月
     * int timerId = SetMonthCalender(12, 15, 10, 0, 0);
     * @endcode
     */
    virtual int SetMonthCalender(int callcount, int day, int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr);
public:
    /**
     * @brief 对象唤醒钩子函数
     * @details 在对象系统初始化的早期阶段调用，主要用于单例对象的初始化
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 主要给单例类的对象使用，在系统启动时调用
     */
    virtual int Awake() { return 0; }
    
    /**
     * @brief 对象初始化钩子函数
     * @details 在对象唤醒后调用，用于对象的主要初始化工作
     * @return int 初始化结果，0表示成功，非0表示失败
     * @note 主要给单例类的对象使用，在Awake之后调用
     */
    virtual int Init() { return 0; }
    
    /**
     * @brief 对象关闭钩子函数
     * @details 在对象系统关闭时调用，用于清理资源
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 主要给单例类的对象使用，在系统关闭时调用
     */
    virtual int Shut() { return 0; }
    
    /**
     * @brief 对象最终化钩子函数
     * @details 在对象系统完全关闭前调用，用于最后的清理工作
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 主要给单例类的对象使用，在Shut之后调用
     */
    virtual int Finalize() { return 0; }
    
    /**
     * @brief 配置重载后钩子函数
     * @details 在配置文件重新加载完成后调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理配置变更后的逻辑
     */
    virtual int AfterOnReloadConfig() { return 0; }
    
    /**
     * @brief 服务器热修复钩子函数
     * @details 在服务器热修复时调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理热修复相关的逻辑
     */
    virtual int HotfixServer() { return 0; }
    
    /**
     * @brief 检查服务器停止条件钩子函数
     * @details 检查服务器是否满足停止条件
     * @return int 检查结果，0表示可以停止，非0表示不能停止
     * @note 用于优雅关闭服务器前的检查
     */
    virtual int CheckStopServer() { return 0; }
    
    /**
     * @brief 服务器停止钩子函数
     * @details 在服务器停止时调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理服务器停止时的清理工作
     */
    virtual int StopServer() { return 0; }
    
    /**
     * @brief 服务器被杀死钩子函数
     * @details 在服务器进程被强制终止时调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理进程被杀死时的紧急清理
     */
    virtual int OnServerKilling() { return 0; }
    
    /**
     * @brief 所有连接完成后钩子函数
     * @details 在所有服务器连接建立完成后调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理连接建立完成后的初始化逻辑
     */
    virtual int AfterAllConnectFinish() { return 0; }
    
    /**
     * @brief 所有描述存储加载完成后钩子函数
     * @details 在所有描述存储（配置数据）加载完成后调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理配置数据加载完成后的逻辑
     */
    virtual int AfterAllDescStoreLoaded() { return 0; }
    
    /**
     * @brief 所有连接和描述存储完成后钩子函数
     * @details 在所有连接和描述存储都完成后调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 综合了连接和配置加载的完成状态
     */
    virtual int AfterAllConnectAndAllDescStore() { return 0; }
    
    /**
     * @brief 数据库对象加载完成后钩子函数
     * @details 在从数据库加载对象完成后调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理数据库数据加载完成后的逻辑
     */
    virtual int AfterObjFromDBLoaded() { return 0; }
    
    /**
     * @brief 服务器同步数据完成后钩子函数
     * @details 在服务器间数据同步完成后调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理数据同步完成后的逻辑
     */
    virtual int AfterServerSyncData() { return 0; }
    
    /**
     * @brief 应用程序初始化完成后钩子函数
     * @details 在整个应用程序初始化完成后调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 用于处理应用程序完全启动后的逻辑
     */
    virtual int AfterAppInitFinish() { return 0; }
    
    /**
     * @brief 指定类型服务器连接完成后钩子函数
     * @details 在指定类型的服务器连接完成后调用
     * @param serverType 服务器类型
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 针对特定服务器类型的连接完成处理
     */
    virtual int AfterAllConnectFinish(NF_SERVER_TYPE serverType) { return 0; }
    
    /**
     * @brief 指定类型服务器描述存储加载完成后钩子函数
     * @details 在指定类型服务器的描述存储加载完成后调用
     * @param serverType 服务器类型
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 针对特定服务器类型的配置加载完成处理
     */
    virtual int AfterAllDescStoreLoaded(NF_SERVER_TYPE serverType) { return 0; }
    
    /**
     * @brief 指定类型服务器连接和描述存储完成后钩子函数
     * @details 在指定类型服务器的连接和描述存储都完成后调用
     * @param serverType 服务器类型
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 针对特定服务器类型的综合完成状态处理
     */
    virtual int AfterAllConnectAndAllDescStore(NF_SERVER_TYPE serverType) { return 0; }
    
    /**
     * @brief 指定类型服务器数据库对象加载完成后钩子函数
     * @details 在指定类型服务器的数据库对象加载完成后调用
     * @param serverType 服务器类型
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 针对特定服务器类型的数据库加载完成处理
     */
    virtual int AfterObjFromDBLoaded(NF_SERVER_TYPE serverType) { return 0; }
    
    /**
     * @brief 指定类型服务器同步数据完成后钩子函数
     * @details 在指定类型服务器的数据同步完成后调用
     * @param serverType 服务器类型
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 针对特定服务器类型的数据同步完成处理
     */
    virtual int AfterServerSyncData(NF_SERVER_TYPE serverType) { return 0; }
    
    /**
     * @brief 指定类型服务器应用程序初始化完成后钩子函数
     * @details 在指定类型服务器的应用程序初始化完成后调用
     * @param serverType 服务器类型
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 针对特定服务器类型的应用初始化完成处理
     */
    virtual int AfterAppInitFinish(NF_SERVER_TYPE serverType) { return 0; }

public:
#if NF_DEBUG_MODE
    /**
     * @brief 内存魔数检查数值（仅调试模式）
     * @details 用于检查对象内存完整性的魔数，仅在调试模式下有效
     * @note 值应该为OBJECT_MAGIC_CHECK_NUMBER，如果不匹配说明内存被破坏
     */
    int m_iMagicCheckNum;
#endif

    /**
     * @brief 全局ID
     * @details 对象的全局唯一标识符，在分布式系统中保持唯一性
     * @note 用于跨服务器的对象标识和引用
     */
    int m_iGlobalId;
    
    /**
     * @brief 对象ID
     * @details 对象在本地对象池中的唯一标识符
     * @note 用于在对象池中快速定位和管理对象
     */
    int m_iObjId;
    
    /**
     * @brief 对象序列号
     * @details 对象的序列号，用于验证对象的有效性
     * @note 在对象回收和重用时用于防止悬挂指针问题
     */
    int m_iObjSeq;
    
    /**
     * @brief 对象哈希键
     * @details 对象的哈希键值，用于在哈希表中快速查找
     * @note 通常根据对象的业务属性计算得出
     */
    NFObjectHashKey m_iHashKey;
    
    /**
     * @brief 对象类型
     * @details 标识对象的具体类型，对应EN_FRAME_SHMOBJ_TYPE枚举值
     * @note 用于运行时类型识别和对象管理
     */
    int m_iObjType;

    /**
     * @brief 对象回收状态标志
     * @details 标识对象是否处于回收状态
     * @note true表示对象已被回收，不应继续使用；false表示对象可正常使用
     */
    bool m_bIsInRecycle;
};

