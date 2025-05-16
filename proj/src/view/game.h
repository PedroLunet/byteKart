#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdio.h>

#include "model/game_state.h"
#include "view/road.h"
#include "model/player.h"
#include "model/ai_car.h"
#include "sprite.h"
#include "macros.h"
#include "xpm/xpm_files.h"

typedef enum {
  GAME_SUBSTATE_LOADING,
  GAME_SUBSTATE_COUNTDOWN,
  GAME_SUBSTATE_PLAYING,
  GAME_SUBSTATE_PAUSED,
  GAME_SUBSTATE_FINISHED_RACE,
  GAME_STATE_EXITING,
  GAME_EXITED
} GameRunningState;

typedef struct Game {
    GameState base;
    GameRunningState current_running_state;

    Road road_data;
    Player player;
    AICar* ai_cars[MAX_AI_CARS];
    int num_active_ai_cars;

    // Game specific logic variables
    int current_lap;
    float race_timer_s;
    bool race_started;

    // Input state
    bool player_skid_input_active;
    int player_turn_input_sign; // -1 for left, 0 for none, 1 for right
    bool pause_requested;

} Game;

// Public Game Class Methods
Game *game_state_create_playing(int difficulty, xpm_map_t *player_xpm, char *road_data_file, xpm_map_t *road_xpm, xpm_map_t *finish_xpm);
void playing_destroy(Game *this);
void playing_draw(Game *this);
void playing_process_event(Game *this, EventType event);
void playing_update_state(Game *this);
GameRunningState playing_get_current_substate(Game *this);
void playing_reset_state(Game *this);

#endif

