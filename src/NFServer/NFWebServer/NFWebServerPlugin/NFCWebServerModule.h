// -------------------------------------------------------------------------
//    @FileName         :    NFCWebServerModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCWebServerModule
//    @Desc             :    NFShmXFrame Web服务器模块接口定义
//                          提供HTTP服务和Web管理功能
//                          支持HTTP请求处理、Web界面管理和API服务
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFServerDefine.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFCore/NFCommMap.hpp"
#include "NFServerComm/NFServerCommon/NFIWebServerModule.h"

/**
 * @brief Web服务器模块实现类
 *
 * NFCWebServerModule是NFIWebServerModule接口的具体实现，
 * 负责处理HTTP服务和Web管理相关的功能和业务逻辑。
 *
 * 该模块提供以下主要功能：
 * - HTTP请求处理
 * - Web界面管理
 * - API服务提供
 * - 静态资源服务
 * - Web安全控制
 * - Web事件处理
 *
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCWebServerModule : public NFIWebServerModule
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化Web服务器模块，设置插件管理器
     *
     * @param p 插件管理器指针
     */
    explicit NFCWebServerModule(NFIPluginManager* p);

    /**
     * @brief 析构函数
     *
     * 清理Web服务器模块资源
     */
    virtual ~NFCWebServerModule();

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
     * 初始化Web服务器模块，包括连接管理等
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