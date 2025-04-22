#ifndef XPM_H
#define XPM_H

#include "video_card.h"

typedef struct {
    uint32_t *colors;
    uint16_t width;
    uint16_t height;
} Sprite;

extern Sprite *struct_left_line_tile;
extern Sprite *struct_right_line_tile;
extern Sprite *struct_outspace_tile;
extern Sprite *struct_track_tile;
extern Sprite *struct_track_line_tile;

int load_tiles();
int draw_horizontal_track(uint32_t y);
int load_tiles();
int draw_sprite_xpm(Sprite *sprite, int x, int y);

#endif /* XPM_H */

