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
//    @FileName         :    NFRedisClientSocket.h
//    @Author           :    lvsheng.huang
//    @Date             :   2022-09-18
//    @Module           :    NFRedisClientSocket
//    @Desc             :    Redis客户端Socket头文件，负责管理与Redis服务器的网络连接。
//                          该文件定义了NFShmXFrame框架的Redis客户端Socket管理类，基于libevent和hiredis实现，
//                          负责与Redis服务器建立连接、维护连接状态、处理网络事件等。
//                          主要功能包括网络连接管理建立维护关闭Redis连接、事件驱动基于libevent的异步事件处理、
//                          数据传输支持命令发送和响应接收、状态监控实时监控连接状态和网络事件、
//                          自动重连支持断线重连机制、协议解析集成hiredis进行Redis协议解析
//    @Description      :    Redis客户端Socket头文件，负责管理与Redis服务器的网络连接
//
// -------------------------------------------------------------------------

#ifndef NFREDISPLUGIN_NFREDISCLIENTSOCKET_H
#define NFREDISPLUGIN_NFREDISCLIENTSOCKET_H

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <atomic>

#if NF_PLATFORM == NF_PLATFORM_WIN

#elif NF_PLATFORM == NF_PLATFORM_APPLE || NF_PLATFORM == NF_PLATFORM_LINUX || NF_PLATFORM == NF_PLATFORM_ANDROID

#if NF_PLATFORM == NF_PLATFORM_APPLE

#include <libkern/OSByteOrder.h>

#endif

#include <netinet/in.h>

#ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif

#include <sys/socket.h>
#include <unistd.h>

#endif

#include "libevent/include/event2/bufferevent.h"
#include "libevent/include/event2/buffer.h"
#include "libevent/include/event2/listener.h"
#include "libevent/include/event2/util.h"
#include "libevent/include/event2/thread.h"
#include "libevent/include/event2/event_compat.h"
#include "libevent/include/event2/bufferevent_struct.h"
#include "libevent/include/event2/event.h"

#include "hiredis/hiredis.h"

/**
 * @class NFRedisClientSocket
 * @brief Redis客户端Socket管理类
 * 
 * 基于libevent和hiredis实现的Redis客户端网络连接管理器。
 * 负责与Redis服务器建立连接、维护连接状态、处理网络事件等。
 * 
 * 主要功能：
 * - 网络连接管理：建立、维护、关闭Redis连接
 * - 事件驱动：基于libevent的异步事件处理
 * - 数据传输：支持命令发送和响应接收
 * - 状态监控：实时监控连接状态和网络事件
 * - 自动重连：支持断线重连机制
 * - 协议解析：集成hiredis进行Redis协议解析
 */
class NFRedisClientSocket
{
public:
	/**
	 * @enum NF_NET_EVENT
	 * @brief 网络事件类型枚举
	 * 
	 * 定义各种网络连接事件类型，用于状态监控和事件处理
	 */
	enum NF_NET_EVENT
	{
		NF_NET_EVENT_NONE = 0,        ///< 无事件状态
		NF_NET_EVENT_EOF = 0x10,      ///< 连接结束事件（对端关闭连接）
		NF_NET_EVENT_ERROR = 0x20,    ///< 网络错误事件
		NF_NET_EVENT_TIMEOUT = 0x40,  ///< 连接超时事件
		NF_NET_EVENT_CONNECTED = 0x80, ///< 连接建立成功事件
	};

	/**
	 * @brief 构造函数
	 * 初始化Redis客户端Socket，创建libevent相关组件
	 */
	NFRedisClientSocket();
	
	/**
	 * @brief 析构函数
	 * 释放网络资源，关闭连接，清理libevent组件
	 */
	virtual ~NFRedisClientSocket();

	/**
	 * @brief 连接到Redis服务器
	 * @param ip Redis服务器IP地址
	 * @param port Redis服务器端口号
	 * @return 返回连接的文件描述符，-1表示连接失败
	 * 
	 * 建立与Redis服务器的TCP连接，设置相应的回调函数
	 */
	int64_t Connect(const std::string& ip, const int port);
	
	/**
	 * @brief 重新连接到Redis服务器
	 * @param ip Redis服务器IP地址
	 * @param port Redis服务器端口号
	 * @return 返回true表示重连成功，false表示失败
	 * 
	 * 断线重连功能，先关闭现有连接再建立新连接
	 */
	bool ReConnect(const std::string& ip, const int port);
	
	/**
	 * @brief 关闭连接
	 * @return 返回0表示成功，非0表示失败
	 * 
	 * 主动关闭与Redis服务器的连接，释放相关资源
	 */
    int Close();
    
    /**
     * @brief 发送数据到Redis服务器
     * @param buf 要发送的数据缓冲区
     * @param count 要发送的数据字节数
     * @return 返回实际发送的字节数，-1表示发送失败
     * 
     * 将Redis命令数据发送到服务器
     */
    int Write(const char *buf, size_t count);

    /**
     * @brief 执行网络事件处理
     * @return 返回0表示正常，非0表示异常
     * 
     * 处理待发送的网络事件，驱动libevent事件循环
     */
    int Execute();

	/**
	 * @brief 检查连接状态
	 * @return 返回true表示已连接，false表示未连接
	 * 
	 * 判断当前是否与Redis服务器保持有效连接
	 */
	bool IsConnect();
	
	/**
	 * @brief 检查是否无事件状态
	 * @return 返回true表示无事件，false表示有待处理事件
	 * 
	 * 判断当前网络状态是否为空闲状态
	 */
	bool IsNone();
	
	/**
	 * @brief 获取Redis读取器
	 * @return 返回hiredis的redisReader指针
	 * 
	 * 获取用于解析Redis响应的读取器实例
	 */
	redisReader* GetRedisReader();

protected:
	/**
	 * @brief 监听器回调函数（静态）
	 * @param listener 事件监听器
	 * @param fd 文件描述符
	 * @param sa 套接字地址
	 * @param socklen 地址长度
	 * @param user_data 用户数据指针
	 * 
	 * libevent监听器的回调函数，处理新连接事件
	 */
	static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int socklen, void* user_data);
	
	/**
	 * @brief 连接读取回调函数（静态）
	 * @param bev 缓冲事件结构
	 * @param user_data 用户数据指针
	 * 
	 * 当有数据可读时被调用，处理来自Redis服务器的响应
	 */
	static void conn_readcb(struct bufferevent* bev, void* user_data);
	
	/**
	 * @brief 连接写入回调函数（静态）
	 * @param bev 缓冲事件结构
	 * @param user_data 用户数据指针
	 * 
	 * 当写缓冲区可写时被调用，处理数据发送完成事件
	 */
	static void conn_writecb(struct bufferevent* bev, void* user_data);
	
	/**
	 * @brief 连接事件回调函数（静态）
	 * @param bev 缓冲事件结构
	 * @param events 事件类型
	 * @param user_data 用户数据指针
	 * 
	 * 处理连接相关的各种事件（连接、断开、错误等）
	 */
	static void conn_eventcb(struct bufferevent* bev, short events, void* user_data);
	
	/**
	 * @brief 日志回调函数（静态）
	 * @param severity 日志级别
	 * @param msg 日志消息
	 * 
	 * libevent的日志输出回调函数
	 */
	static void log_cb(int severity, const char* msg);

	/**
	 * @brief 从URL中提取IP地址
	 * @param url 输入的URL字符串
	 * @return 返回提取的IP地址字符串
	 * 
	 * 解析URL格式的地址字符串，提取其中的IP部分
	 */
	std::string GetIP(const std::string& url);

private:
	/**
	 * @brief libevent事件基础结构
	 * 管理所有网络事件的核心结构
	 */
	struct event_base* base;
	
	/**
	 * @brief 缓冲事件结构
	 * 管理连接的读写缓冲区和事件
	 */
	struct bufferevent* bev;
	
	/**
	 * @brief 事件监听器
	 * 用于监听新的连接请求（主要用于服务器模式）
	 */
	struct evconnlistener* listener;

	/**
	 * @brief 当前网络事件状态
	 * 记录当前连接的事件状态
	 */
	NF_NET_EVENT mNetStatus;
	
	/**
	 * @brief Redis协议读取器
	 * hiredis提供的Redis响应解析器
	 */
	redisReader* m_pRedisReader;
	
	/**
	 * @brief 连接文件描述符
	 * 当前网络连接的文件描述符
	 */
	int64_t fd_;
};


#endif //NFREDISPLUGIN_NFREDISCLIENTSOCKET_H
