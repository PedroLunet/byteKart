#include <lcom/lcf.h>

#include "menu.h"
#include "xpm/xpm_files.h" 
#include "xpm.h"

extern vbe_mode_info_t vbe_mode_info;

int (draw_main_screen)() {

    //Sprite *title_sprite = create_sprite_xpm((xpm_map_t) title);
    //if (title_sprite == NULL) return 1; 

    Sprite *frame_sprite = create_sprite_xpm((xpm_map_t) play);
    if (frame_sprite == NULL) return 1;

    Sprite *quit_sprite = create_sprite_xpm((xpm_map_t) quit);
    if (quit_sprite == NULL) return 1;

    uint32_t x_center = (vbe_mode_info.XResolution - frame_sprite->width) / 2;
    uint32_t y_center = (vbe_mode_info.YResolution - frame_sprite->height) / 2;

    //if (draw_sprite_xpm(title_sprite, x_center, y_center - 10) != 0) return 1;
    if (draw_sprite_xpm(frame_sprite, x_center, y_center) != 0) return 1; 
    if (draw_sprite_xpm(quit_sprite, x_center - 7, y_center + 80) != 0) return 1;


    return 0;
}


int (draw_game_over_screen)() {

    return 0;
}