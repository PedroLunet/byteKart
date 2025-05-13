#ifndef SELECT_CAR_H
#define SELECT_CAR_H

#include "model/game_state.h"
#include "xpm/xpm_files.h"
#include "view/ui.h"
#include "macros.h"

typedef enum {
    CAR_START,
    CAR_FIRST,
    CAR_SECOND,
    CAR_THIRD,
    CAR_FOURTH,
    CAR_EXITED
} CarSelection;

typedef struct SelectCar {
    GameState base;
    int selectedOption;
    CarSelection chosenLevel;
    UIComponent *uiRoot;
} SelectCar;

// Public SelectCar Class Methods
SelectCar *select_car_create();
void select_car_destroy(SelectCar *this);
void select_car_draw(SelectCar *this);
void select_car_process_event(SelectCar *this, EventType event);
CarSelection select_car_get_chosen_level(SelectCar *this);
void select_car_reset_state(SelectCar *this);

#endif //SELECT_CAR_H
