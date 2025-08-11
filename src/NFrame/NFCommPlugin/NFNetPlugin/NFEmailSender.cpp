// -------------------------------------------------------------------------
//    @FileName         :    NFEmailSender.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFEmailSender
//    @Desc             :    SMTP邮件发送功能实现，基于libcurl实现邮件发送
//
// -------------------------------------------------------------------------

#include "NFEmailSender.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFPluginModule/NFCheck.h"

#include <iostream>
#include <sstream>
#include <fstream>

/**
 * @file NFEmailSender.cpp
 * @brief SMTP邮件发送功能实现文件
 * 
 * 该文件实现了基于libcurl的SMTP邮件发送功能，包括：
 * - Base64编码解码功能
 * - SMTP邮件发送类实现
 * - 邮件MIME格式构建
 * - 附件处理功能
 * 
 * 主要功能：
 * - 支持SMTP服务器连接
 * - 支持多收件人
 * - 支持邮件附件
 * - 支持HTML格式邮件
 * - 支持SSL/TLS安全连接
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief Base64编码字符表
 * 
 * 用于Base64编码的标准字符集，包含64个可打印字符
 */
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

/**
 * @brief 检查字符是否为Base64编码字符
 * 
 * @param c 要检查的字符
 * @return true 是Base64字符，false 不是
 */
static inline bool is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

/**
 * @brief Base64编码函数
 * 
 * 将二进制数据编码为Base64字符串，用于邮件附件传输
 * 
 * @param bytes_to_encode 要编码的字节数组
 * @param in_len 字节数组长度
 * @return Base64编码后的字符串
 */
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
	std::string ret;
	int i = 0, j = 0;
	unsigned char char_array_3[3], char_array_4[4];

	// 每3个字节编码为4个Base64字符
	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			// 执行Base64编码转换
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			// 转换为Base64字符
			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	// 处理剩余的字节
	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		// 添加填充字符
		while ((i++ < 3))
			ret += '=';

	}

	return std::move(ret);

}

/**
 * @brief Base64解码函数
 * 
 * 将Base64字符串解码为二进制数据
 * 
 * @param encoded_string Base64编码的字符串
 * @return 解码后的二进制数据字符串
 */
std::string base64_decode(std::string const& encoded_string)
{
	int in_len = encoded_string.size();
	int i = 0, j = 0, in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	// 每4个Base64字符解码为3个字节
	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			// 查找Base64字符对应的索引
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			// 执行Base64解码转换
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			// 添加解码后的字节
			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	// 处理剩余的Base64字符
	if (i)
	{
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret += char_array_3[j];
	}

	return std::move(ret);
}
 
/**
 * @brief SMTP邮件发送类构造函数
 * 
 * 初始化邮件发送器，包括：
 * - 初始化libcurl库
 * - 设置字符编码
 * - 清空收件人和附件列表
 * 
 * @param charset 邮件字符编码，默认为UTF-8
 */
NFSmtpSendMail::NFSmtpSendMail(const std::string & charset)
{
    static bool curInit = false;
    if (!curInit)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        curInit = true;
    }
    m_strCharset = charset;
    m_vRecvMail.clear();
}
 
/**
 * @brief 设置SMTP服务器信息
 * 
 * 配置SMTP服务器的连接参数
 * 
 * @param username SMTP服务器用户名
 * @param password SMTP服务器密码
 * @param servername SMTP服务器地址
 * @param port SMTP服务器端口
 */
void NFSmtpSendMail::SetSmtpServer(const std::string & username, const std::string &password, const std::string & servername, const std::string & port)
{
    m_strUserName = username;
    m_strPassword = password;
    m_strServerName = servername;
    m_strPort = port;
}
 
/**
 * @brief 设置发送者显示名称
 * 
 * 将发送者名称进行Base64编码，支持中文显示
 * 
 * @param sendname 发送者显示名称
 */
void NFSmtpSendMail::SetSendName(const std::string & sendname)
{
    std::string strTemp = "";
    strTemp += "=?";
    strTemp += m_strCharset;
    strTemp += "?B?";
    strTemp += base64_encode((unsigned char *)sendname.c_str(), sendname.size());//NFBase64::Encode(sendname);//
    strTemp += "?=";
    m_strSendName = strTemp;
    //m_strSendName = sendname;
}
 
/**
 * @brief 设置发送者邮箱地址
 * 
 * @param sendmail 发送者邮箱地址
 */
void NFSmtpSendMail::SetSendMail(const std::string & sendmail)
{
    m_strSendMail = sendmail;
}
 
/**
 * @brief 添加收件人邮箱地址
 * 
 * 可以多次调用此方法添加多个收件人
 * 
 * @param recvmail 收件人邮箱地址
 */
void NFSmtpSendMail::AddRecvMail(const std::string & recvmail)
{
    m_vRecvMail.push_back(recvmail);
}
 
/**
 * @brief 设置邮件主题
 * 
 * 将邮件主题进行Base64编码，支持中文主题
 * 
 * @param subject 邮件主题
 */
void NFSmtpSendMail::SetSubject(const std::string & subject)
{
    std::string strTemp = "";
    strTemp = "Subject: ";
    strTemp += "=?";
    strTemp += m_strCharset;
    strTemp += "?B?";
    strTemp += base64_encode((unsigned char *)subject.c_str(), subject.size());//NFBase64::Encode(subject);//
    strTemp += "?=";
    m_strSubject = strTemp;
}
 
/**
 * @brief 设置邮件正文内容
 * 
 * @param content 邮件正文内容
 */
void NFSmtpSendMail::SetBodyContent(const std::string & content)
{
    m_strContent = content;
}
 
/**
 * @brief 添加邮件附件
 * 
 * 可以多次调用此方法添加多个附件
 * 
 * @param filename 附件文件路径
 */
void NFSmtpSendMail::AddAttachment(const std::string & filename)
{
    m_vAttachMent.push_back(filename);
}
 
/**
 * @brief 发送邮件
 * 
 * 使用libcurl发送SMTP邮件，包括：
 * - 构建MIME格式邮件内容
 * - 配置SMTP连接参数
 * - 发送邮件数据
 * - 处理发送结果
 * 
 * @return true 发送成功，false 发送失败
 */
bool NFSmtpSendMail::SendMail()
{
    CreatMessage();
    bool ret = true;
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    curl = curl_easy_init();
    if (curl)
    {
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, m_strUserName.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, m_strPassword.c_str());
        std::string tmp = "smtps://";
        tmp += m_strServerName;
        tmp += ":";
        tmp += m_strPort;
        // 注意不能直接传入tmp，应该带上.c_str()，否则会导致下面的
        // curl_easy_perform调用返回CURLE_COULDNT_RESOLVE_HOST错误
        // 码
        curl_easy_setopt(curl, CURLOPT_URL, tmp.c_str());
        /* If you want to connect to a site who isn't using a certificate that is
        * signed by one of the certs in the CA bundle you have, you can skip the
        * verification of the server's certificate. This makes the connection
        * A LOT LESS SECURE.
        *
        * If you have a CA cert for the server stored someplace else than in the
        * default bundle, then the CURLOPT_CAPATH option might come handy for
        * you. */
#ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
        /* If the site you're connecting to uses a different host name that what
        * they have mentioned in their server certificate's commonName (or
        * subjectAltName) fields, libcurl will refuse to connect. You can skip
        * this check, but this will make the connection less secure. */
#ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
        /* Note that this option isn't strictly required, omitting it will result
        * in libcurl sending the MAIL FROM command with empty sender data. All
        * autoresponses should have an empty reverse-path, and should be directed
        * to the address in the reverse-path which triggered them. Otherwise,
        * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
        * details.
        */
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, m_strSendMail.c_str());
        /* Add two recipients, in this particular case they correspond to the
        * To: and Cc: addressees in the header, but they could be any kind of
        * recipient. */
        for (size_t i = 0; i < m_vRecvMail.size(); i++)
        {
            recipients = curl_slist_append(recipients, m_vRecvMail[i].c_str());
        }
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        std::stringstream stream;
        stream.str(m_strMessage.c_str());
        stream.flush();
        /* We're using a callback function to specify the payload (the headers and
        * body of the message). You could just use the CURLOPT_READDATA option to
        * specify a FILE pointer to read from. */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, &NFSmtpSendMail::payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&stream);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        /* Since the traffic will be encrypted, it is very useful to turn on debug
        * information within libcurl to see what is happening during the
        * transfer */
        int nTimes = 0;
        /* Send the message */
        res = curl_easy_perform(curl);
        CURLINFO info = CURLINFO_NONE;
        long http_version = 0;
        curl_easy_getinfo(curl, info, &http_version);
        /* Check for errors */
        while (res != CURLE_OK)
        {
            nTimes++;
            if (nTimes > 5)
            {
                break;
            }
            fprintf(stderr, "curl_easy_perform() failed: %s\n\n", curl_easy_strerror(res));
            ret = false;
            /*				Sleep( 100 );
            res = curl_easy_perform(curl); */
        }
        /* Free the list of recipients */
        curl_slist_free_all(recipients);
        /* Always cleanup */
        curl_easy_cleanup(curl);
    }
    return ret;
}
 
/**
 * @brief libcurl数据源回调函数
 * 
 * 为libcurl提供邮件数据源，将MIME格式的邮件内容传递给libcurl
 * 
 * @param ptr 数据指针
 * @param size 数据块大小
 * @param nmemb 数据块数量
 * @param stream 流指针
 * @return 实际写入的数据大小
 */
size_t NFSmtpSendMail::payload_source(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t num_bytes = size * nmemb;
    char* data = (char*)ptr;
    std::stringstream* strstream = (std::stringstream*)stream;
    strstream->read(data, num_bytes);
    return strstream->gcount();
}
 
/**
 * @brief 创建邮件MIME内容
 * 
 * 构建完整的MIME格式邮件内容，包括：
 * - 邮件头信息（From, To, Subject等）
 * - 正文内容
 * - 附件内容（如果有）
 */
void NFSmtpSendMail::CreatMessage()
{
    m_strMessage = "From: ";
    m_strMessage += m_strSendName + "<" + m_strSendMail + ">"/*m_strSendMail*/;
    m_strMessage += "\r\nReply-To: ";
    m_strMessage += m_strSendMail;
    m_strMessage += "\r\nTo: ";
    for (size_t i = 0; i < m_vRecvMail.size(); i++)
    {
        if (i > 0)
        {
            m_strMessage += ",";
        }
        m_strMessage += m_vRecvMail[i];
    }
    m_strMessage += "\r\n";
    m_strMessage += m_strSubject;
    m_strMessage += "\r\nX-Mailer: JXO Mailer V1.2";
    m_strMessage += "\r\nMime-Version: 1.0";
    // 	m_strMessage += "\r\nContent-Type: multipart/mixed;";
    // 	m_strMessage += "boundary=\"simple boundary\"";
    // 	m_strMessage += "\r\nThis is a multi-part message in MIME format.";
    // 	m_strMessage += "\r\n--simple boundary";
    //正文
    m_strMessage += "\r\nContent-Type: text/html;";
    m_strMessage += "charset=";
    m_strMessage += "\"";
    m_strMessage += m_strCharset;
    m_strMessage += "\"";
    m_strMessage += "\r\nContent-Transfer-Encoding: 7BIT";
    m_strMessage += "\r\n\r\n";
    m_strMessage += m_strContent;
    //附件
    std::string filename = "";
    std::string filetype = "";
    for (size_t i = 0; i < m_vAttachMent.size(); i++)
    {
        m_strMessage += "\r\n--simple boundary";
        GetFileName(m_vAttachMent[i], filename);
        GetFileType(m_vAttachMent[i], filetype);
        SetContentType(filetype);
        SetFileName(filename);
        m_strMessage += "\r\nContent-Type: ";
        m_strMessage += m_strContentType;
        m_strMessage += "\tname=";
        m_strMessage += "\"";
        m_strMessage += m_strFileName;
        m_strMessage += "\"";
        m_strMessage += "\r\nContent-Disposition:attachment;filename=";
        m_strMessage += "\"";
        m_strMessage += m_strFileName;
        m_strMessage += "\"";
        m_strMessage += "\r\nContent-Transfer-Encoding:base64";
        m_strMessage += "\r\n\r\n";
        FILE *pt = NULL;
        if ((pt = fopen(m_vAttachMent[i].c_str(), "rb")) == NULL)
        {
            std::cerr << "打开文件失败: " << m_vAttachMent[i] << std::endl;
            continue;
        }
        fseek(pt, 0, SEEK_END);
        int len = ftell(pt);
        fseek(pt, 0, SEEK_SET);
        int rlen = 0;
        char buf[55];
        for (int j = 0; j < len / 54 + 1; j++)
        {
            memset(buf, 0, 55);
            rlen = fread(buf, sizeof(char), 54, pt);
            m_strMessage += base64_encode((const unsigned char*)buf, rlen);//NFBase64::Encode(std::string(buf, rlen));//
            m_strMessage += "\r\n";
        }
        fclose(pt);
        pt = NULL;
    }
    /*	m_strMessage += "\r\n--simple boundary--\r\n";*/
}
 
 
/**
 * @brief 获取文件类型索引
 * 
 * 根据文件扩展名返回对应的文件类型索引
 * 
 * @param stype 文件扩展名
 * @return 文件类型索引，-1表示未知类型
 */
int NFSmtpSendMail::GetFileType(std::string const & stype)
{
    if (stype == "txt")
    {
        return 0;
    }
    else if (stype == "xml")
    {
        return 1;
    }
    else if (stype == "html")
    {
        return 2;
    }
    else if (stype == "jpeg")
    {
        return 3;
    }
    else if (stype == "png")
    {
        return 4;
    }
    else if (stype == "gif")
    {
        return 5;
    }
    else if (stype == "exe")
    {
        return 6;
    }
    return -1;
}
 
/**
 * @brief 设置文件名
 * 
 * 将文件名进行Base64编码，支持中文文件名
 * 
 * @param FileName 文件名
 */
void NFSmtpSendMail::SetFileName(const std::string & FileName)
{
    std::string EncodedFileName = "=?";
    EncodedFileName += m_strCharset;
    EncodedFileName += "?B?";//修改
    EncodedFileName += base64_encode((unsigned char *)FileName.c_str(), FileName.size());//NFBase64::Encode(FileName);//
    EncodedFileName += "?=";
    m_strFileName = EncodedFileName;
}
 
/**
 * @brief 设置文件的Content-Type
 * 
 * 根据文件类型设置对应的MIME类型
 * 
 * @param stype 文件类型
 */
void NFSmtpSendMail::SetContentType(std::string const & stype)
{
    int type = GetFileType(stype);
    switch (type)
    {
    //
    case 0:
        m_strContentType = "plain/text;";
        break;
    case 1:
        m_strContentType = "text/xml;";
        break;
    case 2:
        m_strContentType = "text/html;";
    case 3:
        m_strContentType = "image/jpeg;";
        break;
    case 4:
        m_strContentType = "image/png;";
        break;
    case 5:
        m_strContentType = "image/gif;";
        break;
    case 6:
        m_strContentType = "application/x-msdownload;";
        break;
    default:
        m_strContentType = "application/octet-stream;";
        break;
    }
}
 
/**
 * @brief 从文件路径中提取文件名
 * 
 * 从完整的文件路径中提取文件名部分
 * 
 * @param file 文件路径
 * @param filename 提取的文件名（输出）
 */
void NFSmtpSendMail::GetFileName(const std::string& file, std::string& filename)
{
    std::string::size_type p = file.find_last_of('/');
    if (p == std::string::npos)
        p = file.find_last_of('\\');
    if (p != std::string::npos)
    {
        p += 1; // get past folder delimeter
        filename = file.substr(p, file.length() - p);
    }
}
 
/**
 * @brief 从文件路径中提取文件类型
 * 
 * 从完整的文件路径中提取文件扩展名
 * 
 * @param file 文件路径
 * @param stype 提取的文件类型（输出）
 */
void NFSmtpSendMail::GetFileType(const std::string & file, std::string & stype)
{
    std::string::size_type p = file.find_last_of('.');
    if (p != std::string::npos)
    {
        p += 1; // get past folder delimeter
        stype = file.substr(p, file.length() - p);
    }
}