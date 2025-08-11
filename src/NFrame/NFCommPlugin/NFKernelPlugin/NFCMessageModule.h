// -------------------------------------------------------------------------
//    @FileName         :    NFCMessageModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCMessageModule
//    @Desc             :    网络消息模块头文件，负责网络通信、消息路由和服务器管理。
//                          该文件定义了NFShmXFrame框架的核心网络通信模块，提供完整的网络通信解决方案，
//                          包括网络服务管理、消息通信、服务发现与路由、RPC服务、HTTP支持、回调系统。
//                          主要功能包括服务器绑定和监听、客户端连接管理、点对点消息发送、
//                          服务器间消息路由、广播消息支持、事务性消息传输、RPC服务注册和调用、
//                          HTTP服务器和客户端功能、协程化处理支持
//    @Description      :    网络消息模块头文件，负责网络通信、消息路由和服务器管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFINetModule.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFServerLinkData.h"
#include <stdint.h>
#include <unordered_set>

/**
 * @class NFCMessageModule
 * @brief 网络消息模块实现类
 *
 * NFCMessageModule是NFShmXFrame框架的核心网络通信模块，提供了完整的网络通信解决方案：
 * 
 * 网络服务管理：
 * - 服务器绑定和监听功能
 * - 客户端连接管理
 * - 服务器间连接建立和维护
 * - 网络连接的生命周期管理
 * 
 * 消息通信：
 * - 点对点消息发送
 * - 服务器间消息路由
 * - 广播消息支持
 * - 事务性消息传输(Trans)
 * - 支持protobuf和字符串消息格式
 * 
 * 服务发现与路由：
 * - 服务器注册和发现
 * - 智能负载均衡
 * - 跨服务器通信路由
 * - 数据库服务器选择
 * 
 * RPC服务：
 * - RPC服务注册和调用
 * - 异步RPC支持
 * - 协程化RPC处理
 * 
 * HTTP支持：
 * - HTTP服务器功能
 * - HTTP客户端请求
 * - RESTful API支持
 * - HTTP消息过滤和路由
 * 
 * 回调系统：
 * - 消息回调注册
 * - 网络事件回调
 * - 支持协程化回调处理
 * - 灵活的消息分发机制
 * 
 * 高级特性：
 * - 支持多种数据包解析类型
 * - 安全连接支持
 * - 配置热重载
 * - 邮件和企业微信通知
 * 
 * @note 该模块是分布式服务架构的核心组件
 * @note 支持协程化处理以提高并发性能
 */
class NFCMessageModule : public NFIMessageModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	explicit NFCMessageModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 */
	virtual ~NFCMessageModule();

	/**
	 * @brief 模块觉醒初始化
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Awake() override;

	/**
	 * @brief 最终化处理
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Finalize() override;

	/**
	 * @brief 模块定时更新
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int Tick() override;

	/**
	 * @brief 配置重载处理
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int OnReloadConfig() override;

	/**
	 * @brief 绑定服务器监听端口
	 * @param eServerType 服务器类型
	 * @param url 监听地址，格式为"ip:port"
	 * @param nNetThreadNum 网络线程数，默认为1
	 * @param nMaxConnectNum 最大连接数，默认为100
	 * @param nPacketParseType 数据包解析类型，默认为内部解析
	 * @param bSecurity 是否启用安全连接，默认为false
	 * @return 返回绑定的链接ID，0表示失败
	 * 
	 * 创建网络监听服务，等待其他服务器或客户端的连接。
	 */
	virtual uint64_t BindServer(NF_SERVER_TYPE eServerType, const std::string& url, uint32_t nNetThreadNum = 1, uint32_t nMaxConnectNum = 100,
								uint32_t nPacketParseType = PACKET_PARSE_TYPE_INTERNAL, bool bSecurity = false) override;

	/**
	 * @brief 重置并初始化数据包解析器
	 * @param parseType 解析类型，指定数据包的解析方式
	 * @param pPacketParse 数据包解析器对象指针
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 根据指定的解析类型重新配置数据包解析器，确保数据包能够正确解析。
	 */
	virtual int ResetPacketParse(uint32_t parseType, NFIPacketParse* pPacketParse) override;

	/**
	 * @brief 连接到目标服务器
	 * @param eServerType 服务器类型
	 * @param url 目标服务器地址，格式为"ip:port"
	 * @param nPacketParseType 数据包解析类型，默认为0
	 * @param bSecurity 是否启用安全连接，默认为false
	 * @return 返回连接的链接ID，0表示失败
	 * 
	 * 主动连接到指定的服务器，建立客户端连接。
	 */
	virtual uint64_t ConnectServer(NF_SERVER_TYPE eServerType, const std::string& url, uint32_t nPacketParseType = 0, bool bSecurity = false) override;

	/**
	 * @brief 恢复服务器连接
	 * @param eServerType 服务器类型
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 重新建立与指定类型服务器的连接。
	 */
	virtual int ResumeConnect(NF_SERVER_TYPE eServerType) override;

	/**
	 * @brief 获取连接的IP地址
	 * @param usLinkId 链接ID
	 * @return 返回IP地址字符串
	 */
	virtual std::string GetLinkIp(uint64_t usLinkId) override;

	/**
	 * @brief 获取连接的端口号
	 * @param usLinkId 链接ID
	 * @return 返回端口号
	 */
	virtual uint32_t GetPort(uint64_t usLinkId) override;

	/**
	 * @brief 关闭指定的网络连接
	 * @param usLinkId 链接ID
	 */
	virtual void CloseLinkId(uint64_t usLinkId) override;

	/**
	 * @brief 关闭服务器连接
	 * @param eServerType 当前服务器类型
	 * @param destServer 目标服务器类型
	 * @param busId 业务ID
	 * @param usLinkId 链接ID
	 */
	virtual void CloseServer(NF_SERVER_TYPE eServerType, NF_SERVER_TYPE destServer, uint32_t busId, uint64_t usLinkId) override;

	/**
	 * @brief 透传数据包
	 * @param usLinkId 链接ID
	 * @param packet 数据包对象
	 * 
	 * 将数据包原样转发到指定连接，不做任何处理。
	 */
	virtual void TransPackage(uint64_t usLinkId, NFDataPackage& packet) override;

	/**
	 * @brief 处理接收到的消息
	 * @param packet 数据包对象
	 * 
	 * 对接收到的消息进行处理和分发。
	 */
	virtual void OnHandleMessage(NFDataPackage& packet) override;

	/**
	 * @brief 发送消息（字符串版本）
	 * @param usLinkId 链接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param strData 消息数据
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @param srcId 源ID，默认为0
	 * @param dstId 目标ID，默认为0
	 */
	virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 发送消息（字节数组版本）
	 * @param usLinkId 链接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param msg 消息数据指针
	 * @param nLen 消息长度
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @param srcId 源ID，默认为0
	 * @param dstId 目标ID，默认为0
	 */
	virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg, uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 发送消息（protobuf版本）
	 * @param usLinkId 链接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param xData protobuf消息对象
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @param srcId 源ID，默认为0
	 * @param dstId 目标ID，默认为0
	 */
	virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 发送服务器消息（字符串版本）
	 * @param usLinkId 链接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param strData 消息数据
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @param srcId 源ID，默认为0
	 * @param dstId 目标ID，默认为0
	 * 
	 * 专门用于服务器间通信的消息发送。
	 */
	virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 发送服务器消息（字节数组版本）
	 * @param usLinkId 链接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param msg 消息数据指针
	 * @param nLen 消息长度
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @param srcId 源ID，默认为0
	 * @param dstId 目标ID，默认为0
	 */
	virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg, uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 发送服务器消息（protobuf版本）
	 * @param usLinkId 链接ID
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param xData protobuf消息对象
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @param srcId 源ID，默认为0
	 * @param dstId 目标ID，默认为0
	 */
	virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) override;

	/**
	 * @brief 向指定服务器发送消息（protobuf版本）
	 * @param eSendType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param srcBusId 源服务器业务ID
	 * @param dstBusId 目标服务器业务ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData protobuf消息数据
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendMsgToServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nModuleId, uint32_t nMsgId, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0) override;

	/**
	 * @brief 向指定服务器发送消息（字符串版本）
	 * @param eSendType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param srcBusId 源服务器业务ID
	 * @param dstBusId 目标服务器业务ID
	 * @param nModuleId 模块ID
	 * @param nMsgId 消息ID
	 * @param xData 字符串消息数据
	 * @param param1 参数1，默认为0
	 * @param param2 参数2，默认为0
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendMsgToServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nModuleId, uint32_t nMsgId, const std::string& xData, uint64_t param1 = 0, uint64_t param2 = 0) override;

	/**
	 * @brief 发送事务消息（protobuf版本）
	 * @param eSendType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param srcBusId 源服务器业务ID
	 * @param dstBusId 目标服务器业务ID
	 * @param nMsgID 消息ID
	 * @param xData protobuf消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @param rsp_trans_id 响应事务ID，默认为0
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 事务消息用于需要确认和回复的消息传输。
	 */
	virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nMsgID, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	/**
	 * @brief 发送事务消息（字符串版本）
	 * @param eSendType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param srcBusId 源服务器业务ID
	 * @param dstBusId 目标服务器业务ID
	 * @param nMsgID 消息ID
	 * @param xData 字符串消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @param rsp_trans_id 响应事务ID，默认为0
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t nMsgID, const std::string& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	/**
	 * @brief 发送带模块ID的事务消息（protobuf版本）
	 * @param eSendType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param srcBusId 源服务器业务ID
	 * @param dstBusId 目标服务器业务ID
	 * @param moduleId 模块ID
	 * @param nMsgID 消息ID
	 * @param xData protobuf消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @param rsp_trans_id 响应事务ID，默认为0
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t moduleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	/**
	 * @brief 发送带模块ID的事务消息（字符串版本）
	 * @param eSendType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param srcBusId 源服务器业务ID
	 * @param dstBusId 目标服务器业务ID
	 * @param moduleId 模块ID
	 * @param nMsgID 消息ID
	 * @param xData 字符串消息数据
	 * @param req_trans_id 请求事务ID，默认为0
	 * @param rsp_trans_id 响应事务ID，默认为0
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendTrans(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE recvType, uint32_t srcBusId, uint32_t dstBusId, uint32_t moduleId, uint32_t nMsgID, const std::string& xData, uint32_t req_trans_id = 0, uint32_t rsp_trans_id = 0) override;

	/**
	 * @brief 根据服务器ID获取服务器数据
	 * @param eSendType 服务器类型
	 * @param busId 业务ID
	 * @return 返回服务器数据的智能指针，不存在则返回nullptr
	 */
	virtual NF_SHARE_PTR<NFServerData> GetServerByServerId(NF_SERVER_TYPE eSendType, uint32_t busId) override;

	/**
	 * @brief 根据连接ID获取服务器数据
	 * @param eSendType 服务器类型
	 * @param unlinkId 连接ID
	 * @return 返回服务器数据的智能指针，不存在则返回nullptr
	 */
	virtual NF_SHARE_PTR<NFServerData> GetServerByUnlinkId(NF_SERVER_TYPE eSendType, uint64_t unlinkId) override;

	/**
	 * @brief 创建服务器数据
	 * @param eSendType 发送方服务器类型
	 * @param busId 业务ID
	 * @param busServerType 目标服务器类型
	 * @param data 服务器信息报告
	 * @return 返回创建的服务器数据智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> CreateServerByServerId(NF_SERVER_TYPE eSendType, uint32_t busId, NF_SERVER_TYPE busServerType, const NFrame::ServerInfoReport& data) override;

	/**
	 * @brief 创建到服务器的连接
	 * @param eSendType 发送方服务器类型
	 * @param busId 业务ID
	 * @param linkId 连接ID
	 */
	virtual void CreateLinkToServer(NF_SERVER_TYPE eSendType, uint32_t busId,
									uint64_t linkId) override;

	/**
	 * @brief 删除服务器连接
	 * @param eSendType 服务器类型
	 * @param linkId 连接ID
	 */
	virtual void DelServerLink(NF_SERVER_TYPE eSendType, uint64_t linkId) override;

	/**
	 * @brief 获取路由数据
	 * @param eSendType 服务器类型
	 * @return 返回路由数据指针
	 */
	virtual NFServerData* GetRouteData(NF_SERVER_TYPE eSendType) override;

	/**
	 * @brief 获取路由数据（常量版本）
	 * @param eSendType 服务器类型
	 * @return 返回路由数据常量指针
	 */
	virtual const NFServerData* GetRouteData(NF_SERVER_TYPE eSendType) const override;

	/**
	 * @brief 获取主服务器数据
	 * @param eSendType 服务器类型
	 * @return 返回主服务器数据指针
	 */
	virtual NFServerData* GetMasterData(NF_SERVER_TYPE eSendType) override;

	/**
	 * @brief 获取主服务器数据（常量版本）
	 * @param eSendType 服务器类型
	 * @return 返回主服务器数据常量指针
	 */
	virtual const NFServerData* GetMasterData(NF_SERVER_TYPE eSendType) const override;

	/**
	 * @brief 关闭所有连接
	 * @param eSendType 服务器类型
	 */
	virtual void CloseAllLink(NF_SERVER_TYPE eSendType) override;

	/**
	 * @brief 获取服务器连接ID
	 * @param eSendType 服务器类型
	 * @return 返回服务器连接ID
	 */
	virtual uint64_t GetServerLinkId(NF_SERVER_TYPE eSendType) const override;

	/**
	 * @brief 设置服务器连接ID
	 * @param eSendType 服务器类型
	 * @param linkId 连接ID
	 */
	virtual void SetServerLinkId(NF_SERVER_TYPE eSendType, uint64_t linkId) override;

	/**
	 * @brief 获取客户端连接ID
	 * @param eSendType 服务器类型
	 * @return 返回客户端连接ID
	 */
	virtual uint64_t GetClientLinkId(NF_SERVER_TYPE eSendType) const override;

	/**
	 * @brief 设置客户端连接ID
	 * @param eSendType 服务器类型
	 * @param linkId 连接ID
	 */
	virtual void SetClientLinkId(NF_SERVER_TYPE eSendType, uint64_t linkId) override;

	/**
	 * @brief 根据服务器类型获取所有服务器
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @return 返回该类型所有服务器的列表
	 */
	virtual std::vector<NF_SHARE_PTR<NFServerData>> GetServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) override;

	/**
	 * @brief 根据服务器类型获取第一个服务器
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @return 返回该类型第一个服务器的智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetFirstServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) override;

	/**
	 * @brief 根据服务器类型获取第一个服务器（指定跨服属性）
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @param crossServer 是否跨服
	 * @return 返回该类型第一个服务器的智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetFirstServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, bool crossServer) override;

	/**
	 * @brief 根据服务器类型随机获取服务器
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @return 返回随机选择的服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetRandomServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) override;

	/**
	 * @brief 根据服务器类型随机获取服务器（指定跨服属性）
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @param crossServer 是否跨服
	 * @return 返回随机选择的服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetRandomServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, bool crossServer) override;

	/**
	 * @brief 根据服务器类型和数值获取合适的服务器
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @param value 用于一致性哈希的数值
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, uint64_t value) override;

	/**
	 * @brief 根据服务器类型和数值获取合适的服务器（指定跨服属性）
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @param value 用于一致性哈希的数值
	 * @param crossServer 是否跨服
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, uint64_t value, bool crossServer) override;

	/**
	 * @brief 根据服务器类型和字符串获取合适的服务器
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @param value 用于一致性哈希的字符串
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, const std::string& value) override;

	/**
	 * @brief 根据服务器类型和字符串获取合适的服务器（指定跨服属性）
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @param value 用于一致性哈希的字符串
	 * @param crossServer 是否跨服
	 * @return 返回通过一致性哈希选择的服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetSuitServerByServerType(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, const std::string& value, bool crossServer) override;

	/**
	 * @brief 获取所有服务器
	 * @param eSendType 服务器类型
	 * @return 返回所有服务器的列表
	 */
	virtual std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE eSendType) override;

	/**
	 * @brief 获取指定类型的所有服务器
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @return 返回该类型所有服务器的列表
	 */
	virtual std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes) override;

	/**
	 * @brief 获取指定类型的所有服务器（指定跨服属性）
	 * @param eSendType 发送方服务器类型
	 * @param serverTypes 目标服务器类型
	 * @param isCrossServer 是否跨服，默认为false
	 * @return 返回该类型所有服务器的列表
	 */
	virtual std::vector<NF_SHARE_PTR<NFServerData>> GetAllServer(NF_SERVER_TYPE eSendType, NF_SERVER_TYPE serverTypes, bool isCrossServer = false) override;

	/**
	 * @brief 获取所有数据库名称
	 * @param eSendType 服务器类型
	 * @return 返回所有数据库名称的列表
	 */
	virtual std::vector<std::string> GetDBNames(NF_SERVER_TYPE eSendType) override;

	/**
	 * @brief 获取模块的所有消息ID
	 * @param eSendType 服务器类型
	 * @param moduleId 模块ID
	 * @return 返回该模块所有消息ID的集合
	 */
	virtual std::set<uint32_t> GetAllMsg(NF_SERVER_TYPE eSendType, uint32_t moduleId) override;

	/**
	 * @brief 获取第一个数据库服务器
	 * @param eSendType 服务器类型
	 * @param dbName 数据库名称
	 * @return 返回第一个数据库服务器的智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetFirstDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName) override;
	
	/**
	 * @brief 随机获取数据库服务器
	 * @param eSendType 服务器类型
	 * @param dbName 数据库名称
	 * @return 返回随机选择的数据库服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GeRandomDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName) override;
	
	/**
	 * @brief 根据数值获取合适的数据库服务器
	 * @param eSendType 服务器类型
	 * @param dbName 数据库名称
	 * @param value 用于一致性哈希的数值
	 * @return 返回通过一致性哈希选择的数据库服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetSuitDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName, uint64_t value) override;
	
	/**
	 * @brief 根据字符串获取合适的数据库服务器
	 * @param eSendType 服务器类型
	 * @param dbName 数据库名称
	 * @param value 用于一致性哈希的字符串
	 * @return 返回通过一致性哈希选择的数据库服务器智能指针
	 */
	virtual NF_SHARE_PTR<NFServerData> GetSuitDbServer(NF_SERVER_TYPE eSendType, const std::string& dbName, const std::string& value) override;

	/**
	 * @brief 向指定服务器广播事件
	 * @param eType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param dstBusId 目标业务ID
	 * @param nEventID 事件ID
	 * @param bySrcType 源类型
	 * @param nSrcID 源ID
	 * @param message 事件消息
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int BroadcastEventToServer(NF_SERVER_TYPE eType, NF_SERVER_TYPE recvType, uint32_t dstBusId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message& message) override;
	
	/**
	 * @brief 向指定类型的所有服务器广播事件
	 * @param eType 发送方服务器类型
	 * @param recvType 接收方服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 源类型
	 * @param nSrcID 源ID
	 * @param message 事件消息
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int BroadcastEventToServer(NF_SERVER_TYPE eType, NF_SERVER_TYPE recvType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message& message) override;
	
	/**
	 * @brief 向所有服务器广播事件
	 * @param eType 发送方服务器类型
	 * @param nEventID 事件ID
	 * @param bySrcType 源类型
	 * @param nSrcID 源ID
	 * @param message 事件消息
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int BroadcastEventToAllServer(NF_SERVER_TYPE eType, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message& message) override;
	
	/**
	 * @brief 向所有服务器广播事件（带业务ID）
	 * @param eType 发送方服务器类型
	 * @param busId 业务ID
	 * @param nEventID 事件ID
	 * @param bySrcType 源类型
	 * @param nSrcID 源ID
	 * @param message 事件消息
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int BroadcastEventToAllServer(NF_SERVER_TYPE eType, uint32_t busId, uint32_t nEventID, uint32_t bySrcType, uint64_t nSrcID, const google::protobuf::Message &message) override;
public:
	/**
	 * @brief 设置网络模块
	 * @param driver 网络模块接口实现
	 * 
	 * 设置底层网络通信模块，运行时只支持一种网络驱动。
	 * 支持rawudp、tbuspp或第三方网络库。
	 */
	void SetNetModule(NFINetModule* driver);
public:
	/**
	 * @brief 删除目标模块的所有注册回调
	 * @param pTarget 目标动态模块
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool DelAllCallBack(NFIDynamicModule* pTarget) override;

	/**
	 * @brief 删除一个连接的所有回调
	 * @param eType 服务器类型
	 * @param unLinkId 连接ID
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool DelAllCallBack(NF_SERVER_TYPE eType, uint64_t unLinkId) override;

	/**
	 * @brief 添加模块0的消息回调
	 * @param eType 服务器类型
	 * @param nMsgID 消息ID
	 * @param pTarget 目标模块
	 * @param cb 回调函数
	 * @param createCo 是否创建协程
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 一个消息只能有一个处理函数。
	 */
	virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nMsgID, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) override;

	/**
	 * @brief 添加指定模块的消息回调
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param pTarget 目标模块
	 * @param cb 回调函数
	 * @param createCo 是否创建协程
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 一个消息只能有一个处理函数。
	 */
	virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) override;

	/**
	 * @brief 添加模块0的消息回调（无目标模块版本）
	 * @param eType 服务器类型
	 * @param nMsgID 消息ID
	 * @param cb 回调函数
	 * @param createCo 是否创建协程
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nMsgID, const NET_RECEIVE_FUNCTOR& cb, bool createCo) override;

	/**
	 * @brief 添加指定模块的消息回调（无目标模块版本）
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param cb 回调函数
	 * @param createCo 是否创建协程
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool AddMessageCallBack(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, const NET_RECEIVE_FUNCTOR& cb, bool createCo) override;

	/**
	 * @brief 为未注册的消息添加统一处理回调
	 * @param eType 服务器类型
	 * @param linkId 连接ID
	 * @param pTarget 目标模块
	 * @param cb 回调函数
	 * @param createCo 是否创建协程
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 为没有特定处理函数的消息提供统一的处理回调。
	 */
	virtual bool AddOtherCallBack(NF_SERVER_TYPE eType, uint64_t linkId, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) override;

	/**
	 * @brief 添加网络事件回调
	 * @param eType 服务器类型
	 * @param linkId 连接ID
	 * @param pTarget 目标模块
	 * @param cb 事件回调函数
	 * @param createCo 是否创建协程
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 添加连接建立、断开等网络事件的处理函数。
	 */
	virtual bool AddEventCallBack(NF_SERVER_TYPE eType, uint64_t linkId, NFIDynamicModule* pTarget, const NET_EVENT_FUNCTOR& cb, bool createCo) override;

	/**
	 * @brief 添加全局消息回调
	 * @param eType 服务器类型
	 * @param pTarget 目标模块
	 * @param cb 回调函数
	 * @param createCo 是否创建协程
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 对所有消息添加统一回调，通过返回值决定是否处理该消息。
	 * 返回true表示将处理这个消息，false表示不处理。
	 */
	virtual bool AddAllMsgCallBack(NF_SERVER_TYPE eType, NFIDynamicModule* pTarget, const NET_RECEIVE_FUNCTOR& cb, bool createCo) override;

	/**
	 * @brief 添加RPC服务（带目标模块）
	 * @param serverType 服务器类型
	 * @param nMsgID 消息ID
	 * @param pBase 基础模块
	 * @param pRpcService RPC服务接口
	 * @param createCo 是否创建协程，默认为false
	 * @return 返回true表示成功，false表示失败
	 */
	bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nMsgID, NFIDynamicModule* pBase, NFIRpcService* pRpcService, bool createCo = false) override;

	/**
	 * @brief 添加RPC服务（带模块ID和目标模块）
	 * @param serverType 服务器类型
	 * @param nModuleID 模块ID
	 * @param nMsgID 消息ID
	 * @param pBase 基础模块
	 * @param pRpcService RPC服务接口
	 * @param createCo 是否创建协程，默认为false
	 * @return 返回true表示成功，false表示失败
	 */
	bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nModuleID, uint32_t nMsgID, NFIDynamicModule* pBase, NFIRpcService* pRpcService, bool createCo = false) override;

	/**
	 * @brief 添加RPC服务（无目标模块）
	 * @param serverType 服务器类型
	 * @param nMsgID 消息ID
	 * @param pRpcService RPC服务接口
	 * @param createCo 是否创建协程，默认为false
	 * @return 返回true表示成功，false表示失败
	 */
	bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nMsgID, NFIRpcService* pRpcService, bool createCo = false) override;

	/**
	 * @brief 添加RPC服务（带模块ID，无目标模块）
	 * @param serverType 服务器类型
	 * @param nModuleID 模块ID
	 * @param nMsgID 消息ID
	 * @param pRpcService RPC服务接口
	 * @param createCo 是否创建协程，默认为false
	 * @return 返回true表示成功，false表示失败
	 */
	bool AddRpcService(NF_SERVER_TYPE serverType, uint32_t nModuleID, uint32_t nMsgID, NFIRpcService* pRpcService, bool createCo = false) override;

	/**
	 * @brief 接收网络数据包回调
	 * @param connectionLink 连接链接ID
	 * @param objectLinkId 对象链接ID
	 * @param packet 数据包
	 * @return 返回0表示成功，非0表示失败
	 */
	int OnReceiveNetPack(uint64_t connectionLink, uint64_t objectLinkId, NFDataPackage& packet);

	/**
	 * @brief 处理接收到的网络数据包
	 * @param connectionLink 连接链接ID
	 * @param objectLinkId 对象链接ID
	 * @param packet 数据包
	 * @return 返回0表示成功，非0表示失败
	 */
	int OnHandleReceiveNetPack(uint64_t connectionLink, uint64_t objectLinkId, NFDataPackage& packet);

	/**
	 * @brief 网络事件回调
	 * @param nEvent 事件类型
	 * @param serverLinkId 服务器链接ID
	 * @param objectLinkId 对象链接ID
	 * @return 返回0表示成功，非0表示失败
	 */
	int OnSocketNetEvent(eMsgType nEvent, uint64_t serverLinkId, uint64_t objectLinkId);

	/**
	 * @brief 处理RPC服务回调
	 * @param connectionLink 连接链接ID
	 * @param objectLinkId 对象链接ID
	 * @param svrPkg 服务器数据包
	 * @param param1 参数1
	 * @param param2 参数2
	 * @return 返回0表示成功，非0表示失败
	 */
	int OnHandleRpcService(uint64_t connectionLink, uint64_t objectLinkId, const NFrame::Proto_FramePkg& svrPkg, uint64_t param1, uint64_t param2);
public:
	/**
	 * @brief 响应HTTP消息
	 * @param serverType 服务器类型
	 * @param req HTTP请求句柄
	 * @param strMsg 响应消息内容
	 * @param code HTTP状态码，默认为WEB_OK
	 * @param reason 状态原因，默认为"OK"
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, const NFIHttpHandle& req, const std::string& strMsg,
								 NFWebStatus code = NFWebStatus::WEB_OK, const std::string& reason = "OK");

	/**
	 * @brief 响应HTTP消息（使用请求ID）
	 * @param serverType 服务器类型
	 * @param requestId 请求ID
	 * @param strMsg 响应消息内容
	 * @param code HTTP状态码，默认为WEB_OK
	 * @param reason 状态原因，默认为"OK"
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, uint64_t requestId, const std::string& strMsg,
								 NFWebStatus code = NFWebStatus::WEB_OK,
								 const std::string& reason = "OK");

	/**
	 * @brief 发送HTTP GET请求
	 * @param serverType 服务器类型
	 * @param strUri 请求URI
	 * @param respone 响应回调函数
	 * @param xHeaders HTTP头部映射，默认为空
	 * @param timeout 超时时间（秒），默认为3秒
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int HttpGet(NF_SERVER_TYPE serverType, const std::string& strUri,
						const HTTP_CLIENT_RESPONE& respone,
						const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(),
						int timeout = 3);

	/**
	 * @brief 发送HTTP POST请求
	 * @param serverType 服务器类型
	 * @param strUri 请求URI
	 * @param strPostData POST数据
	 * @param respone 响应回调函数
	 * @param xHeaders HTTP头部映射，默认为空
	 * @param timeout 超时时间（秒），默认为3秒
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int HttpPost(NF_SERVER_TYPE serverType, const std::string& strUri, const std::string& strPostData, const HTTP_CLIENT_RESPONE& respone,
						 const std::map<std::string, std::string>& xHeaders = std::map<std::string, std::string>(),
						 int timeout = 3);

	/**
	 * @brief 发送邮件通知
	 * @param serverType 服务器类型
	 * @param title 邮件标题
	 * @param subject 邮件主题
	 * @param content 邮件内容
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendEmail(NF_SERVER_TYPE serverType, const std::string& title, const std::string& subject, const string& content) override;

	/**
	 * @brief 发送企业微信通知
	 * @param serverType 服务器类型
	 * @param content 通知内容
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SendWxWork(NF_SERVER_TYPE serverType, const string& content) override;

protected:
	/**
	 * @brief 添加HTTP消息回调
	 * @param serverType 服务器类型
	 * @param strCommand HTTP命令路径
	 * @param eRequestType HTTP请求类型（GET、POST等）
	 * @param cb HTTP消息处理回调函数
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool AddHttpMsgCB(NF_SERVER_TYPE serverType, const std::string& strCommand, const NFHttpType eRequestType,
							  const HTTP_RECEIVE_FUNCTOR& cb);

	/**
	 * @brief 添加HTTP其他消息回调
	 * @param serverType 服务器类型
	 * @param eRequestType HTTP请求类型
	 * @param cb HTTP消息处理回调函数
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 用于处理未注册路径的HTTP请求。
	 */
	virtual bool AddHttpOtherMsgCB(NF_SERVER_TYPE serverType, const NFHttpType eRequestType, const HTTP_RECEIVE_FUNCTOR& cb);

	/**
	 * @brief 添加HTTP过滤器回调
	 * @param serverType 服务器类型
	 * @param strCommand HTTP命令路径
	 * @param cb HTTP过滤器回调函数
	 * @return 返回true表示成功，false表示失败
	 * 
	 * 在HTTP请求处理前进行过滤和预处理。
	 */
	virtual bool AddHttpFilterCB(NF_SERVER_TYPE serverType, const std::string& strCommand, const HTTP_FILTER_FUNCTOR& cb);

	/**
	 * @brief 处理接收到的HTTP网络包
	 * @param serverType 服务器类型
	 * @param req HTTP请求对象
	 * @return 返回true表示成功，false表示失败
	 */
	virtual bool OnHttpReceiveNetPack(uint32_t serverType, const NFIHttpHandle& req);

	/**
	 * @brief HTTP数据包过滤处理
	 * @param serverType 服务器类型
	 * @param req HTTP请求对象
	 * @return 返回HTTP状态码
	 * 
	 * 对HTTP请求进行过滤，决定是否继续处理。
	 */
	virtual NFWebStatus OnHttpFilterPack(uint32_t serverType, const NFIHttpHandle& req);

protected:
	/**
	 * @brief 网络模块接口指针
	 * 
	 * 指向底层网络通信模块的接口，负责具体的网络I/O操作。
	 * 支持多种网络库的抽象接口。
	 */
	NFINetModule* m_netModule;

	/**
	 * @brief 回调函数映射表
	 * 
	 * 存储所有注册的回调函数，包括消息回调、事件回调等。
	 * 用于消息分发和事件处理。
	 */
	std::vector<CallBack> mxCallBack;

	/**
	 * @brief 服务器连接数据列表
	 * 
	 * 存储所有服务器的连接信息和状态数据。
	 * 包括连接状态、服务器信息、路由数据等。
	 */
	std::vector<ServerLinkData> mServerLinkData;
};
