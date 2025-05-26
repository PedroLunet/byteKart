#include <lcom/lcf.h>
#include "obb.h"

void obb_update(OBB *obb, Point world_center, Vector forward_direction,
                float entity_half_width, float entity_half_height) {
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

    ext_x_vec.x = obb->axis[0].x * obb->half_extent[0];
    ext_x_vec.y = obb->axis[0].y * obb->half_extent[0];
    ext_y_vec.x = obb->axis[1].x * obb->half_extent[1];
    ext_y_vec.y = obb->axis[1].y * obb->half_extent[1];

    // Top-Right (center + ext_x + ext_y)
    corners[0].x = obb->center.x + ext_x_vec.x + ext_y_vec.x;
    corners[0].y = obb->center.y + ext_x_vec.y + ext_y_vec.y;

    // Bottom-Right (center + ext_x - ext_y)
    corners[1].x = obb->center.x + ext_x_vec.x - ext_y_vec.x;
    corners[1].y = obb->center.y + ext_x_vec.y - ext_y_vec.y;

    // Bottom-Left (center - ext_x - ext_y)
    corners[2].x = obb->center.x - ext_x_vec.x - ext_y_vec.x;
    corners[2].y = obb->center.y - ext_x_vec.y - ext_y_vec.y;

    // Top-Left (center - ext_x + ext_y)
    corners[3].x = obb->center.x - ext_x_vec.x + ext_y_vec.x;
    corners[3].y = obb->center.y - ext_x_vec.y + ext_y_vec.y;
}

static void project_obb_onto_axis(const OBB *obb, Vector *axis, float *min_proj, float *max_proj) {
    // Project OBB center onto the axis
    float center_proj = vector_dot_product((Vector*)&obb->center, axis);

    // Project half-extents onto the axis
    // The projected radius is the sum of the projections of the scaled local axes.
    float r_proj = obb->half_extent[0] * fabsf(vector_dot_product(&obb->axis[0], axis)) + obb->half_extent[1] * fabsf(vector_dot_product(&obb->axis[1], axis));

    *min_proj = center_proj - r_proj;
    *max_proj = center_proj + r_proj;
}

static void project_segment_onto_axis(Point p0, Point p1, Vector *axis, float *min_proj, float *max_proj) {
    float proj0 = vector_dot_product((Vector*)&p0, axis);
    float proj1 = vector_dot_product((Vector*)&p1, axis);

    if (proj0 < proj1) {
        *min_proj = proj0;
        *max_proj = proj1;
    } else {
        *min_proj = proj1;
        *max_proj = proj0;
    }
}

static bool check_interval_overlap(float minA, float maxA, float minB, float maxB) {
    return (maxA >= minB && maxB >= minA);
}


bool obb_check_collision_obb_vs_obb(const OBB *obb1, const OBB *obb2) {
    if (!obb1 || !obb2) return false;

    Vector test_axes[4];
    test_axes[0] = obb1->axis[0];
    test_axes[1] = obb1->axis[1];
    test_axes[2] = obb2->axis[0];
    test_axes[3] = obb2->axis[1];

    float min1, max1, min2, max2;

    for (int i = 0; i < 4; ++i) {
        project_obb_onto_axis(obb1, &test_axes[i], &min1, &max1);
        project_obb_onto_axis(obb2, &test_axes[i], &min2, &max2);

        if (!check_interval_overlap(min1, max1, min2, max2)) {
            return false;
        }
    }

    return true;
}


bool obb_check_collision_obb_vs_line_segment(const OBB *obb, Point p0, Point p1) {
    if (!obb) return false;

    Vector test_axes[3];
    test_axes[0] = obb->axis[0];
    test_axes[1] = obb->axis[1];

    Vector edge_vec = { p1.x - p0.x, p1.y - p0.y, 0.0f };
    vector_init(&edge_vec, edge_vec.x, edge_vec.y);
    if (edge_vec.magnitude < 0.0001f) return false;

    test_axes[2].x = -edge_vec.y;
    test_axes[2].y = edge_vec.x;
    vector_init(&test_axes[2], test_axes[2].x, test_axes[2].y);
    vector_normalize(&test_axes[2]);

    float min_obb_proj, max_obb_proj, min_line_proj, max_line_proj;

    for (int i = 0; i < 3; ++i) {
        project_obb_onto_axis(obb, &test_axes[i], &min_obb_proj, &max_obb_proj);
        project_segment_onto_axis(p0, p1, &test_axes[i], &min_line_proj, &max_line_proj);

        if (!check_interval_overlap(min_obb_proj, max_obb_proj, min_line_proj, max_line_proj)) {
            return false;
        }
    }
    return true;
}

