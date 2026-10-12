#pragma once
#include <atomic>

class TcpService {
    public:
        TcpService();
        ~TcpService();
        bool start(uint16_t port);

    private:
        uint16_t port_;
        std::atomic<bool> running_;
};
