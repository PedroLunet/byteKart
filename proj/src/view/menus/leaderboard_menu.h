#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "model/game_state.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"

#define MAX_ENTRIES 4
#define MAX_NAME_LENGTH 9

typedef struct LeaderboardEntries {
  char name[MAX_NAME_LENGTH];
  float time;
} LeaderboardEntries;

extern int count_leaderboard_entries;
extern LeaderboardEntries leaderboard_entries[MAX_ENTRIES];

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
void add_entry_to_leaderboard(LeaderboardEntries entries[], int *count, const char *name, float time);


#endif

