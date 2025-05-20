#ifndef ROAD_H
#define ROAD_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sprites/sprite.h"
#include "model/geometry.h"
#include "xpm/xpm_files.h"
#include "view/utils/ui.h"
#include "../loadingUI.h"


struct Player_s;

typedef struct Road_s {
    Point *center_points;
    int num_center_points;

    Point *left_edge_points;
    Point *right_edge_points;

    int road_width;
    uint32_t background_color;

    Sprite *prerendered_track_image;
    Point world_origin_of_track_image;

    Point start_point;
    Point end_point;
} Road;

void draw_road_background(Sprite *road_sprite1, Sprite *road_sprite2, int road_y1, int road_y2);
void cleanup_road_background();

int road_calculate_edge_points(Road *road);
int road_load(Road *road, const char *filename, int road_width_param, uint32_t default_bg_color, const char *prerendered_track_bin_file, LoadingUI *loading_ui);
void road_destroy(Road *road);
void road_draw(Road *road);
bool road_update_entity_on_track(Road *road, Point *entity_world_pos, int *p_entity_current_segment_idx, Vector *out_tangent, Point *out_closest_point_on_centerline);
bool road_get_tangent_at_world_pos_fullscan(Road *road, Point *world_pos, Vector *out_tangent, int *out_segment_idx, Point *out_closest_point_on_centerline);
bool road_get_centerline_point(Road *road, int point_index, Point *out_point);

Road *create_road(int road_number);

#endif
