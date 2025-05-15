#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "video_card.h"
#include "geometry.h"
#include "macros.h"

typedef struct {
    Point position;
    int width;
    int height;
    Vector direction;
    float speed;
    Vector velocity;
} Player;

void create_player(Player *this, Point *center, Vector *direction);
void player_movement(Player *this);
void player_set_speed(Player *this, float speed);
void player_turn(Player *this, int turn_direction_sign);

#endif // PLAYER_H
