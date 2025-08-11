// -------------------------------------------------------------------------
//    @FileName         :    NFServerDefine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//    @Desc             :    服务器通用定义头文件，提供服务器相关的通用功能和定义。
//                          该文件定义了服务器通用类，包括服务器信息写入功能、
//                          系统信息处理功能、服务器配置管理。
//                          主要功能包括提供服务器信息的统一处理、支持系统信息收集、
//                          支持服务器配置管理、提供服务器状态报告。
//                          服务器通用定义是NFShmXFrame框架的服务器基础组件，负责：
//                          - 服务器信息的统一处理
//                          - 系统信息的收集和报告
//                          - 服务器配置的管理
//                          - 服务器状态的监控和报告
//                          - 跨平台服务器支持
//                          - 服务器信息格式化
//
// -------------------------------------------------------------------------
#pragma once

#include <stdint.h>
#include <functional>
#include <string>
#include <vector>

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFKernelMessage/FrameMsg.pb.h"
#include "NFComm/NFPluginModule/NFConfigDefine.h"
#include "NFComm/NFPluginModule/NFSystemInfo.h"

/**
 * @brief 服务器通用类
 * 
 * 该类提供了服务器相关的通用功能，包括：
 * - 服务器信息写入和处理
 * - 系统信息收集和格式化
 * - 服务器配置管理
 * - 服务器状态报告
 * 
 * 主要功能：
 * - 将服务器配置信息写入到报告对象
 * - 将系统信息写入到报告对象
 * - 提供统一的服务器信息处理接口
 * - 支持跨平台服务器信息收集
 * 
 * 使用方式：
 * @code
 * // 写入服务器配置信息
 * NFrame::ServerInfoReport report;
 * NFServerCommon::WriteServerInfo(&report, pConfig);
 * 
 * // 写入系统信息
 * NFSystemInfo sysInfo;
 * NFServerCommon::WriteServerInfo(&report, sysInfo);
 * @endcode
 */
class NFServerCommon
{
public:
    /**
     * @brief 构造函数
     */
    NFServerCommon()
    {
    }

    /**
     * @brief 析构函数
     */
    virtual ~NFServerCommon()
    {
    };

    /**
     * @brief 写入服务器信息（配置版本）
     * 
     * 将服务器配置信息写入到报告对象中，包括：
     * - 服务器类型和ID
     * - 服务器地址和端口
     * - 服务器状态和负载
     * - 服务器配置参数
     * 
     * @param pData 服务器信息报告对象
     * @param pConfig 服务器配置对象
     */
    static void WriteServerInfo(NFrame::ServerInfoReport* pData, NFServerConfig* pConfig);

    /**
     * @brief 写入服务器信息（系统信息版本）
     * 
     * 将系统信息写入到报告对象中，包括：
     * - CPU使用率和内存使用率
     * - 网络连接数和处理性能
     * - 系统运行时间和负载信息
     * - 硬件资源使用情况
     * 
     * @param pData 服务器信息报告对象
     * @param systemInfo 系统信息对象
     */
    static void WriteServerInfo(NFrame::ServerInfoReport* pData, const NFSystemInfo& systemInfo);
};
