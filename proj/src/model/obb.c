// obb.c

#include "obb.h"
#include "model/geometry.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

void obb_update(OBB *obb, Point world_center, Vector forward_direction,
                float entity_half_width, float entity_half_height) {
    if (!obb) return;
    obb->center = world_center;
    obb->axis[1] = forward_direction;
    obb->axis[0].x = forward_direction.y;
    obb->axis[0].y = -forward_direction.x;
    vector_normalize(&obb->axis[0]);
    vector_normalize(&obb->axis[1]);

    obb->half_extent[0] = entity_half_width;
    obb->half_extent[1] = entity_half_height;
}

void obb_get_world_corners(const OBB *obb, Point corners[4]) {
    if (!obb) return;
    Vector ext_x_vec = {obb->axis[0].x * obb->half_extent[0], obb->axis[0].y * obb->half_extent[0], 0.0f};
    Vector ext_y_vec = {obb->axis[1].x * obb->half_extent[1], obb->axis[1].y * obb->half_extent[1], 0.0f};
    vector_init(&ext_x_vec, ext_x_vec.x, ext_x_vec.y);
    vector_init(&ext_y_vec, ext_y_vec.x, ext_y_vec.y);

    corners[0].x = obb->center.x + ext_x_vec.x + ext_y_vec.x;
    corners[0].y = obb->center.y + ext_x_vec.y + ext_y_vec.y;
    corners[1].x = obb->center.x - ext_x_vec.x + ext_y_vec.x;
    corners[1].y = obb->center.y - ext_x_vec.y + ext_y_vec.y;
    corners[2].x = obb->center.x - ext_x_vec.x - ext_y_vec.x;
    corners[2].y = obb->center.y - ext_x_vec.y - ext_y_vec.y;
    corners[3].x = obb->center.x + ext_x_vec.x - ext_y_vec.x;
    corners[3].y = obb->center.y + ext_x_vec.y - ext_y_vec.y;
}

static void project_obb_onto_axis(const OBB *obb, const Vector *axis, float *min_proj, float *max_proj) {
    Vector obb_center_vec = {obb->center.x, obb->center.y, 0.0f};
    vector_init(&obb_center_vec, obb_center_vec.x, obb_center_vec.y);

    float center_proj = vector_dot_product(&obb_center_vec, axis);
    float r_proj = obb->half_extent[0] * fabsf(vector_dot_product(&obb->axis[0], axis)) +
                   obb->half_extent[1] * fabsf(vector_dot_product(&obb->axis[1], axis));
    *min_proj = center_proj - r_proj;
    *max_proj = center_proj + r_proj;
}

static bool get_interval_overlap(float minA, float maxA, float minB, float maxB, float *overlap) {
    if (maxA < minB || maxB < minA) {
        if (overlap) *overlap = 0;
        return false;
    }
    if (overlap) {
        *overlap = fminf(maxA, maxB) - fmaxf(minA, minB);
    }
    return true;
}

void obb_check_collision_obb_vs_obb(const OBB *obb1, const OBB *obb2, CollisionInfo *info) {
    if (!obb1 || !obb2 || !info) {
        if(info) info->occurred = false;
        return;
    }
    info->occurred = false;
    info->penetration_depth = FLT_MAX;

    Vector test_axes[4];
    test_axes[0] = obb1->axis[0];
    test_axes[1] = obb1->axis[1];
    test_axes[2] = obb2->axis[0];
    test_axes[3] = obb2->axis[1];

    float min1, max1, min2, max2;
    float current_overlap;

    for (int i = 0; i < 4; ++i) {
        Vector current_axis = test_axes[i];
        project_obb_onto_axis(obb1, &current_axis, &min1, &max1);
        project_obb_onto_axis(obb2, &current_axis, &min2, &max2);

        if (!get_interval_overlap(min1, max1, min2, max2, &current_overlap)) {
            info->occurred = false;
            return;
        }
        if (current_overlap < info->penetration_depth) {
            info->penetration_depth = current_overlap;
            info->collision_normal = current_axis;
        }
    }
    info->occurred = true;
    Vector center_to_center = {obb2->center.x - obb1->center.x, obb2->center.y - obb1->center.y, 0.0f};
    vector_init(&center_to_center, center_to_center.x, center_to_center.y);
    if (vector_dot_product(&center_to_center, &info->collision_normal) < 0.0f) {
        info->collision_normal.x *= -1.0f;
        info->collision_normal.y *= -1.0f;
    }
}

void obb_check_collision_obb_vs_line_segment(const OBB *car_obb, Point p0, Point p1, CollisionInfo *info) {
    if (!car_obb || !info) {
        if(info) info->occurred = false;
        return;
    }
    info->occurred = false;

    OBB segment_obb;
    Vector segment_dir;
    segment_dir.x = p1.x - p0.x;
    segment_dir.y = p1.y - p0.y;
    vector_init(&segment_dir, segment_dir.x, segment_dir.y);

    if (segment_dir.magnitude < 0.001f) {
        info->occurred = false;
        return;
    }
    vector_normalize(&segment_dir);

    segment_obb.center.x = (p0.x + p1.x) / 2.0f;
    segment_obb.center.y = (p0.y + p1.y) / 2.0f;

    float segment_half_length = sqrtf(powf(p1.x - p0.x, 2) + powf(p1.y - p0.y, 2)) / 2.0f;
    float segment_half_thickness = 0.5f;

    obb_update(&segment_obb, segment_obb.center, segment_dir, segment_half_thickness, segment_half_length);

    obb_check_collision_obb_vs_obb(car_obb, &segment_obb, info);
}
