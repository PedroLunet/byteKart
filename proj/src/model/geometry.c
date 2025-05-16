#include <lcom/lcf.h>
#include "geometry.h"

void point_init(Point *point, float x, float y) {
    point->x = x;
    point->y = y;
}

void vector_init(Vector *vector, float x, float y) {
    if (!vector) return;
    vector->x = x;
    vector->y = y;
    vector->magnitude = sqrtf(x * x + y * y);
}

float vector_dot_product(Vector *v1, Vector *v2) {
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
