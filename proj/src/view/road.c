#include <lcom/lcf.h>
#include "road.h"

static Sprite *road_sprite1 = NULL;
static Sprite *road_sprite2 = NULL;
static int road_y1 = 0;
static int road_y2 = 0;
extern vbe_mode_info_t vbe_mode_info;

void draw_road_background() {
    if (!road_sprite1 || !road_sprite2) {
        road_sprite1 = sprite_create_xpm((xpm_map_t) road_xpm, 0, 0, 0, 0);
        road_sprite2 = sprite_create_xpm((xpm_map_t) road_xpm, 0, 0, 0, 0);
        if (!road_sprite1 || !road_sprite2) return;
        road_y1 = 0;
        road_y2 = -road_sprite1->height;
    }
    sprite_draw_xpm(road_sprite1, 0, road_y1, false);
    sprite_draw_xpm(road_sprite2, 0, road_y2, false);
}

void cleanup_road_background() {
    if (road_sprite1) {
        sprite_destroy(road_sprite1);
        road_sprite1 = NULL;
    }
    if (road_sprite2) {
        sprite_destroy(road_sprite2);
        road_sprite2 = NULL;
    }
}

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

    if (road->road_texture) {
        sprite_destroy(road->road_texture);
        road->road_texture = NULL;
    }
    if (road->finish_line_sprite) {
        sprite_destroy(road->finish_line_sprite);
        road->finish_line_sprite = NULL;
    }
}

int road_load(Road *road, const char *filename, int road_width_param, uint32_t default_bg_color, const char *const *road_texture_xpm, const char *const *finish_line_xpm) {
    if (!road || !filename) {
        return 1;
    }

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

    if (road_texture_xpm) {
        road->road_texture = sprite_create_xpm(road_texture_xpm, 0, 0, 0, 0);
        if (!road->road_texture) fprintf(stderr, "Warning: Failed to load road texture.\n");
    } else {
        road->road_texture = NULL;
    }

    if (finish_line_xpm) {
        road->finish_line_sprite = sprite_create_xpm(finish_line_xpm, 0, 0, 0, 0);
        if (!road->finish_line_sprite) fprintf(stderr, "Warning: Failed to load finish line sprite.\n");
    } else {
        road->finish_line_sprite = NULL;
    }

    // Position and orient finish line
    if (road->num_center_points >= 2) {
        road->finish_line_position = road->end_point;
        Vector last_segment_dir;
        last_segment_dir.x = road->center_points[road->num_center_points-1].x - road->center_points[road->num_center_points-2].x;
        last_segment_dir.y = road->center_points[road->num_center_points-1].y - road->center_points[road->num_center_points-2].y;
        vector_normalize(&last_segment_dir);
        road->finish_line_direction.x = -last_segment_dir.y; // Normal
        road->finish_line_direction.y = last_segment_dir.x;
        vector_init(&road->finish_line_direction, road->finish_line_direction.x, road->finish_line_direction.y);
        vector_normalize(&road->finish_line_direction);
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

    if (last_known_idx < 0) last_known_idx = 0;
    if (last_known_idx >= road->num_center_points - 1) last_known_idx = road->num_center_points - 2;

    int search_radius = 5;
    int best_found_idx_in_window = -1;
    float min_dist_sq_in_window = -1.0f;
    Point temp_closest_point;

    int start_search_idx = last_known_idx - search_radius;
    if (start_search_idx < 0) start_search_idx = 0;

    int end_search_idx = last_known_idx + search_radius;
    if (end_search_idx >= road->num_center_points - 1) end_search_idx = road->num_center_points - 2;

    // Local search window
    for (int i = start_search_idx; i <= end_search_idx; ++i) {
        Point seg_a = road->center_points[i];
        Point seg_b = road->center_points[i+1];
        Point current_segment_closest_p;
        float current_dist_sq = dist_sq_point_segment(*entity_world_pos, seg_a, seg_b, &current_segment_closest_p);

        if (best_found_idx_in_window == -1 || current_dist_sq < min_dist_sq_in_window) {
            min_dist_sq_in_window = current_dist_sq;
            best_found_idx_in_window = i;
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
        Point p2 = road->center_points[best_found_idx_in_window+1];
        out_tangent->x = p2.x - p1.x;
        out_tangent->y = p2.y - p1.y;
        vector_init(out_tangent, out_tangent->x, out_tangent->y);
        vector_normalize(out_tangent);
        return true;
    }

    if (out_tangent) { out_tangent->x = 1; out_tangent->y = 0; vector_init(out_tangent, 1,0); vector_normalize(out_tangent); }
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

