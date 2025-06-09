#include "convex_hull.hpp"
#include <iostream>
#include <vector>
#include <deque>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <stdexcept>

constexpr int NUM_RUNS = 1000;

int main() {
    try {
        int n;
        std::cout << "Enter number of points:" << std::endl;
        std::cin >> n;
        std::cin.ignore();

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

        // Vector-based
        double total_vec = 0.0;
        float area_vec = 0.0f;
        for (int i = 0; i < NUM_RUNS; ++i) {
            auto points_vec = points; // copy for fair test
            auto start_vec = std::chrono::high_resolution_clock::now();
            std::vector<Point> hull_vec = convex_hull_vector(points_vec);
            area_vec = convex_hull_area_vector(hull_vec);
            auto end_vec = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed_vec = end_vec - start_vec;
            total_vec += elapsed_vec.count();
        }

        // Deque-based
        double total_deque = 0.0;
        float area_deque = 0.0f;
        for (int i = 0; i < NUM_RUNS; ++i) {
            auto points_deque = points; // copy for fair test
            auto start_deque = std::chrono::high_resolution_clock::now();
            std::deque<Point> hull_deque = convex_hull_deque(points_deque);
            area_deque = convex_hull_area_deque(hull_deque);
            auto end_deque = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed_deque = end_deque - start_deque;
            total_deque += elapsed_deque.count();
        }

        std::cout << "Vector-based convex hull area: " << area_vec
                  << " | Avg Time: " << (total_vec / NUM_RUNS) << " ms" << std::endl;
        std::cout << "Deque-based convex hull area: " << area_deque
                  << " | Avg Time: " << (total_deque / NUM_RUNS) << " ms" << std::endl;

        if (total_vec < total_deque)
            std::cout << "Vector-based implementation is faster on average." << std::endl;
        else if (total_vec > total_deque)
            std::cout << "Deque-based implementation is faster on average." << std::endl;
        else
            std::cout << "Both implementations have the same average speed." << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}