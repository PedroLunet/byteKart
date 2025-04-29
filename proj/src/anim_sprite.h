#ifndef ANIM_SPRITE_H
#define ANIM_SPRITE_H

#include <stdarg.h>
#include "sprite.h"

typedef struct AnimSprite AnimSprite;

AnimSprite *create_animSprite(uint8_t no_pic, xpm_map_t pic1[], ...);
void destroy_animSprite(AnimSprite *sp);

// public methods
static int animate_impl(AnimSprite *sp);

#endif // ANIM_SPRITE_H
