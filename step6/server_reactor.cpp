#include "server_reactor.hpp"
#include "reactor.hpp"
#include "convex_hull.hpp"
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
#include <fcntl.h>

#define BACKLOG 10
#define BUFSIZE 1024

static std::vector<Point> points;
static std::map<int, int> points_to_read; // fd -> points left to read

// Global reactor pointer so callbacks can access it
static void* global_reactor = nullptr;

std::string handle_command(const std::string& cmdline);

void handle_client(int fd);

void handle_new_connection(int listener_fd) {
    while (true) {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        int newfd = accept(listener_fd, (struct sockaddr*)&clientaddr, &addrlen);
        if (newfd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more connections to accept right now
                break;
            } else {
                perror("accept");
                break;
            }
        }
        // Optionally set non-blocking (not required for this simple server)
        // int flags = fcntl(newfd, F_GETFL, 0);
        // fcntl(newfd, F_SETFL, flags | O_NONBLOCK);

        std::cout << "[SERVER] New client connected: fd=" << newfd << std::endl;
        std::string welcome = "Welcome to the Convex Hull Server!\n";
        send(newfd, welcome.c_str(), welcome.size(), 0);
        addFdToReactor(global_reactor, newfd, handle_client);
    }
}

void handle_client(int fd) {
    char buf[BUFSIZE];
    int nbytes = recv(fd, buf, sizeof(buf) - 1, 0);
    if (nbytes <= 0) {
        if (nbytes == 0) {
            std::cout << "Socket " << fd << " hung up\n";
        } else {
            perror("recv");
        }
        removeFdFromReactor(global_reactor, fd);
        close(fd);
        points_to_read.erase(fd);
        return;
    }
    buf[nbytes] = '\0';


    std::istringstream iss(buf);
    std::string line;
    std::ostringstream response;
    while (std::getline(iss, line)) {
        if (line.empty()) continue;

        if (points_to_read.count(fd) && points_to_read[fd] > 0) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream point_iss(line);
            float x, y;
            std::ostringstream point_response;
            if (!(point_iss >> x >> y)) {
                point_response << "Invalid point format. Example: 1,2\n";
            } else {
                points.push_back({x, y});
                points_to_read[fd]--;
                if (points_to_read[fd] == 0) {
                    point_response << "Graph updated with " << points.size() << " points.\n";
                    points_to_read.erase(fd);
                } else {
                    point_response << "Point added. " << points_to_read[fd] << " more to go.\n";
                }
            }
            response << point_response.str();
            continue;
        }

        if (line.find("Newgraph") == 0) {
            std::istringstream liss(line);
            std::string cmd, n_str;
            liss >> cmd;
            if (!(liss >> n_str)) {
                response << "Invalid usage. Example: Newgraph 4\n";
                continue;
            }
            int n;
            std::istringstream n_iss(n_str);
            if (!(n_iss >> n) || n < 1) {
                response << "Invalid usage. Example: Newgraph 4\n";
                continue;
            }
            points.clear();
            points_to_read[fd] = n;
            response << "OK. Send " << n << " points (x,y per line):\n";
            continue;
        }

        response << handle_command(line);
    }
    std::string resp = response.str();
    send(fd, resp.c_str(), resp.size(), 0);
}

std::string handle_command(const std::string& cmdline) {
    std::istringstream iss(cmdline);
    std::string cmd;
    iss >> cmd;
    std::ostringstream response;

    if (cmd == "CH") {
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

void run_server_reactor(int port) {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
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
        exit(2);
    }

    if (listen(listener, BACKLOG) < 0) {
        perror("listen");
        exit(3);
    }
    fcntl(listener, F_SETFL, O_NONBLOCK);

    std::cout << "Server started on port " << port << std::endl;

    // Start the reactor and add the listener socket
    global_reactor = startReactor();
    addFdToReactor(global_reactor, listener, handle_new_connection);

    // Keep the main thread alive
    while (true) {
        sleep(1);
    }

    stopReactor(global_reactor);
    close(listener);
}