#include "reactor.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000

// --- Server side ---
void* global_reactor = nullptr;

void on_client(int fd) {
    char buf[1024];
    int n = read(fd, buf, sizeof(buf)-1);
    if (n <= 0) {
        std::cout << "Client disconnected: fd=" << fd << std::endl;
        removeFdFromReactor(global_reactor, fd);
        close(fd);
        return;
    }
    buf[n] = 0;
    std::cout << "Received from client: " << buf;
    write(fd, buf, n); // Echo back
}

void on_new_connection(int listener_fd) {
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(listener_fd, (sockaddr*)&client_addr, &addrlen);
    if (client_fd >= 0) {
        std::cout << "New client: fd=" << client_fd << std::endl;
        addFdToReactor(global_reactor, client_fd, on_client);
    }
}

// --- Client side ---
void run_client() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to connect\n";
        return;
    }

    std::cout << "Connected to echo server. Type messages:\n";
    char buf[1024];
    while (true) {
        std::string msg;
        std::getline(std::cin, msg);
        if (msg == "exit") break;
        msg += "\n";
        write(sock, msg.c_str(), msg.size());
        int n = read(sock, buf, sizeof(buf)-1);
        if (n <= 0) break;
        buf[n] = 0;
        std::cout << "Echo: " << buf;
    }
    close(sock);
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "server") {
        int listener = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(PORT);

        bind(listener, (sockaddr*)&addr, sizeof(addr));
        listen(listener, 10);

        std::cout << "Echo server running on port " << PORT << std::endl;
        global_reactor = startReactor();
        addFdToReactor(global_reactor, listener, on_new_connection);

        // Main event loop
        while (true) {
            runReactor(global_reactor); // This will block and handle all events
        }
        stopReactor(global_reactor);
        close(listener);
    } else {
        run_client();
    }
    return 0;
}