#include <lcom/lcf.h>
#include "obb.h"

void obb_update(OBB *obb, Point world_center, Vector forward_direction, float entity_half_width, float entity_half_height) {
    if (!obb) return;
    obb->center = world_center;
    obb->axis[1] = forward_direction;
    obb->axis[0].x = forward_direction.y;
    obb->axis[0].y = -forward_direction.x;

    obb->half_extent[0] = entity_half_width;
    obb->half_extent[1] = entity_half_height;
}

void obb_get_world_corners(const OBB *obb, Point corners[4]) {
    if (!obb) return;
    Vector ext_x_vec, ext_y_vec;
    vector_init(&ext_x_vec, obb->axis[0].x * obb->half_extent[0], obb->axis[0].y * obb->half_extent[0]);
    vector_init(&ext_y_vec, obb->axis[1].x * obb->half_extent[1], obb->axis[1].y * obb->half_extent[1]);

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

static void project_segment_onto_axis(Point p0, Point p1, const Vector *axis, float *min_proj, float *max_proj) {
    Vector p0_vec = {p0.x, p0.y, 0.0f}; vector_init(&p0_vec, p0.x, p0.y);
    Vector p1_vec = {p1.x, p1.y, 0.0f}; vector_init(&p1_vec, p1.x, p1.y);

    float proj0 = vector_dot_product(&p0_vec, axis);
    float proj1 = vector_dot_product(&p1_vec, axis);

    if (proj0 < proj1) { *min_proj = proj0; *max_proj = proj1; }
    else { *min_proj = proj1; *max_proj = proj0; }
}

static bool get_interval_overlap(float minA, float maxA, float minB, float maxB, float *overlap) {
    if (maxA < minB || maxB < minA) {
        *overlap = 0;
        return false;
    }
    *overlap = fminf(maxA, maxB) - fmaxf(minA, minB);
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


void obb_check_collision_obb_vs_line_segment(const OBB *obb, Point p0, Point p1, CollisionInfo *info) {
    if (!obb || !info) {
        if(info) info->occurred = false;
        return;
    }

    info->occurred = false;
    info->penetration_depth = FLT_MAX;

    Vector test_axes[3];
    test_axes[0] = obb->axis[0];
    test_axes[1] = obb->axis[1];

    Vector edge_vec = { p1.x - p0.x, p1.y - p0.y, 0.0f};
    vector_init(&edge_vec, edge_vec.x, edge_vec.y);
    if (edge_vec.magnitude < 0.0001f) {
        // TODO: Implement OBB vs Point collision if necessary, or treat as no collision for a line.
        info->occurred = false;
        return;
    }

    test_axes[2].x = -edge_vec.y;
    test_axes[2].y = edge_vec.x;
    vector_init(&test_axes[2], test_axes[2].x, test_axes[2].y);
    vector_normalize(&test_axes[2]);

    float min_obb_proj, max_obb_proj, min_line_proj, max_line_proj;
    float current_overlap;

    for (int i = 0; i < 3; ++i) {
        Vector current_axis = test_axes[i];

        project_obb_onto_axis(obb, &current_axis, &min_obb_proj, &max_obb_proj);
        project_segment_onto_axis(p0, p1, &current_axis, &min_line_proj, &max_line_proj);

        if (!get_interval_overlap(min_obb_proj, max_obb_proj, min_line_proj, max_line_proj, &current_overlap)) {
            info->occurred = false;
            return;
        }
        if (current_overlap < info->penetration_depth) {
            info->penetration_depth = current_overlap;
            info->collision_normal = current_axis;
        }
    }

    info->occurred = true;

    Point line_midpoint = {(p0.x + p1.x) / 2.0f, (p0.y + p1.y) / 2.0f};
    Vector obb_center_to_line_mid = {line_midpoint.x - obb->center.x, line_midpoint.y - obb->center.y, 0.0f};
    vector_init(&obb_center_to_line_mid, obb_center_to_line_mid.x, obb_center_to_line_mid.y);

    if (vector_dot_product(&obb_center_to_line_mid, &info->collision_normal) < 0.0f) {
        info->collision_normal.x *= -1.0f;
        info->collision_normal.y *= -1.0f;
    }
}

