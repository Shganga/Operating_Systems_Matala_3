#pragma once
#include <vector>

struct Point {
    float x, y;
    bool operator<(const Point& other) const;
};

std::vector<Point> convex_hull(std::vector<Point>& points);
float convex_hull_area(const std::vector<Point>& hull);