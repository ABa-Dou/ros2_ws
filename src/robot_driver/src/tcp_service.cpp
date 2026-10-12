#include "tcp_service.hpp"

bool TcpService::start(uint16_t port) {
    if (running_) {
        return false; // Already running
    }
    
    port_ = port;
    
    running_ = true;

    return true;
}

