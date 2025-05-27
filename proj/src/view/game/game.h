#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdio.h>

#include "model/game_state.h"
#include "road.h"
#include "view/utils/renderer.h"
#include "view/utils/ui.h"
#include "model/player.h"
#include "model/ai_car.h"
#include "sprites/sprite.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "road.h"
#include "car.h"
#include "view/utils/loadingUI.h"
#include "model/race_result.h"
#include "cronometer.h"
#include "../menus/pause_menu.h"
#include "view/menus/finish_race_menu.h"

typedef enum {
  GAME_SUBSTATE_LOADING,
  GAME_SUBSTATE_COUNTDOWN,
  GAME_SUBSTATE_PLAYING,
  GAME_SUBSTATE_PAUSED,
  GAME_SUBSTATE_PLAYER_FINISHED,
  GAME_SUBSTATE_RACE_FINISH_DELAY,
  GAME_SUBSTATE_FINISHED_RACE,
  GAME_SUBSTATE_BACK_TO_MENU,
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

    int current_lap;
    int total_laps;
    float race_timer_s;
    bool race_started;
    bool player_has_finished;
    float player_finish_time;

    bool player_skid_input_active;
    int player_skid_input_sign; // -1 for left, 0 for none, 1 for right
    int player_turn_input_sign; // -1 for left, 0 for none, 1 for right
    bool pause_requested;
    Pause *pauseMenu;
    FinishRace *finishRaceMenu;

    float timer_count_down;
    float finish_race_delay_timer;

    float precomputed_cos_skid;
    float precomputed_sin_skid;

    Car playerCar;
    Sprite *road_sprite1;
    Sprite *road_sprite2;
    int road_y1;
    int road_y2;

    float cronometer_time;

    RaceResult current_race_positions[MAX_AI_CARS + 1];
    int current_total_racers;

} Game;

// Public Game Class Methods
Game *game_state_create_playing(int difficulty, int car_choice, char *road_data_file, char *road_surface_file);
void playing_destroy(Game *this);
void playing_draw(Game *this);
void playing_process_event(Game *this, EventType event);
void playing_update_state(Game *this);
GameRunningState playing_get_current_substate(Game *this);
void playing_reset_state(Game *this);

#endif

