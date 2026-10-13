#pragma once
#include <string>

namespace common_cfg
{
    constexpr int MAX_CONNECTIONS = 10;
    constexpr int DEFAULT_PORT = 8888;
    constexpr int BUFFER_SIZE = 8192;

    struct ClientInfo {
        int sockfd;
        std::string ip;
        int port;
    };

} // namespace common_cfg
