// -------------------------------------------------------------------------
//    @FileName         :    NFCNosqlModule.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFCNosqlModule
//    @Description      :    NoSQL数据库模块实现文件
//
// -------------------------------------------------------------------------

#include <algorithm>
#include "NFCNosqlModule.h"
#include "NFComm/NFCore/NFTime.h"
#include "NFComm/NFPluginModule/NFCheck.h"

/**
 * @brief NFCNosqlModule构造函数
 * @param p 插件管理器指针
 * 
 * 初始化NoSQL模块，创建NoSQL驱动管理器实例
 */
NFCNosqlModule::NFCNosqlModule(NFIPluginManager* p): NFINosqlModule(p)
{
    m_pNoSqlDriverManager = NF_NEW NFCNosqlDriverManager();
}

/**
 * @brief NFCNosqlModule析构函数
 * 
 * 释放NoSQL模块占用的资源
 */
NFCNosqlModule::~NFCNosqlModule()
{

}

/**
 * @brief 初始化NoSQL模块
 * @return 返回0表示初始化成功
 * 
 * 目前为空实现，预留给子类扩展初始化逻辑
 */
int NFCNosqlModule::Init()
{
	return 0;
}

/**
 * @brief 关闭NoSQL模块
 * @return 返回0表示关闭成功
 * 
 * 目前为空实现，预留给子类扩展关闭逻辑
 */
int NFCNosqlModule::Shut()
{
	return 0;
}

/**
 * @brief 检查NoSQL连接是否可用
 * @return 返回true表示连接可用，false表示不可用
 * 
 * 委托给驱动管理器进行连接状态检查
 */
bool NFCNosqlModule::Enable()
{
	return m_pNoSqlDriverManager->Enable();
}

/**
 * @brief 检查NoSQL是否繁忙
 * @return 返回true表示繁忙，false表示空闲
 * 
 * 委托给驱动管理器进行繁忙状态检查
 */
bool NFCNosqlModule::Busy()
{
	return m_pNoSqlDriverManager->Busy();
}

/**
 * @brief 保持NoSQL连接活跃
 * @return 返回true表示保活成功，false表示失败
 * 
 * 委托给驱动管理器进行连接保活操作
 */
bool NFCNosqlModule::KeepLive()
{
	return m_pNoSqlDriverManager->KeepLive();
}

/**
 * @brief 模块主循环处理
 * @return 返回0表示处理成功，非0表示处理失败
 * 
 * 委托给驱动管理器进行定期维护操作，如连接检查、异步操作处理等
 */
int NFCNosqlModule::Tick()
{
	return m_pNoSqlDriverManager->Tick();
}

/**
 * @brief 添加NoSQL服务器（仅指定ID和IP）
 * @param strID 服务器唯一标识符
 * @param strIP 服务器IP地址
 * @return 返回0表示添加成功，非0表示失败
 * 
 * 使用默认端口添加NoSQL服务器连接
 */
int NFCNosqlModule::AddNosqlServer(const std::string& strID, const std::string& strIP)
{
	return m_pNoSqlDriverManager->AddNosqlServer(strID, strIP);
}

/**
 * @brief 添加NoSQL服务器（指定ID、IP和端口）
 * @param strID 服务器唯一标识符
 * @param strIP 服务器IP地址
 * @param nPort 服务器端口号
 * @return 返回0表示添加成功，非0表示失败
 * 
 * 使用指定端口添加NoSQL服务器连接
 */
int NFCNosqlModule::AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort)
{
	return m_pNoSqlDriverManager->AddNosqlServer(strID, strIP, nPort);
}

/**
 * @brief 添加NoSQL服务器（指定ID、IP、端口和密码）
 * @param strID 服务器唯一标识符
 * @param strIP 服务器IP地址
 * @param nPort 服务器端口号
 * @param strPass 服务器连接密码
 * @return 返回0表示添加成功，非0表示失败
 * 
 * 使用完整参数添加NoSQL服务器连接
 */
int NFCNosqlModule::AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort, const std::string& strPass)
{
	return m_pNoSqlDriverManager->AddNosqlServer(strID, strIP, nPort, strPass);
}

/**
 * @brief 选择对象数据
 * @param strID 服务器ID
 * @param select 选择条件
 * @param select_res 选择结果
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象选择操作
 */
int NFCNosqlModule::SelectObj(const std::string& strID, const NFrame::storesvr_selobj &select, NFrame::storesvr_selobj_res &select_res)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->SelectObj(select, select_res);
}

/**
 * @brief 保存对象数据（使用选择结构）
 * @param strID 服务器ID
 * @param select 保存的数据结构
 * @param select_res 保存结果
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象保存操作
 */
int NFCNosqlModule::SaveObj(const std::string& strID, const NFrame::storesvr_selobj &select,
                            NFrame::storesvr_selobj_res &select_res)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->SaveObj(select, select_res);
}

/**
 * @brief 保存对象数据（使用插入结构）
 * @param strID 服务器ID
 * @param select 插入的数据结构
 * @param select_res 插入结果
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象插入操作
 */
int NFCNosqlModule::SaveObj(const std::string& strID, const NFrame::storesvr_insertobj &select, NFrame::storesvr_insertobj_res &select_res)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->SaveObj(select, select_res);
}

/**
 * @brief 保存对象数据（使用修改结构）
 * @param strID 服务器ID
 * @param select 修改的数据结构
 * @param select_res 修改结果
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象修改操作
 */
int NFCNosqlModule::SaveObj(const std::string& strID, const NFrame::storesvr_modobj &select, NFrame::storesvr_modobj_res &select_res)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->SaveObj(select, select_res);
}

/**
 * @brief 保存对象数据（使用更新结构）
 * @param strID 服务器ID
 * @param select 更新的数据结构
 * @param select_res 更新结果
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象更新操作
 */
int NFCNosqlModule::SaveObj(const std::string& strID, const NFrame::storesvr_updateobj &select, NFrame::storesvr_updateobj_res &select_res)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->SaveObj(select, select_res);
}

/**
 * @brief 删除对象数据（使用删除结构）
 * @param strID 服务器ID
 * @param select 删除条件
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象删除操作
 */
int NFCNosqlModule::DeleteObj(const std::string& strID, const NFrame::storesvr_delobj &select)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->DeleteObj(select);
}

/**
 * @brief 删除对象数据（使用插入结构作为删除条件）
 * @param strID 服务器ID
 * @param select 删除条件（复用插入结构）
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象删除操作
 */
int NFCNosqlModule::DeleteObj(const std::string& strID, const NFrame::storesvr_insertobj &select)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->DeleteObj(select);
}

/**
 * @brief 删除对象数据（使用修改结构作为删除条件）
 * @param strID 服务器ID
 * @param select 删除条件（复用修改结构）
 * @return 返回0表示操作成功，-1表示失败
 * 
 * 根据服务器ID获取对应的驱动，然后执行对象删除操作
 */
int NFCNosqlModule::DeleteObj(const std::string& strID, const NFrame::storesvr_modobj &select)
{
    auto pDriver = m_pNoSqlDriverManager->GetNosqlDriver(strID);
    CHECK_EXPR(pDriver, -1, "pDriver == NULL, nServerID:{}", strID);
    return pDriver->DeleteObj(select);
}

/**
 * @brief 获取指定服务器的NoSQL驱动
 * @param strID 服务器唯一标识符
 * @return 返回对应的NoSQL驱动指针，如果不存在则返回nullptr
 * 
 * 委托给驱动管理器获取指定服务器的驱动实例
 */
NFINosqlDriver* NFCNosqlModule::GetNosqlDriver(const std::string& strID)
{
	return m_pNoSqlDriverManager->GetNosqlDriver(strID);
}

