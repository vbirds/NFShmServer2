﻿// -------------------------------------------------------------------------
//    @FileName         :    NFIPacketParse.cpp
//    @Author           :    xxxxx
//    @Date             :   xxxx-xx-xx
//    @Email			:    xxxxxxxxx@xxx.xxx
//    @Module           :    NFNetPlugin
//
// -------------------------------------------------------------------------

#include "NFIPacketParse.h"
#include "InternalPacketParse.h"
#include "ExternalPacketParse.h"

std::vector<NFIPacketParse*> NFIPacketParse::m_pPacketParse = { CreatePacketParse(PACKET_PARSE_TYPE_INTERNAL), CreatePacketParse(PACKET_PARSE_TYPE_EXTERNAL), CreatePacketParse(PACKET_PARSE_TYPE_OLD_EXTERNAL) };

void NFIPacketParse::DeletePacketParse(NFIPacketParse* pPacketParse)
{
	delete pPacketParse;
}

NFIPacketParse* NFIPacketParse::CreatePacketParse(int parseType)
{
	if (parseType == PACKET_PARSE_TYPE_INTERNAL)
	{
		return NF_NEW InternalPacketParse();
	}
	else if (parseType == PACKET_PARSE_TYPE_EXTERNAL)
	{
		return NF_NEW ExternalPacketParse();
	}
	else
	{
		return NF_NEW InternalPacketParse();
	}
}

int NFIPacketParse::DeCode(uint32_t packetType, const char* strData, uint32_t unLen, char*& outData, uint32_t& outLen, uint32_t& allLen, NFDataPackage& recvPackage)
{
	if (packetType < 0 || packetType >= m_pPacketParse.size())
	{
		return m_pPacketParse[PACKET_PARSE_TYPE_INTERNAL]->DeCodeImpl(strData, unLen, outData, outLen, allLen, recvPackage);
	}
	return m_pPacketParse[packetType]->DeCodeImpl(strData, unLen, outData, outLen, allLen, recvPackage);
}

int NFIPacketParse::EnCode(uint32_t packetType, const uint32_t unMsgID, uint64_t nSendValue, uint64_t nSendId, const char* strData, const uint32_t unDataLen, NFBuffer& buffer, uint64_t nSendBusLinkId)
{
	if (packetType < 0 || packetType >= m_pPacketParse.size())
	{
		return m_pPacketParse[PACKET_PARSE_TYPE_INTERNAL]->EnCodeImpl(unMsgID, nSendValue, nSendId, strData, unDataLen, buffer, nSendBusLinkId);
	}
	return m_pPacketParse[packetType]->EnCodeImpl(unMsgID, nSendValue, nSendId, strData, unDataLen, buffer, nSendBusLinkId);
}

// 使用 lzf 算法 压缩、解压
int NFIPacketParse::Compress(int32_t packetType, const char* inBuffer, int inLen, void *outBuffer, unsigned int outSize)
{
    if (packetType < 0 || packetType >= m_pPacketParse.size())
    {
        return m_pPacketParse[PACKET_PARSE_TYPE_INTERNAL]->CompressImpl(inBuffer, inLen, outBuffer, outSize);
    }
    return m_pPacketParse[packetType]->CompressImpl(inBuffer, inLen, outBuffer, outSize);
}

int NFIPacketParse::Decompress(int32_t packetType, const char* inBuffer, int inLen, void *outBuffer, int outSize)
{
    if (packetType < 0 || packetType >= m_pPacketParse.size())
    {
        return m_pPacketParse[PACKET_PARSE_TYPE_INTERNAL]->DecompressImpl(inBuffer, inLen, outBuffer, outSize);
    }
    return m_pPacketParse[packetType]->DecompressImpl(inBuffer, inLen, outBuffer, outSize);
}