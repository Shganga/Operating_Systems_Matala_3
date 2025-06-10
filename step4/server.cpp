#include "server.hpp"
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

#define BACKLOG 10
#define BUFSIZE 1024

static std::vector<Point> points;
static std::map<int, int> points_to_read; 

std::string handle_command(const std::string& cmdline) {
    std::istringstream iss(cmdline);
    std::string cmd;
    iss >> cmd;
    std::ostringstream response;

    if (cmd == "Newgraph") {
        int n;
        if (!(iss >> n) || n < 1) {
            response << "Invalid usage. Example: Newgraph 4\n";
            return response.str();
        }
        points.clear();
        response << "OK. Send " << n << " points (x,y per line):\n";
        return response.str();
    }else if (cmd == "CH") {
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

void run_server(int port) {
    int listener, newfd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE];
    fd_set master, read_fds;
    int fdmax;

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
        return;
    }

    if (listen(listener, BACKLOG) < 0) {
        perror("listen");
        return;
    }

    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;

    std::cout << "Server started on port " << port << std::endl;

    while (true) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            return;
        }

        for (int i = 0; i <= fdmax; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    // New connection
                    addrlen = sizeof(clientaddr);
                    newfd = accept(listener, (struct sockaddr*)&clientaddr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax) fdmax = newfd;
                        std::string welcome = "Welcome to the Convex Hull Server!\n";
                        send(newfd, welcome.c_str(), welcome.size(), 0);
                    }
                } else {
                    int nbytes = recv(i, buf, sizeof(buf) - 1, 0);
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            std::cout << "Socket " << i << " hung up\n";
                        } else {
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &master);
                        points_to_read.erase(i);
                    } else {
                        buf[nbytes] = '\0';
                        std::istringstream iss(buf);
                        std::string line;
                        std::ostringstream response;
                        while (std::getline(iss, line)) {
                            if (line.empty()) continue;

                            if (points_to_read.count(i) && points_to_read[i] > 0) {
                                std::replace(line.begin(), line.end(), ',', ' ');
                                std::istringstream point_iss(line);
                                float x, y;
                                std::ostringstream point_response;
                                if (!(point_iss >> x >> y)) {
                                    point_response << "Invalid point format. Example: 1,2\n";
                                } else {
                                    points.push_back({x, y});
                                    points_to_read[i]--;
                                    if (points_to_read[i] == 0) {
                                        point_response << "Graph updated with " << points.size() << " points.\n";
                                        points_to_read.erase(i);
                                    } else {
                                        point_response << "Point added. " << points_to_read[i] << " more to go.\n";
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
                                points_to_read[i] = n;
                                response << "OK. Send " << n << " points (x,y per line):\n";
                                continue;
                            }

                            response << handle_command(line);
                        }
                        std::string resp = response.str();
                        send(i, resp.c_str(), resp.size(), 0);
                    }
                }
            }
        }
    }
}