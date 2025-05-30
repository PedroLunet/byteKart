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

void renderer_present_buffer() {
    if (swap_buffers() != 0) {
        printf("Renderer Error: Failed to swap buffers.\n");
    }
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

void renderer_draw_road(const Road *road, const Player *player_view) {
    if (!road || !player_view) return;

    if (road->prerendered_track_image && road->prerendered_track_image->map) {

        int screen_pivot_x = s_screen_width / 2;
        int screen_pivot_y = s_screen_height / 2;

        float local_pivot_in_track_image_x = player_view->world_position_car_center.y - road->world_origin_of_track_image.x; // 6000 -> 1000
        float local_pivot_in_track_image_y = player_view->world_position_car_center.x - road->world_origin_of_track_image.y; // 1000 -> 6000

        float cos_track_rotation = -player_view->forward_direction.x;
        float sin_track_rotation = -player_view->forward_direction.y;

        sprite_draw_rotated_around_local_pivot(
            (Sprite*)road->prerendered_track_image,
            screen_pivot_x, // 400
            screen_pivot_y, // 300
            (int)roundf(local_pivot_in_track_image_x), // 1000
            (int)roundf(local_pivot_in_track_image_y), // 6000
            cos_track_rotation,
            sin_track_rotation,
            false
        );

    }
}

void renderer_draw_player_car(const Player *player, bool skid_input, int player_skid_input_sign, float cos_skid, float sin_skid) {
    if (!player) return;
    int screen_center_x = s_screen_width / 2;
    int screen_center_y = s_screen_height / 2;

    if (player->sprite) {
        float cos_tilt = 1.0f;
        float sin_tilt = 0.0f;
        if (skid_input) {
            cos_tilt = cos_skid;
            sin_tilt = -sin_skid * player_skid_input_sign;
        }

        int sprite_pivot_x = player->sprite->width / 2;
    	int sprite_pivot_y = player->sprite->height / 2;

        sprite_draw_rotated_around_local_pivot(player->sprite, screen_center_x, screen_center_y, sprite_pivot_x, sprite_pivot_y, cos_tilt, sin_tilt, true);
    }
}

void renderer_draw_ai_car(const AICar *ai_car, const Player *player_view) {
    if (!ai_car || !player_view) return;

    if (ai_car->sprite) {
        Point_i screen_position;
        renderer_transform_world_to_screen(player_view, ai_car->world_position, &screen_position);

        float rel_x =  ai_car->forward_direction.x * player_view->forward_direction.x +
                       ai_car->forward_direction.y * player_view->forward_direction.y;
        float rel_y = -ai_car->forward_direction.x * player_view->forward_direction.y +
                        ai_car->forward_direction.y * player_view->forward_direction.x;

        int sprite_pivot_x = ai_car->sprite->width / 2;
        int sprite_pivot_y = ai_car->sprite->height / 2;

        sprite_draw_rotated_around_local_pivot(
            (Sprite*)ai_car->sprite,
            screen_position.x, screen_position.y,
            sprite_pivot_x, sprite_pivot_y,
            rel_x, -rel_y,
            true
        );
    }
}

