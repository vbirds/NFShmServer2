// -------------------------------------------------------------------------
//    @FileName         :    NFCHttpClient.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCHttpClient
//    @Desc             :    基于Evpp库的HTTP客户端类，提供HTTP请求和响应处理功能
//
// -------------------------------------------------------------------------

#pragma once

#include <evpp/event_loop_thread.h>
#include <evpp/httpc/request.h>
#include <unordered_map>
#include "NFComm/NFCore/NFConcurrentQueue.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFObjectPool.hpp"

class NFCHttpClient;

/**
 * @file NFCHttpClient.h
 * @brief Evpp HTTP客户端头文件
 * 
 * 该文件定义了基于evpp库的HTTP客户端，包括：
 * - HTTP客户端类定义
 * - HTTP客户端消息类
 * - HTTP客户端参数类
 * - 客户端初始化和生命周期管理
 * - HTTP请求和响应处理
 * 
 * 主要特性：
 * - 基于evpp库的高性能HTTP客户端
 * - 支持异步请求和响应处理
 * - 自动超时检测和管理
 * - 线程安全的请求处理
 * - 对象池优化性能
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @struct NFHttpClientMsg
 * @brief HTTP客户端消息结构体
 * 
 * 用于存储HTTP响应的相关信息，包括响应体、状态码和请求ID
 */
class NFHttpClientMsg
{
public:
    /**
     * @brief 默认构造函数
     */
    NFHttpClientMsg();

    /**
     * @brief 清空消息内容
     */
    void Clear();

    /**
     * @brief 拷贝构造函数
     * @param msg 要拷贝的消息对象
     */
    NFHttpClientMsg(const NFHttpClientMsg& msg);

    /**
     * @brief 赋值操作符
     * @param msg 要赋值的消息对象
     * @return 当前对象引用
     */
    NFHttpClientMsg& operator=(const NFHttpClientMsg& msg);

    std::string m_body;  ///< 响应体内容
    int m_code;          ///< HTTP状态码
    int m_reqId;         ///< 请求ID
};

/**
 * @class NFCHttpClientParam
 * @brief HTTP客户端参数类
 * 
 * 用于存储HTTP请求的参数信息，包括请求ID、响应回调和超时时间
 */
class NFCHttpClientParam
{
public:
    /**
     * @brief 构造函数
     * @param id 请求ID
     * @param func 响应回调函数
     * @param timeout 超时时间（秒）
     */
    NFCHttpClientParam(int id, const HTTP_CLIENT_RESPONE& func, uint32_t timeout = 3);

    /**
     * @brief 析构函数
     */
    ~NFCHttpClientParam();

    /**
     * @brief 检查是否超时
     * @return true 已超时，false 未超时
     */
    bool IsTimeOut() const;

public:
    int m_id = 0;                    ///< 请求ID
    HTTP_CLIENT_RESPONE m_resp;      ///< 响应回调函数
    uint32_t m_timeout;              ///< 超时时间（秒）
};

/**
 * @class NFCHttpClient
 * @brief 基于Evpp库的HTTP客户端实现类
 * 
 * 该类基于libevent的evpp库实现高性能HTTP客户端功能
 * 主要用于HTTP请求的发送和响应处理
 * 
 * 主要功能：
 * - HTTP GET/POST请求发送
 * - 异步响应处理
 * - 请求超时管理
 * - 并发请求支持
 * - 响应回调处理
 * 
 * 特性：
 * - 基于libevent的高性能事件驱动
 * - 支持异步非阻塞I/O
 * - 自动超时检测
 * - 线程安全的请求处理
 * - 支持自定义请求头
 * 
 * 使用方式：
 * - 创建客户端实例
 * - 调用HttpGet()或HttpPost()发送请求
 * - 设置响应回调函数处理结果
 * - 监控请求状态和超时
 */
class NFCHttpClient final
{
public:
    /**
     * @brief 构造函数
     */
    NFCHttpClient();

    /**
     * @brief 析构函数
     */
    ~NFCHttpClient();

    /**
     * @brief 执行HTTP客户端
     * @return true 执行成功，false 执行失败
     */
    bool Execute();

    /**
     * @brief 处理消息逻辑线程
     */
    void ProcessMsgLogicThread();

public:
    /**
     * @brief 处理HTTP GET请求的响应
     *
     * @param response 共享的HTTP响应对象，包含响应的状态码、头信息和身体内容
     * @param request 发出的HTTP GET请求的指针，用于访问请求的详细信息
     */
    void HandleHttpGetResponse(const std::shared_ptr<evpp::httpc::Response>& response, const evpp::httpc::GetRequest* request);

    /**
     * @brief 处理HTTP POST请求的响应
     *
     * @param response 共享的HTTP响应对象，包含响应的状态码、头信息和身体内容
     * @param request 发出的HTTP POST请求的指针，用于访问请求的详细信息
     */
    void HandleHttpPostResponse(const std::shared_ptr<evpp::httpc::Response>& response, const evpp::httpc::PostRequest* request);

    /**
     * @brief 发起HTTP GET请求
     *
     * @param strUri 请求的URI地址
     * @param respone 回调函数，用于处理接收到的HTTP响应
     * @param xHeaders 请求的头信息，默认为空
     * @param timeout 请求的超时时间，默认为3秒
     *
     * @return 请求的结果代码
     */
    int HttpGet(const std::string& strUri,
                const HTTP_CLIENT_RESPONE& respone,
                const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(),
                int timeout = 3);

    /**
     * @brief 发起HTTP POST请求
     *
     * @param strUri 请求的URI地址
     * @param strPostData POST请求的数据内容
     * @param respone 回调函数，用于处理接收到的HTTP响应
     * @param xHeaders 请求的头信息，默认为空
     * @param timeout 请求的超时时间，默认为3秒
     *
     * @return 请求的结果代码
     */
    int HttpPost(const std::string& strUri, const std::string& strPostData, const HTTP_CLIENT_RESPONE& respone,
                 const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(),
                 int timeout = 3);

private:
    /**
     * @brief 存储HTTP客户端参数的哈希映射
     * 
     * 键为请求ID，值为HTTP客户端参数指针
     */
    std::unordered_map<int, NFCHttpClientParam*> m_httpClientMap;

    /**
     * @brief 事件循环线程
     * 
     * 用于处理HTTP客户端事件
     */
    evpp::EventLoopThread m_threadLoop;

    /**
     * @brief 消息队列
     * 
     * 用于存储和处理HTTP客户端消息
     */
    NFConcurrentQueue<NFHttpClientMsg> m_msgQueue;

    /**
     * @brief 对象池
     * 
     * 用于管理HTTP客户端参数对象的生命周期
     */
    NFObjectPool<NFCHttpClientParam>* m_pHttpClientParamPool;

    /**
     * @brief 静态请求ID
     * 
     * 用于标识和追踪HTTP请求
     */
    int m_staticReqId;
};
