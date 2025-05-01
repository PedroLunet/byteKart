#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>
#include "main.h"
#include "sprite.h"

typedef enum {
    MENU_MAIN,
    MENU_FINISHED_PLAY,
    MENU_FINISHED_QUIT,
    MENU_EXITED
} MenuSubstate;

typedef struct {
    MenuSubstate currentSubstate;
    Sprite *titleSprite;
    Sprite *playSprite;
    Sprite *leaderboardSprite;
    Sprite *quitSprite;
    uint32_t centerX;
    uint32_t centerY;
    int selectedOption;
} Menu;

void (draw_mouse_pointer)(int x, int y, bool is_hovering);
void (clear_mouse_pointer)(int x, int y);

void (draw_static_menu)();
int (navigate_main_menu)();

bool (is_mouse_over_option)(int mouse_x, int mouse_y, int *selected);
void (update_mouse_position)(int *x, int *y);

int (draw_game_over_screen)();

// Public Menu Class Method
Menu *menu_create();
void menu_destroy(Menu *this);
void menu_draw(Menu *this);
void menu_process_event(Menu *this, EventType event);
MenuSubstate menu_get_current_substate(Menu *this);
void menu_reset_state(Menu *this);

#endif /* MENU_H_ */
