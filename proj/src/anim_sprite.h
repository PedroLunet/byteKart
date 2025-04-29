#ifndef ANIM_SPRITE_H
#define ANIM_SPRITE_H

#include <stdarg.h>
#include "sprite.h"

typedef struct {
    Sprite *sp;
    int aspeed; // no. frames per pixmap
    int cur_aspeed; // no. frames left to next change
    int num_fig; // number of pixmaps
    int cur_fig; // current pixmap
    uint32_t **map; // array of pointers to pixmaps
} AnimSprite;

AnimSprite *create_animSprite(uint8_t no_pic, xpm_map_t pic1[], ...);
int animate_animSprite(AnimSprite *sp,);
void destroy_animSprite(AnimSprite *sp);

#endif //ANIM_SPRITE_H
