/**
 * @file macros.h
 * @brief Common macro definitions and constants used throughout the project.
 * 
 */
#ifndef MACROS_H
#define MACROS_H

#include <math.h>

/**
 * @enum EventType
 * @brief Enumeration of different system event types.
 */
typedef enum {
  EVENT_NONE,    /**< No event */
  EVENT_TIMER,   /**< Timer interrupt event */
  EVENT_KEYBOARD,/**< Keyboard event */
  EVENT_MOUSE,   /**< Mouse event */
  // EVENT_SERIAL,/**< Serial port event (commented out) */
  NUM_EVENTS    /**< Number of event types */
} EventType;

/** @name Track file paths
 *  File paths for track data and surfaces
 */
/**@{*/
#define TRACK_1_FILENAME "/home/lcom/labs/proj/src/tracks/track_1.dat"
#define TRACK_1_SURFACE_FILENAME "/home/lcom/labs/proj/src/tracks/track_surface.bin"
/**@}*/

/** @name Font-related constants */
/**@{*/
#define MAX_FONT_GLYPHS 382
/**@}*/

/** @name Road boundaries */
/**@{*/
#define RIGHT_BOUNDARY 480
#define LEFT_BOUNDARY 130
/**@}*/

/** @name Keyboard scancodes for keys */
/**@{*/
// ESC key
#define ESC_MAKECODE 0x01
#define ESC_BREAKCODE 0x81

// Arrow keys makecodes
#define UP_ARROW 0x48
#define DOWN_ARROW 0x50
#define LEFT_ARROW 0x4B
#define RIGHT_ARROW 0x4D
#define SPACEBAR 0x39
#define P_KEY 0x19

// Arrow keys breakcodes
#define UP_ARROW_BREAK 0xC8
#define DOWN_ARROW_BREAK 0xD0
#define LEFT_ARROW_BREAK 0xCB
#define RIGHT_ARROW_BREAK 0xCD
#define SPACEBAR_BREAK 0xB9
#define P_KEY_BREAK 0xD9

// Enter key
#define ENTER_KEY 0x1C
#define ENTER_BREAKCODE 0x9C

// Backspace key
#define BACKSPACE_KEY 0x0E
#define BACKSPACE_BREAKCODE 0x8E

// Letter keys (makecodes)
#define A_KEY 0x1E
#define B_KEY 0x30
#define C_KEY 0x2E
#define D_KEY 0x20
#define E_KEY 0x12
#define F_KEY 0x21
#define G_KEY 0x22
#define H_KEY 0x23
#define I_KEY 0x17
#define J_KEY 0x24
#define K_KEY 0x25
#define L_KEY 0x26
#define M_KEY 0x32
#define N_KEY 0x31
#define O_KEY 0x18
#define P_KEY_LETTER 0x19  
#define Q_KEY 0x10
#define R_KEY 0x13
#define S_KEY 0x1F
#define T_KEY 0x14
#define U_KEY 0x16
#define V_KEY 0x2F
#define W_KEY 0x11
#define X_KEY 0x2D
#define Y_KEY 0x15
#define Z_KEY 0x2C

// Number keys (makecodes)
#define NUM_0_KEY 0x0B
#define NUM_1_KEY 0x02
#define NUM_2_KEY 0x03
#define NUM_3_KEY 0x04
#define NUM_4_KEY 0x05
#define NUM_5_KEY 0x06
#define NUM_6_KEY 0x07
#define NUM_7_KEY 0x08
#define NUM_8_KEY 0x09
#define NUM_9_KEY 0x0A
/**@}*/

/** @name Color constants */
/**@{*/
#define BACKGROUND_COLOR 0x000000
#define TRANSPARENT 0xFFFFFE
/**@}*/

/** @name Game parameters */
/**@{*/
#define MAX_LAPS 3
#define MAX_AI_CARS 3

#define FINISH_LINE_SEGMENT_IDX 400
#define LAP_APPROACH_ZONE_PERCENTAGE 0.85f
#define LAP_DEPARTURE_ZONE_PERCENTAGE 0.15f

#define PLAYER_TURN_INCREMENT_DEG 1.0f
#define PLAYER_TURN_INCREMENT_RAD (PLAYER_TURN_INCREMENT_DEG * (M_PI / 180.0f))

#define PLAYER_BASE_SPEED 450.0f

#define PLAYER_ACCELERATION 100.0f
#define PLAYER_DECELERATION 80.0f
#define PLAYER_SKID_DECELERATION 150.0f

#define PLAYER_SKID_ANGLE (M_PI / 12.0f)

#define AI_EASY_BASE_SPEED 420.0f
#define AI_MEDIUM_BASE_SPEED 450.0f
#define AI_HARD_BASE_SPEED 480.0f

#define AI_ACCELERATION 100.0f
#define AI_DECELERATION 80.0f

#define AI_EASY_LOOKAHEAD 336.0f // 0.8 seconds
#define AI_MEDIUM_LOOKAHEAD 450.0f // 1.0 seconds
#define AI_HARD_LOOKAHEAD 540.0f // 1.2 seconds

#define AI_MAX_STEERING_RATE_RAD_PER_SEC (M_PI / 2.0f)

#define AI_EASY_PATH_ADHERENCE 0.6f
#define AI_MEDIUM_PATH_ADHERENCE 0.8f
#define AI_HARD_PATH_ADHERENCE 1.0f
/**@}*/

#endif
