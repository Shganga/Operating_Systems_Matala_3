#include "reactor_proactor.hpp"
#include <map>
#include <set>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include <atomic>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

struct Reactor {
    std::map<int, reactorFunc> fd_to_func;
    std::set<int> fds;
    bool running = false;
};

void* startReactor() {
    Reactor* reactor = new Reactor();
    reactor->running = false;
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
    reactor->running = true; 
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

struct ProactorState {
    int listenfd;
    proactorFunc func;
    std::atomic<bool> running;
};

void* proactor_accept_loop(void* arg) {
    ProactorState* state = static_cast<ProactorState*>(arg);
    state->running = true;
    while (state->running) {
        int client_fd = accept(state->listenfd, nullptr, nullptr);
        if (client_fd < 0) {
            if (!state->running) break;
            usleep(10000); // Sleep 10ms to avoid busy loop on error
            continue;
        }
        // Spawn a thread for the client
        pthread_t tid;
        int* pfd = new int(client_fd); // Pass fd by pointer to avoid race
        pthread_create(&tid, nullptr, [](void* arg) -> void* {
            int fd = *static_cast<int*>(arg);
            delete static_cast<int*>(arg);
            // User's handler
            extern proactorFunc global_proactor_func;
            return global_proactor_func(fd);
        }, pfd);
        pthread_detach(tid);
    }
    delete state;
    return nullptr;
}

proactorFunc global_proactor_func = nullptr;

pthread_t startProactor(int sockfd, proactorFunc threadFunc) {
    global_proactor_func = threadFunc;
    ProactorState* state = new ProactorState;
    state->listenfd = sockfd;
    state->func = threadFunc;
    state->running = true;
    pthread_t tid;
    pthread_create(&tid, nullptr, proactor_accept_loop, state);
    return tid;
}

int stopProactor(pthread_t tid) {
    return pthread_cancel(tid);
}
