#include <lcom/lcf.h>
#include "geometry.h"

void point_init(Point *point, float x, float y) {
    point->x = x;
    point->y = y;
}

float point_dist_sq(const Point *p1, const Point *p2) {
    if (!p1 || !p2) return 0.0f;
    float dx = p1->x - p2->x;
    float dy = p1->y - p2->y;
    return dx * dx + dy * dy;
}

float point_dist(const Point *p1, const Point *p2) {
    return sqrtf(point_dist_sq(p1, p2));
}

void vector_init(Vector *vector, float x, float y) {
    if (!vector) return;
    vector->x = x;
    vector->y = y;
    vector->magnitude = sqrtf(x * x + y * y);
}

float vector_dot_product(const Vector *v1, const Vector *v2) {
    return (v1->x * v2->x + v1->y * v2->y);
}

void vector_normalize(Vector *vector) {
    if (!vector) return;
    vector->magnitude = sqrtf(vector->x * vector->x + vector->y * vector->y);

    if (vector->magnitude < 0.00001f) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        return;
    }
    vector->x /= vector->magnitude;
    vector->y /= vector->magnitude;
    vector->magnitude = 1.0f;
}

void vector_init_with_points(Vector *vector, const Point *p1, const Point *p2) {
    if (!vector || !p1 || !p2) return;
    vector->x = p2->x - p1->x;
    vector->y = p2->y - p1->y;
    vector->magnitude = sqrtf(vector->x * vector->x + vector->y * vector->y);
}
