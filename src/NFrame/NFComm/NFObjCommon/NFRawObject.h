// -------------------------------------------------------------------------
//    @FileName         :    NFRawObject.h
//    @Author           :    gaoyi
//    @Date             :    23-9-25
//    @Email			:    445267987@qq.com
//    @Module           :    NFRawObject
//
// -------------------------------------------------------------------------

/**
 * @file NFRawObject.h
 * @brief 原始对象类定义文件
 * @details 定义了NFrame框架中的原始对象类NFRawObject，用于封装和管理共享内存对象。
 *          提供了定时器功能的代理接口，允许非共享内存对象也能使用共享内存对象的
 *          定时器服务。这是一个适配器类，桥接普通对象和共享内存对象。
 * 
 * @author gaoyi
 * @date 23-9-25
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFRawObject: 原始对象适配器类
 *       - 共享内存对象的封装和管理
 *       - 定时器功能的代理接口
 *       - 支持各种类型的定时器（一次性、循环、日历等）
 *       - 对象生命周期管理
 * 
 * @warning 使用原始对象时需要注意：
 *          - 必须先调用InitShmObj()初始化共享内存对象
 *          - 定时器功能依赖于内部的共享内存对象
 *          - 对象销毁时需要清理所有定时器
 *          - 不要直接操作内部的共享内存对象指针
 */

#pragma once


#include <NFComm/NFPluginModule/NFCheck.h>

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFObjCommon/NFObject.h"
#include "NFComm/NFObjCommon/NFObjPtr.h"

/**
 * @class NFRawObject
 * @brief 原始对象适配器类
 * @details 用于封装和管理共享内存对象，为普通对象提供共享内存对象的服务。主要功能是提供
 *          定时器功能的代理接口，允许非共享内存对象也能使用共享内存对象的定时器服务。
 *          这是一个适配器类，桥接普通对象和共享内存对象。
 * 
 * @note 核心功能：
 *       - 共享内存对象的封装和管理
 *       - 定时器功能的代理接口
 *       - 支持各种类型的定时器（一次性、循环、日历等）
 *       - 对象生命周期管理
 *       - 自动资源清理
 * 
 * @warning 使用注意事项：
 *          - 必须先调用InitShmObj()初始化共享内存对象
 *          - 定时器功能依赖于内部的共享内存对象
 *          - 对象销毁时会自动清理所有定时器
 *          - 不要直接操作内部的共享内存对象指针
 */
class NFRawObject
{
public:
    /**
     * @brief 构造函数
     * @details 根据共享内存管理器的创建模式，选择调用CreateInit()或ResumeInit()
     */
    NFRawObject();

    /**
     * @brief 虚析构函数
     * @details 清理原始对象资源，自动删除所有关联的定时器
     * @note 析构时会调用DeleteAllTimer()清理所有定时器资源
     */
    virtual ~NFRawObject();

    /**
     * @brief 创建初始化函数
     * @details 当对象第一次创建时调用，初始化共享内存对象指针
     * @return int 初始化结果，0表示成功
     */
    int CreateInit();

    /**
     * @brief 恢复初始化函数
     * @details 当对象从共享内存恢复时调用
     * @return int 恢复结果，0表示成功
     */
    int ResumeInit();

public:
    /**
     * @brief 初始化共享内存对象
     * @details 绑定一个共享内存对象，后续的定时器功能将通过此对象提供
     * @param pShmObj 要绑定的共享内存对象指针
     * @return int 初始化结果，0表示成功，非0表示失败
     * @note 必须在使用定时器功能前调用此函数
     * @warning 传入的对象指针必须有效，且在原始对象生命周期内保持有效
     */
    int InitShmObj(const NFObject* pShmObj);
    
    /**
     * @brief 获取共享内存对象
     * @details 返回当前绑定的共享内存对象指针
     * @return NFObject* 共享内存对象指针，如果未绑定则返回nullptr
     * @note 返回的指针可能为空，使用前应该检查有效性
     */
    NFObject* GetShmObj();

public:
    /**
     * @brief 定时器回调函数
     * @details 当注册的定时器触发时调用此函数，默认实现返回0
     * @param timeId 定时器ID，用于标识具体的定时器
     * @param callCount 调用次数，对于循环定时器表示第几次调用
     * @return int 处理结果，0表示成功，非0表示失败
     * @note 这是虚函数，继承类可以重写以处理具体的定时器逻辑
     */
    virtual int OnTimer(int timeId, int callCount);

public:
    /**
     * @brief 删除指定的定时器
     * @details 根据定时器ID删除对应的定时器
     * @param timeObjId 要删除的定时器ID
     * @return int 删除结果，0表示成功，非0表示失败
     * @note 如果共享内存对象为空，返回0但不执行删除操作
     */
    virtual int DeleteTimer(int timeObjId)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->DeleteTimer(timeObjId);
    }

    /**
     * @brief 删除所有定时器
     * @details 删除当前对象关联的所有定时器
     * @return int 删除结果，0表示成功，-1表示共享内存对象无效
     * @note 通常在对象销毁时调用，确保清理所有定时器资源
     */
    virtual int DeleteAllTimer()
    {
        if (m_pShmObj)
        {
            return m_pShmObj->DeleteAllTimer(this);
        }
        return -1;
    }

    /**
     * @brief 注册一次性定时器（相对时间）
     * @details 注册一个在指定时间后执行一次的定时器
     * @param hour 小时数，距离现在多少小时后执行
     * @param minutes 分钟数，距离现在多少分钟后执行
     * @param second 秒数，距离现在多少秒后执行
     * @param microSec 毫秒数，距离现在多少毫秒后执行
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 总延迟时间 = hour*3600 + minutes*60 + second + microSec/1000（秒）
     */
    virtual int SetTimer(int hour, int minutes, int second, int microSec)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetTimer(hour, minutes, second, microSec, this);
    }

    /**
     * @brief 注册日历定时器（绝对时间点）
     * @details 注册一个在今天指定时间点执行一次的定时器
     * @param hour 小时，0-23
     * @param minutes 分钟，0-59
     * @param second 秒，0-59
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 如果指定时间已过，将在明天的该时间执行
     */
    virtual int SetCalender(int hour, int minutes, int second)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetCalender(hour, minutes, second, this);
    }

    /**
     * @brief 注册时间戳定时器（绝对时间戳）
     * @details 注册一个在指定时间戳时执行一次的定时器
     * @param timestamp 执行时间的时间戳（秒）
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 时间戳必须是未来的时间，否则可能立即执行
     */
    virtual int SetCalender(uint64_t timestamp)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetCalender(timestamp, this);
    }

    /**
     * @brief 注册循环定时器（相对时间开始）
     * @details 注册一个循环执行的定时器，首次在指定时间后执行
     * @param interval 循环间隔时间（毫秒）
     * @param callcount 执行次数，-1表示无限循环
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 首次延迟后，每隔interval毫秒执行一次
     */
    virtual int SetTimer(int interval, int callcount, int hour, int minutes, int second, int microSec)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetTimer(interval, callcount, hour, minutes, second, microSec, this);
    }

    /**
     * @brief 注册每日定时器（相对时间开始）
     * @details 注册一个每日执行的定时器，首次在指定时间后执行
     * @param callcount 执行天数，-1表示每天都执行
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 首次执行后，每隔24小时执行一次
     */
    virtual  int SetDayTime(int callcount, int hour, int minutes, int second, int microSec)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetDayTime(callcount, hour, minutes, second, microSec, this);
    }

    /**
     * @brief 注册每日日历定时器（绝对时间点）
     * @details 注册一个每日在指定时间点执行的定时器
     * @param callcount 执行天数，-1表示每天都执行
     * @param hour 执行的小时，0-23
     * @param minutes 执行的分钟，0-59
     * @param second 执行的秒，0-59
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 每天在指定的时间点执行，如23:30:00
     */
    virtual  int SetDayCalender(int callcount, int hour, int minutes, int second)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetDayCalender(callcount, hour, minutes, second, this);
    }

    /**
     * @brief 注册每周定时器（相对时间开始）
     * @details 注册一个每周执行的定时器，首次在指定时间后执行
     * @param callcount 执行周数，-1表示每周都执行
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 首次执行后，每隔7天执行一次
     */
    virtual int SetWeekTime(int callcount, int hour, int minutes, int second, int microSec)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetWeekTime(callcount, hour, minutes, second, microSec, this);
    }

    /**
     * @brief 注册每周日历定时器（绝对时间点）
     * @details 注册一个每周在指定星期和时间点执行的定时器
     * @param callcount 执行周数，-1表示每周都执行
     * @param weekDay 星期几，0-6（0表示星期日）
     * @param hour 执行的小时，0-23
     * @param minutes 执行的分钟，0-59
     * @param second 执行的秒，0-59
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 每周在指定的星期和时间点执行
     */
    virtual int SetWeekCalender(int callcount, int weekDay, int hour, int minutes, int second)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetWeekCalender(callcount, weekDay, hour, minutes, second, this);
    }

    /**
     * @brief 注册每月定时器（相对时间开始）
     * @details 注册一个每月执行的定时器，首次在指定时间后执行
     * @param callcount 执行月数，-1表示每月都执行
     * @param hour 首次执行距离现在的小时数
     * @param minutes 首次执行距离现在的分钟数
     * @param second 首次执行距离现在的秒数
     * @param microSec 首次执行距离现在的毫秒数
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 首次执行后，每隔30天执行一次（建议在同一天执行）
     */
    virtual int SetMonthTime(int callcount, int hour, int minutes, int second, int microSec)
    {
        CHECK_NULL(0, m_pShmObj);
        return m_pShmObj->SetMonthTime(callcount, hour, minutes, second, microSec, this);
    }

    /**
     * @brief 注册每月日历定时器（绝对时间点）
     * @details 注册一个每月在指定日期和时间点执行的定时器
     * @param callcount 执行月数，-1表示每月都执行
     * @param day 执行的日期，1-31
     * @param hour 执行的小时，0-23
     * @param minutes 执行的分钟，0-59
     * @param second 执行的秒，0-59
     * @return int 定时器ID，用于后续操作；失败返回0
     * @note 每月在指定的日期和时间点执行，如每月15号10点
     * @warning 对于没有指定日期的月份（如2月30日），行为未定义
     */
    virtual int SetMonthCalender(int callcount, int day, int hour, int minutes, int second)
    {
        CHECK_NULL(0, m_pShmObj.GetPoint());
        return m_pShmObj->SetMonthCalender(callcount, day, hour, minutes, second, this);
    }

protected:
    /**
     * @brief 共享内存对象智能指针
     * @details 指向绑定的共享内存对象，提供定时器等服务
     * @note 使用智能指针确保对象生命周期的安全管理
     */
    NFObjPtr<NFObject> m_pShmObj;
};