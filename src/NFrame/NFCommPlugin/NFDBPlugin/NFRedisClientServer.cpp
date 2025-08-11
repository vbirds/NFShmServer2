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
//    @FileName         :    NFRedisClientServer.cpp
//    @Author           :    lvsheng.huang
//    @Date             :   2022-09-18
//    @Module           :    NFRedisClientServer
//    @Description      :    Redis服务器管理操作实现文件，提供Redis服务器级别的管理命令
//
// -------------------------------------------------------------------------

#include "NFRedisDriver.h"

// 实现Redis服务器管理相关操作
// 包括：FLUSHALL, FLUSHDB, INFO, CONFIG等服务器管理命令

void NFRedisDriver::FLUSHALL()
{
    NFRedisCommand cmd(GET_NAME(FLUSHALL));

	NF_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply != nullptr)
	{
		
	}
}

void NFRedisDriver::FLUSHDB()
{
    NFRedisCommand cmd(GET_NAME(FLUSHDB));

	NF_SHARE_PTR<redisReply> pReply = BuildSendCmd(cmd);
	if (pReply != nullptr)
	{
		
	}
}
