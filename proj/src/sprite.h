#ifndef XPM_H
#define XPM_H

#include "video_card.h"

typedef struct {
    int x, y;
    int width, height;
    int xspeed, yspeed;
    char *map;
} Sprite;

extern char *pixil_frame_0[];

Sprite *create_sprite_xpm(xpm_map_t sprite);
int draw_sprite_xpm(Sprite *sprite, int x, int y);

#endif /* XPM_H */

