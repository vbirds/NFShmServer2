// -------------------------------------------------------------------------
//    @FileName         :    NFCMysqlDriverManager.cpp
//    @Author           :    Chuanbo.Guo
//    @Date             :   2022-09-18
//    @Module           :    NFCMysqlDriverManager
//    @Description      :    MySQL驱动管理器实现文件，提供MySQL连接池管理功能
//
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#include <WinSock2.h>
#include <winsock.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

#include "NFCMysqlDriver.h"
#include "NFCMysqlDriverManager.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @brief NFCMysqlDriverManager构造函数
 * 初始化最后检查时间为0
 */
NFCMysqlDriverManager::NFCMysqlDriverManager(): mnLastCheckTime(0)
{
}

/**
 * @brief NFCMysqlDriverManager析构函数
 * 释放所有MySQL驱动实例并清空映射表
 */
NFCMysqlDriverManager::~NFCMysqlDriverManager()
{
	for (NFCMysqlDriver* pMysqlDriver = mvMysql.First(); nullptr != pMysqlDriver; pMysqlDriver = mvMysql.Next())
	{
		NF_SAFE_DELETE(pMysqlDriver);
		pMysqlDriver = nullptr;
	}
	mvMysql.ClearAll();
	for (NFCMysqlDriver* pMysqlDriver = mvInvalidMsyql.First(); nullptr != pMysqlDriver; pMysqlDriver = mvInvalidMsyql.
	     Next())
	{
		NF_SAFE_DELETE(pMysqlDriver);
		pMysqlDriver = nullptr;
	}
	mvInvalidMsyql.ClearAll();
}

/**
 * @brief 获取指定的MySQL驱动
 * @param serverID 服务器唯一标识符
 * @return 返回对应的MySQL驱动指针，如果不存在则返回nullptr
 */
NFCMysqlDriver* NFCMysqlDriverManager::GetMysqlDriver(const std::string& serverID)
{
	return mvMysql.GetElement(serverID);
}

/**
 * @brief 关闭指定的MySQL连接
 * @param serverID 服务器唯一标识符
 * @return 返回0表示关闭成功，非0表示失败
 */
int NFCMysqlDriverManager::CloseMysql(const std::string& serverID)
{
    NFCMysqlDriver* pMysqlDriver = mvMysql.GetElement(serverID);
    if (pMysqlDriver == NULL)
    {
        pMysqlDriver = mvInvalidMsyql.GetElement(serverID);
    }
    CHECK_EXPR(pMysqlDriver , -1, "nServerID:{} not exist", serverID);

    pMysqlDriver->Disconnect();
    NF_SAFE_DELETE(pMysqlDriver);
    mvMysql.RemoveElement(serverID);
    mvInvalidMsyql.RemoveElement(serverID);
    return 0;
}

void NFCMysqlDriverManager::CheckMysql()
{
    if (NFGetSecondTime() - mnLastCheckTime <= 10) return;
    mnLastCheckTime = NFGetSecondTime();

	std::string nServerID;
	std::vector<std::string> xIntVec;
	for (NFCMysqlDriver* pMysqlDriver = mvMysql.First(nServerID); pMysqlDriver != nullptr; pMysqlDriver = mvMysql.
	     Next(nServerID))
	{
		if (!pMysqlDriver->Enable())
		{
			xIntVec.push_back(nServerID);
			mvInvalidMsyql.AddElement(nServerID, pMysqlDriver);
		}
	}

	for (int i = 0; i < (int)xIntVec.size(); ++i)
	{
		mvMysql.RemoveElement(xIntVec[i]);
	}
	//////////////////////////////////////////////////////////////////////////
	xIntVec.clear();
	nServerID.clear();

	for (NFCMysqlDriver* pMysqlDriver = mvInvalidMsyql.First(nServerID); pMysqlDriver != nullptr; pMysqlDriver =
	     mvInvalidMsyql.Next(nServerID))
	{
		//if (!pMysqlDriver->Enable() && pMysqlDriver->CanReconnect())
		if (!pMysqlDriver->Enable())
		{
			pMysqlDriver->Reconnect();
			if (pMysqlDriver->Enable())
			{
				xIntVec.push_back(nServerID);
				mvMysql.AddElement(nServerID, pMysqlDriver);
			}
		}
	}

	for (int i = 0; i < (int)xIntVec.size(); ++i)
	{
		mvInvalidMsyql.RemoveElement(xIntVec[i]);
	}
}

int NFCMysqlDriverManager::AddMysqlServer(const std::string& serverID, const std::string& strIP, const int nPort,
                                           const std::string strDBName, const std::string strDBUser,
                                           const std::string strDBPwd, const int nRconnectTime/* = 10*/,
                                           const int nRconneCount/* = -1*/)
{
    NFCMysqlDriver* pMysqlDriver = mvMysql.GetElement(serverID);
	CHECK_EXPR(pMysqlDriver == NULL, 0, "pMysqlDriver == NULL, nServerID:{} exist", serverID);

    NFCMysqlDriver* pInvalidMysqlDriver = mvInvalidMsyql.GetElement(serverID);
	CHECK_EXPR(pInvalidMysqlDriver == NULL, -1, "pInvalidRedisDriver == NULL, nServerID:{} exist", serverID);


	pMysqlDriver = NF_NEW NFCMysqlDriver(nRconnectTime, nRconneCount);
	int iRet = pMysqlDriver->Connect(strDBName, strIP, nPort, strDBUser, strDBPwd);
    if (iRet == 0) {
        mvMysql.AddElement(serverID, pMysqlDriver);
    } else {
        mvInvalidMsyql.AddElement(serverID, pMysqlDriver);
        NFLogError(NF_LOG_DEFAULT, 0, "Connect Mysql Failed!");
        return -1;
    }

    NFLogInfo(NF_LOG_DEFAULT, 0,
              "Connecy Mysql Success:nServerID:{}, strIP:{}, nPort:{}, strDBName:{}, strDBUser:{}, strDBPwd:{}",
			serverID, strIP, nPort, strDBName, strDBUser, strDBPwd);

    return 0;
}
