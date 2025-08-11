// -------------------------------------------------------------------------
//    @FileName         :    NFCNetModule.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFCNetModule
//    @Desc             :    网络模块实现文件，提供网络通信的核心功能实现
//
// -------------------------------------------------------------------------

#include "NFCNetModule.h"

#include <enet/enet.h>
#include <NFComm/NFPluginModule/NFITaskModule.h>

#include "NFEmailSender.h"
#include "NFPacketParseMgr.h"
#include "Bus/NFCBusMessage.h"
#include "Enet/NFEnetMessage.h"
#include "Evpp/NFEvppNetMessage.h"
#include "evpp/httpc/ssl.h"
#include "NFComm/NFCore/NFServerIDUtil.h"
#include "NFComm/NFCore/NFSocketLibFunction.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"
#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"

/**
 * @file NFCNetModule.cpp
 * @brief 网络模块实现文件
 * 
 * 该文件实现了网络模块的核心功能，包括：
 * - 网络模块的初始化和销毁
 * - 服务器绑定和客户端连接
 * - 消息发送和接收处理
 * - HTTP请求处理
 * - 邮件发送功能
 * 
 * 主要功能：
 * - 提供统一的网络通信接口
 * - 支持多种网络协议（TCP、UDP、HTTP等）
 * - 支持服务器和客户端模式
 * - 提供邮件发送功能
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief 网络模块构造函数
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
NFCNetModule::NFCNetModule(NFIPluginManager* p): NFINetModule(p)
{
	// 初始化ENet库
	enet_initialize();
	// 初始化Socket库
	NFSocketLibFunction::InitSocket();
	
#if defined(EVPP_HTTP_CLIENT_SUPPORTS_SSL)
	// 初始化SSL支持
	if (!evpp::httpc::GetSSLCtx())
    {
        evpp::httpc::InitSSL();
    }
#endif
	
	// 初始化Evpp服务器数组
	m_evppServerArray.resize(NF_ST_MAX);
	for (int i = 0; i < NF_ST_MAX; ++i)
	{
		m_evppServerArray[i] = nullptr;
	}
	
	// 初始化Bus服务器数组
	m_busServerArray.resize(NF_ST_MAX);
	for (int i = 0; i < NF_ST_MAX; ++i)
	{
		m_busServerArray[i] = nullptr;
	}
	
	// 初始化Enet服务器数组
	m_enetServerArray.resize(NF_ST_MAX);
	for (int i = 0; i < NF_ST_MAX; ++i)
	{
		m_enetServerArray[i] = nullptr;
	}

	// 初始化数据包解析管理器
	NFPacketParseMgr::m_pPacketParse.resize(100);
}

/**
 * @brief 网络模块析构函数
 * 
 * 清理网络模块资源，包括：
 * - 清理ENet库
 * - 清理SSL资源（如果启用）
 */
NFCNetModule::~NFCNetModule()
{
	// 清理ENet库
	enet_deinitialize();
	
#if defined(EVPP_HTTP_CLIENT_SUPPORTS_SSL)
	// 清理SSL资源
    if (evpp::httpc::GetSSLCtx())
    {
        evpp::httpc::CleanSSL();
    }
#endif
}

/**
 * @brief 模块唤醒，初始化网络模块
 * @return 初始化结果，0表示成功
 */
int NFCNetModule::Awake()
{
	return 0;
}

/**
 * @brief 模块关闭前的准备工作
 * @return 操作结果，0表示成功
 */
int NFCNetModule::BeforeShut()
{
	return 0;
}

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
int NFCNetModule::Shut()
{
	// 关闭所有Evpp服务器
	for (size_t i = 0; i < m_evppServerArray.size(); i++)
	{
		if (m_evppServerArray[i] != nullptr)
		{
			m_evppServerArray[i]->Shut();
		}
	}
	
	// 关闭所有Bus服务器
	for (size_t i = 0; i < m_busServerArray.size(); i++)
	{
		if (m_busServerArray[i] != nullptr)
		{
			m_busServerArray[i]->Shut();
		}
	}
	
	// 关闭所有Enet服务器
	for (size_t i = 0; i < m_enetServerArray.size(); i++)
	{
		if (m_enetServerArray[i] != nullptr)
		{
			m_enetServerArray[i]->Shut();
		}
	}
	return 0;
}

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
int NFCNetModule::Finalize()
{
	for (size_t i = 0; i < m_evppServerArray.size(); i++)
	{
		if (m_evppServerArray[i] != nullptr)
		{
			m_evppServerArray[i]->Finalize();
			NF_SAFE_DELETE(m_evppServerArray[i]);
		}
	}
	for (size_t i = 0; i < m_busServerArray.size(); i++)
	{
		if (m_busServerArray[i] != nullptr)
		{
			m_busServerArray[i]->Finalize();
			NF_SAFE_DELETE(m_busServerArray[i]);
		}
	}
	for (size_t i = 0; i < m_enetServerArray.size(); i++)
	{
		if (m_enetServerArray[i] != nullptr)
		{
			m_enetServerArray[i]->Finalize();
			NF_SAFE_DELETE(m_enetServerArray[i]);
		}
	}
	m_evppServerArray.clear();
	m_busServerArray.clear();
	m_enetServerArray.clear();
	/**
	 * @brief 释放资源
	 */
	NFPacketParseMgr::ReleasePacketParse();
	return 0;
}

/**
 * @brief 模块就绪状态下的心跳处理
 * 
 * 处理所有服务器的就绪状态心跳
 * 
 * @return 操作结果，0表示成功
 */
int NFCNetModule::ReadyTick()
{
	for (size_t i = 0; i < m_evppServerArray.size(); i++)
	{
		if (m_evppServerArray[i] != nullptr)
		{
			m_evppServerArray[i]->ReadyTick();
		}
	}
	for (size_t i = 0; i < m_busServerArray.size(); i++)
	{
		if (m_busServerArray[i] != nullptr)
		{
			m_busServerArray[i]->ReadyTick();
		}
	}
	for (size_t i = 0; i < m_enetServerArray.size(); i++)
	{
		if (m_enetServerArray[i] != nullptr)
		{
			m_enetServerArray[i]->ReadyTick();
		}
	}
	return 0;
}

/**
 * @brief 模块心跳，处理网络事件和消息
 * 
 * 处理所有服务器的定时心跳
 * 
 * @return 操作结果，0表示成功
 */
int NFCNetModule::Tick()
{
	for (size_t i = 0; i < m_evppServerArray.size(); i++)
	{
		if (m_evppServerArray[i] != nullptr)
		{
			m_evppServerArray[i]->Tick();
		}
	}
	for (size_t i = 0; i < m_busServerArray.size(); i++)
	{
		if (m_busServerArray[i] != nullptr)
		{
			m_busServerArray[i]->Tick();
		}
	}
	for (size_t i = 0; i < m_enetServerArray.size(); i++)
	{
		if (m_enetServerArray[i] != nullptr)
		{
			m_enetServerArray[i]->Tick();
		}
	}
	return 0;
}

/**
 * @brief 根据服务器类型获取服务器对象
 * 
 * 根据服务器类型返回对应的Evpp网络消息对象
 * 
 * @param serverType 服务器类型
 * @return 网络消息对象指针，如果未找到返回nullptr
 */
NFINetMessage* NFCNetModule::GetServerByServerType(NF_SERVER_TYPE serverType) const
{
	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		return m_evppServerArray[serverType];
	}
	return nullptr;
}

/**
 * @brief 连接服务器函数
 * 
 * 根据URL协议类型选择相应的网络协议进行连接：
 * - tcp/http: 使用Evpp协议
 * - udp: 使用Enet协议
 * - bus: 使用Bus协议
 * 
 * @param serverType 服务器类型
 * @param url 服务器URL地址
 * @param packetParseType 数据包解析类型
 * @param security 是否使用安全连接
 * @return 连接ID，失败返回0
 */
uint64_t NFCNetModule::ConnectServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t packetParseType, bool security)
{
	NFChannelAddress addr;
	if (!NFServerIDUtil::MakeAddress(url, addr))
	{
		NFLogError(NF_LOG_DEFAULT, 0, "usl:{} error", url);
		return 0;
	}

	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		if (addr.mScheme == "tcp" || addr.mScheme == "http")
		{
			NFMessageFlag flag;
			flag.mStrIp = addr.mHost;
			flag.nPort = addr.mPort;
			flag.mPacketParseType = packetParseType;
			flag.mSecurity = security;

			NFINetMessage* pServer = m_evppServerArray[serverType];
			if (!pServer)
			{
#ifdef USE_NET_EVPP
				pServer = NF_NEW NFEvppNetMessage(m_pObjPluginManager, serverType);
#else

#endif
				pServer->SetRecvCb(m_recvCb);
				pServer->SetEventCb(m_eventCb);
				m_evppServerArray[serverType] = pServer;
			}


			uint64_t linkId = pServer->ConnectServer(flag);
			return linkId;
		}
		else if (addr.mScheme == "udp")
		{
			NFMessageFlag flag;
			flag.mStrIp = addr.mHost;
			flag.nPort = addr.mPort;
			flag.mPacketParseType = packetParseType;
			flag.mSecurity = security;

			NFINetMessage* pServer = m_enetServerArray[serverType];
			if (!pServer)
			{
#ifdef USE_NET_EVPP
				pServer = NF_NEW NFEnetMessage(m_pObjPluginManager, serverType);
#else

#endif
				pServer->SetRecvCb(m_recvCb);
				pServer->SetEventCb(m_eventCb);
				m_enetServerArray[serverType] = pServer;
			}

			uint64_t linkId = pServer->ConnectServer(flag);
			return linkId;
		}
		else if (addr.mScheme == "bus")
		{
			uint32_t busid = NFServerIDUtil::GetBusID(addr.mHost);
			if (busid <= 0)
			{
				NFLogError(NF_LOG_DEFAULT, 0, "BusAddrAton Failed! host:{}", addr.mHost);
				return 0;
			}
			NFMessageFlag flag;
			flag.mStrIp = addr.mHost;
			flag.nPort = addr.mPort;
			flag.mBusId = busid;
			flag.mBusLength = addr.mPort;
			flag.mPacketParseType = packetParseType;
			flag.mSecurity = security;

			NFINetMessage* pServer = m_busServerArray[serverType];
			if (!pServer)
			{
				pServer = NF_NEW NFCBusMessage(m_pObjPluginManager, serverType);
				pServer->SetRecvCb(m_recvCb);
				pServer->SetEventCb(m_eventCb);
				m_busServerArray[serverType] = pServer;
			}

			uint64_t linkId = pServer->ConnectServer(flag);
			return linkId;
		}
	}
	return 0;
}

/**
 * @brief 恢复连接到指定类型的服务器
 * 
 * 目前只支持Bus协议的连接恢复
 * 
 * @param serverType 服务器类型
 * @return 操作结果，0表示成功，-1表示失败
 */
int NFCNetModule::ResumeConnect(NF_SERVER_TYPE serverType)
{
	NFINetMessage* pServer = m_busServerArray[serverType];
	if (pServer)
	{
		return pServer->ResumeConnect();
	}
	return -1;
}

/**
 * @brief 重置数据包解析器
 * 
 * 重置指定类型的数据包解析器
 * 
 * @param parseType 解析类型
 * @param pPacketParse 新的解析器指针
 * @return 操作结果，0表示成功
 */
int NFCNetModule::ResetPacketParse(uint32_t parseType, NFIPacketParse* pPacketParse)
{
	return NFPacketParseMgr::ResetPacketParse(parseType, pPacketParse);
}

/**
 * @brief 绑定服务器函数
 * 
 * 根据URL协议类型选择相应的网络协议进行服务器绑定：
 * - tcp/http: 使用Evpp协议
 * - udp: 使用Enet协议
 * - bus: 使用Bus协议
 * 
 * @param serverType 服务器类型
 * @param url 服务器URL地址
 * @param netThreadNum 网络线程数量
 * @param maxConnectNum 最大连接数
 * @param packetParseType 数据包解析类型
 * @param security 是否使用安全连接
 * @return 绑定结果，成功返回连接ID，失败返回0
 */
uint64_t NFCNetModule::BindServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t netThreadNum, uint32_t maxConnectNum, uint32_t packetParseType, bool security)
{
	NFChannelAddress addr;
	if (!NFServerIDUtil::MakeAddress(url, addr))
	{
		NFLogError(NF_LOG_DEFAULT, 0, "usl:{} error", url);
		return 0;
	}

	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		if (addr.mScheme == "tcp" || addr.mScheme == "http")
		{
			NFMessageFlag flag;
			flag.mStrIp = addr.mHost;
			flag.nPort = addr.mPort;
			flag.mPacketParseType = packetParseType;
			flag.nNetThreadNum = netThreadNum;
			flag.mMaxConnectNum = maxConnectNum;
			flag.mSecurity = security;
			if (addr.mScheme == "http")
			{
				flag.bHttp = true;
			}

			NFINetMessage* pServer = m_evppServerArray[serverType];
			if (!pServer)
			{
				pServer = NF_NEW NFEvppNetMessage(m_pObjPluginManager, serverType);
				pServer->SetRecvCb(m_recvCb);
				pServer->SetEventCb(m_eventCb);
				pServer->SetHttpRecvCb(m_httpReceiveCb);
				pServer->SetHttpFilterCb(m_httpFilter);
				m_evppServerArray[serverType] = pServer;
			}

			uint64_t linkId = pServer->BindServer(flag);
			if (linkId > 0)
			{
				return linkId;
			}

			NFLogError(NF_LOG_DEFAULT, 0, "Add Server Failed!");
		}
		else if (addr.mScheme == "udp")
		{
			NFMessageFlag flag;
			flag.mStrIp = addr.mHost;
			flag.nPort = addr.mPort;
			flag.mPacketParseType = packetParseType;
			flag.nNetThreadNum = netThreadNum;
			flag.mMaxConnectNum = maxConnectNum;
			flag.mSecurity = security;

			NFINetMessage* pServer = m_enetServerArray[serverType];
			if (!pServer)
			{
				pServer = NF_NEW NFEnetMessage(m_pObjPluginManager, serverType);
				pServer->SetRecvCb(m_recvCb);
				pServer->SetEventCb(m_eventCb);
				pServer->SetHttpRecvCb(m_httpReceiveCb);
				pServer->SetHttpFilterCb(m_httpFilter);
				m_enetServerArray[serverType] = pServer;
			}

			uint64_t linkId = pServer->BindServer(flag);
			if (linkId > 0)
			{
				return linkId;
			}

			NFLogError(NF_LOG_DEFAULT, 0, "Add Server Failed!");
		}
		else if (addr.mScheme == "bus")
		{
			uint32_t busid = NFServerIDUtil::GetBusID(addr.mHost);
			if (busid <= 0)
			{
				NFLogError(NF_LOG_DEFAULT, 0, "BusAddrAton Failed! host:{}", addr.mHost);
				return 0;
			}
			NFMessageFlag flag;
			flag.mStrIp = addr.mHost;
			flag.nPort = addr.mPort;
			flag.mBusId = busid;
			flag.mBusLength = addr.mPort;
			flag.mPacketParseType = packetParseType;
			flag.nNetThreadNum = netThreadNum;
			flag.mMaxConnectNum = maxConnectNum;
			flag.mSecurity = security;

			NFINetMessage* pServer = m_busServerArray[serverType];
			if (!pServer)
			{
				pServer = NF_NEW NFCBusMessage(m_pObjPluginManager, serverType);

				pServer->SetRecvCb(m_recvCb);
				pServer->SetEventCb(m_eventCb);
				m_busServerArray[serverType] = pServer;
			}

			uint64_t linkId = pServer->BindServer(flag);
			if (linkId > 0)
			{
				return linkId;
			}

			NFLogError(NF_LOG_DEFAULT, 0, "Add Server Failed!");
		}
	}
	return 0;
}

/**
 * @brief 获取连接的IP地址
 * 
 * 根据连接ID获取对应的IP地址
 * 
 * @param linkId 连接ID
 * @return IP地址字符串
 */
std::string NFCNetModule::GetLinkIp(uint64_t linkId)
{
	uint32_t serverType = GetServerTypeFromUnlinkId(linkId);
	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		uint32_t isServer = GetServerLinkModeFromUnlinkId(linkId);
		if (isServer == NF_IS_NET)
		{
			auto pServer = m_evppServerArray[serverType];
			if (pServer)
			{
				return pServer->GetLinkIp(linkId);
			}
			else
			{
				return std::string();
			}
		}
		else if (isServer == NF_IS_ENET)
		{
			auto pServer = m_enetServerArray[serverType];
			if (pServer)
			{
				return pServer->GetLinkIp(linkId);
			}
			else
			{
				return std::string();
			}
		}
		else
		{
			auto pServer = m_busServerArray[serverType];
			if (pServer)
			{
				return pServer->GetLinkIp(linkId);
			}
			else
			{
				return std::string();
			}
		}
	}
	return std::string();
}

/**
 * @brief 获取连接的端口号
 * 
 * 根据连接ID获取对应的端口号
 * 
 * @param linkId 连接ID
 * @return 端口号
 */
uint32_t NFCNetModule::GetPort(uint64_t linkId)
{
	uint32_t serverType = GetServerTypeFromUnlinkId(linkId);
	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		uint32_t isServer = GetServerLinkModeFromUnlinkId(linkId);
		if (isServer == NF_IS_NET)
		{
			auto pServer = m_evppServerArray[serverType];
			if (pServer)
			{
				return pServer->GetPort(linkId);
			}
			else
			{
				return 0;
			}
		}
		else if (isServer == NF_IS_ENET)
		{
			auto pServer = m_enetServerArray[serverType];
			if (pServer)
			{
				return pServer->GetPort(linkId);
			}
			else
			{
				return 0;
			}
		}
		else
		{
			auto pServer = m_busServerArray[serverType];
			if (pServer)
			{
				return pServer->GetPort(linkId);
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;
}

/**
 * @brief 关闭指定的连接
 * 
 * 根据连接ID关闭对应的连接
 * 
 * @param linkId 连接ID
 */
void NFCNetModule::CloseLinkId(uint64_t linkId)
{
	if (linkId == 0) return;

	uint32_t serverType = GetServerTypeFromUnlinkId(linkId);

	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		uint32_t isServer = GetServerLinkModeFromUnlinkId(linkId);
		if (isServer == NF_IS_NET)
		{
			auto pServer = m_evppServerArray[serverType];
			if (pServer)
			{
				pServer->CloseLinkId(linkId);
				return;
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "CloseLinkId error, usLinkId:{} not exist!", linkId);
			}
		}
		else if (isServer == NF_IS_ENET)
		{
			auto pServer = m_enetServerArray[serverType];
			if (pServer)
			{
				pServer->CloseLinkId(linkId);
				return;
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "CloseLinkId error, usLinkId:{} not exist!", linkId);
			}
		}
		else
		{
			auto pServer = m_busServerArray[serverType];
			if (pServer)
			{
				pServer->CloseLinkId(linkId);
				return;
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "CloseLinkId error, usLinkId:{} not exist!", linkId);
			}
		}
	}
	NFLogError(NF_LOG_DEFAULT, 0, "CloseLinkId error, usLinkId:{} not exist!", linkId);
}

/**
 * @brief 发送字符串数据
 * 
 * 将字符串数据打包成数据包并发送
 * 
 * @param linkId 连接ID
 * @param moduleId 模块ID
 * @param msgId 消息ID
 * @param strData 字符串数据
 * @param param1 参数1
 * @param param2 参数2
 * @param srcId 源ID
 * @param dstId 目标ID
 */
void NFCNetModule::Send(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const std::string& strData, uint64_t param1, uint64_t param2, uint64_t srcId, uint64_t dstId)
{
	NFDataPackage packet;
	packet.mModuleId = moduleId;
	packet.nMsgId = msgId;
	packet.nParam1 = param1;
	packet.nParam2 = param2;
	packet.nSrcId = srcId;
	packet.nDstId = dstId;

	Send(linkId, packet, strData.data(), strData.length());
}

/**
 * @brief 发送原始字节数据
 * 
 * 将原始字节数据打包成数据包并发送
 * 
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
void NFCNetModule::Send(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const char* msg, uint32_t len, uint64_t param1, uint64_t param2, uint64_t srcId, uint64_t dstId)
{
	NFDataPackage packet;
	packet.mModuleId = moduleId;
	packet.nMsgId = msgId;
	packet.nParam1 = param1;
	packet.nParam2 = param2;
	packet.nSrcId = srcId;
	packet.nDstId = dstId;

	Send(linkId, packet, msg, len);
}

/**
 * @brief 发送Protobuf消息
 * 
 * 将Protobuf消息打包成数据包并发送
 * 
 * @param linkId 连接ID
 * @param moduleId 模块ID
 * @param msgId 消息ID
 * @param data Protobuf消息对象
 * @param param1 参数1
 * @param param2 参数2
 * @param srcId 源ID
 * @param dstId 目标ID
 */
void NFCNetModule::Send(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const google::protobuf::Message& data, uint64_t param1, uint64_t param2, uint64_t srcId, uint64_t dstId)
{
	NFDataPackage packet;
	packet.mModuleId = moduleId;
	packet.nMsgId = msgId;
	packet.nParam1 = param1;
	packet.nParam2 = param2;
	packet.nSrcId = srcId;
	packet.nDstId = dstId;

	Send(linkId, packet, data);
}

/**
 * @brief 向服务器发送字符串数据
 * 
 * 将字符串数据打包成数据包并发送给服务器
 * 
 * @param linkId 连接ID
 * @param moduleId 模块ID
 * @param msgId 消息ID
 * @param strData 字符串数据
 * @param param1 参数1
 * @param param2 参数2
 * @param srcId 源ID
 * @param dstId 目标ID
 */
void NFCNetModule::SendServer(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const std::string& strData, uint64_t param1, uint64_t param2, uint64_t srcId, uint64_t dstId)
{
	NFDataPackage packet;
	packet.mModuleId = moduleId;
	packet.nMsgId = msgId;
	packet.nParam1 = param1;
	packet.nParam2 = param2;
	packet.nSrcId = srcId;
	packet.nDstId = dstId;

	Send(linkId, packet, strData.data(), strData.length());
}

/**
 * @brief 向服务器发送原始字节数据
 * 
 * 将原始字节数据打包成数据包并发送给服务器
 * 
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
void NFCNetModule::SendServer(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const char* msg, uint32_t len, uint64_t param1, uint64_t param2, uint64_t srcId, uint64_t dstId)
{
	NFDataPackage packet;
	packet.mModuleId = moduleId;
	packet.nMsgId = msgId;
	packet.nParam1 = param1;
	packet.nParam2 = param2;
	packet.nSrcId = srcId;
	packet.nDstId = dstId;

	Send(linkId, packet, msg, len);
}

/**
 * @brief 向服务器发送Protobuf消息
 * 
 * 将Protobuf消息打包成数据包并发送给服务器
 * 
 * @param linkId 连接ID
 * @param moduleId 模块ID
 * @param msgId 消息ID
 * @param data Protobuf消息对象
 * @param param1 参数1
 * @param param2 参数2
 * @param srcId 源ID
 * @param dstId 目标ID
 */
void NFCNetModule::SendServer(uint64_t linkId, uint32_t moduleId, uint32_t msgId, const google::protobuf::Message& data, uint64_t param1, uint64_t param2, uint64_t srcId, uint64_t dstId)
{
	NFDataPackage packet;
	packet.mModuleId = moduleId;
	packet.nMsgId = msgId;
	packet.nParam1 = param1;
	packet.nParam2 = param2;
	packet.nSrcId = srcId;
	packet.nDstId = dstId;

	Send(linkId, packet, data);
}

/**
 * @brief 透传数据包
 * 
 * 直接发送数据包，不进行额外的处理
 * 
 * @param linkId 连接ID
 * @param packet 数据包对象
 */
void NFCNetModule::TransPackage(uint64_t linkId, NFDataPackage& packet)
{
	Send(linkId, packet, packet.GetBuffer(), packet.GetSize());
}

/**
 * @brief 发送数据包（原始数据）
 * 
 * 根据连接ID选择合适的服务器发送数据包
 * 
 * @param linkId 连接ID
 * @param packet 数据包
 * @param msg 消息数据
 * @param len 数据长度
 * @return true 发送成功，false 发送失败
 */
bool NFCNetModule::Send(uint64_t linkId, NFDataPackage& packet, const char* msg, uint32_t len)
{
	uint32_t serverType = GetServerTypeFromUnlinkId(linkId);

	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		uint32_t isServer = GetServerLinkModeFromUnlinkId(linkId);
		if (isServer == NF_IS_NET)
		{
			auto pServer = m_evppServerArray[serverType];
			if (pServer)
			{
				return Send(pServer, linkId, packet, msg, len);
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "SendByServerID error, usLinkId:{} not exist!", linkId);
			}
		}
		else if (isServer == NF_IS_ENET)
		{
			auto pServer = m_enetServerArray[serverType];
			if (pServer)
			{
				return Send(pServer, linkId, packet, msg, len);
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "SendByServerID error, usLinkId:{} not exist!", linkId);
			}
		}
		else
		{
			auto pServer = m_busServerArray[serverType];
			if (pServer)
			{
				return Send(pServer, linkId, packet, msg, len);
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "bus SendByServerID error, usLinkId:{} not exist!", linkId);
			}
		}
	}

	if (linkId != 0)
	{
		NFLogError(NF_LOG_DEFAULT, 0, "SendByServerID error, usLinkId:{} not exist!", linkId);
	}

	return false;
}

/**
 * @brief 发送数据包（Protobuf数据）
 * 
 * 根据连接ID选择合适的服务器发送Protobuf数据包
 * 
 * @param linkId 连接ID
 * @param packet 数据包
 * @param data Protobuf消息对象
 * @return true 发送成功，false 发送失败
 */
bool NFCNetModule::Send(uint64_t linkId, NFDataPackage& packet, const google::protobuf::Message& data)
{
	uint32_t serverType = GetServerTypeFromUnlinkId(linkId);

	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		uint32_t isServer = GetServerLinkModeFromUnlinkId(linkId);
		if (isServer == NF_IS_NET)
		{
			auto pServer = m_evppServerArray[serverType];
			if (pServer)
			{
				return Send(pServer, linkId, packet, data);
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "SendByServerID error, usLinkId:{} not exist!", linkId);
			}
		}
		else if (isServer == NF_IS_ENET)
		{
			auto pServer = m_enetServerArray[serverType];
			if (pServer)
			{
				return Send(pServer, linkId, packet, data);
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "SendByServerID error, usLinkId:{} not exist!", linkId);
			}
		}
		else
		{
			auto pServer = m_busServerArray[serverType];
			if (pServer)
			{
				return Send(pServer, linkId, packet, data);
			}
			else
			{
				NFLogError(NF_LOG_DEFAULT, 0, "bus SendByServerID error, usLinkId:{} not exist!", linkId);
			}
		}
	}

	if (linkId != 0)
	{
		NFLogError(NF_LOG_DEFAULT, 0, "SendByServerID error, usLinkId:{} not exist!", linkId);
	}

	return false;
}

/**
 * @brief 将消息编码后通过pServer发送出去（原始数据）
 * 
 * @param pServer 网络消息对象指针
 * @param linkId 连接ID
 * @param packet 数据包
 * @param msg 消息数据
 * @param len 数据长度
 * @return true 发送成功，false 发送失败
 */
bool NFCNetModule::Send(NFINetMessage* pServer, uint64_t linkId, NFDataPackage& packet, const char* msg, uint32_t len)
{
	if (pServer)
	{
		return pServer->Send(linkId, packet, msg, len);
	}
	return false;
}

/**
 * @brief 将Protobuf消息编码后通过pServer发送出去
 * 
 * @param pServer 网络消息对象指针
 * @param linkId 连接ID
 * @param packet 数据包
 * @param data Protobuf消息对象
 * @return true 发送成功，false 发送失败
 */
bool NFCNetModule::Send(NFINetMessage* pServer, uint64_t linkId, NFDataPackage& packet, const google::protobuf::Message& data)
{
	if (pServer)
	{
		return pServer->Send(linkId, packet, data);
	}
	return false;
}

/**
 * @brief 响应HTTP消息（使用HTTP句柄）
 * 
 * 通过Evpp服务器响应HTTP消息
 * 
 * @param serverType 服务器类型
 * @param req HTTP请求句柄
 * @param strMsg 响应消息
 * @param code HTTP状态码
 * @param reason 状态原因
 * @return true 响应成功，false 响应失败
 */
bool NFCNetModule::ResponseHttpMsg(NF_SERVER_TYPE serverType, const NFIHttpHandle& req, const std::string& strMsg, NFWebStatus code, const std::string& reason)
{
	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		NFINetMessage* pServer = m_evppServerArray[serverType];
		if (pServer)
		{
			return pServer->ResponseHttpMsg(req, strMsg, code, reason);
		}
	}
	return false;
}

/**
 * @brief 响应HTTP消息（使用请求ID）
 * 
 * 通过Evpp服务器响应HTTP消息
 * 
 * @param serverType 服务器类型
 * @param requestId 请求ID
 * @param strMsg 响应消息
 * @param code HTTP状态码
 * @param reason 状态原因
 * @return true 响应成功，false 响应失败
 */
bool NFCNetModule::ResponseHttpMsg(NF_SERVER_TYPE serverType, uint64_t requestId, const std::string& strMsg, NFWebStatus code, const std::string& reason)
{
	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		NFINetMessage* pServer = m_evppServerArray[serverType];
		if (pServer)
		{
			return pServer->ResponseHttpMsg(requestId, strMsg, code, reason);
		}
	}
	return false;
}

/**
 * @brief 发送HTTP GET请求
 * 
 * 通过Evpp服务器发送HTTP GET请求
 * 
 * @param serverType 服务器类型
 * @param strUri 请求URI
 * @param respone 响应回调
 * @param xHeaders 请求头
 * @param timeout 超时时间
 * @return 请求结果
 */
int NFCNetModule::HttpGet(NF_SERVER_TYPE serverType, const std::string& strUri, const HTTP_CLIENT_RESPONE& respone, const std::map<std::string, std::string>& xHeaders, int timeout)
{
	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		NFINetMessage* pServer = m_evppServerArray[serverType];
		if (pServer)
		{
			return pServer->HttpGet(strUri, respone, xHeaders, timeout);
		}
	}
	return -1;
}

/**
 * @brief 发送HTTP POST请求
 * 
 * 通过Evpp服务器发送HTTP POST请求
 * 
 * @param serverType 服务器类型
 * @param strUri 请求URI
 * @param strPostData POST数据
 * @param respone 响应回调
 * @param xHeaders 请求头
 * @param timeout 超时时间
 * @return 请求结果
 */
int NFCNetModule::HttpPost(NF_SERVER_TYPE serverType, const std::string& strUri, const std::string& strPostData, const HTTP_CLIENT_RESPONE& respone, const std::map<std::string, std::string>& xHeaders, int timeout)
{
	if (serverType > NF_ST_NONE && serverType < NF_ST_MAX)
	{
		NFINetMessage* pServer = m_evppServerArray[serverType];
		if (pServer)
		{
			return pServer->HttpPost(strUri, strPostData, respone, xHeaders, timeout);
		}
	}
	return -1;
}

/**
 * @brief 发送邮件
 * 
 * 使用SMTP协议发送邮件，目前只在Linux平台支持
 * 
 * @param serverType 服务器类型
 * @param title 邮件标题
 * @param subject 邮件主题
 * @param content 邮件内容
 * @return 发送结果，0表示成功，-1表示失败
 */
int NFCNetModule::SendEmail(NF_SERVER_TYPE serverType, const std::string& title, const std::string& subject, const string& content)
{
#if NF_PLATFORM == NF_PLATFORM_LINUX
    NFServerConfig* pConfig = FindModule<NFIConfigModule>()->GetAppConfig(NF_ST_MASTER_SERVER);
    CHECK_NULL(0, pConfig);

    NFSmtpSendMail sendMail;
    sendMail.SetSmtpServer(pConfig->sendEmail, pConfig->sendEmailPass,pConfig->sendEmailUrl, pConfig->sendEmailPort);
    sendMail.SetSendName(title);
    sendMail.SetSendMail(pConfig->sendEmail);
    sendMail.AddRecvMail(pConfig->recvEmail);
    sendMail.SetSubject(subject);
    std::string dumpInfo = content;
    NFStringUtility::Replace(dumpInfo, "\n", "<br/>");
    sendMail.SetBodyContent(dumpInfo);
    if (!sendMail.SendMail())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "Send Message(title:{} subject:{}) To Email:{} Failed", title, subject, pConfig->recvEmail);
        return -1;
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "Send Message(title:{} subject:{}) To Email:{} Success", title, subject, pConfig->recvEmail);
        return 0;
    }
#else
	return 0;
#endif
}
