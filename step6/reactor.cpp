#include "reactor.hpp"
#include <thread>
#include <map>
#include <set>
#include <atomic>
#include <sys/select.h>
#include <unistd.h>
#include <mutex>

struct Reactor {
    std::map<int, reactorFunc> fd_to_func;
    std::set<int> fds;
    std::atomic<bool> running{true};
    std::thread loop_thread;
    std::mutex mtx;
};

static void reactorLoop(Reactor* reactor) {
    while (reactor->running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        int maxfd = 0;

        std::set<int> fds_copy;
        {
            std::lock_guard<std::mutex> lock(reactor->mtx);
            for (int fd : reactor->fds) {
                FD_SET(fd, &readfds);
                if (fd > maxfd) maxfd = fd;
            }
            fds_copy = reactor->fds; // Copy the set!
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ready = select(maxfd + 1, &readfds, nullptr, nullptr, &tv);
        if (ready > 0) {
            for (int fd : fds_copy) {
                reactorFunc func = nullptr;
                {
                    std::lock_guard<std::mutex> lock(reactor->mtx);
                    if (reactor->fd_to_func.count(fd))
                        func = reactor->fd_to_func[fd];
                }
                if (func && FD_ISSET(fd, &readfds)) {
                    func(fd);
                }
            }
        }
    }
}

void* startReactor() {
    Reactor* reactor = new Reactor();
    reactor->loop_thread = std::thread(reactorLoop, reactor);
    return reactor;
}

int addFdToReactor(void* reactor_ptr, int fd, reactorFunc func) {
    Reactor* reactor = static_cast<Reactor*>(reactor_ptr);
    std::lock_guard<std::mutex> lock(reactor->mtx);
    reactor->fds.insert(fd);
    reactor->fd_to_func[fd] = func;
    return 0;
}

int removeFdFromReactor(void* reactor_ptr, int fd) {
    Reactor* reactor = static_cast<Reactor*>(reactor_ptr);
    std::lock_guard<std::mutex> lock(reactor->mtx);
    reactor->fds.erase(fd);
    reactor->fd_to_func.erase(fd);
    return 0;
}

int stopReactor(void* reactor_ptr) {
    Reactor* reactor = static_cast<Reactor*>(reactor_ptr);
    reactor->running = false;
    if (reactor->loop_thread.joinable())
        reactor->loop_thread.join();
    delete reactor;
    return 0;
}