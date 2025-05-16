#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdio.h>

#include "model/game_state.h"
#include "sprite.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "road.h"
#include "car.h"

typedef enum {
  GAME_SUBSTATE_PLAYING,
  GAME_SUBSTATE_GAME_OVER,
  GAME_FINISHED,
  GAME_EXITED,
} GameSubstate;

/*
typedef struct {
  int x, y;
  int speed;
  bool active;
  Sprite *obstacle_sprite;
} Obstacle;
*/

typedef struct Game {
    GameState base;
    GameSubstate currentSubstate;
    Car playerCar;
    Sprite *road_sprite1;
    Sprite *road_sprite2;
    int road_y1;
    int road_y2;
} Game;

// Public Game Class Methods
Game *game_create(int car_choice);
void game_destroy(Game *this);
void game_draw(Game *this);
void game_process_event(Game *this, EventType event);
void game_update_state(Game *this); // If you have update logic
GameSubstate game_get_current_substate(Game *this);
void game_reset_state(Game *this); // If needed

#endif

