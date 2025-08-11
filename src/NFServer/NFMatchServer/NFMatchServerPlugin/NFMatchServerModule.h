// -------------------------------------------------------------------------
//    @FileName         :    NFMatchServerModule.h
//    @Author           :    gaoyi
//    @Date             :    2024/12/18
//    @Email            :    445267987@qq.com
//    @Module           :    NFMatchServerModule
//    @Desc             :    NFShmXFrame匹配服务器模块接口定义
//                          提供玩家匹配和队列管理功能
//                          支持匹配算法、队列管理和匹配结果处理
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFServerComm/NFServerCommon/NFIMatchServerModule.h"

/**
 * @brief 匹配服务器模块实现类
 *
 * NFMatchServerModule是NFIMatchServerModule接口的具体实现，
 * 负责玩家匹配和队列管理相关的功能和业务逻辑。
 *
 * 该模块提供以下主要功能：
 * - 玩家匹配算法
 * - 匹配队列管理
 * - 匹配结果处理
 * - 匹配规则配置
 * - 匹配状态跟踪
 * - 匹配事件处理
 *
 * @author gaoyi
 * @date 2024/12/18
 */
class NFMatchServerModule : public NFIMatchServerModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化匹配服务器模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFMatchServerModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     *
     * 清理匹配服务器模块资源
     */
    virtual ~NFMatchServerModule();

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
     * 初始化匹配服务器模块，包括连接管理等
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