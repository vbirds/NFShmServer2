#include "evpp/libevent.h"
#include "evpp/httpc/conn_pool.h"
#include "evpp/httpc/response.h"
#include "evpp/httpc/request.h"
#include "evpp/httpc/url_parser.h"

#if defined(EVPP_HTTP_CLIENT_SUPPORTS_SSL)
#include <openssl/err.h>
#endif

namespace evpp {
namespace httpc {
const std::string Request::empty_ = "";

Request::Request(ConnPool* pool, EventLoop* loop, const std::string& http_uri, const std::string& body)
    : pool_(pool), loop_(loop), host_(pool->host()), port_(pool->port()), uri_(http_uri), body_(body) {
}

Request::Request(EventLoop* loop)
    : pool_(nullptr), loop_(loop) {

}

Request::~Request() {
    assert(loop_->IsInLoopThread());
}

int Request::Init(const std::string& http_url, const std::string& body, Duration timeout)
{
    body_ = body;

    //TODO performance compare
#if LIBEVENT_VERSION_NUMBER >= 0x02001500
    struct evhttp_uri* evuri = evhttp_uri_parse(http_url.c_str());
    if (evuri == NULL)
    {
        return -1;
    }

    const char* scheme = evhttp_uri_get_scheme(evuri);
    if (scheme == NULL)
    {
        return -2;
    }

    std::string lowwerScheme(scheme);
    std::transform(lowwerScheme.begin(), lowwerScheme.end(), lowwerScheme.begin(), (int (*)(int)) std::tolower);
    if (lowwerScheme.compare("https") != 0 && lowwerScheme.compare("http") != 0)
    {
        return -3;
    }

    const char* path = evhttp_uri_get_path(evuri);
    if (path == NULL) {
        return -4;
    }

    uri_ = path;
    if (uri_.size() == 0)
    {
        uri_ += "/";
    }

    const char* query = evhttp_uri_get_query(evuri);
    if (query && strlen(query) > 0) {
        uri_ += "?";
        uri_ += query;
    }

    const char* host = evhttp_uri_get_host(evuri);
    if (host == NULL)
    {
        return -5;
    }

    host_ = host;
    port_ = evhttp_uri_get_port(evuri);

#if defined(EVPP_HTTP_CLIENT_SUPPORTS_SSL)
    bool enable_ssl = scheme && strcasecmp(scheme, "https") == 0;
    if (port_ < 0) {
        port_ = enable_ssl ? 443 : 80;
    }
    conn_.reset(new Conn(loop_, host_, port_, enable_ssl, timeout));
#else
    if (port_ < 0) {
        port_ = 80;
    }
    conn_.reset(new Conn(loop_, host_, port_, timeout));
#endif
    evhttp_uri_free(evuri);
#else
    URLParser p(http_url);
    conn_.reset(new Conn(loop_, p.host, p.port, timeout));
    if (p.query.empty()) {
        uri_ = p.path;
    } else {
        uri_ = p.path + "?" + p.query;
    }
    host_ = p.host;
    port_ = p.port;
#endif
    return 0;
}

void Request::Execute(const Handler& h) {
    handler_ = h;
    loop_->RunInLoop(std::bind(&Request::ExecuteInLoop, this));
}

void Request::ExecuteInLoop() {
    DEVPP_LOG_TRACE;
    assert(loop_->IsInLoopThread());
    evhttp_cmd_type req_type = EVHTTP_REQ_GET;

    std::string errmsg;
    struct evhttp_request* req = nullptr;

    if (conn_) {
        assert(pool_ == nullptr);
        if (!conn_->Init()) {
            errmsg = "conn init fail";
            goto failed;
        }
    } else {
        assert(pool_);
        conn_ = pool_->Get(loop_);
        if (!conn_->Init()) {
            errmsg = "conn init fail";
            goto failed;
        }
    }

    req = evhttp_request_new(&Request::HandleResponse, this);
    if (!req) {
        errmsg = "evhttp_request_new fail";
        goto failed;
    }

    if (evhttp_add_header(req->output_headers, "host", conn_->host().c_str())) {
        evhttp_request_free(req);
        errmsg = "evhttp_add_header failed";
        goto failed;
    }

    for (const auto& header : headers_) {
        if (evhttp_add_header(
                req->output_headers, header.first.c_str(), header.second.c_str())) {
            evhttp_request_free(req);
            errmsg = "evhttp_add_header failed";
            goto failed;
        }
    }

    if (!body_.empty()) {
        req_type = EVHTTP_REQ_POST;
        if (evbuffer_add(req->output_buffer, body_.c_str(), body_.size())) {
            evhttp_request_free(req);
            errmsg = "evbuffer_add fail";
            goto failed;
        }
    }

    if (evhttp_make_request(conn_->evhttp_conn(), req, req_type, uri_.c_str()) != 0) {
        // At here conn_ has owned this req, so don't need to free it.
        errmsg = "evhttp_make_request fail";
        goto failed;
    }

    return;

failed:
    // Retry
    if (retried_ < retry_number_) {
        EVPP_LOG_WARN << "this=" << this << " http request failed : " << errmsg << " retried=" << retried_ << " max retry_time=" << retry_number_ << ". Try again.";
        Retry();
        return;
    }

    std::shared_ptr<Response> response(new Response(this, nullptr));
    handler_(response);
}

void Request::AddHeader(const std::string& header, const std::string& value) {
    headers_[header] = value;
}

void Request::Retry() {
    retried_ += 1;

    // Recycling the http Connection object for retry.
    // Connection will be obtained again by ExecuteInLoop
    if (pool_) {
        pool_->Put(conn_);
        conn_.reset();
    }

    if (retry_interval_.IsZero()) {
        ExecuteInLoop();
    } else {
        loop_->RunAfter(retry_interval_, std::bind(&Request::ExecuteInLoop, this));
    }
}

void Request::HandleResponse(struct evhttp_request* r, void* v) {
    Request* thiz = (Request*)v;
    assert(thiz);
    thiz->HandleResponse(r);
}

void Request::HandleResponse(struct evhttp_request* r) {
    assert(loop_->IsInLoopThread());

    if (r) {
        int response_code = r->response_code;
        bool needs_retry = response_code >= 500 && response_code < 600;
        if (!needs_retry || retried_ >= retry_number_) {
            EVPP_LOG_WARN << "this=" << this << " response_code=" << r->response_code << " retried=" << retried_ << " max retry_time=" << retry_number_;
            std::shared_ptr<Response> response(new Response(this, r));

            //Recycling the http Connection object
            if (pool_) {
                pool_->Put(conn_);
                conn_.reset();
            }

            handler_(response);
            return;
        }
    }

    // Retry
    if (retried_ < retry_number_) {
        EVPP_LOG_WARN << "this=" << this << " response_code=" << (r ? r->response_code : 0) << " retried=" << retried_ << " max retry_time=" << retry_number_ << ". Try again";
        Retry();
        return;
    }

#if defined(EVPP_HTTP_CLIENT_SUPPORTS_SSL)
    if (!r) {
        int errcode = EVUTIL_SOCKET_ERROR();
        unsigned long oslerr;
        bool printed_some_error = false;
        char buffer[256];
        while ((oslerr = bufferevent_get_openssl_error(conn_->bufferevent()))) {
            ERR_error_string_n(oslerr, buffer, sizeof(buffer));
            EVPP_LOG_ERROR << "Openssl error: " << buffer;
            NFLogError(NF_LOG_DEFAULT, 0, "Openssl error:{}", (char*)buffer);
            printed_some_error = true;
        }
        if (!printed_some_error) {
            EVPP_LOG_ERROR << "socket error(" << errcode << "): "
                << evutil_socket_error_to_string(errcode);
            NFLogError(NF_LOG_DEFAULT, 0, "socket error({}):{}", errcode, evutil_socket_error_to_string(errcode));
        }
    }
#endif
    // Eventually this Request failed
    std::shared_ptr<Response> response(new Response(this, r));

    // Recycling the http Connection object
    if (pool_) {
        pool_->Put(conn_);
        conn_.reset();
    }

    handler_(response);
}

} // httpc
} // evpp


