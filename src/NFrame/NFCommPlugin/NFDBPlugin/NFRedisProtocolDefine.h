/*
            This file is part of: 
                NFShmXFrame
            https://github.com/ketoo/NoahGameFrame

   Copyright 2009 - 2019 NFShmXFrame(NoahGameFrame)

   File creator: lvsheng.huang
   
   NFShmXFrame is open-source software and you can redistribute it and/or modify
   it under the terms of the License; besides, anyone who use this file/software must include this copyright announcement.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// -------------------------------------------------------------------------
//    @FileName         :    NFRedisProtocolDefine.h
//    @Author           :    lvsheng.huang
//    @Date             :   2022-09-18
//    @Module           :    NFRedisProtocolDefine
//    @Desc             :    Redis协议定义头文件，定义Redis RESP协议相关的常量和枚举。
//                          该文件定义了NFShmXFrame框架的Redis协议定义，包含Redis RESP协议相关的常量和枚举，
//                          定义Redis命令执行后的结果状态、Redis服务器响应的不同数据类型、
//                          Redis RESP协议类型标识符、CRLF行结束符、Redis标准成功状态字符串等。
//                          主要功能包括Redis操作结果状态枚举、Redis RESP协议响应类型枚举、
//                          协议常量定义、状态字符串定义、行结束符定义
//    @Description      :    Redis协议定义头文件，定义Redis RESP协议相关的常量和枚举
//
// -------------------------------------------------------------------------

#ifndef NFREDISPLUGIN_NFREDISPROTOCOLDEFINE_H
#define NFREDISPLUGIN_NFREDISPROTOCOLDEFINE_H
#include <string>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

/*
Redis RESP协议类型标识符定义：
状态回复（status reply）的第一个字节是 "+"           //For Simple Strings the first byte of the reply is "+"
错误回复（error reply）的第一个字节是 "-"            //For Errors the first byte of the reply is "-"
整数回复（integer reply）的第一个字节是 ":"          //For Integers the first byte of the reply is ":"
批量回复（bulk reply）的第一个字节是 "$"             //For Bulk Strings the first byte of the reply is "$"
多条批量回复（multi bulk reply）的第一个字节是 "*"    //For Arrays the first byte of the reply is "*"
 */

/**
 * @enum NFREDIS_RESULT_STATUS
 * @brief Redis操作结果状态枚举
 * 
 * 定义Redis命令执行后的结果状态，用于判断操作是否成功
 */
enum NFREDIS_RESULT_STATUS
{
	NFREDIS_RESULT_STATUS_OK,         ///< 操作成功完成
	NFREDIS_RESULT_STATUS_UNKNOW,     ///< 未知状态或错误
	NFREDIS_RESULT_STATUS_IMCOMPLETE, ///< 操作未完成或数据不完整
};

/**
 * @enum NFREDIS_RESP_TYPE
 * @brief Redis RESP协议响应类型枚举
 * 
 * 定义Redis服务器响应的不同数据类型，基于RESP协议规范
 */
enum NFREDIS_RESP_TYPE
{
    NFREDIS_RESP_UNKNOW,  ///< 未知类型
    NFREDIS_RESP_NIL,     ///< 空值类型（NULL）
    NFREDIS_RESP_STATUS,  ///< 状态回复（Simple Strings）
    NFREDIS_RESP_ERROR,   ///< 错误回复（Errors）
    NFREDIS_RESP_INT,     ///< 整数回复（Integers）
    NFREDIS_RESP_BULK,    ///< 批量字符串（Bulk Strings）
    NFREDIS_RESP_ARRAY,   ///< 数组回复（Arrays）
};

/**
 * @brief CRLF行结束符
 * Redis RESP协议中使用"\r\n"作为行分隔符
 */
static const std::string NFREDIS_CRLF = "\r\n";

/**
 * @brief CRLF分隔符的字节长度
 * 值为2，对应"\r\n"的长度
 */
static const int NFREDIS_SIZEOF_CRLF = 2;

/**
 * @brief Redis标准成功状态字符串
 * 大多数Redis命令成功执行后返回"OK"状态
 */
static const std::string NFREDIS_STATUS_OK = "OK";

//static const char* NFREDIS_STATUS_REPLY = "+";    //状态回复（status reply）的第一个字节是 "+"
//static const char* NFREDIS_ERROR_REPLY = "-";	    //错误回复（error reply）的第一个字节是 "-"
//static const char* NFREDIS_INT_REPLY = ":";	    //整数回复（integer reply）的第一个字节是 ":"
//static const char* NFREDIS_BULK_REPLY = "$";	    //批量回复（bulk reply）的第一个字节是 "$"
//static const char* NFREDIS_ARRAY_REPLY = "*";	    //多条批量回复（multi bulk reply）的第一个字节是 "*"

/**
 * @def GET_NAME(functionName)
 * @brief 获取函数名称的宏定义
 * @param functionName 函数名
 * @return 返回函数名的字符串表示
 * 
 * 将函数名转换为字符串，主要用于调试和日志输出
 */
#define GET_NAME(functionName)   (#functionName)

#endif //NFREDISPLUGIN_NFREDISPROTOCOLDEFINE_H
