#pragma once
namespace net_tool {
    bool create_tcp_socket(int& sockfd);
    bool set_socket_reuseaddr(int sockfd);
    
}