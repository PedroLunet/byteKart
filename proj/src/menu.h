#ifndef _MENU_H_
#define _MENU_H_

#include "xpm/xpm_files.h" 
#include "sprite.h"
#include "macros.h"
#include "kbc.h"
#include "mouse.h"

void (draw_mouse_pointer)(int x, int y, bool is_hovering);
void (clear_mouse_pointer)(int x, int y);

void (draw_static_menu)();
int (navigate_main_menu)();

bool (is_mouse_over_option)(int mouse_x, int mouse_y, int *selected);
void (update_mouse_position)(int *x, int *y);

int (draw_game_over_screen)();

#endif
