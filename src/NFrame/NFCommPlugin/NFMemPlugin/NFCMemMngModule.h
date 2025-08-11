// -------------------------------------------------------------------------
//    @FileName         :    NFCMemMngModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFKernelPlugin
//    @Desc             :    内存管理模块头文件，提供共享内存对象、定时器、事件、事务等统一管理功能。
//                          该文件定义了NFShmXFrame框架的内存管理模块，负责共享内存对象段的分配与管理、
//                          全局ID分配、定时器管理、事件管理、事务管理、对象生命周期管理、
//                          内存分配与释放、对象查找与遍历、定时器调度、事件处理、事务管理等。
//                          主要功能包括高效的内存管理、对象生命周期管理、定时器与事件统一调度、
//                          支持多种对象类型的注册与管理、自动资源回收、跨进程共享内存支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"


class NFMemGlobalId;
class NFTransBase;

class NFCMemMngModule;

class NFMemObjSeg;

/**
 * @brief 内存对象段交换计数器类，用于管理对象段的统计信息
 * 
 * 该类用于跟踪和管理内存对象段的统计信息，包括对象大小、数量、类型等。
 * 在对象类型反注册时用于释放相关资源，并提供对象段的生命周期管理。
 */
class NFMemObjSegSwapCounter
{
    friend class NFCMemMngModule;

public:
    /**
     * @brief 构造函数，初始化计数器对象
     */
    NFMemObjSegSwapCounter()
    {
        clear();
    }

    /**
     * @brief 设置对象段指针
     * @param pObjSeg 对象段指针
     */
    void SetObjSeg(NFMemObjSeg* pObjSeg);

    /**
     * @brief 清理计数器对象，重置所有成员变量为默认值
     * @note 用于对象类型反注册时释放相关资源
     */
    void clear()
    {
        m_nObjSize = 0;
        m_iItemCount = 0;
        m_iObjType = 0;
        m_iUseHash = false;
        m_singleton = false;
        m_pResumeFn = nullptr;
        m_pCreateFn = nullptr;
        m_pDestroyFn = nullptr;
        m_pObjSeg = nullptr;
        m_pParent = nullptr;
    }
public:
    std::string m_szClassName;        ///< 类名称
    size_t m_nObjSize;                ///< 对象大小
    int m_iItemCount;                 ///< 对象数量
    int m_iObjType;                   ///< 对象类型
    bool m_iUseHash;                  ///< 是否使用哈希表
    bool m_singleton;                 ///< 是否为单例对象
    NFMemObjSeg* m_pObjSeg;           ///< 对象段指针

    NFObject*(*m_pResumeFn)(void*);   ///< 恢复对象函数指针
    NFObject*(*m_pCreateFn)();        ///< 创建对象函数指针
    void (*m_pDestroyFn)(NFObject*);  ///< 销毁对象函数指针

    NFMemObjSegSwapCounter* m_pParent; ///< 父对象段计数器指针
    std::unordered_set<int> m_childrenObjType; ///< 子对象类型集合
    std::unordered_set<int> m_parentObjType;   ///< 父对象类型集合
};

/**
 * @brief 内存管理模块类，提供共享内存对象的统一管理功能
 * 
 * 该类是NFShmXFrame框架的核心内存管理模块，负责管理所有共享内存对象，
 * 包括对象段的分配与管理、全局ID分配、定时器管理、事件管理、事务管理等。
 * 提供高效的内存管理、对象生命周期管理、定时器与事件统一调度等功能。
 */
class NFCMemMngModule final : public NFIMemMngModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    explicit NFCMemMngModule(NFIPluginManager* p);

    /**
     * @brief 析构函数
     */
    ~NFCMemMngModule() override;

public:
    /**
     * @brief 在所有插件加载完成后调用
     * @return 执行结果
     */
    int AfterLoadAllPlugin() override;

    /**
     * @brief 模块唤醒函数
     * @return 执行结果
     */
    int Awake() override;

    /**
     * @brief 模块初始化函数
     * @return 执行结果
     */
    int Init() override;

    /**
     * @brief 模块心跳函数，处理定时器和事务
     * @return 执行结果
     */
    int Tick() override;

    /**
     * @brief 模块关闭函数
     * @return 执行结果
     */
    int Shut() override;

    /**
     * @brief 模块最终化函数
     * @return 执行结果
     */
    int Finalize() override;

    /**
     * @brief 配置重载后调用
     * @return 执行结果
     */
    int AfterOnReloadConfig() override;

    /**
     * @brief 热修复服务器
     * @return 执行结果
     */
    int HotfixServer() override;

    /**
     * @brief 检查服务器停止状态
     * @return 执行结果
     */
    int CheckStopServer() override;

    /**
     * @brief 停止服务器
     * @return 执行结果
     */
    int StopServer() override;

    /**
     * @brief 服务器被杀死时调用
     * @return 执行结果
     */
    int OnServerKilling() override;

    /**
     * @brief 所有连接完成时调用
     * @return 执行结果
     */
    int AfterAllConnectFinish() override;

    /**
     * @brief 所有描述存储加载完成时调用
     * @return 执行结果
     */
    int AfterAllDescStoreLoaded() override;

    /**
     * @brief 所有连接和描述存储加载完成时调用
     * @return 执行结果
     */
    int AfterAllConnectAndAllDescStore() override;

    /**
     * @brief 从数据库加载对象后调用
     * @return 执行结果
     */
    int AfterObjFromDBLoaded() override;

    /**
     * @brief 服务器同步数据后调用
     * @return 执行结果
     */
    int AfterServerSyncData() override;

    /**
     * @brief 应用初始化完成后调用
     * @return 执行结果
     */
    int AfterAppInitFinish() override;

    /**
     * @brief 服务器类型初始化完成后调用
     * @param serverType 服务器类型
     * @return 执行结果
     */
    int AfterAllConnectFinish(NF_SERVER_TYPE serverType) override;

    /**
     * @brief 服务器类型描述存储加载完成后调用
     * @param serverType 服务器类型
     * @return 执行结果
     */
    int AfterAllDescStoreLoaded(NF_SERVER_TYPE serverType) override;

    /**
     * @brief 服务器类型连接和描述存储加载完成后调用
     * @param serverType 服务器类型
     * @return 执行结果
     */
    int AfterAllConnectAndAllDescStore(NF_SERVER_TYPE serverType) override;

    /**
     * @brief 服务器类型从数据库加载对象后调用
     * @param serverType 服务器类型
     * @return 执行结果
     */
    int AfterObjFromDBLoaded(NF_SERVER_TYPE serverType) override;

    /**
     * @brief 服务器类型同步数据后调用
     * @param serverType 服务器类型
     * @return 执行结果
     */
    int AfterServerSyncData(NF_SERVER_TYPE serverType) override;

    /**
     * @brief 服务器类型应用初始化完成后调用
     * @param serverType 服务器类型
     * @return 执行结果
     */
    int AfterAppInitFinish(NF_SERVER_TYPE serverType) override;
public:

    /**
    * 共享内存模式
    */
    EN_OBJ_MODE GetInitMode() override;

    /**
    * 共享内存模式
    */
    void SetInitMode(EN_OBJ_MODE mode) override;

    /**
    * 共享内存创建对象模式
    */
    EN_OBJ_MODE GetCreateMode() override;

    /**
    * 共享内存创建对象模式
    */
    void SetCreateMode(EN_OBJ_MODE mode) override;

    EN_OBJ_MODE GetRunMode() override;

    /**
    * @brief  对象seq自增
    */
    int IncreaseObjSeqNum() override;

    /**
    * @brief  NFMemGlobalId
    */
    NFMemGlobalId* GetGlobalId() const;

    /**
    * @brief  设置功能内存初始化成功
    */
    void SetShmInitSuccessFlag() override;

    /**
     * @brief 删除指定定时器
     *
     * 该函数用于删除与指定对象关联的定时器。
     *
     * @param pObj 指向共享内存对象的指针，该对象与定时器相关联。
     * @param timeObjId 定时器的唯一标识符，用于指定要删除的定时器。
     * @return int 返回操作结果，通常为成功或失败的状态码。
     */
    int DeleteTimer(NFObject* pObj, int timeObjId) override;

    /**
     * @brief 删除所有与指定对象关联的定时器
     *
     * 该函数用于删除与指定共享内存对象关联的所有定时器。
     *
     * @param pObj 指向共享内存对象的指针，该对象与定时器相关联。
     * @return int 返回操作结果，通常为成功或失败的状态码。
     */
    int DeleteAllTimer(NFObject* pObj) override;

    /**
     * @brief 删除所有与指定对象和原始共享内存对象关联的定时器
     *
     * 该函数用于删除与指定共享内存对象和原始共享内存对象关联的所有定时器。
     *
     * @param pObj 指向共享内存对象的指针，该对象与定时器相关联。
     * @param pRawShmObj 指向原始共享内存对象的指针，该对象与定时器相关联。
     * @return int 返回操作结果，通常为成功或失败的状态码。
     */
    int DeleteAllTimer(NFObject* pObj, NFRawObject* pRawShmObj) override;

    /**
     * @brief 注册一个定时器，该定时器在距离当前时间指定的时分秒毫秒后执行一次。
     *
     * @param pObj 指向NFShmObj对象的指针，该对象将与定时器关联。
     * @param hour 距离当前时间的小时数。
     * @param minutes 距离当前时间的分钟数。
     * @param second 距离当前时间的秒数。
     * @param microSec 距离当前时间的毫秒数。
     * @param pRawShmObj 指向NFRawShmObj对象的指针，可选参数，默认为nullptr。
     * @return int 返回定时器注册的结果，通常为成功或失败的状态码。
     */
    int SetTimer(NFObject* pObj, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 注册一个定时器，该定时器在指定的时分秒时间点执行一次。
     *
     * @param pObj 指向NFShmObj对象的指针，该对象将与定时器关联。
     * @param hour 执行时间点的小时数。
     * @param minutes 执行时间点的分钟数。
     * @param second 执行时间点的秒数。
     * @param pRawShmObj 指向NFRawShmObj对象的指针，可选参数，默认为nullptr。
     * @return int 返回定时器注册的结果，通常为成功或失败的状态码。
     */
    int SetCalender(NFObject* pObj, int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 注册一个定时器，该定时器在指定的时间戳（秒为单位）执行一次。
     *
     * @param pObj 指向NFShmObj对象的指针，该对象将与定时器关联。
     * @param timestamp 执行时间点的时间戳，单位为秒。
     * @param pRawShmObj 指向NFRawShmObj对象的指针，可选参数，默认为nullptr。
     * @return int 返回定时器注册的结果，通常为成功或失败的状态码。
     */
    int SetCalender(NFObject* pObj, uint64_t timestamp, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 注册循环执行定时器
     *
     * 该函数用于注册一个循环执行的定时器，定时器的第一次执行时间由当前时间加上指定的时分秒毫秒确定，
     * 后续的执行间隔由指定的毫秒数决定。
     *
     * @param pObj 指向NFShmObj对象的指针，表示定时器关联的对象
     * @param interval 循环间隔时间，单位为毫秒
     * @param callCount 定时器调用的次数，如果为0则表示无限循环
     * @param hour 第一次执行距离当前时间的小时数
     * @param minutes 第一次执行距离当前时间的分钟数
     * @param second 第一次执行距离当前时间的秒数
     * @param microSec 第一次执行距离当前时间的毫秒数
     * @param pRawShmObj 指向NFRawShmObj对象的指针，默认为nullptr
     * @return int 返回定时器的ID或错误码
     */
    int SetTimer(NFObject* pObj, int interval, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 注册循环执行定时器
     *
     * 该函数用于注册一个循环执行的定时器，定时器的第一次执行时间由当前时间加上指定的时分秒毫秒确定，
     * 后续的执行间隔由系统默认值决定。
     *
     * @param pObj 指向NFShmObj对象的指针，表示定时器关联的对象
     * @param callCount 定时器调用的次数，如果为0则表示无限循环
     * @param hour 第一次执行距离当前时间的小时数
     * @param minutes 第一次执行距离当前时间的分钟数
     * @param second 第一次执行距离当前时间的秒数
     * @param microSec 第一次执行距离当前时间的毫秒数
     * @param pRawShmObj 指向NFRawShmObj对象的指针，默认为nullptr
     * @return int 返回定时器的ID或错误码
     */
    int SetDayTime(NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 注册某一个时间点日循环执行定时器
     *
     * 该函数用于注册一个每日在指定时间点执行的定时器，定时器将在每天的指定时分秒时刻执行。
     *
     * @param pObj 指向NFShmObj对象的指针，表示定时器关联的对象
     * @param callCount 定时器调用的次数，如果为0则表示无限循环
     * @param hour 每天开始执行的小时数
     * @param minutes 每天开始执行的分钟数
     * @param second 每天开始执行的秒数
     * @param pRawShmObj 指向NFRawShmObj对象的指针，默认为nullptr
     * @return int 返回定时器的ID或错误码
     */
    int SetDayCalender(NFObject* pObj, int callCount, int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 设置周循环定时器
     *
     * 该函数用于设置一个周循环定时器，定时器将在指定的时间间隔后首次执行，并在之后的每周同一时间重复执行。
     *
     * @param pObj 指向NFShmObj对象的指针，用于管理定时器的共享内存对象。
     * @param callCount 定时器的调用次数，表示定时器将执行的总次数。
     * @param hour 首次执行距离当前时间的小时数。
     * @param minutes 首次执行距离当前时间的分钟数。
     * @param second 首次执行距离当前时间的秒数。
     * @param microSec 首次执行距离当前时间的微秒数。
     * @param pRawShmObj 指向NFRawShmObj对象的指针，用于管理原始共享内存对象，默认为nullptr。
     * @return int 返回操作结果，通常为成功或错误码。
     */
    int SetWeekTime(NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 注册周循环定时器
     *
     * 该函数用于注册一个周循环定时器，定时器将在每周的指定时间点开始执行。
     *
     * @param pObj 指向NFShmObj对象的指针，用于管理定时器的共享内存对象。
     * @param callCount 定时器的调用次数，表示定时器将执行的总次数。
     * @param weekDay 定时器开始执行的星期几，范围为0-6（0表示周日，1表示周一，依此类推）。
     * @param hour 定时器开始执行的小时数。
     * @param minutes 定时器开始执行的分钟数。
     * @param second 定时器开始执行的秒数。
     * @param pRawShmObj 指向NFRawShmObj对象的指针，用于管理原始共享内存对象，默认为nullptr。
     * @return int 返回操作结果，通常为成功或错误码。
     */
    int SetWeekCalender(NFObject* pObj, int callCount, int weekDay, int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 设置月循环定时器
     *
     * 该函数用于设置一个月循环定时器，定时器将在指定的时间间隔后首次执行，并在之后的每月同一时间重复执行。
     *
     * @param pObj 指向NFShmObj对象的指针，用于管理定时器的共享内存对象。
     * @param callCount 定时器的调用次数，表示定时器将执行的总次数。
     * @param hour 首次执行距离当前时间的小时数。
     * @param minutes 首次执行距离当前时间的分钟数。
     * @param second 首次执行距离当前时间的秒数。
     * @param microSec 首次执行距离当前时间的微秒数。
     * @param pRawShmObj 指向NFRawShmObj对象的指针，用于管理原始共享内存对象，默认为nullptr。
     * @return int 返回操作结果，通常为成功或错误码。
     */
    int SetMonthTime(NFObject* pObj, int callCount, int hour, int minutes, int second, int microSec, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 注册一个月循环执行的定时器
     *
     * 该函数用于在每月的某一天、某一时间点注册一个定时器，定时器会在每月的该时间点触发执行。
     *
     * @param pObj 指向共享内存对象的指针，用于存储定时器相关信息。
     * @param callCount 定时器触发的次数，若为0则表示无限次触发。
     * @param day 一个月中的某一天，范围为1-31。
     * @param hour 一天中的某一小时，范围为0-23。
     * @param minutes 一小时中的某一分钟，范围为0-59。
     * @param second 一分钟中的某一秒，范围为0-59。
     * @param pRawShmObj 指向原始共享内存对象的指针，可选参数，默认为nullptr。
     * @return int 返回操作结果，成功返回0，失败返回错误码。
     */
    int SetMonthCalender(NFObject* pObj, int callCount, int day, int hour, int minutes, int second, NFRawObject* pRawShmObj = nullptr) override;

    /**
     * @brief 触发执行事件
     *
     * 该函数用于在指定服务器类型上触发一个事件，并传递相关的事件信息。
     *
     * @param serverType 服务器类型，表示事件触发的目标服务器。
     * @param eventId 事件ID，用于标识具体的事件。
     * @param srcType 事件源类型，表示事件的来源类型。
     * @param srcId 事件源ID，表示事件的具体来源。
     * @param message 事件消息，包含事件的具体信息，使用Google Protocol Buffers格式。
     * @return int 返回操作结果，成功返回0，失败返回错误码。
     */
    int FireExecute(NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const google::protobuf::Message& message) override;

    /**
     * @brief 订阅事件
     *
     * 该函数用于在指定服务器类型上订阅一个事件，并指定事件的来源和描述信息。
     *
     * @param pObj 指向共享内存对象的指针，用于存储订阅事件的相关信息。
     * @param serverType 服务器类型，表示订阅事件的目标服务器。
     * @param eventId 事件ID，用于标识具体的事件。
     * @param srcType 事件源类型，表示事件的来源类型。
     * @param srcId 事件源ID，表示事件的具体来源。
     * @param desc 事件描述信息，用于描述事件的具体内容。
     * @return int 返回操作结果，成功返回0，失败返回错误码。
     */
    int Subscribe(NFObject* pObj, NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId, const std::string& desc) override;

    /**
     * @brief 取消订阅指定的事件。
     *
     * 该函数用于取消订阅由 `pObj` 对象在 `serverType` 服务器类型下，针对特定事件 `eventId` 的订阅。
     * 订阅的源类型和源ID分别由 `srcType` 和 `srcId` 指定。
     *
     * @param pObj 指向共享内存对象的指针，表示订阅的主体。
     * @param serverType 服务器类型，表示事件订阅的服务器环境。
     * @param eventId 事件ID，表示要取消订阅的具体事件。
     * @param srcType 源类型，表示事件的来源类型。
     * @param srcId 源ID，表示事件的具体来源标识。
     * @return 返回操作结果，通常为成功或失败的状态码。
     */
    int UnSubscribe(NFObject* pObj, NF_SERVER_TYPE serverType, uint32_t eventId, uint32_t srcType, uint64_t srcId) override;

    /**
     * @brief 取消所有订阅。
     *
     * 该函数用于取消由 `pObj` 对象发起的所有事件订阅。
     *
     * @param pObj 指向共享内存对象的指针，表示订阅的主体。
     * @return 返回操作结果，通常为成功或失败的状态码。
     */
    int UnSubscribeAll(NFObject* pObj) override;

public:
    NFMemObjSeg* GetObjSeg(int iType) const;

    NFMemObjSegSwapCounter* GetObjSegSwapCounter(int iType);

    int GetItemCount(int iType) override;

    int GetUsedCount(int iType) override;

    int GetFreeCount(int iType) override;

    int GetGlobalId(int iType, int iIndex, NFObject* pObj) override;

    int GetObjId(int iType, NFObject* pObj) override;

    void* AllocMemForObject(int iType) override;

    void FreeMemForObject(int iType, void* pMem) override;

    NFObject* GetObjByObjId(int iType, int iIndex) override;

    NFObject* CreateObjByHashKey(int iType, NFObjectHashKey hashKey) override;

    NFObject* GetObjByHashKey(int iType, NFObjectHashKey hashKey) override;

    NFObject* CreateObj(int iType) override;

    NFObject* GetHeadObj(int iType) override;

    NFObject* GetNextObj(int iType, NFObject* pObj) override;

    void DestroyObj(NFObject* pObj) override;

    void ClearAllObj(int iType) override;

    const std::unordered_set<int>& GetChildrenType(int iType) override;

    int DestroyObjAutoErase(int iType, int maxNum = INVALID_ID, const DESTROY_OBJECT_AUTO_ERASE_FUNCTION& func = nullptr) override;

    NFObject* GetObjByGlobalId(int iType, int iGlobalId, bool withChildrenType = false) override;

    NFObject* GetObjByGlobalIdWithNoCheck(int iGlobalId) override;

    // 根据混合ID获得对象
    // iType不为-1表示校验对象类型
    NFObject* GetObjByMiscId(int iMiscId, int iType = -1) override;

    bool IsEnd(int iType, int iIndex) override;

    void SetSecOffSet(int iOffset) override;

    int GetSecOffSet() const override;

    /**
     * @brief ShmObj类链表迭代器+1
     * @param iType
     * @param iPos
     * @return
     */
    size_t IterIncr(int iType, size_t iPos) override;

    /**
     * @brief ShmObj类链表迭代器-1
     * @param iType
     * @param iPos
     * @return
     */
    size_t IterDecr(int iType, size_t iPos) override;

    iterator IterBegin(int iType) override;

    iterator IterEnd(int iType) override;

    const_iterator IterBegin(int iType) const override;

    const_iterator IterEnd(int iType) const override;

    iterator Erase(iterator iter) override;

    bool IsValid(iterator iter) override;

    NFObject* GetIterObj(int iType, size_t iPos) override;

    const NFObject* GetIterObj(int iType, size_t iPos) const override;

    bool IsTypeValid(int iType) const override;

    NFTransBase* CreateTrans(int iType) override;

    NFTransBase* GetTrans(uint64_t ullTransId) override;

public:
    std::string GetClassName(int bType) override;

    int GetClassType(int bType) override;

public:
    NFMemObjSegSwapCounter* CreateCounterObj(int bType);

    int InitAllObjSeg();

    void RegisterClassToObjSeg(int bType, size_t nObjSize, int iItemCount, NFObject*(*pfResumeObj)(void*),
                               NFObject*(*pCreateFn)(),
                               void (*pDestroy)(NFObject*), int parentType, const std::string& pszClassName,
                               bool useHash = false, bool singleton = false) override;

    void UnRegisterClassToObjSeg(int bType) override;

    size_t GetAllObjSize() const;

    int InitializeAllObj();

    int InitMemObjectGlobal();

    int InitSpecialMemObj();

private:
    size_t m_iObjSegSizeTotal;
    int m_iTotalObjCount;
    std::vector<NFMemObjSegSwapCounter> m_nObjSegSwapCounter;
    EN_OBJ_MODE m_enRunMode;
    EN_OBJ_MODE m_enCreateMode; //创建对象模式
    size_t m_siShmSize;
    NFMemGlobalId* m_pGlobalId;
};
