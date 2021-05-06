// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_SERVER_TCP_SERVER_H
#define INVERTER_TOOLS_SERVER_TCP_SERVER_H

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <csignal>
#include <atomic>
#include <netinet/in.h>

#include "connection.h"
#include "../numeric_types.h"
#include "../formatter/formatter.h"
#include "../p18/client.h"
#include "../p18/types.h"
#include "../voltronic/device.h"
#include "../voltronic/time.h"

namespace server {

typedef std::lock_guard<std::mutex> LockGuard;

class Connection;

struct CachedResponse {
    u64 time;
    std::shared_ptr<p18::response_type::BaseResponse> response;
};

class Server {
private:
    int sock_;
    std::string host_;
    int port_;
    bool verbose_;
    p18::Client client_;
    std::shared_ptr<voltronic::Device> device_;

    u64 cacheTimeout_;
    std::map<p18::CommandType, CachedResponse> cache_;

    std::mutex threads_mutex_;
    std::mutex client_mutex_;

    std::vector<Connection*> connections_;

public:
    static const u64 CACHE_TIMEOUT = 1000;

    volatile std::atomic<bool> sigCaught = 0;

    explicit Server(std::shared_ptr<voltronic::Device> device);
    ~Server();

    void setVerbose(bool verbose);
    void setCacheTimeout(u64 timeout);
    void start(std::string& host, int port);

    bool verbose() const { return verbose_; }
    void addConnection(Connection* conn);
    void removeConnection(Connection* conn);
    size_t getConnectionsCount() const;

    std::shared_ptr<p18::response_type::BaseResponse> executeCommand(p18::CommandType commandType, std::vector<std::string>& arguments);
};


class ServerError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}

#endif //INVERTER_TOOLS_SERVER_TCP_SERVER_H
