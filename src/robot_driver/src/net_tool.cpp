#include "net_tool.hpp"

namespace net_tool {
    bool create_tcp_socket(int& sockfd){
        sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "[ERROR] socket create failed\n";
            return false;
        }
        return true;
    }

    bool set_socket_nodelay(int sockfd) {
        int opt = 1;
        if (::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
            std::cerr << "[ERROR] TCP_NODELAY failed\n";
            return false;
        }
        return true;
    }

    bool set_socket_timeout(int sockfd, int sec, int usec) {
        struct timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        if (::setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            std::cerr << "[ERROR] SO_RCVTIMEO failed\n";
            return false;
        }
        return true;
    }

    bool bind_socket(int sockfd, uint16_t port) {
        sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);
        if (::bind(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            std::cerr << "[ERROR] bind port " << port << " failed\n";
            return false;
        }
        return true;
    }

    bool listen_socket(int sockfd, int backlog) {
        if (::listen(sockfd, backlog) < 0) {
            std::cerr << "[ERROR] listen failed\n";
            return false;
        }
        return true;
    }

    int accept_socket(int server_fd, sockaddr_in& client_addr) {
        socklen_t client_len = sizeof(client_addr);
        return ::accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    }

    void close_socket(int sockfd) {
        if (sockfd >= 0) {
            ::close(sockfd);
        }
    }

    ssize_t send_data(int sockfd, const std::string& data) {
        return ::send(sockfd, data.c_str(), data.size(), 0);
    }
    ssize_t recv_data(int sockfd, char* buffer, size_t len) {
        return ::recv(sockfd, buffer, len - 1, 0);
    }

    std::string get_ip_str(const sockaddr_in& addr) {
        char buf[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
        return std::string(buf);
    }

    uint16_t get_port(const sockaddr_in& addr) {
        return ntohs(addr.sin_port);
    }
}