#include <vector>
#include <cstdio>
#include <algorithm>

struct Vec2f {
    float x, y;
};

Vec2f operator+(const Vec2f &vec1, const Vec2f &vec2) {
    return {vec1.x + vec2.x, vec1.y + vec2.y};
}

Vec2f operator-(const Vec2f &vec1, const Vec2f &vec2) {
    return {vec1.x - vec2.x, vec1.y - vec2.y};
}

float crossProduct(const Vec2f &a, const Vec2f &b) {
    return a.x * b.y - a.y * b.x;
}

// function for checking whether a polygon is clockwise or not, uses shoelace formula
bool isClockwise(const std::vector<Vec2f> &vertices) {
    float sum = 0.0;

    // goes through all the vertices and calculates the area of a parallelogram formed by the current and next vectors,
    // adds the area to the sum
    for (int i = 0; i < vertices.size(); ++i) {
        const Vec2f &current = vertices[i];
        const Vec2f &next = vertices[(i + 1) % vertices.size()];
        sum += (next.x - current.x) * (next.y + current.y);
    }

    // if sum is larger than 0, the polygon is defined in a clockwise order, otherwise it is in counter-clockwise order
    return sum > 0.0;
}

bool isConvex(const Vec2f &prev, const Vec2f &current, const Vec2f &next) {
    // calculate the edges connecting to the current point
    Vec2f prevEdge = prev - current;
    Vec2f nextEdge = next - current;

    // if the cross product is positive, the angle is convex
    return crossProduct(prevEdge, nextEdge) > 0.0f;
}

bool isPointInsideTriangle(const Vec2f &point, const Vec2f &prev, const Vec2f &current, const Vec2f &next) {
    // this uses barycentric coordinates for finding out if point is inside triangle

    /*
    Mathematically, if (alpha, beta, gamma) are the barycentric coordinates of a point with respect to a triangle,
    and (prev.x, prev.y), (current.x, current.y), and (next.x, next.y) are the coordinates of the three vertices (prev, current, and next),
    then the point (x, y) can be expressed as follows:

    point.x = alpha * prev.x + beta * current.x + gamma * next.x
    point.y = alpha * prev.y + beta * current.y + gamma * next.y

    That means that if all the barycentric coordinates of the point are positive, the point is inside the triangle
    */

    Vec2f prevToCurrent = current - prev;
    Vec2f currentToNext = next - current;
    Vec2f nextToPrev = prev - next;

    Vec2f prevToPoint = point - prev;
    Vec2f currentToPoint = point - current;
    Vec2f nextToPoint = point - next;

    float alpha = crossProduct(prevToCurrent, prevToPoint);
    float beta = crossProduct(currentToNext, currentToPoint);
    float gamma = crossProduct(nextToPrev, nextToPoint);

    if (alpha > 0.0f || beta > 0.0f || gamma > 0.0f) {
        return false;
    }

    return true;
}

bool
isEar(const std::vector<Vec2f> &vertices, unsigned int prevIndex, unsigned int currentIndex, unsigned int nextIndex) {
    // get the previous, current and next points of the polygon
    const Vec2f &prev = vertices[prevIndex];
    const Vec2f &current = vertices[currentIndex];
    const Vec2f &next = vertices[nextIndex];

    // check if no other vertices are inside the triangle formed by the vertex and its neighbours
    for (int j = 0; j < vertices.size(); ++j) {
        // check all the vertices except the current and its neighbours (previous and next)
        if (j != prevIndex && j != currentIndex && j != nextIndex) {
            // get the point that we want to test if it is inside the triangle
            const Vec2f &point = vertices[j];

            if (isPointInsideTriangle(point, prev, current, next)) {
                return false; // the triangle has a point inside -> it is not an ear
            }
        }
    }

    return true; // no points are inside the triangle -> it is an ear
}

std::vector<std::array<Vec2f, 3>> triangulate(std::vector<Vec2f> &vertices) {
    std::vector<std::array<Vec2f, 3>> triangles;

    // check if the polygon is cw
    if (!isClockwise(vertices)) {
        // reverse to be cw
        std::reverse(vertices.begin(), vertices.end());
    }

    // repeat until there is only one triangle left
    while (vertices.size() > 3) {
        bool earFound = false;

        // go through all vertices
        for (int i = 0; i < vertices.size(); ++i) {
            // get the previous, current and next indices, preventing out of bounds exception
            unsigned int prevIndex = (i - 1 + vertices.size()) % vertices.size();
            unsigned int currentIndex = i;
            unsigned int nextIndex = (i + 1) % vertices.size();

            // get the vertices from the indices
            const Vec2f &prev = vertices[prevIndex];
            const Vec2f &current = vertices[currentIndex];
            const Vec2f &next = vertices[nextIndex];

            // if the chosen points are convex and it is an ear
            if (isConvex(prev, current, next) && isEar(vertices, prevIndex, currentIndex, nextIndex)) {
                // add the triangle
                triangles.push_back({prev, current, next});
                // remove the vertex for which happened the check
                vertices.erase(vertices.begin() + i);
                earFound = true;
                break;
            }
        }

        if (!earFound) {
            // no ear found, break to avoid an infinite loop
            break;
        }
    }

    // add last triangle
    triangles.push_back({vertices[0], vertices[1], vertices[2]});

    // return triangulated polygon
    return triangles;
}


int main() {
    // Example usage
    std::vector<Vec2f> polygon = {
            {-1, -1},
            {-2, 1},
            {1,  1},
            {0,  0},
            {3,  -1},
    };

    std::vector<std::array<Vec2f, 3>> triangles1 = triangulate(polygon);
    // Output the result
    for (const auto &triangle: triangles1) {
        printf("Triangle: ");
        for (const auto &point: triangle) {
            printf("(%f, %f) ", point.x, point.y);
        }
        printf("\n");
    }

    return 0;
}