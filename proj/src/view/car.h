#ifndef CAR_H
#define CAR_H

#include "macros.h"
#include "sprite.h"

#define STEP 10

typedef struct Car {
    int x, y;
    int speed;
    Sprite *car_sprite;
} Car;

void car_move_left(Car *car, int min, int step);
void car_move_right(Car *car, int max, int step);

#endif

