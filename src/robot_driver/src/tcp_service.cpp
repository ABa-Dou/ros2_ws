#include "tcp_service.hpp"

TcpService::TcpService()
    : server_fd_(-1), port_(0), running_(false)
{
}

TcpService::~TcpService()
{
    stop();
}

bool TcpService::start(uint16_t port)
{
    if (running_) {
        return false;
    }
    port_ = port;
    if (!net_tool::create_tcp_socket(server_fd_)) {
        return false;
    }
    if (!net_tool::set_socket_reuseaddr(server_fd_)) {
        net_tool::close_socket(server_fd_);
        server_fd_ = -1;
        return false;
    }
    if (!net_tool::bind_socket(server_fd_, port_)) {
        net_tool::close_socket(server_fd_);
        server_fd_ = -1;
        return false;
    }
    if (!net_tool::listen_socket(server_fd_, common_cfg::MAX_CONNECTIONS)) {
        net_tool::close_socket(server_fd_);
        server_fd_ = -1;
        return false;
    }
    running_ = true;
    accept_thread_ = std::thread(&TcpService::accept_loop, this);
    return true;
}

void TcpService::stop()
{
    if (!running_) return;
    running_ = false;

    if (server_fd_ >= 0) {
        net_tool::close_socket(server_fd_);
        server_fd_ = -1;
    }

    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }

    std::lock_guard<std::mutex> lock(clients_mtx_);
    for (auto& t : client_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    client_threads_.clear();
}

void TcpService::accept_loop()
{
    while (running_) {
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        int client_fd = net_tool::accept_socket(server_fd_, client_addr);
        if (client_fd < 0) {
            if (!running_) break;
            continue;
        }

        net_tool::set_socket_nodelay(client_fd);
        net_tool::set_socket_timeout(client_fd, 1, 0);

        std::lock_guard<std::mutex> lock(clients_mtx_);
        client_threads_.emplace_back(&TcpService::recv_loop, this, client_fd);
    }
}

void TcpService::recv_loop(int client_fd)
{
    char buf[common_cfg::BUFFER_SIZE];
    while (running_) {
        ssize_t n = net_tool::recv_data(client_fd, buf, sizeof(buf));
        if (n <= 0) {
            if (!running_) break;
            break;
        }
        if (on_data) {
            on_data(reinterpret_cast<const uint8_t*>(buf), static_cast<size_t>(n));
        }
    }
    net_tool::close_socket(client_fd);
}