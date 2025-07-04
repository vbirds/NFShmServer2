#include "evpp/inner_pre.h"

#include "sync_udp_client.h"
#include "evpp/libevent.h"
#include "evpp/sockets.h"

namespace evpp {
namespace udp {
namespace sync {
Client::Client() {
    sockfd_ = INVALID_SOCKET;
    memset(&remote_addr_, 0, sizeof(remote_addr_));
}

Client::~Client(void) {
    Close();
}

bool Client::Connect(const struct sockaddr_in& addr) {
    memcpy(&remote_addr_, &addr, sizeof(remote_addr_));
    return Connect();
}

bool Client::Connect(const char* host, int port) {
    char buf[32] = {};
    snprintf(buf, sizeof buf, "%s:%d", host, port);
    return Connect(buf);
}

bool Client::Connect(const struct sockaddr_storage& addr) {
    memcpy(&remote_addr_, &addr, sizeof(remote_addr_));
    return Connect();
}

bool Client::Connect(const char* addr/*host:port*/) {
    remote_addr_ = sock::ParseFromIPPort(addr);
    return Connect();
}

bool Client::Connect(const struct sockaddr& addr) {
    memcpy(&remote_addr_, &addr, sizeof(remote_addr_));
    return Connect();
}

bool Client::Connect() {
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    sock::SetReuseAddr(sockfd_);

    struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&remote_addr_);
    socklen_t addrlen = sizeof(*addr);
    int ret = ::connect(sockfd_, addr, addrlen);

    if (ret != 0) {
        EVPP_LOG_ERROR << "Failed to connect to remote "
                  << sock::ToIPPort(&remote_addr_)
                  << ", errno=" << errno << " " << strerror(errno);
        NFLogError(NF_LOG_DEFAULT, 0, "Failed to connect to remote {} , errno={} {}", sock::ToIPPort(&remote_addr_), errno, strerror(errno));
        Close();
        return false;
    }

    connected_ = true;
    return true;
}

void Client::Close() {
    EVUTIL_CLOSESOCKET(sockfd_);
}


std::string Client::DoRequest(const std::string& data, uint32_t timeout_ms) {
    if (!Send(data)) {
        int eno = errno;
        EVPP_LOG_ERROR << "sent failed, errno=" << eno << " " << strerror(eno) << " , dlen=" << data.size();
        NFLogError(NF_LOG_DEFAULT, 0, "sent failed, errno={} {} , dlen={}", eno, strerror(eno), data.size());
        return "";
    }

    sock::SetTimeout(sockfd_, timeout_ms);

    size_t buf_size = 1472; // The UDP max payload size
    MessagePtr msg(new Message(sockfd_, buf_size));
    socklen_t addrLen = sizeof(struct sockaddr);
    int readn = ::recvfrom(sockfd_, msg->WriteBegin(), buf_size, 0, msg->mutable_remote_addr(), &addrLen);
    int err = errno;
    if (readn >= 0) {
        msg->WriteBytes(readn);
        return std::string(msg->data(), msg->size());
    } else {
        EVPP_LOG_ERROR << "errno=" << err << " " << strerror(err) << " recvfrom return -1";
        NFLogError(NF_LOG_DEFAULT, 0, "errno={} {}  recvfrom return -1", err, strerror(err));
    }

    return "";
}

std::string Client::DoRequest(const std::string& remote_ip, int port, const std::string& udp_package_data, uint32_t timeout_ms) {
    Client c;
    if (!c.Connect(remote_ip.data(), port)) {
        return "";
    }

    return c.DoRequest(udp_package_data, timeout_ms);
}

bool Client::Send(const char* msg, size_t len) {
    if (connected_) {
        int sentn = ::send(sockfd(), msg, len, 0);
        return static_cast<size_t>(sentn) == len;
    }

    struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&remote_addr_);
    socklen_t addrlen = sizeof(*addr);
    int sentn = ::sendto(sockfd(),
                         msg, len, 0,
                         addr,
                         addrlen);
    return sentn > 0;
}

bool Client::Send(const std::string& msg) {
    return Send(msg.data(), msg.size());
}

bool Client::Send(const std::string& msg, const struct sockaddr_in& addr) {
    return Client::Send(msg.data(), msg.size(), addr);
}


bool Client::Send(const char* msg, size_t len, const struct sockaddr_in& addr) {
    Client c;
    if (!c.Connect(addr)) {
        return false;
    }

    return c.Send(msg, len);
}

bool Client::Send(const MessagePtr& msg) {
    return Client::Send(msg->data(), msg->size(), *reinterpret_cast<const struct sockaddr_in*>(msg->remote_addr()));
}

bool Client::Send(const Message* msg) {
    return Client::Send(msg->data(), msg->size(), *reinterpret_cast<const struct sockaddr_in*>(msg->remote_addr()));
}

}
}
}


