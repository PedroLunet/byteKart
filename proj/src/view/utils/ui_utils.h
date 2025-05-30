#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <stdlib.h>

#include "ui.h"
#include "fonts/font.h" 
#include "model/game_state.h"

UIComponent *create_menu_option(const char *text, Font *font, int width, int height, UIComponent *parent);

bool is_mouse_over_menu_options(GameState *base, int mouse_x, int mouse_y, UIComponent **options, size_t num_options, int *selected_option, uint32_t default_color, uint32_t hover_color);

UIComponent *create_back_button(Font *font, UIComponent *parent);

UIComponent *create_title_text(const char *title, Font *font, uint32_t color, UIComponent *parent);

UIComponent *create_main_container(Sprite *background, int gap, int top_pad, int bot_pad, int left_pad, int right_pad);

UIComponent *create_row_options(int gap, int layout, UIComponent *parent);

bool is_mouse_over_back_button(UIComponent *backButton, int mouse_x, int mouse_y, int *selected_option, int option);
#endif

