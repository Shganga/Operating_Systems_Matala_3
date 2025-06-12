#include "server.hpp"
#include "convex_hull.hpp"
#include "reactor_proactor.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

#define BACKLOG 10
#define BUFSIZE 1024

static std::vector<Point> points;
static std::map<int, int> points_to_read;
static std::mutex points_mutex; 
static std::condition_variable_any ch_cond;

std::string handle_command(const std::string& cmdline) {
    std::istringstream iss(cmdline);
    std::string cmd;
    iss >> cmd;
    std::ostringstream response;

    std::lock_guard<std::mutex> lock(points_mutex);

    if (cmd == "Newgraph") {
        int n;
        if (!(iss >> n) || n < 1) {
            response << "Invalid usage. Example: Newgraph 4\n";
            return response.str();
        }
        points.clear();
        ch_cond.notify_all();
        response << "OK. Send " << n << " points (x,y per line):\n";
        return response.str();
    } else if (cmd == "CH") {
        try {
            if (points.size() < 3) {
                response << "Need at least 3 points to compute convex hull.\n";
            } else {
                std::vector<Point> hull = convex_hull(points);
                float area = convex_hull_area(hull);
                response << "Convex hull area: " << area << "\n";
            }
        } catch (const std::exception& ex) {
            response << "Error: " << ex.what() << "\n";
        }
        return response.str();
    } else if (cmd == "Newpoint") {
        std::string coords;
        if (!(iss >> coords)) {
            response << "Invalid usage. Example: Newpoint 1,2\n";
            return response.str();
        }
        std::replace(coords.begin(), coords.end(), ',', ' ');
        std::istringstream coord_iss(coords);
        float x, y;
        if (!(coord_iss >> x >> y)) {
            response << "Invalid usage. Example: Newpoint 1,2\n";
        } else {
            points.push_back({x, y});
            ch_cond.notify_all();
            response << "Point (" << x << "," << y << ") added.\n";
        }
        return response.str();
    } else if (cmd == "Removepoint") {
        std::string coords;
        if (!(iss >> coords)) {
            response << "Invalid usage. Example: Removepoint 1,2\n";
            return response.str();
        }
        std::replace(coords.begin(), coords.end(), ',', ' ');
        std::istringstream coord_iss(coords);
        float x, y;
        if (!(coord_iss >> x >> y)) {
            response << "Invalid usage. Example: Removepoint 1,2\n";
        } else {
            auto it = std::find_if(points.begin(), points.end(),
                [x, y](const Point& p) { return p.x == x && p.y == y; });
            if (it != points.end()) {
                points.erase(it);
                ch_cond.notify_all();
                response << "Point (" << x << "," << y << ") removed.\n";
            } else {
                response << "Point (" << x << "," << y << ") not found.\n";
            }
        }
        return response.str();
    } else {
        response << "Unknown command.\n";
        return response.str();
    }
}

void* client_thread(int client_fd) {
    char buf[BUFSIZE];
    ssize_t nbytes;
    std::string leftover;

    std::string welcome = "Welcome to the Convex Hull Server!\n";
    send(client_fd, welcome.c_str(), welcome.size(), 0);

    while ((nbytes = recv(client_fd, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[nbytes] = '\0';
        std::string input = leftover + std::string(buf);
        size_t pos = 0;
        std::string line;
        std::ostringstream response;

        while ((pos = input.find('\n')) != std::string::npos) {
            line = input.substr(0, pos);
            input.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty()) {
                bool handled = false;
                {
                    std::lock_guard<std::mutex> lock(points_mutex);
                    if (points_to_read[client_fd] > 0) {
                        // Parse as point
                        std::replace(line.begin(), line.end(), ',', ' ');
                        std::istringstream iss(line);
                        float x, y;
                        if (!(iss >> x >> y)) {
                            response << "Invalid point format. Example: 1,2\n";
                        } else {
                            points.push_back({x, y});
                            points_to_read[client_fd]--;
                            ch_cond.notify_all();
                            if (points_to_read[client_fd] == 0) {
                                response << "Graph updated with " << points.size() << " points.\n";
                            } else {
                                response << "Point added. " << points_to_read[client_fd] << " more to go.\n";
                            }
                        }
                        handled = true;
                    }
                }
                if (!handled) {
                    std::istringstream iss(line);
                    std::string cmd;
                    iss >> cmd;
                    if (cmd == "Newgraph") {
                        int n;
                        iss >> n;
                        {
                            std::lock_guard<std::mutex> lock(points_mutex);
                            points_to_read[client_fd] = n;
                        }
                    }
                    response << handle_command(line);
                }
            }
        }
        leftover = input; 

        std::string resp = response.str();
        if (!resp.empty()) {
            send(client_fd, resp.c_str(), resp.size(), 0);
        }
    }
    {
        std::lock_guard<std::mutex> lock(points_mutex);
        points_to_read.erase(client_fd);
    }
    close(client_fd);
    std::cout << "Client thread exiting (fd=" << client_fd << ")\n";
    return nullptr;
}

void ch_monitor_thread() {
    bool last_state = false;
    while (true) {
        std::unique_lock<std::mutex> lock(points_mutex);
        ch_cond.wait(lock, []{ return true; }); // Wake up on notify

        float area = 0.0f;
        try {
            if (points.size() >= 3) {
                std::vector<Point> hull = convex_hull(points);
                area = convex_hull_area(hull);
            }
        } catch (...) {}

        bool now_at_least_100 = (area >= 100.0f);
        if (now_at_least_100 && !last_state) {
            std::cout << "At Least 100 units belongs to CH" << std::endl;
        } else if (!now_at_least_100 && last_state) {
            std::cout << "At Least 100 units no longer belongs to CH" << std::endl;
        }
        last_state = now_at_least_100;
    }
}

void run_server(int port) {
    int listener;
    struct sockaddr_in serveraddr;

    // Start the CH monitor thread
    std::thread(ch_monitor_thread).detach();

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        return;
    }

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind");
        close(listener);
        return;
    }

    if (listen(listener, BACKLOG) < 0) {
        perror("listen");
        close(listener);
        return;
    }

    std::cout << "Server started on port " << port << std::endl;

    // Use the proactor to handle clients
    pthread_t proactor_tid = startProactor(listener, client_thread);

    // Wait for the proactor thread to finish (infinite loop)
    pthread_join(proactor_tid, nullptr);

    close(listener);
}