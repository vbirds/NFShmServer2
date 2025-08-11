// -------------------------------------------------------------------------
//    @FileName         :    NFCNetModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFCNetModule
//    @Desc             :    网络模块实现类头文件，提供客户端和服务器的网络通信功能。
//                          该文件定义了网络模块的实现类，包括网络模块类定义、
//                          服务器和客户端管理功能、消息发送和接收功能、HTTP请求处理功能。
//                          主要功能包括提供统一的网络通信接口、支持多种网络协议（TCP、UDP、HTTP等）、
//                          支持服务器和客户端模式、提供邮件发送功能。
//                          网络模块是NFShmXFrame框架的核心网络组件，负责：
//                          - TCP/UDP网络通信管理
//                          - HTTP/HTTPS协议支持
//                          - 数据包解析和处理
//                          - 网络连接池管理
//                          - 跨服务器通信
//                          - 邮件发送功能
//                          - 网络事件分发和处理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFINetModule.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFINetMessage.h"
#include <NFComm/NFPluginModule/NFIPacketParse.h>

/**
 * @brief 网络模块实现类，继承自NFINetModule
 * 
 * 该类是网络模块的核心实现，提供以下功能：
 * - 服务器绑定和客户端连接
 * - 数据包的发送和接收
 * - HTTP请求处理
 * - 网络事件回调处理
 * - 邮件发送功能
 * - 网络连接池管理
 * - 跨服务器通信
 * 
 * 支持多种网络协议和传输方式：
 * - Evpp (基于libevent的C++网络库)
 * - Enet (UDP可靠传输协议)
 * - Bus (进程间通信)
 * - HTTP/HTTPS协议
 * 
 * 使用方式：
 * @code
 * // 绑定服务器
 * uint64_t serverId = netModule->BindServer(NF_ST_GAME_SERVER, "tcp://0.0.0.0:8080");
 * 
 * // 连接服务器
 * uint64_t clientId = netModule->ConnectServer(NF_ST_GAME_SERVER, "tcp://127.0.0.1:8080");
 * 
 * // 发送消息
 * netModule->Send(clientId, moduleId, msgId, data, param1, param2);
 * @endcode
 */
class NFCNetModule final : public NFINetModule
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化网络模块，包括：
	 * - 初始化ENet库
	 * - 初始化Socket库
	 * - 初始化SSL支持（如果启用）
	 * - 初始化各种服务器数组
	 * - 初始化数据包解析管理器
	 * 
	 * @param p 插件管理器指针
	 */
	explicit NFCNetModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 * 
	 * 清理网络模块资源，包括：
	 * - 清理ENet库
	 * - 清理SSL资源（如果启用）
	 */
	~NFCNetModule() override;

	/**
	 * @brief 设置接收回调
	 * 
	 * 设置网络消息接收回调函数
	 * 
	 * @param recvCb 接收回调函数
	 */
	void SetRecvCB(const NET_CALLBACK_RECEIVE_FUNCTOR& recvCb) override
	{
		m_recvCb = recvCb;
	}

	/**
	 * @brief 设置连接事件回调
	 * 
	 * 设置网络连接事件回调函数
	 * 
	 * @param eventCb 事件回调函数
	 */
	void SetEventCB(const NET_CALLBACK_EVENT_FUNCTOR& eventCb) override
	{
		m_eventCb = eventCb;
	}

	/**
	 * @brief 设置HTTP接收回调
	 * 
	 * 设置HTTP请求接收回调函数
	 * 
	 * @param recvCb HTTP接收回调函数
	 */
	void SetHttpRecvCB(const HTTP_RECEIVE_FUNCTOR& recvCb) override
	{
		m_httpReceiveCb = recvCb;
	}

	/**
	 * @brief 设置HTTP过滤器回调
	 * 
	 * 设置HTTP请求过滤器回调函数
	 * 
	 * @param eventCb HTTP过滤器回调函数
	 */
	void SetHttpFilterCB(const HTTP_FILTER_FUNCTOR& eventCb) override
	{
		m_httpFilter = eventCb;
	}

	/**
	 * @brief 模块唤醒，初始化网络模块
	 * @return 初始化结果，0表示成功
	 */
	int Awake() override;

	/**
	 * @brief 模块关闭前的准备工作
	 * @return 操作结果，0表示成功
	 */
	int BeforeShut() override;

	/**
	 * @brief 关闭模块，停止所有网络服务
	 * 
	 * 关闭所有类型的服务器：
	 * - Evpp服务器
	 * - Bus服务器  
	 * - Enet服务器
	 * 
	 * @return 操作结果，0表示成功
	 */
	int Shut() override;

	/**
	 * @brief 释放模块资源和数据
	 * 
	 * 清理所有网络资源，包括：
	 * - 释放所有连接对象
	 * - 清理HTTP服务器和客户端
	 * - 停止线程池
	 * - 释放网络对象池
	 * - 清理缓冲区
	 * 
	 * @return 释放结果，0表示成功
	 */
	int Finalize() override;

	/**
	 * @brief 模块就绪状态下的心跳处理
	 * @return 操作结果，0表示成功
	 */
	int ReadyTick() override;

	/**
	 * @brief 模块心跳，处理网络事件和消息
	 * @return 操作结果，0表示成功
	 */
	int Tick() override;

	/**
	 * @brief 绑定服务器函数
	 *
	 * 本函数用于在特定类型的服务器上进行绑定操作它允许指定服务器类型、URL、网络线程数量、最大连接数、数据包解析类型和安全性选项
	 *
	 * @param serverType 服务器类型，表示要绑定的服务器的类型
	 * @param url 服务器的URL地址，用于网络连接
	 * @param netThreadNum 网络线程数量，默认为1，用于处理网络消息的线程数
	 * @param maxConnectNum 最大连接数，默认为100，表示服务器能同时处理的最大连接数
	 * @param packetParseType 数据包解析类型，默认为内部解析类型，指示如何解析接收到的数据包
	 * @param security 是否启用安全性，默认为否，表示是否需要在通信中启用安全措施
	 *
	 * @return 返回一个uint64_t类型的值，通常用于表示绑定操作的结果或生成的唯一标识符
	 */
	uint64_t BindServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t netThreadNum = 1, uint32_t maxConnectNum = 100, uint32_t packetParseType = PACKET_PARSE_TYPE_INTERNAL, bool security = false) override;

	/**
	 * @brief 重置并初始化解析包
	 *
	 * 本函数旨在根据指定的解析类型和解析包对象，进行重置和初始化操作，以确保数据包的解析过程正确进行。
	 *
	 * @param parseType 解析类型，一个无符号32位整数，用于指定解析的类型或模式。
	 * @param pPacketParse 指向NFIPacketParse对象的指针，表示要进行重置和初始化的数据包对象。
	 * @return 返回一个整数值，表示操作的结果，具体含义取决于实现。
	 */
	int ResetPacketParse(uint32_t parseType, NFIPacketParse* pPacketParse) override;

	/**
	 * @brief 连接服务器函数
	 *
	 * @param serverType 服务器类型，表示要连接的服务器的种类
	 * @param url 服务器的URL地址，格式如"ip:port"，用于建立网络连接
	 * @param packetParseType 数据包解析类型，默认为0，用于指定数据包的解析方式
	 * @param security 是否使用安全连接，默认为false，表示不使用安全连接
	 *
	 * @return 返回uint64_t类型的连接ID，用于后续的操作和管理
	 *
	 * 此函数用于建立与特定类型服务器的连接，可以根据需要选择不同的数据包解析方式和是否使用安全连接
	 * 它是异步操作，通过回调函数处理连接结果
	 */
	uint64_t ConnectServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t packetParseType = 0, bool security = false) override;

	/**
	 * @brief 恢复连接到指定类型的服务器
	 * @param serverType 服务器类型
	 * @return 操作结果，0表示成功
	 */
	int ResumeConnect(NF_SERVER_TYPE serverType) override;

	/**
	 * @brief 根据服务器类型获取服务器对象
	 * @param serverType 服务器类型
	 * @return 网络消息对象指针，如果未找到返回nullptr
	 */
	NFINetMessage* GetServerByServerType(NF_SERVER_TYPE serverType) const;

	/**
	 * @brief 获取连接的IP地址
	 * @param linkId 连接ID
	 * @return IP地址字符串
	 */
	std::string GetLinkIp(uint64_t linkId) override;
	
	/**
	 * @brief 获取连接的端口号
	 * @param linkId 连接ID
	 * @return 端口号
	 */
	uint32_t GetPort(uint64_t linkId) override;

	/**
	 * @brief 关闭指定的连接
	 * @param linkId 连接ID
	 */
	void CloseLinkId(uint64_t linkId) override;

	/**
	 * @brief 发送字符串数据
	 * @param linkId 连接ID
	 * @param moduleId 模块ID
	 * @param msgId 消息ID
	 * @param strData 字符串数据
	 * @param param1 参数1
	 * @param param2 参数2
	 * @param srcId 源ID
	 * @param dstId 目标ID
	 */
	void Send(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const std::string& strData, uint64_t param1, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 发送原始字节数据
	 * @param linkId 连接ID
	 * @param moduleId 模块ID
	 * @param msgId 消息ID
	 * @param msg 数据指针
	 * @param len 数据长度
	 * @param param1 参数1
	 * @param param2 参数2
	 * @param srcId 源ID
	 * @param dstId 目标ID
	 */
	void Send(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const char* msg, uint32_t len, uint64_t param1, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 发送Protobuf消息
	 * @param linkId 连接ID
	 * @param moduleId 模块ID
	 * @param msgId 消息ID
	 * @param data Protobuf消息对象
	 * @param param1 参数1
	 * @param param2 参数2
	 * @param srcId 源ID
	 * @param dstId 目标ID
	 */
	void Send(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const google::protobuf::Message& data, uint64_t param1, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 向服务器发送字符串数据
	 * @param linkId 连接ID
	 * @param moduleId 模块ID
	 * @param msgId 消息ID
	 * @param strData 字符串数据
	 * @param param1 参数1
	 * @param param2 参数2
	 * @param srcId 源ID
	 * @param dstId 目标ID
	 */
	void SendServer(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const std::string& strData, uint64_t param1, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 向服务器发送原始字节数据
	 * @param linkId 连接ID
	 * @param moduleId 模块ID
	 * @param msgId 消息ID
	 * @param msg 数据指针
	 * @param len 数据长度
	 * @param param1 参数1
	 * @param param2 参数2
	 * @param srcId 源ID
	 * @param dstId 目标ID
	 */
	void SendServer(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const char* msg, uint32_t len, uint64_t param1, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 向服务器发送Protobuf消息
	 * @param linkId 连接ID
	 * @param moduleId 模块ID
	 * @param msgId 消息ID
	 * @param data Protobuf消息对象
	 * @param param1 参数1
	 * @param param2 参数2
	 * @param srcId 源ID
	 * @param dstId 目标ID
	 */
	void SendServer(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const google::protobuf::Message& data, uint64_t param1, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 透传数据包
	 * @param linkId 连接ID
	 * @param packet 数据包对象
	 */
	void TransPackage(uint64_t linkId, NFDataPackage& packet) override;

	/**
	 * @brief 发送数据包（原始数据）
	 * @param linkId 连接ID
	 * @param packet 数据包
	 * @param msg 消息数据
	 * @param len 数据长度
	 * @return true 发送成功，false 发送失败
	 */
	bool Send(uint64_t linkId, NFDataPackage& packet, const char* msg, uint32_t len);

	/**
	 * @brief 发送数据包（Protobuf数据）
	 * @param linkId 连接ID
	 * @param packet 数据包
	 * @param data Protobuf消息对象
	 * @return true 发送成功，false 发送失败
	 */
	bool Send(uint64_t linkId, NFDataPackage& packet, const google::protobuf::Message& data);

	/**
	 * @brief 响应HTTP消息（使用HTTP句柄）
	 * @param serverType 服务器类型
	 * @param req HTTP请求句柄
	 * @param strMsg 响应消息
	 * @param code HTTP状态码
	 * @param reason 状态原因
	 * @return true 响应成功，false 响应失败
	 */
	bool ResponseHttpMsg(NF_SERVER_TYPE serverType, const NFIHttpHandle& req, const std::string& strMsg, NFWebStatus code = WEB_OK, const std::string& reason = "OK") override;

	/**
	 * @brief 响应HTTP消息（使用请求ID）
	 * @param serverType 服务器类型
	 * @param requestId 请求ID
	 * @param strMsg 响应消息
	 * @param code HTTP状态码
	 * @param reason 状态原因
	 * @return true 响应成功，false 响应失败
	 */
	bool ResponseHttpMsg(NF_SERVER_TYPE serverType, uint64_t requestId, const std::string& strMsg, NFWebStatus code = WEB_OK, const std::string& reason = "OK") override;

	/**
	 * @brief 发送HTTP GET请求
	 * @param serverType 服务器类型
	 * @param strUri 请求URI
	 * @param respone 响应回调
	 * @param xHeaders 请求头
	 * @param timeout 超时时间
	 * @return 请求结果
	 */
	int HttpGet(NF_SERVER_TYPE serverType, const std::string& strUri, const HTTP_CLIENT_RESPONE& respone, const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(), int timeout = 3) override;

	/**
	 * @brief 发送HTTP POST请求
	 * @param serverType 服务器类型
	 * @param strUri 请求URI
	 * @param strPostData POST数据
	 * @param respone 响应回调
	 * @param xHeaders 请求头
	 * @param timeout 超时时间
	 * @return 请求结果
	 */
	int HttpPost(NF_SERVER_TYPE serverType, const std::string& strUri, const std::string& strPostData, const HTTP_CLIENT_RESPONE& respone, const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(), int timeout = 3) override;

	/**
	 * @brief 发送邮件
	 * @param serverType 服务器类型
	 * @param title 邮件标题
	 * @param subject 邮件主题
	 * @param content 邮件内容
	 * @return 发送结果
	 */
	int SendEmail(NF_SERVER_TYPE serverType, const std::string& title, const std::string& subject, const string& content) override;

protected:
	/**
	 * @brief 将消息编码后通过pServer发送出去
	 *
	 * @param pServer 网络消息对象指针
	 * @param linkId 连接ID
	 * @param packet 数据包
	 * @param msg 消息数据
	 * @param len 数据长度
	 * @return true 发送成功，false 发送失败
	 */
	bool Send(NFINetMessage* pServer, uint64_t linkId, NFDataPackage& packet, const char* msg, uint32_t len);
	
	/**
	 * @brief 将Protobuf消息编码后通过pServer发送出去
	 *
	 * @param pServer 网络消息对象指针
	 * @param linkId 连接ID
	 * @param packet 数据包
	 * @param data Protobuf消息对象
	 * @return true 发送成功，false 发送失败
	 */
	bool Send(NFINetMessage* pServer, uint64_t linkId, NFDataPackage& packet, const google::protobuf::Message& data);

private:
	/**
	 * @brief 处理接受数据的回调
	 */
	NET_CALLBACK_RECEIVE_FUNCTOR m_recvCb;

	/**
	 * @brief 网络事件回调
	 */
	NET_CALLBACK_EVENT_FUNCTOR m_eventCb;

	/**
	 * @brief HTTP处理接受数据的回调
	 */
	HTTP_RECEIVE_FUNCTOR m_httpReceiveCb;
	
	/**
	 * @brief HTTP过滤器回调
	 */
	HTTP_FILTER_FUNCTOR m_httpFilter;

	/**
	 * @brief Evpp服务器数组
	 * 
	 * 存储所有Evpp类型的网络消息对象，按服务器类型索引
	 */
	std::vector<NFINetMessage*> m_evppServerArray;
	
	/**
	 * @brief Enet服务器数组
	 * 
	 * 存储所有Enet类型的网络消息对象，按服务器类型索引
	 */
	std::vector<NFINetMessage*> m_enetServerArray;
	
	/**
	 * @brief Bus服务器数组
	 * 
	 * 存储所有Bus类型的网络消息对象，按服务器类型索引
	 */
	std::vector<NFINetMessage*> m_busServerArray;
};
