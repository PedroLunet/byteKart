#ifndef GAME_H
#define GAME_H

#include "sprite.h"
#include "macros.h"
#include "road.h"
#include "xpm/xpm_files.h"


typedef enum {
  GAME_STATE_MENU,
  GAME_STATE_DIFFICULTY,
  GAME_STATE_TRACK,
  GAME_STATE_PLAYING,
  GAME_STATE_GAME_OVER,
  GAME_STATE_EXIT,
} GameState;

typedef struct {
 int x, y;
 int speed;
 Sprite *car_sprite;
} Car;

/*
typedef struct {
  int x, y;
  int speed;
  bool active;
  Sprite *obstacle_sprite;
} Obstacle;
*/

void game_init();
void game_draw();
void game_cleanup();

#endif

