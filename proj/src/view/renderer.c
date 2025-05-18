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

void renderer_draw_road(const Road *road, const Player *player_view) {
    if (!road || !player_view || road->num_center_points < 2) return;
    uint32_t road_color = 0x303030;

    for (int i = 0; i < road->num_center_points; ++i) {
        int next_i = (i + 1) % road->num_center_points;
        Point_i p_left_start_screen, p_left_end_screen;
        Point_i p_right_start_screen, p_right_end_screen;

        if (road->left_edge_points && road->right_edge_points) {
            renderer_transform_world_to_screen(player_view, road->left_edge_points[i], &p_left_start_screen);
            renderer_transform_world_to_screen(player_view, road->left_edge_points[next_i], &p_left_end_screen);
            renderer_draw_line(p_left_start_screen.x, p_left_start_screen.y, p_left_end_screen.x, p_left_end_screen.y, road_color);

            renderer_transform_world_to_screen(player_view, road->right_edge_points[i], &p_right_start_screen);
            renderer_transform_world_to_screen(player_view, road->right_edge_points[next_i], &p_right_end_screen);
            renderer_draw_line(p_right_start_screen.x, p_right_start_screen.y, p_right_end_screen.x, p_right_end_screen.y, road_color);
        }
    }

    if (road->finish_line_sprite) {
        Point_i finish_screen_pos;
        renderer_transform_world_to_screen(player_view, road->finish_line_position, &finish_screen_pos);

        float cos_finish_angle = road->finish_line_direction.x;
        float sin_finish_angle = road->finish_line_direction.y;

        sprite_draw_rotated_xpm((Sprite*)road->finish_line_sprite, finish_screen_pos.x, finish_screen_pos.y, cos_finish_angle, sin_finish_angle, true);
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

