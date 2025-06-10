#include "client.hpp"
#include <iostream>

int main() {
    std::string host = "127.0.0.1";
    std::string port = "9034";
    int sockfd = connect_to_server(host, port);
    if (sockfd == -1) {
        std::cerr << "Failed to connect to server." << std::endl;
        return 1;
    }
    run_client(sockfd);
    return 0;
}