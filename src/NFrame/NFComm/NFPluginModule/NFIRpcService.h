// -------------------------------------------------------------------------
//    @FileName         :    NFIRpcService.h
//    @Author           :    gaoyi
//    @Date             :    23-3-24
//    @Email			:    445267987@qq.com
//    @Module           :    NFIRpcService
//    @Description      :    RPC服务接口定义，提供远程过程调用的抽象层
//                           支持类型安全的消息处理、动态服务绑定和编译时检查
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFBaseObj.h"

/// @brief RPC服务超时时间定义（毫秒）
/// 调试模式下使用200秒超时，便于调试；发布模式下使用3秒超时，保证性能
#ifdef NF_DEBUG_MODE
#define DEFINE_RPC_SERVICE_TIME_OUT_MS (2000000) ///< 调试模式：200秒超时，便于断点调试
#else
#define DEFINE_RPC_SERVICE_TIME_OUT_MS (3000) ///< 发布模式：3秒超时，保证响应性能
#endif

class NFObject;

/**
 * @brief RPC服务基础接口类
 * 
 * NFIRpcService 提供了最基础的RPC服务抽象：
 * 
 * 核心功能：
 * - 定义RPC服务的统一接口
 * - 支持连接ID和参数传递
 * - 提供Protocol Buffers消息处理
 * - 支持双向通信机制
 * 
 * 设计特点：
 * - 轻量级的接口设计
 * - 支持异步消息处理
 * - 兼容网络传输层
 * - 提供扩展性基础
 * 
 * 使用场景：
 * - 网络RPC服务
 * - 分布式系统通信
 * - 微服务架构
 * - 跨进程调用
 * 
 * 实现要求：
 * - 子类必须实现run方法
 * - 确保线程安全性
 * - 处理异常情况
 * - 返回合适的错误码
 */
class NFIRpcService : public NFBaseObj
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * 
     * 初始化RPC服务基础对象，设置插件管理器。
     */
    NFIRpcService(NFIPluginManager* p):NFBaseObj(p)
    {

    }

    /**
     * @brief 执行RPC服务处理
     * @param unLinkId 网络连接ID，用于标识客户端连接
     * @param reqSvrPkg 请求数据包，包含完整的协议消息
     * @param param1 自定义参数1，用于传递额外信息
     * @param param2 自定义参数2，用于传递额外信息
     * @return 执行结果，0表示成功，非0表示失败
     * 
     * 这是RPC服务的核心处理方法，负责：
     * - 解析请求数据包
     * - 执行具体的业务逻辑
     * - 生成响应数据
     * - 返回处理结果
     * 
     * 使用示例：
     * @code
     * class MyRpcService : public NFIRpcService {
     * public:
     *     int run(uint64_t unLinkId, const NFrame::Proto_FramePkg& reqSvrPkg, 
     *             uint64_t param1, uint64_t param2) override {
     *         // 解析请求数据
     *         // 执行业务逻辑
     *         // 发送响应
     *         return 0;
     *     }
     * };
     * @endcode
     * 
     * @note 此方法应该是线程安全的
     * @note 应该处理所有可能的异常情况
     */
    virtual int run(uint64_t unLinkId, const NFrame::Proto_FramePkg& reqSvrPkg, uint64_t param1, uint64_t param2) = 0;
};

/**
 * @brief 动态RPC服务接口类
 * 
 * NFIDynamicRpcService 提供了更高级的动态RPC服务功能：
 * 
 * 核心功能：
 * - 支持对象化的消息处理
 * - Protocol Buffers类型安全
 * - 多种对象类型支持
 * - 统一的调用接口
 * 
 * 设计特点：
 * - 基于对象的服务模型
 * - 类型安全的消息处理
 * - 支持NFBaseObj和NFObject
 * - 动态方法绑定
 * 
 * 使用场景：
 * - 对象化的RPC服务
 * - 类型安全的消息处理
 * - 动态服务注册
 * - 灵活的业务逻辑
 */
class NFIDynamicRpcService : public NFBaseObj
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * 
     * 初始化动态RPC服务对象。
     */
    NFIDynamicRpcService(NFIPluginManager* p):NFBaseObj(p)
    {

    }

    /**
     * @brief 处理NFBaseObj对象的RPC调用
     * @param pObject 目标对象指针，继承自NFBaseObj
     * @param request 请求消息，Protocol Buffers格式
     * @param respone 响应消息，Protocol Buffers格式
     * @return 执行结果，0表示成功，非0表示失败
     * 
     * 专门处理基于NFBaseObj的RPC调用，提供类型安全的消息处理。
     * 
     * 使用示例：
     * @code
     * class UserService : public NFBaseObj {
     * public:
     *     int GetUserInfo(GetUserRequest& req, GetUserResponse& resp) {
     *         // 处理用户信息查询
     *         return 0;
     *     }
     * };
     * @endcode
     */
    virtual int run(NFBaseObj *pObject, google::protobuf::Message& request, google::protobuf::Message& respone) = 0;

    /**
     * @brief 处理NFObject对象的RPC调用
     * @param pObject 目标对象指针，继承自NFObject
     * @param request 请求消息，Protocol Buffers格式
     * @param respone 响应消息，Protocol Buffers格式
     * @return 执行结果，0表示成功，非0表示失败
     * 
     * 专门处理基于NFObject（共享内存对象）的RPC调用。
     * 适用于需要持久化或跨进程访问的对象。
     */
    virtual int run(NFObject* pObject, google::protobuf::Message& request, google::protobuf::Message& respone) = 0;
};

/**
 * @brief 模板化的动态RPC服务实现类
 * @tparam BaseType 服务对象类型，必须继承自NFBaseObj或NFObject
 * @tparam RequestType 请求消息类型，必须继承自google::protobuf::Message
 * @tparam ResponeType 响应消息类型，必须继承自google::protobuf::Message
 * 
 * NFCDynamicRpcService 是动态RPC服务的具体实现：
 * 
 * 核心功能：
 * - 编译时类型检查
 * - 自动类型转换
 * - 成员函数绑定
 * - 类型安全调用
 * 
 * 设计特点：
 * - 模板元编程确保类型安全
 * - 自动进行动态类型转换
 * - 支持成员函数指针绑定
 * - 统一的错误处理机制
 * 
 * 使用示例：
 * @code
 * class GameService : public NFBaseObj {
 * public:
 *     int HandleLogin(LoginRequest& req, LoginResponse& resp) {
 *         // 处理登录逻辑
 *         resp.set_success(true);
 *         return 0;
 *     }
 * };
 * 
 * // 创建RPC服务
 * auto rpcService = new NFCDynamicRpcService<GameService, LoginRequest, LoginResponse>(
 *     pluginManager, gameService, &GameService::HandleLogin);
 * @endcode
 * 
 * 编译时检查：
 * - 确保RequestType继承自google::protobuf::Message
 * - 确保ResponeType继承自google::protobuf::Message
 * - 确保成员函数签名匹配
 * 
 * 错误处理：
 * - 自动检查类型转换是否成功
 * - 验证对象和函数指针有效性
 * - 返回标准错误码
 */
template<typename BaseType, typename RequestType, typename ResponeType>
class NFCDynamicRpcService : public NFIDynamicRpcService
{
    /// @brief 编译时检查：确保RequestType继承自google::protobuf::Message
    static_assert((TIsDerived<RequestType, google::protobuf::Message>::Result), "the class RequestType must is google::protobuf::Message");
    /// @brief 编译时检查：确保ResponeType继承自google::protobuf::Message
    static_assert((TIsDerived<ResponeType, google::protobuf::Message>::Result), "the class ResponeType must is google::protobuf::Message");
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * @param pBase 服务对象指针（未使用，保留用于扩展）
     * @param handleRecieve 成员函数指针，指向实际的处理方法
     * 
     * 创建模板化的RPC服务实例，绑定处理函数。
     */
    NFCDynamicRpcService(NFIPluginManager* p, BaseType *pBase, int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone)): NFIDynamicRpcService(p)
    {
        m_function = handleRecieve;
    }

    /**
     * @brief 执行NFBaseObj对象的RPC调用
     * @param pObject 目标对象指针
     * @param request 请求消息引用
     * @param respone 响应消息引用
     * @return 执行结果，0表示成功，ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED表示函数不存在
     * 
     * 执行过程：
     * 1. 动态转换消息类型
     * 2. 动态转换对象类型
     * 3. 验证所有转换成功
     * 4. 调用绑定的成员函数
     * 5. 返回执行结果
     */
    virtual int run(NFBaseObj *pObject, google::protobuf::Message& request, google::protobuf::Message& respone) override
    {
        RequestType* pRequest = dynamic_cast<RequestType*>(&request);
        ResponeType* pRespone = dynamic_cast<ResponeType*>(&respone);
        BaseType* pBase = dynamic_cast<BaseType*>(pObject);
        if (pBase && pRequest && pRespone && m_function)
        {
            return (pBase->*m_function)(dynamic_cast<RequestType&>(request), dynamic_cast<ResponeType&>(respone));
        }

        return NFrame::ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED;
    }

    /**
     * @brief 执行NFObject对象的RPC调用
     * @param pObject 目标对象指针（NFObject类型）
     * @param request 请求消息引用
     * @param respone 响应消息引用
     * @return 执行结果，0表示成功，ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED表示函数不存在
     * 
     * 专门处理共享内存对象的RPC调用，执行过程与NFBaseObj版本相同。
     */
    virtual int run(NFObject *pObject, google::protobuf::Message& request, google::protobuf::Message& respone) override
    {
        RequestType* pRequest = dynamic_cast<RequestType*>(&request);
        ResponeType* pRespone = dynamic_cast<ResponeType*>(&respone);
        BaseType* pBase = dynamic_cast<BaseType*>(pObject);
        if (pBase && pRequest && pRespone && m_function)
        {
            return (pBase->*m_function)(dynamic_cast<RequestType&>(request), dynamic_cast<ResponeType&>(respone));
        }

        return NFrame::ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED;
    }

    /// @brief 成员函数指针，指向实际的RPC处理方法
    int (BaseType::*m_function)(RequestType& request, ResponeType& respone);
};

/**
 * @brief RPC服务绑定宏定义
 * @param msgId 消息ID，用于标识RPC服务类型
 * @param RequestType 请求消息类型
 * @param ResponeType 响应消息类型
 * 
 * 该宏用于在编译时建立RPC服务协议号与消息类型的绑定关系：
 * 
 * 作用：
 * - 编译时类型安全检查
 * - 防止运行时RPC服务错误
 * - 确保消息类型一致性
 * - 提供编译期错误检测
 * 
 * 使用方法：
 * @code
 * // 在NFServerBindRpcService.h或NFLogicBindRpcService.h中定义
 * DEFINE_BIND_RPC_SERVICE(1001, LoginRequest, LoginResponse);
 * DEFINE_BIND_RPC_SERVICE(1002, GetUserInfoRequest, GetUserInfoResponse);
 * @endcode
 * 
 * 文件位置：
 * - 服务器架构层：NFServerComm/NFServerCommon/NFServerBindRpcService.h
 * - 游戏逻辑层：游戏目录/NFLogicBindRpcService.h
 * 
 * @note 必须在使用RPC服务之前定义相应的绑定宏
 * @note 每个消息ID只能绑定一种消息类型组合
 */
#define DEFINE_BIND_RPC_SERVICE(msgId, RequestType, ResponeType) \
template<>                                                      \
struct NFBindRpcService<msgId, RequestType, ResponeType>\
{                                                               \
    enum { value = 1 };\
    typedef std::true_type type;\
};

/**
 * @brief RPC服务绑定静态断言宏
 * @param msgId 消息ID
 * @param RequestType 请求消息类型
 * @param ResponeType 响应消息类型
 * 
 * 该宏用于编译时检查RPC服务绑定是否已定义：
 * 
 * 功能：
 * - 编译期间强制检查RPC绑定
 * - 如果未定义绑定宏，编译失败
 * - 提供清晰的错误提示信息
 * - 确保RPC服务配置正确
 * 
 * 错误信息：
 * "You Must First Define DEFINE_BIND_RPC_SERVICE(msgId, RequestType, ResponeType)"
 * 
 * 触发时机：
 * - 注册RPC服务时
 * - 调用RPC服务时
 * - 任何使用RPC的编译单元
 * 
 * 使用示例：
 * @code
 * // 在RPC服务注册代码中自动调用
 * template<size_t msgId, typename RequestType, typename ResponeType>
 * void RegisterRpcService() {
 *     STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
 *     // 注册逻辑...
 * }
 * @endcode
 * 
 * @note 这是一个内部使用的宏，开发者通常不需要直接调用
 * @note 编译失败时，需要先定义相应的DEFINE_BIND_RPC_SERVICE宏
 */
#define STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType) static_assert(NFBindRpcService<msgId, RequestType, ResponeType>::value, \
    "You Must First Define DEFINE_BIND_RPC_SERVICE("#msgId", "#RequestType", "#ResponeType")")\

/**
 * @brief RPC服务绑定模板结构体
 * @tparam msgId 消息ID
 * @tparam RequestType 请求消息类型
 * @tparam ResponeType 响应消息类型
 * 
 * 这个模板结构体是RPC绑定检查机制的核心：
 * 
 * 默认行为：
 * - value = 0：表示未绑定
 * - type = std::false_type：编译时false类型
 * 
 * 特化行为（通过DEFINE_BIND_RPC_SERVICE宏）：
 * - value = 1：表示已绑定
 * - type = std::true_type：编译时true类型
 * 
 * 工作原理：
 * 1. 默认情况下所有RPC都未绑定（value=0）
 * 2. 使用DEFINE_BIND_RPC_SERVICE宏特化模板（value=1）
 * 3. STATIC_ASSERT_BIND_RPC_SERVICE检查value值
 * 4. 编译时确保所有RPC都已正确绑定
 * 
 * @note 这是SFINAE（Substitution Failure Is Not An Error）技术的应用
 * @note 通过模板特化实现编译时配置检查
 */
template<size_t msgId, class RequestType, class ResponeType>
struct NFBindRpcService
{
    enum { value = 0 };           ///< 默认值为0，表示未绑定
    typedef std::false_type type; ///< 默认类型为false_type
};
