#include "reactor.hpp"
#include <map>
#include <set>
#include <sys/select.h>
#include <unistd.h>

struct Reactor {
    std::map<int, reactorFunc> fd_to_func;
    std::set<int> fds;
    bool running = false;
};

void* startReactor() {
    Reactor* reactor = new Reactor();
    reactor->running = false; // Only run when runReactor() is called
    return reactor;
}

int addFdToReactor(void* reactor_ptr, int fd, reactorFunc func) {
    Reactor* reactor = static_cast<Reactor*>(reactor_ptr);
    reactor->fds.insert(fd);
    reactor->fd_to_func[fd] = func;
    return 0;
}

int removeFdFromReactor(void* reactor_ptr, int fd) {
    Reactor* reactor = static_cast<Reactor*>(reactor_ptr);
    reactor->fds.erase(fd);
    reactor->fd_to_func.erase(fd);
    return 0;
}

int stopReactor(void* reactor_ptr) {
    Reactor* reactor = static_cast<Reactor*>(reactor_ptr);
    reactor->running = false;
    return 0;
}

// Call this in your main loop to run the reactor (blocking)
void runReactor(void* reactor_ptr) {
    Reactor* reactor = static_cast<Reactor*>(reactor_ptr);
    reactor->running = true; // Start running now
    while (reactor->running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        int maxfd = 0;
        for (int fd : reactor->fds) {
            FD_SET(fd, &readfds);
            if (fd > maxfd) maxfd = fd;
        }
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int ready = select(maxfd + 1, &readfds, nullptr, nullptr, &tv);
        if (ready > 0) {
            // Copy fds to avoid iterator invalidation if a callback removes an fd
            std::set<int> fds_copy = reactor->fds;
            for (int fd : fds_copy) {
                if (FD_ISSET(fd, &readfds) && reactor->fd_to_func.count(fd)) {
                    reactor->fd_to_func[fd](fd);
                }
            }
        }
    }
}