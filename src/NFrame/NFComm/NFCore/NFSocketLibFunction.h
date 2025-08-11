// -------------------------------------------------------------------------
//    @FileName         :    NFSocketLibFunction.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFSocketLibFunction.h
 * @brief 跨平台套接字库函数封装
 * 
 * 此文件提供了跨平台的套接字操作封装，统一了Windows和Linux下的
 * 套接字API差异，提供了一致的接口用于网络编程。
 */

#pragma once

#include "NFPlatform.h"

/**
 * @name Windows平台网络头文件
 * @brief Windows平台所需的网络相关头文件
 * @{
 */
#if NF_PLATFORM == NF_PLATFORM_WIN
#include <winsock2.h>
#include <WinError.h>
#include <winsock.h>
#include <Ws2tcpip.h>
#include <errno.h>

#else
/**
 * @name Linux平台网络头文件
 * @brief Linux平台所需的网络相关头文件
 * @{
 */
#include <signal.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <sys/uio.h>
/** @} */
#endif

/**
 * @name 跨平台套接字类型和错误码定义
 * @brief 统一Windows和Linux的套接字类型和错误码
 * @{
 */
#if NF_PLATFORM == NF_PLATFORM_WIN
/** @brief Windows平台的套接字文件描述符类型 */
typedef SOCKET NFSocketFD;
/** @brief 获取最后错误码的宏（Windows版本） */
#define NF_ERRNO WSAGetLastError()
/** @brief 非套接字错误码（Windows版本） */
#define NF_ENOTSOCK WSAENOTSOCK
/** @brief 非阻塞操作会阻塞错误码（Windows版本） */
#define NF_EWOULDBLOCK WSAEWOULDBLOCK
/** @brief 中断错误码（Windows版本） */
#define NF_EINTR WSAEINTR
/** @brief 连接中止错误码（Windows版本） */
#define NF_ECONNABORTED WSAECONNABORTED
/** @brief 套接字错误返回值（Windows版本） */
#define NF_SOCKET_ERROR SOCKET_ERROR
/** @brief 无效套接字值（Windows版本） */
#define NF_INVALID_SOCKET INVALID_SOCKET

#else
/** @brief Linux平台的套接字文件描述符类型 */
typedef int NFSocketFD;
/** @brief 获取最后错误码的宏（Linux版本） */
#define NF_ERRNO errno
/** @brief 非套接字错误码（Linux版本） */
#define NF_ENOTSOCK EBADF
/** @brief 非阻塞操作会阻塞错误码（Linux版本） */
#define NF_EWOULDBLOCK EAGAIN
/** @brief 中断错误码（Linux版本） */
#define NF_EINTR EINTR
/** @brief 连接中止错误码（Linux版本） */
#define NF_ECONNABORTED ECONNABORTED
/** @brief 套接字错误返回值（Linux版本） */
#define NF_SOCKET_ERROR (-1)
/** @brief 无效套接字值（Linux版本） */
#define NF_INVALID_SOCKET (-1)
#endif
/** @} */

/**
 * @brief 套接字库函数工具类
 * 
 * NFSocketLibFunction提供了跨平台的套接字操作函数，封装了
 * Windows和Linux下套接字API的差异，提供统一的接口。
 * 
 * 主要功能：
 * - 套接字库初始化和清理
 * - 套接字选项设置（如TCP_NODELAY、SO_REUSEADDR等）
 * - 套接字状态设置（阻塞/非阻塞）
 * - 套接字信息查询
 * - 跨平台的套接字操作封装
 * 
 * 使用方法：
 * @code
 * // 初始化套接字库
 * if (!NFSocketLibFunction::InitSocket()) {
 *     // 初始化失败处理
 * }
 * 
 * // 创建套接字并设置选项
 * NFSocketFD fd = socket(AF_INET, SOCK_STREAM, 0);
 * NFSocketLibFunction::SocketNodelay(fd);
 * NFSocketLibFunction::SocketNonBlocking(fd);
 * 
 * // 清理套接字库
 * NFSocketLibFunction::DestroySocket();
 * @endcode
 */
class NFSocketLibFunction
{
public:
	/**
	 * @brief 初始化套接字库
	 * 
	 * 在Windows平台上初始化Winsock库，在Linux平台上设置信号处理。
	 * 必须在使用任何套接字操作之前调用。
	 * 
	 * Windows平台：
	 * - 初始化Winsock 2.2版本
	 * - 设置静态标志避免重复初始化
	 * 
	 * Linux平台：
	 * - 忽略SIGPIPE信号，避免在写入已关闭套接字时程序终止
	 * 
	 * @return bool 初始化成功返回true，失败返回false
	 */
	static bool InitSocket()
	{
		bool ret = true;
#if NF_PLATFORM == NF_PLATFORM_WIN
		static WSADATA g_WSAData;
		static bool WinSockIsInit = false;
		if (WinSockIsInit)
		{
			return true;
		}
		if (WSAStartup(MAKEWORD(2, 2), &g_WSAData) == 0)
		{
			WinSockIsInit = true;
		}
		else
		{
			ret = false;
		}
#else
		signal(SIGPIPE, SIG_IGN);
#endif

		return ret;
	}

	/**
	 * @brief 清理套接字库
	 * 
	 * 清理套接字库资源，在程序退出前调用。
	 * 在Windows平台上调用WSACleanup()，Linux平台上无操作。
	 */
	static void DestroySocket()
	{
#if NF_PLATFORM == NF_PLATFORM_WIN
		WSACleanup();
#endif
	}

	/**
	 * @brief 设置套接字TCP_NODELAY选项
	 * 
	 * 启用TCP_NODELAY选项，禁用Nagle算法，使小数据包能够立即发送，
	 * 减少延迟。适用于对实时性要求高的应用。
	 * 
	 * @param fd 套接字文件描述符
	 * @return int 设置成功返回0，失败返回-1
	 */
	static int  SocketNodelay(NFSocketFD fd)
	{
		const int flag = 1;
		return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));
	}

	/**
	 * @brief 设置套接字为阻塞模式
	 * 
	 * 将套接字设置为阻塞模式，套接字操作（如recv、send等）会一直等待
	 * 直到操作完成或发生错误。
	 * 
	 * @param fd 套接字文件描述符
	 * @return bool 设置成功返回true，失败返回false
	 * 
	 * @note 阻塞模式是套接字的默认模式
	 * @note 适用于简单的同步网络编程
	 */
	static bool SocketBlock(NFSocketFD fd)
	{
		int err;
		unsigned long ul = false;
#if NF_PLATFORM == NF_PLATFORM_WIN
		err = ioctlsocket(fd, FIONBIO, &ul);
#else
		err = ioctl(fd, FIONBIO, &ul);
#endif

		return err != NF_SOCKET_ERROR;
	}

	/**
	 * @brief 设置套接字为非阻塞模式
	 * 
	 * 将套接字设置为非阻塞模式，套接字操作会立即返回，
	 * 如果操作无法立即完成则返回错误码（如EAGAIN/EWOULDBLOCK）。
	 * 
	 * @param fd 套接字文件描述符
	 * @return bool 设置成功返回true，失败返回false
	 * 
	 * @note 非阻塞模式适用于高性能的异步网络编程
	 * @note 需要配合select/epoll等多路复用机制使用
	 * @note 大多数网络库和框架使用非阻塞模式
	 */
	static bool SocketNonblock(NFSocketFD fd)
	{
		int err;
		unsigned long ul = true;
#if NF_PLATFORM == NF_PLATFORM_WIN
		err = ioctlsocket(fd, FIONBIO, &ul);
#else
		err = ioctl(fd, FIONBIO, &ul);
#endif

		return err != NF_SOCKET_ERROR;
	}

	/**
	 * @brief 设置套接字发送缓冲区大小
	 * 
	 * 设置套接字的发送缓冲区（SO_SNDBUF）大小，影响发送性能和内存使用。
	 * 
	 * @param fd 套接字文件描述符
	 * @param sd_size 发送缓冲区大小（字节）
	 * @return int 设置成功返回0，失败返回-1
	 * 
	 * @note 较大的缓冲区可以提高批量发送的性能
	 * @note 过大的缓冲区会增加内存使用和延迟
	 * @note 系统可能会调整实际的缓冲区大小
	 */
	static int  SocketSetSendSize(NFSocketFD fd, int sd_size)
	{
		return ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&sd_size, sizeof(sd_size));
	}

	/**
	 * @brief 设置套接字接收缓冲区大小
	 * 
	 * 设置套接字的接收缓冲区（SO_RCVBUF）大小，影响接收性能和内存使用。
	 * 
	 * @param fd 套接字文件描述符
	 * @param rd_size 接收缓冲区大小（字节）
	 * @return int 设置成功返回0，失败返回-1
	 * 
	 * @note 较大的缓冲区可以减少数据丢失的可能性
	 * @note 过大的缓冲区会增加内存使用
	 * @note 对于高带宽网络，适当增大缓冲区有助于性能
	 */
	static int  SocketSetRecvSize(NFSocketFD fd, int rd_size)
	{
		return ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&rd_size, sizeof(rd_size));
	}

	/**
	 * @brief 创建套接字
	 * 
	 * 创建一个新的套接字，这是所有网络操作的基础。
	 * 
	 * @param af 地址族（如AF_INET表示IPv4，AF_INET6表示IPv6）
	 * @param type 套接字类型（如SOCK_STREAM表示TCP，SOCK_DGRAM表示UDP）
	 * @param protocol 协议类型（通常为0，表示使用默认协议）
	 * @return NFSocketFD 成功返回套接字描述符，失败返回NF_INVALID_SOCKET
	 * 
	 * 常用参数组合：
	 * - TCP套接字：af=AF_INET, type=SOCK_STREAM, protocol=0
	 * - UDP套接字：af=AF_INET, type=SOCK_DGRAM, protocol=0
	 * - IPv6 TCP：af=AF_INET6, type=SOCK_STREAM, protocol=0
	 */
	static NFSocketFD SocketCreate(int af, int type, int protocol)
	{
		return ::socket(af, type, protocol);
	}

	/**
	 * @brief 关闭套接字
	 * 
	 * 关闭指定的套接字，释放相关资源。关闭后的套接字描述符不能再使用。
	 * 
	 * @param fd 要关闭的套接字文件描述符
	 * 
	 * @note Windows平台使用closesocket()，Linux平台使用close()
	 * @note 关闭套接字会中断所有相关的网络操作
	 * @note 应该确保所有引用该套接字的操作都已完成
	 */
	static void SocketClose(NFSocketFD fd)
	{
#if NF_PLATFORM == NF_PLATFORM_WIN
		::closesocket(fd);
#else
		::close(fd);
#endif
	}

	/**
	 * @brief 连接到远程服务器
	 * 
	 * 创建客户端套接字并连接到指定的服务器地址和端口。
	 * 支持IPv4和IPv6两种协议。
	 * 
	 * @param isIPV6 是否使用IPv6协议，true表示IPv6，false表示IPv4
	 * @param server_ip 服务器IP地址字符串
	 * @param port 服务器端口号
	 * @return NFSocketFD 成功返回连接的套接字描述符，失败返回NF_INVALID_SOCKET
	 * 
	 * @note 此方法会自动调用InitSocket()初始化套接字库
	 * @note 连接失败时会自动关闭创建的套接字
	 * @note 支持IPv4和IPv6地址格式
	 * @note 遇到EINTR错误时会自动重试
	 * 
	 * 使用示例：
	 * @code
	 * // 连接IPv4服务器
	 * NFSocketFD fd = NFSocketLibFunction::Connect(false, "192.168.1.100", 8080);
	 * 
	 * // 连接IPv6服务器
	 * NFSocketFD fd6 = NFSocketLibFunction::Connect(true, "::1", 8080);
	 * @endcode
	 */
	static NFSocketFD Connect(bool isIPV6, const std::string& server_ip, int port)
	{
		InitSocket();

		struct sockaddr_in ip4Addr = sockaddr_in();
		struct sockaddr_in6 ip6Addr = sockaddr_in6();
		struct sockaddr_in* paddr = &ip4Addr;
		int addrLen = sizeof(ip4Addr);

		NFSocketFD clientfd = isIPV6 ?
			SocketCreate(AF_INET6, SOCK_STREAM, 0) :
			SocketCreate(AF_INET, SOCK_STREAM, 0);

		if (clientfd == NF_INVALID_SOCKET)
		{
			return clientfd; //-V1020
		}

		bool ptonResult = false;
		if (isIPV6)
		{
			ip6Addr.sin6_family = AF_INET6;
			ip6Addr.sin6_port = htons(port);
			ptonResult = inet_pton(AF_INET6,
				server_ip.c_str(),
				&ip6Addr.sin6_addr) > 0;
			paddr = (struct sockaddr_in*) & ip6Addr; //-V641
			addrLen = sizeof(ip6Addr);
		}
		else
		{
			ip4Addr.sin_family = AF_INET;
			ip4Addr.sin_port = htons(port);
			ptonResult = inet_pton(AF_INET,
				server_ip.c_str(),
				&ip4Addr.sin_addr) > 0;
		}

		if (!ptonResult)
		{
			SocketClose(clientfd);
			return NF_INVALID_SOCKET;
		}

		while (::connect(clientfd, (struct sockaddr*)paddr, addrLen) < 0)
		{
			if (EINTR == NF_ERRNO)
			{
				continue;
			}

			SocketClose(clientfd);
			return NF_INVALID_SOCKET;
		}

		return clientfd; //-V1020
	}

	/**
	 * @brief 创建监听套接字
	 * 
	 * 创建服务端监听套接字，绑定到指定地址和端口，并开始监听连接。
	 * 支持IPv4和IPv6两种协议。
	 * 
	 * @param isIPV6 是否使用IPv6协议，true表示IPv6，false表示IPv4
	 * @param ip 绑定的IP地址字符串，NULL或"0.0.0.0"表示绑定所有接口
	 * @param port 监听的端口号
	 * @param back_num 监听队列的最大长度
	 * @return NFSocketFD 成功返回监听套接字描述符，失败返回NF_INVALID_SOCKET
	 * 
	 * @note 此方法会自动调用InitSocket()初始化套接字库
	 * @note 自动设置SO_REUSEADDR选项，允许地址重用
	 * @note 创建失败时会自动关闭套接字
	 * @note back_num建议设置为128或更大的值
	 * 
	 * 使用示例：
	 * @code
	 * // 监听IPv4所有接口的8080端口
	 * NFSocketFD listener = NFSocketLibFunction::Listen(false, "0.0.0.0", 8080, 128);
	 * 
	 * // 监听IPv6特定地址
	 * NFSocketFD listener6 = NFSocketLibFunction::Listen(true, "::1", 8080, 128);
	 * @endcode
	 */
	static NFSocketFD Listen(bool isIPV6, const char* ip, int port, int back_num)
	{
		InitSocket();

		struct sockaddr_in ip4Addr = sockaddr_in();
		struct sockaddr_in6 ip6Addr = sockaddr_in6();
		struct sockaddr_in* paddr = &ip4Addr;
		int addrLen = sizeof(ip4Addr);

		const auto socketfd = isIPV6 ?
			socket(AF_INET6, SOCK_STREAM, 0) :
			socket(AF_INET, SOCK_STREAM, 0);
		if (socketfd == NF_INVALID_SOCKET)
		{
			return NF_INVALID_SOCKET;
		}

		bool ptonResult = false;
		if (isIPV6)
		{
			ip6Addr.sin6_family = AF_INET6;
			ip6Addr.sin6_port = htons(port);
			ptonResult = inet_pton(AF_INET6, ip, &ip6Addr.sin6_addr) > 0;
			paddr = (struct sockaddr_in*) & ip6Addr; //-V641
			addrLen = sizeof(ip6Addr);
		}
		else
		{
			ip4Addr.sin_family = AF_INET;
			ip4Addr.sin_port = htons(port);
			ip4Addr.sin_addr.s_addr = INADDR_ANY;
			ptonResult = inet_pton(AF_INET, ip, &ip4Addr.sin_addr) > 0;
		}

		const int reuseaddr_value = 1;
		if (!ptonResult ||
			::setsockopt(socketfd,
				SOL_SOCKET,
				SO_REUSEADDR,
				(const char*)&reuseaddr_value,
				sizeof(int)) < 0)
		{
			SocketClose(socketfd);
			return NF_INVALID_SOCKET;
		}

		const int bindRet = ::bind(socketfd, (struct sockaddr*)paddr, addrLen);
		if (bindRet == NF_SOCKET_ERROR ||
			listen(socketfd, back_num) == NF_SOCKET_ERROR)
		{
			SocketClose(socketfd);
			return NF_INVALID_SOCKET;
		}

		return socketfd;
	}

	/**
	 * @brief 将套接字地址转换为IP字符串
	 * 
	 * 将sockaddr结构中的IP地址转换为可读的字符串格式。
	 * 支持IPv4和IPv6地址格式。
	 * 
	 * @param sa 指向sockaddr结构的指针
	 * @return std::string IP地址字符串，失败时返回"Unknown AF"
	 * 
	 * @note 支持AF_INET（IPv4）和AF_INET6（IPv6）地址族
	 * @note 使用inet_ntop函数进行地址转换
	 * @note 返回的字符串最大长度为INET6_ADDRSTRLEN
	 */
	static std::string getIPString(const struct sockaddr* sa)
	{
#if NF_PLATFORM == NF_PLATFORM_WIN
		using PAddrType = PVOID;
#else
		using PAddrType = const void*;
#endif
		char tmp[INET6_ADDRSTRLEN] = { 0 };
		switch (sa->sa_family)
		{
		case AF_INET:
			inet_ntop(AF_INET, (PAddrType)(&(((const struct sockaddr_in*)sa)->sin_addr)),
				tmp, sizeof(tmp));
			break;
		case AF_INET6:
			inet_ntop(AF_INET6, (PAddrType)(&(((const struct sockaddr_in6*)sa)->sin6_addr)),
				tmp, sizeof(tmp));
			break;
		default:
			return "Unknown AF";
		}

		return tmp;
	}

	/**
	 * @brief 获取套接字对端的IP地址
	 * 
	 * 获取连接的对端（远程端）的IP地址字符串。
	 * 适用于已建立连接的TCP套接字。
	 * 
	 * @param fd 套接字文件描述符
	 * @return std::string 对端IP地址字符串，失败时返回空字符串
	 * 
	 * @note 只能用于已连接的套接字
	 * @note Windows和Linux平台使用不同的sockaddr结构
	 * @note 失败时可能是由于套接字未连接或已关闭
	 * 
	 * 使用示例：
	 * @code
	 * NFSocketFD clientFd = accept(listenFd, NULL, NULL);
	 * std::string clientIP = NFSocketLibFunction::GetIPOfSocket(clientFd);
	 * std::cout << "Client IP: " << clientIP << std::endl;
	 * @endcode
	 */
	static std::string GetIPOfSocket(NFSocketFD fd)
	{
#if NF_PLATFORM == NF_PLATFORM_WIN
		struct sockaddr name = sockaddr();
		int namelen = sizeof(name);
		if (::getpeername(fd, (struct sockaddr*) & name, &namelen) == 0)
		{
			return getIPString(&name);
		}
#else
		struct sockaddr_in name = sockaddr_in();
		socklen_t namelen = sizeof(name);
		if (::getpeername(fd, (struct sockaddr*) & name, &namelen) == 0)
		{
			return getIPString((const struct sockaddr*) & name);
		}
#endif

		return "";
	}

	/**
	 * @brief 通过套接字发送数据
	 * 
	 * 向套接字发送指定长度的数据。
	 * 自动处理EWOULDBLOCK错误（非阻塞模式下的正常情况）。
	 * 
	 * @param fd 套接字文件描述符
	 * @param buffer 要发送的数据缓冲区
	 * @param len 要发送的数据长度（字节）
	 * @return int 实际发送的字节数，错误时返回负值，EWOULDBLOCK时返回0
	 * 
	 * @note 非阻塞模式下，EWOULDBLOCK不视为错误，返回0
	 * @note 返回值小于len不一定表示错误，可能需要多次调用
	 * @note 调用者应检查返回值处理部分发送的情况
	 * 
	 * 使用示例：
	 * @code
	 * const char* data = "Hello, World!";
	 * int sent = NFSocketLibFunction::SocketSend(fd, data, strlen(data));
	 * if (sent < 0) {
	 *     // 发送错误
	 * } else if (sent == 0) {
	 *     // 非阻塞模式下暂时无法发送
	 * } else {
	 *     // 成功发送 sent 字节
	 * }
	 * @endcode
	 */
	static int SocketSend(NFSocketFD fd, const char* buffer, int len)
	{
		int transnum = ::send(fd, buffer, len, 0);
		if (transnum < 0 && NF_EWOULDBLOCK == NF_ERRNO)
		{
			transnum = 0;
		}

		/*  send error if transnum < 0  */
		return transnum;
	}

	/**
	 * @brief 接受新的连接
	 * 
	 * 从监听套接字接受一个新的连接请求。
	 * 这是服务端处理客户端连接的标准方法。
	 * 
	 * @param listenSocket 监听套接字描述符
	 * @param addr [out] 可选参数，用于接收客户端地址信息
	 * @param addrLen [in,out] 可选参数，地址结构的长度
	 * @return NFSocketFD 新连接的套接字描述符，失败返回NF_INVALID_SOCKET
	 * 
	 * @note 如果不需要客户端地址信息，addr和addrLen可以传NULL
	 * @note 在非阻塞模式下，如果没有连接请求会立即返回错误
	 * @note 返回的套接字继承监听套接字的某些属性
	 * 
	 * 使用示例：
	 * @code
	 * struct sockaddr_in clientAddr;
	 * socklen_t addrLen = sizeof(clientAddr);
	 * NFSocketFD clientFd = NFSocketLibFunction::Accept(listenFd, 
	 *     (struct sockaddr*)&clientAddr, &addrLen);
	 * if (clientFd != NF_INVALID_SOCKET) {
	 *     // 处理新连接
	 * }
	 * @endcode
	 */
	static NFSocketFD Accept(NFSocketFD listenSocket, struct sockaddr* addr, socklen_t* addrLen)
	{
		return ::accept(listenSocket, addr, addrLen);
	}

	/**
	 * @brief 获取套接字对端地址信息
	 * 
	 * 获取连接套接字的对端（远程端）地址信息。
	 * 返回IPv6格式的地址结构，兼容IPv4地址。
	 * 
	 * @param sockfd 套接字文件描述符
	 * @return struct sockaddr_in6 对端地址结构，失败时返回初始化的结构
	 * 
	 * @note 使用IPv6结构以兼容IPv4和IPv6地址
	 * @note 失败时返回的结构中的字段为初始值
	 * @note 只能用于已连接的套接字
	 */
	static struct sockaddr_in6 getPeerAddr(NFSocketFD sockfd)
	{
		struct sockaddr_in6 peeraddr = sockaddr_in6();
		socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
		if (::getpeername(sockfd, (struct sockaddr*)(&peeraddr), &addrlen) < 0) //-V641
		{
			return peeraddr;
		}
		return peeraddr;
	}

	/**
	 * @brief 获取套接字本地地址信息
	 * 
	 * 获取套接字绑定的本地地址信息。
	 * 返回IPv6格式的地址结构，兼容IPv4地址。
	 * 
	 * @param sockfd 套接字文件描述符
	 * @return struct sockaddr_in6 本地地址结构，失败时返回初始化的结构
	 * 
	 * @note 使用IPv6结构以兼容IPv4和IPv6地址
	 * @note 失败时返回的结构中的字段为初始值
	 * @note 可用于已绑定或已连接的套接字
	 */
	static struct sockaddr_in6 getLocalAddr(NFSocketFD sockfd)
	{
		struct sockaddr_in6 localaddr = sockaddr_in6();
		socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
		if (::getsockname(sockfd, (struct sockaddr*)(&localaddr), &addrlen) < 0) //-V641
		{
			return localaddr;
		}
		return localaddr;
	}

	/**
	 * @brief 检查是否为自连接
	 * 
	 * 检查套接字是否连接到自己（本地地址和对端地址相同）。
	 * 自连接通常是程序错误，应该避免。
	 * 
	 * @param fd 套接字文件描述符
	 * @return bool 如果是自连接返回true，否则返回false
	 * 
	 * @note 同时比较IP地址和端口号
	 * @note 支持IPv4和IPv6地址检查
	 * @note Windows和Linux平台对IPv6地址使用不同的结构成员
	 * @note 自连接可能导致程序逻辑错误，建议检查并处理
	 * 
	 * 使用示例：
	 * @code
	 * if (NFSocketLibFunction::IsSelfConnect(clientFd)) {
	 *     std::cerr << "Warning: Self-connection detected!" << std::endl;
	 *     NFSocketLibFunction::SocketClose(clientFd);
	 * }
	 * @endcode
	 */
	static bool IsSelfConnect(NFSocketFD fd)
	{
		struct sockaddr_in6 localaddr = getLocalAddr(fd);
		struct sockaddr_in6 peeraddr = getPeerAddr(fd);

		if (localaddr.sin6_family == AF_INET)
		{
			const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr); //-V641
			const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr); //-V641
			return laddr4->sin_port == raddr4->sin_port
				&& laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
		}
		else if (localaddr.sin6_family == AF_INET6)
		{
#if NF_PLATFORM == NF_PLATFORM_WIN
			return localaddr.sin6_port == peeraddr.sin6_port
				&& memcmp(&localaddr.sin6_addr.u.Byte,
					&peeraddr.sin6_addr.u.Byte,
					sizeof localaddr.sin6_addr.u.Byte) == 0;
#else
			return localaddr.sin6_port == peeraddr.sin6_port
				&& memcmp(&localaddr.sin6_addr.s6_addr,
					&peeraddr.sin6_addr.s6_addr,
					sizeof localaddr.sin6_addr.s6_addr) == 0;
#endif
		}
		else
		{
			return false;
		}
	}

	/**
	 * @brief 将地址字符串转换为总线ID
	 * 
	 * 将IP地址字符串转换为网络字节序的32位整数（总线ID）。
	 * 主要用于内部地址标识和路由。
	 * 
	 * @param strAddr IP地址字符串
	 * @param busid [out] 输出的总线ID（网络字节序）
	 * @return int 成功返回转换后的地址值，失败返回-1
	 * 
	 * @note 使用inet_addr函数进行地址转换
	 * @note 返回的是网络字节序的地址
	 * @note 主要用于IPv4地址转换
	 * @note 无效的地址格式会导致转换失败
	 * 
	 * 使用示例：
	 * @code
	 * uint32_t busId;
	 * int result = NFSocketLibFunction::BusAddrAton("192.168.1.100", busId);
	 * if (result >= 0) {
	 *     // 转换成功，busId包含网络字节序的地址
	 * }
	 * @endcode
	 */
	static int BusAddrAton(const std::string& strAddr, uint32_t& busid)
	{
		int iRet = 0;

		iRet = inet_addr(strAddr.c_str());
		if (iRet < 0) {
			return -1;
		}

		busid = iRet;
		return iRet;
	}
};
