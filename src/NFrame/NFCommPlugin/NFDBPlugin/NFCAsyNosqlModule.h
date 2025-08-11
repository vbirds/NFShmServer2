// -------------------------------------------------------------------------
//    @FileName         :    NFCAsyNosqlModule.h
//    @Author           :    gaoyi
//    @Date             :    23-8-15
//    @Email            :    445267987@qq.com
//    @Module           :    NFCAsyNosqlModule
//    @Desc             :    异步NoSQL模块头文件，提供异步NoSQL数据库操作接口。
//                          该文件定义了NFShmXFrame框架的异步NoSQL模块，继承自NFIAsyNosqlModule接口，
//                          提供异步的NoSQL数据库操作功能，采用Actor模式实现异步操作，
//                          避免阻塞主线程，提高系统并发性能，主要针对Redis等NoSQL数据库的异步操作。
//                          主要功能包括异步NoSQL操作查询插入更新删除对象、Actor池管理异步任务执行、
//                          回调机制操作完成后通过回调函数通知结果、多服务器支持可连接多个NoSQL服务器、
//                          高并发非阻塞式操作提高系统性能
//    @Description      :    异步NoSQL模块头文件，提供异步NoSQL数据库操作接口
//
// -------------------------------------------------------------------------

#pragma once

#include "NFCMysqlDriverManager.h"
#include "NFComm/NFPluginModule/NFIAsyNosqlModule.h"

/**
 * @class NFCAsyNosqlModule
 * @brief 异步NoSQL模块实现类
 * 
 * 继承自NFIAsyNosqlModule接口，提供异步的NoSQL数据库操作功能。
 * 采用Actor模式实现异步操作，避免阻塞主线程，提高系统并发性能。
 * 主要针对Redis等NoSQL数据库的异步操作。
 * 
 * 主要功能：
 * - 异步NoSQL操作：查询、插入、更新、删除对象
 * - Actor池管理：管理异步任务的执行
 * - 回调机制：操作完成后通过回调函数通知结果
 * - 多服务器支持：可连接多个NoSQL服务器
 * - 高并发：非阻塞式操作提高系统性能
 */
class NFCAsyNosqlModule final : public NFIAsyNosqlModule
{
public:
    /**
     * @brief NFCAsyNosqlModule 构造函数
     * @param pPluginManager NFIPluginManager 指针，用于插件管理和模块依赖关系
     */
    explicit NFCAsyNosqlModule(NFIPluginManager* pPluginManager);

    /**
     * @brief NFCAsyNosqlModule 析构函数
     * 用于清理资源，释放内存和关闭数据库连接
     */
    ~NFCAsyNosqlModule() override;

    /**
     * @brief 执行模块的主要逻辑
     * @return 返回执行结果，0表示成功，非0表示失败
     * 
     * 定期执行模块维护任务，如处理异步任务队列、检查连接状态等
     */
    int Tick() override;

    /**
     * @brief 初始化 Actor 池
     * @param iMaxTaskGroup 最大任务组数，用于控制并发任务的组织
     * @param iMaxActorNum 最大 Actor 数量，默认为 0（自动设置）
     * @return 返回初始化结果，true表示成功，false表示失败
     * 
     * Actor池用于管理异步任务的执行，提高并发处理能力
     */
    bool InitActorPool(int iMaxTaskGroup, int iMaxActorNum = 0) override;

    /**
     * @brief 添加NoSQL数据库服务器
     * @param strServerId 服务器唯一标识符
     * @param strNoSqlIp NoSQL 数据库的 IP 地址
     * @param iNosqlPort NoSQL 数据库的端口号
     * @param strNoSqlPass NoSQL 数据库的认证密码
     * @return 返回操作结果，0表示成功，非0表示失败
     * 
     * 添加新的NoSQL服务器连接配置，支持多服务器管理
     */
    int AddDBServer(const std::string& strServerId, const std::string& strNoSqlIp, int iNosqlPort, const std::string& strNoSqlPass) override;

    /**
     * @brief 异步查询对象
     * @param strServerId 服务器唯一标识符
     * @param stSelect 查询条件结构体
     * @param fCallback 查询完成后的回调函数
     * @return 返回操作结果，0表示成功提交，非0表示失败
     * 
     * 异步执行对象查询操作，结果通过回调函数返回
     */
    int SelectObj(const std::string& strServerId, const NFrame::storesvr_selobj& stSelect, const SelectObjCb& fCallback) override;

    /**
     * @brief 异步删除对象
     * @param strServerId 服务器唯一标识符
     * @param stSelect 删除条件结构体
     * @param fCallback 删除完成后的回调函数
     * @return 返回操作结果，0表示成功提交，非0表示失败
     * 
     * 异步执行对象删除操作，结果通过回调函数返回
     */
    int DeleteObj(const std::string& strServerId, const NFrame::storesvr_delobj& stSelect, const DeleteObjCb& fCallback) override;

    /**
     * @brief 异步插入对象
     * @param strServerId 服务器唯一标识符
     * @param stSelect 插入数据结构体
     * @param bCallback 插入完成后的回调函数
     * @return 返回操作结果，0表示成功提交，非0表示失败
     * 
     * 异步执行对象插入操作，结果通过回调函数返回
     */
    int InsertObj(const std::string& strServerId, const NFrame::storesvr_insertobj& stSelect, const InsertObjCb& bCallback) override;

    /**
     * @brief 异步修改对象
     * @param strServerId 服务器唯一标识符
     * @param stSelect 修改条件和数据结构体
     * @param fCallback 修改完成后的回调函数
     * @return 返回操作结果，0表示成功提交，非0表示失败
     * 
     * 异步执行对象修改操作，结果通过回调函数返回
     */
    int ModifyObj(const std::string& strServerId, const NFrame::storesvr_modobj& stSelect, const ModifyObjCb& fCallback) override;

    /**
     * @brief 异步更新对象
     * @param strServerId 服务器唯一标识符
     * @param stSelect 更新条件和数据结构体
     * @param fCallback 更新完成后的回调函数
     * @return 返回操作结果，0表示成功提交，非0表示失败
     * 
     * 异步执行对象更新操作，结果通过回调函数返回
     */
    int UpdateObj(const std::string& strServerId, const NFrame::storesvr_updateobj& stSelect, const UpdateObjCb& fCallback) override;

private:
    /**
     * @brief 最后一次检查时间戳
     * 用于记录上次执行定期检查操作的时间，控制检查频率
     */
    uint64_t m_ullLastCheckTime;
    
    /**
     * @brief 组件初始化状态标志
     * 标识模块是否已经完成初始化，防止重复初始化
     */
    bool m_bInitComponent;
};
