#ifndef _MENU_H_
#define _MENU_H_

#include "xpm/xpm_files.h" 
#include "sprite.h"
#include "macros.h"
#include "kbc.h"

void (draw_static_menu)(Sprite *title_sprite, Sprite *play_sprite, Sprite *quit_sprite, uint32_t y_center);
void (select_main_menu_option)(int option, Sprite *title_sprite, Sprite *play_sprite, Sprite *quit_sprite, uint32_t x_center, uint32_t y_center);
int (draw_main_screen)();
int (navigate_main_menu)();
int (draw_game_over_screen)();

#endif
