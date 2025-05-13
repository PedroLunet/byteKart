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
void vector_init(Vector *vector, float x, float y);
float vector_dot_product(Vector *v1, Vector *v2);
void vector_normalize(Vector *vector);

#endif //GEOMETRY_H
