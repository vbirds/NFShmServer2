/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.9 */

#include <sstream>
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "ServerCommon.nanopb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

PacketMsg::PacketMsg()
{
	if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode()) {
		CreateInit();
	} else {
		ResumeInit();
	}
}

PacketMsg::~PacketMsg()
{
}

PacketMsg::PacketMsg(const PacketMsg& stArgsData)
{
	if (this != &stArgsData) {
		CopyData(stArgsData);
	}
}

PacketMsg& PacketMsg::operator=(const PacketMsg& stArgsData)
{
	if (this != &stArgsData) {
		CopyData(stArgsData);
	}
	return *this;
}

void PacketMsg::CopyData(const PacketMsg& stArgsData)
{
    cmd = stArgsData.cmd;
    serverType = stArgsData.serverType;
    upper_limit = stArgsData.upper_limit;
    min_interval = stArgsData.min_interval;
    control_time = stArgsData.control_time;
}

int PacketMsg::CreateInit()
{
    cmd = 0;
    serverType = 0;
    upper_limit = 0;
    min_interval = 0;
    control_time = 0;
	return 0;
}

int PacketMsg::ResumeInit()
{
	return 0;
}

void PacketMsg::Init()
{
    cmd = 0;
    serverType = 0;
    upper_limit = 0;
    min_interval = 0;
    control_time = 0;
}

bool PacketMsg::FromPb(const NFServer::PacketMsg& cc)
{
    cmd = cc.cmd();
    serverType = cc.servertype();
    upper_limit = cc.upper_limit();
    min_interval = cc.min_interval();
    control_time = cc.control_time();
    return true;
}

void PacketMsg::ToPb(NFServer::PacketMsg* cc) const
{
    cc->set_cmd(cmd);
    cc->set_servertype(serverType);
    cc->set_upper_limit(upper_limit);
    cc->set_min_interval(min_interval);
    cc->set_control_time(control_time);
    return;
}

NFServer::PacketMsg PacketMsg::ToPb() const
{
    NFServer::PacketMsg cc;
    ToPb(&cc);
    return cc;
}

std::string PacketMsg::ShortDebugString() const
{
    std::stringstream ss;
    ss << "{";
    ss << "cmd:" << cmd << ", ";
    ss << "serverType:" << serverType << ", ";
    ss << "upper_limit:" << upper_limit << ", ";
    ss << "min_interval:" << min_interval << ", ";
    ss << "control_time:" << control_time;
    ss << "}";
    return ss.str();
}

ServerPacketMsg::ServerPacketMsg()
{
	if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode()) {
		CreateInit();
	} else {
		ResumeInit();
	}
}

ServerPacketMsg::~ServerPacketMsg()
{
}

ServerPacketMsg::ServerPacketMsg(const ServerPacketMsg& stArgsData)
{
	if (this != &stArgsData) {
		CopyData(stArgsData);
	}
}

ServerPacketMsg& ServerPacketMsg::operator=(const ServerPacketMsg& stArgsData)
{
	if (this != &stArgsData) {
		CopyData(stArgsData);
	}
	return *this;
}

void ServerPacketMsg::CopyData(const ServerPacketMsg& stArgsData)
{
    PacketMsg_list.clear();
    PacketMsg_list = stArgsData.PacketMsg_list;
}

int ServerPacketMsg::CreateInit()
{
	return 0;
}

int ServerPacketMsg::ResumeInit()
{
	return 0;
}

void ServerPacketMsg::Init()
{
}

bool ServerPacketMsg::FromPb(const NFServer::ServerPacketMsg& cc)
{
    PacketMsg_list.clear();
    for (int i = 0; i < cc.packetmsg_list_size(); ++i)
    {
        PacketMsg temp;
        if (!temp.FromPb(cc.packetmsg_list(i)))
        {
            if (NULL != g_nanopb_frompb_log_handle)
                g_nanopb_frompb_log_handle("FromPb Failed, struct:ServerPacketMsg, field:PacketMsg_list, cur count:%d", cc.packetmsg_list_size());
            return false;
        }
        PacketMsg_list.push_back(temp);
    }
    return true;
}

void ServerPacketMsg::ToPb(NFServer::ServerPacketMsg* cc) const
{
    cc->clear_packetmsg_list();
    for (auto iter = PacketMsg_list.begin(); iter != PacketMsg_list.end(); ++iter)
    {
        iter->ToPb(cc->add_packetmsg_list());
    }
    return;
}

NFServer::ServerPacketMsg ServerPacketMsg::ToPb() const
{
    NFServer::ServerPacketMsg cc;
    ToPb(&cc);
    return cc;
}

std::string ServerPacketMsg::ShortDebugString() const
{
    std::stringstream ss;
    ss << "{";
    ss << "PacketMsg_list(" << PacketMsg_list.size()<< "):[";
    for (auto iter = PacketMsg_list.begin(); iter != PacketMsg_list.end(); ++iter)
    {
        ss << iter->ShortDebugString();
    }
;
    ss << "}";
    return ss.str();
}




/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(ServerPacketMsg, PacketMsg_list[0]) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_PacketMsg_ServerPacketMsg)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(ServerPacketMsg, PacketMsg_list[0]) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_PacketMsg_ServerPacketMsg)
#endif


/* @@protoc_insertion_point(eof) */
