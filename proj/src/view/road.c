#include <lcom/lcf.h>
#include "road.h"

extern vbe_mode_info_t vbe_mode_info;

void draw_road_background(Sprite *road_sprite1, Sprite *road_sprite2, int road_y1, int road_y2) {
    if (!road_sprite1 || !road_sprite2) return;
    sprite_draw_xpm(road_sprite1, 0, road_y1, false);
    sprite_draw_xpm(road_sprite2, 0, road_y2, false);
}

void cleanup_road_background() {
}

