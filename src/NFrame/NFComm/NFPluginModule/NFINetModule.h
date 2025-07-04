// -------------------------------------------------------------------------
//    @FileName         :    NFIMsgModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFIModule.h"
#include "google/protobuf/message.h"
#include "NFComm/NFPluginModule/NFIHttpHandle.h"

#include <stdint.h>

#include "NFIPacketParse.h"

/// @brief 网络驱动接口
class NFINetModule: public NFIModule
{
public:
	NFINetModule(NFIPluginManager* p): NFIModule(p) {}
	virtual ~NFINetModule() {}

	/**
	 *@brief  设置接收回调.
	 */
	template <typename BaseType>
	void SetRecvCB(BaseType* pBaseType, int (BaseType::*handleRecieve)(uint64_t connectionLink, uint64_t objectLinkId, NFDataPackage& packet))
	{
		NET_CALLBACK_RECEIVE_FUNCTOR func = std::bind(handleRecieve, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		SetRecvCB(func);
	}

	/**
	 *@brief  设置连接事件回调.
	 */
	template <typename BaseType>
	void SetEventCB(BaseType* pBaseType, int (BaseType::*handleEvent)(eMsgType nEvent, uint64_t connectionLink, uint64_t objectLinkId))
	{
		NET_CALLBACK_EVENT_FUNCTOR func = std::bind(handleEvent, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		SetEventCB(func);
	}

    /**
    *@brief  设置接收回调.
    */
    template<typename BaseType>
    void SetHttpRecvCB(BaseType *pBaseType, bool (BaseType::*handleRecieve)(uint32_t, const NFIHttpHandle &req)) {
        HTTP_RECEIVE_FUNCTOR func = std::bind(handleRecieve, pBaseType, std::placeholders::_1, std::placeholders::_2);
        SetHttpRecvCB(func);
    }

    /**
     *@brief  设置连接事件回调.
     */
    template<typename BaseType>
    void SetHttpFilterCB(BaseType *pBaseType, NFWebStatus(BaseType::*handleFilter)(uint32_t, const NFIHttpHandle &req)) {
        HTTP_FILTER_FUNCTOR func = std::bind(handleFilter, pBaseType, std::placeholders::_1, std::placeholders::_2);
        SetHttpFilterCB(func);
    }

	/**
	 *@brief  设置接收回调.
	 */
	virtual void SetRecvCB(const NET_CALLBACK_RECEIVE_FUNCTOR& recvcb) = 0;

	/**
	 *@brief  设置连接事件回调.
	 */
	virtual void SetEventCB(const NET_CALLBACK_EVENT_FUNCTOR& eventcb) = 0;

    /**
    *@brief  设置接收回调.
    */
    virtual void SetHttpRecvCB(const HTTP_RECEIVE_FUNCTOR& recvcb) = 0;

    /**
     *@brief  设置连接事件回调.
     */
    virtual void SetHttpFilterCB(const HTTP_FILTER_FUNCTOR& eventcb) = 0;

	/**
	 * @brief 添加服务器
	 *
	 * @param  eType		服务器类型
	 * @param  nServerID	服务器ID
	 * @param  nMaxClient	服务器最大连接客户端数
	 * @param  nPort		服务器监听端口
	 * @return int			返回0错误
	 */
	virtual uint64_t BindServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t netThreadNum = 1, uint32_t maxConnectNum = 100, uint32_t packetParseType = PACKET_PARSE_TYPE_INTERNAL, bool security = false) = 0;

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
	virtual uint64_t ConnectServer(NF_SERVER_TYPE serverType, const std::string& url, uint32_t packetParseType = 0, bool security = false) = 0;

    virtual int ResumeConnect(NF_SERVER_TYPE eServerType) = 0;

	virtual std::string GetLinkIp(uint64_t usLinkId) = 0;
    virtual uint32_t GetPort(uint64_t usLinkId) = 0;

	virtual void CloseLinkId(uint64_t usLinkId) = 0;

    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg,uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void Send(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const std::string& strData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const char* msg,uint32_t nLen, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void SendServer(uint64_t usLinkId, uint32_t nModuleId, uint32_t nMsgID, const google::protobuf::Message& xData, uint64_t param1 = 0, uint64_t param2 = 0, uint64_t srcId = 0, uint64_t dstId = 0) = 0;

    virtual void TransPackage(uint64_t usLinkId, NFDataPackage& packet) = 0;

    virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, const NFIHttpHandle &req, const std::string &strMsg,
                                 NFWebStatus code = NFWebStatus::WEB_OK, const std::string &reason = "OK") = 0;

    virtual bool ResponseHttpMsg(NF_SERVER_TYPE serverType, uint64_t requestId, const std::string &strMsg,
                                 NFWebStatus code = NFWebStatus::WEB_OK,
                                 const std::string &reason = "OK") = 0;

    virtual int HttpGet(NF_SERVER_TYPE serverType, const std::string &strUri,
                        const HTTP_CLIENT_RESPONE &respone,
                        const std::map<std::string, std::string> &xHeaders = std::map<std::string, std::string>(),
                        int timeout = 3) = 0;

    virtual int HttpPost(NF_SERVER_TYPE serverType, const std::string &strUri, const std::string &strPostData, const HTTP_CLIENT_RESPONE &respone,
                         const std::map<std::string, std::string> &xHeaders = std::map<std::string, std::string>(),
                         int timeout = 3) = 0;

    virtual int SendEmail(NF_SERVER_TYPE serverType, const std::string& title, const std::string& subject, const string &content) = 0;
};
