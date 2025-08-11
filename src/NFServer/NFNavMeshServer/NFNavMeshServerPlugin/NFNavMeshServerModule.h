// -------------------------------------------------------------------------
//    @FileName         :    NFNavMeshServerModule.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFNavMeshServerModule
//    @Desc             :    NFShmXFrame导航网格服务器模块接口定义
//                          提供路径查找和导航计算功能
//                          支持A*算法、寻路优化和导航网格管理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFServerComm/NFServerCommon/NFINavMeshServerModule.h"

/**
 * @brief 导航网格服务器模块实现类
 *
 * NFNavMeshServerModule是NFINavMeshServerModule接口的具体实现，
 * 负责处理路径查找和导航计算相关的功能和业务逻辑。
 *
 * 该模块提供以下主要功能：
 * - 路径查找和导航计算
 * - A*算法实现
 * - 导航网格管理
 * - 寻路优化
 * - 动态障碍物处理
 * - 导航事件处理
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFNavMeshServerModule : public NFINavMeshServerModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化导航网格服务器模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFNavMeshServerModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     *
     * 清理导航网格服务器模块资源
     */
    virtual ~NFNavMeshServerModule();

    /**
     * @brief 模块唤醒
     *
     * 在模块初始化完成后调用，进行必要的初始化工作
     *
     * @return 初始化结果状态码
     */
    virtual int Awake() override;

    /**
     * @brief 模块初始化
     *
     * 初始化导航网格服务器模块，包括连接管理等
     *
     * @return 初始化结果状态码
     */
    virtual int Init() override;

    /**
     * @brief 模块定时更新
     *
     * 每帧调用，处理模块的定时任务
     *
     * @return 处理结果状态码
     */
    virtual int Tick() override;

    /**
     * @brief 动态插件处理
     *
     * 处理动态插件的加载和卸载
     *
     * @return 处理结果状态码
     */
    virtual int OnDynamicPlugin() override;

    /**
     * @brief 处理来自服务器的信息
     *
     * 处理来自其他服务器的消息请求
     *
     * @param unLinkId 连接ID
     * @param packet 数据包
     * @return 处理结果状态码
     */
    virtual int OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet) override;
};