// -------------------------------------------------------------------------
//    @FileName         :    NFServerDefine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------
#pragma once

#include <functional>
#include <list>
#include <string>
#include <vector>

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFSlice.hpp"
#include "NFComm/NFCore/NFBuffer.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"
#include "NFComm/NFKernelMessage/FrameEnum.pb.h"
#include "NFComm/NFKernelMessage/FrameEnum.nanopb.h"

#define MAX_CLIENT_INDEX 1000000                  //客户端掩码 一百万
#define MAX_CLIENT_MASK 0xfffff                   //0x000 00000000 fffff 后20位，5个f，给客户端索引用, 客户端掩码20位, 最大1048576 > 一百万 1000000
#define MAX_BUS_ID_MASK 0xffffffff00000           //0x0 00 ffffffff 00000 中间32位， 从21位到52位，给服务器的唯一ID，busId用， 需要左移位20
#define MAX_SERVER_TYPE_MASK 0x0ff0000000000000   //0x0 ff 00000000 00000从53到60位， 给服务器类型用，需要左移位52
#define MAX_IS_SERVER_MASK 0xf000000000000000     //0xf 00 00000000 00000从61到64位， 是什么类型的， 网络net, 共享内存bus

#define NF_IS_NONE 0
#define NF_IS_NET 1
#define NF_IS_BUS 2
#define NF_IS_ENET 3

#define GetUnLinkId(linkMode, serverType, busId, serverIndex)    ((((uint64_t)serverIndex) & MAX_CLIENT_MASK) | ((((uint64_t)busId) << 20) & MAX_BUS_ID_MASK)  | ((((uint64_t)serverType) << 52) & MAX_SERVER_TYPE_MASK) | ((((uint64_t)linkMode << 60) & MAX_IS_SERVER_MASK)));
#define GetServerTypeFromUnlinkId(UnlinkId)        ((((uint64_t)UnlinkId) & MAX_SERVER_TYPE_MASK) >> 52);
#define GetServerLinkModeFromUnlinkId(UnlinkId)        ((((uint64_t)UnlinkId) & MAX_IS_SERVER_MASK) >> 60);
#define GetServerIndexFromUnlinkId(UnlinkId)    (((uint64_t)UnlinkId) & MAX_CLIENT_MASK);
#define GetBusIdFromUnlinkId(UnlinkId)    ((((uint64_t)UnlinkId) & MAX_BUS_ID_MASK) >> 20);

#define CLIENT_MSG_PROCESS_NO_PRINTF(xPacket, xMsg)                 \
    if (!xMsg.ParseFromArray(xPacket.GetBuffer(), xPacket.GetSize()))                \
    {                                                    \
        NFLogError(NF_LOG_DEFAULT, 0, "Protobuf Parse Message Failed, packet:{}", xPacket.ToString()); \
        return -1;                                        \
    }

#define CLIENT_MSG_PROCESS_WITH_PRINTF(xPacket, xMsg)                 \
    if (!xMsg.ParseFromArray(xPacket.GetBuffer(), xPacket.GetSize()))                \
    {                                                    \
        NFLogError(NF_LOG_DEFAULT, 0, "Protobuf Parse Message Failed, packet:{} error:{}", xPacket.ToString(), xMsg.InitializationErrorString()); \
        return -1;                                        \
    }\
    NFLogTrace(NF_LOG_DEFAULT, xPacket.nParam1, "recv packet:{}, json:{}", xPacket.ToString(), xMsg.Utf8DebugString());

#define WEB_MSG_PROCESS_WITH_PRINTF(xMsg, reqHandle) \
            \
    if (reqHandle.GetType() == NF_HTTP_REQ_GET)      \
    {                                                \
        if (NFProtobufCommon::GetMessageFromGetHttp(&xMsg, reqHandle) != 0)\
        {                                            \
            data.set_request_id(req.GetRequestId());                                          \
            NFLogError(NF_LOG_DEFAULT, 0, "Protobuf Parse Message Failed Fromn Http Get, get uri:{}", req.GetOriginalUri()); \
            return false;                               \
        }                                            \
        data.set_request_id(req.GetRequestId());                                              \
        NFLogInfo(NF_LOG_DEFAULT, 0, "url:{}", reqHandle.GetOriginalUri()); \
    }                                                \
    else                                             \
    {                                                \
        std::string error;                                                 \
        if (!NFProtobufCommon::JsonToProtoMessage(reqHandle.GetBody(), &xMsg, &error))                \
        {                                            \
            data.set_request_id(req.GetRequestId());                                          \
            NFLogError(NF_LOG_DEFAULT, 0, "Protobuf Parse Message Failed Fromn Http Post Json, json:{}, error:{}", reqHandle.GetBody(), error); \
            return false;                                        \
        }                                            \
        data.set_request_id(req.GetRequestId());                                              \
        NFLogInfo(NF_LOG_DEFAULT, 0, "json:{}", reqHandle.GetBody()); \
    }
// (uint16) 如果修改增加此处大小，需要修改接受缓冲区大小，与客户端 NetDefine.cs 中的定义一直
const uint16_t MAX_CLIENT_NET_PACK_LENGTH = 1024 * 48;
const uint32_t MAX_SERVER_NET_PACK_LENGTH = 1024 * 512;
const uint32_t INNER_SENDBUF_DEFAULT_SIZE = 1024 * 1024 * 2;
const uint32_t INNER_RECVBUF_DEFAULT_SIZE = 1024 * 1024 * 2;
const uint32_t MAX_WEB_NET_PACK_LENGTH    = 1024 * 512;

#define WG_INT_MAX32 0x7FFFFFFFL

enum NF_MODULE_TYPE
{
	NF_MODULE_FRAME = 0,
	NF_MODULE_SERVER = 1, //服务器内网通讯
	NF_MODULE_CLIENT = 2, //客户端外部协议
	NF_MODULE_MAX    = 3,
};

#define NF_NET_MAX_MSG_ID 20000

enum PacketParseType
{
	PACKET_PARSE_TYPE_INTERNAL        = 0, //内网协议
	PACKET_PARSE_TYPE_EXTERNAL = 1, //外部协议
};

enum
{
	APP_INIT_TASK_GROUP_NONE                    = 0,
	APP_INIT_TASK_GROUP_SERVER_CONNECT          = 1,
	APP_INIT_TASK_GROUP_SERVER_LOAD_DESC_STORE  = 2,
	APP_INIT_TASK_GROUP_SERVER_LOAD_OBJ_FROM_DB = 3,
	APP_INIT_TASK_GROUP_SERVER_REGISTER         = 4,
	APP_INIT_MAX_TASK_GROUP                     = 5,
};

enum
{
	APP_INIT_NONE,
	//
	APP_INIT_CONNECT_MASTER,
	APP_INIT_CONNECT_ROUTE_AGENT_SERVER,
	APP_INIT_CONNECT_ROUTE_SERVER,
	APP_INIT_CONNECT_PROXY_AGENT_SERVER,
	APP_INIT_NEED_STORE_SERVER,
	APP_INIT_NEED_WORLD_SERVER,
	APP_INIT_NEED_CENTER_SERVER,
	APP_INIT_REGISTER_WORLD_SERVER,
	APP_INIT_REGISTER_CENTER_SERVER,

	//
	APP_INIT_DESC_STORE_LOAD,
	//

	APP_INIT_LOAD_GLOBAL_DATA_DB,

	APP_INIT_MAX,
};

#define NF_NO_FIX_FAME_HANDLE_MAX_MSG_COUNT 10000
#define NF_FIX_FRAME_HANDLE_MAX_MSG_COUNT 2000    //release test handle 2000 msg => cpu 20%

std::string GetServerName(NF_SERVER_TYPE serverId);
bool IsRouteServer(NF_SERVER_TYPE serverType);
bool IsMasterServer(NF_SERVER_TYPE serverType);
bool IsWorkServer(NF_SERVER_TYPE serverType);

enum eMsgType
{
	eMsgType_Num          = 0,
	eMsgType_CONNECTED    = 1,
	eMsgType_DISCONNECTED = 2,
	eMsgType_RECIVEDATA   = 3,
	eMsgType_SENDBUFFER   = 4,
};

enum eAccountEventType
{
	eAccountEventType_Num          = 0,
	eAccountEventType_CONNECTED    = 1,
	eAccountEventType_DISCONNECTED = 2,
	eAccountEventType_RECONNECTED  = 3,
};

const uint32_t s_compressBitPos = 15;

struct NFDataPackage
{
	NFDataPackage()
	{
		Clear();
	}

	virtual ~NFDataPackage()
	{
	}

	NFDataPackage(const NFDataPackage& data)
	{
		if (this != &data)
		{
			Copy(data);
		}
	}

	NFDataPackage& operator=(const NFDataPackage& data)
	{
		if (this != &data)
		{
			Copy(data);
		}
		return *this;
	}

	void Copy(const NFDataPackage& data)
	{
		mModuleId        = data.mModuleId;
		nMsgId           = data.nMsgId;
		nParam1          = data.nParam1;
		nParam2          = data.nParam2;
		nSrcId           = data.nSrcId;
		nDstId           = data.nDstId;
		nSendBusLinkId   = data.nSendBusLinkId;
		bCompress        = data.bCompress;
		nServerLinkId    = data.nServerLinkId;
		nObjectLinkId    = data.nObjectLinkId;
		nPacketParseType = data.nPacketParseType;
		isSecurity       = data.isSecurity;
		nErrCode         = data.nErrCode;
		nBuffer          = data.nBuffer;
		nMsgLen          = data.nMsgLen;
		nMsgSeq          = data.nMsgSeq;
	}

	std::string ToString() const
	{
		return NF_FORMAT("(mdouleId:{} msgId:{} param1:{} param2:{} nMsgLen:{} nMsgSeq:{})", mModuleId, nMsgId, nParam1, nParam2, nMsgLen, nMsgSeq);
	}

	void Clear()
	{
		mModuleId        = 0;
		nMsgId           = 0;
		nParam1          = 0;
		nParam2          = 0;
		nSrcId           = 0;
		nDstId           = 0;
		nSendBusLinkId   = 0;
		bCompress        = false;
		nServerLinkId    = 0;
		nObjectLinkId    = 0;
		nPacketParseType = 0;
		isSecurity       = false;
		nBuffer          = NULL;
		nMsgLen          = 0;
		nErrCode         = 0;
		nMsgSeq          = 0;
	}

	const char* GetBuffer() const
	{
		return nBuffer;
	}

	std::string GetData() const
	{
		return std::string(nBuffer, nMsgLen);
	}

	uint64_t GetSize() const
	{
		return nMsgLen;
	}

	uint64_t GetParam1() const
	{
		return nParam1;
	}

	uint64_t GetParam2() const
	{
		return nParam2;
	}

	uint64_t GetSrcId() const
	{
		return nSrcId;
	}

	uint64_t GetDstId() const
	{
		return nDstId;
	}

	uint64_t GetServerType() const
	{
		return GetServerTypeFromUnlinkId(nObjectLinkId);
	}

	uint64_t GetObjectLinkId() const
	{
		return nObjectLinkId;
	}

	uint64_t GetMsgId() const
	{
		return nMsgId;
	}

	uint32_t mModuleId;
	uint32_t nMsgId;
	uint64_t nParam1;
	uint64_t nParam2;
	uint64_t nSrcId;
	uint64_t nDstId;
	uint64_t nSendBusLinkId;
	bool     bCompress;
	uint64_t nServerLinkId;
	uint64_t nObjectLinkId;
	uint32_t nPacketParseType;
	bool     isSecurity;
	int32_t  nErrCode;
	char*    nBuffer;
	uint64_t nMsgLen;
	uint32_t nMsgSeq;
};

typedef std::function<int(uint64_t serverLinkId, uint64_t objectLinkId, NFDataPackage& packet)> NET_CALLBACK_RECEIVE_FUNCTOR;

typedef std::function<int(uint64_t unLinkId, NFDataPackage& packet)> NET_RECEIVE_FUNCTOR;

typedef std::function<int(eMsgType nEvent, uint64_t unLinkId)> NET_EVENT_FUNCTOR;

typedef std::function<int(eMsgType nEvent, uint64_t serverLinkId, uint64_t objectLinkId)> NET_CALLBACK_EVENT_FUNCTOR;

typedef std::function<void(int iRet, google::protobuf::Message& message)> RPC_TRANS_DESC_STORE_CB;

typedef std::function<int(uint64_t userId, const google::protobuf::Message* message)> LOG_BEHAVIOR_CALLBAK_FUNCTOR;

/**
* @brief log打印配置
*
* @param  mLogId log配置ID
* @param  mDisplay 是否打印这个LOG
* @param  mLevel 输出等级
* @param  mLogName 显示名字
*/
class LogInfoConfig
{
public:
	LogInfoConfig()
	{
		mLogId   = 0;
		mDisplay = false;
		mLevel   = 0;
	}

	uint32_t    mLogId;
	bool        mDisplay;
	uint32_t    mLevel;
	std::string mLogName;
};

//////////////////////////////////////////////////////////////////////////
class NFServerData
{
public:
	NFServerData()
	{
		mUnlinkId        = 0;
		mRouteAgentBusId = 0;
	}

	~NFServerData()
	{
		mUnlinkId        = 0;
		mRouteAgentBusId = 0;
	}

	uint32_t GetBusId() const { return mServerInfo.bus_id(); }

	std::string GetServerId() const { return mServerInfo.server_id(); }

	uint32_t GetServerType() const { return mServerInfo.server_type(); }

	std::string GetServerName() const { return mServerInfo.server_name(); }

	std::string GetServerIp() const { return mServerInfo.server_ip(); }

	uint32_t GetServerPort() const { return mServerInfo.server_port(); }

	uint32_t GetServerHttpPort() const { return mServerInfo.server_http_port(); }

	std::string GetExternalServerIp() const { return mServerInfo.external_server_ip(); }

	uint32_t GetExternalServerPort() const { return mServerInfo.external_server_port(); }

	std::list<uint64_t> GetMapIdList() const
	{
		std::list<uint64_t> list;
		for (int i = 0; i < mServerInfo.map_id_list_size(); ++i)
		{
			list.push_back(mServerInfo.map_id_list(i));
		}
		return list;
	}

	uint64_t                 mUnlinkId;
	uint32_t                 mRouteAgentBusId;
	NFrame::ServerInfoReport mServerInfo;
};

typedef struct tagUidAndIndex
{
	uint64_t m_ullUid;
	int32_t  m_iIdx;
} TUidAndIndex;

int UidCompare(const TUidAndIndex* pstLeft, const TUidAndIndex* pstRight);

int UidHash(const TUidAndIndex* pstKey);

struct TUid2Uid
{
	uint64_t m_ullUid;
	uint64_t m_ullTargetUid;
};

int Uid2Compare(const TUid2Uid* pstLeft, const TUid2Uid* pstRight);

int Uid2Hash(const TUid2Uid* pstKey);

const int MAX_NAME_STR_LEN = 32;

typedef struct tagStrAndID
{
	char m_szName[MAX_NAME_STR_LEN];
	int  m_iID;
} TStrAndID;

int StrCompare(const TStrAndID* pstLeft, const TStrAndID* pstRight);

int StrHash(const TStrAndID* pstKey);
