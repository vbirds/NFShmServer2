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
//    @FileName         :    NFRedisCommand.h
//    @Author           :    lvsheng.huang
//    @Date             :   2022-09-18
//    @Module           :    NFRedisCommand
//    @Desc             :    Redis命令构建器头文件，提供Redis命令构建和序列化功能。
//                          该文件定义了NFShmXFrame框架的Redis命令构建器类，用于构建和序列化Redis命令，
//                          支持流式的参数添加方式，负责将Redis命令及其参数组装成符合Redis协议格式的字符串。
//                          主要功能包括构建Redis命令支持命令名称和参数的添加、流式接口使用operator<<提供链式调用、
//                          协议序列化将命令转换为Redis RESP协议格式、类型安全支持多种数据类型的参数
//    @Description      :    Redis命令构建器头文件，提供Redis命令构建和序列化功能
//
// -------------------------------------------------------------------------

#ifndef NFREDISPLUGIN_NFREDISCOMMAND_H
#define NFREDISPLUGIN_NFREDISCOMMAND_H

#include <iostream>
#include <vector>
#include <sstream>

#include "NFRedisProtocolDefine.h"

/**
 * @class NFRedisCommand
 * @brief Redis命令构建器类
 * 
 * 用于构建和序列化Redis命令，支持流式的参数添加方式。
 * 该类负责将Redis命令及其参数组装成符合Redis协议格式的字符串。
 * 
 * 主要功能：
 * - 构建Redis命令：支持命令名称和参数的添加
 * - 流式接口：使用operator<<提供链式调用
 * - 协议序列化：将命令转换为Redis RESP协议格式
 * - 类型安全：支持多种数据类型的参数
 * 
 * 使用示例：
 * @code
 * NFRedisCommand cmd("SET");
 * cmd << "key" << "value";
 * std::string serialized = cmd.Serialize();
 * @endcode
 */
class NFRedisCommand
{
public:
    /**
     * @brief 构造函数
     * @param cmd Redis命令名称（如"GET", "SET", "HGET"等）
     * 
     * 初始化Redis命令构建器，设置要执行的Redis命令名称
     */
    NFRedisCommand( const std::string& cmd )
    {
        mxParam.push_back( cmd );
    }

    /**
     * @brief 析构函数
     * 
     * 释放命令构建器占用的资源
     */
    ~NFRedisCommand()
    {

    }

    /**
     * @brief 重载流操作符，用于添加命令参数
     * @tparam T 参数类型，支持任何可以转换为字符串的类型
     * @param t 要添加的参数值
     * @return 返回当前命令构建器的引用，支持链式调用
     * 
     * 该方法支持流式接口，可以连续添加多个参数：
     * cmd << param1 << param2 << param3;
     */
    template <typename T>
    NFRedisCommand& operator<<( const T& t )
    {
        std::stringstream str;
        str << t;
        mxParam.push_back(str.str());
        return *this;
    }

    /**
     * @brief 序列化Redis命令为RESP协议格式
     * @return 返回序列化后的Redis命令字符串
     * 
     * 将构建的Redis命令及其参数转换为Redis服务器可以理解的RESP协议格式。
     * RESP协议格式示例：
     * *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
     * 
     * 格式说明：
     * - *N 表示数组有N个元素
     * - $M 表示字符串长度为M
     * - \r\n 是CRLF分隔符
     */
    std::string Serialize() const
    {
        std::stringstream xDataString;
        xDataString << '*' << mxParam.size() << NFREDIS_CRLF;
        std::vector<std::string>::const_iterator it = mxParam.begin();
        for ( ; it != mxParam.end(); ++it )
        {
            xDataString << '$' << it->size() << NFREDIS_CRLF;
            xDataString << *it << NFREDIS_CRLF;
        }

        return xDataString.str();
    }

private:
    /**
     * @brief 存储Redis命令参数的容器
     * 
     * 第一个元素是命令名称，后续元素是命令参数
     */
    std::vector<std::string> mxParam;
};


#endif //NFREDISPLUGIN_NFREDISCOMMAND_H
