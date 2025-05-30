#ifndef PHYSICS_UTILS_H
#define PHYSICS_UTILS_H

#include "model/geometry.h"
#include "obb.h"
#include <stdbool.h>
#include <math.h>

void physics_resolve_overlap(Point *position_to_correct, const Vector *mtv_normal, float penetration_depth);
void physics_apply_bounce(Vector *velocity, float *current_speed, Vector *forward_direction, const Vector *surface_normal, float restitution_coefficient);

#endif //PHYSICS_UTILS_H
