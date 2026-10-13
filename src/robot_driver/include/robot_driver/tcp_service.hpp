#pragma once
#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include "net_tool.hpp"
#include "common_cfg.hpp"

class TcpService {
public:
    TcpService();
    ~TcpService();

    bool start(uint16_t port);
    void stop();

    std::function<void(const uint8_t* data, size_t len)> on_data;

private:
    int server_fd_;
    uint16_t port_;
    std::atomic<bool> running_;

    std::thread accept_thread_;
    std::vector<std::thread> client_threads_;
    std::mutex clients_mtx_;

    void accept_loop();
    void recv_loop(int client_fd);
};