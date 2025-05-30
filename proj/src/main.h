/**
 * @file main.h
 * @brief Main header for managing core game states and system-level interrupt handlers.
 *
 */
#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include <stdint.h>
#include <stdio.h>
#include <stdio.h>

#include "controller/video_card.h"
#include "controller/i8042.h"
#include "controller/i8254.h"
#include "controller/kbc.h"
#include "controller/mouse.h"
#include "lcom/timer.h"
#include "sprites/sprite.h"
#include "macros.h"
#include "fonts/font.h"
#include "xpm/xpm_files.h"

#include "model/game_state.h"
#include "view/menus/menu.h"
#include "view/menus/select_difficulty.h"
#include "view/menus/select_car.h"
#include "view/game/game.h"
#include "view/menus/pause_menu.h"
#include "view/menus/leaderboard_menu.h"

/**
 * @enum MainState
 * @brief Enumerates the top-level application states.
 */
typedef enum {
  MENU,               /**< Main menu screen. */
  SELECT_DIFFICULTY,  /**< Difficulty selection screen. */
  LEADERBOARD,        /**< Leaderboard screen. */
  SELECT_CAR,         /**< Car selection screen. */
  SELECT_TRACK,       /**< Track selection screen (not yet implemented). */
  GAME,               /**< Main gameplay state. */
  QUIT                /**< Application exit request. */
} MainState;

/**
 * @typedef InterruptHandler
 * @brief Defines a function pointer type for interrupt service routines.
 */
typedef void (*InterruptHandler)();

/**
 * @var interruptHandlers
 * @brief Array of interrupt handlers mapped by EventType.
 *
 * Indexed using the EventType enum defined elsewhere.
 */
InterruptHandler interruptHandlers[NUM_EVENTS] = {
  NULL,              /**< No event (placeholder for index alignment). */
  timer_int_handler, /**< Timer interrupt handler. */
  kbc_ih,            /**< Keyboard controller interrupt handler. */
  mouse_ih           /**< Mouse interrupt handler. */
};

#endif //MAIN_H
