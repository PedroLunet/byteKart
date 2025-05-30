/**
 * @file game.h
 * @brief Declares the Game struct and main gameplay loop functions.
 *
 */

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
#include "view/menus/leaderboard_menu.h"

/**
 * @enum GameRunningState
 * @brief Enumerates the different states of the game during play.
 */
typedef enum {
  GAME_SUBSTATE_LOADING,           /**< Initial loading screen. */
  GAME_SUBSTATE_COUNTDOWN,         /**< Pre-race countdown. */
  GAME_SUBSTATE_PLAYING,           /**< Main gameplay loop. */
  GAME_SUBSTATE_PAUSED,            /**< Game is paused. */
  GAME_SUBSTATE_PLAYER_FINISHED,   /**< Player has completed the race. */
  GAME_SUBSTATE_RACE_FINISH_DELAY, /**< Waiting for other racers to finish. */
  GAME_SUBSTATE_NAME_INPUT,        /**< Player is entering their name. */
  GAME_SUBSTATE_FINISHED_RACE,     /**< Race has ended. */
  GAME_SUBSTATE_BACK_TO_MENU,      /**< Transitioning back to the menu. */
  GAME_STATE_EXITING,              /**< Game is exiting. */
  GAME_EXITED                      /**< Exit is complete. */
} GameRunningState;

/**
 * @struct Game
 * @brief Contains all state and data for a running game session.
 */
typedef struct Game {
    GameState base;                     /**< Inherited base game state. */
    GameRunningState current_running_state; /**< Current substate of gameplay. */

    Road road_data;                     /**< The current race track. */
    Player player;                      /**< The player character. */
    AICar* ai_cars[MAX_AI_CARS];        /**< Array of AI-controlled cars. */
    int num_active_ai_cars;            /**< Number of AI cars currently in race. */

    int current_lap;                   /**< Player’s current lap number. */
    int total_laps;                    /**< Total number of laps in the race. */
    float race_timer_s;               /**< Elapsed race time in seconds. */
    bool race_started;                /**< Indicates if the race has begun. */
    bool player_has_finished;         /**< Indicates if the player has crossed the finish line. */
    float player_finish_time;         /**< Time at which the player finished the race. */

    bool player_skid_input_active;     /**< Whether skid input is currently active. */
    int player_skid_input_sign;        /**< Direction of skid input (-1 = left, 1 = right). */
    int player_turn_input_sign;        /**< Direction of steering input (-1 = left, 1 = right). */
    bool pause_requested;              /**< If a pause has been requested. */
    bool replay_requested;             /**< If a replay has been requested. */
    Pause *pauseMenu;                  /**< Pointer to the pause menu instance. */
    FinishRace *finishRaceMenu;        /**< Pointer to the finish race menu. */

    float timer_count_down;            /**< Countdown before race start. */
    float finish_race_delay_timer;     /**< Delay timer after race ends before transitioning. */

    float precomputed_cos_skid;        /**< Precomputed cosine value for skid logic. */
    float precomputed_sin_skid;        /**< Precomputed sine value for skid logic. */

    Car playerCar;                     /**< Player car data. */
    Sprite *road_sprite1;              /**< Background sprite 1. */
    Sprite *road_sprite2;              /**< Background sprite 2. */
    int road_y1;                       /**< Y position for road_sprite1. */
    int road_y2;                       /**< Y position for road_sprite2. */

    float cronometer_time;             /**< Cronometer display time. */

    RaceResult current_race_positions[MAX_AI_CARS + 1]; /**< Array storing current race positions. */
    int current_total_racers;          /**< Total number of racers in the event. */

    int previous_player_score;         /**< Score before the current race. */
    float wrong_direction_timer;       /**< Time spent going in the wrong direction. */
    bool is_going_wrong_direction;     /**< Whether the player is driving incorrectly. */

    char player_name[MAX_NAME_LENGTH]; /**< Player’s name (entered after race). */
    int name_length;                   /**< Current length of entered name. */
    UIComponent *nameInputContainer;   /**< UI container for name entry. */
    UIComponent *nameInputText;        /**< UI text component for displaying entered name. */
} Game;

/**
 * @brief Initializes and returns a new Game instance for playing.
 *
 * @param difficulty Chosen difficulty level.
 * @param car_choice Selected car index.
 * @param road_data_file File path for road geometry.
 * @param road_surface_file File path for road surface image.
 * @return Pointer to the created Game instance.
 */
Game *game_state_create_playing(int difficulty, int car_choice, char *road_data_file, char *road_surface_file);

/**
 * @brief Cleans up and frees all resources used by the game.
 *
 * @param this Pointer to the Game instance.
 */
void playing_destroy(Game *this);

/**
 * @brief Draws all components of the game (road, cars, UI, etc.).
 *
 * @param this Pointer to the Game instance.
 */
void playing_draw(Game *this);

/**
 * @brief Processes input and system events while the game is active.
 *
 * @param this Pointer to the Game instance.
 * @param event The event to process.
 */
void playing_process_event(Game *this, EventType event);

/**
 * @brief Updates the game logic (physics, timers, state changes).
 *
 * @param this Pointer to the Game instance.
 */
void playing_update_state(Game *this);

/**
 * @brief Returns the current running substate of the game.
 *
 * @param this Pointer to the Game instance.
 * @return The current GameRunningState.
 */
GameRunningState playing_get_current_substate(Game *this);

/**
 * @brief Resets the current game state to its initial configuration.
 *
 * @param this Pointer to the Game instance.
 */
void playing_reset_state(Game *this);

/**
 * @brief Indicates whether the player has requested a replay.
 *
 * @param this Pointer to the Game instance.
 * @return true if a replay is requested, false otherwise.
 */
bool playing_is_replay_requested(Game *this);

#endif

