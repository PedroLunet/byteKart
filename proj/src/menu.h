#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>
#include "main.h"
#include "sprite.h"

typedef enum {
    MENU_MAIN,
    MENU_PLAY_SELECTED,
    MENU_LEADERBOARD_SELECTED,
    MENU_QUIT_SELECTED,
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

// Public Menu Class Method
Menu *menu_create();
void menu_destroy(Menu *this);
void menu_draw(Menu *this);
void menu_process_event(Menu *this, EventType event);
MenuSubstate menu_get_current_substate(Menu *this);
void menu_reset_state(Menu *this);

#endif /* MENU_H_ */
