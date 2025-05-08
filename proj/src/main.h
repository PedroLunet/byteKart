#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdio.h>

#include "controller/video_card.h"
#include "controller/i8042.h"
#include "controller/i8254.h"
#include "controller/kbc.h"
#include "controller/mouse.h"
#include "lcom/timer.h"
#include "sprite.h"
#include "macros.h"
#include "game.h"
#include "road.h"

typedef enum {
  MENU,
  SELECT_DIFFICULTY,
  PLAY,
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


#include "model/game_state.h"
#include "menu.h"
#include "select_car.h"


#endif //MAIN_H
