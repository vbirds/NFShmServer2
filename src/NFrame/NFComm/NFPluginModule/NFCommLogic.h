// -------------------------------------------------------------------------
//    @FileName         :    NFCommLogic.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCommLogic.h
//    @Description      :    通用逻辑工具类定义，提供系统中常用的逻辑处理功能
//                           包括登录令牌生成、安全验证等通用工具方法
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFMD5.h"
#include <vector>
#include <string>

/// @brief 登录令牌生成的基础字符串常量，用于增强令牌安全性
#define LOGIN_TOKEN "ShmNFrame"

/**
 * @brief 通用逻辑工具类
 * 
 * NFCommLogic 提供了系统中常用的逻辑处理功能，主要包括：
 * 
 * 1. 安全令牌生成：
 *    - 登录令牌的生成和验证
 *    - 基于时间和用户信息的安全哈希
 * 
 * 2. 通用算法支持：
 *    - MD5哈希计算
 *    - 字符串处理和验证
 * 
 * 设计特点：
 * - 静态工具类，无需实例化
 * - 线程安全的操作方法
 * - 高效的算法实现
 * - 易于扩展的架构
 * 
 * 使用场景：
 * - 用户登录验证
 * - API接口安全校验
 * - 数据完整性验证
 * - 通用哈希计算
 */
class NFCommLogic
{
public:
    /**
     * @brief 默认构造函数
     */
    NFCommLogic() { }
    
    /**
     * @brief 虚析构函数
     * 
     * 确保派生类能够正确析构。
     */
    virtual ~NFCommLogic() { }
public:
    /**
     * @brief 生成登录验证令牌
     * @param account 用户账号
     * @param userId 用户唯一ID
     * @param time 时间戳，通常为当前时间
     * @param specialStr 特殊字符串，用于增强安全性
     * @return 返回生成的登录令牌字符串
     * 
     * 基于用户信息、时间戳和特殊字符串生成安全的登录令牌。
     * 令牌采用MD5哈希算法生成，确保唯一性和安全性。
     * 
     * 算法原理：
     * - 将所有参数按特定顺序拼接
     * - 添加系统预定义的安全字符串
     * - 使用MD5算法生成最终令牌
     * 
     * 使用示例：
     * @code
     * std::string account = "player123";
     * uint64_t userId = 12345;
     * uint64_t currentTime = GetCurrentTimestamp();
     * std::string specialKey = "game_server_key";
     * 
     * std::string token = NFCommLogic::GetLoginToken(account, userId, currentTime, specialKey);
     * // 将token发送给客户端，用于后续验证
     * @endcode
     * 
     * @note 生成的令牌具有时效性，建议配合时间验证使用
     * @note 特殊字符串应该在服务器和客户端之间保密
     */
    static std::string GetLoginToken(const std::string& account, uint64_t userId, uint64_t time, const std::string& specialStr);
};