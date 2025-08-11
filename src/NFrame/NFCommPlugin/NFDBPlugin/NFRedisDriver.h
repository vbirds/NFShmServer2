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
//    @FileName         :    NFRedisDriver.h
//    @Author           :    lvsheng.huang
//    @Date             :   2022-09-18
//    @Module           :    NFRedisDriver
//    @Desc             :    Redis数据库驱动头文件，提供Redis数据库操作接口。
//                          该文件定义了NFShmXFrame框架的Redis数据库驱动类，提供Redis数据库的完整操作功能，
//                          包括连接管理、键操作、字符串操作、哈希操作、列表操作、集合操作、
//                          有序集合操作、发布订阅、服务器管理、数据存储等功能。
//                          主要功能包括Redis连接建立和认证、各种数据类型的CRUD操作、
//                          支持Protobuf对象的存储和查询、事务处理和错误处理
//    @Description      :    Redis数据库驱动头文件，提供Redis数据库操作接口
//
// -------------------------------------------------------------------------

#ifndef NFREDISPLUGIN_NFREDISCLIENT_H
#define NFREDISPLUGIN_NFREDISCLIENT_H

#define GET_NAME(functionName)   (#functionName)

#include <string>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <ctime>
#include <sstream>
#include <iostream>
#include <random>
#include <thread>

#include "NFRedisCommand.h"
#include "NFRedisClientSocket.h"

#include "NFComm/NFPluginModule/NFINosqlModule.h"
#include "NFComm/NFKernelMessage/FrameSqlData.pb.h"

/**
 * @class NFRedisDriver
 * @brief Redis数据库驱动类
 * 
 * 继承自NFINosqlDriver接口，提供Redis数据库的完整操作功能。
 * 封装了Redis的所有主要命令，包括字符串、哈希、列表、集合、有序集合等数据类型的操作。
 * 
 * 主要功能：
 * - 连接管理：连接建立、认证、状态检查
 * - 键操作：删除、存在性检查、过期时间设置
 * - 字符串操作：GET/SET、追加、递增递减
 * - 哈希操作：HGET/HSET、HMGET/HMSET等
 * - 列表操作：LPUSH/RPUSH、LPOP/RPOP等
 * - 集合操作：SADD/SREM、并集交集等
 * - 有序集合操作：ZADD/ZREM、排名查询等
 * - 发布订阅：PUBLISH/SUBSCRIBE
 * - 服务器管理：INFO、FLUSHDB等
 * - 数据存储：支持Protobuf对象的存储和查询
 */
class NFRedisDriver : public NFINosqlDriver
{
public:
	/**
	 * @brief 构造函数
	 * 初始化Redis驱动实例
	 */
	NFRedisDriver();
	
	/**
	 * @brief 析构函数
	 * 释放Redis驱动占用的资源
	 */
	virtual ~NFRedisDriver() {}

	/**
	 * @brief 连接到Redis服务器
	 * @param ip Redis服务器IP地址
	 * @param port Redis服务器端口号
	 * @param auth 认证密码，默认为空
	 * @return 返回true表示连接成功，false表示连接失败
	 */
	virtual bool Connect(const std::string& ip, const int port, const std::string& auth = "");

	/**
	 * @brief 检查连接是否可用
	 * @return 返回true表示连接可用，false表示不可用
	 */
	virtual bool Enable();
	
	/**
	 * @brief 检查连接是否已认证
	 * @return 返回true表示已认证，false表示未认证
	 */
	virtual bool Authed();
	
	/**
	 * @brief 检查连接是否繁忙
	 * @return 返回true表示繁忙，false表示空闲
	 */
	virtual bool Busy();

	/**
	 * @brief 执行Redis命令
	 * @return 返回true表示执行成功，false表示执行失败
	 */
	virtual bool Execute();
	
	/**
	 * @brief 保持连接活跃
	 * @return 返回true表示保活成功，false表示保活失败
	 */
	virtual bool KeepLive();

	/**
	 * @brief 重新连接Redis服务器
	 * @return 返回true表示重连成功，false表示重连失败
	 */
	bool ReConnect();

	/**
	 * @brief 检查是否已连接
	 * @return 返回true表示已连接，false表示未连接
	 */
	bool IsConnect();

public:
	/**
	 * @brief 根据条件选择数据
	 * @param select 选择条件
	 * @param privateKey 私有键
	 * @param fields 字段集合
	 * @param privateKeySet 私有键集合
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int SelectByCond(const NFrame::storesvr_sel& select, std::string& privateKey, std::unordered_set<std::string>& fields, std::unordered_set<std::string>& privateKeySet);

	/**
	 * @brief 获取私有键
	 * @param packageName 包名
	 * @param className 类名
	 * @param privateKey 输出的私有键
	 * @return 返回0表示成功，非0表示失败
	 */
	virtual int GetPrivateKey(const std::string& packageName, const std::string& className, std::string& privateKey);

	/**
	 * @brief 根据条件选择数据（完整版本）
	 * @param select 选择条件对象
	 * @param privateKey 私有键
	 * @param fields 字段集合
	 * @param privateKeySet 私有键集合
	 * @param leftPrivateKeySet 剩余的私有键集合（输出）
	 * @param select_res 选择结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SelectByCond(const NFrame::storesvr_sel &select, const std::string& privateKey, const std::unordered_set<std::string>& fields,
                       const std::unordered_set<std::string>& privateKeySet, std::unordered_set<std::string>& leftPrivateKeySet,
                       ::google::protobuf::RepeatedPtrField<NFrame::storesvr_sel_res>& select_res);

	/**
	 * @brief 根据条件删除数据
	 * @param select 删除条件对象
	 * @param privateKey 私有键
	 * @param privateKeySet 私有键集合
	 * @param select_res 删除结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int DeleteByCond(const NFrame::storesvr_del &select, const std::string& privateKey,
                             const std::unordered_set<std::string>& privateKeySet,
                             NFrame::storesvr_del_res &select_res);

	/**
	 * @brief 根据条件修改数据
	 * @param select 修改条件对象
	 * @param privateKey 私有键
	 * @param privateKeySet 私有键集合
	 * @param leftPrivateKeySet 剩余的私有键集合（输出）
	 * @param select_res 修改结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int ModifyByCond(const NFrame::storesvr_mod &select, const std::string& privateKey,
                     const std::unordered_set<std::string>& privateKeySet, std::unordered_set<std::string>& leftPrivateKeySet,
                     NFrame::storesvr_mod_res &select_res);

	/**
	 * @brief 根据条件更新数据
	 * @param select 更新条件对象
	 * @param privateKey 私有键
	 * @param privateKeySet 私有键集合
	 * @param leftPrivateKeySet 剩余的私有键集合（输出）
	 * @param select_res 更新结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int UpdateByCond(const NFrame::storesvr_update &select, const std::string& privateKey,
                             const std::unordered_set<std::string>& privateKeySet, std::unordered_set<std::string>& leftPrivateKeySet,
                             NFrame::storesvr_update_res &select_res);
public:

	/**
	 * @brief 选择对象数据
	 * @param select 选择条件对象
	 * @param select_res 选择结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SelectObj(const NFrame::storesvr_selobj &select, NFrame::storesvr_selobj_res &select_res);

	/**
	 * @brief 保存对象数据（字段映射版本）
	 * @param packageName 包名
	 * @param tbName 表名
	 * @param className 类名
	 * @param privateKey 私有键
	 * @param recordsMap 记录字段映射
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SaveObj(const std::string& packageName, const std::string& tbName, const std::string& className, const std::string& privateKey, const std::map<std::string, std::string>& recordsMap);
    
	/**
	 * @brief 保存对象数据（记录字符串版本）
	 * @param packageName 包名
	 * @param tbName 表名
	 * @param className 类名
	 * @param record 记录字符串
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SaveObj(const std::string& packageName, const std::string& tbName, const std::string& className, const std::string& record);
    
	/**
	 * @brief 保存对象数据（选择对象版本）
	 * @param select 选择条件对象
	 * @param select_res 选择结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SaveObj(const NFrame::storesvr_selobj &select, NFrame::storesvr_selobj_res &select_res);
    
	/**
	 * @brief 保存对象数据（插入对象版本）
	 * @param select 插入条件对象
	 * @param select_res 插入结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SaveObj(const NFrame::storesvr_insertobj &select, NFrame::storesvr_insertobj_res& select_res);
    
	/**
	 * @brief 保存对象数据（修改对象版本）
	 * @param select 修改条件对象
	 * @param select_res 修改结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SaveObj(const NFrame::storesvr_modobj &select, NFrame::storesvr_modobj_res& select_res);
    
	/**
	 * @brief 保存对象数据（更新对象版本）
	 * @param select 更新条件对象
	 * @param select_res 更新结果（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int SaveObj(const NFrame::storesvr_updateobj &select, NFrame::storesvr_updateobj_res& select_res);

	/**
	 * @brief 删除对象数据（删除对象版本）
	 * @param select 删除条件对象
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int DeleteObj(const NFrame::storesvr_delobj &select);
    
	/**
	 * @brief 删除对象数据（插入对象版本）
	 * @param select 插入条件对象
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int DeleteObj(const NFrame::storesvr_insertobj &select);
    
	/**
	 * @brief 删除对象数据（修改对象版本）
	 * @param select 修改条件对象
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int DeleteObj(const NFrame::storesvr_modobj &select);
public:
	/**
	 * @brief 检查对象是否存在
	 * @param db_key 数据库键名
	 * @return 返回true表示对象存在，false表示不存在
	 */
    virtual bool ExistObj(const std::string& db_key);
    
	/**
	 * @brief 获取对象键信息
	 * @param packageName 包名
	 * @param tableName 表名
	 * @param className 类名
	 * @param record 记录数据
	 * @param privateKey 私有键（输出）
	 * @param privateKeyValue 私有键值（输出）
	 * @param db_key 数据库键（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    virtual int GetObjKey(const std::string& packageName, const std::string& tableName, const std::string& className, const std::string& record, std::string& privateKey, std::string& privateKeyValue, std::string& db_key);
public:
	/**
	 * @brief 获取私有键集合
	 * @param tbname 表名
	 * @param field 字段名
	 * @param fieldValue 字段值
	 * @return 返回私有键字符串
	 */
    std::string GetPrivateKeys(const std::string& tbname, const std::string& field, const std::string& fieldValue);
    
	/**
	 * @brief 获取私有字段信息
	 * @param packageName 包名
	 * @param className 类名
	 * @param record 记录数据
	 * @param field 字段名（输出）
	 * @param fieldValue 字段值（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    int GetPrivateFields(const std::string& packageName, const std::string& className, const std::string& record, std::string& field, std::string& fieldValue);
    
	/**
	 * @brief 获取对象字段映射
	 * @param packageName 包名
	 * @param className 类名
	 * @param record 记录数据
	 * @param keyMap 键映射（输出）
	 * @param kevValueMap 键值映射（输出）
	 * @return 返回0表示成功，非0表示失败
	 */
    int GetObjFields(const std::string& packageName, const std::string& className, const std::string& record, std::map<std::string, std::string> &keyMap, std::map<std::string, std::string> &kevValueMap);
public:
	/**
	 * @brief 设置对象数据
	 * @param packageName 包名
	 * @param className 类名
	 * @param key 键名
	 * @param value 值
	 * @return 返回true表示设置成功，false表示失败
	 */
    bool SetObj(const std::string& packageName, const std::string& className, const std::string& key, const std::string& value);
    
	/**
	 * @brief 获取对象数据（指定字段版本）
	 * @param packageName 包名
	 * @param className 类名
	 * @param key 键名
	 * @param fields 要获取的字段集合
	 * @param value 返回的值（输出）
	 * @return 返回true表示获取成功，false表示失败
	 */
    bool GetObj(const std::string& packageName, const std::string& className, const std::string& key, const std::unordered_set<std::string>& fields, std::string& value);
    
	/**
	 * @brief 获取对象数据（完整版本）
	 * @param packageName 包名
	 * @param className 类名
	 * @param key 键名
	 * @param value 返回的值（输出）
	 * @return 返回true表示获取成功，false表示失败
	 */
    bool GetObj(const std::string& packageName, const std::string& className, const std::string& key, std::string& value);

    /**
     * @brief 将数据库结果行转换为Protobuf消息对象
     * 
     * 将数据库查询结果中的一行数据转化并存储在指定的Protobuf Message对象中。
     * 这个方法实现了数据库记录到对象的自动映射功能。
     * 
     * @param vecFields 字段名称向量，包含所有字段的名称
     * @param vecValues 字段值向量，包含对应字段的值，与vecFields一一对应
     * @param packageName 包名，用于定位Protobuf包
     * @param className 类名，用于创建对应的Message类型
     * @param pMessage 输出参数，转换后的Message对象指针
     * @return 返回0表示转换成功，非0表示转换失败
     * 
     * @note vecFields和vecValues的大小必须相等，且一一对应
     * @note 调用者负责释放pMessage指针指向的内存
     */
    int TransTableRowToMessage(const std::vector<std::string> &vecFields, const std::vector<std::string>& vecValues, const std::string &packageName, const std::string &className,
                               google::protobuf::Message **pMessage);
public:

	/**
	* @brief 如果为Redis设置了密码，必须使用AUTH命令连接到服务器，然后才能使用其他命令
	* @param auth 认证密码
	* @return 成功返回true，失败返回false
	*/
	virtual bool AUTH(const std::string& auth);

	/**
	* @brief 选择数据库
	* @param dbnum 数据库索引号
	* @return 成功返回true，失败返回false
	*/
	virtual bool SelectDB(int dbnum);

	/*
	ECHO
	PING
	QUIT
	*/
	/////////client key//////////////
	/**
	* @brief 删除一个键
	* @param key 键名
	* @return 成功返回true，失败返回false
	*/
	virtual bool DEL(const std::string& key);

	//NF_SHARE_PTR<NFRedisResult> DUMP(const std::string& key, std::string& out);

	/**
	* @brief 检查键是否存在
	* @param key 键名
	* @return 如果键存在返回true，不存在返回false
	*/
	virtual bool EXISTS(const std::string& key);

	/**
	* @brief 为键设置超时时间，超时后键将自动删除
	* @param key 键名
	* @param secs 超时秒数
	* @return 设置成功返回true，失败返回false（如果键不存在）
	*/
	virtual bool EXPIRE(const std::string& key, const unsigned int secs);

	/**
	* @brief 类似EXPIRE，但使用Unix时间戳设置过期时间
	* @param key 键名
	* @param unixTime Unix时间戳
	* @return 如果设置成功返回true，键不存在返回false
	*/
	virtual bool EXPIREAT(const std::string& key, const int64_t unixTime);

	/**
	* @brief 移除键的过期时间，将键从易失性转为持久性
	* @param key 键名
	* @return 如果移除成功返回true，键不存在或没有过期时间返回false
	*/
	virtual bool PERSIST(const std::string& key);

	/**
	* @brief 返回键的剩余生存时间
	* @param key 键名
	* @return TTL秒数，负值表示错误
	* 键不存在返回-2，键存在但没有过期时间返回-1
	*/
	virtual int TTL(const std::string& key);

	/**
	* @brief 返回存储在键中的值的类型
	* @param key 键名
	* @return 键的类型字符串，键不存在时返回"none"
	*/
	virtual std::string TYPE(const std::string& key);
	//NF_SHARE_PTR<NFRedisResult> SCAN(const std::string& key);


	/////////client String//////////////
	/**
	* @brief 如果键已存在且是字符串，将值追加到字符串末尾
	* 如果键不存在，创建为空字符串
	* @param key 键名
	* @param value 要追加的值
	* @param length 追加操作后字符串的长度
	* @return 追加操作后字符串的长度
	*/
	virtual bool APPEND(const std::string& key, const std::string& value, int& length);

	/**
	* @brief 将存储在键中的数字减1
	* 如果键不存在，在执行操作前设置为0
	* @param key 键名
	* @param value 减量后的值
	* @return 减量后键的值
	* 如果键包含错误类型的值或无法表示为整数的字符串，返回错误
	*/
	virtual bool DECR(const std::string& key, int64_t& value);

	/**
	* @brief 将存储在键中的数字减去指定的减量
	* 如果键不存在，在执行操作前设置为0
	* @param key 键名
	* @param decrement 减量值
	* @param value 减量后的值
	* @return 减量后键的值
	* 如果键包含错误类型的值或无法表示为整数的字符串，返回错误
	*/
	virtual bool DECRBY(const std::string& key, const int64_t decrement, int64_t& value);

	/**
	* @brief 获取键的值，如果键不存在返回空字符串
	* @param key 键名
	* @param value 输出的值
	* @return 键的值，键不存在时返回空字符串。如果存储的值不是字符串则返回错误
	*/
	virtual bool GET(const std::string& key, std::string & value);

	//NF_SHARE_PTR<NFRedisResult> GETBIT(const std::string& key);
	//NF_SHARE_PTR<NFRedisResult> GETRANGE(const std::string& key);
	/**
	* @brief 原子性地设置键为值并返回存储在键中的旧值
	* @param key 键名
	* @param value 新值
	* @param oldValue 旧值
	* @return 存储在键中的旧值，键不存在时返回空字符串
	*/
	virtual bool GETSET(const std::string& key, const std::string& value, std::string& oldValue);

	/**
	* @brief 将存储在键中的数字增1
	* 如果键不存在，在执行操作前设置为0
	* @param key 键名
	* @param value 增量后的值
	* @return 增量后键的值
	* 如果键包含错误类型的值或无法表示为整数的字符串，返回错误
	*/
	virtual bool INCR(const std::string& key, int64_t& value);

	/**
	* @brief 将存储在键中的数字增加指定的增量
	* 如果键不存在，在执行操作前设置为0
	* @param key 键名
	* @param increment 增量值
	* @param value 增量后的值
	* @return 增量后键的值
	* 如果键包含错误类型的值或无法表示为整数的字符串，返回错误
	*/
	virtual bool INCRBY(const std::string& key, const int64_t increment, int64_t& value);

	/**
	* @brief 将存储在键中表示浮点数的字符串增加指定的增量
	* @param key 键名
	* @param increment 增量值
	* @param value 增量后的值
	* @return 增量后键的值
	* 如果键包含错误类型的值或无法表示为浮点数的字符串，返回错误
	*/
	virtual bool INCRBYFLOAT(const std::string& key, const float increment, float& value);

	/**
	* @brief 返回所有指定键的值
	* @param keys 键名数组
	* @param values 键值数组
	* @return 指定键的值列表
	* 对于不存在或不是字符串值的键，返回空字符串。因此操作永远不会失败
	*/
	virtual bool MGET(const string_vector& keys, string_vector& values);

	/**
	* @brief 将给定的键设置为各自的值，MSET是原子的，所有给定的键都是一次设置的
	* @param values 键值对数组
	* @return 总是返回OK，因为MSET不会失败
	*/
	virtual void MSET(const string_pair_vector& values);

	//NF_SHARE_PTR<NFRedisResult> MSETNX(const std::string& key);
	//NF_SHARE_PTR<NFRedisResult> PSETEX(const std::string& key);
	/**
	* @brief 设置键来保存字符串值。如果键已经保存值，无论其类型如何都会被覆盖
	* @param key 键名
	* @param value 键值
	* @return 如果SET正确执行返回true
	* 如果因为用户指定了NX或XX选项但条件不满足而未执行SET操作，返回false
	*/
	virtual bool SET(const std::string& key, const std::string& value);

	//NF_SHARE_PTR<NFRedisResult> SETBIT(const std::string& key);
	/**
	* @brief 设置键来保存字符串值，并在给定秒数后设置键超时
	* @param key 键名
	* @param value 键值
	* @param time 超时时间（秒）
	* @return 如果SETEX正确执行返回true，当秒数无效时返回false
	*/
	virtual bool SETEX(const std::string& key, const std::string& value, int time);

	/**
	* @brief SET if Not eXists - 仅当键不存在时设置键来保存字符串值
	* @param key 键名
	* @param value 键值
	* @return 如果键被设置返回true，如果键未被设置返回false
	*/
	virtual bool SETNX(const std::string& key, const std::string& value);
	//NF_SHARE_PTR<NFRedisResult> SETRANGE(const std::string& key);

	/**
	* @brief 返回存储在键中的字符串值的长度
	* @param key 键名
	* @param length 输出的字符串长度
	* @return 键处字符串的长度，键不存在或类型错误时返回false
	*/
	virtual bool STRLEN(const std::string& key, int& length);

	/////////client hash//////////////
	/**
	* @brief Removes the specified fields from the hash stored at key
	* @param field/fields [in] the fields you want to remove
	* @return the number of fields that were removed from the hash, not including specified but non existing fields.
	*/
	virtual int HDEL(const std::string& key, const std::string& field);
	virtual int HDEL(const std::string& key, const string_vector& fields);

	/**
	* @brief Returns if field is an existing field in the hash stored at key.
	* @param field [in] the field you want to check
	* @return true if the hash contains field. false if the hash does not contain field, or key does not exist.
	*/
	virtual bool HEXISTS(const std::string& key, const std::string& field);

	/**
	* @brief Returns the value associated with field in the hash stored at key.
	* @param field [in] the field you want to get
	* @param value [out] the value you want to get
	* @return true if the hash contains field. false if the hash does not contain field, or key does not exist.
	*/
	virtual bool HGET(const std::string& key, const std::string& field, std::string& value);

	/**
	* @brief Returns all field names in the hash stored at key.
	* @param key [in] the name of the key
	* @param values [out] all the key & values of the key
	* @return true when key exist, false when key does not exist.
	*/
	virtual bool HGETALL(const std::string& key, std::vector<string_pair>& values);

	/**
	* @brief Increments the number stored at key by increment
	* If the key does not exist, it is set to 0 before performing the operation
	* @param key [in] name of key
	* @param field [in] field
	* @param by [in] increment
	* @return the value of key after the increment
	* An error is returned if the key contains a value of the wrong type or contains a string that can not be represented as integer.
	*/
	virtual bool HINCRBY(const std::string& key, const std::string& field, const int by, int64_t& value);

	/**
	* @brief Increment the string representing a floating point number stored at key by the specified increment
	* @param key [in] name of key
	* @param field [in] field
	* @param by [in] increment
	* @param value [out] the value of key after the increment
	* @return the value of key after the increment
	* An error is returned if the key contains a value of the wrong type or contains a string that can not be represented as float/double.
	*/
	virtual bool HINCRBYFLOAT(const std::string& key, const std::string& field, const float by, float& value);

	/**
	* @brief Returns all field names in the hash stored at key.
	* @param key [in] the name of the key
	* @param fields [out] the fields of the key
	* @return true when key exist, false when key does not exist.
	*/
	virtual bool HKEYS(const std::string& key, std::vector<std::string>& fields);

    virtual bool KEYS(const std::string& key, std::vector<std::string>& values);

	/**
	* @brief Returns the number of fields contained in the hash stored at key.
	* @param key [in] the name of the key
	* @param number [out] number of fields in the hash
	* @return true when key exist, false when key does not exist.
	*/
	virtual bool HLEN(const std::string& key, int& number);

	/**
	* @brief Returns the values associated with the specified fields in the hash stored at key.
	* @param key [in] the name of the key
	* @param field [in] the fields you want to get
	* @param values [out] the values return
	* @return list of values associated with the given fields, in the same order as they are requested.
	*/
	virtual bool HMGET(const std::string& key, const string_vector& fields, string_vector& values);

	/**
	* @brief Sets the specified fields to their respective values in the hash stored at key
	* @param key [in] the name of the key
	* @param values [in] the fields/value you want to set
	* @return true if cmd success, false when the key not a hashmap
	* If the key does not exist, a new key holding a hash is created
	*/
	virtual bool HMSET(const std::string& key, const std::vector<string_pair>& values);
	virtual bool HMSET(const std::string& key, const string_vector& fields, const string_vector& values);

	/**
	* @brief Sets the specified field to their respective values in the hash stored at key
	* @param key [in] the name of the key
	* @param values [in] the fields/value you want to set
	* @return true if cmd success, false when the key not a hashmap
	* If the key does not exist, a new key holding a hash is created
	*/
	virtual bool HSET(const std::string& key, const std::string& field, const std::string& value);

	/**
	* @brief SET if Not eXists --- Sets field in the hash stored at key to value, only if field does not yet exist
	* @param key [in] name of key
	* @param field [in] field of the hashmap
	* @param value [in] value of the field
	* @return true if the key was set, false if the key was not set(maybe not a hashmap key)
	*/
	virtual bool HSETNX(const std::string& key, const std::string& field, const std::string& value);

	/**
	* @brief Returns all values in the hash stored at key.
	* @param key [in] name of key
	* @param values [out] fields/values that you want to know
	* @return false when key does not exist or not a hashmap key
	*/
	virtual bool HVALS(const std::string& key, string_vector& values);
	//NF_SHARE_PTR<NFRedisResult> HSCAN(const std::string& key, const std::string& field);

	/**
	* @brief Returns the length of the string value stored at key
	* @param key [in] name of key
	* @param field [in] field that you want to know
	* @param length [out] the length of the string at field, or 0 when field does not exist.
	* @return true when cmd success, false when key does not exist or not a list key.
	*/
	virtual bool HSTRLEN(const std::string& key, const std::string& field, int& length);

	/////////client list//////////////

	//NF_SHARE_PTR<NFRedisResult> BLPOP(const std::string& key, string_vector& values);
	//NF_SHARE_PTR<NFRedisResult> BRPOP(const std::string& key, string_vector& values);
	//NF_SHARE_PTR<NFRedisResult> BRPOPLPUSH(const std::string& key, string_vector& values);
	/**
	* @brief Returns the element at index index in the list stored at key
	* @param key [in] name of key
	* @param index [in] index that you want to know
	* @param value [out] th value that you got
	* @return true when cmd success, false when key does not exist or not a list key.
	*/
	virtual bool LINDEX(const std::string& key, const int index, std::string& value);
	//NF_SHARE_PTR<NFRedisResult> LINSERT(const std::string& key, const std::string& value1, const std::string& value2);

	/**
	* @brief Returns the length of the list stored at key
	* @param key [in] name of key
	* @param length [out] the length that you got
	* @return true when cmd success, false when key does not exist or not a list key.
	*/
	virtual bool LLEN(const std::string& key, int& length);

	/**
	* @brief Removes and returns the first element of the list stored at key.
	* @param key [in] name of key
	* @param value [out] the value of the first element
	* @return true when cmd success, false when key does not exist or not a list key.
	*/
	virtual bool LPOP(const std::string& key, std::string& value);

	/**
	* @brief Insert all the specified values at the head of the list stored at key
	* @param key [in] name of key
	* @param value [in] the value that you want to push to the head
	* @return length of the list when cmd success, 0 when key does not a list key.
	*/
	virtual int LPUSH(const std::string& key, const std::string& value);

	/**
	* @brief Insert all the specified values at the head of the list stored at key
	* only if key already exists and holds a list.
	* @param key [in] name of key
	* @param value [in] the value that you want to push to the head
	* @return length of the list when cmd success, 0 when key does not a list key.
	*/
	virtual int LPUSHX(const std::string& key, const std::string& value);


	/**
	* @brief Returns the specified elements of the list stored at key by start(included) and end(included)
	* @param key [in] name of key
	* @param start [in]
	* @param end [in]
	* @param values [out] the value that you want to get
	* @return true when cmd success, false when key does not exist or dose not a list key.
	*/
	virtual bool LRANGE(const std::string& key, const int start, const int end, string_vector& values);
	//NF_SHARE_PTR<NFRedisResult> LREM(const std::string& key, string_vector& values);

	/**
	* @brief Sets the list element at index to value
	* @param key [in] name of key
	* @param index [in] the index of this list
	* @param value [in] the value that you want set
	* @return true when cmd success, false when key dose not a list key.
	*/
	virtual bool LSET(const std::string& key, const int index, const std::string& value);
	//NF_SHARE_PTR<NFRedisResult> LTRIM(const std::string& key, string_vector& values);

	/**
	* @brief Removes and returns the last element of the list stored at key.
	* @param key [in] name of key
	* @param value [out] the value of the last element
	* @return true when cmd success, false when key does not exist or not a list key.
	*/
	virtual bool RPOP(const std::string& key, std::string& value);
	//NF_SHARE_PTR<NFRedisResult> RPOPLPUSH(const std::string& key, string_vector& values);

	/**
	* @brief Insert all the specified values at the last of the list stored at key
	* @param key [in] name of key
	* @param value [in] the value that you want to push to the head
	* @return length of the list when cmd success, 0 when key does not a list key.
	*/
	virtual int RPUSH(const std::string& key, const std::string& value);

	/**
	* @brief Inserts value at the tail of the list stored at key, only if key already exists and holds a list
	* @param key [in] name of key
	* @param value [in] the value that you want push
	* @return true when cmd success, false when key does not exist or dose not a list key.
	*/
	virtual int RPUSHX(const std::string& key, const std::string& value);

	/////////client set//////////////

	/**
	* @brief cmd SADD
	* @param key [in] name of key
	* @param member [in]
	* @return return the number of elements added to the sets when cmd success.
	*/
	virtual int SADD(const std::string& key, const std::string& member);

	/**
	* @brief cmd SCARD
	* @param key [in] name of key
	* @param nCount [out] the size of set
	* @return return true when cmd success.
	*/
	virtual bool SCARD(const std::string& key, int& nCount);

	/**
	* @brief cmd SDIFF
	* @param key_1 [in] name of diff_key1
	* @param key_2 [in] name of diff_key2
	* @param output [out] difference sets
	* @return return true when cmd success.
	*/
	virtual bool SDIFF(const std::string& key_1, const std::string& key_2, string_vector& output);

	/**
	* @brief cmd SDIFFSTORE
	* @param store_key [in] store_diff_key
	* @param key_1 [in] name of diff_key1
	* @param key_2 [in] name of diff_key2
	* @return return true when cmd success.
	*/
	virtual int SDIFFSTORE(const std::string& store_key, const std::string& diff_key1, const std::string& diff_key2);

	/**
	* @brief cmd SINTER
	* @param store_key [in] store_diff_key
	* @param key_1 [in] name of inter_key1
	* @param key_2 [in] name of inter_key2
	* @param output [out] inter_key1 and inter_key2 intersection
	* @return return true when cmd success.
	*/
	virtual bool SINTER(const std::string& key_1, const std::string& key_2, string_vector& output);

	/**
	* @brief cmd SINTERSTORE
	* @param store_key [in] inter_store_key
	* @param inter_key1 [in] name of inter_key1
	* @param inter_key2 [in] name of inter_key2
	* @return return true when cmd success.
	*/
	virtual int SINTERSTORE(const std::string& inter_store_key, const std::string& inter_key1, const std::string& inter_key2);

	/**
	* @brief cmd SISMEMBER
	* @param store_key [in] name of key
	* @param member [in] set's member
	* @return return true when the member in set.
	*/
	virtual bool SISMEMBER(const std::string& key, const std::string& member);

	/**
	* @brief cmd SMEMBERS
	* @param store_key [in] name of key
	* @param output [out] all member of set
	* @return return true when cmd success.
	*/
	virtual bool SMEMBERS(const std::string& key, string_vector& output);

	/**
	* @brief cmd SMOVE
	* @param source_key [in] name of source set key
	* @param dest_key [in] name of destination set key
	* @param member [in] the member of source set
	* @return return true when cmd success.
	*/
	virtual bool SMOVE(const std::string& source_key, const std::string& dest_key, const std::string& member);

	/**
	* @brief cmd SPOP
	* @param key [in] name of key
	* @param output [out] remove member
	* @return return true when cmd success.
	*/
	virtual bool SPOP(const std::string& key, std::string& output);

	/**
	* @brief cmd SRANDMEMBER
	* @param key [in] name of key
	* @param count [in] the number of member
	* @param output [out] remove members
	* @return return true when cmd success.
	*/
	virtual bool SRANDMEMBER(const std::string& key, int count, string_vector& output);

	/**
	* @brief cmd SREM
	* @param key [in] name of key
	* @param output [out] remove members
	* @return return the number of remove member
	*/
	virtual int SREM(const std::string& key, const string_vector& members);

	/**
	* @brief cmd SUNION
	* @param union_key1 [in] name of union_key1
	* @param union_key2 [in] name of union_key2
	* @param output [out] set1 and set2 union member
	* @return return true when cmd success.
	*/
	virtual bool SUNION(const std::string& union_key1, const std::string& union_key2, string_vector& output);

	/**
	* @brief cmd SUNIONSTORE
	* @param dest_store_key [in] name of destination set
	* @param union_key1 [in] name of union_key1
	* @param union_key2 [out] name of union_key2
	* @return return true when cmd success.
	*/
	virtual int SUNIONSTORE(const std::string& dest_store_key, const std::string& union_key1, const std::string& union_key2);



	/////////client SortedSet//////////////
	/**
	* @brief Adds all the specified members with the specified scores to the sorted set stored at key
	* @param key [in] name of key
	* @param member [in]
	* @param score [in]
	* @return return the number of elements added to the sorted sets when cmd success, false when key dose not a z key.
	*/
	virtual int ZADD(const std::string& key, const std::string& member, const double score);

	/**
	* @brief Returns the number of elements of the sorted set stored at key.
	* @param key [in] name of key
	* @return return the number( of elements) of the sorted set, or 0 if key does not exist or not a z key
	*/
	virtual bool ZCARD(const std::string& key, int &nCount);

	/**
	* @brief Returns the number of elements in the sorted set at key with a score between min and max.
	* @param key [in] name of key
	* @param start [in]
	* @param end [in]
	* @return the number of elements in the specified score range, or 0 if key does not exist or not a z key
	*/
	virtual bool ZCOUNT(const std::string& key, const double start, const double end, int &nCount);

	/**
	* @brief Increments the score of member in the sorted set stored at key by increment
	* If member does not exist in the sorted set, it is added with increment as its score (as if its previous score was 0.0)
	* @param key [in] name of key
	* @param member [in]
	* @param score [in]
	* @param newScore [out] the new score of member
	* @return the value of key after the increment
	* An error is returned if the key contains a value of the wrong type or contains a string that can not be represented as float/double.
	*/
	virtual bool ZINCRBY(const std::string& key, const std::string & member, const double score, double& newScore);

	/**
	* @brief Returns the specified range of elements in the sorted set stored at key
	* @param key [in] name of key
	* @param start [in]
	* @param end [in]
	* @param values [out] the members of this range
	* @return true when cmd success, false when key does not exist or not a z key.
	*/
	virtual bool ZRANGE(const std::string& key, const int start, const int end, string_score_vector& values);

	/**
	* @brief Returns all the elements in the sorted set at key with a score between min and max
	* (including elements with score equal to min or max), the elements are considered to be ordered from low to high scores.
	* @param key [in] name of key
	* @param start [in]
	* @param end [in]
	* @param values [out] the members of this range
	* @return true when cmd success, false when key does not exist or not a z key.
	*/
	virtual bool ZRANGEBYSCORE(const std::string & key, const double start, const double end, string_score_vector& values);

	/**
	* @brief Returns the rank of member in the sorted set stored at key, with the scores ordered from low to high
	* @param key [in] name of key
	* @param member [in] the members of this range
	* @param rank [out] the rank of this member
	* @return true if member exists in the sorted set, false when member does not exist or not a z key.
	*/
	virtual bool ZRANK(const std::string & key, const std::string & member, int& rank);

	/**
	* @brief Removes the specified members from the sorted set stored at key. Non existing members are ignored.
	* @param key [in] name of key
	* @param member [in] the members of this range
	* @return true if member exists in the sorted set and removed success, false when member does not exist or not a z key.
	*/
	virtual bool ZREM(const std::string& key, const std::string& member);

	/**
	* @brief Removes all elements in the sorted set stored at key with rank between start and stop
	* @param key [in] name of key
	* @param start [in]
	* @param end [in]
	* @return true if cmd removed success, false when the key is not a z key.
	*/
	virtual bool ZREMRANGEBYRANK(const std::string& key, const int start, const int end);

	/**
	* @brief Removes all elements in the sorted set stored at key with a score between min and max (inclusive).
	* @param key [in] name of key
	* @param min [in]
	* @param max [in]
	* @return true if cmd removed success, false when the key is not a z key.
	*/
	virtual bool ZREMRANGEBYSCORE(const std::string& key, const double min, const double max);

	/**
	* @brief Returns the specified range of elements in the sorted set stored at key.
	* The elements are considered to be ordered from the highest to the lowest score.
	* @param key [in] name of key
	* @param start [in]
	* @param end [in]
	* @param values [out] the members of this range
	* @return true when cmd success, false when key does not exist or not a z key.
	*/
	virtual bool ZREVRANGE(const std::string& key, const int start, const int end, string_score_vector& values);

	/**
	* @brief Returns all the elements in the sorted set at key with a score between max and min
	* (including elements with score equal to max or min)
	* @param key [in] name of key
	* @param start [in]
	* @param end [in]
	* @param values [out] the members of this range
	* @return true when cmd success, false when key does not exist or not a z key.
	*/
	virtual bool ZREVRANGEBYSCORE(const std::string & key, const double start, const double end, string_score_vector& values);


	/**
	* @brief Returns the rank of member in the sorted set stored at key, with the scores ordered from high to low.
	* @param key [in] name of key
	* @param member [in] the members of this range
	* @param rank [out] the rank of this member
	* @return true if member is exists in the sorted set, false when member does not exist or not a z key.
	*/
	virtual bool ZREVRANK(const std::string& key, const std::string& member, int& rank);

	/**
	* @brief Returns the score of member in the sorted set at key.
	* @param key [in] name of key
	* @param member [in] the members of this range
	* @param rank [out] the rank of this member
	* @return true if member is exists in the sorted set, false when member does not exist or not a z key.
	*/
	virtual bool ZSCORE(const std::string& key, const std::string& member, double& score);


	/////////client server//////////////
	/**
	* @brief Removes all data of all DB
	*/
	virtual void FLUSHALL();

	/**
	* @brief Removes all the keys of current DB
	*/
	virtual void FLUSHDB();



	/////////client pubsub//////////////
	/**	@brief cmd PUBLISH
	* @param key [in] name of key
	* @param value [in] publish's msg
	* @return return true when cmd success.
	*/
	virtual bool PUBLISH(const std::string& key, const std::string& value);

	/**	@brief cmd SUBSCRIBE
	* @param key [in] name of key
	* @return return true when cmd success.
	*/
	virtual bool SUBSCRIBE(const std::string& key);

	/**	@brief cmd UNSUBSCRIBE
	* @param key [in] name of key
	* @return return true when cmd success.
	*/
	virtual	bool UNSUBSCRIBE(const std::string& key);

protected:
	/**
	 * @brief 构建并发送Redis命令
	 * @param cmd Redis命令对象，包含要执行的命令和参数
	 * @return 返回Redis回复对象的智能指针
	 * 
	 * 该方法负责将NFRedisCommand对象转换为Redis协议格式，
	 * 并通过网络连接发送到Redis服务器，然后接收服务器的回复。
	 */
	NF_SHARE_PTR<redisReply> BuildSendCmd(const NFRedisCommand& cmd);
	
	/**
	 * @brief 解析Redis服务器的回复
	 * @return 返回解析后的Redis回复对象的智能指针
	 * 
	 * 该方法从网络连接中读取数据并解析Redis服务器的回复，
	 * 将原始的网络数据转换为结构化的redisReply对象。
	 */
	NF_SHARE_PTR<redisReply> ParseForReply();


private:
	/**
	 * @brief 认证状态标志
	 * 
	 * 标识当前连接是否已通过Redis服务器的认证。
	 * true表示已认证，false表示未认证或认证失败。
	 */
	bool mbAuthed;
	
	/**
	 * @brief 繁忙状态标志
	 * 
	 * 标识当前Redis连接是否正在处理命令。
	 * true表示连接繁忙（正在执行命令），false表示连接空闲。
	 */
	bool mbBusy;
	
	/**
	 * @brief Redis客户端Socket连接对象
	 * 
	 * 负责与Redis服务器的网络通信，包括连接建立、
	 * 数据发送和接收等底层网络操作。
	 */
	NFRedisClientSocket* m_pRedisClientSocket;
};


#endif //NFREDISPLUGIN_NFREDISCLIENT_H
