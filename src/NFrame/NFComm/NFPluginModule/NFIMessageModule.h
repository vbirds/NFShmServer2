// -------------------------------------------------------------------------
//    @FileName         :    NFIMessageModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    消息通信模块接口定义，提供完整的网络通信、RPC服务
//                           HTTP处理、消息路由和服务器集群管理功能
//
// -------------------------------------------------------------------------

/**
 * @file NFIMessageModule.h
 * @brief 消息通信模块接口定义
 * @details 该文件定义了NFrame框架的核心消息通信接口，提供了：
 *          - 网络通信（TCP/UDP）和连接管理
 *          - RPC服务的注册、调用和响应处理
 *          - HTTP服务器和客户端功能
 *          - 服务器间的消息路由和负载均衡
 *          - 事件广播和集群管理
 *          - 协程支持的异步消息处理
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFIModule.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFCore/NFCRC32.h"
#include "google/protobuf/message.h"
#include "NFComm/NFKernelMessage/FrameSqlData.pb.h"
#include "NFICoroutineModule.h"
#include "NFServerDefine.h"
#include "NFComm/NFCore/NFCommon.h"
#include "NFIConfigModule.h"
#include "NFError.h"
#include "NFRoute.h"

#include <map>
#include <unordered_map>
#include <list>
#include <string>
#include <set>
#include <functional>

#include "NFIPacketParse.h"

class NFIDynamicModule;

/**
 * @brief 消息通信模块接口类
 * 
 * NFIMessageModule 是NFrame框架的核心通信模块，提供了完整的分布式通信解决方案：
 * 
 * 1. RPC远程过程调用：
 *    - 类型安全的RPC服务注册和调用
 *    - 同步和异步RPC支持
 *    - 脚本RPC集成
 *    - 协程化的RPC调用
 *    - 自动序列化和反序列化
 * 
 * 2. 消息处理系统：
 *    - 基于消息ID的路由机制
 *    - 模块化的消息处理
 *    - 消息回调注册和管理
 *    - 事件驱动的通信模式
 *    - 协程化的消息处理
 * 
 * 3. HTTP服务支持：
 *    - RESTful API接口
 *    - HTTP客户端和服务器
 *    - 请求过滤和中间件
 *    - 文件上传下载
 *    - 异步HTTP处理
 * 
 * 4. 服务器集群管理：
 *    - 多服务器类型支持
 *    - 服务发现和注册
 *    - 负载均衡和路由
 *    - 故障转移和恢复
 *    - 跨服务器通信
 * 
 * 5. 网络连接管理：
 *    - TCP/UDP连接池
 *    - 连接状态监控
 *    - 自动重连机制
 *    - 连接安全管理
 *    - 网络事件处理
 * 
 * 6. 数据传输优化：
 *    - Protocol Buffers序列化
 *    - 数据压缩和加密
 *    - 批量消息处理
 *    - 流量控制
 *    - 带宽优化
 * 
 * 设计特点：
 * - 高性能的异步I/O架构
 * - 类型安全的模板化接口
 * - 协程友好的调用方式
 * - 插件化的扩展机制
 * - 完善的错误处理和恢复
 * 
 * 使用场景：
 * - 分布式游戏服务器
 * - 微服务架构
 * - 实时通信系统
 * - API网关服务
 * - 物联网平台
 * 
 * 架构优势：
 * - 统一的通信抽象层
 * - 灵活的服务组合方式
 * - 高度可扩展的设计
 * - 生产级的稳定性
 */
class NFIMessageModule : public NFIModule
{
public:
    /**
     * @brief 模板化的RPC服务实现类
     * @tparam RequestType 请求消息类型，必须继承自google::protobuf::Message
     * @tparam ResponeType 响应消息类型，必须继承自google::protobuf::Message
     * 
     * NFCRpcService 提供了类型安全的RPC服务实现：
     * 
     * 核心功能：
     * - 编译时类型检查和验证
     * - 多种函数签名的支持
     * - 自动消息序列化和反序列化
     * - 协程和回调的支持
     * - 参数传递和上下文管理
     * 
     * 支持的函数签名：
     * - 基础RPC：int handler(RequestType& req, ResponeType& resp)
     * - 带连接ID：int handler(uint64_t linkId, RequestType& req, ResponeType& resp)
     * - 带参数：int handler(RequestType& req, ResponeType& resp, uint64_t p1, uint64_t p2)
     * - 带回调：int handler(RequestType& req, ResponeType& resp, std::function<void()> cb)
     * - 通用接口：int handler(uint32_t msgId, Message& req, Message& resp, uint64_t p1, uint64_t p2)
     * 
     * 设计特点：
     * - 零拷贝的消息处理
     * - 自动错误码传播
     * - 灵活的回调机制
     * - 协程无缝集成
     * 
     * 使用示例：
     * @code
     * // 定义RPC处理器
     * class UserService : public NFIDynamicModule {
     * public:
     *     int HandleLogin(LoginRequest& req, LoginResponse& resp) {
     *         // 处理登录逻辑
     *         resp.set_success(true);
     *         return 0;
     *     }
     * };
     * 
     * // 注册RPC服务
     * msgModule->AddRpcService<1001>(NF_ST_GAME, userService, &UserService::HandleLogin);
     * @endcode
     */
    template <typename RequestType, typename ResponeType>
    class NFCRpcService : public NFIRpcService
    {
        /// @brief 编译时检查：确保RequestType继承自google::protobuf::Message
        static_assert((TIsDerived<RequestType, google::protobuf::Message>::Result), "the class RequestType must is google::protobuf::Message");
        /// @brief 编译时检查：确保ResponeType继承自google::protobuf::Message
        static_assert((TIsDerived<ResponeType, google::protobuf::Message>::Result), "the class ResponeType must is google::protobuf::Message");

    public:
        /**
         * @brief 构造函数 - 支持带连接ID的成员函数
         * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
         * @param p 插件管理器指针
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，处理RPC调用
         */
        template <typename BaseType>
        NFCRpcService(NFIPluginManager* p, BaseType* pBase,
                      int (BaseType::*handleRecieve)(uint64_t unLinkId, RequestType& request, ResponeType& respone)) : NFIRpcService(p)
        {
            static_assert((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
            m_functionWithLink = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        /**
         * @brief 构造函数 - 支持基础成员函数
         * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
         * @param p 插件管理器指针
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，基础RPC处理
         */
        template <typename BaseType>
        NFCRpcService(NFIPluginManager* p, BaseType* pBase, int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone)) : NFIRpcService(p)
        {
            static_assert((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
            m_function = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2);
        }

        /**
         * @brief 构造函数 - 支持带回调的成员函数
         * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
         * @param p 插件管理器指针
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，支持异步回调的RPC处理
         */
        template <typename BaseType>
        NFCRpcService(NFIPluginManager* p, BaseType* pBase, int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone, const std::function<void()>& cb)) : NFIRpcService(p)
        {
            static_assert((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
            m_functionWithCallBack = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        /**
         * @brief 构造函数 - 支持通用消息处理的成员函数
         * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
         * @param p 插件管理器指针
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，通用消息处理接口
         */
        template <typename BaseType>
        NFCRpcService(NFIPluginManager* p, BaseType* pBase, int (BaseType::*handleRecieve)(uint32_t msgId, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1, uint64_t param2)) : NFIRpcService(p)
        {
            static_assert((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
            m_functionCommonWithParam = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                                  std::placeholders::_4, std::placeholders::_5);
        }

        /**
         * @brief 构造函数 - 支持带参数的成员函数
         * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
         * @param p 插件管理器指针
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，支持额外参数传递的RPC处理
         */
        template <typename BaseType>
        NFCRpcService(NFIPluginManager* p, BaseType* pBase, int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone, uint64_t param1, uint64_t param2)) : NFIRpcService(p)
        {
            static_assert((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
            m_functionWithParam = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                            std::placeholders::_4);
        }

        /**
         * @brief 构造函数 - 支持带连接ID的静态函数
         * @param p 插件管理器指针
         * @param handleRecieve 静态函数指针，处理RPC调用
         */
        NFCRpcService(NFIPluginManager* p, int (*handleRecieve)(uint64_t unLinkId, RequestType& request, ResponeType& respone)) : NFIRpcService(p)
        {
            m_functionWithLink = std::bind(handleRecieve, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        /**
         * @brief 构造函数 - 支持基础静态函数
         * @param p 插件管理器指针
         * @param handleRecieve 静态函数指针，基础RPC处理
         */
        NFCRpcService(NFIPluginManager* p, int (*handleRecieve)(RequestType& request, ResponeType& respone))
            : NFIRpcService(p)
        {
            m_function = std::bind(handleRecieve, std::placeholders::_1, std::placeholders::_2);
        }

        /**
         * @brief 构造函数 - 支持带回调的静态函数
         * @param p 插件管理器指针
         * @param handleRecieve 静态函数指针，支持异步回调的RPC处理
         */
        NFCRpcService(NFIPluginManager* p, int (*handleRecieve)(RequestType& request, ResponeType& respone, const std::function<void()>& cb)) : NFIRpcService(p)
        {
            m_functionWithCallBack = std::bind(handleRecieve, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        /**
         * @brief 构造函数 - 支持通用消息处理的静态函数
         * @param p 插件管理器指针
         * @param handleRecieve 静态函数指针，通用消息处理接口
         */
        NFCRpcService(NFIPluginManager* p, int (*handleRecieve)(uint32_t msgId, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1, uint64_t param2)) : NFIRpcService(p)
        {
            m_functionCommonWithParam = std::bind(handleRecieve, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        }

        /**
         * @brief 构造函数 - 支持带参数的静态函数
         * @param p 插件管理器指针
         * @param handleRecieve 静态函数指针，支持额外参数传递的RPC处理
         */
        NFCRpcService(NFIPluginManager* p, int (*handleRecieve)(RequestType& request, ResponeType& respone, uint64_t param1, uint64_t param2)) : NFIRpcService(p)
        {
            m_functionWithParam = std::bind(handleRecieve, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        }

        /**
         * @brief 执行RPC服务处理
         * @param unLinkId 网络连接ID
         * @param reqSvrPkg 请求数据包
         * @param param1 自定义参数1
         * @param param2 自定义参数2
         * @return 执行结果，0表示成功
         * 
         * 这是RPC服务的核心执行方法，负责：
         * - 消息类型验证和哈希检查
         * - 请求消息的反序列化
         * - 调用注册的处理函数
         * - 响应消息的序列化
         * - 错误码的设置和传播
         * - 响应消息的发送
         */
        int run(uint64_t unLinkId, const NFrame::Proto_FramePkg& reqSvrPkg, uint64_t param1, uint64_t param2) override
        {
            RequestType req;
            ResponeType rsp;
            CHECK_EXPR(NFHash::hash<std::string>()(req.GetTypeName()) == reqSvrPkg.rpc_info().req_rpc_hash(), NFrame::ERR_CODE_RPC_DECODE_FAILED,
                       "NFCRpcService reqHash Not Equal:{}, nMsgId:{}", req.GetTypeName(), reqSvrPkg.msg_id());
            CHECK_EXPR(NFHash::hash<std::string>()(rsp.GetTypeName()) == reqSvrPkg.rpc_info().rsp_rpc_hash(), NFrame::ERR_CODE_RPC_DECODE_FAILED,
                       "NFCRpcService rspHash Not Equal:{}, nMsgId:{}", rsp.GetTypeName(), reqSvrPkg.msg_id());

            req.ParsePartialFromString(reqSvrPkg.msg_data());

            uint32_t eServerType = GetServerTypeFromUnlinkId(unLinkId);
            uint32_t reqBusId = reqSvrPkg.rpc_info().req_bus_id();
            uint32_t reqServerType = reqSvrPkg.rpc_info().req_server_type();

            int iRet = 0;
            NFrame::Proto_FramePkg svrPkg;
            svrPkg.set_msg_id(reqSvrPkg.msg_id());
            svrPkg.mutable_rpc_info()->set_req_rpc_id(0);
            svrPkg.mutable_rpc_info()->set_rsp_rpc_id(reqSvrPkg.rpc_info().req_rpc_id());
            svrPkg.mutable_rpc_info()->set_req_rpc_hash(reqSvrPkg.rpc_info().req_rpc_hash());
            svrPkg.mutable_rpc_info()->set_rsp_rpc_hash(reqSvrPkg.rpc_info().rsp_rpc_hash());
            svrPkg.mutable_rpc_info()->set_is_script_rpc(reqSvrPkg.rpc_info().is_script_rpc());
            if (m_function || m_functionWithParam || m_functionCommonWithParam || m_functionWithLink || m_functionWithCallBack)
            {
                if (m_function)
                {
                    iRet = m_function(req, rsp);
                }
                else if (m_functionCommonWithParam)
                {
                    iRet = m_functionCommonWithParam(reqSvrPkg.msg_id(), req, rsp, param1, param2);
                }
                else if (m_functionWithParam)
                {
                    iRet = m_functionWithParam(req, rsp, param1, param2);
                }
                else if (m_functionWithLink)
                {
                    iRet = m_functionWithLink(unLinkId, req, rsp);
                }
                else if (m_functionWithCallBack)
                {
                    iRet = m_functionWithCallBack(req, rsp, [eServerType, reqServerType, reqBusId, &svrPkg, &rsp, this]()
                    {
                        svrPkg.set_msg_data(rsp.SerializePartialAsString());
                        svrPkg.mutable_rpc_info()->set_rpc_ret_code(0);
                        FindModule<NFIMessageModule>()->SendMsgToServer((NF_SERVER_TYPE)eServerType, (NF_SERVER_TYPE)reqServerType, 0, reqBusId,
                                                                        NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD, svrPkg);
                    });
                }
                svrPkg.set_msg_data(rsp.SerializePartialAsString());
                svrPkg.mutable_rpc_info()->set_rpc_ret_code(iRet);
            }
            else
            {
                svrPkg.mutable_rpc_info()->set_rpc_ret_code(NFrame::ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED);
            }

            FindModule<NFIMessageModule>()->SendMsgToServer((NF_SERVER_TYPE)eServerType, (NF_SERVER_TYPE)reqServerType, 0, reqBusId,
                                                            NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD, svrPkg);

            return 0;
        }

        std::function<int(RequestType& request, ResponeType& respone)> m_function;                                                                     ///< 基础RPC处理函数
        std::function<int(uint32_t msgId, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1,
                          uint64_t param2)> m_functionCommonWithParam;                                                                                  ///< 通用消息处理函数
        std::function<int(RequestType& request, ResponeType& respone, uint64_t param1, uint64_t param2)> m_functionWithParam;                       ///< 带参数的RPC处理函数
        std::function<int(uint64_t unLinkId, RequestType& request, ResponeType& respone)> m_functionWithLink;                                        ///< 带连接ID的RPC处理函数
        std::function<int(RequestType& request, ResponeType& respone, const std::function<void()>& cb)> m_functionWithCallBack;                     ///< 带回调的RPC处理函数
    };

public:
    /**
     * @brief 脚本RPC服务实现类
     * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
     * 
     * NFCScriptRpcService 专门为脚本语言集成设计的RPC服务：
     * 
     * 核心功能：
     * - 支持动态类型的消息处理
     * - 字符串形式的消息序列化
     * - 脚本语言友好的接口
     * - 运行时类型检查
     * - 灵活的消息格式支持
     * 
     * 设计特点：
     * - 无需编译时类型绑定
     * - 支持动态消息创建
     * - 适配各种脚本语言
     * - 简化的接口设计
     * 
     * 使用场景：
     * - Lua脚本RPC调用
     * - Python脚本集成
     * - 动态配置的RPC服务
     * - 热更新的业务逻辑
     */
    template <typename BaseType>
    class NFCScriptRpcService : public NFIRpcService
    {
        /// @brief 编译时检查：确保BaseType继承自NFIDynamicModule
        static_assert((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");

    public:
        /**
         * @brief 构造函数 - 支持带连接ID的脚本RPC处理
         * @param p 插件管理器指针
         * @param reqType 请求消息类型名称
         * @param rspType 响应消息类型名称
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，处理脚本RPC调用
         */
        NFCScriptRpcService(NFIPluginManager* p, const std::string& reqType, const std::string& rspType, BaseType* pBase,
                            int (BaseType::*handleRecieve)(uint64_t unLinkId, uint32_t msgId, const std::string& reqType, const std::string& request,
                                                           const std::string& rspType, std::string& respone)) : NFIRpcService(p)
        {
            m_reqType = reqType;
            m_rspType = rspType;
            m_functionWithLink = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                           std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
        }

        /**
         * @brief 构造函数 - 支持基础脚本RPC处理
         * @param p 插件管理器指针
         * @param reqType 请求消息类型名称
         * @param rspType 响应消息类型名称
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，基础脚本RPC处理
         */
        NFCScriptRpcService(NFIPluginManager* p, const std::string& reqType, const std::string& rspType, BaseType* pBase,
                            int (BaseType::*handleRecieve)(uint32_t msgId, const std::string& reqType, const std::string& request,
                                                           const std::string& rspType, std::string& respone)) : NFIRpcService(p)
        {
            m_reqType = reqType;
            m_rspType = rspType;
            m_function = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
                                   std::placeholders::_5);
        }

        /**
         * @brief 构造函数 - 支持带回调的脚本RPC处理
         * @param p 插件管理器指针
         * @param reqType 请求消息类型名称
         * @param rspType 响应消息类型名称
         * @param pBase 服务对象指针
         * @param handleRecieve 成员函数指针，支持异步回调的脚本RPC处理
         */
        NFCScriptRpcService(NFIPluginManager* p, const std::string& reqType, const std::string& rspType, BaseType* pBase,
                            int (BaseType::*handleRecieve)(uint32_t msgId, const std::string& reqType, const std::string& request,
                                                           const std::string& rspType, std::string& respone, const std::function<void()>& cb))
            : NFIRpcService(p)
        {
            m_reqType = reqType;
            m_rspType = rspType;
            m_functionWithCallBack = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                               std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
        }

        /**
         * @brief 执行脚本RPC服务处理
         * @param unLinkId 网络连接ID
         * @param reqSvrPkg 请求数据包
         * @param param1 自定义参数1
         * @param param2 自定义参数2
         * @return 执行结果，0表示成功
         * 
         * 脚本RPC的核心执行方法，负责：
         * - 动态类型验证和哈希检查
         * - 字符串形式的消息处理
         * - 调用脚本处理函数
         * - 响应消息的组装和发送
         */
        virtual int run(uint64_t unLinkId, const NFrame::Proto_FramePkg& reqSvrPkg, uint64_t param1, uint64_t param2) override
        {
            std::string rsp;
            CHECK_EXPR(NFHash::hash<std::string>()(m_reqType) == reqSvrPkg.rpc_info().req_rpc_hash(), NFrame::ERR_CODE_RPC_DECODE_FAILED,
                       "NFCScriptRpcService reqHash Not Equal:{}, nMsgId:{}", m_reqType, reqSvrPkg.msg_id());
            CHECK_EXPR(NFHash::hash<std::string>()(m_rspType) == reqSvrPkg.rpc_info().rsp_rpc_hash(), NFrame::ERR_CODE_RPC_DECODE_FAILED,
                       "NFCScriptRpcService rspHash Not Equal:{}, nMsgId:{}", m_rspType, reqSvrPkg.msg_id());

            uint32_t eServerType = GetServerTypeFromUnlinkId(unLinkId);
            uint32_t reqBusId = reqSvrPkg.rpc_info().req_bus_id();
            uint32_t reqServerType = reqSvrPkg.rpc_info().req_server_type();

            int iRet = 0;
            NFrame::Proto_FramePkg svrPkg;
            svrPkg.set_msg_id(reqSvrPkg.msg_id());
            svrPkg.mutable_rpc_info()->set_req_rpc_id(0);
            svrPkg.mutable_rpc_info()->set_rsp_rpc_id(reqSvrPkg.rpc_info().req_rpc_id());
            svrPkg.mutable_rpc_info()->set_req_rpc_hash(reqSvrPkg.rpc_info().req_rpc_hash());
            svrPkg.mutable_rpc_info()->set_rsp_rpc_hash(reqSvrPkg.rpc_info().rsp_rpc_hash());
            svrPkg.mutable_rpc_info()->set_is_script_rpc(reqSvrPkg.rpc_info().is_script_rpc());
            if (m_function || m_functionWithLink || m_functionWithCallBack)
            {
                if (m_function)
                {
                    iRet = m_function(reqSvrPkg.msg_id(), m_reqType, reqSvrPkg.msg_data(), m_rspType, rsp);
                }
                else if (m_functionWithLink)
                {
                    iRet = m_functionWithLink(unLinkId, reqSvrPkg.msg_id(), m_reqType, reqSvrPkg.msg_data(), m_rspType, rsp);
                }
                else if (m_functionWithCallBack)
                {
                    iRet = m_functionWithCallBack(reqSvrPkg.msg_id(), m_reqType, reqSvrPkg.msg_data(), m_rspType, rsp,
                                                  [eServerType, reqServerType, reqBusId, &svrPkg, &rsp, this]()
                                                  {
                                                      svrPkg.set_msg_data(rsp);
                                                      svrPkg.mutable_rpc_info()->set_rpc_ret_code(0);
                                                      FindModule<NFIMessageModule>()->SendMsgToServer((NF_SERVER_TYPE)eServerType,
                                                                                                      (NF_SERVER_TYPE)reqServerType, 0, reqBusId,
                                                                                                      NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD, svrPkg);
                                                  });
                }
                svrPkg.set_msg_data(rsp);
                svrPkg.mutable_rpc_info()->set_rpc_ret_code(iRet);
            }
            else
            {
                svrPkg.mutable_rpc_info()->set_rpc_ret_code(NFrame::ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED);
            }

            FindModule<NFIMessageModule>()->SendMsgToServer((NF_SERVER_TYPE)eServerType, (NF_SERVER_TYPE)reqServerType, 0, reqBusId,
                                                            NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD, svrPkg);

            return 0;
        }

        std::string m_reqType;                                                                                                                               ///< 请求消息类型名称
        std::string m_rspType;                                                                                                                               ///< 响应消息类型名称
        std::function<int(uint32_t msgId, const std::string& reqType, const std::string& request, const std::string& rspType,
                          std::string& respone)> m_function;                                                                                                ///< 基础脚本RPC处理函数
        std::function<int(uint64_t unLinkId, uint32_t msgId, const std::string& reqType, const std::string& request, const std::string& rspType,
                          std::string& respone)> m_functionWithLink;                                                                                       ///< 带连接ID的脚本RPC处理函数
        std::function<int(uint32_t msgId, const std::string& reqType, const std::string& request, const std::string& rspType, std::string& respone,
                          const std::function<void()>& cb)> m_functionWithCallBack;                                                                       ///< 带回调的脚本RPC处理函数
    };

public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     * 
     * 初始化消息模块，设置基础配置和状态。
     */
    NFIMessageModule(NFIPluginManager* p) : NFIModule(p)
    {
    }

    /**
     * @brief 析构函数
     * 
     * 清理消息模块资源，关闭连接和释放内存。
     */
    virtual ~NFIMessageModule()
    {
    }

public:
    // HTTP请求处理器注册相关方法
    
    /**
     * @brief 添加HTTP请求处理器（指定路径）
     * @tparam BaseType 处理器类型，必须继承自合适的基类
     * @param serverType 服务器类型
     * @param strPath HTTP路径
     * @param eRequestType HTTP请求类型（GET、POST等）
     * @param pBase 处理器对象指针
     * @param handleRecieve 处理函数指针
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定的HTTP路径注册处理器，支持RESTful API设计。
     */
    template <typename BaseType>
    bool AddHttpRequestHandler(NF_SERVER_TYPE serverType, const std::string& strPath, const NFHttpType eRequestType,
                               BaseType* pBase, bool (BaseType::*handleRecieve)(uint32_t, const NFIHttpHandle& req))
    {
        HTTP_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2);
        return AddHttpMsgCB(serverType, strPath, eRequestType, functor);
    }

    /**
     * @brief 添加HTTP请求处理器（默认处理器）
     * @tparam BaseType 处理器类型
     * @param serverType 服务器类型
     * @param eRequestType HTTP请求类型
     * @param pBase 处理器对象指针
     * @param handleRecieve 处理函数指针
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定请求类型注册默认处理器，处理未匹配路径的请求。
     */
    template <typename BaseType>
    bool AddHttpRequestHandler(NF_SERVER_TYPE serverType, const NFHttpType eRequestType, BaseType* pBase,
                               bool (BaseType::*handleRecieve)(uint32_t, const NFIHttpHandle& req))
    {
        HTTP_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2);
        return AddHttpOtherMsgCB(serverType, eRequestType, functor);
    }

    /**
     * @brief 添加HTTP网络过滤器
     * @tparam BaseType 过滤器类型
     * @param serverType 服务器类型
     * @param strPath HTTP路径
     * @param pBase 过滤器对象指针
     * @param handleFilter 过滤函数指针
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定路径添加请求过滤器，用于认证、限流等中间件功能。
     */
    template <typename BaseType>
    bool AddHttpNetFilter(NF_SERVER_TYPE serverType, const std::string& strPath, BaseType* pBase,
                          NFWebStatus (BaseType::*handleFilter)(uint32_t, const NFIHttpHandle& req))
    {
        HTTP_FILTER_FUNCTOR functor = std::bind(handleFilter, pBase, std::placeholders::_1, std::placeholders::_2);

        return AddHttpFilterCB(serverType, strPath, functor);
    }

public:
    // 消息回调注册相关方法
    
    /**
     * @brief 添加消息回调处理器
     * @tparam BaseType 处理器类型，必须继承自NFIDynamicModule
     * @param eType 服务器类型
     * @param nMsgID 消息ID
     * @param pBase 处理器对象指针
     * @param handleRecieve 处理函数指针
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定消息ID注册处理器，支持协程化的异步处理。
     */
    template <typename BaseType>
    bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nMsgID, BaseType* pBase, int (BaseType::*handleRecieve)(uint64_t unLinkId, NFDataPackage& packet), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NET_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2);
        return AddMessageCallBack(eType, nMsgID, pBase, functor, createCo);
    }

    /**
     * @brief 添加模块化消息回调处理器
     * @tparam BaseType 处理器类型，必须继承自NFIDynamicModule
     * @param eType 服务器类型
     * @param nModuleId 模块ID
     * @param nMsgID 消息ID
     * @param pBase 处理器对象指针
     * @param handleRecieve 处理函数指针
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定模块和消息ID注册处理器，支持模块化的消息处理。
     */
    template <typename BaseType>
    bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, BaseType* pBase, int (BaseType::*handleRecieve)(uint64_t unLinkId, NFDataPackage& packet), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NET_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2);
        return AddMessageCallBack(eType, nModuleId, nMsgID, pBase, functor, createCo);
    }

    /**
     * @brief 添加静态函数消息回调处理器
     * @param eType 服务器类型
     * @param nMsgID 消息ID
     * @param handleRecieve 静态处理函数指针
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     */
    bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nMsgID, int (*handleRecieve)(uint64_t unLinkId, NFDataPackage& packet), bool createCo = false)
    {
        NET_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, std::placeholders::_1, std::placeholders::_2);
        return AddMessageCallBack(eType, nMsgID, functor, createCo);
    }

    /**
     * @brief 添加模块化静态函数消息回调处理器
     * @param eType 服务器类型
     * @param nModuleId 模块ID
     * @param nMsgID 消息ID
     * @param handleRecieve 静态处理函数指针
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     */
    bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, int (*handleRecieve)(uint64_t unLinkId, NFDataPackage& packet), bool createCo = false)
    {
        NET_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, std::placeholders::_1, std::placeholders::_2);
        return AddMessageCallBack(eType, nModuleId, nMsgID, functor, createCo);
    }

    /**
     * @brief 添加其他消息回调处理器
     * @tparam BaseType 处理器类型，必须继承自NFIDynamicModule
     * @param eType 服务器类型
     * @param linkId 连接ID
     * @param pBase 处理器对象指针
     * @param handleRecieve 处理函数指针
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 为未注册的消息添加通用处理器。
     */
    template <typename BaseType>
    bool AddOtherCallBack(NF_SERVER_TYPE eType, uint64_t linkId, BaseType* pBase, int (BaseType::*handleRecieve)(uint64_t unLinkId, NFDataPackage& packet), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NET_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2);

        return AddOtherCallBack(eType, linkId, pBase, functor, createCo);
    }

    /**
     * @brief 添加事件回调处理器
     * @tparam BaseType 处理器类型，必须继承自NFIDynamicModule
     * @param eType 服务器类型
     * @param linkId 连接ID
     * @param pBase 处理器对象指针
     * @param handler 事件处理函数指针
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 为网络事件（连接、断开等）添加处理器。
     */
    template <typename BaseType>
    bool AddEventCallBack(NF_SERVER_TYPE eType, uint64_t linkId, BaseType* pBase, int (BaseType::*handler)(eMsgType nEvent, uint64_t unLinkId), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIMessageProcessor");
        NET_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        return AddEventCallBack(eType, linkId, pBase, functor, createCo);
    }

    /**
     * @brief 添加全局消息回调处理器
     * @tparam BaseType 处理器类型，必须继承自NFIDynamicModule
     * @param eType 服务器类型
     * @param pBase 处理器对象指针
     * @param handleRecieve 处理函数指针
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 对所有消息添加统一回调，通过返回值决定是否处理消息：
     * - 返回0：处理该消息
     * - 返回非0：不处理该消息
     */
    template <typename BaseType>
    bool AddAllMsgCallBack(NF_SERVER_TYPE eType, BaseType* pBase, int (BaseType::*handleRecieve)(uint64_t unLinkId, NFDataPackage& packet), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NET_RECEIVE_FUNCTOR functor = std::bind(handleRecieve, pBase, std::placeholders::_1, std::placeholders::_2);

        return AddAllMsgCallBack(eType, pBase, functor, createCo);
    }

public:
    // ========================================================================
    // RPC服务注册接口 - 成员函数版本
    // ========================================================================
    
    /**
     * @brief 添加RPC服务（成员函数版本 - 带连接ID）
     * @tparam msgId 消息ID，编译时常量
     * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
     * @tparam RequestType 请求消息类型，必须继承自google::protobuf::Message
     * @tparam ResponeType 响应消息类型，必须继承自google::protobuf::Message
     * @param serverType 服务器类型
     * @param pBase 服务对象指针
     * @param handleRecieve 成员函数指针，处理函数签名：int (T::*)(uint64_t linkId, RequestType&, ResponeType&)
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 注册一个RPC服务处理器，处理函数可以访问网络连接ID。
     * 编译时进行类型检查，确保消息类型的正确性。
     */
    template <size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(uint64_t unLinkId, RequestType& request, ResponeType& respone), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, msgId, pBase, pRpcService, createCo);
    }

    /**
     * @brief 添加RPC服务（成员函数版本 - 带模块ID和连接ID）
     * @tparam moduleId 模块ID，编译时常量
     * @tparam msgId 消息ID，编译时常量
     * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
     * @tparam RequestType 请求消息类型
     * @tparam ResponeType 响应消息类型
     * @param serverType 服务器类型
     * @param pBase 服务对象指针
     * @param handleRecieve 成员函数指针
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 支持模块化的RPC服务注册，允许同一消息ID在不同模块中有不同的处理器。
     */
    template <size_t moduleId, size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(uint64_t unLinkId, RequestType& request, ResponeType& respone), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pBase, pRpcService, createCo);
    }

    template <size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase, int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, msgId, pBase, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase, int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pBase, pRpcService, createCo);
    }

    template <size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(uint32_t msg, google::protobuf::Message& request, google::protobuf::Message& respone,
                                                      uint64_t param1, uint64_t param2),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, msgId, pBase, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(uint32_t msg, google::protobuf::Message& request, google::protobuf::Message& respone,
                                                      uint64_t param1, uint64_t param2),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pBase, pRpcService, createCo);
    }

    template <size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone, uint64_t param1, uint64_t param2),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, msgId, pBase, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone, uint64_t param1, uint64_t param2),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pBase, pRpcService, createCo);
    }

    template <size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone, const std::function<void()>& cb),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, msgId, pBase, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename BaseType, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, BaseType* pBase,
                       int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone, const std::function<void()>& cb),
                       bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, pBase, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pBase, pRpcService, createCo);
    }

public:
    // ========================================================================
    // RPC服务注册接口 - 静态函数版本
    // ========================================================================
    
    /**
     * @brief 添加RPC服务（静态函数版本 - 带连接ID）
     * @tparam msgId 消息ID，编译时常量
     * @tparam RequestType 请求消息类型，必须继承自google::protobuf::Message
     * @tparam ResponeType 响应消息类型，必须继承自google::protobuf::Message
     * @param serverType 服务器类型
     * @param handleRecieve 静态函数指针，处理函数签名：int (*)(uint64_t, RequestType&, ResponeType&)
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 使用静态函数注册RPC服务，不需要关联特定的对象实例。
     * 适用于无状态的服务处理逻辑。
     */
    template <size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(uint64_t unLinkId, RequestType& request, ResponeType& respone), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, msgId, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(uint64_t unLinkId, RequestType& request, ResponeType& respone), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pRpcService, createCo);
    }

    template <size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(RequestType& request, ResponeType& respone), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, msgId, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(RequestType& request, ResponeType& respone), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pRpcService, createCo);
    }

    template <size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(uint32_t msg, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1, uint64_t param2), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, msgId, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(uint32_t msg, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1, uint64_t param2), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pRpcService, createCo);
    }

    template <size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(RequestType& request, ResponeType& respone, uint64_t param1, uint64_t param2), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, msgId, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(RequestType& request, ResponeType& respone, uint64_t param1, uint64_t param2), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pRpcService, createCo);
    }

    template <size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(RequestType& request, ResponeType& respone, const std::function<void()>& cb), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, msgId, pRpcService, createCo);
    }

    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponeType>
    bool AddRpcService(NF_SERVER_TYPE serverType, int (*handleRecieve)(RequestType& request, ResponeType& respone, const std::function<void()>& cb), bool createCo = false)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        NFIRpcService* pRpcService = new NFCRpcService<RequestType, ResponeType>(m_pObjPluginManager, handleRecieve);
        return AddRpcService(serverType, moduleId, msgId, pRpcService, createCo);
    }

public:
    // ========================================================================
    // 脚本RPC服务注册接口
    // ========================================================================
    
    /**
     * @brief 添加脚本RPC服务（带连接ID）
     * @tparam BaseType 服务类型，必须继承自NFIDynamicModule
     * @param serverType 服务器类型
     * @param nMsgId 消息ID
     * @param reqType 请求消息类型名称
     * @param rspType 响应消息类型名称
     * @param pBase 服务对象指针
     * @param handleRecieve 成员函数指针，处理函数签名：int (T::*)(uint64_t, uint32_t, string&, string&, string&, string&)
     * @param createCo 是否创建协程处理，默认false
     * @return 注册成功返回true，失败返回false
     * 
     * 专为脚本语言设计的RPC服务注册，支持动态类型和字符串形式的消息处理。
     * 适用于Lua、Python等脚本语言的RPC调用。
     */
    template <typename BaseType>
    bool AddScriptRpcService(NF_SERVER_TYPE serverType, uint32_t nMsgId, const std::string& reqType, const std::string& rspType, BaseType* pBase,
                             int (BaseType::*handleRecieve)(uint64_t unLinkId, uint32_t msgId, const std::string& reqType, const std::string& request,
                                                            const std::string& rspType, std::string& respone), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCScriptRpcService<BaseType>(m_pObjPluginManager, reqType, rspType, pBase, handleRecieve);
        return AddRpcService(serverType, nMsgId, pBase, pRpcService, createCo);
    }

    template <typename BaseType>
    bool AddScriptRpcService(NF_SERVER_TYPE serverType, uint32_t nMsgId, const std::string& reqType, const std::string& rspType, BaseType* pBase,
                             int (BaseType::*handleRecieve)(uint32_t msgId, const std::string& reqType, const std::string& request,
                                                            const std::string& rspType,
                                                            std::string& respone), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCScriptRpcService<BaseType>(m_pObjPluginManager, reqType, rspType, pBase, handleRecieve);
        return AddRpcService(serverType, nMsgId, pBase, pRpcService, createCo);
    }

    template <typename BaseType>
    bool AddScriptRpcService(NF_SERVER_TYPE serverType, uint32_t nMsgId, const std::string& reqType, const std::string& rspType, BaseType* pBase,
                             int (BaseType::*handleRecieve)(uint32_t msgId, const std::string& reqType, const std::string& request,
                                                            const std::string& rspType,
                                                            std::string& respone, const std::function<void()>& cb), bool createCo = false)
    {
        NF_ASSERT_MSG((TIsDerived<BaseType, NFIDynamicModule>::Result), "the class must inherit NFIDynamicModule");
        NFIRpcService* pRpcService = new NFCScriptRpcService<BaseType>(m_pObjPluginManager, reqType, rspType, pBase, handleRecieve);
        return AddRpcService(serverType, nMsgId, pBase, pRpcService, createCo);
    }

    // ========================================================================
    // RPC服务调用接口 - 同步版本（协程）
    // ========================================================================
    
    /**
     * @brief 同步RPC调用（协程版本 - 基础）
     * @tparam msgId 消息ID，编译时常量
     * @tparam RequestType 请求消息类型，必须继承自google::protobuf::Message
     * @tparam ResponeType 响应消息类型，必须继承自google::protobuf::Message
     * @param serverType 发送方服务器类型
     * @param dstServerType 目标服务器类型
     * @param dstBusId 目标服务器总线ID
     * @param request 请求消息对象
     * @param respone 响应消息对象（输出参数）
     * @param param1 自定义参数1，默认为0
     * @param param2 自定义参数2，默认为0
     * @return 调用结果，0表示成功，非0表示失败
     * 
     * @warning 该方法必须在协程内部调用，会阻塞当前协程直到收到响应
     * @note 内部使用默认模块ID（NF_MODULE_SERVER）
     * @details 该方法执行以下步骤：
     *          1. 序列化请求消息
     *          2. 发送RPC请求到目标服务器
     *          3. 挂起当前协程等待响应
     *          4. 反序列化响应消息
     *          5. 返回调用结果
     */
    template <size_t msgId, typename RequestType, typename ResponeType>
    int GetRpcService(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, const RequestType& request, ResponeType& respone,
                      uint64_t param1 = 0, uint64_t param2 = 0)
    {
        return GetRpcService<NF_MODULE_SERVER, msgId>(serverType, dstServerType, dstBusId, request, respone, param1, param2);
    }

    /**
     * @brief 同步RPC调用（协程版本 - 带模块ID）
     * @tparam moduleId 模块ID，编译时常量
     * @tparam msgId 消息ID，编译时常量
     * @tparam RequestType 请求消息类型
     * @tparam ResponeType 响应消息类型
     * @param serverType 发送方服务器类型
     * @param dstServerType 目标服务器类型
     * @param dstBusId 目标服务器总线ID
     * @param request 请求消息对象
     * @param respone 响应消息对象（输出参数）
     * @param param1 自定义参数1，默认为0
     * @param param2 自定义参数2，默认为0
     * @return 调用结果，0表示成功，非0表示失败
     * 
     * 支持模块化的同步RPC调用，允许指定目标模块ID进行精确路由。
     */
    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponeType>
    int GetRpcService(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, const RequestType& request, ResponeType& respone,
                      uint64_t param1 = 0, uint64_t param2 = 0)
    {
        STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType);
        static_assert((TIsDerived<RequestType, google::protobuf::Message>::Result), "the class RequestType must is google::protobuf::Message");
        static_assert((TIsDerived<ResponeType, google::protobuf::Message>::Result), "the class ResponeType must is google::protobuf::Message");
        NF_ASSERT_MSG(FindModule<NFICoroutineModule>()->IsInCoroutine(), "Call GetRpcService Must Int the Coroutine");
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(serverType);
        CHECK_EXPR(pConfig, -1, "can't find server config! servertype:{}", GetServerName(serverType));

        NFrame::Proto_FramePkg svrPkg;
        svrPkg.set_module_id(moduleId);
        svrPkg.set_msg_id(msgId);
        svrPkg.set_msg_data(request.SerializePartialAsString());
        svrPkg.mutable_rpc_info()->set_req_rpc_id(FindModule<NFICoroutineModule>()->CurrentTaskId());
        svrPkg.mutable_rpc_info()->set_req_rpc_hash(NFHash::hash<std::string>()(request.GetTypeName()));
        svrPkg.mutable_rpc_info()->set_rsp_rpc_hash(NFHash::hash<std::string>()(respone.GetTypeName()));
        svrPkg.mutable_rpc_info()->set_req_server_type(serverType);
        svrPkg.mutable_rpc_info()->set_req_bus_id(pConfig->BusId);
        svrPkg.mutable_rpc_info()->set_is_script_rpc(false);

        SendMsgToServer(serverType, dstServerType, pConfig->BusId, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD, svrPkg, param1, param2);

        int32_t iRet = FindModule<NFICoroutineModule>()->SetUserData(&respone);
        CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));

        iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS);

        FindModule<NFICoroutineModule>()->SetUserData(NULL);

        CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{} rpc Msg Id:{} request:{} respone:{}", GetErrorStr(iRet), msgId, request.GetTypeName(),
                   respone.GetTypeName());
        return iRet;
    }

    // ========================================================================
    // RPC服务调用接口 - 异步版本（回调）
    // ========================================================================
    
    /**
     * @brief 异步RPC调用（回调版本 - 基础）
     * @tparam msgId 消息ID，编译时常量
     * @tparam RequestType 请求消息类型，必须继承自google::protobuf::Message
     * @tparam ResponFunc 响应回调函数类型
     * @param serverType 发送方服务器类型
     * @param dstServerType 目标服务器类型
     * @param dstBusId 目标服务器总线ID
     * @param request 请求消息对象
     * @param rpcCb 响应回调函数，签名：void(int retCode, ResponeType& response)
     * @param param1 自定义参数1，默认为0
     * @param param2 自定义参数2，默认为0
     * @param is_immediately 是否立即执行，默认true
     * @return 返回RPC请求ID，可用于取消请求；-1表示失败
     * 
     * @note 该方法会自动创建协程来处理RPC调用，不能在其他协程内调用
     * @warning 不要在已有协程上下文中调用此方法，会导致协程嵌套问题
     * @details 该方法适用于非阻塞的RPC调用场景：
     *          - 自动创建独立协程处理RPC
     *          - 通过回调函数异步接收响应
     *          - 支持超时和错误处理
     *          - 可选择立即执行或延迟执行
     */
    template <size_t msgId, typename RequestType, typename ResponFunc>
    int64_t GetRpcService(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, const RequestType& request, const ResponFunc& rpcCb,
                          uint64_t param1 = 0, uint64_t param2 = 0, bool is_immediately = true)
    {
        return GetRpcServiceInner<NF_MODULE_SERVER, msgId>(serverType, dstServerType, dstBusId, request, rpcCb, &ResponFunc::operator(), param1, param2, is_immediately);
    }

    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponFunc>
    int64_t GetRpcService(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, const RequestType& request, const ResponFunc& rpcCb,
                          uint64_t param1 = 0, uint64_t param2 = 0, bool is_immediately = true)
    {
        return GetRpcServiceInner<moduleId, msgId>(serverType, dstServerType, dstBusId, request, rpcCb, &ResponFunc::operator(), param1, param2, is_immediately);
    }

    int GetScriptRpcService(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, uint32_t msgId, const std::string& reqType,
                            const std::string& request, const std::string& rspType, std::string& respone)
    {
        NF_ASSERT_MSG(FindModule<NFICoroutineModule>()->IsInCoroutine(), "Call GetScriptRpcService Must Int the Coroutine");
        NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(serverType);
        CHECK_EXPR(pConfig, -1, "can't find server config! servertype:{}", GetServerName(serverType));

        NFrame::Proto_FramePkg svrPkg;
        svrPkg.set_module_id(NF_MODULE_SERVER);
        svrPkg.set_msg_id(msgId);
        svrPkg.set_msg_data(request);
        svrPkg.mutable_rpc_info()->set_req_rpc_id(FindModule<NFICoroutineModule>()->CurrentTaskId());
        svrPkg.mutable_rpc_info()->set_req_rpc_hash(NFHash::hash<std::string>()(reqType));
        svrPkg.mutable_rpc_info()->set_rsp_rpc_hash(NFHash::hash<std::string>()(rspType));
        svrPkg.mutable_rpc_info()->set_req_server_type(serverType);
        svrPkg.mutable_rpc_info()->set_req_bus_id(pConfig->BusId);
        svrPkg.mutable_rpc_info()->set_is_script_rpc(true);

        SendMsgToServer(serverType, dstServerType, pConfig->BusId, dstBusId, NF_MODULE_FRAME, NFrame::NF_SERVER_TO_SERVER_RPC_CMD, svrPkg);
        NFrame::Proto_ScriptRpcResult result;
        result.set_req_type(reqType);
        result.set_rsp_type(rspType);
        int32_t iRet = FindModule<NFICoroutineModule>()->SetUserData(&result);
        CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));

        iRet = FindModule<NFICoroutineModule>()->Yield(DEFINE_RPC_SERVICE_TIME_OUT_MS);

        respone = result.respone();
        FindModule<NFICoroutineModule>()->SetUserData(NULL);

        CHECK_EXPR(iRet == 0, iRet, "Yield Failed, Error:{}", GetErrorStr(iRet));
        return iRet;
    }

    int64_t GetScriptRpcService(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, uint32_t msgId, const std::string& reqType,
                                const std::string& request, const std::string& rspType,
                                const std::function<void(int rpcRetCode, const std::string& rspType, std::string& respone)>& func)
    {
        return FindModule<NFICoroutineModule>()->MakeCoroutine(
            [=]()
            {
                std::string respone;
                int iRet = FindModule<NFIMessageModule>()->GetScriptRpcService(serverType, dstServerType, dstBusId, msgId, reqType, request,
                                                                               rspType, respone);
                if (func)
                {
                    func(iRet, rspType, respone);
                }
            });
    }

private:
    /**
     * @brief 这个函数会先创建一个协程， 获取远程服务器的rpc服务，不能在别的协程里调用这个函数
     * @tparam RequestType
     * @tparam ResponeType
     * @param serverType
     * @param dstServerType
     * @param dstBusId
     * @param nMsgId
     * @param request
     * @param rpcCb
     * @return
     */
    template <size_t moduleId, size_t msgId, typename RequestType, typename ResponFunc, typename ResponeType>
    int64_t GetRpcServiceInner(NF_SERVER_TYPE serverType, NF_SERVER_TYPE dstServerType, uint32_t dstBusId, const RequestType& request,
                               const ResponFunc& responFunc, void (ResponFunc::*pf)(int rpcRetCode, ResponeType& respone) const, uint64_t param1 = 0,
                               uint64_t param2 = 0, bool is_immediately = true)
    {
        return FindModule<NFICoroutineModule>()->MakeCoroutine(
            [=]()
            {
                ResponeType respone;
                int iRet = FindModule<NFIMessageModule>()->GetRpcService<moduleId, msgId>(serverType,
                                                                                          dstServerType,
                                                                                          dstBusId, request,
                                                                                          respone, param1, param2);
                (responFunc.*pf)(iRet, respone);
            }, is_immediately);
    }

public:
    /**
     * @brief 添加rpc服务
     * @param serverType
     * @param pBase
     * @param pRpcService
     * @return
     */
    virtual bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nMsgID, NFIDynamicModule* pBase, NFIRpcService* pRpcService, bool createCo = false) = 0;

    // 添加RPC服务的接口
    // 本函数的目的是在指定的服务器类型和模块ID下注册一个新的RPC服务
    // 这个接口允许从动态模块向特定类型的服务器添加RPC服务，从而实现模块间的远程过程调用
    // 参数:
    //   serverType: 服务器类型，指定了RPC服务所属的服务器类别
    //   nModuleID: 模块ID，唯一标识了RPC服务所属的模块
    //   nMsgID: 消息ID，用于标识具体的RPC方法
    //   pBase: 指向动态模块的指针，表示添加RPC服务的动态模块
    //   pRpcService: 指向RPC服务接口的指针，用于实现RPC方法的调用
    //   createCo: 是否创建协程，用于支持异步操作，默认为false
    // 返回值:
    //   bool: 表示添加RPC服务是否成功
    virtual bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nModuleID, uint32_t nMsgID, NFIDynamicModule* pBase, NFIRpcService* pRpcService, bool createCo = false) = 0;

    virtual bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nMsgID, NFIRpcService* pRpcService, bool createCo = false) = 0;

    virtual bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nModuleID, uint32_t nMsgID, NFIRpcService* pRpcService, bool createCo = false) = 0;

public:
    /**
     * @brief 添加服务器
     *
     * @param  eType		服务器类型
     * @param  nServerID	服务器ID
     * @param  nMaxClient	服务器最大连接客户端数
     * @param  nPort		服务器监听端口
     * @return int			返回0错误
     */
    virtual uint64_t BindServer(NF_SERVER_TYPE eServerType, const std::string& url, uint32_t nNetThreadNum = 1, uint32_t nMaxConnectNum = 100,
                                uint32_t nPacketParseType = PACKET_PARSE_TYPE_INTERNAL, bool bSecurity = false) = 0;

    /**
     * 重置并初始化解析包。
     *
     * 本函数旨在根据指定的解析类型和解析包对象，进行重置和初始化操作，以确保数据包的解析过程正确进行。
     *
     * @param parseType 解析类型，一个无符号32位整数，用于指定解析的类型或模式。
     * @param pPacketParse 指向NFIPacketParse对象的指针，表示要进行重置和初始化的数据包对象。
     * @return 返回一个整数值，表示操作的结果，具体含义取决于实现。
     */
    virtual int ResetPacketParse(uint32_t parseType, NFIPacketParse* pPacketParse) = 0;

    /**
     * @brief 添加服务器
     *
     * @param  eType		服务器类型
     * @param  nServerID	服务器ID
     * @param  nMaxClient	服务器最大连接客户端数
     * @param  nPort		服务器监听端口
     * @return int			返回0错误
     */
    virtual uint64_t ConnectServer(NF_SERVER_TYPE eServerType, const std::string& url, uint32_t nPacketParseType = 0, bool bSecurity = false) = 0;

    virtual int ResumeConnect(NF_SERVER_TYPE eServerType) = 0;

    virtual std::string GetLinkIp(uint64_t usLinkId) = 0;

    virtual uint32_t GetPort(uint64_t usLinkId) = 0;

    virtual void CloseLinkId(uint64_t usLinkId) = 0;

    virtual void CloseServer(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServer, uint32_t busId, uint64_t usLinkId) = 0;

    virtual void TransPackage(uint64_t usLinkId, NFDataPackage& packet) = 0;

    virtual void OnHandleMessage(NFDataPackage& packet) = 0;

    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg, uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void Send(uint64_t usLinkId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0)
    {
        Send(usLinkId, NF_MODULE_SERVER, nMsgID, strData, param1, param2);
    }

    virtual void Send(uint64_t usLinkId, uint32_t nMsgID, const char* msg, uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0)
    {
        Send(usLinkId, NF_MODULE_SERVER, nMsgID, msg, nLen, param1, param2);
    }

    virtual void Send(uint64_t usLinkId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0)
    {
        Send(usLinkId, NF_MODULE_SERVER, nMsgID, xData, param1, param2);
    }

    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg, uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual int SendMsgToServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0) = 0;

    virtual int SendMsgToServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nModuleId, uint32_t nMsgId, const std::string& xData, uint64_t param1 = 0, uint64_t param2 = 0) = 0;

    virtual int SendMsgToServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0)
    {
        return SendMsgToServer(eSendType, recvType, srcBusId, dstBusId, NF_MODULE_SERVER, nMsgId, xData, param1, param2);
    }

    virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nMsgID, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) = 0;

    virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nMsgID, const std::string& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) = 0;

    virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t moduleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) = 0;

    virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t moduleId, uint32_t nMsgID, const std::string& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetServerByServerId(NF_SERVER_TYPE eSendType, uint32_t busId) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetServerByUnlinkId(NF_SERVER_TYPE eSendType, uint64_t unlinkId) = 0;

    virtual NF_SHARE_PTR<NFServerData> CreateServerByServerId(NF_SERVER_TYPE eSendType, uint32_t busId, NF_SERVER_TYPE busServerType, const NFrame::ServerInfoReport& data) = 0;

    virtual void CreateLinkToServer(NF_SERVER_TYPE eSendType, uint32_t busId, uint64_t linkId) = 0;

    virtual void DelServerLink(NF_SERVER_TYPE eSendType, uint64_t linkId) = 0;

    virtual NFServerData* GetRouteData(NF_SERVER_TYPE eSendType) = 0;

    virtual const NFServerData* GetRouteData(NF_SERVER_TYPE eSendType) const = 0;

    virtual NFServerData* GetMasterData(NF_SERVER_TYPE eSendType) = 0;

    virtual const NFServerData* GetMasterData(NF_SERVER_TYPE eSendType) const = 0;

    virtual void CloseAllLink(NF_SERVER_TYPE eSendType) = 0;

    virtual uint64_t GetServerLinkId(NF_SERVER_TYPE eSendType) const = 0;

    virtual void SetServerLinkId(NF_SERVER_TYPE eSendType, uint64_t linkId) = 0;

    virtual uint64_t GetClientLinkId(NF_SERVER_TYPE eSendType) const = 0;

    virtual void SetClientLinkId(NF_SERVER_TYPE eSendType, uint64_t linkId) = 0;

    virtual std::vector<NF_SHARE_PTR<NFServerData>> GetServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetFirstServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetFirstServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, bool crossServer) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetRandomServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetRandomServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, bool crossServer) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, uint64_t value) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, uint64_t value, bool crossServer) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, const std::string& value) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, const std::string& value, bool crossServer) = 0;

    virtual std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE eSendType) = 0;

    virtual std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) = 0;

    virtual std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, bool isCrossServer) = 0;

    virtual std::vector<std::string> GetDBNames(NF_SERVER_TYPE eSendType) = 0;

    virtual std::set<uint32_t> GetAllMsg(NF_SERVER_TYPE eSendType, uint32_t moduleId) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetFirstDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName) = 0;

    virtual NF_SHARE_PTR<NFServerData> GeRandomDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetSuitDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName, uint64_t value) = 0;

    virtual NF_SHARE_PTR<NFServerData> GetSuitDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName, const std::string& value) = 0;

public:
    virtual int BroadcastEventToServer(NF_SERVER_TYPE eType, NF_SERVER_TYPE recvType, uint32_t dstBusId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID,
                                       const google::protobuf::Message& message) = 0;

    virtual int BroadcastEventToServer(NF_SERVER_TYPE eType, NF_SERVER_TYPE recvType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID,
                                       const google::protobuf::Message& message) = 0;

    virtual int BroadcastEventToAllServer(NF_SERVER_TYPE eType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID,
                                          const google::protobuf::Message& message) = 0;

    virtual int BroadcastEventToAllServer(NF_SERVER_TYPE eType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID,
                                          const google::protobuf::Message& message) = 0;

public:
    virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, const NFIHttpHandle& req, const std::string& strMsg,
                                 NFWebStatus code = NFWebStatus::WEB_OK, const std::string& reason = "OK") = 0;

    virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, uint64_t requestId, const std::string& strMsg,
                                 NFWebStatus code = NFWebStatus::WEB_OK,
                                 const std::string& reason = "OK") = 0;

    virtual int HttpGet(NF_SERVER_TYPE serverType, const std::string& strUri,
                        const HTTP_CLIENT_RESPONE& respone,
                        const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(),
                        int timeout = 3) = 0;

    virtual int HttpPost(NF_SERVER_TYPE serverType, const std::string& strUri, const std::string& strPostData, const HTTP_CLIENT_RESPONE& respone,
                         const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(),
                         int timeout = 3) = 0;

    virtual int SendEmail(NF_SERVER_TYPE serverType, const std::string& title, const std::string& subject, const string& content) = 0;

    virtual int SendWxWork(NF_SERVER_TYPE serverType, const string& content) = 0;

public:
    virtual bool AddHttpMsgCB(NF_SERVER_TYPE serverType, const std::string& strCommand, const NFHttpType eRequestType, const HTTP_RECEIVE_FUNCTOR& cb) = 0;

    virtual bool AddHttpOtherMsgCB(NF_SERVER_TYPE serverType, const NFHttpType eRequestType, const HTTP_RECEIVE_FUNCTOR& cb) = 0;

    virtual bool AddHttpFilterCB(NF_SERVER_TYPE serverType, const std::string& strCommand, const HTTP_FILTER_FUNCTOR& cb) = 0;

public:
    /**
     * @brief 删除目标模块的所有注册回调
     * @param pTarget 目标动态模块指针
     * @return 删除成功返回true，失败返回false
     * 
     * 清理指定模块注册的所有消息回调、事件回调和其他回调，
     * 用于模块卸载或重置时的资源清理。
     */
    virtual bool DelAllCallBack(NFIDynamicModule* pTarget) = 0;

    /**
     * @brief 删除指定连接的所有回调
     * @param eType 服务器类型
     * @param unLinkId 网络连接ID
     * @return 删除成功返回true，失败返回false
     * 
     * 清理指定网络连接的所有回调函数，
     * 用于连接断开时的资源清理。
     */
    virtual bool DelAllCallBack(NF_SERVER_TYPE eType, uint64_t unLinkId) = 0;

    /**
     * @brief 添加消息回调处理器（模块ID为0）
     * @param eType 服务器类型
     * @param nMsgID 消息ID
     * @param pTarget 目标模块指针
     * @param cb 回调函数对象
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定消息ID添加回调处理器，每个消息只能有一个处理函数。
     * 模块ID为0表示使用默认模块。
     */
    virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nMsgID, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) = 0;

    /**
     * @brief 添加模块化消息回调处理器
     * @param eType 服务器类型
     * @param nModuleId 模块ID
     * @param nMsgID 消息ID
     * @param pTarget 目标模块指针
     * @param cb 回调函数对象
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定模块ID和消息ID添加回调处理器，
     * 支持模块化的消息路由和处理。
     */
    virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) = 0;

    /**
     * @brief 添加静态消息回调处理器（模块ID为0）
     * @param eType 服务器类型
     * @param nMsgID 消息ID
     * @param cb 回调函数对象
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定消息ID添加静态回调处理器，
     * 不需要关联特定的模块对象。
     */
    virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nMsgID, const NET_RECEIVE_FUNCTOR& cb, bool createCo) = 0;

    /**
     * @brief 添加模块化静态消息回调处理器
     * @param eType 服务器类型
     * @param nModuleId 模块ID
     * @param nMsgID 消息ID
     * @param cb 回调函数对象
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 为指定模块ID和消息ID添加静态回调处理器，
     * 支持模块化的静态消息处理。
     */
    virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, const NET_RECEIVE_FUNCTOR& cb, bool createCo) = 0;

    /**
     * @brief 添加其他消息的统一回调处理器
     * @param eType 服务器类型
     * @param linkId 连接ID
     * @param pTarget 目标模块指针
     * @param cb 回调函数对象
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 为未注册过的消息添加统一处理回调函数，
     * 用于处理未知或动态消息类型。
     */
    virtual bool AddOtherCallBack(NF_SERVER_TYPE eType, uint64_t linkId, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) = 0;

    /**
     * @brief 添加全局消息回调处理器
     * @param eType 服务器类型
     * @param pTarget 目标模块指针
     * @param cb 回调函数对象
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 对所有消息添加统一的回调处理器。
     * 通过回调函数的返回值判断是否处理消息：
     * - 返回0：处理该消息
     * - 返回非0：不处理该消息
     */
    virtual bool AddAllMsgCallBack(NF_SERVER_TYPE eType, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) = 0;

    /**
     * @brief 添加网络事件回调处理器
     * @param eType 服务器类型
     * @param linkId 连接ID
     * @param pTarget 目标模块指针
     * @param cb 事件回调函数对象
     * @param createCo 是否创建协程处理
     * @return 注册成功返回true，失败返回false
     * 
     * 添加连接事件和断线事件的处理函数，
     * 用于监控网络连接状态变化。
     */
    virtual bool AddEventCallBack(NF_SERVER_TYPE eType, uint64_t linkId, NFIDynamicModule* pTarget, const NET_EVENT_FUNCTOR& cb, bool createCo) = 0;
};
