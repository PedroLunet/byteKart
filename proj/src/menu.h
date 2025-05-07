#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>
#include "sprite.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

typedef enum {
    MENU_MAIN,
    MENU_FINISHED_PLAY,
    MENU_SHOW_LEADERBOARD,
    MENU_FINISHED_QUIT,
    MENU_EXITED
} MenuSubstate;

typedef struct {
    MenuSubstate currentSubstate;
    Sprite *titleSprite;
    Sprite *playSprite;
    Sprite *leaderboardSprite;
    Sprite *quitSprite;
    Sprite *cursorSprite;
    Sprite *cursorPointerSprite;
    Sprite *prev_cursor;
    uint32_t centerX;
    uint32_t centerY;
    int mouse_x;
    int mouse_y;
    int prev_mouse_x;
    int prev_mouse_y;
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
