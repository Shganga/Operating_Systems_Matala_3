#include "client.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

int connect_to_server(const std::string& host, const std::string& port) {
    struct addrinfo hints{}, *servinfo, *p;
    int rv, sockfd = -1;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return -1;
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            sockfd = -1;
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);
    return sockfd;
}

void run_client(int sockfd) {
    char buf[1024];
    ssize_t numbytes = recv(sockfd, buf, sizeof(buf) - 1, 0);
    if (numbytes > 0) {
        buf[numbytes] = '\0';
        std::cout << buf;
    }

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "Exit") break;
        line += "\n";
        if (send(sockfd, line.c_str(), line.size(), 0) == -1) {
            std::cerr << "Send failed.\n";
            break;
        }
        numbytes = recv(sockfd, buf, sizeof(buf) - 1, 0);
        if (numbytes <= 0) {
            std::cout << "Server closed connection.\n";
            break;
        }
        buf[numbytes] = '\0';
        std::cout << buf;
    }

    close(sockfd);
    std::cout << "Disconnected.\n";
}