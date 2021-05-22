// SPDX-License-Identifier: BSD-3-Clause

#include <cstring>
#include <string>
#include <cerrno>
#include <algorithm>
#include <memory>
#include <utility>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../voltronic/exceptions.h"
#include "../p18/exceptions.h"
#include "../voltronic/time.h"
#include "../logging.h"
//#include "hexdump/hexdump.h"
#include "server.h"
#include "connection.h"
#include "signal.h"

namespace server {

Server::Server(std::shared_ptr<voltronic::Device> device)
    : sock_(0)
    , port_(0)
    , cacheTimeout_(CACHE_TIMEOUT)
    , delay_(0)
    , endExecutionTime_(0)
    , verbose_(false)
    , device_(std::move(device)) {
    client_.setDevice(device_);
}

void Server::setVerbose(bool verbose) {
    verbose_ = verbose;
    device_->setVerbose(verbose);
}

void Server::setCacheTimeout(u64 timeout) {
    cacheTimeout_ = timeout;
}

void Server::setDelay(u64 delay) {
    delay_ = delay;
}

Server::~Server() {
    if (sock_ > 0)
        close(sock_);
}

void Server::start(std::string& host, int port) {
    host_ = host;
    port_ = port;

    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ == -1)
        throw ServerError("failed to create socket");

    struct linger sl = {0};
    sl.l_onoff = 1;
    sl.l_linger = 0;
    if (setsockopt(sock_, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) == -1)
        throw ServerError("setsockopt(linger): " + std::string(strerror(errno)));

    int flag = 1;
    if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
        throw ServerError("setsockopt(reuseaddr): " + std::string(strerror(errno)));

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host_.c_str());
    serv_addr.sin_port = htons(port_);
    memset(serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));

    if (bind(sock_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        throw ServerError("bind: " + std::string(strerror(errno)));

    if (listen(sock_, 50))
        throw ServerError("start: " + std::string(strerror(errno)));

    while (!shutdownCaught) {
        struct sockaddr_in addr = {0};
        socklen_t addr_size = sizeof(addr);

        if (verbose_)
            mylog << "waiting for client..";

        int sock = accept(sock_, (struct sockaddr*)&addr, &addr_size);
        if (sock == -1)
            continue;

        auto conn = new Connection(sock, addr, this);
        addConnection(conn);
    }
}

void Server::addConnection(Connection *conn) {
    if (verbose_)
        myerr << "adding " << conn->ipv4();
    LockGuard lock(threads_mutex_);
    connections_.emplace_back(conn);
}

void Server::removeConnection(Connection *conn) {
    if (verbose_)
        myerr << "removing " << conn->ipv4();
    LockGuard lock(threads_mutex_);
    connections_.erase(std::remove(connections_.begin(), connections_.end(), conn), connections_.end());
}

size_t Server::getConnectionsCount() const {
    return connections_.size();
}

std::shared_ptr<p18::response_type::BaseResponse> Server::executeCommand(p18::CommandType commandType, std::vector<std::string>& arguments) {
    LockGuard lock(client_mutex_);

    auto it = cache_.find(commandType);
    if (it != cache_.end()) {
        auto cr = it->second;
        if (voltronic::timestamp() - cr.time <= cacheTimeout_ && arguments == cr.arguments) {
            return cr.response;
        }

        cache_.erase(it);
    }

    if (delay_ != 0 && endExecutionTime_ != 0) {
        u64 now = voltronic::timestamp();
        u64 diff = now - endExecutionTime_;

        if (diff < delay_)
            usleep((delay_ - diff) * 1000);
    }

    try {
        auto response = client_.execute(commandType, arguments);
        endExecutionTime_ = voltronic::timestamp();

        CachedResponse cr {
            .time = endExecutionTime_,
            .arguments = arguments,
            .response = response
        };
        cache_[commandType] = cr;

        return response;
    }
    catch (voltronic::DeviceError& e) {
        throw std::runtime_error("device error: " + std::string(e.what()));
    }
    catch (voltronic::TimeoutError& e) {
        throw std::runtime_error("timeout: " + std::string(e.what()));
    }
    catch (voltronic::InvalidDataError& e) {
        throw std::runtime_error("data is invalid: " + std::string(e.what()));
    }
    catch (p18::InvalidResponseError& e) {
        throw std::runtime_error("response is invalid: " + std::string(e.what()));
    }
}

}