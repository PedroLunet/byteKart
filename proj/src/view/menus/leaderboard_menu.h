#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "model/game_state.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"

typedef enum {
  LEADERBOARD_MENU,
  LEADERBOARD_BACK_TO_MENU
} LeaderboardSubstate;

typedef struct {
  GameState base;
  LeaderboardSubstate currentSubstate;
  Sprite *leaderboardSprite;
  UIComponent *uiRoot;
  UIComponent *backButton;
} Leaderboard;

Leaderboard *leaderboard_create();
void leaderboard_destroy(Leaderboard *this);
void leaderboard_draw(Leaderboard *this);
void leaderboard_process_event(Leaderboard *this, EventType event);
LeaderboardSubstate leaderboard_get_current_substate(Leaderboard *this);
void leaderboard_reset_state(Leaderboard *this);


#endif

