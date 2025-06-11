#include "reactor.hpp"
#include <iostream>
#include <unistd.h>
#include <atomic>
#include <cstring>

std::atomic<bool> should_exit{false};

void on_stdin(int fd) {
    char buf[100];
    int n = read(fd, buf, sizeof(buf)-1);
    if (n > 0) {
        buf[n] = 0;
        std::cout << "Got input: " << buf;
        // Remove trailing newline for comparison
        std::string input(buf);
        if (!input.empty() && input.back() == '\n')
            input.pop_back();
        if (input == "exit") {
            should_exit = true;
        }
    }
}

int main() {
    std::cout << "Reactor demo: Type something and press Enter. Type 'exit' to quit.\n";
    void* reactor = startReactor();
    addFdToReactor(reactor, 0, on_stdin); // 0 = stdin
    while (!should_exit) {
        usleep(100000); // Sleep 0.1 sec to reduce busy-waiting
    }
    stopReactor(reactor);
    std::cout << "Reactor stopped. Exiting.\n";
    return 0;
}