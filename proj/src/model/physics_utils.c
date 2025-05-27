#include <lcom/lcf.h>
#include "physics_utils.h"

void physics_resolve_overlap(Point *position_to_correct, const Vector *mtv_normal, float penetration_depth) {
    if (!position_to_correct || !mtv_normal || penetration_depth <= 0.0f) {
        return;
    }

    position_to_correct->x += mtv_normal->x * penetration_depth;
    position_to_correct->y += mtv_normal->y * penetration_depth;
}

void physics_apply_bounce(Vector *velocity, float *current_speed, Vector *forward_direction, const Vector *surface_normal, float restitution_coefficient) {
    if (!velocity || !current_speed || !forward_direction || !surface_normal) {
        return;
    }

    float vel_along_normal = vector_dot_product(velocity, surface_normal);

    if (vel_along_normal < 0) {
        float j = -(1.0f + restitution_coefficient) * vel_along_normal;

        velocity->x += j * surface_normal->x;
        velocity->y += j * surface_normal->y;

        vector_init(velocity, velocity->x, velocity->y);
        *current_speed = velocity->magnitude;

        if (*current_speed > 0.001f) {
            forward_direction->x = velocity->x / *current_speed;
            forward_direction->y = velocity->y / *current_speed;
            vector_normalize(forward_direction);
        } else {
            *current_speed = 0.0f;
        }
    }
}
