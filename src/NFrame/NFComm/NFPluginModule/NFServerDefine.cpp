// -------------------------------------------------------------------------
//    @FileName         :    NFServerDefine.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------
#include "NFServerDefine.h"

std::string GetServerName(NF_SERVER_TYPE serverId)
{
    return NFrame::NF_SERVER_TYPE_Name(static_cast<NFrame::NF_SERVER_TYPE>(serverId));
}

bool IsRouteServer(NF_SERVER_TYPE serverType)
{
    if (serverType == NF_ST_ROUTE_SERVER || serverType == NF_ST_ROUTE_AGENT_SERVER || serverType == NF_ST_PROXY_AGENT_SERVER)
    {
        return true;
    }
    return false;
}

bool IsMasterServer(NF_SERVER_TYPE serverType)
{
    if (serverType == NF_ST_MASTER_SERVER)
    {
        return true;
    }
    return false;
}

bool IsWorkServer(NF_SERVER_TYPE serverType)
{
    if (IsMasterServer(serverType) || IsRouteServer(serverType) || serverType == NF_ST_PROXY_SERVER || serverType == NF_ST_STORE_SERVER || serverType == NF_ST_NONE)
    {
        return false;
    }
    return true;
}

int UidCompare(const TUidAndIndex *pstLeft, const TUidAndIndex *pstRight)
{
    if (pstLeft->m_ullUid > pstRight->m_ullUid)
    {
        return 1;
    }
    else if (pstLeft->m_ullUid < pstRight->m_ullUid)
    {
        return -1;
    }

    return 0;
}


int UidHash(const TUidAndIndex *pstKey)
{
    return (int) (pstKey->m_ullUid % WG_INT_MAX32);
}

int Uid2Compare(const TUid2Uid *pstLeft, const TUid2Uid *pstRight)
{
    if (pstLeft->m_ullUid > pstRight->m_ullUid)
    {
        return 1;
    }
    else if (pstLeft->m_ullUid < pstRight->m_ullUid)
    {
        return -1;
    }

    return 0;
}


int Uid2Hash(const TUid2Uid *pstKey)
{
    return (int) (pstKey->m_ullUid % WG_INT_MAX32);
}


int StrCompare(const TStrAndID *pstLeft, const TStrAndID *pstRight)
{
    int iLeftLen = strlen(pstLeft->m_szName);
    int iRightLen = strlen(pstRight->m_szName);

    if (iLeftLen > iRightLen)
    {
        return 1;
    }
    else if (iLeftLen < iRightLen)
    {
        return -1;
    }
    else
    {
        return memcmp((const void *) pstLeft->m_szName, (const void *) pstRight->m_szName, iLeftLen);
    }

    return 0;
}

int StrHash(const TStrAndID *pstKey)
{
    int iHash = 0;
    int iPara = 0;
    int iLength = strlen(pstKey->m_szName);
    iLength = (std::min)(iLength, MAX_NAME_STR_LEN);
    const char *pcKey = (const char *) pstKey->m_szName;

    for (int i = 0; i < iLength; i++)
    {
        iHash = (iHash << 4) + pcKey[i];
        iPara = (iHash & 0xF0000000L);

        if (iPara != 0)
        {
            iHash ^= (iPara >> 24);
            iHash &= ~iPara;
        }
    }

    iHash &= 0x7FFFFFFF;

    return iHash;
}
