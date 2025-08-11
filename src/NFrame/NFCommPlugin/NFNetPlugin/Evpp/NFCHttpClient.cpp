// -------------------------------------------------------------------------
//    @FileName         :    NFCHttpClient.cpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCHttpClient.cpp
//
// -------------------------------------------------------------------------

#include "NFCHttpClient.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "evpp/httpc/response.h"
#include "NFComm/NFCore/NFCommon.h"

/**
 * @file NFCHttpClient.cpp
 * @brief Evpp HTTP客户端实现文件
 * 
 * 该文件实现了基于evpp库的HTTP客户端，包括：
 * - HTTP客户端的初始化和销毁
 * - HTTP客户端消息类的实现
 * - HTTP客户端参数类的实现
 * - HTTP请求发送和响应处理
 * - 异步请求管理和超时处理
 * 
 * 主要功能：
 * - 创建和管理evpp HTTP客户端
 * - 发送HTTP GET/POST请求
 * - 处理异步响应
 * - 支持请求超时管理
 * - 对象池优化性能
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 * @version 1.0
 */

/**
 * @brief HTTP客户端消息构造函数
 * 
 * 初始化HTTP客户端消息对象，设置默认值
 */
NFHttpClientMsg::NFHttpClientMsg()
{
    m_code = 0;
    m_reqId = 0;
}

/**
 * @brief 清空HTTP客户端消息内容
 * 
 * 重置所有成员变量为默认值
 */
void NFHttpClientMsg::Clear()
{
    m_body.clear();
    m_code = 0;
    m_reqId = 0;
}

/**
 * @brief HTTP客户端消息拷贝构造函数
 * 
 * @param msg 要拷贝的HTTP客户端消息对象
 */
NFHttpClientMsg::NFHttpClientMsg(const NFHttpClientMsg& msg)
{
    if (this != &msg)
    {
        m_body = msg.m_body;
        m_code = msg.m_code;
        m_reqId = msg.m_reqId;
    }
}

/**
 * @brief HTTP客户端消息赋值操作符
 * 
 * @param msg 要赋值的HTTP客户端消息对象
 * @return 当前对象引用
 */
NFHttpClientMsg& NFHttpClientMsg::operator=(const NFHttpClientMsg& msg)
{
    if (this != &msg)
    {
        m_body = msg.m_body;
        m_code = msg.m_code;
        m_reqId = msg.m_reqId;
    }
    return *this;
}

/**
 * @brief HTTP客户端参数构造函数
 * 
 * 初始化HTTP客户端参数对象，包括：
 * - 设置请求ID
 * - 设置响应回调函数
 * - 计算超时时间
 * 
 * @param id 请求ID
 * @param func 响应回调函数
 * @param timeout 超时时间（秒）
 */
NFCHttpClientParam::NFCHttpClientParam(int id, const HTTP_CLIENT_RESPONE& func, uint32_t timeout): m_id(id), m_resp(func)
{
    m_timeout = NF_ADJUST_TIMENOW() + timeout * 10;
}

/**
 * @brief HTTP客户端参数析构函数
 * 
 * 清理HTTP客户端参数对象资源
 */
NFCHttpClientParam::~NFCHttpClientParam()
{
}

/**
 * @brief 检查是否超时
 * 
 * 检查当前时间是否超过设置的超时时间
 * 
 * @return true 已超时，false 未超时
 */
bool NFCHttpClientParam::IsTimeOut() const
{
    return NF_ADJUST_TIMENOW() > m_timeout;
}

/**
 * @brief HTTP客户端构造函数
 * 
 * 初始化HTTP客户端，包括：
 * - 启动事件循环线程
 * - 初始化请求ID
 * - 创建对象池
 */
NFCHttpClient::NFCHttpClient()
{
    m_threadLoop.Start();
    m_staticReqId = 10000;
    m_pHttpClientParamPool = NF_NEW NFObjectPool<NFCHttpClientParam>(1000, false);
}

/**
 * @brief HTTP客户端析构函数
 * 
 * 清理HTTP客户端资源，包括：
 * - 停止事件循环线程
 * - 释放对象池
 */
NFCHttpClient::~NFCHttpClient()
{
    m_threadLoop.Stop(true);
    if (m_pHttpClientParamPool)
    {
        NF_SAFE_DELETE(m_pHttpClientParamPool);
    }
}

/**
 * @brief 处理HTTP GET请求响应
 * 
 * 处理HTTP GET请求的响应，包括：
 * - 记录响应日志
 * - 创建响应消息
 * - 将消息加入队列
 * - 清理请求对象
 * 
 * @param response HTTP响应对象
 * @param request HTTP GET请求对象
 */
void NFCHttpClient::HandleHttpGetResponse(const std::shared_ptr<evpp::httpc::Response>& response,
                                          const evpp::httpc::GetRequest* request)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "HttpRespone url:http://{}{} code:{} body:{}", request->host(), request->uri(),
               response->http_code(), response->body().ToString());
    NF_ASSERT(request == response->request());

    NFHttpClientMsg msg;
    msg.m_code = response->http_code();
    msg.m_body = response->body().ToString();
    msg.m_reqId = request->GetId();
    while (!m_msgQueue.Enqueue(msg))
    {
    }

    NF_SAFE_DELETE(request); // The request MUST BE deleted in EventLoop thread.
}

/**
 * @brief 发送HTTP GET请求
 * 
 * 发送HTTP GET请求，包括：
 * - 记录请求日志
 * - 创建请求对象
 * - 设置请求头
 * - 发送请求
 * 
 * @param strUri 请求URI
 * @param respone 响应回调函数
 * @param xHeaders 请求头
 * @param timeout 超时时间
 * @return 请求结果
 */
int NFCHttpClient::HttpGet(const string& strUri, const HTTP_CLIENT_RESPONE& respone,
                           const map<std::string, std::string>& xHeaders, int timeout)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "HttpGet uri:{} headers:{}", strUri, NFCommon::tostr(xHeaders));

    auto req = new evpp::httpc::GetRequest(m_threadLoop.loop());
    CHECK_EXPR(req, -1, "new GetRequest Failed!");

    int ret = req->Init(strUri, "", evpp::Duration(timeout));
    CHECK_RET(ret, "req->Init uri:{} posData:{} Failed, ret:{}", strUri, "", ret);

    req->SetId(m_staticReqId++);

    for (auto iter = xHeaders.begin(); iter != xHeaders.end(); ++iter)
    {
        req->AddHeader(iter->first, iter->second);
    }

    NFCHttpClientParam* pParam = m_pHttpClientParamPool->MallocObjWithArgs(req->GetId(), respone, timeout);
    CHECK_EXPR_ASSERT(pParam, -1, "m_pHttpClientParamPool->MallocObj() Failed");

    m_httpClientMap.emplace(pParam->m_id, pParam);

    req->Execute(std::bind(&NFCHttpClient::HandleHttpGetResponse, this, std::placeholders::_1, req));
    return 0;
}

int NFCHttpClient::HttpPost(const string& strUri, const string& strPostData, const HTTP_CLIENT_RESPONE& respone,
                            const map<std::string, std::string>& xHeaders, int timeout)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "HttpPost uri:{} headers:{}", strUri, NFCommon::tostr(xHeaders));

    auto req = new evpp::httpc::PostRequest(m_threadLoop.loop());
    CHECK_EXPR(req, -1, "new PostRequest Failed!");

    int ret = req->Init(strUri, strPostData, evpp::Duration(timeout));
    CHECK_RET(ret, "req->Init uri:{} posData:{} Failed", strUri, strPostData);

    req->SetId(m_staticReqId++);

    for (auto iter = xHeaders.begin(); iter != xHeaders.end(); ++iter)
    {
        req->AddHeader(iter->first, iter->second);
    }

    NFCHttpClientParam* pParam = m_pHttpClientParamPool->MallocObjWithArgs(req->GetId(), respone, timeout);
    CHECK_EXPR_ASSERT(pParam, -1, "m_pHttpClientParamPool->MallocObj() Failed");
    m_httpClientMap.emplace(pParam->m_id, pParam);

    req->Execute(std::bind(&NFCHttpClient::HandleHttpPostResponse, this, std::placeholders::_1, req));
    return 0;
}

void NFCHttpClient::HandleHttpPostResponse(const shared_ptr<evpp::httpc::Response>& response,
                                           const evpp::httpc::PostRequest* request)
{
    NFLogTrace(NF_LOG_DEFAULT, 0, "HttpRespone url:http://{}{} code:{} body:{}", request->host(), request->uri(),
               response->http_code(), response->body().ToString());
    NF_ASSERT(request == response->request());

    NFHttpClientMsg msg;
    msg.m_code = response->http_code();
    msg.m_body = response->body().ToString();
    msg.m_reqId = request->GetId();
    while (!m_msgQueue.Enqueue(msg))
    {
    }

    NF_SAFE_DELETE(request); // The request MUST BE deleted in EventLoop thread.
}

void NFCHttpClient::ProcessMsgLogicThread()
{
    int maxTimes = 10000;
    while (!m_msgQueue.IsQueueEmpty() && --maxTimes >= 0)
    {
        std::vector<NFHttpClientMsg> vecMsg;
        vecMsg.resize(200);
        m_msgQueue.TryDequeueBulk(vecMsg);
        for (size_t i = 0; i < vecMsg.size(); i++)
        {
            NFHttpClientMsg* pMsg = &vecMsg[i];
            if (pMsg)
            {
                auto iter = m_httpClientMap.find(pMsg->m_reqId);
                if (iter != m_httpClientMap.end())
                {
                    NFCHttpClientParam* pParam = iter->second;
                    if (pParam)
                    {
                        pParam->m_resp(pMsg->m_code, pMsg->m_body);
                        m_pHttpClientParamPool->FreeObj(pParam);
                    }
                    m_httpClientMap.erase(iter);
                }
            }
        }
    }
}

bool NFCHttpClient::Execute()
{
    ProcessMsgLogicThread();

    for (auto iter = m_httpClientMap.begin(); iter != m_httpClientMap.end();)
    {
        NFCHttpClientParam* pParam = iter->second;
        if (pParam)
        {
            if (!pParam->IsTimeOut())
            {
                ++iter;
                continue;
            }
            m_pHttpClientParamPool->FreeObj(pParam);
        }
        iter = m_httpClientMap.erase(iter);
    }
    return true;
}

