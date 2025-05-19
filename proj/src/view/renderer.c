#include <lcom/lcf.h>
#include "renderer.h"

extern vbe_mode_info_t vbe_mode_info;
static int s_screen_width = 0;
static int s_screen_height = 0;

int renderer_init() {
    s_screen_width = vbe_mode_info.XResolution;
    s_screen_height = vbe_mode_info.YResolution;

    if (s_screen_width == 0 || s_screen_height == 0) {
        return 1;
    }
    return 0;
}

void renderer_clear_buffer(uint32_t color) {
    if (vg_draw_rectangle(0, 0, s_screen_width, s_screen_height, color) != 0) {
        printf("Renderer Error: Failed to clear buffer using vg_draw_rectangle.\n");
    }
}

void renderer_transform_world_to_screen(const Player *player_view, Point world_pos, Point_i *screen_pos) {
    if (!player_view || !screen_pos) return;

    Point car_center_world = player_view->world_position_car_center;

    float player_dx = player_view->forward_direction.x;
    float player_dy = player_view->forward_direction.y;

    float translated_x = world_pos.x - car_center_world.x;
    float translated_y = world_pos.y - car_center_world.y;

    float view_space_x = translated_x * player_dy - translated_y * player_dx;
    float view_space_y = translated_x * player_dx + translated_y * player_dy;

    screen_pos->x = (int)roundf((player_view->view_width / 2.0f) + view_space_x);
    screen_pos->y = (int)roundf((player_view->view_height / 2.0f) - view_space_y);
}

void renderer_draw_pixel(int x, int y, uint32_t color) {
    vg_draw_pixel((uint16_t)x, (uint16_t)y, color);
}

void renderer_draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        renderer_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void renderer_draw_rectangle_filled(int x, int y, int width, int height, uint32_t color) {
    vg_draw_rectangle((uint16_t)x, (uint16_t)y, (uint16_t)width, (uint16_t)height, color);
}

void renderer_draw_road(const Road *road, const Player *player_view, float total_dist_scrolled_world) {
    if (!road || !player_view || !road->road_texture_tile || !road->road_texture_tile->map ||
        road->num_center_points < 2 || road->road_texture_tile->height == 0) {

        // Fallback to wireframe if no texture or not enough data
        if (road && player_view && road->num_center_points >=2 && road->left_edge_points && road->right_edge_points) {
            uint32_t road_edge_color = 0x404040;
            for (int i = 0; i < road->num_center_points; ++i) {
                int next_i = (i + 1) % road->num_center_points;
                Point_i p_left_start_screen, p_left_end_screen, p_right_start_screen, p_right_end_screen;
                renderer_transform_world_to_screen(player_view, road->left_edge_points[i], &p_left_start_screen);
                renderer_transform_world_to_screen(player_view, road->left_edge_points[next_i], &p_left_end_screen);
                renderer_draw_line(p_left_start_screen.x, p_left_start_screen.y, p_left_end_screen.x, p_left_end_screen.y, road_edge_color);
                renderer_transform_world_to_screen(player_view, road->right_edge_points[i], &p_right_start_screen);
                renderer_transform_world_to_screen(player_view, road->right_edge_points[next_i], &p_right_end_screen);
                renderer_draw_line(p_right_start_screen.x, p_right_start_screen.y, p_right_end_screen.x, p_right_end_screen.y, road_edge_color);
            }
        }
        return;
    }

    // These local variables were unused as the information is passed via player_view
    // or calculated fresh within the loop.
    // Point car_center_world = player_view->world_position_car_center; // UNUSED
    // float player_cos_angle = player_view->forward_direction.x;       // UNUSED
    // float player_sin_angle = player_view->forward_direction.y;       // UNUSED
    // float screen_center_x_float = player_view->view_width / 2.0f;    // UNUSED
    // float screen_center_y_float = player_view->view_height / 2.0f;   // UNUSED

    float world_units_per_texture_row = 1.0f; // How many world units one row of texture represents in length.
                                            // Adjust this based on your texture tile's intended scale.
                                            // If tile height is 70px and represents 70 world units, this is 1.0.
                                            // If tile height is 70px and represents 35 world units (texture appears larger), this is 0.5.

    for (int i = 0; i < road->num_center_points; ++i) {
        Point p_world_center = road->center_points[i];
        Point p_next_world = road->center_points[(i + 1) % road->num_center_points];

        Vector segment_dir_world;
        segment_dir_world.x = p_next_world.x - p_world_center.x;
        segment_dir_world.y = p_next_world.y - p_world_center.y;
        vector_init(&segment_dir_world, segment_dir_world.x, segment_dir_world.y);
        if (segment_dir_world.magnitude < 0.001f) {
            if (i > 0) {
                segment_dir_world.x = p_world_center.x - road->center_points[i-1].x;
                segment_dir_world.y = p_world_center.y - road->center_points[i-1].y;
            } else {
                segment_dir_world.x = 1.0f; segment_dir_world.y = 0.0f;
            }
            vector_init(&segment_dir_world, segment_dir_world.x, segment_dir_world.y);
            if (segment_dir_world.magnitude < 0.001f) {segment_dir_world.x=1.0f; segment_dir_world.y=0.0f; vector_init(&segment_dir_world,1,0);}
        }
        vector_normalize(&segment_dir_world);

        Point_i p_screen_center;
        renderer_transform_world_to_screen(player_view, p_world_center, &p_screen_center);

        int tile_max_dim = road->road_texture_tile->width > road->road_texture_tile->height ?
                           road->road_texture_tile->width : road->road_texture_tile->height;
        if (p_screen_center.x < -tile_max_dim || p_screen_center.x > s_screen_width + tile_max_dim ||
            p_screen_center.y < -tile_max_dim || p_screen_center.y > s_screen_height + tile_max_dim) {
            continue;
        }

        // V-coordinate calculation for texture scrolling
        // This needs a proper accumulated distance along the track.
        // For now, using a simplified approach based on 'i' and total_dist_scrolled_world
        // A more accurate V would be:
        // float v_world = total_dist_scrolled_world + (distance from track start to point 'i');
        // int texture_y_row = ((int)(v_world / world_units_per_texture_row)) % road->road_texture_tile->height;

        // Simpler V for now, will cause texture "swimming" on curves if not careful with total_dist_scrolled_world
        // and how 'i' relates to actual distance.
        float temp_dist_for_v = total_dist_scrolled_world + (float)i * 1.0f; // Assuming 1 unit per centerline point
        int texture_y_row = ( (int)(temp_dist_for_v / world_units_per_texture_row) ) % road->road_texture_tile->height;
        if (texture_y_row < 0) texture_y_row += road->road_texture_tile->height;


        // For the scanline strip drawing method, we would calculate screen_x_start, screen_x_end
        // based on transformed left/right edges for this p_screen_center.y
        // renderer_draw_textured_horizontal_strip(p_screen_center.y, screen_x_left, screen_x_right,
        //                                         road->road_texture_tile, texture_y_row, true);

        // Sticking to stamping the tile for now, as scanline texturing per point is complex here.
        float cos_segment_angle = segment_dir_world.x;
        float sin_segment_angle = segment_dir_world.y;

        // Adjust for tile's default orientation (e.g., if tile is "tall" and world angle 0 is +X)
        float cos_render = sin_segment_angle;  // cos(seg_angle - PI/2)
        float sin_render = -cos_segment_angle; // sin(seg_angle - PI/2)

        sprite_draw_rotated_xpm((Sprite*)road->road_texture_tile,
                                p_screen_center.x,
                                p_screen_center.y,
                                cos_render,
                                sin_render,
                                true);
    }

    // Draw finish line sprite (if it's a separate entity and not part of pre-rendered track)
    if (road->finish_line_sprite && road->finish_line_sprite->map) {
        Point_i finish_screen_pos;
        renderer_transform_world_to_screen(player_view, road->finish_line_position, &finish_screen_pos);
        float cos_finish_angle = road->finish_line_direction.x;
        float sin_finish_angle = road->finish_line_direction.y;
        sprite_draw_rotated_xpm((Sprite*)road->finish_line_sprite,
                                finish_screen_pos.x, finish_screen_pos.y,
                                cos_finish_angle, sin_finish_angle, true);
    }
}

void renderer_draw_player_car(const Player *player) {
    if (!player) return;
    int screen_center_x = s_screen_width / 2;
    int screen_center_y = s_screen_height / 2;

    if (player->sprite) {
        float cos_car_angle = player->forward_direction.x;
        float sin_car_angle = player->forward_direction.y;

        float cos_render = cos_car_angle;
        float sin_render = -sin_car_angle;

        sprite_draw_rotated_xpm((Sprite*)player->sprite, screen_center_x, screen_center_y, cos_render, sin_render, true);

    }
}

void renderer_draw_ai_car(const AICar *ai_car, const Player *player_view) {
    if (!ai_car || !player_view) return;

    Point_i screen_pos;
    renderer_transform_world_to_screen(player_view, ai_car->world_position, &screen_pos);

    if (ai_car->sprite) {
        float rough_sprite_radius = (ai_car->sprite->width + ai_car->sprite->height) / 4.0f;
        if (screen_pos.x + rough_sprite_radius < 0 || screen_pos.x - rough_sprite_radius > s_screen_width ||
            screen_pos.y + rough_sprite_radius < 0 || screen_pos.y - rough_sprite_radius > s_screen_height) {
            return;
        }

        float cos_ai_angle = ai_car->forward_direction.x;
        float sin_ai_angle = ai_car->forward_direction.y;

        float cos_render = sin_ai_angle;
        float sin_render = -cos_ai_angle;
        sprite_draw_rotated_xpm((Sprite*)ai_car->sprite, screen_pos.x, screen_pos.y, cos_render, sin_render, true);
    }
}

