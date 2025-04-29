#include <lcom/lcf.h>
#include "sprite.h"
#include "xpm/tiles.h"
#include "macros.h"

Sprite *create_sprite_xpm(xpm_map_t pic, int x, int y, int xspeed, int yspeed) {
    Sprite *sp = (Sprite *) malloc(sizeof(Sprite));
    if (sp == NULL) return NULL;

    xpm_image_t img;
    sp->map = (uint32_t *) xpm_load(pic, XPM_8_8_8_8, &img);
    if (sp->map == NULL) {
        free(sp);
        return NULL;
    }

    sp->width = img.width; sp->height=img.height;
    sp->x = x; sp->y = y;
    sp->xspeed = xspeed; sp->yspeed = yspeed;
    
    return sp;
}

int draw_sprite_xpm(Sprite *sprite, int x, int y) {
    uint16_t height = sprite->height;
    uint16_t width = sprite->width;
    uint32_t current_color;

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            current_color = sprite->map[w + h * width];
            if (current_color == TRANSPARENT) continue;
            if (vg_draw_pixel(x + w, y + h, current_color) != 0) return 1;
        }
    }

    return 0;
}

void destroy_sprite(Sprite *sp) {
    if (sp == NULL) return;
    free(sp->map); 
    free(sp);     
}
