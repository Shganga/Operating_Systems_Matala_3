#include "reactor_proactor.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>


void* client_handler(int client_fd) {
    char buf[1024];
    ssize_t n;
    std::cout << "Client thread started for fd=" << client_fd << std::endl;
    while ((n = recv(client_fd, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = '\0';
        std::cout << "Received from client (fd=" << client_fd << "): " << buf;
        send(client_fd, buf, n, 0); 
    }
    std::cout << "Client thread exiting for fd=" << client_fd << std::endl;
    close(client_fd);
    return nullptr;
}

int main() {
    int port = 9034;
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        return 1;
    }

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind");
        close(listener);
        return 2;
    }

    if (listen(listener, 10) < 0) {
        perror("listen");
        close(listener);
        return 3;
    }

    std::cout << "Proactor server started on port " << port << std::endl;

    pthread_t proactor_tid = startProactor(listener, client_handler);

    pthread_join(proactor_tid, nullptr);

    close(listener);
    return 0;
}