#include "convex_hull.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>

void print_instructions() {
    std::cout << "Convex Hull Interactive Program\n";
    std::cout << "Available commands:\n";
    std::cout << "  Newgraph n         - Start a new graph with n points (enter n lines of x,y)\n";
    std::cout << "  CH                 - Compute and print the convex hull area\n";
    std::cout << "  Newpoint x y       - Add a new point (x,y)\n";
    std::cout << "  Removepoint x y    - Remove a point (x,y)\n";
    std::cout << "  Exit               - Exit the program\n";
    std::cout << "--------------------------------------------------\n";
}

int main() {
    std::vector<Point> points;
    std::string line;
    print_instructions();
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "Newgraph") {
            int n;
            if (!(iss >> n) || n < 1) {
                std::cout << "Invalid usage. Example: Newgraph 4\n";
                continue;
            }
            points.clear();
            std::cout << "Enter " << n << " points (x,y per line):\n";
            for (int i = 0; i < n; ++i) {
                if (!std::getline(std::cin, line)) {
                    std::cout << "Not enough points entered.\n";
                    break;
                }
                std::replace(line.begin(), line.end(), ',', ' ');
                std::istringstream point_iss(line);
                float x, y;
                if (!(point_iss >> x >> y)) {
                    std::cout << "Invalid point format. Example: 1,2\n";
                    --i; // retry this point
                    continue;
                }
                points.push_back({x, y});
            }
            std::cout << "Graph updated with " << points.size() << " points.\n";
        } else if (cmd == "CH") {
            try {
                if (points.size() < 3) {
                    std::cout << "Need at least 3 points to compute convex hull.\n";
                    continue;
                }
                std::vector<Point> hull = convex_hull(points);
                float area = convex_hull_area(hull);
                std::cout << "Convex hull area: " << area << std::endl;
            } catch (const std::exception& ex) {
                std::cout << "Error: " << ex.what() << std::endl;
            }
        } else if (cmd == "Newpoint") {
            std::string coords;
            if (!(iss >> coords)) {
                std::cout << "Invalid usage. Example: Newpoint 1,2\n";
                continue;
            }
            std::replace(coords.begin(), coords.end(), ',', ' ');
            std::istringstream coord_iss(coords);
            float x, y;
            if (!(coord_iss >> x >> y)) {
                std::cout << "Invalid usage. Example: Newpoint 1,2\n";
                continue;
            }
            points.push_back({x, y});
            std::cout << "Point (" << x << "," << y << ") added.\n";
        } else if (cmd == "Removepoint") {
        std::string coords;
        if (!(iss >> coords)) {
            std::cout << "Invalid usage. Example: Removepoint 1,2\n";
            continue;
        }
        std::replace(coords.begin(), coords.end(), ',', ' ');
        std::istringstream coord_iss(coords);
        float x, y;
        if (!(coord_iss >> x >> y)) {
            std::cout << "Invalid usage. Example: Removepoint 1,2\n";
            continue;
        }
        auto it = std::find_if(points.begin(), points.end(),
            [x, y](const Point& p) { return p.x == x && p.y == y; });
        if (it != points.end()) {
            points.erase(it);
            std::cout << "Point (" << x << "," << y << ") removed.\n";
        } else {
            std::cout << "Point (" << x << "," << y << ") not found.\n";
        }
        } else if (cmd == "Exit") {
            std::cout << "Exiting program. Goodbye!\n";
            break;
        } else if (cmd.empty()) {
            continue;
        } else {
            std::cout << "Unknown command. Type one of the following:\n";
            print_instructions();
        }
    }
    return 0;
}