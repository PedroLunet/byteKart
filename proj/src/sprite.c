#include <lcom/lcf.h>
#include "sprite.h"
#include "macros.h"

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

int sprite_draw_rotated_xpm(Sprite *sprite, int center_screen_x, int center_screen_y, float cos_a, float sin_a, bool has_transparent) {
    if (!sprite || !sprite->map) return 1;

    float sprite_half_width = sprite->width / 2.0f;
    float sprite_half_height = sprite->height / 2.0f;

    // Bounding box calculation uses the passed cos_a and sin_a
    float corners_x_rel[4], corners_y_rel[4];
    float world_corners_x[4], world_corners_y[4];

    corners_x_rel[0] = -sprite_half_width; corners_y_rel[0] = -sprite_half_height;
    corners_x_rel[1] =  sprite_half_width; corners_y_rel[1] = -sprite_half_height;
    corners_x_rel[2] =  sprite_half_width; corners_y_rel[2] =  sprite_half_height;
    corners_x_rel[3] = -sprite_half_width; corners_y_rel[3] =  sprite_half_height;

    float min_screen_x_bb_rel =  1e9; float max_screen_x_bb_rel = -1e9;
    float min_screen_y_bb_rel =  1e9; float max_screen_y_bb_rel = -1e9;

    for (int i = 0; i < 4; ++i) {
        world_corners_x[i] = corners_x_rel[i] * cos_a - corners_y_rel[i] * sin_a;
        world_corners_y[i] = corners_x_rel[i] * sin_a + corners_y_rel[i] * cos_a;

        if (world_corners_x[i] < min_screen_x_bb_rel) min_screen_x_bb_rel = world_corners_x[i];
        if (world_corners_x[i] > max_screen_x_bb_rel) max_screen_x_bb_rel = world_corners_x[i];
        if (world_corners_y[i] < min_screen_y_bb_rel) min_screen_y_bb_rel = world_corners_y[i];
        if (world_corners_y[i] > max_screen_y_bb_rel) max_screen_y_bb_rel = world_corners_y[i];
    }

    uint16_t hres = get_hres();
    uint16_t vres = get_vres();

    int start_draw_x = (int)floorf(center_screen_x + min_screen_x_bb_rel);
    int end_draw_x   = (int)ceilf(center_screen_x + max_screen_x_bb_rel);
    int start_draw_y = (int)floorf(center_screen_y + min_screen_y_bb_rel);
    int end_draw_y   = (int)ceilf(center_screen_y + max_screen_y_bb_rel);

    if (start_draw_x < 0) start_draw_x = 0;
    if (end_draw_x > hres) end_draw_x = hres;
    if (start_draw_y < 0) start_draw_y = 0;
    if (end_draw_y > vres) end_draw_y = vres;

    uint32_t transparent_color_val = sprite->map[sprite->width * sprite->height - 1];

    for (int sy = start_draw_y; sy < end_draw_y; ++sy) {
        float dy_screen = (float)sy - (float)center_screen_y;

        float base_dx_screen = (float)start_draw_x - (float)center_screen_x;
        // Pre-calculate for the start of the row, using passed cos_a and sin_a
        float current_tex_x_center_rel = base_dx_screen * cos_a + dy_screen * sin_a;
        float current_tex_y_center_rel = -base_dx_screen * sin_a + dy_screen * cos_a;

        for (int sx = start_draw_x; sx < end_draw_x; ++sx) {
            if (sx > start_draw_x) {
                current_tex_x_center_rel += cos_a;
                current_tex_y_center_rel -= sin_a;
            }

            int tex_x = (int)roundf(current_tex_x_center_rel + sprite_half_width);
            int tex_y = (int)roundf(current_tex_y_center_rel + sprite_half_height);

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
