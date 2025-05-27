#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include "model/game_state.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"
#include "view/game/cronometer.h"

typedef enum {
  PAUSE_MENU,
  PAUSE_RESUME,
  PAUSE_MAIN_MENU
} PauseSubstate;

typedef struct Pause {
  GameState base;
  PauseSubstate currentPauseSubstate;
  int selectedOption;
  UIComponent *uiRoot;
} Pause;

Pause *pause_menu_create();
void pause_menu_destroy(Pause *this);
void pause_draw(Pause *this);
void pause_process_event(Pause *this, EventType event);
PauseSubstate pause_get_current_substate(Pause *this);
void pause_reset_state(Pause *this);

#endif
