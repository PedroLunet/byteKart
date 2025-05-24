#ifndef MACROS_H
#define MACROS_H

#include <math.h>

typedef enum {
  EVENT_NONE,
  EVENT_TIMER,
  EVENT_KEYBOARD,
  EVENT_MOUSE,
  // EVENT_SERIAL,
  NUM_EVENTS
} EventType;

// Tracks
#define TRACK_1_FILENAME "/home/lcom/labs/proj/src/tracks/track_1.dat"
#define TRACK_1_SURFACE_FILENAME "/home/lcom/labs/proj/src/tracks/track_surface.bin"

// Fonts
#define MAX_FONT_GLYPHS 382

// Boundaries for the road
#define RIGHT_BOUNDARY 480
#define LEFT_BOUNDARY 130

// ESC key
#define ESC_MAKECODE 0x01
#define ESC_BREAKCODE 0x81

// makecodes for arrow keys
#define UP_ARROW 0x48
#define DOWN_ARROW 0X50
#define LEFT_ARROW 0X4B
#define RIGHT_ARROW 0X4D
#define SPACEBAR 0x39
#define P_KEY 0x19

// breakcodes for arrow keys
#define UP_ARROW_BREAK 0xC8
#define DOWN_ARROW_BREAK 0XD0
#define LEFT_ARROW_BREAK 0XCB
#define RIGHT_ARROW_BREAK 0XCD
#define SPACEBAR_BREAK 0xB9
#define P_KEY_BREAK 0xD9

// Enter key
#define ENTER_KEY 0x1C
#define ENTER_BREAKCODE 0x9C

#define BACKGROUND_COLOR 0x000000
#define TRANSPARENT 0xFFFFFE

// Game
#define MAX_LAPS 3
#define MAX_AI_CARS 1

#define FINISH_LINE_SEGMENT_IDX 0
#define ACTUAL_FINISH_LINE_SEGMENT_IDX 50
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

#define AI_EASY_LOOKAHEAD 180.0f // 0.8s
#define AI_MEDIUM_LOOKAHEAD 250.0f // 1.0s
#define AI_HARD_LOOKAHEAD 340.0f // 1.2s

#define AI_MAX_STEERING_RATE_RAD_PER_SEC (M_PI / 2.0f)

#define AI_EASY_PATH_ADHERENCE 0.6f
#define AI_MEDIUM_PATH_ADHERENCE 0.8f
#define AI_HARD_PATH_ADHERENCE 1.0f

#endif
