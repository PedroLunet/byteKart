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

    // printf("Current Speed: %d, Velocity: (%d, %d)\n", (int)*current_speed, (int)velocity->x, (int)velocity->y);
    float vel_along_normal = vector_dot_product(velocity, surface_normal);

    if (vel_along_normal < 0) {
      /*
        float j = -velocity->magnitude * restitution_coefficient;

        velocity->x += j * surface_normal->x;
        velocity->y += j * surface_normal->y;

        vector_init(velocity, velocity->x, velocity->y);
        *current_speed = -velocity->magnitude;
       */
      *current_speed = -300.0f;
    }
}
