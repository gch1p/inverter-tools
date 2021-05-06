// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_CONNECTION_H
#define INVERTER_TOOLS_CONNECTION_H

#include <thread>
#include <netinet/in.h>
#include <sstream>

#include "server.h"
#include "../formatter/formatter.h"

namespace server {

class Server;
struct Response;

struct ConnectionOptions {
    ConnectionOptions()
        : version(1), format(formatter::Format::JSON)
    {}

    unsigned version;
    formatter::Format format;
};


class Connection {
private:
    int sock_;
    std::thread thread_;
    struct sockaddr_in addr_;
    Server* server_;
    ConnectionOptions options_;

public:
    explicit Connection(int sock, struct sockaddr_in addr, Server* server);
    ~Connection();
    void run();
    std::string ipv4() const;
    bool sendResponse(Response& resp) const;
    int readLoop(char* buf, size_t bufSize) const;
    bool writeLoop(const char* buf, size_t bufSize) const;
    Response processRequest(char* buf);
};


enum class RequestType {
    Version,
    Format,
    Execute,
    Raw,
};


enum class ResponseType {
    OK,
    Error
};


struct Response {
    ResponseType type;
    std::ostringstream buf;
};
std::ostream& operator<<(std::ostream& os, Response& resp);

}

#endif //INVERTER_TOOLS_CONNECTION_H
