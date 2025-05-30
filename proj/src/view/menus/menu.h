/**
 * @file menu.h
 * @brief Defines the main menu screen and related UI behavior.
 * 
 */
#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>
#include "model/game_state.h"
#include "sprites/sprite.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"
#include "fonts/font.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"

/**
 * @enum MenuSubstate
 * @brief Enum representing different substates of the main menu.
 */
typedef enum {
    MENU_MAIN,
    MENU_FINISHED_PLAY,
    MENU_FINISHED_LEADERBOARD,
    MENU_FINISHED_QUIT,
    MENU_EXITED
} MenuSubstate;

/**
 * @struct Menu
 * @brief Represents the state and UI of the main menu.
 *
 * Contains all visual components and internal state needed
 * to render and interact with the main menu.
 */
typedef struct {
    GameState base;
    MenuSubstate currentSubstate;
    Sprite *titleSprite;
    Sprite *playSprite;
    Sprite *leaderboardSprite;
    Sprite *quitSprite;
    int selectedOption;
    UIComponent *uiRoot;
} Menu;

/**
 * @brief Creates and initializes the main menu.
 *
 * @return Pointer to a new Menu object, or NULL on failure.
 */
Menu *menu_create();

/**
 * @brief Destroys the menu and frees all allocated resources.
 *
 * @param this Pointer to the Menu instance to destroy.
 */
void menu_destroy(Menu *this);

/**
 * @brief Draws the current state of the menu screen.
 *
 * @param this Pointer to the Menu instance.
 */
void menu_draw(Menu *this);

/**
 * @brief Processes an input event within the menu.
 *
 * @param this Pointer to the Menu instance.
 * @param event The input event to process.
 */
void menu_process_event(Menu *this, EventType event);

/**
 * @brief Returns the current substate of the menu.
 *
 * @param this Pointer to the Menu instance.
 * @return The current MenuSubstate.
 */
MenuSubstate menu_get_current_substate(Menu *this);

/**
 * @brief Resets the menu state, typically used on re-entry.
 *
 * @param this Pointer to the Menu instance.
 */
void menu_reset_state(Menu *this);

#endif 
