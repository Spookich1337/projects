#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// --- Алгоритм Рабина-Карпа ---
std::vector<int> rabin_karp(const std::string& pattern, const std::string& text) {
    const long long BASE = 256;
    const long long PRIME = 1e9 + 7;

    int m = pattern.length();
    int n = text.length();
    long long pattern_hash = 0;
    long long text_hash = 0;
    long long h = 1;
    std::vector<int> result;

    if (m > n) return result;

    for (int i = 0; i < m - 1; i++) {
        h = (h * BASE) % PRIME;
    }

    for (int i = 0; i < m; i++) {
        pattern_hash = (BASE * pattern_hash + pattern[i]) % PRIME;
        text_hash = (BASE * text_hash + text[i]) % PRIME;
    }

    for (int i = 0; i <= n - m; i++) {
        if (pattern_hash == text_hash) {
            if (text.substr(i, m) == pattern) {
                result.push_back(i);
            }
        }

        if (i < n - m) {
            text_hash = (BASE * (text_hash - text[i] * h) + text[i + m]) % PRIME;
            if (text_hash < 0) text_hash += PRIME;
        }
    }
    return result;
}

struct Point {
    double x, y;
};

double area(const std::vector<Point>& points) {
    double res = 0.0;
    int n = points.size();
    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        res += points[i].x * points[next].y - points[i].y * points[next].x;
    }
    return std::abs(res) / 2.0;
}

double orientation(Point a, Point b, Point c) {
    return (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
}

std::vector<Point> graham(std::vector<Point> points) {
    int n = points.size();
    if (n < 3) return points;

    for (int i = 1; i < n; i++) {
        if (points[i].x < points[0].x) {
            std::swap(points[0], points[i]);
        }
    }

    for (int i = 2; i < n; i++) {
        int temp = i;
        while (temp > 1 && orientation(points[0], points[temp - 1], points[temp]) > 0) {
            std::swap(points[temp], points[temp - 1]);
            temp--;
        }
    }

    std::vector<Point> hull;
    hull.push_back(points[0]);
    hull.push_back(points[1]);

    for (int i = 2; i < n; i++) {
        while (hull.size() > 1 && orientation(hull[hull.size() - 2], hull.back(), points[i]) > 0) {
            hull.pop_back();
        }
        hull.push_back(points[i]);
    }

    return hull;
}

int main() {
    // Тест Рабина-Карпа
    std::string text = "ABABDABACDABABCABAB";
    std::string pattern = "ABABCABAB";
    std::vector<int> matches = rabin_karp(pattern, text);
    
    std::cout << "Rabin-Karp matches at indices: ";
    for (int idx : matches) std::cout << idx << " ";
    std::cout << "\n\n";

    // Тест алгоритма Грэхема
    std::vector<Point> pts = {{0, 3}, {1, 1}, {2, 2}, {4, 4}, {0, 0}, {1, 2}, {3, 1}, {3, 3}};
    std::vector<Point> hull = graham(pts);

    std::cout << "Convex Hull Points:\n";
    for (const auto& p : hull) {
        std::cout << "(" << p.x << ", " << p.y << ")\n";
    }
    
    std::cout << "Area: " << area(hull) << std::endl;

    return 0;
}