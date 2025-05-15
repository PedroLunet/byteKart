#ifndef ROAD_H
#define ROAD_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sprite.h"
#include "geometry.h"
#include "function.h"
#include "xpm/xpm_files.h"

typedef struct Road {
    Point *center_points;
    int num_center_points;

    Point *left_edge_points;
    Point *right_edge_points;

    int road_width;
    uint32_t background_color;

    Sprite *road_texture;
    Sprite *finish_line_sprite;
    Point finish_line_position;
    Vector finish_line_direction;

    Point start_point;
    Point end_point;
} Road;

void draw_road_background();
void cleanup_road_background();

int road_calculate_edge_points(Road *road);
int road_load(Road *road, const char *filename, int road_width_param, uint32_t default_bg_color, const char *const *road_texture_xpm, const char *const *finish_line_xpm);
void road_destroy(Road *road);
void road_draw(Road *road, const Player *player_camera);
bool road_update_entity_on_track(Road *road, Point *entity_world_pos, int *p_entity_current_segment_idx, Vector *out_tangent, Point *out_closest_point_on_centerline);
bool road_get_tangent_at_world_pos_fullscan(Road *road, Point *world_pos, Vector *out_tangent, int *out_segment_idx, Point *out_closest_point_on_centerline);
bool road_get_centerline_point(Road *road, int point_index, Point *out_point);

Road *create_road(int road_number);

#endif
