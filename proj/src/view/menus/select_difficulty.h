/**
 * @file select_difficulty.h
 * @brief Declares the interface for the difficulty selection menu.
 *
 */
#ifndef SELECT_DIFFICULTY_H
#define SELECT_DIFFICULTY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "model/game_state.h"
#include "view/utils/ui.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"
#include "xpm/xpm_files.h"
#include "view/utils/ui_utils.h"

/**
 * @enum DifficultyLevel
 * @brief Represents the selection state of the difficulty menu.
 */
typedef enum {
    DIFFICULTY_START,
    DIFFICULTY_SELECTED,
    DIFFICULTY_EXITED
} DifficultyLevel;

/**
 * @struct SelectDifficulty
 * @brief Represents the difficulty selection screen and its state.
 *
 * Manages user interaction, current difficulty selection, and UI layout.
 */
typedef struct SelectDifficulty {
    GameState base;
    int selectedOption;
    DifficultyLevel chosenLevel;
    UIComponent *uiRoot;
    UIComponent *backButton;
} SelectDifficulty;

/**
 * @brief Creates and initializes a new difficulty selection screen.
 *
 * @return Pointer to a new SelectDifficulty instance, or NULL on failure.
 */
SelectDifficulty *select_difficulty_create();

/**
 * @brief Destroys the difficulty selection screen and releases resources.
 *
 * @param this Pointer to the SelectDifficulty instance to destroy.
 */
void select_difficulty_destroy(SelectDifficulty *this);

/**
 * @brief Renders the difficulty selection screen.
 *
 * @param this Pointer to the SelectDifficulty instance.
 */
void select_difficulty_draw(SelectDifficulty *this);

/**
 * @brief Processes user input events in the difficulty selection menu.
 *
 * @param this Pointer to the SelectDifficulty instance.
 * @param event The event to process (keyboard, mouse, etc.).
 */
void select_difficulty_process_event(SelectDifficulty *this, EventType event);

/**
 * @brief Retrieves the index of the selected difficulty option.
 *
 * @param this Pointer to the SelectDifficulty instance.
 * @return The selected option index.
 */
int select_difficulty_get_selected_option(SelectDifficulty *this);

/**
 * @brief Gets the current difficulty selection state.
 *
 * @param this Pointer to the SelectDifficulty instance.
 * @return The current DifficultyLevel state.
 */
DifficultyLevel select_difficulty_get_chosen_level(SelectDifficulty *this);

/**
 * @brief Resets the difficulty selection screen to its initial state.
 *
 * @param this Pointer to the SelectDifficulty instance.
 */
void select_difficulty_reset_state(SelectDifficulty *this);

#endif //SELECT_DIFFICULTY_H
