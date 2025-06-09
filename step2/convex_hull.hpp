#pragma once
#include <vector>
#include <deque>

struct Point {
    float x, y;
    bool operator<(const Point& other) const;
};

// Vector-based convex hull
std::vector<Point> convex_hull_vector(std::vector<Point>& points);
float convex_hull_area_vector(const std::vector<Point>& hull);

// Deque-based convex hull
std::deque<Point> convex_hull_deque(std::vector<Point>& points);
float convex_hull_area_deque(const std::deque<Point>& hull);