/**
 * @file road.h
 * @brief Defines the road system, including rendering, geometry, and track interactions.
 *
 */
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
#include "view/utils/loadingUI.h"


struct Player_s; /**< Declaration of the Player struct. */

typedef struct {
    int centerline_segment_index;
    int num_boxes_in_line;
} RawPowerUpLineData;

typedef struct {
    int centerline_segment_index;
    float offset_from_center;
    int type;
} RawObstacleData;

/**
 * @struct Road
 * @brief Represents the game’s road or track, including geometry and visual data.
 */
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

    Sprite *finish_line_sprite;
    Point finish_line_position;
    Vector finish_line_direction;

    RawPowerUpLineData* raw_powerup_data;
    int num_powerup_lines_from_file;

    RawObstacleData* raw_obstacle_data;
    int num_obstacles_from_file;
} Road;

/**
 * @brief Calculates the left and right edge points from the road centerline.
 *
 * @param road Pointer to the Road structure.
 * @return 0 on success, non-zero on failure.
 */

/**
 * @brief Loads road data from a file and initializes the road.
 *
 * @param road Pointer to the Road to initialize.
 * @param filename Path to the file containing the road centerline points.
 * @param road_width_param Width of the road.
 * @param default_bg_color Default background color if image is unavailable.
 * @param prerendered_track_bin_file Path to the pre-rendered track binary file.
 * @param loading_ui Pointer to the loading UI used for visual feedback.
 * @return 0 on success, non-zero on failure.
 */
int road_load(Road *road, const char *filename, int road_width_param, uint32_t default_bg_color, const char *prerendered_track_bin_file, float track_offset_x, float track_offset_y, xpm_map_t var_finish_xpm, LoadingUI *loading_ui);

/**
 * @brief Releases memory associated with a Road structure.
 *
 * @param road Pointer to the Road to destroy.
 */
void road_destroy(Road *road);

/**
 * @brief Updates an entity's position relative to the road centerline.
 *
 * @param road Pointer to the road.
 * @param entity_world_pos Pointer to the entity’s world position.
 * @param p_entity_current_segment_idx Index of the segment currently under the entity (updated).
 * @param out_tangent Output tangent vector along the road at the entity’s location.
 * @param out_closest_point_on_centerline Output closest point on the centerline.
 * @return true if successful, false otherwise.
 */
bool road_update_entity_on_track(Road *road, Point *entity_world_pos, int *p_entity_current_segment_idx, Vector *out_tangent, Point *out_closest_point_on_centerline);

/**
 * @brief Gets the road tangent and closest point at a world position by scanning all segments.
 *
 * @param road Pointer to the road.
 * @param world_pos Position in world space to evaluate.
 * @param out_tangent Output tangent vector.
 * @param out_segment_idx Output segment index.
 * @param out_closest_point_on_centerline Output closest point on the centerline.
 * @return true if successful, false if the position is out of bounds.
 */
bool road_get_tangent_at_world_pos_fullscan(Road *road, Point *world_pos, Vector *out_tangent, int *out_segment_idx, Point *out_closest_point_on_centerline);

/**
 * @brief Retrieves a point on the road centerline.
 *
 * @param road Pointer to the road.
 * @param point_index Index of the point to retrieve.
 * @param out_point Output point.
 * @return true if successful, false if the index is out of bounds.
 */
bool road_get_centerline_point(Road *road, int point_index, Point *out_point);

/**
 * @brief Gets the starting position on the road for a given car index.
 *
 * @param road Pointer to the road.
 * @param car_index Index of the car (used to offset start positions if needed).
 * @return Starting point for the specified car.
 */
Point road_get_start_point(Road *road, int car_index);

#endif
