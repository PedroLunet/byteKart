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

    return this;
}

int sprite_draw_xpm(Sprite *this, int x, int y) {
    uint16_t height = this->height;
    uint16_t width = this->width;
    uint32_t current_color;

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            current_color = this->map[w + h * width];
            if (current_color == TRANSPARENT)
                continue;
            if (vg_draw_pixel(x + w, y + h, current_color) != 0)
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
