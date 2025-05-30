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

typedef struct {
    bool occurred;
    float penetration_depth;
    Vector collision_normal;
 } CollisionInfo;

void obb_update(OBB *obb, Point world_center, Vector forward_direction, float entity_half_width, float entity_half_height);

void obb_get_world_corners(const OBB *obb, Point corners[4]);

void obb_check_collision_obb_vs_obb(const OBB *obb1, const OBB *obb2, CollisionInfo *info);

void obb_check_collision_obb_vs_line_segment(const OBB *obb, Point p0, Point p1, CollisionInfo *info);

#endif //OBB_H
