// -------------------------------------------------------------------------
//    @FileName         :    NFServerCommonDefine.cpp
//    @Author           :    gaoyi
//    @Date             :    2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFServerCommonDefine
//    @Desc             :    服务器通用定义实现文件，提供服务器信息写入功能实现。
//                          该文件实现了服务器通用类的方法，包括服务器配置信息写入、
//                          系统信息写入、服务器状态格式化。
//                          主要功能包括提供服务器信息的统一处理实现、支持系统信息收集、
//                          支持服务器配置管理、提供服务器状态报告。
//                          服务器通用定义实现是NFShmXFrame框架的服务器基础组件实现，负责：
//                          - 服务器信息的统一处理实现
//                          - 系统信息的收集和报告实现
//                          - 服务器配置的管理实现
//                          - 服务器状态的监控和报告实现
//                          - 跨平台服务器支持实现
//                          - 服务器信息格式化实现
//
// -------------------------------------------------------------------------

#include "NFServerCommonDefine.h"
#include "NFComm/NFPluginModule/NFSystemInfo.h"

/**
 * @brief 写入服务器信息（配置版本）
 * 
 * 将服务器配置信息写入到报告对象中，包括：
 * - 服务器类型和ID
 * - 服务器地址和端口
 * - 服务器状态和负载
 * - 服务器配置参数
 * - 数据库配置信息（如果是存储服务器）
 * 
 * @param pData 服务器信息报告对象
 * @param pConfig 服务器配置对象
 */
void NFServerCommon::WriteServerInfo(NFrame::ServerInfoReport* pData, NFServerConfig* pConfig)
{
    // 设置基本服务器信息
    pData->set_bus_id(pConfig->BusId);
    pData->set_server_id(pConfig->ServerId);
    pData->set_server_type(pConfig->ServerType);
    pData->set_server_name(pConfig->ServerName);
    pData->set_is_cross_server(pConfig->CrossServer);
    pData->set_bus_length(pConfig->BusLength);
    pData->set_link_mode(pConfig->LinkMode);
    pData->set_url(pConfig->Url);
    pData->set_server_ip(pConfig->ServerIp);
    pData->set_server_port(pConfig->ServerPort);
    pData->set_route_svr(pConfig->RouteConfig.RouteAgent);
    pData->set_external_server_ip(pConfig->ExternalServerIp);
    pData->set_external_server_port(pConfig->ExternalServerPort);
    
    // 如果是存储服务器，添加数据库名称列表
    if (pConfig->ServerType == NF_ST_STORE_SERVER)
    {
        pData->add_db_name_list(pConfig->MysqlConfig.MysqlDbName);
    }
}

/**
 * @brief 写入服务器信息（系统信息版本）
 * 
 * 将系统信息写入到报告对象中，包括：
 * - CPU使用率和内存使用率
 * - 网络连接数和处理性能
 * - 系统运行时间和负载信息
 * - 硬件资源使用情况
 * - 进程信息和在线用户数
 * 
 * @param pData 服务器信息报告对象
 * @param systemInfo 系统信息对象
 */
void NFServerCommon::WriteServerInfo(NFrame::ServerInfoReport* pData, const NFSystemInfo& systemInfo)
{
    // 设置系统信息
    pData->set_system_info(systemInfo.GetOsInfo().mOsDescription);
    pData->set_total_mem(systemInfo.GetMemInfo().mTotalMem);
    pData->set_free_mem(systemInfo.GetMemInfo().mFreeMem);
    pData->set_used_mem(systemInfo.GetMemInfo().mUsedMem);

    // 设置进程信息
    pData->set_proc_cpu(systemInfo.GetProcessInfo().mCpuUsed);
    pData->set_proc_mem(systemInfo.GetProcessInfo().mMemUsed);
    pData->set_proc_thread(systemInfo.GetProcessInfo().mThreads);
    pData->set_proc_name(systemInfo.GetProcessInfo().mName);
    pData->set_proc_cwd(systemInfo.GetProcessInfo().mCwd);
    pData->set_proc_pid(systemInfo.GetProcessInfo().mPid);
    pData->set_server_cur_online(systemInfo.GetUserCount());
}

