#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include <stdint.h>
#include <stdio.h>
#include <stdio.h>

#include "controller/video_card.h"
#include "controller/i8042.h"
#include "controller/i8254.h"
#include "controller/kbc.h"
#include "controller/mouse.h"
#include "lcom/timer.h"
#include "sprites/sprite.h"
#include "macros.h"
#include "fonts/font.h"
#include "xpm/xpm_files.h"

#include "model/game_state.h"
#include "view/menus/menu.h"
#include "view/menus/select_difficulty.h"
#include "view/menus/select_car.h"
#include "view/game/game.h"
#include "view/menus/pause_menu.h"
#include "view/menus/gameover_menu.h"
#include "view/menus/leaderboard_menu.h"

typedef enum {
  MENU,
  SELECT_DIFFICULTY,
  LEADERBOARD,
  SELECT_CAR,
  SELECT_TRACK,
  GAME,
  GAMEOVER,
  QUIT
} MainState;

typedef void (*InterruptHandler)();

InterruptHandler interruptHandlers[NUM_EVENTS] = {
  NULL,
  timer_int_handler,
  kbc_ih,
  mouse_ih,
  // handleSerialInterrupt,
};

#endif //MAIN_H
