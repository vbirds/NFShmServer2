// -------------------------------------------------------------------------
//    @FileName         :    NFIHttpHandle.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIHttpHandle.h
//    @Description      :    HTTP处理接口定义，提供HTTP请求和响应的处理功能
//                           支持多种HTTP方法、状态码管理和回调机制
//
// -------------------------------------------------------------------------

#pragma once

#include <map>
#include <functional>


#include <map>
#include <functional>
#include <vector>

#include "NFComm/NFCore/NFSlice.hpp"

#if NF_PLATFORM == NF_PLATFORM_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#else

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <atomic>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif

/**
 * @brief HTTP响应状态码枚举
 * 
 * 定义了Web服务中常用的HTTP状态码，用于标识请求处理结果。
 * 遵循HTTP标准状态码规范。
 */
enum NFWebStatus
{
    WEB_OK = 200,           ///< 请求成功
    WEB_AUTH = 401,         ///< 未授权，需要身份验证
    WEB_ERROR = 404,        ///< 未找到资源
    WEB_INTER_ERROR = 500,  ///< 服务器内部错误
    WEB_TIMEOUT = 503,      ///< 服务不可用，通常表示超时
};

/**
 * @brief HTTP请求方法类型枚举
 * 
 * 定义了支持的HTTP请求方法，使用位掩码方式定义，
 * 支持多种方法的组合使用。
 */
enum NFHttpType
{
    NF_HTTP_REQ_GET = 1 << 0,       ///< GET方法，用于获取资源
    NF_HTTP_REQ_POST = 1 << 1,      ///< POST方法，用于提交数据
    NF_HTTP_REQ_HEAD = 1 << 2,      ///< HEAD方法，用于获取资源头信息
    NF_HTTP_REQ_PUT = 1 << 3,       ///< PUT方法，用于更新资源
    NF_HTTP_REQ_DELETE = 1 << 4,    ///< DELETE方法，用于删除资源
    NF_HTTP_REQ_OPTIONS = 1 << 5,   ///< OPTIONS方法，用于获取服务器支持的方法
    NF_HTTP_REQ_TRACE = 1 << 6,     ///< TRACE方法，用于诊断目的
    NF_HTTP_REQ_CONNECT = 1 << 7,   ///< CONNECT方法，用于建立隧道
    NF_HTTP_REQ_PATCH = 1 << 8      ///< PATCH方法，用于部分更新资源
};

class NFCHttpServer;

/**
 * @brief HTTP请求处理接口类
 * 
 * NFIHttpHandle 提供了HTTP请求和响应处理的抽象接口：
 * 
 * 1. 请求信息获取：
 *    - URL、路径、主机信息
 *    - 请求类型和请求体
 *    - 查询参数解析
 * 
 * 2. 响应处理：
 *    - 响应头设置
 *    - 响应消息发送
 *    - 状态码管理
 * 
 * 3. 会话管理：
 *    - 请求ID追踪
 *    - 超时时间管理
 *    - 连接状态重置
 * 
 * 设计特点：
 * - 抽象的HTTP处理接口
 * - 支持同步和异步处理
 * - 完整的HTTP协议支持
 * - 易于扩展和定制
 */
class NFIHttpHandle
{
public:
    /**
     * @brief 默认构造函数
     */
    NFIHttpHandle() = default;

    /**
     * @brief 虚析构函数
     * 
     * 确保派生类能够正确析构。
     */
    virtual ~NFIHttpHandle() = default;

    /**
     * @brief 重置HTTP处理器状态
     * 
     * 清理当前请求的所有状态信息，准备处理下一个请求。
     * 通常在请求处理完成后调用。
     */
    virtual void Reset() = 0;
    
    /**
     * @brief 获取原始URI
     * @return 返回完整的原始URI字符串
     * 
     * 获取客户端发送的原始URI，包含所有查询参数和片段信息。
     */
    virtual std::string GetOriginalUri() const = 0;
    
    /**
     * @brief 获取请求URL
     * @return 返回请求的URL字符串引用
     * 
     * 获取解析后的请求URL，通常不包含查询参数。
     */
    virtual const std::string& GetUrl() const = 0;
    
    /**
     * @brief 获取请求路径
     * @return 返回请求路径字符串引用
     * 
     * 获取URL中的路径部分，不包含域名和查询参数。
     */
    virtual const std::string& GetPath() const = 0;
    
    /**
     * @brief 获取远程主机地址
     * @return 返回客户端IP地址字符串引用
     * 
     * 获取发起请求的客户端IP地址。
     */
    virtual const std::string& GetRemoteHost() const = 0;
    
    /**
     * @brief 获取请求类型
     * @return 返回HTTP方法类型，见NFHttpType枚举
     * 
     * 获取当前请求的HTTP方法类型。
     */
    virtual int GetType() const = 0;
    
    /**
     * @brief 获取请求体内容
     * @return 返回请求体字符串
     * 
     * 获取HTTP请求的消息体内容，通常用于POST、PUT等方法。
     */
    virtual std::string GetBody() const = 0;
    
    /**
     * @brief 获取请求ID
     * @return 返回唯一的请求标识符
     * 
     * 获取当前请求的唯一标识符，用于请求跟踪和日志记录。
     */
    virtual uint64_t GetRequestId() const = 0;
    
    /**
     * @brief 获取超时时间
     * @return 返回超时时间（毫秒）
     * 
     * 获取当前请求的超时时间设置。
     */
    virtual uint64_t GetTimeOut() const = 0;

    /**
     * @brief 添加响应头
     * @param key 响应头名称
     * @param value 响应头值
     * 
     * 向HTTP响应中添加自定义头信息。必须在发送响应消息之前调用。
     * 
     * 使用示例：
     * @code
     * AddResponseHeader("Content-Type", "application/json");
     * AddResponseHeader("Cache-Control", "no-cache");
     * @endcode
     */
    virtual void AddResponseHeader(const std::string& key, const std::string& value) const = 0;
    
    /**
     * @brief 发送响应消息
     * @param strMsg 响应消息内容
     * @param code 响应状态码
     * @param strReason 状态码描述，默认为"OK"
     * @return 发送成功返回true，失败返回false
     * 
     * 向客户端发送HTTP响应消息。调用此方法后，当前请求处理完成。
     * 
     * 使用示例：
     * @code
     * ResponseMsg("{\"status\":\"success\"}", WEB_OK, "Success");
     * ResponseMsg("Not Found", WEB_ERROR, "Resource Not Found");
     * @endcode
     */
    virtual bool ResponseMsg(const std::string& strMsg, NFWebStatus code, const std::string& strReason = "OK") const = 0;
    
    /**
     * @brief 获取查询参数
     * @param query_key 查询参数名称
     * @return 返回对应的参数值，未找到返回空字符串
     * 
     * 从URL查询字符串中获取指定参数的值。
     * 
     * 使用示例：
     * @code
     * // 对于URL: /api/user?id=123&name=test
     * std::string userId = GetQuery("id");     // 返回 "123"
     * std::string userName = GetQuery("name"); // 返回 "test"
     * @endcode
     */
    virtual std::string GetQuery(const std::string& query_key) const = 0;
};

/// @brief HTTP请求接收处理函数类型
/// @param serverType 服务器类型
/// @param req HTTP请求处理接口
/// @return 处理成功返回true，失败返回false
typedef std::function<bool(uint32_t serverType, const NFIHttpHandle& req)> HTTP_RECEIVE_FUNCTOR;

/// @brief HTTP请求过滤函数类型
/// @param serverType 服务器类型
/// @param req HTTP请求处理接口
/// @return 返回过滤结果状态码，WEB_OK表示通过过滤
typedef std::function<NFWebStatus(uint32_t serverType, const NFIHttpHandle& req)> HTTP_FILTER_FUNCTOR;

/// @brief HTTP客户端响应回调函数类型
/// @param code 响应状态码
/// @param resp 响应内容
typedef std::function<void(int code, const std::string& resp)> HTTP_CLIENT_RESPONE;


