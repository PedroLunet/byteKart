#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>
#include "model/game_state.h"
#include "sprite.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"
#include "fonts/font.h"
#include "view/ui.h"
#include "view/ui_utils.h"

typedef enum {
    MENU_MAIN,
    MENU_FINISHED_PLAY,
    MENU_FINISHED_LEADERBOARD,
    MENU_FINISHED_QUIT,
    MENU_EXITED
} MenuSubstate;

typedef struct {
    GameState base;
    MenuSubstate currentSubstate;
    Sprite *titleSprite;
    Sprite *playSprite;
    Sprite *leaderboardSprite;
    Sprite *quitSprite;
    int selectedOption;
    UIComponent *uiRoot;
} Menu;

Menu *menu_create();
void menu_destroy(Menu *this);
void menu_draw(Menu *this);
void menu_process_event(Menu *this, EventType event);
MenuSubstate menu_get_current_substate(Menu *this);
void menu_reset_state(Menu *this);

#endif 
