// -------------------------------------------------------------------------
//    @FileName         :    NFEmailSender.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFEmailSender
//    @Desc             :    SMTP邮件发送功能模块头文件，基于libcurl实现。
//                          该文件提供了基于libcurl的SMTP邮件发送功能，包括SMTP邮件发送类定义、
//                          邮件配置和发送接口、附件支持功能、Base64编码解码功能。
//                          主要功能包括支持SMTP服务器连接、支持多收件人、支持邮件附件、
//                          支持HTML格式邮件、支持SSL/TLS安全连接。
//                          邮件发送器是NFShmXFrame框架的网络功能组件，负责：
//                          - SMTP邮件发送功能
//                          - 多收件人支持
//                          - 邮件附件处理
//                          - HTML格式邮件支持
//                          - SSL/TLS安全连接
//                          - Base64编码解码
//                          - 邮件内容格式化
//
// -------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <list>
#include "curl/curl.h"
 
#define SKIP_PEER_VERIFICATION  
#define SKIP_HOSTNAME_VERIFICATION  

/**
 * @brief SMTP邮件发送类，基于libcurl实现邮件发送功能
 * 
 * 该类提供了完整的SMTP邮件发送功能，包括：
 * - 设置SMTP服务器信息
 * - 配置发送者和接收者信息
 * - 设置邮件主题和内容
 * - 支持附件发送
 * - 支持多收件人
 * - 支持HTML格式邮件
 * - 支持SSL/TLS安全连接
 * - 支持Base64编码附件
 * 
 * 使用示例：
 * @code
 * NFSmtpSendMail mailer;
 * mailer.SetSmtpServer("user@example.com", "password", "smtp.example.com", "587");
 * mailer.SetSendMail("sender@example.com");
 * mailer.AddRecvMail("receiver@example.com");
 * mailer.SetSubject("Test Subject");
 * mailer.SetBodyContent("Test Content");
 * mailer.SendMail();
 * @endcode
 */
class NFSmtpSendMail {
public:
	/**
	 * @brief 构造函数
	 * @param charset 邮件字符编码，默认为UTF-8
	 */
	NFSmtpSendMail(const std::string & charset = "UTF-8");

	/**
	 * @brief 设置SMTP服务器信息
	 * @param username SMTP服务器用户名
	 * @param password SMTP服务器密码
	 * @param servername SMTP服务器地址
	 * @param port SMTP服务器端口，默认为25（SMTPS默认为465）
	 */
	void SetSmtpServer(const std::string &username, const std::string& password, const std::string& servername, const std::string &port = "25");
 
	/**
	 * @brief 设置发送者显示名称
	 * @param sendname 发送者显示名称（可选）
	 */
	void SetSendName(const std::string& sendname);

	/**
	 * @brief 设置发送者邮箱地址
	 * @param sendmail 发送者邮箱地址
	 */
	void SetSendMail(const std::string& sendmail);

	/**
	 * @brief 添加收件人邮箱地址
	 * @param recvmail 收件人邮箱地址
	 * @note 可以多次调用此方法添加多个收件人
	 */
	void AddRecvMail(const std::string& recvmail);

	/**
	 * @brief 设置邮件主题
	 * @param subject 邮件主题
	 */
	void SetSubject(const std::string &subject);

	/**
	 * @brief 设置邮件正文内容
	 * @param content 邮件正文内容
	 */
	void SetBodyContent(const std::string &content);

	/**
	 * @brief 添加邮件附件
	 * @param filename 附件文件路径
	 * @note 可以多次调用此方法添加多个附件
	 */
	void AddAttachment(const std::string &filename);

	/**
	 * @brief 发送邮件
	 * @return true 发送成功，false 发送失败
	 */
	bool SendMail();

private:
 
	/**
	 * @brief 回调函数，将MIME协议的拼接的字符串由libcurl发出
	 * @param ptr 数据指针
	 * @param size 数据块大小
	 * @param nmemb 数据块数量
	 * @param stream 流指针
	 * @return 实际写入的数据大小
	 */
	static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *stream);
 
	/**
	 * @brief 创建邮件MIME内容
	 * 
	 * 构建完整的MIME格式邮件内容，包括：
	 * - 邮件头信息
	 * - 正文内容
	 * - 附件内容（如果有）
	 */
	void CreatMessage();
 
	/**
	 * @brief 获取文件类型
	 * @param stype 文件扩展名
	 * @return 文件类型索引
	 */
	int GetFileType(std::string const& stype);
 
	/**
	 * @brief 设置文件名
	 * @param FileName 文件名
	 */
	void SetFileName(const std::string& FileName);
 
	/**
	 * @brief 设置文件的contenttype
	 * @param stype 文件类型
	 */
	void SetContentType(std::string const& stype);
 
	/**
	 * @brief 从文件路径中提取文件名
	 * @param file 文件路径
	 * @param filename 提取的文件名（输出）
	 */
	void GetFileName(const std::string& file, std::string& filename);
 
	/**
	 * @brief 从文件路径中提取文件类型
	 * @param file 文件路径
	 * @param stype 提取的文件类型（输出）
	 */
	void GetFileType(const std::string& file, std::string& stype);
 
private:
	std::string m_strCharset; ///< 邮件编码
	std::string m_strSubject; ///< 邮件主题
	std::string m_strContent; ///< 邮件内容
	std::string m_strFileName; ///< 文件名
	std::string m_strMessage; ///< 整个MIME协议字符串
	std::string m_strUserName; ///< 用户名
	std::string m_strPassword; ///< 密码
	std::string m_strServerName; ///< smtp服务器
	std::string m_strPort; ///< 端口
	std::string m_strSendName; ///< 发送者姓名
	std::string m_strSendMail; ///< 发送者邮箱
	std::string m_strContentType; ///< 附件contenttype
	std::string m_strFileContent; ///< 附件内容
 
	std::vector<std::string> m_vRecvMail; ///< 收件人容器
	std::vector<std::string> m_vAttachMent; ///< 附件容器
};