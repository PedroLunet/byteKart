/**
 * @file pause_menu.h
 * @brief Declares the interface for the pause menu system in the game.
 *
 */
#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include "model/game_state.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"
#include "view/game/cronometer.h"

/**
 * @enum PauseSubstate
 * @brief Substates representing the actions within the pause menu.
 */
typedef enum {
  PAUSE_MENU,
  PAUSE_RESUME,
  PAUSE_MAIN_MENU
} PauseSubstate;

/**
 * @struct Pause
 * @brief Represents the pause menu state and its UI logic.
 *
 * This struct encapsulates the game pause state, UI navigation,
 * and menu substate transitions.
 */
typedef struct Pause {
  GameState base;
  PauseSubstate currentPauseSubstate;
  int selectedOption;
  UIComponent *uiRoot;
} Pause;

/**
 * @brief Creates and initializes the pause menu state.
 *
 * @return Pointer to the new Pause menu object, or NULL on failure.
 */
Pause *pause_menu_create();

/**
 * @brief Destroys the pause menu and releases its resources.
 *
 * @param this Pointer to the Pause menu instance to destroy.
 */
void pause_menu_destroy(Pause *this);

/**
 * @brief Renders the current state of the pause menu.
 *
 * @param this Pointer to the Pause menu instance.
 */
void pause_draw(Pause *this);

/**
 * @brief Handles input events while in the pause menu.
 *
 * @param this Pointer to the Pause menu instance.
 * @param event The event to be processed.
 */
void pause_process_event(Pause *this, EventType event);

/**
 * @brief Returns the current substate of the pause menu.
 *
 * @param this Pointer to the Pause menu instance.
 * @return The current PauseSubstate.
 */
PauseSubstate pause_get_current_substate(Pause *this);

/**
 * @brief Resets the pause menu to its initial state.
 *
 * @param this Pointer to the Pause menu instance.
 */
void pause_reset_state(Pause *this);

#endif
