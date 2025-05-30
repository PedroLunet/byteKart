#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdint.h>
#include <math.h>

typedef struct {
    float x;
    float y;
} Point;

typedef struct {
  float x;
  float y;
  float magnitude;
} Vector;

void point_init(Point *point, float x, float y);
float point_dist_sq(const Point *p1, const Point *p2);
float point_dist(const Point *p1, const Point *p2);

void vector_init(Vector *vector, float x, float y);
float vector_dot_product(const Vector *v1, const Vector *v2);
void vector_normalize(Vector *vector);
void vector_init_with_points(Vector *vector, const Point *p1, const Point *p2);

#endif //GEOMETRY_H
