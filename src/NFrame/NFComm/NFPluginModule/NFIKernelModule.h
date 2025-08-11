// -------------------------------------------------------------------------
//    @FileName         :    NFIKernelModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFIKernelModule
//    @Description      :    内核模块接口定义，提供系统核心工具函数和基础服务
//                           包括UUID生成、哈希计算、编码解码等基础功能
//
// -------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <unordered_map>
#include "NFIDynamicModule.h"

/**
 * @brief 内核模块接口类
 * 
 * NFIKernelModule 提供了系统的核心基础服务，包括：
 * 
 * 1. UUID生成服务：
 *    - 支持不同位数的UUID生成
 *    - 考虑客户端兼容性的特殊UUID格式
 * 
 * 2. 哈希计算服务：
 *    - MD5哈希计算
 *    - CRC32和CRC16校验计算
 * 
 * 3. 编码解码服务：
 *    - Base64编码和解码
 * 
 * 这些基础服务被整个系统广泛使用，为上层模块提供统一的底层支持。
 */
class NFIKernelModule
	: public NFIDynamicModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFIKernelModule(NFIPluginManager* p) :NFIDynamicModule(p)
	{

	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构。
	 */
	virtual ~NFIKernelModule()
	{

	}

	/**
	 * @brief 获取通用UUID
	 * @return 返回生成的UUID
	 * 
	 * 生成一个通用的64位唯一标识符，用于系统中各种对象的标识。
	 */
	virtual uint64_t GetUUID() = 0;
	
	/**
	 * @brief 获取64位UUID
	 * @return 返回64位UUID
	 * 
	 * 生成一个完整的64位唯一标识符。
	 */
	virtual uint64_t Get64UUID() = 0;
	
    /**
     * @brief 获取53位UUID
     * @return 返回53位UUID
     * 
     * 生成一个53位的唯一标识符，主要用于与JavaScript等对64位整数
     * 支持有限的客户端环境兼容。
     */
    virtual uint64_t Get53UUID() = 0;
    
    /**
     * @brief 获取32位UUID
     * @return 返回32位UUID
     * 
     * 由于客户端Lua中int64_t类型最大只能表示2的53次方，所以角色cid
     * 由原来的64位改成现在的51位表示（12位区服ID+39位序号）。
     * 注：虽然函数名为32UUID，但实际返回51位标识符。
     */
	virtual uint64_t Get32UUID() = 0;
	
	/**
	 * @brief 计算字符串的MD5哈希值
	 * @param str 待计算哈希的字符串
	 * @return 返回MD5哈希值字符串
	 * 
	 * 计算输入字符串的MD5哈希值，常用于数据完整性验证、
	 * 密码存储等场景。
	 */
	virtual std::string GetMD5(const std::string& str) = 0;
	
	/**
	 * @brief 计算字符串的CRC32校验值
	 * @param s 待计算校验值的字符串
	 * @return 返回32位CRC校验值
	 * 
	 * 计算输入字符串的CRC32校验值，用于数据传输和存储的
	 * 错误检测。
	 */
	virtual uint32_t GetCRC32(const std::string& s) = 0;
	
	/**
	 * @brief 计算字符串的CRC16校验值
	 * @param s 待计算校验值的字符串
	 * @return 返回16位CRC校验值
	 * 
	 * 计算输入字符串的CRC16校验值，相比CRC32占用更少空间，
	 * 适用于对空间要求较高的场景。
	 */
	virtual uint16_t GetCRC16(const std::string& s) = 0;
	
	/**
	 * @brief Base64编码
	 * @param s 待编码的字符串
	 * @return 返回Base64编码后的字符串
	 * 
	 * 将输入字符串进行Base64编码，常用于数据传输和存储中
	 * 的二进制数据编码。
	 */
	virtual std::string Base64Encode(const std::string& s) = 0;
	
	/**
	 * @brief Base64解码
	 * @param s 待解码的Base64字符串
	 * @return 返回解码后的原始字符串
	 * 
	 * 将Base64编码的字符串解码为原始数据。
	 */
	virtual std::string Base64Decode(const std::string& s) = 0;
};


