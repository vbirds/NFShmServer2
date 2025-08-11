#include "Encrypt.h"
#include "NFComm/NFCore/NFPlatform.h"

/**
 * @file Encrypt.cpp
 * @brief 网络数据加密解密实现文件
 * 
 * 该文件实现了基于XOR的简单加密解密功能，包括：
 * - 全局加密密钥定义
 * - 数据加密函数实现
 * - 数据解密函数实现
 * - XOR加密算法实现
 * 
 * 主要功能：
 * - 提供网络数据传输的基本安全保护
 * - 支持任意长度数据的加密解密
 * - 使用对称加密算法，加密解密使用相同密钥
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief 全局加密密钥
 * 
 * 用于XOR加密解密的密钥，值为87（二进制：01010111）
 * 加密和解密使用相同的密钥进行XOR运算
 */
int8_t g_key = 87;// 01010111;

/**
 * @brief 数据加密函数实现
 * 
 * 使用XOR算法对数据进行加密，包括：
 * - 遍历数据缓冲区的每个字节
 * - 对每个字节与全局密钥进行XOR运算
 * - 原地修改数据，不返回新缓冲区
 * 
 * @param pChar 要加密的数据缓冲区指针
 * @param length 数据长度
 */
void Encryption(char* pChar, int length)
{
    for (int i = 0; i < length; ++i)
    {
        pChar[i] = static_cast<unsigned char>(pChar[i] ^ g_key);
    }
}

/**
 * @brief 数据解密函数实现
 * 
 * 使用XOR算法对数据进行解密，包括：
 * - 遍历数据缓冲区的每个字节
 * - 对每个字节与全局密钥进行XOR运算
 * - 原地修改数据，不返回新缓冲区
 * 
 * 注意：由于XOR运算的特性，加密和解密使用相同的算法
 * 
 * @param pChar 要解密的数据缓冲区指针
 * @param length 数据长度
 */
void Decryption(char* pChar, int length)
{
    for (int i = 0; i < length; ++i)
    {
        pChar[i] = static_cast<unsigned char>(pChar[i] ^ g_key);
    }
}

