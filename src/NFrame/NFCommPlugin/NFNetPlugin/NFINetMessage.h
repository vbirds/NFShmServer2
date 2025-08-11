// -------------------------------------------------------------------------
//    @FileName         :    NFIServer.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFNetPlugin
//    @Desc             :    网络消息接口头文件，提供网络消息处理的基础接口。
//                          该文件定义了网络消息处理的基础接口，包括网络消息接口类定义、
//                          服务器心跳定时器枚举、网络事件回调函数类型、消息处理接口。
//                          主要功能包括提供网络消息处理的统一接口、支持多种网络协议（TCP、UDP、HTTP等）、
//                          支持服务器和客户端模式、提供心跳检测机制。
//                          网络消息接口是NFShmXFrame框架网络通信的核心抽象层，负责：
//                          - 网络消息处理的统一接口定义
//                          - 服务器和客户端连接管理
//                          - 消息发送和接收接口
//                          - HTTP请求处理接口
//                          - 网络事件回调机制
//                          - 心跳检测和连接维护
//                          - 跨协议网络通信支持
//
//
//                    .::::.
//                  .::::::::.
//                 :::::::::::  FUCK YOU
//             ..:::::::::::'
//           '::::::::::::'
//             .::::::::::
//        '::::::::::::::..
//             ..::::::::::::.
//           ``::::::::::::::::
//            ::::``:::::::::'        .:::.
//           ::::'   ':::::'       .::::::::.
//         .::::'      ::::     .:::::::'::::.
//        .:::'       :::::  .:::::::::' ':::::.
//       .::'        :::::.:::::::::'      ':::::.
//      .::'         ::::::::::::::'         ``::::.
//  ...:::           ::::::::::::'              ``::.
// ```` ':.          ':::::::::'                  ::::..
//                    '.:::::'                    ':'````..
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFCore/NFMutex.h"
#include "NFComm/NFPluginModule/NFIDynamicModule.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"
#include "NFComm/NFPluginModule/NFNetDefine.h"

/**
 * @brief 服务器心跳定时器枚举
 * 
 * 定义了服务器心跳检测相关的定时器类型
 */
enum EnumServerHeartTimer
{
    ENUM_SERVER_CLIENT_TIMER_HEART = 1, ///< 定时发送心跳
    ENUM_SERVER_TIMER_CHECK_HEART = 2, ///< 服务器定时检查心跳包
    ENUM_SERVER_CLIENT_TIMER_HEART_TIME_LONGTH = 1000, ///< 定时发送心跳时间长度 1000ms
    ENUM_SERVER_TIMER_CHECK_HEART_TIME_LONGTH = 1000, ///< 定时发送心跳时间长度 3000ms
};

/**
 * @brief 网络消息接口类
 * 
 * 该类是网络消息处理的基础接口，提供了：
 * - 服务器绑定和客户端连接功能
 * - 消息发送和接收功能
 * - 网络事件处理功能
 * - HTTP请求处理功能
 * - 心跳检测功能
 * - 网络连接管理
 * - 跨协议通信支持
 * 
 * 使用方式：
 * @code
 * class MyNetMessage : public NFINetMessage {
 *     // 实现纯虚函数
 *     uint64_t BindServer(const NFMessageFlag& flag) override;
 *     uint64_t ConnectServer(const NFMessageFlag& flag) override;
 *     bool Send(uint64_t linkId, NFDataPackage& packet, const char* msg, uint32_t nLen) override;
 *     // ... 其他接口实现
 * };
 * @endcode
 */
class NFINetMessage : public NFIDynamicModule
{
    friend class NFCNetMessageDriverModule;

public:
    /**
     * @brief 构造函数
     * 
     * 初始化网络消息接口，包括：
     * - 设置服务器类型
     * - 初始化网络对象最大索引
     * - 验证服务器类型有效性
     * 
     * @param p 插件管理器指针
     * @param serverType 服务器类型
     */
    NFINetMessage(NFIPluginManager* p, NF_SERVER_TYPE serverType) : NFIDynamicModule(p), m_serverType(serverType), m_netObjectMaxIndex(0)
    {
        NF_ASSERT(serverType > NF_ST_NONE && serverType < NF_ST_MAX);
    }

    /**
    * @brief 析构函数
    */
    ~NFINetMessage() override
    {
    }

    /**
     * @brief 设置接收回调（模板版本）
     * 
     * 使用模板设置消息接收回调函数
     * 
     * @tparam BaseType 基类类型
     * @param pBaseType 基类指针
     * @param handleRecv 接收处理函数指针
     */
    template <typename BaseType>
    void SetRecvCb(BaseType* pBaseType,
                   void (BaseType::*handleRecv)(uint64_t connectLinkId, uint64_t objectLinkId, uint64_t valueId,
                                                uint32_t nMsgId, const char* msg, uint32_t nLen))
    {
        m_recvCb = std::bind(handleRecv, pBaseType, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
    }

    /**
     * @brief 设置连接事件回调（模板版本）
     * 
     * 使用模板设置连接事件回调函数
     * 
     * @tparam BaseType 基类类型
     * @param pBaseType 基类指针
     * @param handleEvent 事件处理函数指针
     */
    template <typename BaseType>
    void SetEventCb(BaseType* pBaseType,
                    void (BaseType::*handleEvent)(eMsgType nEvent, uint64_t connectLinkId, uint64_t objectLinkId))
    {
        m_eventCb = std::bind(handleEvent, pBaseType, std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3);
    }

    /**
    * @brief 设置HTTP接收回调（模板版本）
    * 
    * 使用模板设置HTTP请求接收回调函数
    * 
    * @tparam BaseType 基类类型
    * @param pBaseType 基类指针
    * @param handleRecv HTTP接收处理函数指针
    */
    template <typename BaseType>
    void SetHttpRecvCb(BaseType* pBaseType, bool (BaseType::*handleRecv)(uint32_t, const NFIHttpHandle& req))
    {
        m_httpReceiveCb = std::bind(handleRecv, pBaseType, std::placeholders::_1, std::placeholders::_2);
    }

    /**
     * @brief 设置HTTP过滤器回调（模板版本）
     * 
     * 使用模板设置HTTP请求过滤器回调函数
     * 
     * @tparam BaseType 基类类型
     * @param pBaseType 基类指针
     * @param handleFilter HTTP过滤器处理函数指针
     */
    template <typename BaseType>
    void SetHttpFilterCb(BaseType* pBaseType, NFWebStatus (BaseType::*handleFilter)(uint32_t, const NFIHttpHandle& req))
    {
        m_httpFilter = std::bind(handleFilter, pBaseType, std::placeholders::_1, std::placeholders::_2);
    }

    /**
     * @brief 设置接收回调（函数对象版本）
     * 
     * 直接设置消息接收回调函数对象
     * 
     * @param recvCb 接收回调函数对象
     */
    void SetRecvCb(const NET_CALLBACK_RECEIVE_FUNCTOR& recvCb)
    {
        m_recvCb = recvCb;
    }

    /**
     * @brief 设置连接事件回调（函数对象版本）
     * 
     * 直接设置连接事件回调函数对象
     * 
     * @param eventCb 事件回调函数对象
     */
    void SetEventCb(const NET_CALLBACK_EVENT_FUNCTOR& eventCb)
    {
        m_eventCb = eventCb;
    }

    /**
     * @brief 设置HTTP接收回调（函数对象版本）
     * 
     * 直接设置HTTP请求接收回调函数对象
     * 
     * @param recvCb HTTP接收回调函数对象
     */
    void SetHttpRecvCb(const HTTP_RECEIVE_FUNCTOR& recvCb)
    {
        m_httpReceiveCb = recvCb;
    }

    /**
     * @brief 设置HTTP过滤器回调（函数对象版本）
     * 
     * 直接设置HTTP请求过滤器回调函数对象
     * 
     * @param eventCb HTTP过滤器回调函数对象
     */
    void SetHttpFilterCb(const HTTP_FILTER_FUNCTOR& eventCb)
    {
        m_httpFilter = eventCb;
    }

    /**
     * @brief 绑定服务器（纯虚函数）
     *
     * 在指定端口上绑定服务器，等待客户端连接
     *
     * @param flag 消息标志，包含关于连接类型或协议的信息
     * @return 成功绑定后的链接ID，如果失败则返回0
     */
    virtual uint64_t BindServer(const NFMessageFlag& flag) = 0;

    /**
     * @brief 连接服务器（纯虚函数）
     *
     * 连接到指定的服务器
     *
     * @param flag 消息标志，包含关于连接类型或协议的信息
     * @return 成功连接后的链接ID，如果失败则返回0
     */
    virtual uint64_t ConnectServer(const NFMessageFlag& flag) = 0;

    /**
     * @brief 发送数据包（纯虚函数）
     *
     * 通过指定链接发送数据包
     *
     * @param linkId 链接ID，标识数据应发送到的链接
     * @param packet 数据包，包含要发送的数据
     * @param msg 附加消息，可以是任意字符串数据
     * @param nLen 附加消息的长度
     * @return 发送成功返回true，否则返回false
     */
    virtual bool Send(uint64_t linkId, NFDataPackage& packet, const char* msg, uint32_t nLen) = 0;

    /**
     * @brief 发送Protobuf数据包（纯虚函数）
     *
     * 通过指定链接发送Protobuf格式的数据包
     *
     * @param linkId 链接ID，标识数据应发送到的链接
     * @param packet 数据包，包含要发送的数据
     * @param xData protobuf消息，包含结构化数据
     * @return 发送成功返回true，否则返回false
     */
    virtual bool Send(uint64_t linkId, NFDataPackage& packet, const google::protobuf::Message& xData) = 0;

    /**
     * @brief 获取链接IP地址（纯虚函数）
     *
     * 获取指定链接的IP地址
     *
     * @param linkId 链接ID，标识链接
     * @return 返回链接的IP地址字符串，如果失败则返回空字符串
     */
    virtual std::string GetLinkIp(uint64_t linkId) = 0;

    /**
     * @brief 获取链接端口号（纯虚函数）
     *
     * 获取指定链接的端口号
     *
     * @param linkId 链接ID，标识链接
     * @return 返回链接的端口号，如果失败则返回0
     */
    virtual uint32_t GetPort(uint64_t linkId) = 0;

    /**
     * @brief 关闭链接（纯虚函数）
     *
     * 关闭与指定链接ID相关的连接或会话
     *
     * @param linkId 需要关闭的链接的唯一标识符
     */
    virtual void CloseLinkId(uint64_t linkId) = 0;

    /**
     * @brief 获取服务器类型
     *
     * @return 服务器类型的整数值
     */
    virtual uint32_t GetServerType() const { return m_serverType; }

    /**
     * @brief 响应HTTP消息（使用NFIHttpHandle对象）
     *
     * 通过HTTP请求句柄响应HTTP消息
     *
     * @param req HTTP请求的处理对象
     * @param strMsg 响应的消息内容
     * @param code HTTP状态码，默认为WEB_OK
     * @param reason 状态码的描述，默认为"OK"
     * @return 响应是否成功
     */
    virtual bool ResponseHttpMsg(const NFIHttpHandle& req, const std::string& strMsg, NFWebStatus code = WEB_OK, const std::string& reason = "OK") { return false; }

    /**
     * @brief 响应HTTP消息（使用请求ID）
     *
     * 通过请求ID响应HTTP消息
     *
     * @param requestId HTTP请求的唯一标识符
     * @param strMsg 响应的消息内容
     * @param code HTTP状态码，默认为WEB_OK
     * @param reason 状态码的描述，默认为"OK"
     * @return 响应是否成功
     */
    virtual bool ResponseHttpMsg(uint64_t requestId, const std::string& strMsg, NFWebStatus code = WEB_OK, const std::string& reason = "OK") { return false; }

    /**
     * @brief 执行HTTP GET请求
     *
     * 发送HTTP GET请求并处理响应
     *
     * @param strUri 请求的URI
     * @param respone 回调函数，用于处理服务器的响应
     * @param xHeaders 在请求中添加的额外HTTP头，默认为空
     * @param timeout 请求的超时时间（秒），默认为3秒
     * @return -1，表示未实现或出错
     *
     * 注意：此函数提供了一个默认实现，返回-1，表示未实现或出错
     */
    virtual int HttpGet(const std::string& strUri, const HTTP_CLIENT_RESPONE& respone, const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(), int timeout = 3) { return -1; }

    /**
     * @brief 执行HTTP POST请求
     *
     * 发送HTTP POST请求并处理响应
     *
     * @param strUri 请求的URI
     * @param strPostData POST请求的数据
     * @param respone 回调函数，用于处理服务器的响应
     * @param xHeaders 在请求中添加的额外HTTP头，默认为空
     * @param timeout 请求的超时时间（秒），默认为3秒
     * @return -1，表示未实现或出错
     *
     * 注意：此函数提供了一个默认实现，返回-1，表示未实现或出错
     */
    virtual int HttpPost(const std::string& strUri, const std::string& strPostData, const HTTP_CLIENT_RESPONE& respone, const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(), int timeout = 3) { return -1; }

    /**
     * @brief 尝试恢复连接
     *
     * 尝试重新建立断开的连接
     *
     * @return 0，表示未实现或默认行为
     *
     * 注意：此函数提供了一个默认实现，返回0，表示未实现或默认行为
     */
    virtual int ResumeConnect() { return 0; }

protected:
    /**
     * @brief 处理接受数据的回调
     */
    NET_CALLBACK_RECEIVE_FUNCTOR m_recvCb;

    /**
     * @brief 网络事件回调
     */
    NET_CALLBACK_EVENT_FUNCTOR m_eventCb;

    /**
    * @brief 服务器类型
    */
    NF_SERVER_TYPE m_serverType;

    /**
    * @brief 当前链接对象最大索引
    */
    uint32_t m_netObjectMaxIndex;

    /**
    * @brief HTTP处理接受数据的回调
    */
    HTTP_RECEIVE_FUNCTOR m_httpReceiveCb;
    
    /**
    * @brief HTTP过滤器回调
    */
    HTTP_FILTER_FUNCTOR m_httpFilter;
};
