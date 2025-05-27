#ifndef FINISH_RACE_MENU_H
#define FINISH_RACE_MENU_H

#include "model/game_state.h"
#include "model/race_result.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"

typedef enum {
  FINISH_RACE_MENU,
  FINISH_RACE_MAIN_MENU
} FinishRaceSubstate;

typedef struct {
  GameState base;
  FinishRaceSubstate currentFinishRaceSubstate;
  Sprite *finishRaceSprite;
  Sprite *currentScoreSprite;
  Sprite *mainMenuSprite;
  int selectedOption;
  UIComponent *uiRoot;
} FinishRace;

FinishRace *finish_race_menu_create(RaceResult *results, int total_results);
void finish_race_menu_destroy(FinishRace *this);
void finish_race_draw(FinishRace *this);
void finish_race_process_event(FinishRace *this, EventType event);
FinishRaceSubstate finish_race_get_current_substate(FinishRace *this);
void finish_race_reset_state(FinishRace *this);

#endif
