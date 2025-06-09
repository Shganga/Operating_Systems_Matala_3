#include "convex_hull.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>

int main() {
    try {
        int n;
        std::cout << "Enter number of points:" << std::endl;
        std::cin >> n;
        std::cin.ignore(); // Ignore the rest of the line after the number

        std::vector<Point> points;
        std::cout << "Enter each point as x,y (one per line):" << std::endl;
        for (int i = 0; i < n; ++i) {
            std::string line;
            std::getline(std::cin, line);
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream iss(line);
            float x, y;
            iss >> x >> y;
            points.push_back({x, y});
        }

        std::vector<Point> hull = convex_hull(points);
        float area = convex_hull_area(hull);

        std::cout << "Convex hull area: " << area << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}