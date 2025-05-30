/**
 * @file select_car.h
 * @brief Provides the interface for the car selection menu in the game.
 *
 */
#ifndef SELECT_CAR_H
#define SELECT_CAR_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "model/game_state.h"
#include "xpm/xpm_files.h"
#include "view/utils/ui.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"
#include "view/utils/ui_utils.h"

/**
 * @brief Array of XPM maps representing the available car choices.
 */
extern const xpm_map_t car_choices[6];

/**
 * @enum CarSelection
 * @brief Represents the different states of the car selection process.
 */
typedef enum {
    CAR_START,
    CAR_SELECTED,
    CAR_EXITED
} CarSelection;

/**
 * @struct SelectCar
 * @brief Represents the car selection screen and its UI components.
 *
 * Manages user interaction, current selection, and UI navigation logic.
 */
typedef struct SelectCar {
    GameState base;
    int selectedOption;
    CarSelection chosenLevel;
    UIComponent *uiRoot;
    UIComponent *backButton;
} SelectCar;

/**
 * @brief Creates and initializes the car selection screen.
 *
 * @return Pointer to the new SelectCar object, or NULL on failure.
 */
SelectCar *select_car_create();

/**
 * @brief Destroys the car selection screen and frees its resources.
 *
 * @param this Pointer to the SelectCar instance to destroy.
 */
void select_car_destroy(SelectCar *this);

/**
 * @brief Draws the car selection UI on the screen.
 *
 * @param this Pointer to the SelectCar instance.
 */
void select_car_draw(SelectCar *this);

/**
 * @brief Processes user input events in the car selection menu.
 *
 * @param this Pointer to the SelectCar instance.
 * @param event The input event to process.
 */
void select_car_process_event(SelectCar *this, EventType event);

/**
 * @brief Retrieves the currently selected car option.
 *
 * @param this Pointer to the SelectCar instance.
 * @return Index of the selected car.
 */
int select_car_get_selected_option(SelectCar *this);

/**
 * @brief Gets the current car selection substate.
 *
 * @param this Pointer to the SelectCar instance.
 * @return The current CarSelection state.
 */
CarSelection select_car_get_chosen_level(SelectCar *this);

/**
 * @brief Resets the car selection state to its initial configuration.
 *
 * @param this Pointer to the SelectCar instance.
 */
void select_car_reset_state(SelectCar *this);

#endif
