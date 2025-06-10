#include "convex_hull.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

// Comparison operator for Point
bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

static float cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Vector-based convex hull
std::vector<Point> convex_hull_vector(std::vector<Point>& points) {
    if (points.size() < 3) {
        throw std::invalid_argument("At least 3 points are required to compute a convex hull.");
    }
    size_t n = points.size(), k = 0;
    std::sort(points.begin(), points.end());
    std::vector<Point> hull(2 * n);

    // Lower hull
    for (size_t i = 0; i < n; ++i) {
        while (k >= 2 && cross(hull[k-2], hull[k-1], points[i]) <= 0) k--;
        hull[k++] = points[i];
    }
    // Upper hull
    for (size_t i = n - 1, t = k + 1; i > 0; --i) {
        while (k >= t && cross(hull[k-2], hull[k-1], points[i-1]) <= 0) k--;
        hull[k++] = points[i-1];
    }
    hull.resize(k - 1);
    return hull;
}

float convex_hull_area_vector(const std::vector<Point>& hull) {
    if (hull.size() < 3) return 0.0f;
    float area = 0.0f;
    size_t n = hull.size();
    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = hull[i];
        const Point& p2 = hull[(i + 1) % n];
        area += (p1.x * p2.y) - (p2.x * p1.y);
    }
    return std::abs(area) * 0.5f;
}

// Deque-based convex hull
std::deque<Point> convex_hull_deque(std::vector<Point>& points) {
    if (points.size() < 3) {
        throw std::invalid_argument("At least 3 points are required to compute a convex hull.");
    }
    size_t n = points.size();
    std::sort(points.begin(), points.end());
    std::deque<Point> hull;

    // Lower hull
    for (size_t i = 0; i < n; ++i) {
        while (hull.size() >= 2 && cross(hull[hull.size()-2], hull[hull.size()-1], points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }
    // Upper hull
    size_t t = hull.size() + 1;
    for (size_t i = n - 1; i > 0; --i) {
        while (hull.size() >= t && cross(hull[hull.size()-2], hull[hull.size()-1], points[i-1]) <= 0)
            hull.pop_back();
        hull.push_back(points[i-1]);
    }
    hull.pop_back(); // Remove duplicate
    return hull;
}

float convex_hull_area_deque(const std::deque<Point>& hull) {
    if (hull.size() < 3) return 0.0f;
    float area = 0.0f;
    size_t n = hull.size();
    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = hull[i];
        const Point& p2 = hull[(i + 1) % n];
        area += (p1.x * p2.y) - (p2.x * p1.y);
    }
    return std::abs(area) * 0.5f;
}