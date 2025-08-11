// -------------------------------------------------------------------------
//    @FileName         :    NFINetModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Description      :    网络模块接口定义，提供网络通信、HTTP服务和数据包处理功能
//                           支持TCP/UDP连接、HTTP请求响应、消息序列化和回调机制
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFIModule.h"
#include "google/protobuf/message.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"

#include <stdint.h>

#include "NFIPacketParse.h"

/**
 * @brief 网络模块接口类
 * 
 * NFINetModule 提供了完整的网络通信解决方案：
 * 
 * 1. 网络连接管理：
 *    - TCP/UDP服务器的绑定和监听
 *    - 客户端连接的建立和维护
 *    - 连接状态监控和管理
 *    - 自动重连机制
 * 
 * 2. 消息处理框架：
 *    - 灵活的回调机制
 *    - 数据包的解析和序列化
 *    - 消息路由和分发
 *    - 模块化消息处理
 * 
 * 3. HTTP服务支持：
 *    - HTTP服务器功能
 *    - RESTful API支持
 *    - HTTP客户端请求
 *    - 过滤器和中间件
 * 
 * 4. 通信协议：
 *    - Protocol Buffers支持
 *    - 自定义数据包格式
 *    - 消息压缩和加密
 *    - 流量控制和优化
 * 
 * 5. 扩展功能：
 *    - 邮件发送服务
 *    - 文件传输支持
 *    - 多线程网络处理
 *    - 负载均衡支持
 * 
 * 设计特点：
 * - 事件驱动的异步IO
 * - 线程安全的设计
 * - 可配置的连接池
 * - 高性能的消息处理
 * - 灵活的协议扩展
 * 
 * 使用场景：
 * - 游戏服务器通信
 * - 微服务架构
 * - 实时消息推送
 * - API网关服务
 * - 分布式系统通信
 */
class NFINetModule: public NFIModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFINetModule(NFIPluginManager* p): NFIModule(p) {}
	
	/**
	 * @brief 析构函数
	 */
	virtual ~NFINetModule() {}

	/**
	 * @brief 设置接收消息回调（模板版本）
	 * @tparam BaseType 回调对象类型
	 * @param pBaseType 回调对象指针
	 * @param handleRecieve 接收消息的成员函数指针
	 * 
	 * 类型安全的接收回调设置接口，使用模板自动推导类型并绑定成员函数。
	 * 
	 * 使用示例：
	 * @code
	 * class MyHandler {
	 * public:
	 *     int OnReceive(uint64_t linkId, uint64_t objId, NFDataPackage& packet) {
	 *         // 处理接收到的消息
	 *         return 0;
	 *     }
	 * };
	 * 
	 * MyHandler handler;
	 * SetRecvCB(&handler, &MyHandler::OnReceive);
	 * @endcode
	 */
	template <typename BaseType>
	void SetRecvCB(BaseType* pBaseType, int (BaseType::*handleRecieve)(uint64_t connectionLink, uint64_t objectLinkId, NFDataPackage& packet))
	{
		NET_CALLBACK_RECEIVE_FUNCTOR func = std::bind(handleRecieve, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		SetRecvCB(func);
	}

	/**
	 * @brief 设置连接事件回调（模板版本）
	 * @tparam BaseType 回调对象类型
	 * @param pBaseType 回调对象指针
	 * @param handleEvent 处理连接事件的成员函数指针
	 * 
	 * 类型安全的事件回调设置接口，用于处理连接建立、断开等事件。
	 * 
	 * 使用示例：
	 * @code
	 * class MyHandler {
	 * public:
	 *     int OnEvent(eMsgType event, uint64_t linkId, uint64_t objId) {
	 *         switch(event) {
	 *             case MSG_TYPE_CONNECTED:
	 *                 // 处理连接建立
	 *                 break;
	 *             case MSG_TYPE_DISCONNECTED:
	 *                 // 处理连接断开
	 *                 break;
	 *         }
	 *         return 0;
	 *     }
	 * };
	 * 
	 * MyHandler handler;
	 * SetEventCB(&handler, &MyHandler::OnEvent);
	 * @endcode
	 */
	template <typename BaseType>
	void SetEventCB(BaseType* pBaseType, int (BaseType::*handleEvent)(eMsgType nEvent, uint64_t connectionLink, uint64_t objectLinkId))
	{
		NET_CALLBACK_EVENT_FUNCTOR func = std::bind(handleEvent, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		SetEventCB(func);
	}

    /**
	 * @brief 设置HTTP接收回调（模板版本）
	 * @tparam BaseType 回调对象类型
	 * @param pBaseType 回调对象指针
	 * @param handleRecieve 处理HTTP请求的成员函数指针
	 * 
	 * 类型安全的HTTP请求处理回调设置接口。
	 * 
	 * 使用示例：
	 * @code
	 * class MyHttpHandler {
	 * public:
	 *     bool OnHttpRequest(uint32_t requestId, const NFIHttpHandle& req) {
	 *         std::string method = req.GetRequestMethod();
	 *         std::string uri = req.GetRequestUri();
	 *         // 处理HTTP请求
	 *         req.ResponseMsg("Hello World", NFWebStatus::WEB_OK);
	 *         return true;
	 *     }
	 * };
	 * 
	 * MyHttpHandler handler;
	 * SetHttpRecvCB(&handler, &MyHttpHandler::OnHttpRequest);
	 * @endcode
    */
    template<typename BaseType>
    void SetHttpRecvCB(BaseType *pBaseType, bool (BaseType::*handleRecieve)(uint32_t, const NFIHttpHandle &req)) {
        HTTP_RECEIVE_FUNCTOR func = std::bind(handleRecieve, pBaseType, std::placeholders::_1, std::placeholders::_2);
        SetHttpRecvCB(func);
    }

    /**
	 * @brief 设置HTTP过滤回调（模板版本）
	 * @tparam BaseType 回调对象类型
	 * @param pBaseType 回调对象指针
	 * @param handleFilter 处理HTTP过滤的成员函数指针
	 * 
	 * 类型安全的HTTP过滤器回调设置接口，用于请求预处理和权限验证。
	 * 
	 * 使用示例：
	 * @code
	 * class MyHttpFilter {
	 * public:
	 *     NFWebStatus OnHttpFilter(uint32_t requestId, const NFIHttpHandle& req) {
	 *         // 验证权限、检查参数等
	 *         std::string token = req.GetQuery("token");
	 *         if (token.empty()) {
	 *             return NFWebStatus::WEB_AUTH; // 401 Unauthorized
	 *         }
	 *         return NFWebStatus::WEB_OK; // 200 OK
	 *     }
	 * };
	 * 
	 * MyHttpFilter filter;
	 * SetHttpFilterCB(&filter, &MyHttpFilter::OnHttpFilter);
	 * @endcode
     */
    template<typename BaseType>
    void SetHttpFilterCB(BaseType *pBaseType, NFWebStatus(BaseType::*handleFilter)(uint32_t, const NFIHttpHandle &req)) {
        HTTP_FILTER_FUNCTOR func = std::bind(handleFilter, pBaseType, std::placeholders::_1, std::placeholders::_2);
        SetHttpFilterCB(func);
    }

	/**
	 * @brief 设置接收消息回调
	 * @param recvcb 接收消息的回调函数对象
	 * 
	 * 设置处理网络消息接收的回调函数，当有消息到达时会调用此回调。
	 */
	virtual void SetRecvCB(const NET_CALLBACK_RECEIVE_FUNCTOR& recvcb) = 0;

	/**
	 * @brief 设置连接事件回调
	 * @param eventcb 连接事件的回调函数对象
	 * 
	 * 设置处理连接事件的回调函数，如连接建立、断开等事件。
	 */
	virtual void SetEventCB(const NET_CALLBACK_EVENT_FUNCTOR& eventcb) = 0;

    /**
	 * @brief 设置HTTP接收回调
	 * @param recvcb HTTP请求的回调函数对象
	 * 
	 * 设置处理HTTP请求的回调函数，当有HTTP请求到达时会调用此回调。
    */
    virtual void SetHttpRecvCB(const HTTP_RECEIVE_FUNCTOR& recvcb) = 0;

    /**
	 * @brief 设置HTTP过滤回调
	 * @param eventcb HTTP过滤的回调函数对象
	 * 
	 * 设置HTTP请求过滤器回调函数，用于请求预处理和权限验证。
     */
    virtual void SetHttpFilterCB(const HTTP_FILTER_FUNCTOR& eventcb) = 0;

	/**
	 * @brief 绑定服务器
	 * @param serverType 服务器类型，标识服务器的功能类型
	 * @param url 绑定的URL地址，格式如 "tcp://0.0.0.0:8080" 或 "http://0.0.0.0:9090"
	 * @param netThreadNum 网络线程数量，默认为1
	 * @param maxConnectNum 最大连接数，默认为100
	 * @param packetParseType 数据包解析类型，默认为内部协议
	 * @param security 是否启用安全连接（SSL/TLS），默认为false
	 * @return 返回绑定的服务器ID，失败返回0
	 * 
	 * 创建并绑定一个网络服务器，监听指定的地址和端口。
	 * 支持TCP、UDP、HTTP等多种协议。
	 * 
	 * 使用示例：
	 * @code
	 * // 绑定TCP服务器
	 * uint64_t serverId = BindServer(NF_ST_GAME, "tcp://0.0.0.0:8080", 4, 1000);
	 * 
	 * // 绑定HTTP服务器
	 * uint64_t httpId = BindServer(NF_ST_WEB, "http://0.0.0.0:9090", 2, 500);
	 * 
	 * // 绑定安全HTTPS服务器
	 * uint64_t httpsId = BindServer(NF_ST_WEB, "https://0.0.0.0:9443", 2, 500, 
	 *                               PACKET_PARSE_TYPE_INTERNAL, true);
	 * @endcode
	 */
	virtual uint64_t BindServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t netThreadNum = 1, uint32_t maxConnectNum = 100, uint32_t packetParseType = PACKET_PARSE_TYPE_INTERNAL, bool security = false) = 0;

	/**
	 * @brief 重置并初始化数据包解析器
	 * @param parseType 解析类型，指定数据包的解析方式
	 * @param pPacketParse 数据包解析器对象指针
	 * @return 操作结果，0表示成功，非0表示失败
	 * 
	 * 重置并初始化指定类型的数据包解析器，用于自定义数据包格式的处理。
	 * 
	 * 使用示例：
	 * @code
	 * auto parser = new CustomPacketParser();
	 * int result = ResetPacketParse(PACKET_PARSE_TYPE_CUSTOM, parser);
	 * if (result == 0) {
	 *     // 解析器设置成功
	 * }
	 * @endcode
	 */
	virtual int ResetPacketParse(uint32_t parseType, NFIPacketParse* pPacketParse) = 0;

	/**
	 * @brief 连接到服务器
	 * @param serverType 服务器类型，标识要连接的服务器功能类型
	 * @param url 服务器地址，格式如 "tcp://127.0.0.1:8080"
	 * @param packetParseType 数据包解析类型，默认为0（自动检测）
	 * @param security 是否启用安全连接，默认为false
	 * @return 返回连接ID，失败返回0
	 * 
	 * 作为客户端连接到指定的服务器，支持自动重连机制。
	 * 
	 * 使用示例：
	 * @code
	 * // 连接到游戏服务器
	 * uint64_t linkId = ConnectServer(NF_ST_GAME, "tcp://127.0.0.1:8080");
	 * if (linkId > 0) {
	 *     // 连接成功，可以发送消息
	 * }
	 * @endcode
	 */
	virtual uint64_t ConnectServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t packetParseType = 0, bool security = false) = 0;

	/**
	 * @brief 恢复连接
	 * @param eServerType 服务器类型
	 * @return 操作结果，0表示成功
	 * 
	 * 恢复指定类型服务器的连接，用于断线重连场景。
	 */
    virtual int ResumeConnect(NF_SERVER_TYPE eServerType) = 0;

	/**
	 * @brief 获取连接的IP地址
	 * @param usLinkId 连接ID
	 * @return 返回IP地址字符串
	 * 
	 * 获取指定连接的远程IP地址，用于日志记录和安全控制。
	 */
	virtual std::string GetLinkIp(uint64_t usLinkId) = 0;
	
	/**
	 * @brief 获取连接的端口号
	 * @param usLinkId 连接ID
	 * @return 返回端口号
	 * 
	 * 获取指定连接的远程端口号。
	 */
    virtual uint32_t GetPort(uint64_t usLinkId) = 0;

	/**
	 * @brief 关闭指定连接
	 * @param usLinkId 要关闭的连接ID
	 * 
	 * 主动关闭指定的网络连接，释放相关资源。
	 */
	virtual void CloseLinkId(uint64_t usLinkId) = 0;

	/**
	 * @brief 发送字符串消息
	 * @param usLinkId 连接ID
	 * @param nModuleId 模块ID，用于消息路由
	 * @param nMsgID 消息ID，标识消息类型
	 * @param strData 要发送的字符串数据
	 * @param param1 自定义参数1，默认为0
	 * @param param2 自定义参数2，默认为0
	 * @param srcId 源ID，标识消息发送者，默认为0
	 * @param dstId 目标ID，标识消息接收者，默认为0
	 * 
	 * 通过指定连接发送字符串格式的消息。
	 */
    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

	/**
	 * @brief 发送字节数组消息
	 * @param usLinkId 连接ID
	 * @param nModuleId 模块ID，用于消息路由
	 * @param nMsgID 消息ID，标识消息类型
	 * @param msg 要发送的字节数据指针
	 * @param nLen 数据长度
	 * @param param1 自定义参数1，默认为0
	 * @param param2 自定义参数2，默认为0
	 * @param srcId 源ID，标识消息发送者，默认为0
	 * @param dstId 目标ID，标识消息接收者，默认为0
	 * 
	 * 通过指定连接发送字节数组格式的消息，适用于二进制数据传输。
	 */
    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg,uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

	/**
	 * @brief 发送Protocol Buffers消息
	 * @param usLinkId 连接ID
	 * @param nModuleId 模块ID，用于消息路由
	 * @param nMsgID 消息ID，标识消息类型
	 * @param xData Protocol Buffers消息对象
	 * @param param1 自定义参数1，默认为0
	 * @param param2 自定义参数2，默认为0
	 * @param srcId 源ID，标识消息发送者，默认为0
	 * @param dstId 目标ID，标识消息接收者，默认为0
	 * 
	 * 通过指定连接发送Protocol Buffers格式的消息，自动进行序列化。
	 * 
	 * 使用示例：
	 * @code
	 * PlayerInfo playerInfo;
	 * playerInfo.set_player_id(12345);
	 * playerInfo.set_name("Alice");
	 * 
	 * Send(linkId, MODULE_GAME, MSG_PLAYER_INFO, playerInfo);
	 * @endcode
	 */
    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

	/**
	 * @brief 发送服务器间字符串消息
	 * @param usLinkId 连接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param strData 字符串数据
	 * @param param1 自定义参数1，默认为0
	 * @param param2 自定义参数2，默认为0
	 * @param srcId 源服务器ID，默认为0
	 * @param dstId 目标服务器ID，默认为0
	 * 
	 * 用于服务器间通信的字符串消息发送，包含额外的服务器标识信息。
	 */
    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

	/**
	 * @brief 发送服务器间字节数组消息
	 * @param usLinkId 连接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param msg 字节数据指针
	 * @param nLen 数据长度
	 * @param param1 自定义参数1，默认为0
	 * @param param2 自定义参数2，默认为0
	 * @param srcId 源服务器ID，默认为0
	 * @param dstId 目标服务器ID，默认为0
	 * 
	 * 用于服务器间通信的字节数组消息发送。
	 */
    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg,uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

	/**
	 * @brief 发送服务器间Protocol Buffers消息
	 * @param usLinkId 连接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param xData Protocol Buffers消息对象
	 * @param param1 自定义参数1，默认为0
	 * @param param2 自定义参数2，默认为0
	 * @param srcId 源服务器ID，默认为0
	 * @param dstId 目标服务器ID，默认为0
	 * 
	 * 用于服务器间通信的Protocol Buffers消息发送。
	 */
    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

	/**
	 * @brief 转发数据包
	 * @param usLinkId 目标连接ID
	 * @param packet 要转发的数据包对象
	 * 
	 * 将接收到的数据包原样转发到指定连接，用于代理和路由场景。
	 */
    virtual void TransPackage(uint64_t usLinkId, NFDataPackage& packet) = 0;

	/**
	 * @brief 响应HTTP请求
	 * @param serverType 服务器类型
	 * @param req HTTP请求处理对象
	 * @param strMsg 响应消息内容
	 * @param code HTTP状态码，默认为200 OK
	 * @param reason 状态原因短语，默认为"OK"
	 * @return 发送成功返回true，失败返回false
	 * 
	 * 向HTTP客户端发送响应消息。
	 * 
	 * 使用示例：
	 * @code
	 * bool OnHttpRequest(uint32_t requestId, const NFIHttpHandle& req) {
	 *     std::string response = "{\"status\":\"success\"}";
	 *     return ResponseHttpMsg(NF_ST_WEB, req, response, NFWebStatus::WEB_OK);
	 * }
	 * @endcode
	 */
    virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, const NFIHttpHandle &req, const std::string &strMsg,
                                 NFWebStatus code = NFWebStatus::WEB_OK, const std::string &reason = "OK") = 0;

	/**
	 * @brief 通过请求ID响应HTTP请求
	 * @param serverType 服务器类型
	 * @param requestId HTTP请求的唯一标识
	 * @param strMsg 响应消息内容
	 * @param code HTTP状态码，默认为200 OK
	 * @param reason 状态原因短语，默认为"OK"
	 * @return 发送成功返回true，失败返回false
	 * 
	 * 通过请求ID向HTTP客户端发送响应消息，适用于异步处理场景。
	 */
    virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, uint64_t requestId, const std::string &strMsg,
                                 NFWebStatus code = NFWebStatus::WEB_OK,
                                 const std::string &reason = "OK") = 0;

	/**
	 * @brief 发送HTTP GET请求
	 * @param serverType 服务器类型
	 * @param strUri 请求的URI地址
	 * @param respone 响应处理回调函数
	 * @param xHeaders HTTP请求头，默认为空
	 * @param timeout 超时时间（秒），默认为3秒
	 * @return 请求发送成功返回0，失败返回非0值
	 * 
	 * 作为HTTP客户端发送GET请求到指定URI。
	 * 
	 * 使用示例：
	 * @code
	 * std::map<std::string, std::string> headers;
	 * headers["Authorization"] = "Bearer token123";
	 * 
	 * auto callback = [](int code, const std::string& response) {
	 *     if (code == 200) {
	 *         // 处理响应数据
	 *         std::cout << "Response: " << response << std::endl;
	 *     }
	 * };
	 * 
	 * HttpGet(NF_ST_WEB, "http://api.example.com/users", callback, headers, 5);
	 * @endcode
	 */
    virtual int HttpGet(NF_SERVER_TYPE serverType, const std::string &strUri,
                        const HTTP_CLIENT_RESPONE &respone,
                        const std::map<std::string, std::string> &xHeaders = std::map<std::string, std::string>(),
                        int timeout = 3) = 0;

	/**
	 * @brief 发送HTTP POST请求
	 * @param serverType 服务器类型
	 * @param strUri 请求的URI地址
	 * @param strPostData POST请求的数据内容
	 * @param respone 响应处理回调函数
	 * @param xHeaders HTTP请求头，默认为空
	 * @param timeout 超时时间（秒），默认为3秒
	 * @return 请求发送成功返回0，失败返回非0值
	 * 
	 * 作为HTTP客户端发送POST请求到指定URI。
	 * 
	 * 使用示例：
	 * @code
	 * std::string jsonData = "{\"name\":\"Alice\",\"age\":25}";
	 * std::map<std::string, std::string> headers;
	 * headers["Content-Type"] = "application/json";
	 * 
	 * auto callback = [](int code, const std::string& response) {
	 *     // 处理POST响应
	 * };
	 * 
	 * HttpPost(NF_ST_WEB, "http://api.example.com/users", jsonData, callback, headers);
	 * @endcode
	 */
    virtual int HttpPost(NF_SERVER_TYPE serverType, const std::string &strUri, const std::string &strPostData, const HTTP_CLIENT_RESPONE &respone,
                         const std::map<std::string, std::string> &xHeaders = std::map<std::string, std::string>(),
                         int timeout = 3) = 0;

	/**
	 * @brief 发送邮件
	 * @param serverType 服务器类型
	 * @param title 邮件标题
	 * @param subject 邮件主题
	 * @param content 邮件内容
	 * @return 发送成功返回0，失败返回非0值
	 * 
	 * 通过配置的邮件服务发送邮件，用于系统通知和告警。
	 * 
	 * 使用示例：
	 * @code
	 * int result = SendEmail(NF_ST_WEB, 
	 *                       "系统告警", 
	 *                       "服务器状态异常", 
	 *                       "检测到服务器CPU使用率过高，请及时处理。");
	 * @endcode
	 */
    virtual int SendEmail(NF_SERVER_TYPE serverType, const std::string& title, const std::string& subject, const string &content) = 0;
};
