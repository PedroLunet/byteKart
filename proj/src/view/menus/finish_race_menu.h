/**
 * @file finish_race_menu.h
 * @brief Handles the finish race menu UI and logic after a race concludes.
 * 
 */
#ifndef FINISH_RACE_MENU_H
#define FINISH_RACE_MENU_H

#include "model/game_state.h"
#include "model/race_result.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"

/**
 * @enum FinishRaceSubstate
 * @brief Enum to represent substates of the finish race screen.
 */
typedef enum {
  FINISH_RACE_MENU,
  FINISH_RACE_MAIN_MENU
} FinishRaceSubstate;

/**
 * @struct FinishRace
 * @brief Represents the finish race screen and its UI elements.
 *
 * Contains the game state, UI sprites, selection state, and the current substate.
 */
typedef struct {
  GameState base;
  FinishRaceSubstate currentFinishRaceSubstate;
  Sprite *finishRaceSprite;
  Sprite *currentScoreSprite;
  Sprite *mainMenuSprite;
  int selectedOption;
  UIComponent *uiRoot;
} FinishRace;

/**
 * @brief Creates a FinishRace menu state with the given race results.
 *
 * @param results Array of race results.
 * @param total_results Total number of results.
 * @return Pointer to a new FinishRace state, or NULL on failure.
 */
FinishRace *finish_race_menu_create(RaceResult *results, int total_results);

/**
 * @brief Frees resources associated with a FinishRace instance.
 *
 * @param this Pointer to the FinishRace instance to destroy.
 */
void finish_race_menu_destroy(FinishRace *this);

/**
 * @brief Draws the current finish race menu state.
 *
 * @param this Pointer to the FinishRace instance.
 */
void finish_race_draw(FinishRace *this);

/**
 * @brief Processes an input event in the finish race menu.
 *
 * @param this Pointer to the FinishRace instance.
 * @param event The event to handle (keyboard or controller input).
 */
void finish_race_process_event(FinishRace *this, EventType event);

/**
 * @brief Gets the current substate of the finish race screen.
 *
 * @param this Pointer to the FinishRace instance.
 * @return The current FinishRaceSubstate.
 */
FinishRaceSubstate finish_race_get_current_substate(FinishRace *this);

/**
 * @brief Resets the finish race state (when re-entering the screen).
 *
 * @param this Pointer to the FinishRace instance.
 */
void finish_race_reset_state(FinishRace *this);

#endif
