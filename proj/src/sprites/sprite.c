#include <lcom/lcf.h>
#include "sprite.h"

Sprite *sprite_create_xpm(xpm_map_t pic, int x, int y, int xspeed, int yspeed) {
    Sprite *this = (Sprite *)malloc(sizeof(Sprite));
    if (this == NULL) {
        return NULL;
    }

    xpm_image_t img;
    this->map = (uint32_t *)xpm_load(pic, XPM_8_8_8_8, &img);
    if (this->map == NULL) {
        free(this);
        return NULL;
    }

    this->width = img.width;
    this->height = img.height;
    this->x = x;
    this->y = y;
    this->xspeed = xspeed;
    this->yspeed = yspeed;
    this->bytes_per_pixel = 4;
    this->data = (uint8_t *)this->map;

    return this;
}

int sprite_draw_xpm(Sprite *this, int x, int y, bool has_transparent) {
    uint16_t height = this->height;
    uint16_t width = this->width;
    uint32_t current_color;
    uint32_t transparent = this->map[width * height - 1];

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            current_color = this->map[w + h * width];
            if (current_color == transparent && has_transparent)
                continue;
            if (vg_draw_pixel(x + w, y + h, current_color) != 0)
                return 1;
        }
    }

    return 0;
}

int sprite_draw_partial_xpm(Sprite *this, int x, int y, int width, int height, bool has_transparent) {
    uint16_t sprite_width = this->width;
    uint16_t sprite_height = this->height;
    uint32_t current_color;
    uint32_t transparent = this->map[sprite_width * sprite_height - 1];

    int start_x = (x < this->x) ? 0 : x - this->x;
    int start_y = (y < this->y) ? 0 : y - this->y;
    int end_x = (x + width > this->x + sprite_width) ? sprite_width : start_x + width;
    int end_y = (y + height > this->y + sprite_height) ? sprite_height : start_y + height;

    for (int h = start_y; h < end_y; h++) {
        for (int w = start_x; w < end_x; w++) {
            current_color = this->map[w + h * sprite_width];
            if (current_color == transparent && has_transparent)
                continue;
            if (vg_draw_pixel(this->x + w, this->y + h, current_color) != 0)
                return 1;
        }
    }
    return 0;
}

int sprite_draw_rotated_around_local_pivot(
    Sprite *sprite,
    int screen_pivot_x, int screen_pivot_y,
    int local_pivot_x, int local_pivot_y,
    float cos_a, float sin_a,
    bool has_transparent
) {
    if (!sprite || !sprite->map || sprite->width == 0 || sprite->height == 0) return 1;

    /*
    (void)cos_a; // Mark as unused for this simplified version
    (void)sin_a; // Mark as unused for this simplified version

    // Calculate the screen coordinates of the sprite's top-left corner
    // such that its (local_pivot_x, local_pivot_y) aligns with (screen_pivot_x, screen_pivot_y)
    int draw_top_left_x = local_pivot_x - screen_pivot_x;
    int draw_top_left_y = local_pivot_y - screen_pivot_y;

    uint16_t hres = get_hres();
    uint16_t vres = get_vres();
    uint32_t transparent_color_val = sprite->map[sprite->width * sprite->height - 1];

    // Iterate through the sprite's pixels
    for (int sprite_y_offset = draw_top_left_y; sprite_y_offset < draw_top_left_y + sprite->height; ++sprite_y_offset) {
        for (int sprite_x_offset = draw_top_left_x; sprite_x_offset < draw_top_left_x + sprite->width; ++sprite_x_offset) {

            // Current screen pixel to draw
            int current_screen_x = sprite_x_offset - draw_top_left_x;
            int current_screen_y = sprite_y_offset - draw_top_left_y;

            // Basic screen clipping
            if (current_screen_x < 0 || current_screen_x >= hres ||
                current_screen_y < 0 || current_screen_y >= vres) {
                continue;
                }

            uint32_t color = sprite->map[sprite_y_offset * sprite->width + sprite_x_offset];

            if (has_transparent && color == transparent_color_val) {
                continue;
            }
            vg_draw_pixel(current_screen_x, current_screen_y, color);
        }
    }
    return 0;
     */

    float sprite_width_f = (float)sprite->width;
    float sprite_height_f = (float)sprite->height;

    float corners_rel_pivot_x[4], corners_rel_pivot_y[4];
    corners_rel_pivot_x[0] = 0.0f - local_pivot_x;             corners_rel_pivot_y[0] = 0.0f - local_pivot_y;
    corners_rel_pivot_x[1] = sprite_width_f - local_pivot_x;   corners_rel_pivot_y[1] = 0.0f - local_pivot_y;
    corners_rel_pivot_x[2] = sprite_width_f - local_pivot_x;   corners_rel_pivot_y[2] = sprite_height_f - local_pivot_y;
    corners_rel_pivot_x[3] = 0.0f - local_pivot_x;             corners_rel_pivot_y[3] = sprite_height_f - local_pivot_y;

    float min_rot_x_rel_pivot = 1e9, max_rot_x_rel_pivot = -1e9;
    float min_rot_y_rel_pivot = 1e9, max_rot_y_rel_pivot = -1e9;

    for (int i = 0; i < 4; ++i) {
        float rotated_x = corners_rel_pivot_x[i] * cos_a - corners_rel_pivot_y[i] * sin_a;
        float rotated_y = corners_rel_pivot_x[i] * sin_a + corners_rel_pivot_y[i] * cos_a;
        if (rotated_x < min_rot_x_rel_pivot) min_rot_x_rel_pivot = rotated_x;
        if (rotated_x > max_rot_x_rel_pivot) max_rot_x_rel_pivot = rotated_x;
        if (rotated_y < min_rot_y_rel_pivot) min_rot_y_rel_pivot = rotated_y;
        if (rotated_y > max_rot_y_rel_pivot) max_rot_y_rel_pivot = rotated_y;
    }

    uint16_t hres = get_hres();
    uint16_t vres = get_vres();

    int start_draw_x = (int)floorf(screen_pivot_x + min_rot_x_rel_pivot);
    int end_draw_x   = (int)ceilf(screen_pivot_x + max_rot_x_rel_pivot);
    int start_draw_y = (int)floorf(screen_pivot_y + min_rot_y_rel_pivot);
    int end_draw_y   = (int)ceilf(screen_pivot_y + max_rot_y_rel_pivot);

    if (start_draw_x < 0) start_draw_x = 0;
    if (end_draw_x > hres) end_draw_x = hres;
    if (start_draw_y < 0) start_draw_y = 0;
    if (end_draw_y > vres) end_draw_y = vres;

    uint32_t transparent_color_val = sprite->map[sprite->width * sprite->height - 1];

    float dtex_x_dsx = cos_a;
    float dtex_y_dsx = -sin_a;

    for (int sy = start_draw_y; sy < end_draw_y; ++sy) {
        float rel_sy_to_pivot = (float)sy - (float)screen_pivot_y;

        float rel_sx_to_pivot_start = (float)start_draw_x - (float)screen_pivot_x;

        float current_tex_x_unrotated_rel_pivot_start = rel_sx_to_pivot_start * cos_a + rel_sy_to_pivot * sin_a;
        float current_tex_y_unrotated_rel_pivot_start = -rel_sx_to_pivot_start * sin_a + rel_sy_to_pivot * cos_a;

        for (int sx = start_draw_x; sx < end_draw_x; ++sx) {
            float delta = (float)(sx - start_draw_x);
            float current_tex_x_unrotated_rel_pivot = current_tex_x_unrotated_rel_pivot_start + delta * dtex_x_dsx;
            float current_tex_y_unrotated_rel_pivot = current_tex_y_unrotated_rel_pivot_start + delta * dtex_y_dsx;

            int tex_x = (int)roundf(current_tex_x_unrotated_rel_pivot + local_pivot_x);
            int tex_y = (int)roundf(current_tex_y_unrotated_rel_pivot + local_pivot_y);

            if (tex_x >= 0 && tex_x < sprite->width && tex_y >= 0 && tex_y < sprite->height) {
                uint32_t color = sprite->map[tex_y * sprite->width + tex_x];
                if (has_transparent && color == transparent_color_val) {
                    continue;
                }
                vg_draw_pixel(sx, sy, color);
            }
        }
    }
    return 0;
}

void sprite_destroy(Sprite *this) {
    if (this == NULL)
        return;
    if (this->map != NULL)
        free(this->map);
    free(this);
}

int sprite_get_width(Sprite *this) {
    return this->width;
}

int sprite_get_height(Sprite *this) {
    return this->height;
}

int sprite_get_x(Sprite *this) {
    return this->x;
}

int sprite_get_y(Sprite *this) {
    return this->y;
}

void sprite_set_x(Sprite *this, int x) {
    this->x = x;
}

void sprite_set_y(Sprite *this, int y) {
    this->y = y;
}

void sprite_move(Sprite *this, int dx, int dy) {
    this->x += dx;
    this->y += dy;
}
