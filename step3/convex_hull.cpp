#include "convex_hull.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

static float cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Point> convex_hull(std::vector<Point>& points) {
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

float convex_hull_area(const std::vector<Point>& hull) {
    if (hull.size() < 3) {
        // Not enough points to form a polygon
        return 0.0f;
    }
    float area = 0.0f;
    size_t n = hull.size();
    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = hull[i];
        const Point& p2 = hull[(i + 1) % n];
        area += (p1.x * p2.y) - (p2.x * p1.y);
    }
    return std::abs(area) * 0.5f;
}