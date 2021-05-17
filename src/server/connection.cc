// SPDX-License-Identifier: BSD-3-Clause

#include <stdexcept>
#include <unistd.h>
#include <ios>
#include <arpa/inet.h>
#include <cerrno>

#include "connection.h"
#include "../p18/commands.h"
#include "../p18/response.h"
#include "../logging.h"
#include "../common.h"
#include "hexdump/hexdump.h"
#include "signal.h"

#define CHECK_ARGUMENTS_LENGTH(__size__)          \
    if (arguments.size() != (__size__)) {   \
        std::ostringstream error;                \
        error << "invalid arguments count: expected " << (__size__) << ", got " << arguments.size(); \
        throw std::invalid_argument(error.str()); \
    }

#define CHECK_ARGUMENTS_MIN_LENGTH(__size__)          \
    if (arguments.size() < (__size__)) {   \
        std::ostringstream error;                \
        error << "invalid arguments count: expected " << (__size__) << ", got " << arguments.size(); \
        throw std::invalid_argument(error.str()); \
    }


namespace server {

Connection::Connection(int sock, struct sockaddr_in addr, Server* server)
    : sock_(sock), addr_(addr), server_(server)
{
    if (server_->verbose())
        mylog << "new connection from " << ipv4();

    thread_ = std::thread(&Connection::run, this);
    thread_.detach();
}

Connection::~Connection() {
    if (server_->verbose())
        mylog << "closing socket..";

    if (close(sock_) == -1)
        myerr << ipv4() << ": close: " << strerror(errno);

    server_->removeConnection(this);
}

void Connection::run() {
    static int bufSize = 2048;
    char buf[bufSize];

    while (true) {
        long rcvd = readLoop(buf, bufSize - 1);
        if (rcvd == -1) {
            if (errno != EINTR && server_->verbose())
                myerr << ipv4() << ": recv: " << std::string(strerror(errno));
            break;
        }
        if (rcvd == 0)
            break;

        buf[rcvd] = '\0';
        if (*buf == '\4')
            break;

        Response resp = processRequest(buf);
        if (!sendResponse(resp))
            break;
    }

    delete this;
}

int Connection::readLoop(char* buf, size_t bufSize) const {
    char* bufptr = buf;
    int left = static_cast<int>(bufSize);
    int readed = 0;

    while (left > 0) {
        size_t rcvd = recv(sock_, bufptr, left, 0);
        if (rcvd == -1)
            return -1;
        if (rcvd == 0)
            break;

        readed += static_cast<int>(rcvd);
        if (*bufptr == '\4')
            break;

        left -= static_cast<int>(rcvd);
        bufptr += rcvd;

        bufptr[rcvd] = '\0';
        char* ptr = strstr(buf, "\r\n");
        if (ptr)
            break;
    }

    return readed;
}

bool Connection::writeLoop(const char* buf, size_t bufSize) const {
    const char* bufptr = buf;
    int left = static_cast<int>(bufSize);

    while (left > 0) {
        size_t bytesSent = send(sock_, bufptr, left, 0);
        if (bytesSent == -1) {
            if (errno != EINTR && server_->verbose())
                myerr << ipv4() << ": send: " << std::string(strerror(errno));
            return false;
        }

        left -= static_cast<int>(bytesSent);
        bufptr += bytesSent;
    }

    return true;
}

bool Connection::sendResponse(Response& resp) const {
    std::ostringstream sbuf;
    sbuf << resp;

    std::string s = sbuf.str();
    const char* buf = s.c_str();
    size_t bufSize = s.size();

    return writeLoop(buf, bufSize);
}

std::string Connection::ipv4() const {
    char ip[INET_ADDRSTRLEN] = {0};
    const char* result = inet_ntop(AF_INET, (const void*)&addr_.sin_addr, ip, sizeof(ip));
    if (result == nullptr)
        return "?";

    std::ostringstream buf;
    buf << ip << ":" << htons(addr_.sin_port);
    return buf.str();
}

Response Connection::processRequest(char* buf) {
    std::stringstream sbuf;
    int n = 0;
    std::vector<std::string> arguments;
    RequestType type;

    Response resp;
    resp.type = ResponseType::OK;

    try {
        char* last = nullptr;
        const char* delim = " ";
        for (char* token = strtok_r(buf, delim, &last);
                token != nullptr;
                token = strtok_r(nullptr, delim, &last)) {

            char* ptr = strstr(token, "\r\n");
            if (ptr)
                *ptr = '\0';

            if (!n++) {
                std::string s = std::string(token);

                if (s == "format")
                    type = RequestType::Format;

                else if (s == "v")
                    type = RequestType::Version;

                else if (s == "exec")
                    type = RequestType::Execute;

                else if (s == "raw")
                    type = RequestType::Raw;

                else
                    throw std::invalid_argument("invalid token: " + s);

            } else if (strlen(token) > 0)
                arguments.emplace_back(token);
        }

        switch (type) {
            case RequestType::Version: {
                CHECK_ARGUMENTS_LENGTH(1)
                auto v = static_cast<unsigned>(std::stoul(arguments[0]));
                if (v != 1)
                    throw std::invalid_argument("invalid protocol version");
                options_.version = v;
                break;
            }

            case RequestType::Format:
                CHECK_ARGUMENTS_LENGTH(1)
                options_.format = format_from_string(arguments[0]);
                break;

            case RequestType::Execute: {
                CHECK_ARGUMENTS_MIN_LENGTH(1)

                std::string& command = arguments[0];
                auto commandArguments = std::vector<std::string>();

                auto argumentsSlice = std::vector<std::string>(arguments.begin()+1, arguments.end());

                p18::CommandInput input{&argumentsSlice};
                p18::CommandType commandType = p18::validate_input(command, commandArguments, (void*)&input);

                auto response = server_->executeCommand(commandType, commandArguments);
                resp.buf << *(response->format(options_.format).get());

                break;
            }

            case RequestType::Raw: {
                throw std::runtime_error("not implemented");
//                CHECK_ARGUMENTS_LENGTH(1)
//                std::string& raw = arguments[0];
//
//                resp.type = ResponseType::Error;
//                resp.buf << "not implemented";
                break;
            }
        }
    }
    // we except std::invalid_argument and std::runtime_error
    catch (std::exception& e) {
        myerr << e.what();

        resp.type = ResponseType::Error;

        auto err = p18::response_type::ErrorResponse(e.what());
        resp.buf << *(err.format(options_.format));
    }

    return resp;
}

std::ostream& operator<<(std::ostream& os, Response& resp) {
    os << (resp.type == ResponseType::OK ? "ok" : "err");

    resp.buf.seekp(0, std::ios::end);
    size_t size = resp.buf.tellp();
    if (size) {
        resp.buf.seekp(0);
        os << "\r\n" << resp.buf.str();
    }

    return os << "\r\n\r\n";
}

}