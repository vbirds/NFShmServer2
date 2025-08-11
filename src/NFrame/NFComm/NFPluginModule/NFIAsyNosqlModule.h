// -------------------------------------------------------------------------
//    @FileName         :    NFIAsyNosqlModule.h
//    @Author           :    gaoyi
//    @Date             :    23-8-15
//    @Email			:    445267987@qq.com
//    @Module           :    NFIAsyNosqlModule
//    @Description      :    异步NoSQL数据库操作接口，提供非阻塞的NoSQL数据库访问
//                           继承异步模块框架，支持Redis等NoSQL数据库的高性能操作
//
// -------------------------------------------------------------------------

#pragma once

#include "NFIAsyModule.h"

/**
 * @brief 异步NoSQL数据库操作接口类
 * 
 * NFIAsyNosqlModule 提供了异步NoSQL数据库操作的统一接口：
 * 
 * 1. 设计理念：
 *    - 异步非阻塞：所有数据库操作都是异步执行，不阻塞主线程
 *    - 继承架构：继承NFIAsyModule，复用异步Actor框架
 *    - 统一接口：为不同NoSQL数据库提供统一的操作接口
 *    - 高性能：针对NoSQL数据库特点进行的性能优化
 * 
 * 2. 支持的NoSQL数据库：
 *    - Redis：主要支持的NoSQL数据库
 *    - MongoDB：可扩展支持
 *    - Memcached：可扩展支持
 *    - 其他键值存储：通过插件扩展
 * 
 * 3. 核心功能：
 *    - 数据库连接管理：支持多个数据库服务器连接
 *    - 对象操作：针对结构化对象的CRUD操作
 *    - 异步回调：完整的异步操作和回调机制
 *    - 连接池：高效的连接池管理
 *    - 错误处理：完善的错误处理和重连机制
 * 
 * 4. 操作类型：
 *    - SelectObj：查询对象数据
 *    - DeleteObj：删除对象数据
 *    - InsertObj：插入新对象
 *    - ModifyObj：修改现有对象
 *    - UpdateObj：更新对象数据
 * 
 * 5. 异步特性：
 *    - 非阻塞操作：所有数据库操作都是异步执行
 *    - 回调机制：操作完成后通过回调函数返回结果
 *    - 并发控制：支持高并发的数据库访问
 *    - 负载均衡：在多个数据库实例间分布负载
 * 
 * 6. 性能优化：
 *    - 连接复用：高效的连接池管理
 *    - 批量操作：支持批量数据库操作
 *    - 缓存机制：智能的查询结果缓存
 *    - 压缩传输：数据传输的压缩和优化
 * 
 * 使用场景：
 * - 游戏数据缓存：玩家数据、物品数据的快速访问
 * - 会话存储：用户会话和状态信息存储
 * - 排行榜系统：实时排行榜数据管理
 * - 分布式锁：基于Redis的分布式锁实现
 * - 消息队列：轻量级的消息队列功能
 * 
 * 使用示例：
 * @code
 * // 获取NoSQL模块
 * auto nosqlModule = FindModule<NFIAsyNosqlModule>();
 * 
 * // 添加Redis服务器
 * nosqlModule->AddDBServer("redis_main", "127.0.0.1", 6379, "password");
 * 
 * // 异步查询对象
 * NFrame::storesvr_selobj selectReq;
 * selectReq.set_clsname("PlayerData");
 * selectReq.set_objid(playerId);
 * 
 * nosqlModule->SelectObj("redis_main", selectReq, [](int code, const NFrame::storesvr_selobj_res& result) {
 *     if (code == 0) {
 *         // 处理查询结果
 *         ProcessPlayerData(result);
 *     } else {
 *         NFLogError(NF_LOG_DEFAULT, 0, "查询玩家数据失败: {}", code);
 *     }
 * });
 * @endcode
 * 
 * 设计优势：
 * - 简化了NoSQL数据库的异步操作
 * - 提供了统一的数据库访问接口
 * - 支持高并发和高性能的数据访问
 * - 便于扩展和维护
 */
class NFIAsyNosqlModule
        : public NFIAsyModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * 
     * 初始化异步NoSQL模块：
     * - 调用基类构造函数
     * - 设置插件管理器引用
     * - 初始化NoSQL特定的成员变量
     * - 准备异步操作环境
     */
    NFIAsyNosqlModule(NFIPluginManager *p) : NFIAsyModule(p)
    {

    }

    /**
     * @brief 虚析构函数
     * 
     * 清理异步NoSQL模块的资源：
     * - 关闭所有数据库连接
     * - 清理连接池资源
     * - 取消未完成的异步操作
     * - 释放相关内存资源
     */
    virtual ~NFIAsyNosqlModule()
    {

    }

public:
    /**
     * @brief 添加NoSQL数据库服务器连接
     * @param nServerID 服务器标识ID，用于区分不同的数据库实例
     * @param noSqlIp NoSQL服务器IP地址
     * @param nosqlPort NoSQL服务器端口号
     * @param noSqlPass NoSQL服务器连接密码
     * @return 添加成功返回0，失败返回错误码
     * 
     * 向NoSQL模块注册一个数据库服务器连接：
     * 
     * 连接管理：
     * - 支持多个数据库服务器的同时连接
     * - 每个服务器用唯一的nServerID标识
     * - 自动建立和维护连接池
     * - 支持连接的健康检查和自动重连
     * 
     * 连接参数：
     * - nServerID: 服务器的唯一标识，后续操作时用于指定目标服务器
     * - noSqlIp: 数据库服务器的IP地址，支持IPv4和IPv6
     * - nosqlPort: 数据库服务器的端口号，Redis默认为6379
     * - noSqlPass: 连接密码，支持Redis AUTH认证
     * 
     * 连接特性：
     * - 连接池管理：自动管理连接的创建和销毁
     * - 负载均衡：在多个连接间分布请求
     * - 故障切换：连接失败时的自动重试和切换
     * - 性能监控：连接状态和性能的实时监控
     * 
     * 使用示例：
     * @code
     * // 添加主Redis服务器
     * auto result = nosqlModule->AddDBServer("redis_main", "192.168.1.100", 6379, "mypassword");
     * if (result != 0) {
     *     NFLogError(NF_LOG_DEFAULT, 0, "连接主Redis服务器失败");
     * }
     * 
     * // 添加备用Redis服务器
     * nosqlModule->AddDBServer("redis_backup", "192.168.1.101", 6379, "mypassword");
     * 
     * // 添加无密码的本地Redis
     * nosqlModule->AddDBServer("redis_local", "127.0.0.1", 6379, "");
     * @endcode
     */
    virtual int AddDBServer(const std::string& nServerID, const std::string& noSqlIp, int nosqlPort, const std::string& noSqlPass) = 0;

    /**
     * @brief 异步查询对象数据
     * @param nServerID 目标服务器标识ID
     * @param select 查询请求结构，包含查询条件和参数
     * @param cb 查询完成后的回调函数
     * @return 操作提交结果，0表示成功提交异步操作
     * 
     * 从NoSQL数据库异步查询指定对象的数据：
     * 
     * 查询流程：
     * 1. 验证服务器ID和查询参数的有效性
     * 2. 将查询请求提交到异步执行队列
     * 3. 在工作线程中执行实际的数据库查询
     * 4. 查询完成后通过回调函数返回结果
     * 
     * 查询参数：
     * - nServerID: 指定查询的目标数据库服务器
     * - select: storesvr_selobj结构，包含类名、对象ID等查询条件
     * - cb: SelectObjCb类型的回调函数，接收查询结果
     *
     * 回调函数签名：
     * - void(int code, const NFrame::storesvr_selobj_res& result)
     * - code: 操作结果码，0表示成功，非0表示错误
     * - result: 查询结果数据，包含对象的详细信息
     * 
     * 错误处理：
     * - 连接错误：自动重试和故障切换
     * - 超时处理：查询超时的自动处理
     * - 数据不存在：返回相应的错误码
     * - 格式错误：数据格式不匹配的处理
     * 
     * 使用示例：
     * @code
     * // 构造查询请求
     * NFrame::storesvr_selobj selectReq;
     * selectReq.set_clsname("PlayerInfo");
     * selectReq.set_objid(playerId);
     * selectReq.set_select_data("level,exp,gold");
     * 
     * // 执行异步查询
     * nosqlModule->SelectObj("redis_main", selectReq, [playerId](int code, const auto& result) {
     *     if (code == 0) {
     *         NFLogInfo(NF_LOG_DEFAULT, 0, "查询玩家{}数据成功", playerId);
     *         // 处理查询结果
     *         ProcessPlayerData(result);
     *     } else {
     *         NFLogError(NF_LOG_DEFAULT, 0, "查询玩家{}数据失败: {}", playerId, code);
     *     }
     * });
     * @endcode
     */
    virtual int SelectObj(const std::string& nServerID, const NFrame::storesvr_selobj &select,
                          const SelectObjCb& cb) = 0;

    /**
     * @brief 异步删除对象数据
     * @param nServerID 目标服务器标识ID
     * @param select 删除请求结构，包含删除条件
     * @param cb 删除完成后的回调函数
     * @return 操作提交结果，0表示成功提交异步操作
     * 
     * 从NoSQL数据库异步删除指定对象的数据：
     * 
     * 删除流程：
     * 1. 验证删除权限和参数有效性
     * 2. 将删除请求提交到异步执行队列
     * 3. 在工作线程中执行实际的删除操作
     * 4. 删除完成后通过回调函数返回结果
     * 
     * 删除特性：
     * - 安全删除：确保数据一致性和完整性
     * - 级联删除：支持相关数据的级联删除
     * - 软删除：可配置的软删除模式
     * - 事务支持：在支持事务的NoSQL中使用事务
     * 
     * 使用示例：
     * @code
     * // 构造删除请求
     * NFrame::storesvr_delobj deleteReq;
     * deleteReq.set_clsname("PlayerInfo");
     * deleteReq.set_objid(playerId);
     * 
     * // 执行异步删除
     * nosqlModule->DeleteObj("redis_main", deleteReq, [playerId](int code, const auto& result) {
     *     if (code == 0) {
     *         NFLogInfo(NF_LOG_DEFAULT, 0, "删除玩家{}数据成功", playerId);
     *     } else {
     *         NFLogError(NF_LOG_DEFAULT, 0, "删除玩家{}数据失败: {}", playerId, code);
     *     }
     * });
     * @endcode
     */
    virtual int DeleteObj(const std::string& nServerID, const NFrame::storesvr_delobj &select,
                          const DeleteObjCb& cb) = 0;

    /**
     * @brief 异步插入对象数据
     * @param nServerID 目标服务器标识ID
     * @param select 插入请求结构，包含要插入的对象数据
     * @param cb 插入完成后的回调函数
     * @return 操作提交结果，0表示成功提交异步操作
     * 
     * 向NoSQL数据库异步插入新的对象数据：
     * 
     * 插入流程：
     * 1. 验证数据格式和完整性
     * 2. 检查是否存在重复键冲突
     * 3. 将插入请求提交到异步执行队列
     * 4. 在工作线程中执行实际的插入操作
     * 5. 插入完成后通过回调函数返回结果
     * 
     * 插入特性：
     * - 重复检查：自动检查主键和唯一键冲突
     * - 数据验证：插入前的数据格式和约束验证
     * - 原子操作：确保插入操作的原子性
     * - 索引维护：自动维护相关的索引结构
     * 
     * 使用示例：
     * @code
     * // 构造插入请求
     * NFrame::storesvr_insertobj insertReq;
     * insertReq.set_clsname("PlayerInfo");
     * insertReq.set_objid(newPlayerId);
     * insertReq.set_objdata(playerDataJson);
     * 
     * // 执行异步插入
     * nosqlModule->InsertObj("redis_main", insertReq, [newPlayerId](int code, const auto& result) {
     *     if (code == 0) {
     *         NFLogInfo(NF_LOG_DEFAULT, 0, "插入玩家{}数据成功", newPlayerId);
     *     } else {
     *         NFLogError(NF_LOG_DEFAULT, 0, "插入玩家{}数据失败: {}", newPlayerId, code);
     *     }
     * });
     * @endcode
     */
    virtual int InsertObj(const std::string& nServerID, const NFrame::storesvr_insertobj &select,
                          const InsertObjCb& cb) = 0;

    /**
     * @brief 异步修改对象数据
     * @param nServerID 目标服务器标识ID
     * @param select 修改请求结构，包含修改条件和新数据
     * @param cb 修改完成后的回调函数
     * @return 操作提交结果，0表示成功提交异步操作
     * 
     * 异步修改NoSQL数据库中的对象数据：
     * 
     * 修改流程：
     * 1. 验证修改权限和数据有效性
     * 2. 检查目标对象是否存在
     * 3. 将修改请求提交到异步执行队列
     * 4. 在工作线程中执行实际的修改操作
     * 5. 修改完成后通过回调函数返回结果
     * 
     * 修改特性：
     * - 部分更新：支持只修改对象的特定字段
     * - 条件修改：基于条件的有条件修改
     * - 版本控制：支持乐观锁的版本控制
     * - 原子操作：确保修改操作的原子性
     * 
     * 使用示例：
     * @code
     * // 构造修改请求
     * NFrame::storesvr_modobj modifyReq;
     * modifyReq.set_clsname("PlayerInfo");
     * modifyReq.set_objid(playerId);
     * modifyReq.set_modfield("level,exp");
     * modifyReq.set_moddata(newDataJson);
     * 
     * // 执行异步修改
     * nosqlModule->ModifyObj("redis_main", modifyReq, [playerId](int code, const auto& result) {
     *     if (code == 0) {
     *         NFLogInfo(NF_LOG_DEFAULT, 0, "修改玩家{}数据成功", playerId);
     *     } else {
     *         NFLogError(NF_LOG_DEFAULT, 0, "修改玩家{}数据失败: {}", playerId, code);
     *     }
     * });
     * @endcode
     */
    virtual int ModifyObj(const std::string& nServerID, const NFrame::storesvr_modobj &select,
                          const ModifyObjCb& cb) = 0;

    /**
     * @brief 异步更新对象数据
     * @param nServerID 目标服务器标识ID
     * @param select 更新请求结构，包含更新条件和数据
     * @param cb 更新完成后的回调函数
     * @return 操作提交结果，0表示成功提交异步操作
     * 
     * 异步更新NoSQL数据库中的对象数据：
     * 
     * 更新流程：
     * 1. 验证更新参数和数据格式
     * 2. 检查目标对象的存在性和权限
     * 3. 将更新请求提交到异步执行队列
     * 4. 在工作线程中执行实际的更新操作
     * 5. 更新完成后通过回调函数返回结果
     * 
     * 更新特性：
     * - 完整更新：替换对象的全部数据
     * - 增量更新：只更新变化的部分
     * - 批量更新：支持批量对象的更新
     * - 事务更新：在事务中执行更新操作
     * 
     * 与ModifyObj的区别：
     * - UpdateObj通常用于完整的对象替换
     * - ModifyObj更适合部分字段的修改
     * - 具体语义取决于底层NoSQL的实现
     * 
     * 使用示例：
     * @code
     * // 构造更新请求
     * NFrame::storesvr_updateobj updateReq;
     * updateReq.set_clsname("PlayerInfo");
     * updateReq.set_objid(playerId);
     * updateReq.set_objdata(completePlayerDataJson);
     * 
     * // 执行异步更新
     * nosqlModule->UpdateObj("redis_main", updateReq, [playerId](int code, const auto& result) {
     *     if (code == 0) {
     *         NFLogInfo(NF_LOG_DEFAULT, 0, "更新玩家{}数据成功", playerId);
     *     } else {
     *         NFLogError(NF_LOG_DEFAULT, 0, "更新玩家{}数据失败: {}", playerId, code);
     *     }
     * });
     * @endcode
     */
    virtual int UpdateObj(const std::string& nServerID, const NFrame::storesvr_updateobj &select,
                          const UpdateObjCb& cb) = 0;
};
