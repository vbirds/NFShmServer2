// -------------------------------------------------------------------------
//    @FileName         :    NFError.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//    @Description      :    错误处理系统定义，提供统一的错误码转换和错误信息获取功能
//                           支持自定义错误处理函数，实现灵活的错误信息管理
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFSingleton.hpp"

#include <string>
#include <functional>

/// @brief 错误处理函数类型定义
/// @param retCode 错误码
/// @return 错误描述字符串
typedef std::function<std::string(int32_t retCode)> NFErrorFunction;

/**
 * @brief 错误处理管理类
 * 
 * NFError 是一个单例类，负责管理系统中的错误码和错误信息。
 * 提供了错误码到错误描述字符串的转换功能，支持自定义错误处理函数。
 * 
 * 主要功能：
 * - 错误码到错误描述的转换
 * - 支持自定义错误处理函数
 * - 全局统一的错误信息管理
 * 
 * 使用方式：
 * @code
 * // 获取错误描述
 * std::string errorMsg = NFError::Instance()->GetErrorStr(errorCode);
 * 
 * // 设置自定义错误处理函数
 * NFError::Instance()->SetErrorFunction([](int32_t code) {
 *     return "Custom error: " + std::to_string(code);
 * });
 * @endcode
 */
class NFError : public NFSingleton<NFError>
{
public:
    /**
     * @brief 获取错误描述字符串
     * @param retCode 错误码
     * @return 返回对应的错误描述字符串
     * 
     * 根据错误码获取相应的错误描述信息。如果设置了自定义错误处理函数，
     * 则使用自定义函数进行转换；否则使用默认的错误处理逻辑。
     */
    virtual std::string GetErrorStr(int32_t retCode);

    /**
     * @brief 设置自定义错误处理函数
     * @param func 错误处理函数
     * 
     * 设置自定义的错误码转换函数，用于将错误码转换为特定格式的错误描述。
     * 这允许不同的模块或应用程序使用自己的错误信息格式。
     */
    void SetErrorFunction(const NFErrorFunction& func);
private:
    /// @brief 自定义错误处理函数，用于错误码到错误描述的转换
    NFErrorFunction m_func;
};

/**
 * @brief 全局错误描述获取函数
 * @param retCode 错误码
 * @return 返回对应的错误描述字符串
 * 
 * 全局函数，用于快速获取错误描述信息。
 * 内部调用NFError单例的GetErrorStr方法。
 */
std::string GetErrorStr(int32_t retCode);
