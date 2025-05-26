#ifndef OBB_H
#define OBB_H

#include "model/geometry.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

// OBB (Oriented Bounding Box) Structure
typedef struct {
    Point center;
    Vector axis[2];

    float half_extent[2];
} OBB;

// Structure for detailed collision information
// typedef struct {
//     bool occurred;
//     Vector mtv; // Minimum Translation Vector to resolve collision (normal * penetration_depth)
//     // float penetration_depth;
//     // Point collision_points[2];
// } CollisionInfo;

void obb_update(OBB *obb, Point world_center, Vector forward_direction, float entity_half_width, float entity_half_height);

void obb_get_world_corners(const OBB *obb, Point corners[4]);

bool obb_check_collision_obb_vs_obb(const OBB *obb1, const OBB *obb2);

bool obb_check_collision_obb_vs_line_segment(const OBB *obb, Point p0, Point p1);

#endif //OBB_H
