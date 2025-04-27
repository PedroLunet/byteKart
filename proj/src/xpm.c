#include <lcom/lcf.h>
#include "xpm.h"
#include "xpm/tiles.h"

#define TRANSPARENT 0xFFFFFE

extern vbe_mode_info_t vbe_mode_info;

Sprite *struct_left_line_tile;
Sprite *struct_right_line_tile;
Sprite *struct_outspace_tile;
Sprite *struct_track_tile;
Sprite *struct_track_line_tile;

uint32_t track_tiles = 7;
uint32_t x_offset = 0;

Sprite *create_sprite_xpm(xpm_map_t sprite) {
    Sprite *sp = (Sprite *) malloc(sizeof(Sprite));
    if (sp == NULL) return NULL;

    xpm_image_t img;
    sp->colors = (uint32_t *) xpm_load(sprite, XPM_8_8_8_8, &img);
    sp->height = img.height;
    sp->width = img.width;

    if (sp->colors == NULL) {
        free(sp);
        return NULL;
    }
    
    return sp;
}

int draw_sprite_xpm(Sprite *sprite, int x, int y) {
    uint16_t height = sprite->height;
    uint16_t width = sprite->width;
    uint32_t current_color;

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            current_color = sprite->colors[w + h * width];
            if (current_color == TRANSPARENT) continue;
            if (vg_draw_pixel(x + w, y + h, current_color) != 0) return 1;
        }
    }

    return 0;
}


int load_tiles() {
    struct_left_line_tile = create_sprite_xpm((xpm_map_t) left_line_tile);
    if (struct_left_line_tile == NULL) return 1;

    struct_right_line_tile = create_sprite_xpm((xpm_map_t) right_line_tile);
    if (struct_right_line_tile == NULL) return 1;

    struct_outspace_tile = create_sprite_xpm((xpm_map_t) outspace_tile);
    if (struct_outspace_tile == NULL) return 1;

    struct_track_tile = create_sprite_xpm((xpm_map_t) track_tile);
    if (struct_track_tile == NULL) return 1;

    struct_track_line_tile = create_sprite_xpm((xpm_map_t) line_tile);
    if (struct_track_line_tile == NULL) return 1;

    uint32_t tile_width = struct_track_tile->width;
    track_tiles = (vbe_mode_info.XResolution / tile_width) - 4; // 4 non-track tiles

    if (track_tiles < 1) track_tiles = 1;
    if (track_tiles % 2 == 0) track_tiles--;

    uint32_t row_width = (track_tiles + 4) * tile_width;
    x_offset = (vbe_mode_info.XResolution - row_width) / 2;

    return 0;
}

int draw_horizontal_track(uint32_t y) {
    uint32_t tile_width = struct_track_tile->width;
    uint32_t width_drawn = x_offset;

    if (draw_sprite_xpm(struct_outspace_tile, width_drawn, y) != 0) return 1;
    width_drawn += tile_width;

    if (draw_sprite_xpm(struct_left_line_tile, width_drawn, y) != 0) return 1;
    width_drawn += tile_width;

    for (uint32_t i = 0; i < track_tiles; i++) {
        if (draw_sprite_xpm(struct_track_tile, width_drawn, y) != 0) return 1;

        // Optional track line for visual lanes
        if (i == track_tiles / 3 || i == 2 * track_tiles / 3) {
            if (draw_sprite_xpm(struct_track_line_tile, width_drawn, y) != 0) return 1;
        }

        width_drawn += tile_width;
    }

    if (draw_sprite_xpm(struct_right_line_tile, width_drawn, y) != 0) return 1;
    width_drawn += tile_width;

    if (draw_sprite_xpm(struct_outspace_tile, width_drawn, y) != 0) return 1;
    width_drawn += tile_width;

    return 0;
}
