#ifndef SPRITE_H_
#define SPRITE_H_

#include <stdint.h>
#include <stdio.h>

#include "controller/video_card.h"

typedef struct {
    uint32_t *map;
    int x, y;
    int width, height;
    int xspeed, yspeed;
    uint8_t bytes_per_pixel;
    uint8_t *data;
} Sprite;

// Sprite Class Method Prototypes
Sprite *sprite_create_xpm(xpm_map_t pic, int x, int y, int xspeed, int yspeed);
void sprite_destroy(Sprite *this);
int sprite_draw_xpm(Sprite *this, int x, int y, bool has_transparent);
int sprite_draw_partial_xpm(Sprite *this, int x, int y, int width, int height, bool has_transparent);
int sprite_draw_rotated_xpm(Sprite *sprite, int center_screen_x, int center_screen_y, float cos_rotation, float sin_rotation, bool has_transparent);

// Getters and Setters (if needed)
int sprite_get_width(Sprite *this);
int sprite_get_height(Sprite *this);
int sprite_get_x(Sprite *this);
int sprite_get_y(Sprite *this);
void sprite_set_x(Sprite *this, int x);
void sprite_set_y(Sprite *this, int y);

// Move the sprite
void sprite_move(Sprite *this, int dx, int dy);

#endif /* SPRITE_H_ */
