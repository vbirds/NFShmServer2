// -------------------------------------------------------------------------
//    @FileName         :    NFCNosqlDriverManager.cpp
//    @Author           :    Yi.Gao
//    @Date             :   2022-09-18
//    @Module           :    NFCNosqlDriverManager
//    @Description      :    NoSQL驱动管理器实现文件
//
// -------------------------------------------------------------------------

#include <algorithm>
#include "NFCNosqlDriverManager.h"
#include "NFComm/NFCore/NFTime.h"

/**
 * @brief NFCNosqlDriverManager构造函数
 * 初始化最后检查时间为0
 */
NFCNosqlDriverManager::NFCNosqlDriverManager()
{
    mLastCheckTime = 0;
}

/**
 * @brief NFCNosqlDriverManager析构函数
 * 释放所有NoSQL驱动实例并清空映射表
 */
NFCNosqlDriverManager::~NFCNosqlDriverManager()
{
    for (auto pMysqlDriver = mxNoSqlDriver.First(); nullptr != pMysqlDriver; pMysqlDriver = mxNoSqlDriver.Next())
    {
        NF_SAFE_DELETE(pMysqlDriver);
        pMysqlDriver = nullptr;
    }
    mxNoSqlDriver.ClearAll();
}

/**
 * @brief 检查连接是否可用
 * @return 返回false（占位符实现）
 * 
 * 当前为占位符实现，预留给子类扩展
 */
bool NFCNosqlDriverManager::Enable()
{
    return false;
}

/**
 * @brief 检查是否繁忙
 * @return 返回false（占位符实现）
 * 
 * 当前为占位符实现，预留给子类扩展
 */
bool NFCNosqlDriverManager::Busy()
{
    return false;
}

/**
 * @brief 保持连接活跃
 * @return 返回false（占位符实现）
 * 
 * 当前为占位符实现，预留给子类扩展
 */
bool NFCNosqlDriverManager::KeepLive()
{
    return false;
}

/**
 * @brief 主循环处理函数
 * @return 返回0表示处理成功
 * 
 * 执行以下操作：
 * 1. 遍历所有NoSQL驱动，调用其Execute方法
 * 2. 执行连接状态检查和维护
 */
int NFCNosqlDriverManager::Tick()
{
    auto xNosqlDriver = this->mxNoSqlDriver.First();
    while (xNosqlDriver)
    {
        xNosqlDriver->Execute();

        xNosqlDriver = this->mxNoSqlDriver.Next();
    }

    CheckNoSql();

    return 0;
}

/**
 * @brief 添加NoSQL服务器（使用默认端口6379）
 * @param strID 服务器唯一标识符
 * @param strIP 服务器IP地址
 * @return 返回0表示添加成功，-1表示失败
 * 
 * 创建新的Redis驱动实例并尝试连接到指定服务器
 * 如果服务器ID已存在，则返回失败
 */
int NFCNosqlDriverManager::AddNosqlServer(const std::string& strID, const std::string& strIP)
{
    if (!mxNoSqlDriver.GetElement(strID))
    {
        NFRedisDriver* pNoSqlDriver = new NFRedisDriver();
        mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
        if (pNoSqlDriver->Connect(strIP, 6379, ""))
        {
            return 0;
        }

        return -1;
    }

    return -1;
}

/**
 * @brief 添加NoSQL服务器（指定端口）
 * @param strID 服务器唯一标识符
 * @param strIP 服务器IP地址
 * @param nPort 服务器端口号
 * @return 返回0表示添加成功，-1表示失败
 * 
 * 创建新的Redis驱动实例并尝试连接到指定服务器和端口
 * 如果服务器ID已存在，则返回失败
 */
int NFCNosqlDriverManager::AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort)
{
    if (!mxNoSqlDriver.GetElement(strID))
    {
        auto pNoSqlDriver = NF_NEW NFRedisDriver();
        mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
        if (pNoSqlDriver->Connect(strIP, nPort, ""))
        {
            return 0;
        }
        return -1;
    }

    return -1;
}

/**
 * @brief 添加NoSQL服务器（完整参数）
 * @param strID 服务器唯一标识符
 * @param strIP 服务器IP地址
 * @param nPort 服务器端口号
 * @param strPass 服务器连接密码
 * @return 返回0表示添加成功，-1表示失败
 * 
 * 创建新的Redis驱动实例并尝试连接到指定服务器，支持密码认证
 * 如果服务器ID已存在，则返回失败
 */
int NFCNosqlDriverManager::AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort, const std::string& strPass)
{
    if (!mxNoSqlDriver.GetElement(strID))
    {
        auto pNoSqlDriver = NF_NEW NFRedisDriver();
        mxNoSqlDriver.AddElement(strID, pNoSqlDriver);
        if (pNoSqlDriver->Connect(strIP, nPort, strPass))
        {
            return 0;
        }
        return -1;
    }

    return -1;
}

/**
 * @brief 获取指定的NoSQL驱动
 * @param strID 服务器唯一标识符
 * @return 返回可用的NoSQL驱动指针，如果不存在或不可用则返回nullptr
 * 
 * 只返回状态为Enable的驱动实例，确保调用者获得的是可用的连接
 */
NFINosqlDriver* NFCNosqlDriverManager::GetNosqlDriver(const std::string& strID)
{
    auto xDriver = mxNoSqlDriver.GetElement(strID);
    if (xDriver && xDriver->Enable())
    {
        return xDriver;
    }

    return nullptr;
}

/**
 * @brief 定期检查NoSQL连接状态
 * 
 * 每15秒执行一次检查，处理以下情况：
 * 1. 连接正常但未认证：调用AUTH命令进行认证
 * 2. 连接断开：尝试重新连接
 * 
 * 这是一个重要的维护方法，确保所有连接的稳定性和可用性
 */
void NFCNosqlDriverManager::CheckNoSql()
{
    static const int CHECK_TIME = 15;
    if (mLastCheckTime + CHECK_TIME > NF_ADJUST_TIMENOW())
    {
        return;
    }

    mLastCheckTime = NF_ADJUST_TIMENOW();

    auto xNosqlDriver = this->mxNoSqlDriver.First();
    while (xNosqlDriver)
    {
        if (xNosqlDriver->Enable() && !xNosqlDriver->Authed())
        {
            xNosqlDriver->AUTH(xNosqlDriver->GetAuthKey());
        }
        else if (!xNosqlDriver->Enable())
        {
            //reconnect
            xNosqlDriver->ReConnect();
        }

        xNosqlDriver = this->mxNoSqlDriver.Next();
    }
}

