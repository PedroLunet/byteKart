#include <lcom/lcf.h>
#include "road.h"

extern vbe_mode_info_t vbe_mode_info;

int road_calculate_edge_points(Road *road) {
    if (!road || !road->center_points || road->num_center_points < 2) {
        return 1;
    }

    road->left_edge_points = malloc(road->num_center_points * sizeof(Point));
    road->right_edge_points = malloc(road->num_center_points * sizeof(Point));

    if (!road->left_edge_points || !road->right_edge_points) {
        free(road->left_edge_points);
        free(road->right_edge_points);
        road->left_edge_points = NULL;
        road->right_edge_points = NULL;
        return 1;
    }

    float half_width = road->road_width / 2.0f;

    for (int i = 0; i < road->num_center_points; ++i) {
        Vector segment_dir;
        if (i < road->num_center_points - 1) {
            segment_dir.x = road->center_points[i+1].x - road->center_points[i].x;
            segment_dir.y = road->center_points[i+1].y - road->center_points[i].y;
        } else if (road->num_center_points > 1) {
            segment_dir.x = road->center_points[i].x - road->center_points[i-1].x;
            segment_dir.y = road->center_points[i].y - road->center_points[i-1].y;
        } else {
            segment_dir.x = 1.0f; segment_dir.y = 0.0f;
        }

        vector_normalize(&segment_dir);

        Vector normal_left;
        normal_left.x = -segment_dir.y;
        normal_left.y = segment_dir.x;

        road->left_edge_points[i].x = road->center_points[i].x + normal_left.x * half_width;
        road->left_edge_points[i].y = road->center_points[i].y + normal_left.y * half_width;

        road->right_edge_points[i].x = road->center_points[i].x - normal_left.x * half_width;
        road->right_edge_points[i].y = road->center_points[i].y - normal_left.y * half_width;
    }
    return 0;
}

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
}

int road_load(Road *road, const char *filename, int road_width_param, uint32_t default_bg_color, const char *prerendered_track_bin_file, LoadingUI *loading_ui) {
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

    // Read number of points
    if (fread(&road->num_center_points, sizeof(int), 1, file) != 1) {
        printf("Error reading number of points from track file.\n");
        fclose(file);
        return 3;
    }
    if (road->num_center_points < 2) {
        printf("Track file must contain at least 2 points.\n");
        fclose(file);
        return 4;
    }

    // Allocate memory for centerline points
    road->center_points = malloc(road->num_center_points * sizeof(Point));
    if (!road->center_points) {
        printf("Memory allocation failed for center_points.\n");
        fclose(file);
        return 5;
    }

    // Read all centerline points
    typedef struct { int x; int y; } IntPointFile;
    IntPointFile temp_point_from_file;

    for (int i = 0; i < road->num_center_points; ++i) {
        if (fread(&temp_point_from_file, sizeof(IntPointFile), 1, file) != 1) {
            printf("road_load: Error reading point %d data from track file.\n", i);
            free(road->center_points);
            road->center_points = NULL;
            road->num_center_points = 0;
            fclose(file);
            return 6;
        }
        road->center_points[i].x = (float)temp_point_from_file.x;
        road->center_points[i].y = (float)temp_point_from_file.y;
    }
    fclose(file);

    printf("Road: Attempting to load pre-rendered track surface from: %s\n", prerendered_track_bin_file);
    FILE *surface_file = fopen(prerendered_track_bin_file, "rb");
    if (!surface_file) {
        printf("road_load: Failed to open track surface file '%s'.\n", prerendered_track_bin_file);
        road->prerendered_track_image = NULL;
    } else {
        int width = 0, height = 0, file_bpp = 0;
        if (fread(&width, sizeof(int), 1, surface_file) != 1 ||
            fread(&height, sizeof(int), 1, surface_file) != 1 ||
            fread(&file_bpp, sizeof(int), 1, surface_file) != 1) {
            printf("road_load: Error reading header from track surface file '%s'.\n", prerendered_track_bin_file);
            road->prerendered_track_image = NULL;
        } else {
            printf("Road: Track surface header: %dx%d, %d bytes/pixel\n", width, height, file_bpp);
            if (width <= 0 || height <= 0 || (file_bpp != 3 && file_bpp != 4)) {
                printf("road_load: Invalid dimensions or bpp in track surface file.\n");
                road->prerendered_track_image = NULL;
            } else {
                road->prerendered_track_image = (Sprite*)malloc(sizeof(Sprite));
                if (!road->prerendered_track_image) {
                    printf("road_load: Failed to allocate memory for track image sprite.\n");
                } else {
                    memset(road->prerendered_track_image, 0, sizeof(Sprite));
                    road->prerendered_track_image->width = width;
                    road->prerendered_track_image->height = height;
                    if (file_bpp != 4) {
                         printf("Warning: Track surface binary bpp (%d) doesn't match expected 4 bytes for uint32_t map. Pixel data might be misinterpreted.\n", file_bpp);
                    }

                    size_t map_data_size = (size_t)width * height * file_bpp;
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

    // Calculate edge points
    if (road_calculate_edge_points(road) != 0) {
        printf("Failed to calculate road edge points.\n");
        free(road->center_points);
        road->center_points = NULL;
        return 7;
    }

    road->start_point = road->center_points[0];
    road->end_point = road->center_points[road->num_center_points - 1];

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

    int search_radius = 5;
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

