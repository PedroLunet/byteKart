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
