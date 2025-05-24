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

    float sprite_width_f = (float)sprite->width;
    float sprite_height_f = (float)sprite->height;

    float corners_rel_pivot_x[4] = { -local_pivot_x, sprite_width_f - local_pivot_x, sprite_width_f - local_pivot_x, -local_pivot_x };
    float corners_rel_pivot_y[4] = { -local_pivot_y, -local_pivot_y, sprite_height_f - local_pivot_y, sprite_height_f - local_pivot_y };

    float min_rot_x = 1e9f, max_rot_x = -1e9f, min_rot_y = 1e9f, max_rot_y = -1e9f;
    for (int i = 0; i < 4; ++i) {
        float rx = corners_rel_pivot_x[i] * cos_a - corners_rel_pivot_y[i] * sin_a;
        float ry = corners_rel_pivot_x[i] * sin_a + corners_rel_pivot_y[i] * cos_a;
        if (rx < min_rot_x) min_rot_x = rx;
        if (rx > max_rot_x) max_rot_x = rx;
        if (ry < min_rot_y) min_rot_y = ry;
        if (ry > max_rot_y) max_rot_y = ry;
    }

    uint16_t hres = get_hres();
    uint16_t vres = get_vres();

    int start_x = screen_pivot_x + (int)min_rot_x;
    int end_x   = screen_pivot_x + (int)max_rot_x + 1;
    int start_y = screen_pivot_y + (int)min_rot_y;
    int end_y   = screen_pivot_y + (int)max_rot_y + 1;

    if (start_x < 0) start_x = 0;
    if (end_x > hres) end_x = hres;
    if (start_y < 0) start_y = 0;
    if (end_y > vres) end_y = vres;

    uint32_t transparent = sprite->map[sprite->width * sprite->height - 1];

    for (int sy = start_y; sy < end_y; ++sy) {
        float rel_sy = (float)sy - (float)screen_pivot_y;
        for (int sx = start_x; sx < end_x; ++sx) {
            float rel_sx = (float)sx - (float)screen_pivot_x;
            float tex_x_f = rel_sx * cos_a + rel_sy * sin_a + local_pivot_x;
            float tex_y_f = -rel_sx * sin_a + rel_sy * cos_a + local_pivot_y;
            int tex_x = (int)(tex_x_f + 0.5f);
            int tex_y = (int)(tex_y_f + 0.5f);

            if (tex_x >= 0 && tex_x < sprite->width && tex_y >= 0 && tex_y < sprite->height) {
                uint32_t color = sprite->map[tex_y * sprite->width + tex_x];
                if (!has_transparent || color != transparent)
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
