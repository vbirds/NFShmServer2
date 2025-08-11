// -------------------------------------------------------------------------
//    @FileName         :    NFException.hpp
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFException
//
// -------------------------------------------------------------------------

/**
 * @file NFException.hpp
 * @brief 自定义异常类定义
 * 
 * 此文件定义了基于std::exception的自定义异常类，提供了增强的错误信息
 * 和错误码支持。在Linux平台上还支持自动获取调用栈信息。
 */

#ifndef NF_EXCEPTION_HPP
#define NF_EXCEPTION_HPP

#include <string>
#include <exception>
#include "NFPlatform.h"

#if NF_PLATFORM == NF_PLATFORM_LINUX
#include <execinfo.h>
#include <string.h>
#include <stdlib.h>
#include <cerrno>
#endif

/**
 * @brief 自定义异常类
 * 
 * NFException是基于std::exception的增强异常类，提供了以下特性：
 * - 支持自定义错误消息
 * - 支持系统错误码（errno）
 * - 自动转换错误码为可读消息
 * - Linux平台支持调用栈信息（可选）
 * 
 * 主要用途：
 * - 框架内部错误处理
 * - 系统调用错误包装
 * - 业务逻辑异常抛出
 * 
 * 使用方法：
 * @code
 * // 抛出简单异常
 * throw NFException("配置文件不存在");
 * 
 * // 抛出带错误码的异常
 * if (fd < 0) {
 *     throw NFException("打开文件失败", errno);
 * }
 * 
 * // 捕获和处理异常
 * try {
 *     // 可能抛出异常的代码
 * } catch (const NFException& e) {
 *     std::cout << "错误: " << e.what() << std::endl;
 *     std::cout << "错误码: " << e.getErrCode() << std::endl;
 * }
 * @endcode
 */
class NFException : public std::exception
{
public:
	/**
	 * @brief 构造函数（仅错误消息）
	 * 
	 * 创建一个包含自定义错误消息的异常对象，错误码设置为0。
	 * 
	 * @param buffer 异常的描述信息
	 */
	explicit NFException(const std::string &buffer)
		:_buffer(buffer), _code(0)
	{
		//    getBacktrace();
	}

	/**
	 * @brief 构造函数（错误消息+错误码）
	 * 
	 * 创建一个包含自定义错误消息和系统错误码的异常对象。
	 * 会自动使用strerror()将错误码转换为可读的错误描述并附加到消息中。
	 * 
	 * @param buffer 异常的基础描述信息
	 * @param err 系统错误码（如errno），可用strerror获取详细错误信息
	 */
	NFException(const std::string &buffer, int err)
	{
		_buffer = buffer + " :" + strerror(err);
		_code = err;
		//    getBacktrace();
	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类的析构函数能够正确调用，使用throw()规范表示不抛出异常。
	 */
	virtual ~NFException() throw()
	{

	}

	/**
	 * @brief 获取错误信息
	 * 
	 * 重写std::exception的what()方法，返回完整的错误描述信息。
	 * 如果构造时提供了错误码，返回的信息会包含strerror()的输出。
	 * 
	 * @return const char* 错误描述的C字符串指针
	 */
	virtual const char* what() const throw()
	{
		return _buffer.c_str();
	}

	/**
	 * @brief 获取错误码
	 * 
	 * 返回构造异常时设置的错误码。如果使用了仅包含消息的构造函数，
	 * 则返回0。
	 * 
	 * @return int 错误码，0表示没有设置错误码
	 */
	int getErrCode() { return _code; }
private:
	/**
	 * @brief 获取调用栈信息（私有方法）
	 * 
	 * 在Linux平台上使用backtrace()系列函数获取当前的调用栈信息，
	 * 并将其附加到错误消息中。这有助于调试和定位问题。
	 * 
	 * @note 目前此功能被注释掉，如需启用可以取消构造函数中的注释
	 * @note 仅在Linux平台上可用
	 */
	void getBacktrace()
	{
#if NF_PLATFORM == NF_PLATFORM_LINUX
		void * array[64];
		int nSize = backtrace(array, 64);
		char ** symbols = backtrace_symbols(array, nSize);

		for (int i = 0; i < nSize; i++)
		{
			_buffer += symbols[i];
			_buffer += "\n";
		}
		free(symbols);
#endif
	}
private:
	/**
	 * @brief 异常的相关信息
	 * 
	 * 存储完整的错误描述信息，包括用户提供的消息和可能的系统错误描述。
	 */
	std::string  _buffer;

	/**
	 * @brief 系统错误码
	 * 
	 * 存储与异常相关的系统错误码（如errno），可以用于进一步的错误处理。
	 */
	int     _code;
};

#endif

