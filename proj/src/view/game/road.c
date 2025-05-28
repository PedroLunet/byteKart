#include <lcom/lcf.h>
#include "road.h"

extern vbe_mode_info_t vbe_mode_info;

typedef struct {
    int r_x, r_y;
    int c_x, c_y;
    int l_x, l_y;
} IntTrackPointFile;

typedef struct {
    int segment_start_index;
    int num_boxes;
} IntPowerupLineFile;

typedef struct {
    int segment_start_index;
    float offset_from_center;
    int type;
} IntFloatIntObstacleFile;

void road_destroy(Road *road) {
    if (!road) return;

    free(road->center_points);
    road->center_points = NULL;

    free(road->left_edge_points);
    road->left_edge_points = NULL;

    free(road->right_edge_points);
    road->right_edge_points = NULL;

    if (road->prerendered_track_image) {
        sprite_destroy(road->prerendered_track_image);
        road->prerendered_track_image = NULL;
    }

    if (road->finish_line_sprite) {
        sprite_destroy(road->finish_line_sprite);
        road->finish_line_sprite = NULL;
    }

    if (road->raw_powerup_data) {
        free(road->raw_powerup_data);
        road->raw_powerup_data = NULL;
    }

    if (road->raw_obstacle_data) {
        free(road->raw_obstacle_data);
        road->raw_obstacle_data = NULL;
    }
}

int road_load(Road *road, const char *filename, int road_width_param, uint32_t default_bg_color, const char *prerendered_track_bin_file, xpm_map_t var_finish_xpm, LoadingUI *loading_ui) {
    if (!road || !filename) {
        return 1;
    }

    road_destroy(road);
    memset(road, 0, sizeof(Road));

    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening track file");
        return 2;
    }

    // Read Counts
    int num_track_points_from_file = 0;
    int num_powerups_from_file = 0;
    int num_obstacles_from_file = 0;

    if (fread(&num_track_points_from_file, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "road_load: Error reading num_track_points from '%s'.\n", filename);
        fclose(file); return 3;
    }
    if (fread(&num_powerups_from_file, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "road_load: Error reading num_powerups_from_file from '%s'.\n", filename);
        fclose(file); return 3;
    }
    if (fread(&num_obstacles_from_file, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "road_load: Error reading num_obstacles_from_file from '%s'.\n", filename);
        fclose(file); return 3;
    }
    printf("Road DAT Header: TrackPoints=%d, PowerUpLines=%d, Obstacles=%d\n", num_track_points_from_file, num_powerups_from_file, num_obstacles_from_file);
    road->num_center_points = num_track_points_from_file;

    // Load Track Points
    road->center_points = malloc(road->num_center_points * sizeof(Point));
    road->left_edge_points = malloc(road->num_center_points * sizeof(Point));
    road->right_edge_points = malloc(road->num_center_points * sizeof(Point));
	if (!road->center_points || !road->left_edge_points || !road->right_edge_points) {
        printf("road_load: Malloc failed for track point arrays");
        road_destroy(road); fclose(file); return 4;
    }

    IntTrackPointFile temp_track_point;
    for (int i = 0; i < road->num_center_points; ++i) {
        if (fread(&temp_track_point, sizeof(IntTrackPointFile), 1, file) != 1) {
            printf("road_load: Error reading track point set %d from '%s'.\n", i, filename);
            road_destroy(road); fclose(file); return 6;
        }
        road->right_edge_points[i].x = (float)temp_track_point.r_x;
        road->right_edge_points[i].y = (float)temp_track_point.r_y;
        road->center_points[i].x = (float)temp_track_point.c_x;
        road->center_points[i].y = (float)temp_track_point.c_y;
        road->left_edge_points[i].x = (float)temp_track_point.l_x;
        road->left_edge_points[i].y = (float)temp_track_point.l_y;
    }
    printf("Road: Left, Center, Right track points loaded successfully.\n");

    // Load Power-up Lines
    road->num_powerup_lines_from_file = num_powerups_from_file;
    if (road->num_powerup_lines_from_file > 0) {
        road->raw_powerup_data = malloc(road->num_powerup_lines_from_file * sizeof(RawPowerUpLineData));
        if (!road->raw_powerup_data) {
            printf("road_load: Malloc failed for raw_powerup_data");
            road_destroy(road); fclose(file); return 5;
        }
        IntPowerupLineFile temp_pu_line;
        for (int i = 0; i < road->num_powerup_lines_from_file; ++i) {
            if (fread(&temp_pu_line, sizeof(IntPowerupLineFile), 1, file) != 1) {
                printf("road_load: Error reading powerup line %d from '%s'.\n", i, filename);
                road_destroy(road); fclose(file); return 6;
            }
            road->raw_powerup_data[i].centerline_segment_index = temp_pu_line.segment_start_index;
            road->raw_powerup_data[i].num_boxes_in_line = temp_pu_line.num_boxes;
        }
        printf("Road: Raw power-up data loaded (%d lines).\n", road->num_powerup_lines_from_file);
    } else {
        road->raw_powerup_data = NULL;
    }

    // Load Obstacle Data
    road->num_obstacles_from_file = num_obstacles_from_file;
    if (road->num_obstacles_from_file > 0) {
        road->raw_obstacle_data = malloc(road->num_obstacles_from_file * sizeof(RawObstacleData));
        if (!road->raw_obstacle_data) {
            printf("road_load: Malloc failed for raw_obstacle_data");
            road_destroy(road); fclose(file); return 5;
        }
        IntFloatIntObstacleFile temp_obs;
        for (int i = 0; i < road->num_obstacles_from_file; ++i) {
            if (fread(&temp_obs, sizeof(IntFloatIntObstacleFile), 1, file) != 1) {
                printf("road_load: Error reading obstacle %d from '%s'.\n", i, filename);
                road_destroy(road); fclose(file); return 6;
            }
            road->raw_obstacle_data[i].centerline_segment_index = temp_obs.segment_start_index;
            road->raw_obstacle_data[i].offset_from_center = temp_obs.offset_from_center;
            road->raw_obstacle_data[i].type = temp_obs.type;
        }
        printf("Road: Raw obstacle data loaded (%d obstacles).\n", road->num_obstacles_from_file);
    } else {
        road->raw_obstacle_data = NULL;
    }

    fclose(file);
    printf("Road: Finished loading main data from '%s'.\n", filename);

    // Load pre-rendered track surface
    printf("Road: Attempting to load pre-rendered track surface from: %s\n", prerendered_track_bin_file);
    FILE *surface_file = fopen(prerendered_track_bin_file, "rb");
    if (!surface_file) {
        printf("road_load: Failed to open track surface file '%s'.\n", prerendered_track_bin_file);
        road->prerendered_track_image = NULL;
    } else {
        uint32_t width = 0, height = 0;
        const int BYTES_PER_PIXEL_EXPECTED = 4;
        if (fread(&width, sizeof(uint32_t), 1, surface_file) != 1 ||
            fread(&height, sizeof(uint32_t), 1, surface_file) != 1) {
            printf("road_load: Error reading header from track surface file '%s'.\n", prerendered_track_bin_file);
            road->prerendered_track_image = NULL;
        } else {
            printf("Road: Track surface header: %dx%d\n", width, height);
            if (width <= 0 || height <= 0) {
                printf("Road: Track surface header: Width=%u, Height=%u. Assuming %d bytes/pixel (RGBA).\n", width, height, BYTES_PER_PIXEL_EXPECTED);
                road->prerendered_track_image = NULL;
            } else {
                road->prerendered_track_image = (Sprite*)malloc(sizeof(Sprite));
                if (!road->prerendered_track_image) {
                    printf("road_load: Failed to allocate memory for track image sprite.\n");
                } else {
                    memset(road->prerendered_track_image, 0, sizeof(Sprite));
                    road->prerendered_track_image->width = width;
                    road->prerendered_track_image->height = height;

                    size_t map_data_size = (size_t)width * height * BYTES_PER_PIXEL_EXPECTED;
                    road->prerendered_track_image->map = (uint32_t*)malloc(map_data_size);

                    if (!road->prerendered_track_image->map) {
                        printf("road_load: Failed to allocate memory for track image pixel data.\n");
                        free(road->prerendered_track_image);
                        road->prerendered_track_image = NULL;
                    } else {
						size_t bytes_read = 0;
                        int count = 1;
						while (bytes_read < map_data_size) {
    						size_t to_read = map_data_size / 12;
    						if (bytes_read + to_read > map_data_size)
        						to_read = map_data_size - bytes_read;

    							size_t just_read = fread(
        						((uint8_t*)road->prerendered_track_image->map) + bytes_read,
       							1, to_read, surface_file
    						);
   							if (just_read != to_read) {
        						break;
    						}
    						bytes_read += just_read;

    						draw_ui_component(loading_ui->components[count]);
                            count++;
                            if (count >= 3) {
                                count = 0;
                            }
                            swap_buffer_loading_ui();
						}
                    }
                }
            }
        }
        fclose(surface_file);
    }
    road->prerendered_track_image->x = 0;
    road->prerendered_track_image->y = 0;
    road->world_origin_of_track_image.x = 0.0f;
    road->world_origin_of_track_image.y = 0.0f;

    // Set road properties
    road->road_width = road_width_param;
    road->background_color = default_bg_color;

    road->start_point = road->center_points[0];
    road->end_point = road->center_points[road->num_center_points - 1];

    if (var_finish_xpm) {
        road->finish_line_sprite = sprite_create_xpm(var_finish_xpm, 0,0,0,0);
        if (!road->finish_line_sprite) {
            fprintf(stderr, "Warning: Failed to load finish line sprite.\n");
        } else {
            printf("Road: Successfully loaded finish line sprite.\n");
            if (road->num_center_points >= 2) {
                road->finish_line_position = road->end_point;
                Vector last_segment_dir;
                last_segment_dir.x = road->center_points[road->num_center_points-1].x - road->center_points[road->num_center_points-2].x;
                last_segment_dir.y = road->center_points[road->num_center_points-1].y - road->center_points[road->num_center_points-2].y;
                vector_normalize(&last_segment_dir);
                road->finish_line_direction.x = -last_segment_dir.y;
                road->finish_line_direction.y = last_segment_dir.x;
                vector_init(&road->finish_line_direction, road->finish_line_direction.x, road->finish_line_direction.y);
            }
        }
    } else {
        road->finish_line_sprite = NULL;
    }

    return 0;
}

static float dist_sq_point_segment(Point p, Point seg_a, Point seg_b, Point *closest_point_on_seg) {
    Vector ab;
    ab.x = seg_b.x - seg_a.x;
    ab.y = seg_b.y - seg_a.y;
    vector_init(&ab, ab.x, ab.y);

    Vector ap;
    ap.x = p.x - seg_a.x;
    ap.y = p.y - seg_a.y;
    vector_init(&ap, ap.x, ap.y);

    float ab_len_sq = ab.magnitude * ab.magnitude;
    if (fabs(ab_len_sq) < 0.00001f) {
        if (closest_point_on_seg) *closest_point_on_seg = seg_a;
        return ap.magnitude * ap.magnitude;
    }

    float t = vector_dot_product(&ap, &ab) / ab_len_sq;

    if (t < 0.0f) {
        if (closest_point_on_seg) *closest_point_on_seg = seg_a;
        return ap.magnitude * ap.magnitude;
    } else if (t > 1.0f) {
        Vector bp;
        bp.x = p.x - seg_b.x;
        bp.y = p.y - seg_b.y;
        vector_init(&bp, bp.x, bp.y);
        if (closest_point_on_seg) *closest_point_on_seg = seg_b;
        return bp.magnitude * bp.magnitude;
    } else {
        Point proj_p;
        proj_p.x = seg_a.x + t * ab.x;
        proj_p.y = seg_a.y + t * ab.y;
        if (closest_point_on_seg) *closest_point_on_seg = proj_p;

        Vector d_vec;
        d_vec.x = p.x - proj_p.x;
        d_vec.y = p.y - proj_p.y;
        vector_init(&d_vec, d_vec.x, d_vec.y);
        return d_vec.magnitude * d_vec.magnitude;
    }
}

bool road_update_entity_on_track(Road *road, Point *entity_world_pos, int *p_entity_current_segment_idx, Vector *out_tangent, Point *out_closest_point_on_centerline) {
    if (!road || !entity_world_pos || !p_entity_current_segment_idx || !out_tangent || road->num_center_points < 2) {
        if (out_tangent) { out_tangent->x = 1; out_tangent->y = 0; vector_init(out_tangent, 1,0); vector_normalize(out_tangent); }
        return false;
    }

    int last_known_idx = *p_entity_current_segment_idx;
    int N_points = road->num_center_points;

    if (last_known_idx < 0 || last_known_idx >= N_points) {
        last_known_idx = (last_known_idx % N_points + N_points) % N_points;
    }

    int search_radius = 100;
    int best_found_idx_in_window = -1;
    float min_dist_sq_in_window = -1.0f;
    Point temp_closest_point;

    for (int i = 0; i < (2 * search_radius + 1); ++i) {
        int offset = i - search_radius;
        int segment_to_check_idx = (last_known_idx + offset % N_points + N_points) % N_points;

        Point seg_a = road->center_points[segment_to_check_idx];
        Point seg_b = road->center_points[(segment_to_check_idx + 1) % N_points];

        Point current_segment_closest_p;
        float current_dist_sq = dist_sq_point_segment(*entity_world_pos, seg_a, seg_b, &current_segment_closest_p);

        if (best_found_idx_in_window == -1 || current_dist_sq < min_dist_sq_in_window) {
            min_dist_sq_in_window = current_dist_sq;
            best_found_idx_in_window = segment_to_check_idx;
            if (out_closest_point_on_centerline) {
                temp_closest_point = current_segment_closest_p;
            }
        }
    }

    if (best_found_idx_in_window != -1) {
        *p_entity_current_segment_idx = best_found_idx_in_window;
        if (out_closest_point_on_centerline) {
             *out_closest_point_on_centerline = temp_closest_point;
        }

        Point p1 = road->center_points[best_found_idx_in_window];
        Point p2 = road->center_points[(best_found_idx_in_window + 1) % N_points];
        out_tangent->x = p2.x - p1.x;
        out_tangent->y = p2.y - p1.y;
        vector_init(out_tangent, out_tangent->x, out_tangent->y);
        vector_normalize(out_tangent);
        return true;
    }

    if (out_tangent) {
      out_tangent->x = 1;
      out_tangent->y = 0;
      vector_init(out_tangent, 1,0);
      vector_normalize(out_tangent);
    }
    return false;
}

bool road_get_tangent_at_world_pos_fullscan(Road *road, Point *world_pos, Vector *out_tangent, int *out_segment_idx, Point *out_closest_point_on_centerline) {
    if (!road || !world_pos || !out_tangent || road->num_center_points < 2) {
        if (out_tangent) { out_tangent->x = 1; out_tangent->y = 0; vector_init(out_tangent, 1,0); vector_normalize(out_tangent); }
        if (out_segment_idx) *out_segment_idx = -1;
        return false;
    }

    float min_dist_sq = -1.0f;
    int closest_segment_idx_found = -1;
    Point temp_closest_point;

    for (int i = 0; i < road->num_center_points - 1; ++i) {
        Point seg_a = road->center_points[i];
        Point seg_b = road->center_points[i+1];
        Point current_segment_closest_p;
        float dist_sq = dist_sq_point_segment(*world_pos, seg_a, seg_b, &current_segment_closest_p);

        if (closest_segment_idx_found == -1 || dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            closest_segment_idx_found = i;
            if (out_closest_point_on_centerline) {
                temp_closest_point = current_segment_closest_p;
            }
        }
    }

    if (closest_segment_idx_found != -1) {
        if (out_segment_idx) *out_segment_idx = closest_segment_idx_found;
        if (out_closest_point_on_centerline) *out_closest_point_on_centerline = temp_closest_point;

        Point p1 = road->center_points[closest_segment_idx_found];
        Point p2 = road->center_points[closest_segment_idx_found+1];
        out_tangent->x = p2.x - p1.x;
        out_tangent->y = p2.y - p1.y;
        vector_init(out_tangent, out_tangent->x, out_tangent->y);
        vector_normalize(out_tangent);
        return true;
    }

    if (out_tangent) { out_tangent->x = 1; out_tangent->y = 0; vector_init(out_tangent, 1,0); vector_normalize(out_tangent); }
    if (out_segment_idx) *out_segment_idx = -1;
    return false;
}

bool road_get_centerline_point(Road *road, int point_index, Point *out_point) {
    if (!road || !out_point || point_index < 0 || point_index >= road->num_center_points) {
        return false;
    }
    *out_point = road->center_points[point_index];
    return true;
}

Point road_get_start_point(Road *road, int car_index) {

    if (!road || car_index < 0) {
        return (Point){0.0f, 0.0f};
    }

    Point start_point = {0.0f, 0.0f};
    switch (car_index) {
        case 0:
            start_point.y = 1216.0f;
            start_point.x = 6670.0f;
            return start_point;
        case 1:
            start_point.y = 1043.0f;
            start_point.x = 6738.0f;
            return start_point;
        case 2:
            start_point.y = 870.0f;
            start_point.x = 6706.0f;
            return start_point;
        case 3:
            start_point.y = 697.0f;
            start_point.x = 6674.0f;
            return start_point;
        default:
            return start_point;
    }
}

