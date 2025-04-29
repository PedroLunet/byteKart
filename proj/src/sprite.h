#ifndef XPM_H
#define XPM_H

#include "video_card.h"

typedef struct {
    int x, y;
    int width, height;
    int xspeed, yspeed;
    uint32_t *map;
} Sprite;

extern char *pixil_frame_0[];

Sprite *create_sprite_xpm(xpm_map_t pic, int x, int y, int xspeed, int yspeed);
int draw_sprite_xpm(Sprite *sprite, int x, int y);
void destroy_sprite(Sprite *sp);

#endif /* XPM_H */

