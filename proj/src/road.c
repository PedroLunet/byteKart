#include <lcom/lcf.h>
#include "road.h"

static Sprite *road_sprite1 = NULL;
static Sprite *road_sprite2 = NULL;
static int road_y1 = 0;
static int road_y2 = 0;
extern vbe_mode_info_t vbe_mode_info;

void draw_road_background() {
    if (!road_sprite1 || !road_sprite2) {
        road_sprite1 = sprite_create_xpm((xpm_map_t) road_xpm, 0, 0, 0, 0);
        road_sprite2 = sprite_create_xpm((xpm_map_t) road_xpm, 0, 0, 0, 0);
        if (!road_sprite1 || !road_sprite2) return;
        road_y1 = 0;
        road_y2 = -road_sprite1->height;
    }
    sprite_draw_xpm(road_sprite1, 0, road_y1);
    sprite_draw_xpm(road_sprite2, 0, road_y2);
}

void cleanup_road_background() {
    if (road_sprite1) {
        sprite_destroy(road_sprite1);
        road_sprite1 = NULL;
    }
    if (road_sprite2) {
        sprite_destroy(road_sprite2);
        road_sprite2 = NULL;
    }
}

