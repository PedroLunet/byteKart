/**
 * @file leaderboard.h
 * @brief Manages the leaderboard screen and entries in the game.
 *
 */

#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "model/game_state.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"


/**
 * @def MAX_ENTRIES
 * @brief Maximum number of leaderboard entries stored.
 */
#define MAX_ENTRIES 4

/**
 * @def MAX_NAME_LENGTH
 * @brief Maximum length of a player's name in the leaderboard.
 */
#define MAX_NAME_LENGTH 9


/**
 * @struct LeaderboardEntries
 * @brief Stores a single leaderboard entry with a name and time.
 */
typedef struct LeaderboardEntries {
  char name[MAX_NAME_LENGTH];
  float time;
} LeaderboardEntries;

/**
 * @brief Current number of entries in the leaderboard.
 */
extern int count_leaderboard_entries;

/**
 * @brief Global array storing the leaderboard entries.
 */
extern LeaderboardEntries leaderboard_entries[MAX_ENTRIES];

/**
 * @enum LeaderboardSubstate
 * @brief Enum representing substates within the leaderboard screen.
 */
typedef enum {
  LEADERBOARD_MENU,
  LEADERBOARD_BACK_TO_MENU
} LeaderboardSubstate;

/**
 * @struct Leaderboard
 * @brief Represents the leaderboard screen's UI state and components.
 */
typedef struct {
  GameState base;
  LeaderboardSubstate currentSubstate;
  Sprite *leaderboardSprite;
  UIComponent *uiRoot;
  UIComponent *backButton;
} Leaderboard;

/**
 * @brief Creates and initializes the leaderboard state.
 *
 * @return Pointer to the new Leaderboard object, or NULL on failure.
 */
Leaderboard *leaderboard_create();

/**
 * @brief Destroys the given leaderboard instance and frees its resources.
 *
 * @param this Pointer to the Leaderboard instance to destroy.
 */
void leaderboard_destroy(Leaderboard *this);

/**
 * @brief Draws the leaderboard screen and UI elements.
 *
 * @param this Pointer to the Leaderboard instance.
 */
void leaderboard_draw(Leaderboard *this);

/**
 * @brief Processes an input event in the leaderboard screen.
 *
 * @param this Pointer to the Leaderboard instance.
 * @param event Event to be handled.
 */
void leaderboard_process_event(Leaderboard *this, EventType event);

/**
 * @brief Returns the current substate of the leaderboard.
 *
 * @param this Pointer to the Leaderboard instance.
 * @return Current substate of the leaderboard.
 */
LeaderboardSubstate leaderboard_get_current_substate(Leaderboard *this);

/**
 * @brief Resets the leaderboard state.
 *
 * @param this Pointer to the Leaderboard instance.
 */
void leaderboard_reset_state(Leaderboard *this);

/**
 * @brief Adds a new entry to the leaderboard.
 *
 * @param entries Array of leaderboard entries.
 * @param count Pointer to the current count of entries.
 * @param name Player name to be added.
 * @param time Player time to be added.
 */
void add_entry_to_leaderboard(LeaderboardEntries entries[], int *count, const char *name, float time);



#endif

