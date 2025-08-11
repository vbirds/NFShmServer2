// -------------------------------------------------------------------------
//    @FileName         :    NFCHttpServer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCHttpServer
//    @Desc             :    基于Evpp库实现的HTTP服务器类。
//                          该文件定义了基于evpp库的HTTP服务器，包括HTTP服务器类定义、
//                          HTTP请求处理句柄类、HTTP消息处理类、服务器初始化和生命周期管理、
//                          HTTP请求和响应处理。
//                          主要特性包括基于evpp库的高性能HTTP服务器、支持多线程并发处理、
//                          可配置的SSL/TLS支持、灵活的回调机制、对象池管理优化性能
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIHttpHandle.h"
#include "evpp/http/http_server.h"
#include "NFComm/NFCore/NFConcurrentQueue.h"
#include "NFComm/NFPluginModule/NFObjectPool.hpp"
#include <unordered_map>

/**
 * @class NFServerHttpHandle
 * @brief HTTP请求处理句柄类
 * 
 * 该类实现了NFIHttpHandle接口，用于封装HTTP请求的相关信息
 * 提供了获取请求URL、参数、Body等功能
 * 
 * 主要功能：
 * - 封装HTTP请求上下文
 * - 提供请求信息访问接口
 * - 支持请求参数解析
 * - 管理请求生命周期
 * 
 * 使用方式：
 * - 通过AllocHttpRequest()分配句柄
 * - 使用各种Get方法获取请求信息
 * - 调用ResponseMsg()发送响应
 * - 使用完毕后自动回收
 */
class NFServerHttpHandle final : public NFIHttpHandle
{
public:
    /**
     * @brief 构造函数
     */
    NFServerHttpHandle();

    /**
     * @brief 重置句柄状态
     */
    void Reset() override;

    /**
     * @brief 获取原始URI
     * @return 原始URI字符串
     */
    std::string GetOriginalUri() const override { return m_ctx->original_uri(); }

    /**
     * @brief 获取请求URL
     * @return URL字符串
     */
    const std::string& GetUrl() const override { return m_ctx->uri(); }

    /**
     * @brief 获取请求路径
     * @return 路径字符串
     */
    const std::string& GetPath() const override { return m_ctx->uri(); }

    /**
     * @brief 获取远程主机地址
     * @return 远程主机IP地址
     */
    const std::string& GetRemoteHost() const override { return m_ctx->remote_ip(); }

    /**
     * @brief 获取请求类型
     * @return 请求类型
     */
    int GetType() const override { return m_type; }

    /**
     * @brief 获取请求体内容
     * @return 请求体字符串
     */
    std::string GetBody() const override { return m_ctx->body().ToString(); }

    /**
     * @brief 获取请求ID
     * @return 请求ID
     */
    uint64_t GetRequestId() const override { return m_requestId; }

    /**
     * @brief 获取超时时间
     * @return 超时时间
     */
    uint64_t GetTimeOut() const override { return m_timeOut; }

    /**
     * @brief 添加响应头
     * @param key 头字段名
     * @param value 头字段值
     */
    void AddResponseHeader(const std::string& key, const std::string& value) const override;

    /**
     * @brief 发送响应消息
     * @param strMsg 响应消息内容
     * @param code HTTP状态码
     * @param strReason 状态原因
     * @return true 发送成功，false 发送失败
     */
    bool ResponseMsg(const std::string& strMsg, NFWebStatus code, const std::string& strReason = "OK") const override;

    /**
     * @brief 获取查询参数
     * @param queryKey 查询参数名
     * @return 查询参数值
     */
    std::string GetQuery(const std::string& queryKey) const override;

    NFHttpType m_type;                           ///< HTTP请求类型
    uint64_t m_requestId;                        ///< 请求ID
    uint64_t m_timeOut;                          ///< 超时时间
    evpp::http::ContextPtr m_ctx;                ///< HTTP上下文指针
    evpp::http::HTTPSendResponseCallback m_responseCb; ///< 响应回调函数
};

/**
 * @class NFEvppHttMsg
 * @brief Evpp HTTP消息类
 * 
 * 用于封装evpp库的HTTP消息，包括上下文和响应回调
 */
class NFEvppHttMsg final
{
public:
    /**
     * @brief 构造函数
     */
    NFEvppHttMsg();

    /**
     * @brief 析构函数
     */
    ~NFEvppHttMsg();

    /**
     * @brief 拷贝构造函数
     * @param msg 源消息对象
     */
    NFEvppHttMsg(const NFEvppHttMsg& msg);

    /**
     * @brief 赋值操作符
     * @param msg 源消息对象
     * @return 当前对象引用
     */
    NFEvppHttMsg& operator=(const NFEvppHttMsg& msg);

    /**
     * @brief 清空消息内容
     */
    void Clear();

    evpp::http::ContextPtr m_ctx;                ///< HTTP上下文指针
    evpp::http::HTTPSendResponseCallback m_responseCb; ///< 响应回调函数
};

/**
 * @class NFCHttpServer
 * @brief 基于Evpp库的HTTP服务器实现类
 * 
 * 该类基于evpp库实现高性能HTTP服务器功能，支持多线程并发处理
 * 
 * 主要功能：
 * - HTTP服务器监听和连接管理
 * - 多线程并发请求处理
 * - HTTP请求和响应处理
 * - SSL/TLS安全连接支持
 * - 对象池优化性能
 * 
 * 特性：
 * - 基于evpp库的高性能HTTP服务器
 * - 支持多线程并发处理
 * - 可配置的SSL/TLS支持
 * - 灵活的回调机制
 * - 对象池管理优化性能
 * 
 * 使用方式：
 * - 创建服务器实例并初始化
 * - 设置接收和过滤回调函数
 * - 监听指定端口
 * - 处理HTTP请求和响应
 */
class NFCHttpServer final
{
public:
    /**
     * @brief 构造函数：初始化HTTP服务器
     * @param serverType 服务器类型，用于指定服务器的具体类型
     * @param netThreadNum 网络线程数量，用于指定服务器处理网络请求的线程数
     */
    NFCHttpServer(uint32_t serverType, uint32_t netThreadNum);

    /**
     * @brief 析构函数：释放HTTP服务器占用的资源
     */
    ~NFCHttpServer();

    /**
     * @brief 执行服务器主循环
     * @return true 执行成功，false 执行失败
     */
    bool Execute();

    /**
     * @brief 获取服务器类型
     * @return 服务器类型
     */
    uint32_t GetServerType() const;

    /**
     * @brief 初始化服务器，使其监听指定端口
     * @param listenPort 要监听的端口号
     * @return bool 返回初始化结果，true表示成功，false表示失败
     */
    bool InitServer(int listenPort) const;

    /**
     * @brief 初始化服务器，使其监听多个指定端口
     * @param listenPorts 要监听的端口号列表
     * @return bool 返回初始化结果，true表示成功，false表示失败
     */
    bool InitServer(const std::vector<int>& listenPorts) const;

    /**
     * @brief 初始化服务器，使其监听通过逗号分隔的端口列表
     * @param listenPorts 要监听的端口字符串，格式如"80,8080,443"
     * @return bool 返回初始化结果，true表示成功，false表示失败
     */
    bool InitServer(const std::string& listenPorts/*like "80,8080,443"*/) const;

#if defined(EVPP_HTTP_SERVER_SUPPORTS_SSL)
    /**
     * @brief 对指定监听端口设置SSL选项
     * @param listen_port 监听的端口
     * @param enable_ssl 是否开启SSL支持
     * @param certificate_chain_file 证书链文件
     * @param private_key_file 私钥文件
     */
    void SetPortSSLOption(int listen_port,
                          bool enable_ssl,
                          const char* certificate_chain_file = "",
                          const char* private_key_file = "");

    /**
     * @brief 设置端口默认SSL配置选项
     * @param enable_ssl 是否开启SSL支持
     * @param certificate_chain_file 证书链文件
     * @param private_key_file 私钥文件
     */
    void SetPortSSLDefaultOption(
            bool enable_ssl,
            const char* certificate_chain_file = "",
            const char* private_key_file = "");
#endif

    /**
     * @brief 处理消息的逻辑线程函数
     *
     * 该函数负责处理消息的逻辑，通常在独立的线程中运行，以确保消息处理的并发性和高效性。
     * 具体的消息处理逻辑需要在该函数中实现。
     */
    void ProcessMsgLogicThread();

    /**
     * @brief 分配HTTP请求处理句柄
     *
     * 该函数用于分配一个新的HTTP请求处理句柄。返回的句柄可以用于处理HTTP请求和响应。
     *
     * @return NFServerHttpHandle* 返回一个指向新分配的HTTP请求处理句柄的指针。
     */
    NFServerHttpHandle* AllocHttpRequest();

    /**
     * @brief 响应HTTP消息，根据NFIHttpHandle
     *
     * 该函数用于根据给定的NFIHttpHandle对象响应HTTP消息。它将指定的消息、状态码和原因短语发送给客户端。
     *
     * @param req 指向NFIHttpHandle对象的引用，表示当前的HTTP请求。
     * @param strMsg 要发送给客户端的消息内容。
     * @param code HTTP状态码，表示请求的处理结果。
     * @param strReason 可选参数，表示HTTP状态码的原因短语，默认为"OK"。
     * @return bool 返回true表示响应成功，返回false表示响应失败。
     */
    bool ResponseMsg(const NFIHttpHandle& req, const std::string& strMsg, NFWebStatus code, const std::string& strReason = "OK");

    /**
     * @brief 响应HTTP消息，根据请求ID
     *
     * 该函数用于根据给定的请求ID响应HTTP消息。它将指定的消息、状态码和原因短语发送给客户端。
     *
     * @param requestId 请求的唯一标识符，用于识别特定的HTTP请求。
     * @param strMsg 要发送给客户端的消息内容。
     * @param code HTTP状态码，表示请求的处理结果。
     * @param strReason 可选参数，表示HTTP状态码的原因短语，默认为"OK"。
     * @return bool 返回true表示响应成功，返回false表示响应失败。
     */
    bool ResponseMsg(uint64_t requestId, const std::string& strMsg, NFWebStatus code, const std::string& strReason = "OK");

public:
    /**
     * @brief 设置接收回调
     * @param pBaseType 基类指针
     * @param handleRecv 处理接收的函数指针
     */
    template <typename BaseType>
    void SetRecvCb(BaseType* pBaseType, bool (BaseType::*handleRecv)(uint32_t, const NFIHttpHandle& req))
    {
        m_receiveCb = std::bind(handleRecv, pBaseType, std::placeholders::_1, std::placeholders::_2);
    }

    /**
     * @brief 设置连接事件回调
     * @param pBaseType 基类指针
     * @param handleFilter 处理过滤的函数指针
     */
    template <typename BaseType>
    void SetFilterCb(BaseType* pBaseType, NFWebStatus (BaseType::*handleFilter)(uint32_t, const NFIHttpHandle& req))
    {
        m_filter = std::bind(handleFilter, pBaseType, std::placeholders::_1, std::placeholders::_2);
    }

    /**
     * @brief 设置接收回调函数
     * @param recvCb 接收回调函数
     */
    void SetRecvCb(const HTTP_RECEIVE_FUNCTOR& recvCb);

    /**
     * @brief 设置过滤回调函数
     * @param eventCb 过滤回调函数
     */
    void SetFilterCb(const HTTP_FILTER_FUNCTOR& eventCb);

private:
    /**
     * @brief HTTP服务器指针，用于管理和控制HTTP服务器的实例。
     */
    evpp::http::Server* m_pHttpServer;

private:
    /**
     * @brief 服务器监听的端口号。
     */
    uint32_t m_port;

    /**
     * @brief 服务器类型标识，用于区分不同类型的服务器。
     */
    uint32_t m_serverType;

    /**
     * @brief 服务器监听的端口号列表，支持多个端口监听。
     */
    std::vector<uint32_t> m_vecPort;

    /**
     * @brief 并发队列，用于存储待处理的HTTP消息。
     */
    NFConcurrentQueue<NFEvppHttMsg> m_msgQueue;

    /**
     * @brief 索引值，用于唯一标识HTTP请求。
     */
    uint64_t m_index;

    /**
     * @brief HTTP请求映射表，用于存储和管理HTTP请求的处理器。
     */
    std::unordered_map<uint64_t, NFServerHttpHandle*> m_httpRequestMap;

    /**
     * @brief HTTP请求处理器对象池，用于高效地管理和复用HTTP请求处理器。
     */
    NFObjectPool<NFServerHttpHandle>* m_listHttpRequestPool;

protected:
    /**
     * @brief HTTP接收回调函数，用于处理接收到的HTTP请求。
     */
    HTTP_RECEIVE_FUNCTOR m_receiveCb;

    /**
     * @brief HTTP过滤器函数，用于在接收HTTP请求前进行过滤或预处理。
     */
    HTTP_FILTER_FUNCTOR m_filter;
};

