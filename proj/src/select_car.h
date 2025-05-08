#ifndef SELECT_CAR_H
#define SELECT_CAR_H

#include <stdint.h>
#include "sprite.h"
#include "macros.h"

typedef enum {
    SELECT_CAR_CAR1,
    SELECT_CAR_CAR2,
    SELECT_CAR_CAR3,
    SELECT_BACK,
    SELECT_CAR_FINISHED
} SelectCarSubstate;

typedef struct {
    SelectCarSubstate currentSubstate;
    Sprite *car1Sprite;
    Sprite *car2Sprite;
    Sprite *car3Sprite;
    Sprite *backSprite;
    uint32_t centerX;
    uint32_t centerY;
    int mouse_x;
    int mouse_y;
} SelectCar;

// public SelectCar Class Method
SelectCar *select_car_create();
void select_car_destroy(SelectCar *this);
void select_car_draw(SelectCar *this);
void select_car_process_event(SelectCar *this, EventType event);
SelectCarSubstate select_car_get_current_substate(SelectCar *this);
void select_car_reset_state(SelectCar *this);

#endif //SELECT_CAR_H
