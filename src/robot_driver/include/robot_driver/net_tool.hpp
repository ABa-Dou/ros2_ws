#pragma once
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
namespace net_tool {
    bool create_tcp_socket(int& sockfd);
    bool set_socket_nodelay(int sockfd);
    bool set_socket_reuseaddr(int sockfd);
    bool set_socket_timeout(int sockfd, int sec, int usec);
    bool bind_socket(int sockfd, uint16_t port);
    bool listen_socket(int sockfd, int backlog);
    int accept_socket(int server_fd, sockaddr_in& client_addr);
    void close_socket(int sockfd);
    ssize_t send_data(int sockfd, const std::string& data);
    ssize_t recv_data(int sockfd, char* buffer, size_t len);
    std::string get_ip_str(const sockaddr_in& addr);
    uint16_t get_port(const sockaddr_in& addr);
}