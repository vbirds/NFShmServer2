// -------------------------------------------------------------------------
//    @FileName         :    NFCHttpServer.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCHttpServer.cpp
//
// -------------------------------------------------------------------------


#include <NFComm/NFPluginModule/NFCheck.h>
#include "NFCHttpServer.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFCore/NFCommon.h"
#include "evpp/http/context.h"
#include "evpp/libevent.h"
#include "NFComm/NFCore/NFTime.h"

/**
 * @file NFCHttpServer.cpp
 * @brief Evpp HTTP服务器实现文件
 * 
 * 该文件实现了基于evpp库的HTTP服务器，包括：
 * - HTTP服务器的初始化和销毁
 * - HTTP请求处理句柄的实现
 * - HTTP消息处理类的实现
 * - 服务器监听和连接管理
 * - HTTP请求和响应处理
 * - SSL/TLS安全连接支持
 * 
 * 主要功能：
 * - 创建和管理evpp HTTP服务器
 * - 处理HTTP请求和响应
 * - 支持多线程并发处理
 * - 提供SSL/TLS安全连接
 * - 对象池优化性能
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief HTTP请求处理句柄构造函数
 * 
 * 初始化HTTP请求处理句柄，设置默认值
 */
NFServerHttpHandle::NFServerHttpHandle()
{
    m_type = NF_HTTP_REQ_GET;
    m_requestId = 0;
    m_timeOut = 0;
}

/**
 * @brief 重置HTTP请求处理句柄
 * 
 * 清空句柄的所有状态，准备重新使用
 */
void NFServerHttpHandle::Reset()
{
    m_requestId = 0;
    m_timeOut = 0;
    m_ctx = nullptr;
    m_responseCb = nullptr;
}

/**
 * @brief 添加响应头
 * 
 * 向HTTP响应中添加自定义头字段
 * 
 * @param key 头字段名
 * @param value 头字段值
 */
void NFServerHttpHandle::AddResponseHeader(const std::string& key, const std::string& value) const
{
    if (m_ctx)
    {
        m_ctx->AddResponseHeader(key, value);
    }
}

/**
 * @brief 发送响应消息
 * 
 * 向客户端发送HTTP响应消息，包括：
 * - 设置响应头
 * - 设置HTTP状态码
 * - 发送响应内容
 * 
 * @param strMsg 响应消息内容
 * @param code HTTP状态码
 * @param strReason 状态原因
 * @return true 发送成功，false 发送失败
 */
bool NFServerHttpHandle::ResponseMsg(const std::string& strMsg, NFWebStatus code, const std::string& strReason) const
{
    AddResponseHeader("Content-Type", "application/json");
    AddResponseHeader("Access-Control-Allow-Origin", "*");

    if (m_ctx)
    {
        m_ctx->set_response_http_code(code);
    }

    if (m_responseCb)
    {
        m_responseCb(strMsg);
    }
    return true;
}

/**
 * @brief 获取查询参数
 * 
 * 从HTTP请求中获取指定的查询参数
 * 
 * @param queryKey 查询参数名
 * @return 查询参数值
 */
std::string NFServerHttpHandle::GetQuery(const std::string& queryKey) const
{
    if (m_ctx)
    {
        return m_ctx->GetQuery(queryKey);
    }
    return std::string();
}

/**
 * @brief HTTP消息构造函数
 * 
 * 初始化HTTP消息对象
 */
NFEvppHttMsg::NFEvppHttMsg()
{
    Clear();
}

/**
 * @brief HTTP消息析构函数
 * 
 * 清理HTTP消息对象资源
 */
NFEvppHttMsg::~NFEvppHttMsg()
{
    Clear();
}

/**
 * @brief HTTP消息拷贝构造函数
 * 
 * @param msg 要拷贝的HTTP消息对象
 */
NFEvppHttMsg::NFEvppHttMsg(const NFEvppHttMsg& msg)
{
    if (this != &msg)
    {
        m_ctx = msg.m_ctx;
        m_responseCb = msg.m_responseCb;
    }
}

/**
 * @brief HTTP消息赋值操作符
 * 
 * @param msg 要赋值的HTTP消息对象
 * @return 当前对象引用
 */
NFEvppHttMsg& NFEvppHttMsg::operator=(const NFEvppHttMsg& msg)
{
    if (this != &msg)
    {
        m_ctx = msg.m_ctx;
        m_responseCb = msg.m_responseCb;
    }
    return *this;
}

/**
 * @brief 清空HTTP消息内容
 * 
 * 重置所有成员变量为默认值
 */
void NFEvppHttMsg::Clear()
{
    m_ctx = nullptr;
    m_responseCb = nullptr;
}

NFCHttpServer::NFCHttpServer(uint32_t serverType, uint32_t netThreadNum)
{
    m_serverType = serverType;
    m_port = 0;
    m_pHttpServer = new evpp::http::Server(netThreadNum);
    m_index = 0;
    m_listHttpRequestPool = NF_NEW NFObjectPool<NFServerHttpHandle>(1000, false);
    m_pHttpServer->RegisterDefaultHandler([this](evpp::EventLoop*,
                                                 const evpp::http::ContextPtr &ctx,
                                                 const evpp::http::HTTPSendResponseCallback& respCb)
    {
                                              NFEvppHttMsg msg;
                                              msg.m_ctx = ctx;
                                              msg.m_responseCb = respCb;
                                              while (!m_msgQueue.Enqueue(msg))
                                              {
                                              }
    });
}

NFCHttpServer::~NFCHttpServer()
{
    for (auto iter = m_httpRequestMap.begin(); iter != m_httpRequestMap.end(); ++iter)
    {
        NF_SAFE_DELETE(iter->second);
    }
    m_httpRequestMap.clear();

    if (m_listHttpRequestPool)
    {
        NF_SAFE_DELETE(m_listHttpRequestPool);
    }

    if (m_pHttpServer)
    {
        m_pHttpServer->Stop();
        NFSLEEP(1000*1000); // sleep a while to release the listening address and port
        NF_SAFE_DELETE(m_pHttpServer);
    }
}

bool NFCHttpServer::Execute()
{
    ProcessMsgLogicThread();
    std::vector<NFServerHttpHandle*> vec;
    for (auto iter = m_httpRequestMap.begin(); iter != m_httpRequestMap.end(); ++iter)
    {
        auto pRequest = iter->second;
        if (pRequest->m_timeOut + 30 <= static_cast<uint64_t>(NFGetSecondTime()))
        {
            vec.push_back(pRequest);
        }
    }

    for (size_t i = 0; i < vec.size(); i++)
    {
        NFServerHttpHandle *pRequest = vec[i];
        ResponseMsg(*pRequest, "TimeOut Error", WEB_TIMEOUT);
    }

    return true;
}

uint32_t NFCHttpServer::GetServerType() const
{
    return m_serverType;
}

bool NFCHttpServer::InitServer(int listenPort) const
{
    if (m_pHttpServer->Init(listenPort))
    {
        if (m_pHttpServer->Start())
        {
            return true;
        }
        NFLogError(NF_LOG_DEFAULT, 0, "Start Listen Port:{} Failed!", listenPort);
    }

    NFLogError(NF_LOG_DEFAULT, 0, "Init Listen Port:{} Failed!", listenPort);
    return false;
}

bool NFCHttpServer::InitServer(const std::vector<int>& listenPorts) const
{
    if (m_pHttpServer->Init(listenPorts))
    {
        if (m_pHttpServer->Start())
        {
            return true;
        }
        NFLogError(NF_LOG_DEFAULT, 0, "Start Listen Port:{} Failed!", NFCommon::tostr(listenPorts));
    }

    NFLogError(NF_LOG_DEFAULT, 0, "Init Listen Port:{} Failed!", NFCommon::tostr(listenPorts));
    return false;
}

bool NFCHttpServer::InitServer(const std::string& listenPorts/*like "80,8080,443"*/) const
{
    if (m_pHttpServer->Init(listenPorts))
    {
        if (m_pHttpServer->Start())
        {
            return true;
        }
        NFLogError(NF_LOG_DEFAULT, 0, "Start Listen Port:{} Failed!", listenPorts);
    }

    NFLogError(NF_LOG_DEFAULT, 0, "Init Listen Port:{} Failed!", listenPorts);
    return false;
}

void NFCHttpServer::ProcessMsgLogicThread()
{
    int maxTimes = 10000;
    while (!m_msgQueue.IsQueueEmpty() && maxTimes >= 0)
    {
        std::vector<NFEvppHttMsg> vecMsg;
        vecMsg.resize(200);

        m_msgQueue.TryDequeueBulk(vecMsg);
        for (size_t i = 0; i < vecMsg.size(); i++)
        {
            maxTimes--;
            NFEvppHttMsg *pMsg = &vecMsg[i];
            if (pMsg == nullptr) continue;

            NFServerHttpHandle *pRequest = AllocHttpRequest();
            pRequest->m_ctx = pMsg->m_ctx;
            pRequest->m_responseCb = pMsg->m_responseCb;
            pRequest->m_type = static_cast<NFHttpType>(pMsg->m_ctx->req()->type);
            pRequest->m_timeOut = NF_ADJUST_TIMENOW();

            m_httpRequestMap.emplace(pRequest->m_requestId, pRequest);

            bool flag = true;
            if (m_filter)
            {
                //return 401
                try
                {
                    NFWebStatus xWebStatus = m_filter(m_serverType, *pRequest);
                    if (xWebStatus != WEB_OK)
                    {
                        //401
                        ResponseMsg(*pRequest, "Filter error", xWebStatus);
                        flag = false;
                    }
                }
                catch (std::exception &e)
                {
                    ResponseMsg(*pRequest, e.what(), WEB_ERROR);
                    flag = false;
                }
                catch (...)
                {
                    ResponseMsg(*pRequest, "UNKNOW ERROR", WEB_ERROR);
                    flag = false;
                }
            }

            if (flag)
            {
                // call cb
                try
                {
                    if (m_receiveCb)
                    {
                        m_receiveCb(m_serverType, *pRequest);
                    } else
                    {
                        ResponseMsg(*pRequest, "NO PROCESSER", WEB_ERROR);
                    }
                }
                catch (std::exception &e)
                {
                    ResponseMsg(*pRequest, e.what(), WEB_ERROR);
                }
                catch (...)
                {
                    ResponseMsg(*pRequest, "UNKNOW ERROR", WEB_ERROR);
                }
            }
        }
    }
}

NFServerHttpHandle *NFCHttpServer::AllocHttpRequest()
{
    NFServerHttpHandle* pRequest = m_listHttpRequestPool->MallocObj();
    CHECK_EXPR_ASSERT(pRequest, NULL, "mListHttpRequestPool->MallocObj() Failed");

    pRequest->Reset();

    pRequest->m_requestId = ++m_index;

    return pRequest;
}

bool NFCHttpServer::ResponseMsg(const NFIHttpHandle &req, const std::string &strMsg, NFWebStatus code,
                                const std::string &strReason)
{
    req.ResponseMsg(strMsg, code, strReason);

    auto it = m_httpRequestMap.find(req.GetRequestId());
    if (it != m_httpRequestMap.end())
    {
        it->second->Reset();
        m_listHttpRequestPool->FreeObj(it->second);
        m_httpRequestMap.erase(it);
    }
    return true;
}

bool NFCHttpServer::ResponseMsg(uint64_t requestId, const std::string &strMsg, NFWebStatus code,
                                const std::string &strReason)
{
    NFServerHttpHandle* req = nullptr;
    auto it = m_httpRequestMap.find(requestId);
    if (it == m_httpRequestMap.end())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "Response Msg Timeout........ requestId:{}, mStrMsg:{}", requestId, strMsg);
        return false;
    }

    req = it->second;

    bool ret = req->ResponseMsg(strMsg, code, strReason);
    if (!ret)
    {
        NFLogError(NF_LOG_DEFAULT, 0, "Response Msg error........ requestId:{}, mStrMsg:{}", requestId, strMsg);
    }

    req->Reset();
    m_listHttpRequestPool->FreeObj(it->second);
    m_httpRequestMap.erase(it);
    return true;
}

void NFCHttpServer::SetRecvCb(const HTTP_RECEIVE_FUNCTOR& recvCb)
{
    m_receiveCb = recvCb;
}

void NFCHttpServer::SetFilterCb(const HTTP_FILTER_FUNCTOR& eventCb)
{
    m_filter = eventCb;
}

#if defined(EVPP_HTTP_SERVER_SUPPORTS_SSL)
/* berif 对指定监听端口设置SSL选项
 * param listen_port 监听的端口
 * param enable_ssl 是否开启SSL支持
 * param certificate_chain_file 证书链文件
 * param private_key_file 私钥文件
 */
void NFCHttpServer::SetPortSSLOption(int listen_port,
                      bool enable_ssl,
                      const char* certificate_chain_file,
                      const char* private_key_file)
{
    m_pHttpServer->setPortSSLOption(listen_port, enable_ssl, certificate_chain_file, private_key_file);
}

/* berif 设置端口默认SSL配置选项
 * param enable_ssl 是否开启SSL支持
 * param certificate_chain_file 证书链文件
 * param private_key_file 私钥文件
 */
void NFCHttpServer::SetPortSSLDefaultOption(
        bool enable_ssl,
        const char* certificate_chain_file,
        const char* private_key_file)
{
    m_pHttpServer->setPortSSLDefaultOption( enable_ssl, certificate_chain_file, private_key_file);
}
#endif


